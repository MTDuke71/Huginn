"""Compare per-move search depth between the two engines in a fastchess PGN.

fastchess annotates each move with {eval/depth time, n=nodes, sd=seldepth}.
This pairs up the two sides by move number so we can see how much deeper (if
at all) one engine gets at the same point in the same game.

usage: python pgndepth.py <pgn>
"""
import re
import statistics
import sys
from collections import defaultdict

path = sys.argv[1]
text = open(path, encoding="utf-8", errors="replace").read()

games = [g for g in text.split("\n[Event ") if g.strip()]

# {eval/depth clock, n=..., sd=...}
COMMENT = re.compile(
    r"\{([^}]*?)([+-]?(?:\d+\.\d+|M\d+|-M\d+))/(\d+)\s+([\d.]+)s([^}]*)\}"
)
NODES = re.compile(r"n=(\d+)")
SELD = re.compile(r"sd=(\d+)")

by_engine = defaultdict(lambda: {"depth": [], "seldepth": [], "nodes": [], "time": []})
# paired by (game, ply-pair index)
paired = []

for g in games:
    white = re.search(r'\[White "([^"]+)"\]', g)
    black = re.search(r'\[Black "([^"]+)"\]', g)
    if not white or not black:
        continue
    w, b = white.group(1), black.group(1)

    body = g.split("\n\n", 1)[-1]
    # strip header remnants
    entries = []
    for m in COMMENT.finditer(body):
        pre, ev, depth, secs, post = m.groups()
        blob = pre + post
        n = NODES.search(blob)
        sd = SELD.search(blob)
        entries.append({
            "depth": int(depth),
            "seldepth": int(sd.group(1)) if sd else None,
            "nodes": int(n.group(1)) if n else None,
            "time": float(secs),
        })

    # entries alternate White, Black, White, ...
    for i, e in enumerate(entries):
        eng = w if i % 2 == 0 else b
        by_engine[eng]["depth"].append(e["depth"])
        by_engine[eng]["time"].append(e["time"])
        if e["seldepth"] is not None:
            by_engine[eng]["seldepth"].append(e["seldepth"])
        if e["nodes"] is not None:
            by_engine[eng]["nodes"].append(e["nodes"])

    # pair move k of White with move k of Black (same game, adjacent plies)
    for k in range(0, len(entries) - 1, 2):
        paired.append((w, entries[k], b, entries[k + 1]))

names = sorted(by_engine)
if not names:
    print("no annotated moves found — check the PGN comment format")
    sys.exit(1)

print(f"games parsed: {len(games)}   annotated moves: "
      f"{sum(len(v['depth']) for v in by_engine.values())}\n")

hdr = f"{'engine':<18}{'moves':>7}{'depth avg':>11}{'median':>8}{'min':>6}{'max':>6}"
hdr += f"{'seldepth':>10}{'Mnodes avg':>12}{'s/move':>8}"
print(hdr)
print("-" * len(hdr))
for name in names:
    d = by_engine[name]
    sd = d["seldepth"]
    nodes = d["nodes"]
    print(f"{name:<18}{len(d['depth']):>7}"
          f"{statistics.mean(d['depth']):>11.2f}"
          f"{statistics.median(d['depth']):>8.1f}"
          f"{min(d['depth']):>6}{max(d['depth']):>6}"
          f"{(statistics.mean(sd) if sd else 0):>10.2f}"
          f"{(statistics.mean(nodes) / 1e6 if nodes else 0):>12.2f}"
          f"{statistics.mean(d['time']):>8.2f}")

# Head-to-head at the same point in the same game.
if paired:
    a_name = paired[0][0]
    b_name = paired[0][2]
    diffs = [pa["depth"] - pb["depth"] for (na, pa, nb, pb) in paired if na == a_name]
    diffs += [pb["depth"] - pa["depth"] for (na, pa, nb, pb) in paired if na != a_name]
    # normalise: always "first engine alphabetically minus the other"
    first = names[0]
    rel = []
    for (na, pa, nb, pb) in paired:
        if na == first:
            rel.append(pa["depth"] - pb["depth"])
        else:
            rel.append(pb["depth"] - pa["depth"])

    print(f"\nHead-to-head, same game & same move number "
          f"({first} minus {names[1] if len(names) > 1 else '?'}):")
    print(f"  pairs           : {len(rel)}")
    print(f"  mean depth diff : {statistics.mean(rel):+.2f} ply")
    print(f"  median          : {statistics.median(rel):+.1f} ply")
    wins = sum(1 for x in rel if x > 0)
    ties = sum(1 for x in rel if x == 0)
    loss = sum(1 for x in rel if x < 0)
    print(f"  deeper / equal / shallower: {wins} / {ties} / {loss}"
          f"   ({100.0*wins/len(rel):.1f}% deeper)")

    hist = defaultdict(int)
    for x in rel:
        hist[x] += 1
    print("  distribution:", "  ".join(
        f"{k:+d}:{hist[k]}" for k in sorted(hist)))
