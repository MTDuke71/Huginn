"""Mine gauntlet PGNs for winning-side 3-fold repetition draws (BACKLOG #28).

Steps 1-4 of the BACKLOG #28 plan:
  1. Parse the gauntlet PGNs.
  2. Extract every game ending in "Draw by 3-fold repetition".
  3. Record round, side/engine that clinched the repetition, final
     SAN/UCI, final eval/depth/comment, result, full move list.
  4. Reconstruct the position a few plies before the final repetition and
     emit FEN + legal non-repeating alternatives.

Output:
  - A human-readable report to stdout.
  - tools/repetition_candidates.json  (full structured dump)
  - tools/repetition_candidates.epd   (FENs where Huginn_current clinched
    a repetition while its own reported eval was clearly winning, for the
    step-5 re-search and the eventual regression set)

Usage:
    python tools/repetition_analysis.py [pgn ...]

Defaults to the two t5 gauntlet PGNs named in the BACKLOG entry.
"""
import io
import json
import re
import sys
from pathlib import Path

import chess
import chess.pgn

REPO = Path(__file__).resolve().parent.parent
DEFAULT_PGNS = [
    REPO / "gauntlet" / "huginn_vs_t5_intel.pgn",
    REPO / "gauntlet" / "huginn_vs_t5_amd.pgn",
]

# "Winning while repeating" threshold, in centipawns, from the clincher's
# OWN reported eval (positive = good for the side that just moved). Matches
# the shipped root-demotion band (#27/baseline-t6 used +300cp).
WIN_CP = 300

# python-chess strips the surrounding {} so comments arrive as:
#   <eval>/<depth> <time>s[, <reason>]
#   eval is +1.23 / -0.45 / 0.00  or  +M9 / -M10  (mate)
EVAL_RE = re.compile(
    r"([+-]?M?\d+(?:\.\d+)?)\s*/\s*(\d+)\s+([\d.]+)s\s*(?:,\s*(.*?))?\s*$"
)


def parse_eval(token: str):
    """Return (cp, is_mate, mate_in) from the engine's eval token.

    cp is from the moving side's perspective. Mate scores are flagged so
    callers can treat them as artifacts rather than real static evals.
    """
    if token is None:
        return None, False, None
    token = token.strip()
    m = re.match(r"([+-]?)M(\d+)", token)
    if m:
        sign = -1 if m.group(1) == "-" else 1
        return sign * 100000, True, sign * int(m.group(2))
    try:
        return int(round(float(token) * 100)), False, None
    except ValueError:
        return None, False, None


def last_move_comment(game):
    """Walk to the final move; return (node, comment) for the last ply."""
    node = game
    while node.variations:
        node = node.variations[0]
    return node, node.comment or ""


def engine_for_side(headers, color: chess.Color) -> str:
    return headers["White"] if color == chess.WHITE else headers["Black"]


