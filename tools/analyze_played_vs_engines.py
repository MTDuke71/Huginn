"""
Compare every position in a (variation-tree) PGN three ways:
  played move  vs  Stockfish best  vs  Huginn 2.1 best,
and score how much Huginn's chosen move loses vs SF's best (cp-loss).

Walks the FULL variation tree (not just the mainline), dedups positions by FEN
(transpositions included), and writes one CSV row per unique position with both
engines' best move, eval, and search depth. Resumable: re-running continues
from where the CSV left off, so an overnight run survives a crash/restart.

Usage:
    python analyze_played_vs_engines.py [--limit N] [--sf-depth D] [--hug-time T] [--out PATH]
      --limit N     analyze only the first N unique positions (for a quick test)
      --sf-depth D  Stockfish ground-truth depth   (default 22)
      --hug-time T  Huginn seconds/move            (default 1.5)
      --out PATH    output CSV                      (default tools/cc_analysis.csv)
Paths to the PGN / Stockfish / Huginn binaries are the constants below — edit if
they differ on the machine you run this on.
"""
import sys, os, csv, time, argparse
import chess, chess.pgn, chess.engine

# ---- edit these if paths differ on your machine ----------------------------
PGN = r"C:\Users\m_lad\Documents\fastchess-windows-x86-64\CC_Games.pgn"
SF  = r"C:\Users\m_lad\Documents\Chess\stockfish171\stockfish-windows-x86-64-avx2.exe"
HUG = r"C:\Users\m_lad\Documents\fastchess-windows-x86-64\huginn_t15.exe"   # Huginn 2.1
# ----------------------------------------------------------------------------

FIELDS = ["idx","game","ply","fullmove","side","phase","material_cp",
          "played","sf_best","sf_eval_cp","sf_depth",
          "hug_best","hug_eval_cp","hug_depth","sf_eval_after_hug_cp",
          "hug_eq_sf","played_eq_sf","hug_cploss",
          "sf_best_is_capture","sf_best_is_check","fen","line"]

PIECE_CP = {chess.PAWN:100, chess.KNIGHT:320, chess.BISHOP:330,
            chess.ROOK:500, chess.QUEEN:900, chess.KING:0}

def cp(pov):  # PovScore -> cp from side-to-move POV (mates as large cp)
    return pov.relative.score(mate_score=100000)

def material_cp(b):  # white - black, then side-to-move POV
    s = 0
    for pt, v in PIECE_CP.items():
        s += v*(len(b.pieces(pt, chess.WHITE)) - len(b.pieces(pt, chess.BLACK)))
    return s if b.turn == chess.WHITE else -s

def phase_of(b):
    npm = sum(len(b.pieces(pt,c)) for pt in (chess.KNIGHT,chess.BISHOP,chess.ROOK,chess.QUEEN)
              for c in (chess.WHITE,chess.BLACK))
    queens = len(b.pieces(chess.QUEEN,chess.WHITE))+len(b.pieces(chess.QUEEN,chess.BLACK))
    if b.fullmove_number <= 12: return "opening"
    if npm <= 6 or queens == 0:  return "endgame"
    return "middlegame"

def collect_positions(game):
    """DFS pre-order over the whole tree; one entry per unique FEN (first hit)."""
    out, seen = [], set()
    root_board = game.board()
    def rec(node):
        if node.variations:
            b = node.board()
            fen = b.fen()
            if fen not in seen:
                seen.add(fen)
                moves, n = [], node
                while n.parent is not None:
                    moves.append(n.move); n = n.parent
                moves.reverse()
                line = root_board.variation_san(moves) if moves else "(start)"
                out.append((b, node.variations[0].move, node.ply(), line))
            for ch in node.variations:
                rec(ch)
    rec(game)
    return out

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--sf-depth", type=int, default=22)
    ap.add_argument("--hug-time", type=float, default=1.5)
    ap.add_argument("--out", default=os.path.join("tools","cc_analysis.csv"))
    a = ap.parse_args()

    games, seen = [], set()
    with open(PGN, encoding="utf-8-sig") as f:
        while (g := chess.pgn.read_game(f)) is not None:
            sig = tuple(m.uci() for m in g.mainline_moves())
            if sig in seen: continue
            seen.add(sig); games.append(g)

    positions = []
    for gi, g in enumerate(games, 1):
        for (b, played, ply, line) in collect_positions(g):
            positions.append((gi, b, played, ply, line))
    if a.limit: positions = positions[:a.limit]
    total = len(positions)
    print(f"{len(games)} unique game(s); {total} unique positions to analyze")

    done = 0
    if os.path.exists(a.out):
        with open(a.out, newline="") as f:
            done = max(sum(1 for _ in csv.reader(f)) - 1, 0)
        print(f"resuming: {done} positions already done")

    sf  = chess.engine.SimpleEngine.popen_uci(SF)
    hug = chess.engine.SimpleEngine.popen_uci(HUG)
    for e in (sf, hug):
        try: e.configure({"OwnBook": False})
        except Exception: pass

    new_file = not os.path.exists(a.out) or done == 0
    f = open(a.out, "w" if new_file else "a", newline="")
    w = csv.DictWriter(f, fieldnames=FIELDS)
    if new_file: w.writeheader(); f.flush()

    t0 = time.time()
    for idx in range(done, total):
        gi, b, played_mv, ply, line = positions[idx]
        si = sf.analyse(b, chess.engine.Limit(depth=a.sf_depth))
        sf_best = si["pv"][0]; sf_eval = cp(si["score"]); sf_depth = si.get("depth")
        hi = hug.analyse(b, chess.engine.Limit(time=a.hug_time))
        hug_best = hi["pv"][0]; hug_eval = cp(hi["score"]); hug_depth = hi.get("depth")
        if hug_best == sf_best:
            cploss = 0; sf_after = sf_eval          # same move as SF's best
        else:
            b2 = b.copy(); b2.push(hug_best)
            ai = sf.analyse(b2, chess.engine.Limit(depth=a.sf_depth))
            # SF's verdict on Huginn's move, expressed from the MOVING side's POV
            # (so it's directly comparable to sf_eval_cp); cp-loss is the gap.
            sf_after = -cp(ai["score"])
            cploss = sf_eval - sf_after
        w.writerow(dict(
            idx=idx, game=gi, ply=ply, fullmove=b.fullmove_number,
            side=("W" if b.turn==chess.WHITE else "B"), phase=phase_of(b),
            material_cp=material_cp(b),
            played=b.san(played_mv),
            sf_best=b.san(sf_best), sf_eval_cp=sf_eval, sf_depth=sf_depth,
            hug_best=b.san(hug_best), hug_eval_cp=hug_eval, hug_depth=hug_depth,
            sf_eval_after_hug_cp=sf_after,
            hug_eq_sf=int(hug_best==sf_best),
            played_eq_sf=int(played_mv==sf_best),
            hug_cploss=cploss,
            sf_best_is_capture=int(b.is_capture(sf_best)),
            sf_best_is_check=int(b.gives_check(sf_best)),
            fen=b.fen(), line=line))
        f.flush()
        n = idx - done + 1
        if n % 25 == 0 or idx == total-1:
            el = time.time()-t0; rate = el/n; eta = rate*(total-1-idx)
            print(f"  {idx+1}/{total}  {rate:.2f}s/pos  ETA {eta/60:.0f} min", flush=True)
    f.close(); sf.quit(); hug.quit()
    print(f"done -> {a.out}")

if __name__ == "__main__":
    main()
