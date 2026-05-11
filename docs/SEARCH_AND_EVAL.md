# Search & Evaluation — current state and roadmap

Snapshot of what's implemented in Huginn 2.0 (post Phase 4) and what's
planned. Reality check on each item is anchored to file:line so this
stays accurate as the code evolves.

> **Looking for what to work on next?** See [BACKLOG.md](BACKLOG.md) —
> the single-file issue tracker has the live priority queue, blocked
> items with their unblock conditions, and known bugs. This document
> is the long-term roadmap and historical context; the backlog is
> the action list.

## Strength gap diagnosis (2026-04-28, updated 2026-05-11)

> **Status update 2026-05-11:** the bullet points below identified four
> search-shape gaps vs MTLChess. **Three have been addressed**: tuned
> 64×64 LMR table (shipped), SEE qsearch pruning (shipped, +38 Elo),
> aspiration step (a) root-PVS (shipped, +13.9 Elo). Step (b)
> aspiration window is still deferred (BACKLOG #8); adaptive null-move
> R is still flat R=4. Subsequent search work has been search-quality
> improvements (TT-mate +104, TB +16, contempt +40, P1a +22) rather
> than the original tree-shape gaps. **Cumulative ~+180-220 Elo over
> `baseline-t1`** per the Tier 1 progress table. The current
> calibration (2026-05-09 vs rebuilt `mtlchessV3.exe`) is much further
> from "0/20" but still well behind — see External rating ladder.

Calibration gauntlet: **huginn current 0-20 vs MTLChess_v0.3** (~1984 Elo)
at tc=10+0.1. That's a >400 Elo gap, putting huginn current somewhere
around **~1500-1700 Elo at this TC**.

NPS bench, both engines depth 11 from startpos, single thread:

| Engine | Nodes to depth 11 | Time | NPS |
|---|---|---|---|
| Huginn current | 17,777,636 | 8.6 s | ~2.06 Mnps |
| MTLChess_v0.3 | 904,334 | 0.39 s | ~2.33 Mnps |

**Raw NPS is at parity. The entire ~22× time-to-depth gap is search-tree
shape**, not node speed. MTLChess prunes ~20× more aggressively per
depth at the same NPS.

(CLAUDE.md's stale "~220 knps" claim is off by ~10×; the current engine
is ~2 Mnps.)

What MTLChess has that we don't / have weaker:

- **SEE-based capture ordering and qsearch pruning** (skip captures with
  `SEE < 0` in qsearch entirely). This alone shrinks the qsearch tree
  5-10× and is the single biggest tree-shape win available to us. It's
  currently Tier 2 #10 in this doc — **promoted to Tier 1 next-up**.
- **Working aspiration windows + clean PVS** — first move full window,
  subsequent moves null-window, re-search on `score > alpha`, then full
  window again if `alpha < score < beta`. Our attempt at this is broken
  (see Tier 1 progress).
- **Tuned 64×64 LMR table** computed at compile-time. Our LMR is the
  simple "depth ≥ 3, move ≥ 4, R = 1" rule.
- **Adaptive null-move R** (R=3 at depth ≥ 6, else R=2). We use a flat
  R=4 — more aggressive but possibly less accurate.

Conclusion: depth deficit is the dominant factor. Eval improvements
(king safety, mobility) help a depth-deficient engine less than search
improvements do. **Search-shape work (fix aspiration, fix LMP, add SEE,
tune LMR) is the priority**; eval is secondary until the tree is right.

## WAC300 tactical-solving snapshot (2026-04-30)

| Engine | Solved | Failed | Rate | Search time |
|---|---|---|---|---|
| MTLChess_v0.5 (~2314 Elo) | 288 | 12 | 96.0% | 5s/pos |
| Huginn current (post-mobility) | 237 | 63 | 79.0% | 5s/pos |
| Huginn 1.2 (2025-09-11, pre-pure-bitboard) | 231 | 69 | 77.0% | 5s/pos |

Logs: `test/wac_test_log_20260430_231351.txt` (MTL),
`test/wac_test_log_20260430_225140.txt` (huginn current),
`test/wac_test_log_20250911_181535.txt` (huginn 1.2).

**Of huginn's 63 failures, 52 are positions MTL solves** — pure
tactical sight gap. Only 1 position is huginn-only. The 11 "both
fail" are the genuinely hard tactical positions; 5 of those are
knight tactics (Nxa7+, Nxd5, Nb4, Nf5, Nfg5).

Pattern across both the regressions-vs-old-huginn AND the gap-vs-MTL
sets: **check-moves dominate**. That's the diagnostic that drives
"Priority 1" in the Next session plan above.

### Depth and eval comparison — first 10 WAC positions

5s/position search; columns are max depth reached and the score reported
at that depth. `M+N` = mate-in-N from huginn (positive = mating); raw
values near `±29000` from MTL are its mate-found encoding (`MATE - ply`).

| Pos | Huginn d | Huginn score | MTL d | MTL score | Δ depth |
|---|---:|---:|---:|---:|---:|
| WAC.001 | 13 | M+8    | 22 | +28997 | +9  |
| WAC.002 | 11 | +70    | 19 | +285   | +8  |
| WAC.003 | 12 | +320   | 19 | +320   | +7  |
| WAC.004 | 19 | M+4    | 27 | +28997 | +8  |
| WAC.005 | 19 | M+4    | 30 | +28997 | +11 |
| WAC.006 | 18 | +723   | 25 | +1480  | +7  |
| WAC.007 | 12 | +525   | 18 | +491   | +6  |
| WAC.008 | 17 | +1460  | 20 | +1284  | +3  |
| WAC.009 | 10 | **−223**  | 26 | **+28991** | **+16** |
| WAC.010 | 15 | +415   | 21 | +377   | +6  |
| **Mean** | **14.6** | | **22.7** | | **+8.1** |

**Mean depth gap: 8.1 ply.** With effective branching factor ~3-4 in
the pruned tree, that's ~6.5k–65k× more lines effectively examined per
move. The full WAC solving-rate gap (79% vs 96%) is essentially this
depth gap manifesting on the subset of positions where 8 plies makes
a tactical difference.

**WAC.009 is the smoking gun for the check-pruning fix.** Same position,
two engines, two completely different conclusions:

- Huginn at depth 10: **−223 cp** ("I'm losing 2¼ pawns")
- MTL at depth 26:    **+28991** ("I see a forced mate")

The expected move is `Bh2+` — a check-tactic. Huginn's LMR table
reduces exactly the move that would force the mating sequence, so the
search never reaches the depth where the win is visible. This is the
canonical example for the Priority-1 fix: don't reduce check-giving
moves and the engine sees Bh2+'s followup at the same 5s budget.

**Where the engines agree** (WAC.003 +320/+320, WAC.007 +525/+491,
WAC.010 +415/+377): positions that are tactically resolved at moderate
depth — both engines get there. Score magnitude tracks each engine's
own search depth (WAC.006 huginn +723 vs MTL +1480 — same direction,
MTL sees the win is bigger because it's looked further into the
liquidation).

**Mate-find depths:** when the mate is short enough (M+4 / M+8) huginn
sees it too. The gap shows up where the forcing line is longer than
huginn's effective depth allows.

## External rating ladder (calibration)

Transitive Elo estimates derived from a 2026-04-28 gauntlet run: MORA's
published rating is **2191**, MORA scored **-123 Elo** vs MTLChess_v0.5
(100 games, tc=10+0.1), and MTLChess_v0.5 was **+330 Elo** over
MTLChess_v0.3 (100 games, same TC). Working backward:

| Opponent | Estimated Elo | Binary in fastchess folder |
|---|---|---|
| MTLChess_v0.3 | ~1984 | `mtlchessV3.exe` (rebuilt on new machine; the older `mtlchess003.exe` and friends SIGILL on Raptor Lake — BACKLOG #5) |
| MORA | 2191 (anchor) | `MORA110.exe` |
| MTLChess_v0.5 | ~2314 | `mtlchess_v05.exe` (SIGILL on this machine, needs rebuild) |

The earlier "MTLChess ~2100" reference in this doc was probably v0.4,
which sits between v0.3 and v0.5. Huginn's old 0-20 vs "MTLChess" was
almost certainly against v0.4 or v0.5, which explains the lopsided
result. The realistic near-term rung is **MTLChess_v0.3 (~1984)**;
once we beat that, **MORA (2191)** is the next target.

**Recent MTL v0.3 calibrations (`test_huginn_vs_mtlchess_v03.bat`):**

| Date | Stack | Score | Elo |
|---|---|---|---|
| 2026-04-30 | post-mobility, prior machine, old binary | 2W/17L/1D / 20g | ~-340 |
| 2026-05-06 | post-2c TT-mate, rebuilt v0.3 | 5W/89L/6D / 100g | -424 ± 118, LOS 0% |
| 2026-05-09 | post-TB + post-contempt | 2W/95L/3D / 100g | -576 ± 250, LOS 0% |

The 2026-05-09 drop vs 2026-05-06 is consistent with contempt's
textbook downside against much-stronger opponents — contempt biases
away from drawing, and against an opponent we're ~400 Elo behind,
draws are precious. Internal t-chain (t1→t4) remains the trustworthy
regression gauge; external MTL calibration is for long-horizon
position tracking only.

## Tier 1 progress (2026-04-28, updated 2026-05-11)

Bisected each Tier 1 item individually vs `huginn4F` (Phase-4-final
baseline) at tc=10+0.1, 100 games each unless noted.

| # | Item | Status | Result vs 4F |
|---|---|---|---|
| 1 | Aspiration windows | step (a) **shipped** (root PVS); step (b) **RE-DEFERRED 2026-05-11** | step (a): +13.9 Elo / 100g; step (b) re-attempt on t4: -24 / -42 Elo (BACKLOG #8). +25 Elo recovered since original but still net-negative. |
| 2 | Reverse futility / static null-move pruning | ✅ shipped | included in stack |
| 3 | Late Move Pruning (LMP) | **DEFERRED to Tier 3** | see "LMP fix attempts" below — incrementally unblocking via shipped ordering work (BACKLOG #7) |
| 4 | Doubled-pawn penalty (wired) | ✅ shipped | included in stack |
| 5 | Tempo bonus | ✅ shipped | included in stack |
| 6 | Razoring (enabled) | ✅ shipped | +35 Elo (LOS 87.75%, 100 games), tested alone |

**Subsequently shipped on top of t1/t2/t3/t4 (search-side, 2026-05-06 → 2026-05-11):**

| # | Item | Result | Notes |
|---|---|---|---|
| #13 | TT-mate-distance adjustment with cap clamp (the "2c" variant) | +104 ± 62 Elo / 100g vs t2 LOS 99.98% | Ply-tracked, fixes leaf-encoding inconsistency under check extensions. Anchor for `baseline-t3 = 2e97066`. |
| #14 | Movegen pseudo-legal at search call sites | +10 ± 56 Elo / 100g vs t2 | Bench: +41% NPS / +43% wall-clock at depth 11 startpos. |
| #10 | Syzygy tablebase via Fathom | +15.65 ± 42 Elo / 200g vs t3 LOS 77% | Neutral within noise at fast TC; matters more at long TC. |
| #16 | Contempt = 25 cp at draw-score returns | +40.13 ± 41 Elo / 200g vs t3 LOS 97.5% | Anchor for `baseline-t4 = 6e3a761`. |
| #1 P1a | LMR-exempt-check (skip LMR on check-giving moves) | +22.62 ± 44 Elo / 200g vs t4 LOS 84% | Validates the "deferred section unblocks incrementally" hypothesis. Recovers +25 Elo over original 2026-05-02 measurement. |

Cumulative shipped stack across t1 → t4: roughly **+180 to +220 Elo**
over `huginn_t1` (the original Tier 1 base). Internal regression
ladder: `baseline-t1 = 4f0ff0c` → `baseline-t2 = 66685f3` →
`baseline-t3 = 2e97066` → `baseline-t4 = 6e3a761`. Use
`test_huginn_vs_t4.bat` for current-tip gauntlets.

Broken commits preserved at git tag `tier1-stack-broken` for the
eventual fix attempt. Root causes:

- **LMP**: pruning condition uses physical move-list index `i` instead
  of "quiet moves searched." With captures ordered first, in tactical
  positions with ≥5 captures the very first quiet move (at `i ≥ 5`,
  threshold = 5 at depth 1) is pruned outright. Fix: count quiet moves
  in a separate counter and gate on `quiet_count >= threshold`.
  (See "LMP fix attempts" below — the index bug is necessary but not
  sufficient for LMP to work in this engine.)
- **Aspiration / root PVS**: when a root move returns `score >= beta`,
  the inner loop doesn't break — subsequent moves are searched with
  an inverted window `(-beta, -local_alpha)` (since `local_alpha > beta`),
  which can pollute the TT with garbage bound entries that survive the
  re-search. Fix: `if (best_score >= beta) break;` at root, and split
  the aspiration-window change from the root-PVS change so each can
  be tested independently.

  **Update (2026-04-30):** the split was done. Step (a) — root PVS
  with the fail-high break, no aspiration window — committed as
  `28cb2cd`, **+13.9 Elo / 100 games vs t1, LOS 65%**. Tree shape at
  startpos depth 11: 17.78M → 3.02M nodes (5.7× faster). Step (b) —
  layering the actual aspiration window around prev_score — was tested
  in two tunings (Stockfish-style with beta-pull and 1.5× delta growth:
  -75 Elo; MTL-style without beta-pull and 2× delta growth: -49 Elo).
  Both regress. Diagnosis is the same shape as LMP: aspiration windows
  depend on stable inter-iteration scores or strong move ordering, and
  we have neither. Deferred to revisit after SEE + continuation history.
  See "Aspiration step (b) deferred" subsection below.

### Aspiration step (b) deferred (2026-04-30)

| Variant | delta growth | beta-pull on fail-low | vs t1 (gauntlet) |
|---|---|---|---|
| step (a) only (root PVS, no aspiration) | n/a | n/a | **+13.9 Elo / LOS 65%** ✅ shipped |
| step (b) v1 (Stockfish-style)            | × 1.5 | yes | -75 Elo / LOS 1.14% |
| step (b) v2 (MTL-style)                  | × 2   | no  | -49 Elo / LOS 4.96% |

The split-per-step strategy worked exactly as intended for diagnosis:
step (a) is a clean +14 Elo win and lives on the branch. Step (b) layered
on top regresses by ~60-90 Elo regardless of tuning — the issue is
fundamental, not parametric. Likely cause: our PST-driven eval has
larger inter-iteration score swings than mature engines, and our move
ordering isn't strong enough to recover gracefully when the tight
aspiration window forces a re-search. With a 50 cp initial delta and
delta-doubling on fail, each fail costs a full root re-search; in
positions where scores swing > 50 cp between depths (common in our
engine), aspiration burns more time than it saves.

**Decision**: keep step (a), defer step (b). Revisit after SEE-based
move ordering and continuation history land — those should both
stabilize the tree shape and tighten between-depth score variance.

**Re-attempt 2026-05-11 (on top of t4 = TT-mate + TB + contempt + P1a):**

| Variant | INITIAL_DELTA | MIN_DEPTH | vs t4 (200g) |
|---|---|---|---|
| v2 (MTL-style, no beta-pull) | 50  | 4 | -24.36 ± 41.83 Elo, LOS 12.5% |
| v3 (wider, higher min-depth)  | 100 | 6 | -41.89 ± 37.07 Elo, LOS  1.2% |

v3 was strictly worse — wider window meant less in-window cutoff
benefit. The cumulative t2→t4 ordering work (TT-mate +104, TB +16,
contempt +40, P1a +22) lifted v2 by +25 Elo over the original -49 vs
t2, same magnitude as #1 P1a got, but starting deeper in the hole
means it's still net-negative. Re-deferred; WIP on branch
`experiment/aspiration-step-b` (commit `fa6c66e`). The bottleneck is
inter-iteration score stability; aspiration unlocks when continuation
history (#3) or further ordering work lands, not via parameter tuning
of this variant.

### King safety attempt (2026-04-30) — DEFERRED

Implemented Tier 2 #8 in three layers (pawn shield + open files near
king + 3×3 attack zone with piece-weighted attack units), then tuned
across three iterations. None crossed positive Elo against `huginn_t2`:

| Variant | What changed | vs t2 (100 games) |
|---|---|---|
| v1 | Initial: `popcount(attacks & zone) × weight`, `DANGER_MAX = 400` | -20.87 Elo |
| v2 | Presence-based attacks (`if (attacks & zone) += weight`), `DANGER_MAX = 200` | **-3.47 Elo** (best) |
| v3 | v2 + lower file/shield penalties (open 25→18, adj 15→10, missing-shield 8→5) | -17.39 Elo |

The v1→v2 jump (+18 Elo) confirmed the algorithm itself is sound — the
attack-unit over-counting was a real bug. v3 dialed too far in the
"don't penalize king exposure" direction and made the engine careless,
re-confirming v2's settings were near-optimal among hand-tuned values.

After 3 iterations / ~300 games, signal stays at "approximately zero."
Reverted (no commit). Code is in git reflog if/when we revisit. Two
plausible reasons it didn't pay out:

1. **Hand-tuning has hit its ceiling.** Three iterations on six
   parameters can't beat numerical optimization. A Texel-style tuner
   over a few-million-position labelled corpus would likely find a
   parameter set that lands +20-50 Elo. Worth doing once we have other
   eval features competing for the same parameter space (mobility,
   king-attacker-count weighting, etc).
2. **Compounds-but-doesn't-stand-alone.** King safety pays off most
   when paired with mobility (less mobile = harder to escape attacks)
   and with continuation history (better move ordering surfaces the
   tactical sequences king safety predicts). In isolation on a tactically
   weak engine, the eval just nudges piece placement around without
   adding decisive Elo. Re-test after mobility lands.

Files NOT modified for this defer: implementation lived only in
working tree, no commit. Eval params for king safety (`KING_SHIELD_PAWN_BONUS`
etc.) were proposed-then-removed in [src/evaluation.hpp](src/evaluation.hpp).
Implementation block lived in `Engine::evaluate()` in
[src/search.cpp](src/search.cpp).

### LMP fix attempts (2026-04-30) — DEFERRED

The original `i`-vs-`quiet_count` bug was fixed (per-node quiet counter,
gate on `quiet_count >= threshold`). Three iterations of conservatism
dialing produced an asymptote toward parity but never crossed zero:

| Variant | Threshold | Min depth | vs t1 (100g) |
|---|---|---|---|
| Fix #1 | `4 + d²` | 1 | -254 Elo |
| Fix #2 | `4 + d²` | 3 | -56 Elo |
| Fix #3 (planned non-PV gate) | `4 + d²` | 3 | not tested — see below |

Reading: each tighter dial halves the loss but the trend asymptotes
toward small-negative. MTLChess (which we benchmark against) doesn't
implement LMP at all — its tight tree comes from RFP + null move +
SEE-ordered moves + tuned LMR + working aspiration windows. LMP is a
Stockfish-style technique that depends on continuation-history-rich
move ordering. Without that ordering, the top-K quiets aren't reliable
enough for safe pruning at low depth.

**Decision**: drop LMP for now, revisit once SEE and continuation
history are landed (re-classified as Tier 3). The reverted code lives
on git tag `tier1-stack-broken`; the per-node `quiet_count` plumbing is
the right approach when we come back.

The EPD divergence harness ([tools/test_epd_diff.py](tools/test_epd_diff.py))
written during this session is a keeper — it cut iteration time from
~30 min (full gauntlet) to ~2 min (lct2.epd diff at depth 7) for
spotting search-shape changes. Reuse for future search-tree work.

## Current state — Search

Inventory refreshed against `pure-bitboard-engine` tip (2026-05-07). Every
row is anchored to file:line so divergence shows up the next time the doc
is audited.

### Core
| Technique | Where | Status |
|---|---|---|
| Negamax + alpha-beta | [search.cpp:1183](src/search.cpp#L1183) `Engine::AlphaBeta` | ✓ |
| Principal Variation Search (PVS) | [search.cpp:1480](src/search.cpp#L1480) | ✓ null-window for moves ≥ 2, full re-search on score > alpha |
| Iterative deepening | [search.cpp:1805](src/search.cpp#L1805) `searchPosition()` loop | ✓ |
| Quiescence search | [search.cpp:1678](src/search.cpp#L1678) | ✓ captures + promotions, depth-limited (10 plies), SEE-pruned |
| Transposition table | [transposition_table.hpp](src/transposition_table.hpp), probe at [search.cpp:1194](src/search.cpp#L1194), store at [search.cpp:1593](src/search.cpp#L1593) | ✓ EXACT/LOWER/UPPER bounds, depth-preferred replacement, mate-distance adjusted by ply |
| PV table (triangular hash) | [pvtable.cpp](src/pvtable.cpp), reconstruction at [search.cpp:1921](src/search.cpp#L1921) `get_pv_line` | ✓ 2 MB hash, used for UCI `info pv` output |
| Repetition detection | [search.cpp:534](src/search.cpp#L534) `isRepetition` | ✓ 3-fold, last 12 plies, ≥ 6-ply minimum |
| Time management | [search.cpp:436](src/search.cpp#L436) `time_up`, [search.cpp:1821](src/search.cpp#L1821) iteration gate | ✓ stop_time + iteration-start gate (`elapsed > budget/4`) + checkup every 2048 nodes + <100 ms early exit |
| Mate-distance scoring | TT store/probe at [search.cpp:1199](src/search.cpp#L1199) and [search.cpp:1579](src/search.cpp#L1579) | ✓ ply-tracked encoding consistent with check extensions |
| Material-draw detection | [search.cpp:310](src/search.cpp#L310) `MaterialDraw` | ✓ KvK, KNvK, KBvK |
| Polyglot opening book | [polyglot_book.hpp](src/polyglot_book.hpp), used at [search.cpp:1764](src/search.cpp#L1764) | ✓ binary search, weighted moves |
| Syzygy tablebase probe | [syzygy_tablebase.cpp](src/syzygy_tablebase.cpp), in-search probe in `AlphaBeta` | ✓ shipped 2026-05-08 via Fathom (BACKLOG #10). In-search probe fires at depth ≤ 1 when ≤ max_pieces; safe wrapper enforces castling=0 and rule50=0 guards; CURSED_WIN/BLESSED_LOSS classify as draws (±1 cp). Direct bitboard-load probe inputs from `Position::piece_bitboards`. No TT-cache on probe success (halfmove not in zobrist). Default `SyzygyPath` is `c:\TB\`. Requires `-DENABLE_FATHOM=ON` at configure time. |
| Contempt | `evaluate()` insufficient-material, `AlphaBeta` repetition, `AlphaBeta` stalemate | ✓ shipped 2026-05-09 (BACKLOG #16). CONTEMPT = 25 cp returned at all three draw-score sites in negamax-correct sign (always -CONTEMPT from side-to-move POV). +40 Elo / LOS 97.5% vs t3. Caveat: net-negative vs much-stronger opponents — see BACKLOG #5 MTL calibration. |

### Pruning / reductions / extensions
| Technique | Where | Status |
|---|---|---|
| Null-move pruning | [search.cpp:1278](src/search.cpp#L1278) | ✓ R=4, depth ≥ 5, non-pawn material required, blocked when in check |
| Reverse futility / static null-move | [search.cpp:1263](src/search.cpp#L1263) | ✓ depth ≤ 6, margin = 80·depth |
| Futility pruning | [search.cpp:1325](src/search.cpp#L1325) | ✓ depth ≤ 3, margin = 100 + 50·depth |
| Razoring | [search.cpp:1354](src/search.cpp#L1354) | ✓ depth 2-4, margin 400, soft (depth-reduction not return) — **+35 Elo measured** |
| Late Move Reductions (LMR) | [search.cpp:1433](src/search.cpp#L1433) | ✓ 64×64 `log(d)·log(m)/2` table, min depth 3 / move ≥ 4, PVS re-search on fail-high. **P1a exemption (BACKLOG #1, shipped 2026-05-11)**: also skip LMR on moves that give check (lazy `gives_check` lambda using `SqAttacked(opp_king, pos, !side_to_move)` after MakeMove). +22 Elo / LOS 84% vs t4. |
| SEE pruning (qsearch) | [search.cpp:1725](src/search.cpp#L1725), [see.cpp:96](src/see.cpp#L96) | ✓ skip captures with SEE < 0 (excluding promotions) |
| Check extension | [search.cpp:1242](src/search.cpp#L1242) | ✓ `depth++` when in check |
| Internal Iterative Deepening (IID) | [search.cpp:1598](src/search.cpp#L1598) | ✓ when no TT move; ordered as priority 1.5M |
| Multi-cut pruning | [search.cpp:1398](src/search.cpp#L1398) | ✗ behind `#ifdef USE_MULTI_CUT`, **NOT enabled** |
| Late Move Pruning (LMP) | — | ✗ attempted, reverted; needs SEE main-ordering + continuation history first (see Tier 1 progress) |
| Aspiration windows at root | — | ✗ step (a) root-PVS shipped (+13.9 Elo / `28cb2cd`); step (b) re-deferred 2026-05-11 — recovered +25 Elo since original attempt but still net-negative (BACKLOG #8) |

### Move ordering
| Technique | Where | Status |
|---|---|---|
| TT best move | [search.cpp:841](src/search.cpp#L841) | ✓ priority 3,000,000, validated for legality |
| PV move | [search.cpp:879](src/search.cpp#L879) | ✓ priority 2,000,000 |
| IID move | [search.cpp:883](src/search.cpp#L883) | ✓ priority 1,500,000 |
| MVV-LVA captures | [search.cpp:693](src/search.cpp#L693) `init_mvv_lva`, scoring at [search.cpp:886](src/search.cpp#L886) | ✓ base 1M + (victim·100 + 600 - attacker), EP +10k |
| Promotions | [search.cpp:936](src/search.cpp#L936) | ✓ Q=90k, R=50k, B=33k, N=32k |
| Killer moves | [search.cpp:646](src/search.cpp#L646) `update_killer_moves`, scoring at [search.cpp:903](src/search.cpp#L903) | ✓ 2 slots/ply, non-captures only (900k / 800k) |
| Counter-move heuristic | [search.cpp:917](src/search.cpp#L917) (read), [search.cpp:1528](src/search.cpp#L1528) (update) | ✗ gated by `ENABLE_PLY_TRACKED_COUNTERMOVE = 0`. Re-attempted 2026-05-11 (BACKLOG #15) at scores 15K and 1500 on t4 — both noise-bound (LOS 32% and 66%), deferred. WIP on branch `experiment/counter-move-1500`. |
| History heuristic | [search.cpp:602](src/search.cpp#L602) update, [search.cpp:619](src/search.cpp#L619) penalty, [search.cpp:636](src/search.cpp#L636) age, [search.cpp:947](src/search.cpp#L947) scoring | ✓ [piece][to] table, depth² bonus/penalty, ×7/8 age every 3 depths |
| Staged move picker | [search.cpp:838](src/search.cpp#L838) `pick_next_move` | ✓ selection-sort over scored list (no separate stages) |

### Disabled / broken
- **Counter-move heuristic** is implemented but gated off by `ENABLE_PLY_TRACKED_COUNTERMOVE = 0` near the top of `src/search.cpp`. Update path on beta cutoff and read path in ordering both compile out. Re-attempted 2026-05-11 (BACKLOG #15) at scores 15K and 1500 on top of t4 — both ~noise-bound (LOS 32% and 66%), deferred. WIP preserved on branch `experiment/counter-move-1500`.
- **Multi-cut** is behind `#ifdef USE_MULTI_CUT` and not defined in any preset.
- **LMP**: implementation attempted, reverted after gauntlet showed regression. Buggy code preserved at git tag `tier1-stack-broken`; deferred section in BACKLOG #7 is now incrementally unblocking via shipped ordering work.
- **Aspiration step (b)**: re-attempted 2026-05-11 (BACKLOG #8) at two tunings on t4 (delta=50/min-depth=4: -24 Elo / LOS 12%; delta=100/min-depth=6: -42 Elo / LOS 1%). Recovered +25 Elo from original -49 vs t2 — same lift magnitude as #1 P1a got, but starting point was deeper. Re-deferred. WIP on branch `experiment/aspiration-step-b`.

### Notable gaps (techniques not present at all)
- **Aspiration windows** around `prev_score` at root (step (b) deferred)
- **Singular extensions** / recapture extension / passed-pawn-push extension
- **ProbCut**
- **Late Move Pruning** (move-count-based) — see deferral
- **History pruning**, **capture history**, **continuation history** (counter-move + followup)
- **SEE pruning in main search** (qsearch only)
- **SEE-based capture ordering in main search** (still MVV-LVA)
- **Lazy SMP / multithreaded search**
- **MultiPV**
- **Staged move generation** (TT → captures → quiets → bad-captures)
- **TT prefetch**
- **Improving heuristic**

## Current state — Evaluation

Hand-crafted, no NNUE. Phase-aware via discrete buckets (no smooth taper).

### Material & tables
| Term | Where | Status |
|---|---|---|
| Material | `PIECE_VALUES_MG` × popcount, [see.hpp:29](src/see.hpp#L29) | ✓ P=100, N=320, B=330, R=500, Q=900 |
| Piece-square tables | [evaluation.hpp:132-195](src/evaluation.hpp#L132-L195) (P/N/B/R/Q + dual K), used at [search.cpp:88](src/search.cpp#L88) | ✓ separate `KING_TABLE` and `KING_TABLE_ENDGAME` |
| Game-phase detection | [evaluation.hpp:72-76](src/evaluation.hpp#L72-L76) (`ENDGAME_MATERIAL_THRESHOLD = 1150`) | ✓ 3-bucket (open/mid/end), discrete switch — **not a smooth taper** |
| Side-to-move perspective | end of `evaluate()` | ✓ negate score for Black |

### Pawn structure
| Term | Where | Status |
|---|---|---|
| Isolated pawns | [search.cpp:148](src/search.cpp#L148), `ISOLATED_PAWN_PENALTY = 10` | ✓ |
| Doubled pawns | [search.cpp:172](src/search.cpp#L172), `DOUBLED_PAWN_PENALTY = 20` per extra | ✓ |
| Passed pawns | [search.cpp:151](src/search.cpp#L151), `PASSED_PAWN_BONUS[rank]` = {0,5,10,20,35,60,100,200} | ✓ |

### Pieces
| Term | Where | Status |
|---|---|---|
| Bishop pair | [search.cpp:244](src/search.cpp#L244), bonus = 50 | ✓ |
| Rook on open / semi-open file | [search.cpp:185-242](src/search.cpp#L185-L242), 10 / 5 | ✓ |
| Queen on open / semi-open file | same block, 5 / 3 | ✓ |
| Mobility | [search.cpp:255](src/search.cpp#L255), N/B/R/Q safe-square count, MG=5/EG=2 | ✓ |

### Misc
| Term | Where | Status |
|---|---|---|
| Tempo bonus | [search.cpp:305](src/search.cpp#L305), `TEMPO_BONUS = 10` cp | ✓ |
| Insufficient-material draw | [search.cpp:310](src/search.cpp#L310) `MaterialDraw` | ✓ KvK, KNvK, KBvK |
| Mirror-evaluation symmetry test | [search.cpp:409](src/search.cpp#L409) `MirrorAvailTest` | ✓ test harness only |

### Defined but not integrated
These constants exist in [evaluation.hpp](src/evaluation.hpp) but no code path
in `Engine::evaluate()` reads them. Wiring them up is a half-hour
each, often with negligible Elo without a tuner.

| Param | Where | Notes |
|---|---|---|
| Knight outpost (+25) | [evaluation.hpp:82-89](src/evaluation.hpp#L82-L89) | rank gates defined, never queried |
| King shield multiplier (10) | [evaluation.hpp:126](src/evaluation.hpp#L126) | attempted 2026-04-30, regressed; deferred |
| Castle bonus (40) / stuck penalty (20) | [evaluation.hpp:129-130](src/evaluation.hpp#L129-L130) | unused |
| Development bonus (60/40/0) | [evaluation.hpp:117-119](src/evaluation.hpp#L117-L119) | unused |

### Notable gaps (no parameters or implementation)
- **NNUE evaluation** (Tier 3 #15)
- **Smooth tapered eval** — interpolation between MG/EG via a phase score
  rather than the current 3-bucket switch
- **King safety / attack zone** — pawn-shield + king-zone-attacker count
  (Tier 2 #8, attempted and deferred 2026-04-30)
- **Backward pawns**, **connected pawns**, **candidate passers**, **pawn islands**
- **Rook on 7th rank**, **rook behind passed pawn**, **connected rooks**
- **Bad bishop / blocked bishop**
- **Threats** (hanging pieces, weak squares)
- **Space evaluation**
- **Imbalance table** (Stockfish-style material interaction terms)
- **Endgame-specific scaling** beyond king-table swap (no KPK, no
  opposite-color-bishops drawish factor)
- **Pawn hash table**, **eval cache**
- **Tuning framework** (Texel / gradient-descent)

## Planned improvements (ranked by Elo/effort ratio)

Estimates are rough — small numbers (5-20 Elo) are easy to validate at
500 games per change, larger ones (50-100) need 1k-2k games for tight
CIs. Effort is "from scratch" — not counting tuning iterations.

### Tier 1 — High Elo / low effort

| # | Item | Estimated Elo | Effort | Status |
|---|---|---|---|---|
| 1 | **Aspiration windows** | +30-50 | 1-2 hrs | ❌ attempted; reverted. -402 Elo. Root cause: inverted-window TT pollution at root. Fix queued. |
| 2 | **Reverse futility / static null-move pruning** | +20-40 | <1 hr | ✅ shipped (depth ≤ 6, margin = 80·depth) |
| 3 | **Late move pruning (LMP)** | +20-40 | 1 hr | ❌ attempted; reverted. -381 Elo. Root cause: pruning indexed by physical move-list position, not quiet-moves-searched. Fix queued. |
| 4 | **Wire up doubled-pawn evaluation** | +10-20 | 30 min | ✅ shipped |
| 5 | **Tempo bonus** | +5-15 | 5 min | ✅ shipped (10 cp) |
| 6 | **Enable razoring** | +10-25 | 1 hr (test+tune) | ✅ shipped — **+35 Elo measured (LOS 87.75%)** |

### Tier 2 — Medium Elo / medium effort

| # | Item | Estimated Elo | Effort | Notes |
|---|---|---|---|---|
| 7 | **Mobility evaluation** | ✅ shipped 2026-04-30 (+4.34 Elo / 80g vs t2, LOS 54.86%) | commit `626257a` | Wires the existing `MOBILITY_WEIGHT_DEFAULT`/`_ENDGAME` constants. Mild positive vs t2's pure-PST eval. Combining with king-safety on top is queued. |
| 8 | **King safety / attack zone** | attempted, ~0 Elo at hand-tuned settings | 2026-04-30 attempt deferred — re-attempt-able now mobility (#7) is in | See "King safety attempt" subsection above for the three iterations and root-cause discussion. Mobility is now committed (commit `626257a`); the "compounds-but-doesn't-stand-alone" hypothesis can be retested by re-applying the king-safety code on top of the new tip. |
| 9 | **Pawn shield / pawn storm** | +20-40 | 4 hrs | Bonus for pawns sheltering own king, penalty for advanced enemy pawns. |
| 10 | **Static Exchange Evaluation (SEE)** | ✅ shipped (qsearch pruning, +38 Elo); main-ordering pending | partial | Implementation: [src/see.hpp](src/see.hpp), [src/see.cpp](src/see.cpp). Currently used to skip `SEE < 0` captures in qsearch. Capture ordering in main search is still MVV-LVA only — lazy SEE in `pick_next_move` is a future optional lever. |
| 11 | **Continuation / counter-move history** | +30-50 | 1 day | Two-ply move history (`prev_move → this_move`) for ordering. **Status update 2026-05-11:** ply tracking machinery shipped via #13 (TT-mate), so counter-move and continuation history are both unblocked at the infrastructure level. Counter-move at scores 15K/1500 noise-bound (BACKLOG #15). Continuation history (BACKLOG #3) still pending implementation; expected to be the next significant lever. |
| 12 | **Singular extensions** | +20-40 | 1 day | Extend when TT move is much better than alternatives at reduced depth. |
| 13 | **Improving heuristic** | +10-25 | 2 hrs | Track whether eval is improving plies-back; relax LMR / tighten futility when not improving. |
| 14 | **Probcut** | +20-40 | 1 day | At high depth, do a reduced-depth search with raised beta to safely prune. |

### Tier 3 — Big payoff / big effort

| # | Item | Estimated Elo | Effort | Notes |
|---|---|---|---|---|
| 15 | **NNUE evaluation** | +200-300 | 1-2 weeks | Modern small-net evaluation. Drop in a pretrained net (e.g. nnue-pytorch) plus eval+search integration. Single biggest available upgrade. |
| 16 | **Lazy SMP** | +50-80 | 2-3 days | Multi-threaded search via shared TT. Hardware-dependent. |
| 17 | **TB probing wired up** | shipped 2026-05-08 | actual: ~1.5 days of fix-stack | ✅ Shipped via Fathom — BACKLOG #10. "Just removing the gate" turned into four follow-up bugs (uint64_t truncation, popcount macro leak via tbchess.c, rule50/castling guards, TT-cache pollution from halfmove-blind zobrist). +15.65 ± 42 Elo / 200g vs t3 LOS 77% — neutral within noise at tc=10+0.1 as expected; matters more at long TC. Requires `-DENABLE_FATHOM=ON`. |

### Tier 4 — Architectural / nice-to-have

| # | Item | Effort | Notes |
|---|---|---|---|
| 18 | **Migrate `S_MOVE` from 120-sq to 64-sq** | 1-2 days | ~+5-10% nps (~+10-20 Elo). Cleaner code; not a strength path. Discussed in plan memory. |
| 19 | **TT prefetch** | 2 hrs | `__builtin_prefetch` on the TT slot before move loop. Small but cheap. |
| 20 | **Magic-bitboard slider attacks** | 1-2 days | Already partially used; verify all slider attacks go through magics not ray-tracing. |
| 21 | **Smaller move encoding** | 4 hrs | If `S_MOVE` is migrated, the bit packing could shrink further. |

## Next session plan (recorded 2026-04-30 end-of-session)

WAC300 calibration ran tonight against both engines, 5s/position:

| Engine | Solved | Failed | Rate |
|---|---|---|---|
| MTLChess_v0.5 | 288 | 12 | **96.0%** |
| Huginn current | 237 | 63 | **79.0%** |

Of huginn's 63 failures, **52 are positions MTL solves cleanly** — pure
tactical strength gap, not orthogonal sight. The "huginn-only" overlap
is just 1 position. Truly hard problems both engines miss are 11; five
of those are knight tactics (Nxa7+, Nxd5, Nb4, Nf5, Nfg5), which need
deeper search than 5s gives.

**Pattern across both the regressions-vs-old-huginn and the gap-vs-MTL
sets: check-moves dominate.** Bh2+, Qxg7+, Qd1+, Nxg7, Bh6, Qf1+,
Rxb7, Re8, Qxh7+, Qxg6+, Rxg2+ — all positions where the right move
gives check or is a sacrificial check, and huginn doesn't see it.

### Priority 1: Don't prune/reduce moves that give check

**Status update 2026-05-11:** Part (b) — **LMR-exempt-check (P1a) shipped at `2dbd856`** — +22.62 ± 44.20 Elo / 200g vs t4 LOS 84% (BACKLOG #1). Part (a) — qsearch-SEE-exempt-check (P1b) — remains deferred; the original -28 Elo delta from MakeMove+SqAttacked+TakeMove per SEE-pruned capture in the qsearch hot loop is unlikely to be recovered by ordering improvements alone. Pre-shipping analysis preserved below.

Two well-known engine practices we don't yet observe:

  (a) **Skip SEE-pruning when the candidate capture gives check.** Today
      our qsearch has `if (!is_promotion && SEE < 0) continue;` — this
      silently kills sacrificial-check tactics like `Qxh7+` (queen for
      pawn = SEE −700, but the followup wins). Caller of `see()`
      doesn't know whether the move gives check.

  (b) **Skip LMR reduction when the move gives check.** Today the LMR
      table reduces all late, non-capture, non-promotion, non-in-check
      moves. Quiet-check moves like `Bh2+`, `Re8+`, `Rd4+` get reduced
      and the engine doesn't see far enough into the forcing line.

Implementation outline:

```cpp
// Light helper — make/test/unmake
inline bool gives_check(Position& pos, const S_MOVE& m) {
    if (pos.MakeMove(m) != 1) return false;
    bool check = SqAttacked(pos.king_sq[int(pos.side_to_move)],
                            pos, !pos.side_to_move);
    pos.TakeMove();
    return check;
}
```

Costs ~MakeMove+SqAttacked+TakeMove per pruned/reduced move. The fix
probably pays for itself: we save the entire subtree for moves we'd
otherwise prune wrongly. If perf is an issue later, switch to
pre-computed check-square bitboards à la Stockfish.

Sites:
- [src/search.cpp:1573](src/search.cpp#L1573) (qsearch
  SEE prune): wrap with `&& !gives_check()`.
- [src/search.cpp:1330](src/search.cpp#L1330) (LMR
  block): add `&& !gives_check()` to the gate, or set R=0 for that move.

Expected: recover 5-15 of the 24 huginn-vs-old regressions, plus
several of the 51 huginn-vs-MTL gap positions. Per-feature target
+20-40 Elo. Verify with WAC re-run + gauntlet vs t2.

### Priority 2: Re-attempt king safety on top of mobility

The compound-features hypothesis from the king-safety defer note can now
be tested. Cherry-pick the v2 settings (presence-based attack count,
DANGER_MAX = 200) onto the current tip post-mobility and re-gauntlet.
If it lands meaningfully positive, mobility was the missing companion.

### Priority 3: Continuation history (Tier 2 #11)

Two-ply move history (`prev_move → this_move`) for ordering. Compounds
with SEE (better tactical move ordering) and is a prerequisite for
revisiting LMP and aspiration step (b).

### Maintenance / open items

- **Investigate the fastchess hang at 80 games** seen during the
  mobility gauntlet. Could be a one-off (cosmic ray, OS scheduler) or
  a real engine hang on a specific position. If it recurs, capture
  the engine state and dig.
- **Refresh `huginn_t1.exe` / `huginn_t2.exe` baselines** if cumulative
  gain over t2 crosses ~+50 Elo — same +50 trigger as before.
- **Re-run `test_huginn_calibration.bat mtl03 50`** after the check-fix
  lands. Last calibration was 2W/17L/1D vs MTL_v0.3; with the check
  fix we should expect a meaningful improvement.

### Tooling reminders for next session

- `python test/wac_test.py 300 -t 5` — full tactical-solving sweep
  (~25 min). Now accepts `--engine <path>` for any UCI binary.
- `test_huginn_vs_t2.bat 50` — regression match vs frozen t2 baseline.
- `test_huginn_calibration.bat <mtl03|mora|mtl05> 50` — external rating.
- `python tools/test_epd_diff.py test/lct2.epd 7` — fast 2-min EPD
  divergence harness for spotting search-shape changes between builds.

## Historical: how we got here

The 22× time-to-depth gap vs MTLChess says we're losing on tree shape,
not eval. Reorder the work accordingly:

1. ~~**Add SEE — promoted from Tier 2 #10 to Tier 1.**~~ ✅ **Shipped
   2026-04-30 as commit `1cce8de`.** SEE wired into qsearch only (skip
   `SEE < 0` captures). Capture ordering left as MVV-LVA in main search;
   lazy-SEE-in-pick is a future optional Elo lever. Result: +38.4 Elo
   vs t1 / 100 games / LOS 88.98% (combined with step (a)'s +14, the
   stack is now ~+38 over t1). Tree shape at startpos depth 11:
   17.78M → 1.78M nodes (10× faster). *Also a prerequisite for
   revisiting aspiration step (b) and LMP.*
2. ~~**Tune LMR.**~~ ✅ **Shipped 2026-04-30 as commit `66685f3`.**
   Replaced the R=1/R=2 step formula with a 64×64 `log(d)*log(m)/2` table
   computed once via lambda IIFE. Result: +14 Elo marginal vs the SEE tip,
   +52.5 Elo cumulative vs t1 / 100 games / LOS 95.68%.
3. ~~**Tier 2 #8 (king safety)**~~ — attempted, deferred (see "King
   safety attempt" subsection above). Revisit after mobility lands or
   once a Texel-style tuner is available.
4. ~~**Tier 2 #7 (mobility)**~~ ✅ **Shipped 2026-04-30 as commit
   `626257a`.** Wired `MOBILITY_WEIGHT_DEFAULT`/`_ENDGAME` (5/2 cp per
   attack square). Result: +4.34 Elo / 80 games / LOS 54.86% — mild
   positive, well within CI noise. The combined "mobility + king
   safety" hypothesis can now actually be tested by re-attempting
   king safety on top of this commit.
5. **Tier 2 #11 (continuation history)** — improves move ordering,
   compounds with SEE.
6. **Revisit aspiration step (b) and LMP** (now Tier 3) once SEE and
   continuation history are in. Both deferred features regress today
   because of weak move ordering / volatile inter-iteration scores; the
   underlying plumbing was implemented correctly in both attempts.
7. Re-run calibration vs MTLChess_v0.3 after each major fix; target
   crossing **+0 Elo (≥ ~1984)**, then aim at MORA (2191).
8. Refresh the regression baseline whenever cumulative gain over the
   current baseline crosses ~+50 Elo. **First refresh done 2026-04-30:**
   `baseline-t2` (tag) = commit `66685f3` = t1 + step (a) + SEE qsearch
   + LMR table = +52.5 Elo over t1 (LOS 95.68%). Future search/eval
   changes should be matched against `huginn_t2.exe`.

Tooling:
- `test_huginn_vs_t2.bat [rounds]` — **current regression match** vs
  frozen `baseline-t2` (commit `66685f3`).
- `test_huginn_vs_t1.bat [rounds]` — long-term comparison vs original
  `baseline-t1` (commit `4f0ff0c`). Useful for cumulative-since-day-1
  measurements; for per-feature deltas prefer `vs_t2`.
- `test_huginn_calibration.bat <mtl03|mora|mtl05> [rounds]` — external
  rating ladder checks.
- `python tools/test_epd_diff.py [epd_file] [depth]` — fast EPD-based
  divergence harness (compares t1 vs current build by default; edit the
  `ENGINES` list at the top to compare any pair). Useful for quick
  search-shape diagnostics in ~2 min instead of ~30 min.

## Reference

- Estimated Elo numbers from chessprogramming.org community data — actual results vary ±50% per implementation.
- Each change should be tested vs. the previous build at 500-1000 games (10+0.1) before stacking. Document the per-feature delta in `perft/performance_tracking.txt` or a new `docs/STRENGTH_LOG.md`.
