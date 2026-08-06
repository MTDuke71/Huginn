"""BACKLOG #67: mine a fastchess PGN for eval discontinuities ("step functions").

Locates the plies where a game goes from near-equal to decided, and reports
which side was blind, by how much, and for how many plies. Output is a triage
shortlist for tools/analyze_played_vs_engines.py (the Stockfish oracle) — an
engine's own eval is NOT ground truth, so nothing here is a verdict.

usage: python pgn_swing_mine.py <pgn> [swing-threshold-pawns]

Conventions
-----------
fastchess annotates each move `{eval/depth time, n=..., sd=..., nps=...}` with
the eval from the MOVER's point of view. Everything below is normalized to
White's POV (`wp`) so the two sides are directly comparable:

    wp[i] = eval[i] if i is White's ply else -eval[i]

A swing at ply i is `wp[i] - wp[i-1]`. The side that moved at ply i-1 is the
"overturned" side: its assessment is the one the next ply contradicts.

Mate and tablebase scores are CLAMPED before differencing. `+M7` and the
ply-independent `+280.00` TB verdict (#66) would otherwise manufacture huge
fake swings and swamp every statistic.
"""
import re
import statistics
import sys
from collections import defaultdict

path = sys.argv[1]
THRESHOLD = float(sys.argv[2]) if len(sys.argv) > 2 else 0.75
CLAMP = 10.0        # pawns; mate/TB scores are pinned here, not differenced raw
DEPTH_CAP = 100     # FableR's pre-fix mate-spin artifact (see #67 / their fix)

text = open(path, encoding="utf-8", errors="replace").read()
games = [g for g in text.split("\n[Event ") if g.strip()]

COMMENT = re.compile(
    r"\{([^}]*?)([+-]?(?:\d+\.\d+|M\d+|-M\d+))/(\d+)\s+([\d.]+)s([^}]*)\}"
)
SELD = re.compile(r"sd=(\d+)")


def parse_eval(tok):
    """-> (clamped_pawns, is_terminal). Terminal = mate or TB verdict."""
    if "M" in tok:
        return (CLAMP if not tok.startswith("-") else -CLAMP), True
    v = float(tok)
    if abs(v) >= 100.0:          # 280.00 = TB win/loss verdict (#66)
        return (CLAMP if v > 0 else -CLAMP), True
    return max(-CLAMP, min(CLAMP, v)), abs(v) > CLAMP


swings = []                       # one record per discontinuity
per_engine_moves = defaultdict(int)
per_engine_overturned = defaultdict(int)
games_used = 0

for gi, g in enumerate(games):
    w = re.search(r'\[White "([^"]+)"\]', g)
    b = re.search(r'\[Black "([^"]+)"\]', g)
    res = re.search(r'\[Result "([^"]+)"\]', g)
    if not w or not b:
        continue
    w, b, res = w.group(1), b.group(1), (res.group(1) if res else "*")
    body = g.split("\n\n", 1)[-1]

    plies = []
    for m in COMMENT.finditer(body):
        pre, ev, depth, secs, post = m.groups()
        val, terminal = parse_eval(ev)
        sd = SELD.search(pre + post)
        plies.append({
            "eval": val, "terminal": terminal,
            "depth": int(depth), "seldepth": int(sd.group(1)) if sd else None,
        })
    if len(plies) < 4:
        continue
    games_used += 1

    # normalize to White's POV
    for i, p in enumerate(plies):
        p["wp"] = p["eval"] if i % 2 == 0 else -p["eval"]
        p["engine"] = w if i % 2 == 0 else b
        per_engine_moves[p["engine"]] += 1

    for i in range(1, len(plies)):
        cur, prev = plies[i], plies[i - 1]
        if cur["depth"] >= DEPTH_CAP or prev["depth"] >= DEPTH_CAP:
            continue
        delta = cur["wp"] - prev["wp"]
        if abs(delta) < THRESHOLD:
            continue
        # Only count a swing once it leaves the near-equal band; a shift from
        # +3 to +4 is not a game turning, it is an already-won game widening.
        if abs(prev["wp"]) >= 1.5:
            continue

        overturned = prev["engine"]          # its assessment was contradicted

        # Collapse EPISODES. A sustained disagreement (one side stuck at -8.00
        # while the other insists on -0.25) re-triggers the threshold on every
        # ply of that side, so a single blindness gets counted once per move
        # and whichever arm sits in longer episodes looks worse. Merge same
        # game + same overturned side + same sign within 4 plies into one.
        if swings:
            last = swings[-1]
            if (last["game"] == gi + 1 and last["overturned"] == overturned
                    and (last["delta"] > 0) == (delta > 0)
                    and i - last["ply"] <= 4):
                last["episode_plies"] += 1
                if abs(delta) > abs(last["delta"]):
                    last["delta"] = delta
                continue

        per_engine_overturned[overturned] += 1

        # Lag: plies until the overturned side's OWN eval moves at least
        # halfway toward the new assessment.
        lag = None
        for j in range(i + 1, min(i + 12, len(plies))):
            if plies[j]["engine"] != overturned:
                continue
            if (plies[j]["wp"] - prev["wp"]) * (1 if delta > 0 else -1) >= abs(delta) / 2:
                lag = j - i
                break

        swings.append({
            "game": gi + 1, "ply": i, "move_no": i // 2 + 1,
            "delta": delta, "overturned": overturned, "landed_by": cur["engine"],
            "depth_landed": cur["depth"], "depth_overturned": prev["depth"],
            "depth_gap": cur["depth"] - prev["depth"],
            "lag": lag, "terminal": cur["terminal"], "result": res,
            "episode_plies": 1,
        })

