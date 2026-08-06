"""Sample mainline positions from a PGN as an EPD-ish FEN list.

Skips opening-book territory and near-terminal positions, spaces the samples
out within each game so consecutive picks aren't near-duplicates, and dedupes
across games.

usage: python extract_pos.py <pgn> <out.txt> [max_positions] [stride]
"""
import sys

import chess
import chess.pgn

src = sys.argv[1]
out = sys.argv[2]
cap = int(sys.argv[3]) if len(sys.argv) > 3 else 50
stride = int(sys.argv[4]) if len(sys.argv) > 4 else 11

MIN_PLY = 16      # past the book
MAX_PLY = 90      # avoid dead-drawn tails
MIN_PIECES = 8    # avoid trivially-solved endgames (and TB territory)

seen = set()
candidates = []

with open(src, encoding="utf-8", errors="replace") as fh:
    game_no = 0
    while True:
        game = chess.pgn.read_game(fh)
        if game is None:
            break
        game_no += 1

        # Walk the WHOLE game tree, not just the mainline. This PGN is an
        # annotated analysis file: mainlines are ~40 plies but the variations
        # are long and are where most of the real positions live.
        stack = [game]
        while stack:
            node = stack.pop()
            stack.extend(node.variations)

            if node.parent is None:
                continue
            board = node.board()
            ply = board.ply()
            if ply < MIN_PLY or ply > MAX_PLY:
                continue
            if (ply - MIN_PLY) % stride:
                continue
            if board.is_game_over():
                continue
            if len(board.piece_map()) < MIN_PIECES:
                continue
            fen = board.fen()
            key = " ".join(fen.split()[:4])   # ignore clocks for dedupe
            if key in seen:
                continue
            seen.add(key)
            candidates.append(fen)

# Spread the sample evenly over the whole candidate set rather than taking the
# first `cap` — a depth-first walk would otherwise return one dense corner of
# the variation tree, which is the opposite of a representative sample.
if len(candidates) > cap:
    step = len(candidates) / float(cap)
    picked = [candidates[int(i * step)] for i in range(cap)]
else:
    picked = candidates

with open(out, "w", encoding="utf-8") as fh:
    for fen in picked:
        fh.write(fen + "\n")

print(f"{len(picked)} of {len(candidates)} eligible positions "
      f"from {game_no} game(s) -> {out}")
