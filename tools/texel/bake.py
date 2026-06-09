#!/usr/bin/env python3
"""
Bake tuned eval params (#9) from a huginn_tuner output dump back into the
engine source, with zero manual transcription.

    python bake.py <tuner_output.txt>

Generic: it parses every `NAME = { ... };` (array) and `NAME = <int>;` (scalar)
in the "TUNED VALUES" block and rewrites the matching EVAL_PARAM definitions:
  - PIECE_VALUES_MG / PIECE_VALUES_EG  -> src/chess_types.hpp
  - everything else (PSTs, PASSED_PAWN_BONUS, mobility + positional scalars,
    ...) -> src/evaluation.hpp
Whatever the tuner dumps gets baked, so new tunable params need no bake.py edit.
64-element arrays are formatted as an 8x8 grid; smaller arrays inline.

Verify afterwards: rebuild + `huginn_tuner fens.txt --k <K> --max-sweeps 0`;
the reported start MSE must equal the tuner's converged MSE.
"""
import re
import sys
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.normpath(os.path.join(HERE, "..", "..", "src"))
MATERIAL = {"PIECE_VALUES_MG", "PIECE_VALUES_EG"}  # live in chess_types.hpp


def parse_dump(text):
    i = text.find("TUNED VALUES")
    if i >= 0:
        text = text[i:]
    arrays, scalars = {}, {}
    for m in re.finditer(r"\b([A-Z_][A-Z0-9_]*)\s*=\s*\{([^}]*)\}", text):
        arrays[m.group(1)] = [int(x) for x in re.findall(r"-?\d+", m.group(2))]
    for m in re.finditer(r"\b([A-Z_][A-Z0-9_]*)\s*=\s*(-?\d+)\s*;", text):
        scalars[m.group(1)] = int(m.group(2))
    return arrays, scalars


def fmt_array(vals):
    if len(vals) == 64:
        rows = ["    " + "".join(f"{vals[r*8+f]:4d}," for f in range(8)) for r in range(8)]
        return "{\n" + "\n".join(rows) + "}"
    return "{ " + ", ".join(str(v) for v in vals) + " }"


def replace_braces(text, name, vals):
    pat = re.compile(r"(\b" + re.escape(name) + r"\s*=\s*)\{[^}]*\}")
    new, n = pat.subn(lambda m: m.group(1) + fmt_array(vals), text)
    if n != 1:
        sys.exit(f"ERROR: array {name} matched {n} times (expected 1)")
    return new


def replace_scalar(text, name, val):
    pat = re.compile(r"(\b" + re.escape(name) + r"\s*=\s*)-?\d+")
    new, n = pat.subn(lambda m: m.group(1) + str(val), text)
    if n != 1:
        sys.exit(f"ERROR: scalar {name} matched {n} times (expected 1)")
    return new


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: python bake.py <tuner_output.txt>")
    arrays, scalars = parse_dump(open(sys.argv[1], encoding="utf-8", errors="ignore").read())
    if not arrays:
        sys.exit("ERROR: no arrays parsed — is this a tuner dump?")

    ct_path = os.path.join(SRC, "chess_types.hpp")
    ev_path = os.path.join(SRC, "evaluation.hpp")
    ct = open(ct_path, encoding="utf-8").read()
    ev = open(ev_path, encoding="utf-8").read()

    n_arr = n_sc = 0
    for name, vals in arrays.items():
        if name in MATERIAL:
            ct = replace_braces(ct, name, vals)
        else:
            ev = replace_braces(ev, name, vals)
        n_arr += 1
    for name, val in scalars.items():
        ev = replace_scalar(ev, name, val)
        n_sc += 1

    open(ct_path, "w", encoding="utf-8", newline="\n").write(ct)
    open(ev_path, "w", encoding="utf-8", newline="\n").write(ev)
    print(f"baked: {n_arr} arrays + {n_sc} scalars "
          f"({sum(1 for a in arrays if a in MATERIAL)} -> chess_types.hpp, rest -> evaluation.hpp)")


if __name__ == "__main__":
    main()