print(f"games: {games_used}   swings (|delta| >= {THRESHOLD}, from |eval| < 1.5): "
      f"{len(swings)}")
if not swings:
    sys.exit(0)

print("\n--- who gets overturned (lower is better) ---")
hdr = f"{'engine':<24}{'moves':>8}{'overturned':>12}{'per 100 moves':>15}"
print(hdr); print("-" * len(hdr))
for eng in sorted(per_engine_moves):
    mv, ov = per_engine_moves[eng], per_engine_overturned[eng]
    print(f"{eng:<24}{mv:>8}{ov:>12}{100.0*ov/mv:>15.2f}")

print("\n--- depth at the moment of the swing ---")
gaps = [s["depth_gap"] for s in swings]
print(f"  mean depth of side LANDING it     : "
      f"{statistics.mean([s['depth_landed'] for s in swings]):.2f}")
print(f"  mean depth of side OVERTURNED     : "
      f"{statistics.mean([s['depth_overturned'] for s in swings]):.2f}")
print(f"  mean gap (landed - overturned)    : {statistics.mean(gaps):+.2f} ply")
deeper = sum(1 for x in gaps if x > 0)
print(f"  landed by the DEEPER side         : {deeper}/{len(gaps)} "
      f"({100.0*deeper/len(gaps):.1f}%)")

lags = [s["lag"] for s in swings if s["lag"] is not None]
if lags:
    print(f"\n--- blindness lag (plies until the overturned side agrees) ---")
    print(f"  resolved: {len(lags)}/{len(swings)}   mean {statistics.mean(lags):.2f}"
          f"   median {statistics.median(lags):.1f}   max {max(lags)}")
    print(f"  never caught up within 12 plies: {len(swings)-len(lags)}")

print("\n--- swing rate by move number ---")
buckets = defaultdict(int)
for s in swings:
    buckets[min(s["move_no"] // 10 * 10, 60)] += 1
peak = max(buckets.values())
for k in sorted(buckets):
    label = f"{k}-{k+9}" if k < 60 else "60+"
    bar = "#" * max(1, round(50 * buckets[k] / peak))   # scaled to the peak, not clipped
    print(f"  moves {label:<7} {buckets[k]:>5}  {bar}")


def shortlist(rows, title):
    print(f"\n--- {title} ---")
    hdr = (f"{'game':>6}{'move':>6}{'delta':>9}{'d(land)':>9}{'d(over)':>9}"
           f"{'lag':>5}  overturned")
    print(hdr); print("-" * len(hdr))
    for s in rows:
        lag = s["lag"] if s["lag"] is not None else ">12"
        print(f"{s['game']:>6}{s['move_no']:>6}{s['delta']:>+9.2f}"
              f"{s['depth_landed']:>9}{s['depth_overturned']:>9}{str(lag):>5}"
              f"  {s['overturned']}")


by_mag = sorted(swings, key=lambda x: -abs(x["delta"]))
shortlist(by_mag[:10], "top 10 discontinuities (all)")

# The oracle-triage list. Terminal swings (into mate/TB) are pinned at the
# clamp and just mark "the game ended here" — they crowd out the quiet
# positional blunders, which are the ones worth an SF verdict. Long lag =
# the overturned side stayed blind, the #52 signature.
quiet = [s for s in by_mag if not s["terminal"] and abs(s["delta"]) < CLAMP - 0.01]
quiet_blind = sorted(quiet, key=lambda x: (x["lag"] is not None, -abs(x["delta"])))
shortlist(quiet_blind[:15], "top 15 QUIET swings, longest-blindness first (SF triage)")
