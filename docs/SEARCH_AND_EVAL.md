# Search & Evaluation — current state and roadmap

Snapshot of what's implemented in Huginn 2.0 (post Phase 4) and what's
planned. Reality check on each item is anchored to file:line so this
stays accurate as the code evolves.

## Strength gap diagnosis (2026-04-28)

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

## External rating ladder (calibration)

Transitive Elo estimates derived from a 2026-04-28 gauntlet run: MORA's
published rating is **2191**, MORA scored **-123 Elo** vs MTLChess_v0.5
(100 games, tc=10+0.1), and MTLChess_v0.5 was **+330 Elo** over
MTLChess_v0.3 (100 games, same TC). Working backward:

| Opponent | Estimated Elo | Binary in fastchess folder |
|---|---|---|
| MTLChess_v0.3 | ~1984 | `mtlchess003.exe` |
| MORA | 2191 (anchor) | `MORA110.exe` |
| MTLChess_v0.5 | ~2314 | `mtlchess_v05.exe` |

The earlier "MTLChess ~2100" reference in this doc was probably v0.4,
which sits between v0.3 and v0.5. Huginn's old 0-20 vs "MTLChess" was
almost certainly against v0.4 or v0.5, which explains the lopsided
result. The realistic near-term rung is **MTLChess_v0.3 (~1984)**;
once we beat that, **MORA (2191)** is the next target.

Use `test_huginn_calibration.bat <mtl03|mora|mtl05> [rounds]` to run
huginn current against any of these. The script rebuilds and copies
`huginn.exe` automatically.

## Tier 1 progress (2026-04-28)

Bisected each Tier 1 item individually vs `huginn4F` (Phase-4-final
baseline) at tc=10+0.1, 100 games each unless noted.

| # | Item | Status | Result vs 4F |
|---|---|---|---|
| 1 | Aspiration windows | step (a) **shipped** (root PVS); step (b) **DEFERRED** | step (a): +13.9 Elo / 100g; step (b) layered on (a): -75 / -49 Elo over two tunings |
| 2 | Reverse futility / static null-move pruning | ✅ shipped | included in stack |
| 3 | Late Move Pruning (LMP) | **DEFERRED to Tier 3** | see "LMP fix attempts" below — needs SEE + continuation history first |
| 4 | Doubled-pawn penalty (wired) | ✅ shipped | included in stack |
| 5 | Tempo bonus | ✅ shipped | included in stack |
| 6 | Razoring (enabled) | ✅ shipped | +35 Elo (LOS 87.75%, 100 games), tested alone |

Cumulative shipped stack (items 2 + 4 + 5 + 6) = razoring + RFP +
doubled pawns + tempo. 1+2+3 baseline (before razoring) measured at
+7 Elo / 100 games; full stack roughly **+30 to +60 Elo** over 4F.

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