def analyze_game(game, source: str):
    headers = game.headers
    result = headers.get("Result", "*")
    last_node, last_comment = last_move_comment(game)

    if "3-fold repetition" not in last_comment:
        return None
    if result != "1/2-1/2":
        return None

    # The clinching move is the last move played; the side that played it
    # is the OPPOSITE of the board's side-to-move at the final node.
    final_board = last_node.board()
    clincher_color = not final_board.turn
    clincher_engine = engine_for_side(headers, clincher_color)

    m = EVAL_RE.search(last_comment)
    eval_token = m.group(1) if m else None
    depth = int(m.group(2)) if m else None
    cp, is_mate, mate_in = parse_eval(eval_token)

    # Full move list + the board two plies before the end (the position
    # from which the clincher *chose* to repeat, with its alternatives).
    nodes = []
    n = game
    while n.variations:
        n = n.variations[0]
        nodes.append(n)

    clincher_move = nodes[-1].move
    pre_node = nodes[-2] if len(nodes) >= 2 else game
    pre_board = pre_node.board()  # position the clincher moved FROM
    pre_fen = pre_board.fen()

    # Move history to REACH pre_board, so a re-search can replay it via
    # `position fen <start_fen> moves ...` and arrive with the same
    # repetition stack the engine had in-game (t6 root-demotion needs it).
    start_fen = headers.get("FEN", chess.STARTING_FEN)
    moves_uci = [nd.move.uci() for nd in nodes[: len(nodes) - 1]]

    # Legal alternatives that do NOT immediately re-enter a position
    # already seen in the game history up to pre_board.
    history_keys = set()
    walk = chess.Board(headers.get("FEN", chess.STARTING_FEN))
    history_keys.add(walk._transposition_key())
    for nd in nodes[: len(nodes) - 1]:
        walk.push(nd.move)
        history_keys.add(walk._transposition_key())

    alternatives = []
    for mv in pre_board.legal_moves:
        if mv == clincher_move:
            continue
        pre_board.push(mv)
        repeats = pre_board._transposition_key() in history_keys
        pre_board.pop()
        alternatives.append({"uci": mv.uci(),
                             "san": pre_board.san(mv),
                             "immediate_repeat": repeats})
    non_repeating = [a for a in alternatives if not a["immediate_repeat"]]

    huginn_clinched = clincher_engine == "Huginn_current"
    # "Real" winning claim: Huginn_current clinched, its own non-mate eval
    # is >= +WIN_CP for itself, and a non-repeating legal move existed.
    winning_artifact = is_mate or (cp is not None and abs(cp) >= 50000)
    is_candidate = (
        huginn_clinched
        and cp is not None
        and not is_mate
        and cp >= WIN_CP
        and len(non_repeating) > 0
    )

    return {
        "source": source,
        "round": headers.get("Round", "?"),
        "white": headers.get("White"),
        "black": headers.get("Black"),
        "result": result,
        "clincher_engine": clincher_engine,
        "clincher_color": "white" if clincher_color == chess.WHITE else "black",
        "clincher_move_uci": clincher_move.uci(),
        "clincher_eval_token": eval_token,
        "clincher_cp": cp,
        "clincher_depth": depth,
        "is_mate_score": is_mate,
        "mate_in": mate_in,
        "winning_score_is_artifact": winning_artifact,
        "pre_fen": pre_fen,
        "start_fen": start_fen,
        "moves_uci": moves_uci,
        "n_legal": pre_board.legal_moves.count(),
        "n_non_repeating_alternatives": len(non_repeating),
        "non_repeating_alternatives": [a["uci"] for a in non_repeating[:8]],
        "huginn_current_clinched": huginn_clinched,
        "is_winning_repeat_candidate": is_candidate,
        "comment": last_comment.strip(),
    }


def main(argv):
    pgns = [Path(p) for p in argv[1:]] or DEFAULT_PGNS
    rows = []
    for pgn_path in pgns:
        if not pgn_path.exists():
            print(f"!! missing: {pgn_path}", file=sys.stderr)
            continue
        text = pgn_path.read_text(encoding="utf-8", errors="replace")
        stream = io.StringIO(text)
        while True:
            game = chess.pgn.read_game(stream)
            if game is None:
                break
            row = analyze_game(game, pgn_path.name)
            if row is not None:
                rows.append(row)

    total = len(rows)
    huginn = [r for r in rows if r["huginn_current_clinched"]]
    candidates = [r for r in rows if r["is_winning_repeat_candidate"]]
    artifacts = [r for r in huginn if r["winning_score_is_artifact"]]

    print(f"3-fold repetition draws found:        {total}")
    print(f"  ...where Huginn_current clinched:   {len(huginn)}")
    print(f"  ...mate/huge-score artifacts:       {len(artifacts)}")
    print(f"  ...WINNING-REPEAT CANDIDATES (>= +{WIN_CP}cp, non-rep alt"
          f" exists): {len(candidates)}")
    print()
    print("=== Winning-repeat candidates (step-5 re-search targets) ===")
    for r in sorted(candidates, key=lambda x: -x["clincher_cp"]):
        print(f"[{r['source']} R{r['round']}] {r['white']} vs {r['black']}"
              f"  clincher={r['clincher_color']} {r['clincher_move_uci']}"
              f"  eval={r['clincher_eval_token']}/d{r['clincher_depth']}"
              f"  non-rep alts={r['n_non_repeating_alternatives']}")
        print(f"    FEN: {r['pre_fen']}")
        print(f"    alts: {', '.join(r['non_repeating_alternatives'])}")

    out_json = REPO / "tools" / "repetition_candidates.json"
    out_json.write_text(json.dumps(rows, indent=2), encoding="utf-8")

    out_epd = REPO / "tools" / "repetition_candidates.epd"
    with out_epd.open("w", encoding="utf-8") as fh:
        for r in candidates:
            fh.write(f"{r['pre_fen']} ; "
                     f"id \"{r['source']}-R{r['round']}\"; "
                     f"clincher \"{r['clincher_move_uci']}\"; "
                     f"reported_cp {r['clincher_cp']};\n")

    print()
    print(f"wrote {out_json.relative_to(REPO)} ({total} rows)")
    print(f"wrote {out_epd.relative_to(REPO)} ({len(candidates)} candidates)")


if __name__ == "__main__":
    main(sys.argv)
