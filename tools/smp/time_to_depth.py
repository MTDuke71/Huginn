"""Time-to-depth scaling across thread counts.

For each thread count, runs `go depth D` and records the wall-clock time and
node count of EVERY completed iteration, so we can compare time-to-depth at
each depth rather than only the final one.

Anything above 1 thread is nondeterministic, so each config runs N times and
we report the median.

usage: python ttd.py <exe> <depth> <fen|startpos> <runs> <threads...>
"""
import statistics
import subprocess
import sys

exe = sys.argv[1]
depth = int(sys.argv[2])
pos = sys.argv[3]
runs = int(sys.argv[4])
thread_counts = [int(t) for t in sys.argv[5:]]

posline = "position startpos" if pos == "startpos" else f"position fen {pos}"


def one_run(threads):
    """Returns {depth: (time_ms, nodes)} for every iteration reported."""
    p = subprocess.Popen(exe, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         text=True, bufsize=1)
    p.stdin.write("uci\n")
    p.stdin.write(f"setoption name Threads value {threads}\n")
    p.stdin.write("setoption name Hash value 64\n")
    p.stdin.write("isready\n")
    p.stdin.write(f"ucinewgame\n{posline}\ngo depth {depth}\n")
    p.stdin.flush()

    out = {}
    while True:
        line = p.stdout.readline()
        if not line:
            break
        line = line.strip()
        if line.startswith("info depth"):
            f = line.split()
            try:
                d = int(f[f.index("depth") + 1])
                t = int(f[f.index("time") + 1])
                n = int(f[f.index("nodes") + 1])
                out[d] = (t, n)
            except (ValueError, IndexError):
                pass
        if line.startswith("bestmove"):
            break

    p.stdin.write("quit\n")
    p.stdin.flush()
    try:
        p.wait(timeout=5)
    except subprocess.TimeoutExpired:
        p.kill()
    return out


results = {}
for tc in thread_counts:
    per_depth = {}
    for r in range(runs):
        for d, (t, n) in one_run(tc).items():
            per_depth.setdefault(d, []).append((t, n))
    results[tc] = per_depth
    print(f"  ...threads={tc} done", file=sys.stderr)

# --- report -------------------------------------------------------------
depths = sorted(results[thread_counts[0]].keys())
show = [d for d in depths if d >= max(6, depth - 6)]

hdr = f"{'depth':>6} |" + "".join(f"{('t' + str(t)):>22}" for t in thread_counts)
print(hdr)
print("-" * len(hdr))
base = thread_counts[0]
for d in show:
    row = f"{d:>6} |"
    base_t = statistics.median(t for t, _ in results[base].get(d, [(0, 0)]))
    for tc in thread_counts:
        samples = results[tc].get(d)
        if not samples:
            row += f"{'-':>22}"
            continue
        mt = statistics.median(t for t, _ in samples)
        mn = statistics.median(n for _, n in samples)
        speed = (base_t / mt) if mt > 0 else 0.0
        row += f"{mt:>8}ms {mn/1e6:>6.2f}M {speed:>4.2f}x"
    print(row)

print()
print(f"median of {runs} runs per config; speedup is vs t{base} at the same depth")
