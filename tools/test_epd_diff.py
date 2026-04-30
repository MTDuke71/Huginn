"""Compare two UCI engines on an EPD test suite at fixed depth.

Surfaces positions where the engines pick different moves. Does NOT
auto-score against the EPD `bm` field (SAN-to-UCI conversion would
need a chess library) — for divergence positions, inspect the FEN +
both UCI moves manually to see which engine matches `bm`.

Usage:
    python tools/test_epd_diff.py [epd_file] [depth]

Defaults: epd_file=test/lct2.epd, depth=9
"""
import re
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
# Default comparison: current build (head of bisect/work) vs the most-recent
# frozen baseline (huginn_t2 as of 2026-04-30). Edit either entry to compare
# any pair of binaries.
ENGINES = [
    ("t2",      r"C:\Users\m_lad\Documents\fastchess-windows-x86-64\huginn_t2.exe"),
    ("current", str(REPO / "build" / "msvc-x64-release" / "bin" / "Release" / "huginn.exe")),
]


def parse_epd_line(line: str):
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    head = line.split(";", 1)[0].strip()
    m = re.search(r"\bbm\s+([^;]+)", head)
    if not m:
        return None
    bm = m.group(1).strip().rstrip("+#!?")
    fen = head[: m.start()].strip()
    id_match = re.search(r'\bid\s+"([^"]+)"', line)
    pid = id_match.group(1) if id_match else "???"
    return fen, bm, pid


def run_engine(exe: str, fen: str, depth: int) -> tuple[str, int, int]:
    """Returns (uci_bestmove, last_depth_reached, time_ms)."""
    # EPD strips halfmove/fullmove counters; Huginn requires the full 6-field
    # FEN. Append zeros if missing.
    parts = fen.split()
    if len(parts) == 4:
        fen = fen + " 0 1"
    cmd = (
        "uci\n"
        "setoption name OwnBook value false\n"
        f"position fen {fen}\n"
        f"go depth {depth}\n"
        "quit\n"
    )
    t0 = time.monotonic()
    result = subprocess.run(
        [exe], input=cmd, capture_output=True, text=True, timeout=60
    )
    elapsed = int((time.monotonic() - t0) * 1000)
    bm = re.search(r"^bestmove\s+(\S+)", result.stdout, re.MULTILINE)
    last_depth = 0
    for m in re.finditer(r"^info depth (\d+)", result.stdout, re.MULTILINE):
        last_depth = max(last_depth, int(m.group(1)))
    return (bm.group(1) if bm else "?"), last_depth, elapsed


def main():
    epd_path = Path(sys.argv[1]) if len(sys.argv) > 1 else REPO / "test" / "lct2.epd"
    depth = int(sys.argv[2]) if len(sys.argv) > 2 else 9

    positions = []
    with open(epd_path) as f:
        for line in f:
            p = parse_epd_line(line)
            if p:
                positions.append(p)

    name1, exe1 = ENGINES[0]
    name2, exe2 = ENGINES[1]
    print(f"# Comparing {name1} vs {name2} on {epd_path.name} at depth {depth}")
    print(f"# {len(positions)} positions, ~{len(positions) * 2 * 2}s estimated")
    print()
    print(f"{'ID':<16} {'bm(SAN)':<10} {name1:<10} {name2:<10} VERDICT")
    print("-" * 70)

    differs = 0
    for fen, bm, pid in positions:
        m1, d1, t1ms = run_engine(exe1, fen, depth)
        m2, d2, t2ms = run_engine(exe2, fen, depth)
        verdict = "SAME" if m1 == m2 else "DIFFER"
        if verdict == "DIFFER":
            differs += 1
        print(f"{pid:<16} {bm:<10} {m1:<10} {m2:<10} {verdict}  "
              f"(t={t1ms}/{t2ms}ms d={d1}/{d2})")

    print()
    print(f"# {differs}/{len(positions)} positions differ between {name1} and {name2}")


if __name__ == "__main__":
    main()
