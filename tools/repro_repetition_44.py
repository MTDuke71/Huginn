#!/usr/bin/env python3
"""Deterministic repro for BACKLOG #44 — won games drawn by 3-fold repetition.

Root cause: `repetition_count_in_history` used `move_history.size()` as the
history length, but move_history is a high-water-mark buffer (MakeMove
resize-grows it, TakeMove never shrinks it), so during deep search its size
exceeds the current path length `pos.ply`. The scan window slid off the real
predecessors, so a true 3-fold read as a non-repetition at the deepest
iteration (the one that picks the move) and the engine repeated into a draw in
a winning position. Fix: use `pos.ply` as the history length.

The bug only fires with a WARM transposition table (a stale winning score for
the position that has since become a 3-fold). Cold (single search) the engine
finds the win; replaying the moves in ONE process (as a GUI does) reproduces
the draw on the buggy build.

Usage:
    python tools/repro_repetition_44.py            # print the move prefixes
then feed a UCI engine, in ONE process (warm TT):
    position fen <START> moves <prefix_move58>
    go depth 12
    position fen <START> moves <prefix_move60>
    go depth 12
    position fen <START> moves <prefix_move62>
    go depth 12
Buggy build plays f5g6 (Kg6) -> draw; fixed build plays h6h7 -> wins.

Source game: Stash fixed-depth RR, 2026-06-16, Huginn_nmpver (White) vs
Huginn_t16, drawn by 3-fold from a +6.8 won position.
"""

import chess

START = "rnbqkbnr/ppp2p1p/6p1/3pp3/P7/4P2P/1PPP1PP1/RNBQKBNR w KQkq - 0 4"
SAN = (
    "d4 Nc6 dxe5 Nxe5 Bd2 Bg7 Bc3 c5 Nf3 Nxf3+ Qxf3 Kf8 a5 Bf5 Bd3 d4 Bd2 dxe3 "
    "Bxe3 Bxd3 cxd3 Qe7 O-O Bxb2 Ra2 Be5 Re1 Kg7 Bf4 f6 Rb2 Rb8 Nc3 Qc7 Bxe5 "
    "fxe5 Reb1 b6 Nd5 Qf7 Qxf7+ Kxf7 axb6 axb6 Rxb6 Rxb6 Rxb6 Nh6 Nf6 Nf5 Nd7 "
    "Re8 Nxc5 h6 g3 Rd8 Rb7+ Kf6 Kf1 Rc8 Ne4+ Ke6 Rb6+ Kf7 Rf6+ Kg7 Re6 Rc1+ "
    "Kg2 Rd1 Nc5 Re1 Nd7 h5 Rxe5 Rxe5 Nxe5 Kf6 f4 g5 Kf3 gxf4 Nd7+ Ke6 Kxf4 "
    "Nd4 Nc5+ Kd5 Ne4 Ne6+ Kf5 Nd4+ Kg6 Nc6 Nf6+ Kd4 Nxh5 Kxd3 h4 Ke3 Nf6 "
    "Ne7+ Kg5 Nc8 h5 Nd6 h6 Nf7+ Kg6 Ne5+ Kf5 Nf7 Kg6 Ne5+ Kf5 Nf7"
).split()

# White-to-move repetition points: before 58.Kg6 (108 plies), 60.Kg6 (112),
# 62.Kg6 (116). The position before 60.Kg6 and 62.Kg6 is identical (a 3-fold of
# the after-Kg6 position).
MARKS = {108: "move58", 112: "move60", 116: "move62"}


def main():
    board = chess.Board(START)
    uci = []
    print(f"START_FEN = {START}\n")
    for mv in SAN:
        move = board.parse_san(mv)
        uci.append(move.uci())
        board.push(move)
        if len(uci) in MARKS:
            print(f"# {MARKS[len(uci)]} (before White's Kg6) — {len(uci)} plies")
            print("moves " + " ".join(uci) + "\n")


if __name__ == "__main__":
    main()
