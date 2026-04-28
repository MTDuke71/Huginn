# Search & Evaluation — current state and roadmap

Snapshot of what's implemented in Huginn 2.0 (post Phase 4) and what's
planned. Reality check on each item is anchored to file:line so this
stays accurate as the code evolves.

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
| Razoring | [minimal_search.cpp:1208](src/minimal_search.cpp#L1208) | ✗ defined behind `#ifdef USE_RAZORING`, NOT enabled |
| Multi-cut pruning | [minimal_search.cpp:1263](src/minimal_search.cpp#L1263) | ✗ defined behind `#ifdef USE_MULTI_CUT`, NOT enabled |
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
- **Syzygy tablebase probe** at [minimal_search.cpp:1094](src/minimal_search.cpp#L1094) is gated by `if (false && …)` — completely disabled. Default `SyzygyPath` is `d:\TB\` which doesn't exist on this dev machine.

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
| **Doubled pawns** | Constant `DOUBLED_PAWN_PENALTY = 20` defined at [evaluation.hpp:109](src/evaluation.hpp#L109) | ✗ **defined but never read** |
| **Mobility** | Constants `MOBILITY_WEIGHT_*` defined | ✗ **defined but never read** |
| **King safety / attack zone** | None | ✗ |
| **Pawn shield** | None | ✗ |
| **Backward pawns** | None | ✗ |
| **Tempo bonus** | None | ✗ |
| **Knight outpost** | None | ✗ |

## Planned improvements (ranked by Elo/effort ratio)

Estimates are rough — small numbers (5-20 Elo) are easy to validate at
500 games per change, larger ones (50-100) need 1k-2k games for tight
CIs. Effort is "from scratch" — not counting tuning iterations.

### Tier 1 — High Elo / low effort

| # | Item | Estimated Elo | Effort | Notes |
|---|---|---|---|---|
| 1 | **Aspiration windows** | +30-50 | 1-2 hrs | Start each ID iteration with a narrow window around the previous score; on fail, widen and re-search. Standard in every modern engine. |
| 2 | **Reverse futility / static null-move pruning** | +20-40 | <1 hr | At depth ≤ 6, if `eval − margin > beta`, return beta. One of the cheapest wins. |
| 3 | **Late move pruning (LMP)** | +20-40 | 1 hr | At low depth, skip quiet moves after `N + depth²` tries. Pairs well with LMR. |
| 4 | **Wire up doubled-pawn evaluation** | +10-20 | 30 min | Constant exists; just count pawns per file via `popcount(piece_bitboards[c][Pawn] & FILE_MASKS[f]) > 1` and apply penalty. |
| 5 | **Tempo bonus** | +5-15 | 5 min | Add ~10cp to the side-to-move score. Pure free Elo. |
| 6 | **Enable razoring** | +10-25 | 1 hr (test+tune) | Already coded behind `#ifdef USE_RAZORING`. Just turn on, validate, and tune the 400cp margin if needed. |

### Tier 2 — Medium Elo / medium effort

| # | Item | Estimated Elo | Effort | Notes |
|---|---|---|---|---|
| 7 | **Mobility evaluation** | +30-50 | 4-8 hrs | Per-piece `popcount(attack_bb & ~own_pieces)` weighted by piece type. Constants `MOBILITY_WEIGHT_*` already exist. |
| 8 | **King safety / attack zone** | +50-100 | 1-2 days | Count enemy attackers near king + weight by piece type. Big practical Elo across many positions. |
| 9 | **Pawn shield / pawn storm** | +20-40 | 4 hrs | Bonus for pawns sheltering own king, penalty for advanced enemy pawns. |
| 10 | **Static Exchange Evaluation (SEE)** | +30-50 | 1 day | Used in qsearch to prune losing captures, and in main search ordering to push bad-trade captures down. |
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

## Suggested execution order

For maximum compounding Elo with minimum risk:

1. **Tier 1 items 1, 2, 3, 5** in one session (aspiration + reverse futility + LMP + tempo). Test 500 games each vs current at end of session. Cumulative target: +75-150 Elo.
2. **Tier 1 items 4, 6** (doubled pawns + enable razoring). +20-50 Elo. Half a session.
3. **Tier 2 item 8** (king safety) — biggest single eval improvement. Full session.
4. **Tier 2 item 7** (mobility) — second-biggest eval improvement. Full session.
5. **Tier 2 item 10** (SEE) and **item 11** (continuation history). Both substantial; one each.
6. After all tier-1 and most of tier-2, **target depth-9-10 against MTLChess at 10+0.1**. If still losing, the gap to ~2100 is search-tree-shape, not eval, and **NNUE (item 15)** becomes the next move.

## Reference

- Estimated Elo numbers from chessprogramming.org community data — actual results vary ±50% per implementation.
- Each change should be tested vs. the previous build at 500-1000 games (10+0.1) before stacking. Document the per-feature delta in `perft/performance_tracking.txt` or a new `docs/STRENGTH_LOG.md`.
