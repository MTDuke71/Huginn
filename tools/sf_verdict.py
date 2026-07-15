"""
Stockfish's opinion on a WAC/LCT2 test-log's FAILED positions: for each
failure, get SF's own top choice + eval, then SF's verdict (mover-POV cp)
on the *expected* ("bm") move vs the move the engine actually played.
Distinguishes "genuine miss" (expected move is clearly better) from
"dual-solution / close call" (moves are within a few cp of each other, or
the engine's move even scores higher than the tagged bm).

Usage:
    python tools/sf_verdict.py <wac_or_lct2_test_log.txt> [--depth D]

Parses the log's "Testing <id> / FEN: ... / RESULT: FAIL / Engine move: ..."
blocks (same format wac_test.py, wac201_test.py, and lct2_test.py all write).
"""
import sys, re, argparse
import chess, chess.engine

SF = r"C:\Users\m_lad\Documents\Chess\stockfish171\stockfish-windows-x86-64-avx2.exe"

BLOCK_RE = re.compile(
    r"Testing (?P<id>\S+)\n"
    r"FEN: (?P<fen>[^\n]+)\n"
    r"Expected best moves: (?P<expected>[^\n]+)\n"
    r"(?:(?!\nTesting )[\s\S])*?"
    r"RESULT: FAIL\nEngine move: (?P<engine_move>\S+)")

def cp(pov):
    return pov.relative.score(mate_score=100000)

def analyse(board, depth):
    """Fresh SF process per call — avoids any position-tracking desync in a
    long-lived engine handle across many unrelated one-off positions."""
    engine = chess.engine.SimpleEngine.popen_uci(SF)
    try:
        try:
            engine.configure({"OwnBook": False})
        except Exception:
            pass
        return engine.analyse(board, chess.engine.Limit(depth=depth))
    finally:
        engine.quit()

def sf_cp_after(board, move, depth):
    b2 = board.copy()
    b2.push(move)
    info = analyse(b2, depth)
    return -cp(info["score"])

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log_file")
    ap.add_argument("--depth", type=int, default=18)
    a = ap.parse_args()

    text = open(a.log_file, encoding="utf-8", errors="replace").read()
    fails = [m.groupdict() for m in BLOCK_RE.finditer(text)]
    if not fails:
        print("No FAILED positions found in this log.")
        return
    print(f"{len(fails)} failed position(s); Stockfish depth {a.depth} verdict:\n")

    rows = []
    for entry in fails:
        board = chess.Board(entry["fen"])
        try:
            # SF's own top choice + eval of the raw position (ground truth).
            base = analyse(board, a.depth)
            sf_best = base["pv"][0]
            sf_best_cp = cp(base["score"])
        except Exception as e:
            print(f"  {entry['id']:16} FAILED base analysis: {e}", flush=True)
            continue

        try:
            exp_move = board.parse_san(entry["expected"].split(",")[0].strip())
            exp_cp = sf_cp_after(board, exp_move, a.depth)
        except Exception:
            exp_cp = None

        try:
            eng_move = chess.Move.from_uci(entry["engine_move"])
            eng_cp = sf_cp_after(board, eng_move, a.depth)
        except Exception:
            eng_cp = None

        rows.append((entry["id"], board.san(sf_best), sf_best_cp,
                     entry["expected"], exp_cp, entry["engine_move"], eng_cp))
        print(f"  {entry['id']:16} done", flush=True)

    print(f"\n{'id':16} {'SF best':10} {'SF cp':>7}   "
          f"{'expected':10} {'exp cp':>7}   {'played':8} {'played cp':>9}   gap")
    print("-" * 100)
    for (id_, sf_best, sf_best_cp, expected, exp_cp, played, eng_cp) in rows:
        gap = (exp_cp - eng_cp) if (exp_cp is not None and eng_cp is not None) else None
        gap_s = f"{gap:+d}" if gap is not None else "?"
        print(f"  {id_:16} {sf_best:10} {sf_best_cp:>7}   "
              f"{expected:10} {str(exp_cp):>7}   {played:8} {str(eng_cp):>9}   {gap_s}")

if __name__ == "__main__":
    main()