### Core
| Technique | Where | Status |
|---|---|---|
| Iterative deepening | [minimal_search.cpp:1616](src/minimal_search.cpp#L1616) | ✓ |
| Alpha-beta with TT bounds | [minimal_search.cpp:1061](src/minimal_search.cpp#L1061) | ✓ |
| Quiescence search | `quiescence()` in minimal_search.cpp | ✓ depth-limited (10 plies) |
| Transposition table | [transposition_table.hpp](src/transposition_table.hpp) | ✓ EXACT/LOWER/UPPER bounds, mate-distance adjusted |
| Repetition detection | [minimal_search.cpp:429](src/minimal_search.cpp#L429) | ✓ 3-fold, conservative |
| Time management | [minimal_search.cpp:330,1622](src/minimal_search.cpp#L330) | ✓ stop_time + iteration-start gate (`elapsed > budget/4`) + checkup every 2048 nodes |
| Mate distance scoring | TT score adjustment by ply | ✓ |
| Material-draw detection | [minimal_search.cpp:204](src/minimal_search.cpp#L204) | ✓ |

### Pruning / reductions
| Technique | Where | Status |
|---|---|---|
| Null-move pruning | [minimal_search.cpp:1144](src/minimal_search.cpp#L1144) | ✓ R=4, depth ≥ 5, requires non-pawn material |
| Late Move Reductions (LMR) | [minimal_search.cpp:1299](src/minimal_search.cpp#L1299) | ✓ depth ≥ 3, after move 4, with re-search on fail-high |
| Futility pruning | [minimal_search.cpp:1179](src/minimal_search.cpp#L1179) | ✓ depth ≤ 3, margin = 100 + 50·depth |
| Reverse futility / static null-move | minimal_search.cpp (pre-null-move block) | ✓ depth ≤ 6, margin = 80·depth |
| Razoring | minimal_search.cpp | ✓ enabled (was behind disabled `USE_RAZORING` ifdef) — **+35 Elo measured** |
| Late Move Pruning (LMP) | — | ✗ attempted, reverted (see Tier 1 progress) |
| Aspiration windows at root | — | ✗ attempted, reverted (see Tier 1 progress) |
| Multi-cut pruning | minimal_search.cpp | ✗ defined behind `#ifdef USE_MULTI_CUT`, NOT enabled |
| Check extensions | [minimal_search.cpp:1116](src/minimal_search.cpp#L1116) | ✓ `depth++` when in check |
| Internal Iterative Deepening (IID) | [minimal_search.cpp:1255](src/minimal_search.cpp#L1255) | ✓ when no TT move |

### Move ordering
| Technique | Where | Status |
|---|---|---|
| TT best move | [minimal_search.cpp:739](src/minimal_search.cpp#L739) (probe) | ✓ |
| MVV-LVA captures | [movegen.hpp `add_capture_move`](src/movegen.hpp) | ✓ score = 1M + 10·victim − attacker |
| En passant | `add_en_passant_move` | ✓ score = 1000105 |
| Promotions | `add_promotion_move` | ✓ score = 2M + promo·100 + capture·10 |
| Castling | `add_castle_move` | ✓ score = 50000 |
| Killer moves | [minimal_search.cpp:540](src/minimal_search.cpp#L540) | ✓ 2 slots/ply, non-captures only |
| Counter-move heuristic | [minimal_search.cpp:554](src/minimal_search.cpp#L554) | ✓ piece→square table |
| History heuristic | [minimal_search.cpp:496,512,529](src/minimal_search.cpp#L496) | ✓ depth² bonus, depth² penalty, age every 3 depths |
| PV table | [pvtable.cpp](src/pvtable.cpp) | ✓ |

### Disabled / broken
- **Syzygy tablebase probe** at [minimal_search.cpp:1094](src/minimal_search.cpp#L1094) is gated by `if (false && …)` — completely disabled. Default `SyzygyPath` is `c:\TB\`.
- **LMP and aspiration windows**: implementations attempted in this branch but reverted after gauntlet showed catastrophic regression. Buggy code preserved at git tag `tier1-stack-broken`; fixes queued (see Tier 1 progress above and Next steps below).

## Current state — Evaluation

| Term | Where | Status |
|---|---|---|
| Material | `PIECE_VALUES_MG` × popcount | ✓ |
| Piece-square tables | `{PAWN,KNIGHT,BISHOP,ROOK,QUEEN,KING,KING_ENDGAME}_TABLE` | ✓ |
| Game-phase detection | `total_material ≤ 1150` → endgame | ✓ |
| Isolated pawns | `ISOLATED_PAWN_PENALTY = 10` × isolated_pawn_count | ✓ |
| Passed pawns | `PASSED_PAWN_BONUS[rank]` (0,5,10,20,40,75,125,200) | ✓ |
| Bishop pair | `BISHOP_PAIR_BONUS = 50` | ✓ |
| Rook on open / semi-open file | 10 / 5 | ✓ |
| Queen on open / semi-open file | 5 / 3 | ✓ |
| Side-to-move perspective | Negate score if Black to move | ✓ |
| Doubled pawns | `DOUBLED_PAWN_PENALTY = 20` per extra pawn on a file | ✓ |
| Tempo bonus | `TEMPO_BONUS = 10` cp added to side-to-move score | ✓ |
| **Mobility** | Constants `MOBILITY_WEIGHT_*` defined | ✗ **defined but never read** |
| **King safety / attack zone** | None | ✗ |
| **Pawn shield** | None | ✗ |
| **Backward pawns** | None | ✗ |
| **Knight outpost** | None | ✗ |

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
| 7 | **Mobility evaluation** | +30-50 | 4-8 hrs | Per-piece `popcount(attack_bb & ~own_pieces)` weighted by piece type. Constants `MOBILITY_WEIGHT_*` already exist. |
| 8 | **King safety / attack zone** | +50-100 | 1-2 days | Count enemy attackers near king + weight by piece type. Big practical Elo across many positions. |
| 9 | **Pawn shield / pawn storm** | +20-40 | 4 hrs | Bonus for pawns sheltering own king, penalty for advanced enemy pawns. |
| 10 | **Static Exchange Evaluation (SEE)** | ✅ shipped (qsearch pruning, +38 Elo); main-ordering pending | partial | Implementation: [src/see.hpp](src/see.hpp), [src/see.cpp](src/see.cpp). Currently used to skip `SEE < 0` captures in qsearch. Capture ordering in main search is still MVV-LVA only — lazy SEE in `pick_next_move` is a future optional lever. |
| 11 | **Continuation / counter-move history** | +30-50 | 1 day | Two-ply move history (`prev_move → this_move`) for ordering. Most modern engines have this. |
| 12 | **Singular extensions** | +20-40 | 1 day | Extend when TT move is much better than alternatives at reduced depth. |
| 13 | **Improving heuristic** | +10-25 | 2 hrs | Track whether eval is improving plies-back; relax LMR / tighten futility when not improving. |
| 14 | **Probcut** | +20-40 | 1 day | At high depth, do a reduced-depth search with raised beta to safely prune. |

### Tier 3 — Big payoff / big effort

| # | Item | Estimated Elo | Effort | Notes |
|---|---|---|---|---|
| 15 | **NNUE evaluation** | +200-300 | 1-2 weeks | Modern small-net evaluation. Drop in a pretrained net (e.g. nnue-pytorch) plus eval+search integration. Single biggest available upgrade. |
| 16 | **Lazy SMP** | +50-80 | 2-3 days | Multi-threaded search via shared TT. Hardware-dependent. |
| 17 | **TB probing wired up** | +5-50 | 4 hrs | The Syzygy probe code exists; the `if (false &&` gate just needs removing. Useful only with `.rtbw/.rtbz` files installed. |

### Tier 4 — Architectural / nice-to-have

| # | Item | Effort | Notes |
|---|---|---|---|
| 18 | **Migrate `S_MOVE` from 120-sq to 64-sq** | 1-2 days | ~+5-10% nps (~+10-20 Elo). Cleaner code; not a strength path. Discussed in plan memory. |
| 19 | **TT prefetch** | 2 hrs | `__builtin_prefetch` on the TT slot before move loop. Small but cheap. |
| 20 | **Magic-bitboard slider attacks** | 1-2 days | Already partially used; verify all slider attacks go through magics not ray-tracing. |
| 21 | **Smaller move encoding** | 4 hrs | If `S_MOVE` is migrated, the bit packing could shrink further. |

## Next steps (revised priority — search-shape first)

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
2. **Tune LMR.** Replace the depth ≥ 3 / move ≥ 4 / R = 1 rule with a
   compile-time 64×64 lookup table indexed by (depth, move-number).
   Reference [MTLChess src/search.zig:63](C:\Users\m_lad\Repos\MTLChess\src\search.zig).
   Target +20-40 Elo.
3. **Tier 2 #8 (king safety)** — biggest single eval improvement.
   Target +50-100 Elo.
4. **Tier 2 #7 (mobility)** — second-biggest eval improvement.
   Constants `MOBILITY_WEIGHT_*` already defined. Target +30-50 Elo.
5. **Tier 2 #11 (continuation history)** — improves move ordering,
   compounds with SEE.
6. **Revisit aspiration step (b) and LMP** (now Tier 3) once SEE and
   continuation history are in. Both deferred features regress today
   because of weak move ordering / volatile inter-iteration scores; the
   underlying plumbing was implemented correctly in both attempts.
7. Re-run calibration vs MTLChess_v0.3 after each major fix; target
   crossing **+0 Elo (≥ ~1984)**, then aim at MORA (2191). Plan to
   refresh `huginn_t1.exe` baseline once cumulative ≥ +50 Elo lands.

Tooling:
- `test_huginn_vs_t1.bat [rounds]` — regression match vs frozen t1
  baseline (commit `4f0ff0c`, tag `baseline-t1`).
- `test_huginn_calibration.bat <mtl03|mora|mtl05> [rounds]` — external
  rating ladder checks.
- `python tools/test_epd_diff.py [epd_file] [depth]` — fast EPD-based
  divergence harness (compares t1 vs current build, surfaces positions
  where the two engines pick different moves at fixed depth). Useful
  for quick search-shape diagnostics in ~2 min instead of ~30 min.

## Reference

- Estimated Elo numbers from chessprogramming.org community data — actual results vary ±50% per implementation.
- Each change should be tested vs. the previous build at 500-1000 games (10+0.1) before stacking. Document the per-feature delta in `perft/performance_tracking.txt` or a new `docs/STRENGTH_LOG.md`.
