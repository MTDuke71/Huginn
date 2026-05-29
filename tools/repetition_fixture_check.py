"""Quick fixture check for BACKLOG #28 Part 2: replay each regression
case into the current Huginn build and report whether it still plays the
clincher (repeats -> NOT fixed) or avoids it (FIXED). The positions are
already Stockfish-confirmed won, so no oracle is needed here -- only
Huginn's behavior changes between builds.

Usage: python tools/repetition_fixture_check.py [--movetime MS]
"""
import argparse, json, os, re, subprocess, sys, threading

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FIXTURE = os.path.join(ROOT, "tools", "repetition_regression.json")
ENGINE = os.path.join(ROOT, "build", "msvc-x64-release", "bin", "Release", "huginn.exe")
BEST_RE = re.compile(r"^bestmove (\S+)")


def drive(setup_cmds, go_cmd, watchdog_s):
    """Send setup + go, stream stdout until bestmove (go runs on a bg
    thread, so do not send quit before bestmove). Returns bestmove."""
    p = subprocess.Popen([ENGINE], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          text=True, bufsize=1)
    best = {"m": None}

    def kill():
        try:
            p.kill()
        except Exception:
            pass

    wd = threading.Timer(watchdog_s, kill)
    wd.start()
    try:
        p.stdin.write("uci\n"); p.stdin.flush()
        for c in setup_cmds:
            p.stdin.write(c + "\n")
        p.stdin.write(go_cmd + "\n")
        p.stdin.flush()
        for line in p.stdout:
            m = BEST_RE.match(line.strip())
            if m:
                best["m"] = m.group(1)
                break
    finally:
        wd.cancel()
        try:
            p.stdin.write("quit\n"); p.stdin.flush()
        except Exception:
            pass
        try:
            p.wait(timeout=3)
        except Exception:
            kill()
    return best["m"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--movetime", type=int, default=300)
    args = ap.parse_args()

    rows = json.load(open(FIXTURE))
    fixed = 0
    print(f"{'id':<28} {'subclass':<18} {'clincher':<8} {'huginn':<8} verdict")
    for r in rows:
        setup = ["position fen " + r["start_fen"] + " moves " + " ".join(r["moves_uci"])]
        hb = drive(setup, f"go movetime {args.movetime}", args.movetime / 1000.0 + 10)
        repeats = (hb == r["clincher"])
        if not repeats:
            fixed += 1
        verdict = "REPEATS" if repeats else "FIXED"
        print(f"{r['id']:<28} {r['subclass']:<18} {r['clincher']:<8} {str(hb):<8} {verdict}")
    print(f"\nFIXED {fixed}/{len(rows)} (avoided the clincher)")


if __name__ == "__main__":
    main()
