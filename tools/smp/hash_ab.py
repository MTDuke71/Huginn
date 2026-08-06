"""Controlled Hash A/B: identical positions, fixed movetime and Threads, Hash varies.

Sibling of depth_ab.py. That tool holds Hash at 64MB and varies Threads; this
one holds Threads fixed and varies Hash, because at Threads>1 the engine
searches ~5.6x the nodes in the same wall-clock and the TT pressure question
changes completely.

WHY NOT A SINGLE POSITION: under Lazy SMP the search is *deliberately*
nondeterministic (that is the entire mechanism), so time-to-depth on one
position has enormous run-to-run variance — a t8 depth-17 probe on Kiwipete
spanned 4022-5228ms across three runs of the SAME config. Averaging depth at
fixed movetime over ~40 positions is the instrument the lab already validated
(depth_ab.py agreed with the PGN move-pairing method to 0.07 ply).

WHY BLITZ IS POINTLESS HERE (BACKLOG #31's lesson, re-measured 2026-08-06):
hashfull at Hash=64 is 15 permille at t1/550ms and only 31 at t8/550ms — the
table is essentially empty, so a blitz sweep measures nothing. At an LTC-sized
budget it bites: 58 permille at t1/3300ms, 300 at t8/3300ms. Use a movetime in
the LTC range (>=3000ms) or this tool will faithfully report "no difference".

LIMITATION: `ucinewgame` is sent before every position, so this measures
WITHIN-search TT pressure only. Real games also accumulate across moves (#46
clears only on ucinewgame), which makes real pressure strictly higher than
measured here. Treat a null result as "no within-search effect", not as proof
the sweep is closed — that still needs a gauntlet.

usage: python hash_ab.py <exe> <positions.txt> <movetime_ms> <runs> <threads> <hash_mb...>
"""
import statistics
import subprocess
import sys

exe = sys.argv[1]
positions = [l.strip() for l in open(sys.argv[2]) if l.strip()]
movetime = int(sys.argv[3])
runs = int(sys.argv[4])
threads = int(sys.argv[5])
hash_sizes = [int(h) for h in sys.argv[6:]]


def search(proc, fen):
    """Run one position; return (depth, seldepth, nodes, hashfull)."""
    proc.stdin.write("ucinewgame\nisready\n")
    proc.stdin.flush()
    while True:
        line = proc.stdout.readline()
        if not line or line.strip() == "readyok":
            break

    proc.stdin.write(f"position fen {fen}\ngo movetime {movetime}\n")
    proc.stdin.flush()

    depth = seldepth = nodes = hashfull = 0
    while True:
        line = proc.stdout.readline()
        if not line:
            break
        line = line.strip()
        if line.startswith("info depth"):
            f = line.split()
            try:
                depth = int(f[f.index("depth") + 1])
                seldepth = int(f[f.index("seldepth") + 1])
                nodes = int(f[f.index("nodes") + 1])
                if "hashfull" in f:
                    hashfull = int(f[f.index("hashfull") + 1])
            except (ValueError, IndexError):
                pass
        elif line.startswith("bestmove"):
            break
    return depth, seldepth, nodes, hashfull


def run_config(hash_mb, n_runs):
    acc = {i: [] for i in range(len(positions))}
    for _ in range(n_runs):
        p = subprocess.Popen(exe, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             text=True, bufsize=1)
        p.stdin.write("uci\n")
        p.stdin.write(f"setoption name Threads value {threads}\n")
        p.stdin.write(f"setoption name Hash value {hash_mb}\n")
        p.stdin.flush()
        while True:
            line = p.stdout.readline()
            if not line or line.strip() == "uciok":
                break

        for i, fen in enumerate(positions):
            acc[i].append(search(p, fen))

        p.stdin.write("quit\n")
        p.stdin.flush()
        try:
            p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            p.kill()

    return {i: (statistics.median(d for d, _, _, _ in v),
                statistics.median(s for _, s, _, _ in v),
                statistics.median(n for _, _, n, _ in v),
                statistics.median(h for _, _, _, h in v))
            for i, v in acc.items()}


results = {}
for h in hash_sizes:
    results[h] = run_config(h, runs)
    print(f"  ...Hash={h}MB done", file=sys.stderr)

base = hash_sizes[0]
print(f"\n{len(positions)} positions, movetime {movetime}ms, Threads={threads}, "
      f"median of {runs} runs\n")

hdr = (f"{'Hash':<8}{'depth avg':>11}{'seldepth':>10}{'Mnodes':>9}{'hashfull':>10}"
       f"{'vs base ply':>13}{'deeper':>8}{'equal':>7}{'shallower':>11}")
print(hdr)
print("-" * len(hdr))

for h in hash_sizes:
    r = results[h]
    n = len(positions)
    depths = [r[i][0] for i in range(n)]
    diffs = [r[i][0] - results[base][i][0] for i in range(n)]
    up = sum(1 for d in diffs if d > 0)
    eq = sum(1 for d in diffs if d == 0)
    dn = sum(1 for d in diffs if d < 0)
    # paired stderr on the depth delta -- the number that says whether the
    # mean difference is distinguishable from zero at all
    se = (statistics.stdev(diffs) / len(diffs) ** 0.5) if h != base and len(diffs) > 1 else 0.0
    mean_d = statistics.mean(diffs)
    ci = f" +/-{1.96*se:.2f}" if se else ""
    print(f"{str(h)+'MB':<8}{statistics.mean(depths):>11.2f}"
          f"{statistics.mean([r[i][1] for i in range(n)]):>10.2f}"
          f"{statistics.mean([r[i][2] for i in range(n)])/1e6:>9.2f}"
          f"{statistics.mean([r[i][3] for i in range(n)]):>10.0f}"
          f"{mean_d:>+9.2f}{ci:<4}"
          f"{up:>8}{eq:>7}{dn:>11}")

print("\nA mean delta smaller than its 95% CI is not a result. Under Lazy SMP "
      "expect wide CIs;\nif so, raise <runs> or accept that only a gauntlet "
      "can settle it.")
