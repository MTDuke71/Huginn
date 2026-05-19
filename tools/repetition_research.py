"""Step 5 of BACKLOG #28: re-search the winning-repeat candidates.

For each candidate emitted by repetition_analysis.py:

  A. Replay the full game move history into the CURRENT Huginn build
     (`position fen <start_fen> moves ...`) and search at a realistic
     movetime, so the engine's repetition stack matches the in-game
     state and the t6 root-demotion can fire. Question: does current
     Huginn still clinch the repetition, or has it learned to avoid it?

  B. Probe Stockfish on `pre_fen` at fixed depth as an independent
     oracle. Question: is the position objectively winning for the
     clinching side, and what is the best non-repeating move?

Classification per candidate:
  REAL_BUG     Huginn still repeats AND Stockfish says winning
               (|cp| >= WIN_CP, no mate-for-other-side) -> regression set
  FIXED_BY_T6  Huginn now avoids the repetition AND SF says winning
               -> regression-prevention test (already handled)
  ARTIFACT     Stockfish says ~0 / drawn -> Huginn's big eval was wrong,
               not a thrown win -> discard

Usage:
    python tools/repetition_research.py [--movetime MS] [--sf-depth D]
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

import chess

REPO = Path(__file__).resolve().parent.parent
HUGINN = REPO / "build" / "msvc-x64-release" / "bin" / "Release" / "huginn.exe"
STOCKFISH = Path(r"C:\Users\m_lad\Documents"
                 r"\fastchess-windows-x86-64\stockfish.exe")
CANDIDATES = REPO / "tools" / "repetition_candidates.json"
OUT = REPO / "tools" / "repetition_research_results.json"

WIN_CP = 300  # objective "winning" bar on the Stockfish side-to-move score

SCORE_RE = re.compile(r"score (cp|mate) (-?\d+)")
BEST_RE = re.compile(r"^bestmove (\S+)")


def run_uci(exe: Path, setup: str, go: str, timeout: float):
    """Drive a UCI engine: send `setup` then `go`, stream stdout until
    `bestmove`, then quit. `go` runs on a background thread in most
    engines, so we must NOT send `quit` until bestmove is seen.

    Returns (bestmove, score) with score {kind: cp|mate, value:int} from
    the side-to-move POV (last info line before bestmove)."""
    proc = subprocess.Popen(
        [str(exe)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL, text=True, bufsize=1,
    )
    proc.stdin.write(setup)
    proc.stdin.write(go)
    proc.stdin.flush()

    best, last_score = None, None
    try:
        for line in proc.stdout:
            sm = SCORE_RE.search(line)
            if sm:
                last_score = {"kind": sm.group(1), "value": int(sm.group(2))}
            bm = BEST_RE.match(line.strip())
            if bm:
                best = bm.group(1)
                break
        proc.stdin.write("quit\n")
        proc.stdin.flush()
        proc.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
    return best, last_score


def cp_from_pov(score, winning_side_is_stm: bool):
    """Normalize a side-to-move score to the clinching side's POV in cp.
    Mate is mapped to a large signed value."""
    if score is None:
        return None
    v = score["value"]
    if score["kind"] == "mate":
        v = 100000 if v > 0 else -100000
    else:
        v = v  # already cp
    return v if winning_side_is_stm else -v


def classify(huginn_repeats, sf_cp):
    if sf_cp is None:
        return "UNKNOWN"
    if abs(sf_cp) < WIN_CP and sf_cp < WIN_CP:
        # Stockfish does not see a win for the clincher side.
        return "ARTIFACT"
    return "REAL_BUG" if huginn_repeats else "FIXED_BY_T6"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--movetime", type=int, default=300,
                    help="Huginn movetime ms (approx in-game 10+0.1 think)")
    ap.add_argument("--sf-depth", type=int, default=24,
                    help="Stockfish fixed search depth (oracle)")
    ap.add_argument("--limit", type=int, default=0,
                    help="only first N candidates (0 = all)")
    args = ap.parse_args()

    rows = json.load(open(CANDIDATES))
    cands = [r for r in rows if r["is_winning_repeat_candidate"]]
    if args.limit:
        cands = cands[: args.limit]

    results = []
    tally = {}
    for i, r in enumerate(cands, 1):
        pre_fen = r["pre_fen"]
        stm_is_clincher = True  # pre_fen's side to move IS the clincher
        clincher = r["clincher_move_uci"]

        # A. Current Huginn, full history replayed.
        h_best, h_score = run_uci(
            HUGINN,
            "uci\nisready\n"
            f"position fen {r['start_fen']} moves {' '.join(r['moves_uci'])}\n",
            f"go movetime {args.movetime}\n",
            timeout=30,
        )
        huginn_repeats = (h_best == clincher)

        # B. Stockfish oracle on the bare pre-repetition position.
        s_best, s_score = run_uci(
            STOCKFISH,
            f"uci\nisready\nposition fen {pre_fen}\n",
            f"go depth {args.sf_depth}\n",
            timeout=60,
        )
        sf_cp = cp_from_pov(s_score, stm_is_clincher)
        sf_best_repeats = (s_best == clincher)

        verdict = classify(huginn_repeats, sf_cp)
        tally[verdict] = tally.get(verdict, 0) + 1

        row = {
            "id": f"{r['source']}-R{r['round']}",
            "pre_fen": pre_fen,
            "clincher": clincher,
            "pgn_reported_cp": r["clincher_cp"],
            "huginn_bestmove": h_best,
            "huginn_score": h_score,
            "huginn_repeats": huginn_repeats,
            "sf_bestmove": s_best,
            "sf_cp_clincher_pov": sf_cp,
            "sf_best_is_repetition": sf_best_repeats,
            "n_non_repeating_alternatives": r["n_non_repeating_alternatives"],
            "verdict": verdict,
        }
        results.append(row)
        print(f"[{i:2}/{len(cands)}] {row['id']:<28} "
              f"pgn={r['clincher_cp']:>6} "
              f"H:{str(h_best):<6}{'REP' if huginn_repeats else '   '} "
              f"SF:{str(s_best):<6} SFcp={str(sf_cp):>7}  -> {verdict}")

    OUT.write_text(json.dumps(results, indent=2), encoding="utf-8")
    print("\n=== tally ===")
    for k, v in sorted(tally.items()):
        print(f"  {k:<12} {v}")
    print(f"\nwrote {OUT.relative_to(REPO)}")


if __name__ == "__main__":
    main()
