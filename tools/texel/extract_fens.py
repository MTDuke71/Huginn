#!/usr/bin/env python3
"""
Texel training-data extractor (#9).

Reads one or more PGN files and emits labeled, lightly-filtered positions for
Texel tuning of Huginn's evaluation. Each output line is:

    <result> <FEN>

where <result> is the GAME's outcome from White's point of view (1.0 win,
0.5 draw, 0.0 loss) and <FEN> is a quiet-ish position from that game. The C++
tuner reads the float, then passes the rest of the line straight to
set_from_fen().

Why these filters (standard Texel practice):
  * label by final game result, not engine score;
  * skip the opening book plies (over-represented, theory-driven);
  * skip in-check positions (tactically loud — eval != qsearch there);
  * sample a few positions per game (decorrelate; avoid one long game
    dominating; reduce opening-cluster bias);
  * optional Elo floor (master games = cleaner labels).
The C++ tuner can additionally drop non-quiet positions via qsearch == eval.

Usage:
    python extract_fens.py GAMES.pgn [more.pgn ...] -o fens.txt \
        [--min-ply 16] [--sample-per-game 10] [--min-elo 2200] \
        [--min-game-plies 24] [--max-positions 2000000] [--seed 1]

ChessBase / Lichess: export to PGN first, then point this at the export.
"""
import argparse
import random
import sys

try:
    import chess
    import chess.pgn
except ImportError:
    sys.exit("python-chess is required:  python -m pip install python-chess")


def result_to_white_score(result: str):
    if result == "1-0":
        return 1.0
    if result == "0-1":
        return 0.0
    if result == "1/2-1/2":
        return 0.5
    return None  # "*" or malformed — skip the game


def main():
    ap = argparse.ArgumentParser(description="Extract labeled positions for Texel tuning")
    ap.add_argument("pgns", nargs="+", help="input PGN file(s)")
    ap.add_argument("-o", "--out", default="fens.txt", help="output file (default fens.txt)")
    ap.add_argument("--min-ply", type=int, default=16,
                    help="skip the first N plies (opening book); default 16 = 8 full moves")
    ap.add_argument("--sample-per-game", type=int, default=10,
                    help="max positions sampled per game (default 10)")
    ap.add_argument("--min-elo", type=int, default=0,
                    help="skip games where either player's Elo is below this (0 = no filter)")
    ap.add_argument("--min-game-plies", type=int, default=24,
                    help="skip games shorter than this many plies (resignation/abort noise)")
    ap.add_argument("--max-positions", type=int, default=0,
                    help="stop after writing this many positions (0 = unlimited)")
    ap.add_argument("--seed", type=int, default=1, help="RNG seed for per-game sampling")
    args = ap.parse_args()

    rng = random.Random(args.seed)
    written = 0
    games = 0
    skipped_result = skipped_elo = skipped_short = 0

    with open(args.out, "w", encoding="ascii") as out:
        for path in args.pgns:
            with open(path, encoding="utf-8", errors="ignore") as pgn:
                while True:
                    game = chess.pgn.read_game(pgn)
                    if game is None:
                        break
                    games += 1
                    if games % 5000 == 0:
                        print(f"  ...{games} games read, {written} positions written", file=sys.stderr)

                    score = result_to_white_score(game.headers.get("Result", "*"))
                    if score is None:
                        skipped_result += 1
                        continue

                    if args.min_elo > 0:
                        try:
                            we = int(game.headers.get("WhiteElo", "0"))
                            be = int(game.headers.get("BlackElo", "0"))
                        except ValueError:
                            we = be = 0
                        if we < args.min_elo or be < args.min_elo:
                            skipped_elo += 1
                            continue

                    # Collect quiet-ish candidate FENs from this game.
                    board = game.board()
                    candidates = []
                    ply = 0
                    for move in game.mainline_moves():
                        board.push(move)
                        ply += 1
                        if ply < args.min_ply:
                            continue
                        if board.is_check():           # loud: side to move is in check
                            continue
                        candidates.append(board.fen())

                    if ply < args.min_game_plies:
                        skipped_short += 1
                        continue

                    if not candidates:
                        continue
                    k = min(args.sample_per_game, len(candidates))
                    for fen in rng.sample(candidates, k):
                        out.write(f"{score:.1f} {fen}\n")
                        written += 1
                        if args.max_positions and written >= args.max_positions:
                            print(f"Reached --max-positions={args.max_positions}.", file=sys.stderr)
                            _summary(games, written, skipped_result, skipped_elo, skipped_short, args.out)
                            return

    _summary(games, written, skipped_result, skipped_elo, skipped_short, args.out)


def _summary(games, written, sr, se, ss, out):
    print(f"\nDone. {games} games scanned -> {written} positions -> {out}", file=sys.stderr)
    print(f"  skipped: result={sr}  elo={se}  short={ss}", file=sys.stderr)


if __name__ == "__main__":
    main()
