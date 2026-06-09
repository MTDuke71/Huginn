#!/usr/bin/env python3
"""
Bake tuned eval tables (#9) from a huginn_tuner output dump back into the
engine source, with zero manual transcription.

    python bake.py <tuner_output.txt>

Parses the "TUNED VALUES" block and rewrites:
  - src/chess_types.hpp : PIECE_VALUES_MG / PIECE_VALUES_EG
  - src/evaluation.hpp  : the 12 PST tables + MOBILITY_WEIGHT_DEFAULT/ENDGAME

Verify the bake afterwards by rebuilding and running:
  huginn_tuner fens.txt --k <K> --max-sweeps 0
the reported start MSE must equal the tuner's converged MSE.
"""
import re
import sys
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.normpath(os.path.join(HERE, "..", "..", "src"))

TABLES_64 = [
    "PAWN_TABLE", "KNIGHT_TABLE", "BISHOP_TABLE", "ROOK_TABLE", "QUEEN_TABLE",
    "KING_TABLE", "KING_TABLE_ENDGAME",
    "PAWN_TABLE_EG", "KNIGHT_TABLE_EG", "BISHOP_TABLE_EG", "ROOK_TABLE_EG", "QUEEN_TABLE_EG",
]


def parse_dump(text):
    """name -> list[int] for every 'NAME = { ... }' in the TUNED VALUES block."""
    i = text.find("TUNED VALUES")
    if i >= 0:
        text = text[i:]
    out = {}
    for m in re.finditer(r"(\b[A-Z_][A-Z0-9_]*)\s*=\s*\{([^}]*)\}", text):
        name = m.group(1)
        nums = [int(x) for x in re.findall(r"-?\d+", m.group(2))]
        out[name] = nums
    mob = re.search(r"MOBILITY_WEIGHT_DEFAULT\s*=\s*(-?\d+);\s*MOBILITY_WEIGHT_ENDGAME\s*=\s*(-?\d+)", text)
    if mob:
        out["MOBILITY_WEIGHT_DEFAULT"] = [int(mob.group(1))]
        out["MOBILITY_WEIGHT_ENDGAME"] = [int(mob.group(2))]
    return out


def fmt64(vals):
    assert len(vals) == 64, f"expected 64 values, got {len(vals)}"
    rows = []
    for r in range(8):
        rows.append("    " + "".join(f"{vals[r*8+f]:4d}," for f in range(8)))
    return "{\n" + "\n".join(rows) + "};"


def replace_array64(text, name, vals):
    # RHS is either an existing { ... } body or an identifier (copy-init).
    pat = re.compile(r"(std::array<int, 64>\s+" + re.escape(name) +
                     r"\s*=\s*)(\{.*?\}|[A-Za-z_]\w*)(\s*;)", re.DOTALL)
    new, n = pat.subn(lambda m: m.group(1) + fmt64(vals) + m.group(3), text)
    if n != 1:
        sys.exit(f"ERROR: {name} matched {n} times (expected 1)")
    return new


def replace_material(text, name, vals):
    body = "{ " + ", ".join(str(v) for v in vals) + " }"
    pat = re.compile(r"(" + re.escape(name) + r"\s*=\s*)(\{[^}]*\})")
    new, n = pat.subn(lambda m: m.group(1) + body, text)
    if n != 1:
        sys.exit(f"ERROR: {name} matched {n} times (expected 1)")
    return new


def replace_scalar(text, name, val):
    pat = re.compile(r"(" + re.escape(name) + r"\s*=\s*)-?\d+")
    new, n = pat.subn(lambda m: m.group(1) + str(val), text)
    if n != 1:
        sys.exit(f"ERROR: {name} matched {n} times (expected 1)")
    return new


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: python bake.py <tuner_output.txt>")
    dump = parse_dump(open(sys.argv[1], encoding="utf-8", errors="ignore").read())

    for req in TABLES_64 + ["PIECE_VALUES_MG", "PIECE_VALUES_EG"]:
        if req not in dump:
            sys.exit(f"ERROR: {req} not found in dump")

    # chess_types.hpp : material
    ct_path = os.path.join(SRC, "chess_types.hpp")
    ct = open(ct_path, encoding="utf-8").read()
    ct = replace_material(ct, "PIECE_VALUES_MG", dump["PIECE_VALUES_MG"])
    ct = replace_material(ct, "PIECE_VALUES_EG", dump["PIECE_VALUES_EG"])
    open(ct_path, "w", encoding="utf-8", newline="\n").write(ct)

    # evaluation.hpp : 12 tables + mobility
    ev_path = os.path.join(SRC, "evaluation.hpp")
    ev = open(ev_path, encoding="utf-8").read()
    for name in TABLES_64:
        ev = replace_array64(ev, name, dump[name])
    for s in ("MOBILITY_WEIGHT_DEFAULT", "MOBILITY_WEIGHT_ENDGAME"):
        if s in dump:
            ev = replace_scalar(ev, s, dump[s][0])
    open(ev_path, "w", encoding="utf-8", newline="\n").write(ev)

    print(f"baked: PIECE_VALUES_MG/EG -> chess_types.hpp; "
          f"{len(TABLES_64)} tables + mobility -> evaluation.hpp")


if __name__ == "__main__":
    main()
