"""Controlled depth A/B: identical positions, fixed movetime, Threads varies.

This is the confound-free version of the PGN move-pairing comparison. Each
thread count searches the SAME position for the SAME wall-clock budget, so the
only difference is the thread count. Depth deltas are paired per position.

`ucinewgame` is sent before every position so no TT state carries over.

usage: python depth_ab.py <exe> <positions.txt> <movetime_ms> <runs> <threads...>
"""
import statistics
import subprocess
import sys

exe = sys.argv[1]
positions = [l.strip() for l in open(sys.argv[2]) if l.strip()]
movetime = int(sys.argv[3])
runs = int(sys.argv[4])
thread_counts = [int(t) for t in sys.argv[5:]]


def search(proc, fen):
    """Run one position; return (depth, seldepth, nodes)."""
    proc.stdin.write("ucinewgame\nisready\n")
    proc.stdin.flush()
    while True:
        line = proc.stdout.readline()
        if not line or line.strip() == "readyok":
            break

    proc.stdin.write(f"position fen {fen}\ngo movetime {movetime}\n")
    proc.stdin.flush()

    depth = seldepth = nodes = 0
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
            except (ValueError, IndexError):
                pass
        elif line.startswith("bestmove"):
            break
    return depth, seldepth, nodes


def run_config(threads, n_runs):
    """Returns {pos_index: (median_depth, median_seldepth, median_nodes)}."""
    acc = {i: [] for i in range(len(positions))}
    for _ in range(n_runs):
        p = subprocess.Popen(exe, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             text=True, bufsize=1)
        p.stdin.write("uci\n")
        p.stdin.write(f"setoption name Threads value {threads}\n")
        p.stdin.write("setoption name Hash value 64\n")
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

    return {i: (statistics.median(d for d, _, _ in v),
                statistics.median(s for _, s, _ in v),
                statistics.median(n for _, _, n in v))
            for i, v in acc.items()}


results = {}
for tc in thread_counts:
    # t1 is deterministic, so one run is enough.
    n = 1 if tc == 1 else runs
    results[tc] = run_config(tc, n)
    print(f"  ...threads={tc} ({n} run(s)) done", file=sys.stderr)

base = thread_counts[0]
print(f"\n{len(positions)} positions, movetime {movetime}ms, "
      f"median of {runs} runs (t1: 1, deterministic)\n")

hdr = f"{'config':<9}{'depth avg':>11}{'seldepth':>10}{'Mnodes':>9}"
hdr += f"{'vs t1 ply':>11}{'deeper':>8}{'equal':>7}{'shallower':>11}"
print(hdr)
print("-" * len(hdr))

for tc in thread_counts:
    r = results[tc]
    depths = [r[i][0] for i in range(len(positions))]
    seld = [r[i][1] for i in range(len(positions))]
    nodes = [r[i][2] for i in range(len(positions))]

    diffs = [r[i][0] - results[base][i][0] for i in range(len(positions))]
    up = sum(1 for d in diffs if d > 0)
    eq = sum(1 for d in diffs if d == 0)
    dn = sum(1 for d in diffs if d < 0)

    print(f"t{tc:<8}{statistics.mean(depths):>11.2f}"
          f"{statistics.mean(seld):>10.2f}"
          f"{statistics.mean(nodes)/1e6:>9.2f}"
          f"{statistics.mean(diffs):>+11.2f}"
          f"{up:>8}{eq:>7}{dn:>11}")

print("\nper-position depth delta vs t1:")
for tc in thread_counts[1:]:
    diffs = [results[tc][i][0] - results[base][i][0] for i in range(len(positions))]
    hist = {}
    for d in diffs:
        hist[d] = hist.get(d, 0) + 1
    print(f"  t{tc}: " + "  ".join(f"{k:+d}:{hist[k]}" for k in sorted(hist)))
