"""#45 move-level-futility complexity-gate diagnostic.

Compares the flag-OFF (node-level futility, t19) and flag-ON (move-level)
builds on a tactical EPD suite at fixed depth:
  - solves: engine UCI move converted to SAN, matched against the bm set
  - nodes : max cumulative `nodes` reported (search cost at fixed depth)
Reports per-arm solve count + total nodes, and lists solve-status flips.

Usage: python mlf_diag.py <epd> <depth>
"""
import re, subprocess, sys, time
from pathlib import Path
import chess

REPO = Path(r"C:\Users\m_lad\Repos\Huginn")
ARMS = [
    ("OFF(node)", REPO / "build/msvc-x64-release/bin/Release/huginn.exe"),
    ("ON(move)",  REPO / "build/msvc-mlf/bin/Release/huginn.exe"),
]

def parse_epd(line):
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    head = line.split(";", 1)[0].strip()
    m = re.search(r"\bbm\s+([^;]+)", head)
    if not m:
        return None
    bms = m.group(1).strip().split()
    fen = head[:m.start()].strip()
    pid = (re.search(r'id\s+"([^"]+)"', line) or [None, "?"])[1] if 'id' in line else "?"
    idm = re.search(r'id\s+"([^"]+)"', line)
    pid = idm.group(1) if idm else "?"
    return fen, bms, pid

def run(exe, fen, depth):
    parts = fen.split()
    if len(parts) == 4:
        fen = fen + " 0 1"
    cmd = (f"uci\nsetoption name OwnBook value false\n"
           f"position fen {fen}\ngo depth {depth}\nquit\n")
    r = subprocess.run([str(exe)], input=cmd, capture_output=True, text=True, timeout=120)
    bm = re.search(r"^bestmove\s+(\S+)", r.stdout, re.MULTILINE)
    nodes = 0
    for m in re.finditer(r"\bnodes\s+(\d+)", r.stdout):
        nodes = max(nodes, int(m.group(1)))
    return (bm.group(1) if bm else "?"), nodes

def solved(fen, uci, bms):
    parts = fen.split()
    if len(parts) == 4:
        fen = fen + " 0 1"
    b = chess.Board(fen)
    try:
        mv = chess.Move.from_uci(uci)
        san = b.san(mv).rstrip("+#!?")
    except Exception:
        return False, "?"
    norm = {x.rstrip("+#!?") for x in bms}
    return san in norm, san

def main():
    epd = Path(sys.argv[1]) if len(sys.argv) > 1 else REPO / "test/WAC300.epd"
    depth = int(sys.argv[2]) if len(sys.argv) > 2 else 10
    pos = [p for p in (parse_epd(l) for l in open(epd)) if p]
    print(f"# {epd.name}  depth={depth}  {len(pos)} positions")
    counts = {a[0]: 0 for a in ARMS}
    nodesum = {a[0]: 0 for a in ARMS}
    flips = []
    t0 = time.monotonic()
    for fen, bms, pid in pos:
        res = {}
        for name, exe in ARMS:
            uci, nodes = run(exe, fen, depth)
            ok, san = solved(fen, uci, bms)
            counts[name] += ok
            nodesum[name] += nodes
            res[name] = (ok, san, nodes)
        a, b = ARMS[0][0], ARMS[1][0]
        if res[a][0] != res[b][0]:
            flips.append((pid, " ".join(bms), res[a], res[b]))
    dt = time.monotonic() - t0
    print(f"# elapsed {dt:.0f}s\n")
    for name, _ in ARMS:
        print(f"  {name:<10} solved {counts[name]:>3}/{len(pos)}   nodes {nodesum[name]:,}")
    da, db = ARMS[0][0], ARMS[1][0]
    dn = nodesum[db] - nodesum[da]
    print(f"\n  node delta ON-OFF: {dn:+,} ({100*dn/max(1,nodesum[da]):+.1f}%)")
    print(f"  solve delta ON-OFF: {counts[db]-counts[da]:+d}")
    if flips:
        print(f"\n  solve-status flips ({len(flips)}):")
        print(f"  {'ID':<12} {'bm':<10} {da:<22} {db:<22}")
        for pid, bm, ra, rb in flips:
            sa = ('OK ' if ra[0] else 'MISS') + f" {ra[1]}"
            sb = ('OK ' if rb[0] else 'MISS') + f" {rb[1]}"
            print(f"  {pid:<12} {bm:<10} {sa:<22} {sb:<22}")

if __name__ == "__main__":
    main()
