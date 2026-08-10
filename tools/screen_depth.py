#!/usr/bin/env python3
"""Screen a candidate binary against a baseline by depth reached at fixed time.

Why this and not fixed-depth node counts: measured 2026-08-07 (BACKLOG #63,
#68-#70), fixed-depth node count predicted *none* of three SPRT verdicts while
depth-at-fixed-time predicted all three. Node count at fixed depth measures work
at a depth no game ever plays to; depth at fixed time measures what a clock
actually buys. Two minutes per binary here has repeatedly saved hours of arena.

Reads it as a screen, not a verdict: a candidate that loses depth can still win
on eval quality (an NNUE arm is exactly that case), so interpret the delta
alongside what the change is supposed to do.

usage:
    python tools/screen_depth.py <baseline.exe> <candidate.exe> [positions.epd]
                                 [--movetime 1000] [--n 40] [--option "name=value"]...

`--option` is applied to the CANDIDATE only (repeatable), for A/B-ing a single
binary against itself with a flag flipped.
"""

import argparse
import random
import statistics
import subprocess
import sys


def run_one(exe, fen, movetime, options):
    p = subprocess.Popen(exe, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         text=True, bufsize=1)
    p.stdin.write("uci\n")
    for o in options:
        name, _, value = o.partition("=")
        p.stdin.write("setoption name %s value %s\n" % (name, value))
    p.stdin.write("ucinewgame\nisready\n")
    p.stdin.write("position fen %s\ngo movetime %d\n" % (fen, movetime))
    p.stdin.flush()

    depth, nodes = 0, 0
    while True:
        line = p.stdout.readline()
        if not line:
            break
        line = line.strip()
        if line.startswith("info depth"):
            f = line.split()
            try:
                depth = int(f[f.index("depth") + 1])
                if "nodes" in f:
                    nodes = int(f[f.index("nodes") + 1])
            except (ValueError, IndexError):
                pass
        elif line.startswith("bestmove"):
            break
    try:
        p.stdin.write("quit\n")
        p.stdin.flush()
        p.wait(timeout=5)
    except Exception:
        p.kill()
    return depth, nodes


def load_positions(path, n):
    fens = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            # Accept EPD (4 fields), full FEN (6), or "<score> <fen>" datagen lines.
            parts = line.split()
            if parts[0].lstrip("+-").isdigit() and len(parts) >= 5:
                parts = parts[1:]
            if len(parts) < 4:
                continue
            fen = " ".join(parts[:4])
            if len(parts) >= 6 and parts[4].isdigit():
                fen = " ".join(parts[:6])
            else:
                fen += " 0 1"
            fens.append(fen)
    random.Random(20260809).shuffle(fens)
    return fens[:n]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("baseline")
    ap.add_argument("candidate")
    ap.add_argument("positions", nargs="?", default="tools/smp/cc_positions.epd")
    ap.add_argument("--movetime", type=int, default=1000)
    ap.add_argument("--n", type=int, default=40)
    ap.add_argument("--option", action="append", default=[],
                    help="setoption applied to the CANDIDATE (repeatable)")
    ap.add_argument("--base-option", action="append", default=[],
                    help="setoption applied to the BASELINE (repeatable). Needed "
                         "whenever the two arms are the same binary and the "
                         "feature under test defaults ON -- otherwise the "
                         "'baseline' is silently the candidate.")
    a = ap.parse_args()

    fens = load_positions(a.positions, a.n)
    if not fens:
        sys.exit("no positions parsed from %s" % a.positions)
    print("screening %d positions at %d ms" % (len(fens), a.movetime))
    print("  baseline : %s%s"
          % (a.baseline, ("  [" + ", ".join(a.base_option) + "]") if a.base_option else ""))
    print("  candidate: %s%s"
          % (a.candidate, ("  [" + ", ".join(a.option) + "]") if a.option else ""))
    print()

    deltas, b_depths, c_depths = [], [], []
    for i, fen in enumerate(fens, 1):
        bd, _ = run_one(a.baseline, fen, a.movetime, a.base_option)
        cd, _ = run_one(a.candidate, fen, a.movetime, a.option)
        deltas.append(cd - bd)
        b_depths.append(bd)
        c_depths.append(cd)
        print("  %3d/%d  base d%-3d cand d%-3d  %+d" % (i, len(fens), bd, cd, cd - bd),
              flush=True)

    mean = statistics.mean(deltas)
    med = statistics.median(deltas)
    stdev = statistics.stdev(deltas) if len(deltas) > 1 else 0.0
    stderr = stdev / (len(deltas) ** 0.5) if deltas else 0.0

    print()
    print("baseline  mean depth %.2f" % statistics.mean(b_depths))
    print("candidate mean depth %.2f" % statistics.mean(c_depths))
    print("delta     %+.3f ply  (median %+.1f, stderr %.3f)" % (mean, med, stderr))
    print("          won %d / lost %d / tied %d"
          % (sum(d > 0 for d in deltas), sum(d < 0 for d in deltas),
             sum(d == 0 for d in deltas)))
    print()
    # Verdict hint only. The arena settles it; this decides whether the arena is
    # worth booking.
    if mean <= -0.60:
        print("HINT: large depth loss. Historically these did not survive SPRT "
              "unless the change buys a lot of eval quality per node.")
    elif mean <= -0.15:
        print("HINT: mild depth loss — worth running only if the change is an "
              "eval-quality change (an NNUE arm is the standard example).")
    elif mean < 0.15:
        print("HINT: depth-neutral. Verdict rests entirely on move quality.")
    else:
        print("HINT: depth gain. Worth the arena.")


if __name__ == "__main__":
    main()
