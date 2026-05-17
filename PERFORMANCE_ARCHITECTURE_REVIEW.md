# Huginn Performance Architecture Review

Original review: 2026-05-15 (GPT)
Last status update: 2026-05-17

Scope reviewed: architecture docs in [docs/](docs/), especially [docs/POSITION_AND_MOVEGEN_ARCHITECTURE.md](docs/POSITION_AND_MOVEGEN_ARCHITECTURE.md), [docs/BITBOARD_IMPLEMENTATION.md](docs/BITBOARD_IMPLEMENTATION.md), [docs/MOVEGEN_COMPARISON.md](docs/MOVEGEN_COMPARISON.md), [docs/SEARCH_AND_EVAL.md](docs/SEARCH_AND_EVAL.md), [docs/BACKLOG.md](docs/BACKLOG.md), and the hot engine code in [src/](src/).

## Completion Status

Tracking the priorities below against actual ships. Updated alongside each
shipped item.

| Priority | Status | Commit | Bench Δ | Measured Elo |
|---|---|---|---|---|
| 1. TT bound classification fix | ✅ SHIPPED | `7d11f23` (#23) | nodes @d11 −76% (5.06M → 1.20M) | +24.4 / 400g pooled, LOS >>95% |
| 2. Real magic bitboards | ✅ SHIPPED | `3eab266` (#24) | NPS +52% (2.33 → 3.55 Mnps) | +77.7 / 400g pooled vs t4 (combined w/ #23+P1a), LOS >>99.99% |
| 3. `board64[64]` piece cache | ✅ SHIPPED (code), AMD pool pending | `e61f6e5` (#26) | NPS +12% (3.55 → 3.98 Mnps) | Intel 200g: +12.17 ± 31.80, LOS 77%. Pool with AMD for LOS-95%. |
| 4. Shrink + de-eager move list | ⏳ pending | — | — | — |
| 5. Staged move picker | ⏳ pending | — | — | — |
| 6. Cache static eval per node | ⏳ pending | — | — | (cheapest remaining; ~+5-15 Elo expected) |
| 7. Remove dead undo-state writes | ⏳ pending | — | — | (small; can bundle with #6) |
| 8. Gate TT stats counters behind flag | ⏳ pending | — | — | — |
| 9. TT cluster layout + generations | ⏳ pending | — | — | (after #4-7) |
| 10. Incremental PST/phase + pawn hash | ⏳ pending | — | — | (largest remaining speed work) |

**Shipped to date: 3 of 10 priorities**, contributing the bulk of the
~+78 Elo t4 → t5 jump plus pending #26 result. Cumulative wall-clock
to depth 11 startpos: **2285 ms → 234 ms (9.8× faster)**, NPS
**2.21 → 3.98 Mnps (+80%)**.

Related deferral: [BACKLOG #27](docs/BACKLOG.md) — early-queen-PV
eval-quality issue identified during post-#26 bench review. Filed as
low-priority on the grounds that the symptom is search-depth-bound,
not an eval-design flaw; revisit if a future depth ship leaves the
same PV unchanged. Documented in [BACKLOG #27](docs/BACKLOG.md).

## Executive Summary

Brutal version: Huginn is not losing most of its speed because C++ is slow or because the compiler flags are weak. It is spending too much time in avoidable hot-path infrastructure: repeated piece lookup by bitboard scan, runtime ray-walking slider attacks, oversized/eager move lists, repeated static evals at the same node, and a transposition-table bound classification bug that appears to cripple TT pruning.

The repo docs already diagnosed that raw startpos NPS is not the whole problem. I agree with that. The larger strength gap is search-tree shape. But the code still has several accuracy-safe speed wins that should be taken before adding more pruning or eval complexity.

Highest confidence priorities:

1. Fix TT bound classification in [src/search.cpp](src/search.cpp#L1586-L1610). As written, exact and fail-high nodes are likely stored as upper bounds.
2. Replace runtime ray-walking sliders in [src/bitboard.cpp](src/bitboard.cpp#L88-L156) with real magic-bitboard or PEXT lookup tables. The docs currently claim magic bitboards, but the code does not implement them.
3. Add a direct `Piece board64[64]` or equivalent piece-on-square cache. `Position::at_sq64()` currently scans piece bitboards in [src/position.hpp](src/position.hpp#L175-L187).
4. Stop building and scoring full 2 KB move lists before knowing whether the TT move or first capture cuts. Move toward a staged move picker and compact move representation.
5. Cache static eval inside `AlphaBeta()` and remove dead undo-state writes. These are boring changes, but they are exactly the kind that speed an engine without changing chess decisions.
6. Only after the above should you re-attempt aspiration windows, LMP, SEE main-search demotion, or more hand-eval terms.

## The Big Architecture Mismatch

The documentation overstates how far the engine has moved toward a fast pure-bitboard architecture.

- [docs/MOVEGEN_COMPARISON.md](docs/MOVEGEN_COMPARISON.md) says sliders use magic bitboards and are runtime-equivalent to MTLChess. The actual `bishop_attacks()` and `rook_attacks()` in [src/bitboard.cpp](src/bitboard.cpp#L139-L156) call `generate_ray_attacks()` four times, and that function loops square by square with bounds checks and occupancy branches.
- [docs/CLAUDE.md](docs/CLAUDE.md#L130-L150) still talks about mailbox parallel representation, piece lists, and O(1) piece location via piece lists. The current architecture has no `board[120]` and no piece lists. `at()` derives pieces from bitboards.
- [docs/SEARCH_AND_EVAL.md](docs/SEARCH_AND_EVAL.md#L351-L375) lists TT as implemented with exact/lower/upper bounds. Structurally, yes. Behaviorally, the store-side node-type logic looks wrong.

That matters because you can make bad engineering decisions from stale docs. Right now the docs make the slider and square-lookup costs look solved when they are not.

## Priority 1: Fix TT Bound Classification

> **✅ SHIPPED** in commit `7d11f23` (BACKLOG #23, 2026-05-15).
> Bench: depth-11 startpos nodes 5.06M → 1.20M (−76%), time 2285ms → 516ms.
> Gauntlet: pooled 400g vs t4 baseline, **+24.4 Elo, LOS >>95%**.
> See [BACKLOG #23](docs/BACKLOG.md) for full details + ship rationale.

Evidence: [src/search.cpp](src/search.cpp#L1586-L1592)

```cpp
uint8_t node_type;
if (best_score <= alpha) {
    node_type = TTEntry::UPPER_BOUND;
} else if (best_score >= beta) {
    node_type = TTEntry::LOWER_BOUND;
} else {
    node_type = TTEntry::EXACT;
}
```

`alpha` has already been mutated during the move loop. If a move improves alpha, the code sets `alpha = score`. At the end, `best_score <= alpha` is then true. If the move caused a beta cutoff, `alpha` was also raised to that score before the break, so the first branch still wins.

Result: most improved nodes, including exact PV nodes and beta cutoffs, are probably stored as `UPPER_BOUND`. That means the TT can still provide a best move for ordering, but it loses most of its pruning power. This is a huge deal.

Recommended fix:

```cpp
const int original_alpha = alpha;

// ... search moves, mutate alpha as usual ...

uint8_t node_type;
if (best_score <= original_alpha) {
    node_type = TTEntry::UPPER_BOUND;
} else if (best_score >= beta) {
    node_type = TTEntry::LOWER_BOUND;
} else {
    node_type = TTEntry::EXACT;
}
```

Why this is accuracy-safe: it makes TT metadata match alpha-beta semantics. It should not remove search. It should reduce repeat work and make TT cutoffs legitimate.

Validation:

- Add counters for TT stores by bound type and TT usable cutoffs by bound type.
- Before the fix, expect suspiciously high upper-bound stores. After the fix, exact/lower entries should become common.
- Run perft to confirm movegen unchanged.
- Run fixed-depth searches on tactical EPDs and startpos. Expect fewer nodes at same depth, possibly changed PVs because the TT is finally doing its job.
- Gauntlet it. A TT behavior change can expose latent bugs, but this one is too fundamental to ignore.

## Priority 2: Implement Real Slider Attack Tables

> **✅ SHIPPED** in commit `3eab266` (BACKLOG #24, 2026-05-16).
> Plain magic with fixed shifts (rook 52, bishop 55), ~2.25 MB tables,
> magics found at init from a deterministic PRNG seed + exhaustive
> verifier against ray-walk reference.
> Bench: NPS 2.33 → 3.55 Mnps (+52%), time-to-d11 516ms → 263ms.
> Gauntlet: combined with #23+P1a, pooled 400g vs baseline-t4 = **+77.7 Elo,
> LOS >>99.99%** — promoted to `baseline-t5 = 3eab266` (BACKLOG #25).
> Dead-code follow-up `704d84c` removed `generate_ray_attacks` and
> direction constants (no remaining callers).

Evidence: [src/bitboard.cpp](src/bitboard.cpp#L88-L156)

The code uses runtime ray walking:

- Compute file/rank.
- Switch on direction.
- Step up to 7 squares.
- Bounds-check every step.
- Branch when occupancy blocks the ray.

This is called from:

- Slider move generation in [src/movegen_bb.cpp](src/movegen_bb.cpp#L269-L343)
- Attack detection in [src/attack_detection.cpp](src/attack_detection.cpp#L44-L53)
- SEE in [src/see.cpp](src/see.cpp#L53-L64)
- Mobility evaluation in [src/search.cpp](src/search.cpp#L286-L298)

So every search node pays this cost in multiple ways. The docs call these magic bitboards, but they are not magic bitboards.

Recommended implementation choices:

- Best portable baseline: precomputed magic-bitboard attack tables for rook and bishop.
- If targeting modern x64 and MSVC/GCC with BMI2: consider PEXT-based attacks with a compile-time or startup-generated table and a runtime capability fallback.
- Keep the public API as `rook_attacks(square, occupancy)` and `bishop_attacks(square, occupancy)` so this is an internal replacement.

Why this is accuracy-safe: the returned attack masks should be bit-identical. This is a pure implementation swap.

Validation:

- Exhaustive or high-volume random comparison against the current ray walker.
- Perft with known positions.
- Bench attack detection, movegen, SEE-heavy qsearch, and fixed-depth search.

Expected payoff: high. This is one of the rare speed changes that improves movegen, king safety checks, SEE, and eval at once.

## Priority 3: Add Direct Piece-On-Square Storage

> **✅ SHIPPED (code-side)** in commit `e61f6e5` (BACKLOG #26, 2026-05-17).
> Added `std::array<Piece, 64> board64;` to Position; every mutator
> (`set`, `move_piece`, `clear_piece`, `add_piece`) updates it in
> lock-step with the bitboards. `at_sq64()` is now a single array load.
> Bench: NPS 3.55 → 3.98 Mnps (+12%), time-to-d11 263ms → 234ms.
> Gauntlet vs t5: 200g Intel running 2026-05-17 afternoon.
> See [BACKLOG #26](docs/BACKLOG.md) for full details.

Evidence: [src/position.hpp](src/position.hpp#L175-L187)

`at_sq64()` does this:

```cpp
uint64_t bit = 1ULL << s64;
if ((occupied_bitboard & bit) == 0) return Piece::None;
int c = (color_bitboards[0] & bit) ? 0 : 1;
for (int t = int(PieceType::Pawn); t <= int(PieceType::King); ++t) {
    if (piece_bitboards[c][t] & bit) {
        return make_piece(Color(c), PieceType(t));
    }
}
```

This is not an O(1) array lookup. It is a small bitboard scan, and it sits in hot paths:

- Capture tagging in [src/movegen_bb.cpp](src/movegen_bb.cpp#L60-L61), [src/movegen_bb.cpp](src/movegen_bb.cpp#L128-L130), and all slider generators.
- MVV-LVA scoring in [src/movegen.hpp](src/movegen.hpp#L33-L41).
- `MakeMove()` and `TakeMove()` in [src/position.cpp](src/position.cpp#L274-L370).
- TT move validation and history scoring in [src/search.cpp](src/search.cpp#L854-L963).

Recommended fix: add a 64-square piece cache:

```cpp
std::array<Piece, 64> board64;
```

Update it inside `set()`, `add_piece()`, `clear_piece()`, and `move_piece()` alongside the bitboards. Then make `at_sq64()` a direct array load, and make `at(sq120)` only do the 120-to-64 projection plus the array load.

Do not frame this as reverting to mailbox-first architecture. The bitboards can remain the source for set operations, attack masks, occupancy, and iteration. This is just a piece-on-square cache, which almost every fast bitboard engine keeps because captures and make/unmake constantly need the occupant.

Why this is accuracy-safe: if kept in sync, it does not change any move or eval result.

Validation:

- Add debug-only consistency checks: for every occupied bit, `board64[sq]` matches exactly one piece bitboard; for every non-empty board square, the corresponding bit is set.
- Run perft and make/take stress tests.
- Specifically test promotions, en passant, castling, and illegal-move self-undo.

Expected payoff: high. It removes repeated bitboard scans from movegen and make/unmake.

## Priority 4: Shrink and De-Eager the Move List

Evidence: [src/move.hpp](src/move.hpp#L91-L118), [src/movegen.hpp](src/movegen.hpp#L12-L27), [src/search.cpp](src/search.cpp#L838-L985)

`S_MOVE` is 8 bytes because it stores both the packed move and an `int score`. `S_MOVELIST` stores 256 of them, so every move list is 2 KB on the stack. Search creates these lists constantly.

The engine also scores captures eagerly in `S_MOVELIST::add_capture_move()`, then scores moves again in `pick_next_move()`. That is wasted work whenever a TT move or early capture cuts off the node.

Recommended direction:

1. Make the move payload smaller. Long term, move to 64-square from/to and a compact flag enum. A 16-bit move is possible; a 32-bit move is still fine and halves the current move-list payload if the score is separated.
2. Store scores in a parallel array inside the move picker, not inside every `S_MOVE`.
3. Remove generation-time MVV-LVA scoring. Generate bare moves and score lazily when a phase needs them.

Why this is accuracy-safe: the searched move set does not change. Only storage and ordering mechanics change.

Validation:

- Perft must be identical.
- Fixed-depth node counts may change because order changes. Time-to-depth should improve.
- Use WAC/LCT2 to ensure tactical PVs do not regress.

## Priority 5: Replace Full-List Ordering With a Staged Move Picker

Current shape: generate all pseudo-legal moves, then `pick_next_move()` scores the whole list on the first call and selection-sorts one best move at a time.

That is simple, but it is wasteful at cut nodes. The most common successful alpha-beta node does not need a fully generated and fully scored list.

Recommended stages:

1. TT move, validated cheaply.
2. Winning and equal captures.
3. Promotions.
4. Killer moves.
5. High-history quiets.
6. Losing captures.
7. Remaining quiets.

Do not use this as pruning at first. Use it only as ordering and lazy generation. Once the staged picker is correct, LMP and SEE demotion become safer because the engine has better semantics for "how many quiets have really been searched" and "is this capture actually bad enough to delay."

Why this is mostly accuracy-safe: ordering changes can alter practical search under time limits, but a full-depth complete search sees the same legal moves.

Validation:

- Perft unchanged.
- Fixed-depth best moves should be stable or better.
- Track fail-high-first percentage before and after. If ordering improves, `fhf / fh` should improve.

## Priority 6: Cache Static Eval Per Node

Evidence: [src/search.cpp](src/search.cpp#L1263-L1365)

`AlphaBeta()` can compute static eval multiple times at the same node:

- Reverse futility calls `evalPosition(pos)`.
- Futility pruning calls `evalPosition(pos)` again.
- Razoring calls `evalPosition(pos)` again.

Evaluation is not cheap. It loops pieces, computes pawn structure, open files, bishop pair, and mobility. Mobility also calls slider attacks.

Recommended fix: local lazy static eval.

```cpp
int static_eval = 0;
bool has_static_eval = false;
auto get_static_eval = [&]() {
    if (!has_static_eval) {
        static_eval = evalPosition(pos);
        has_static_eval = true;
    }
    return static_eval;
};
```

Then all pruning blocks at that node use `get_static_eval()`.

Why this is accuracy-safe: identical value, fewer calls.

Validation: fixed-depth PV should be unchanged except for timing-sensitive TT/order effects. Node count should be basically unchanged; wall time should improve.

## Priority 7: Remove Dead Undo-State Work

Evidence: [src/position.hpp](src/position.hpp#L55-L65), [src/position.cpp](src/position.cpp#L274-L311), [src/position.cpp](src/position.cpp#L430-L526)

`S_UNDO` stores `king_sq_backup` and `material_score_backup`, and `MakeMove()` calls `save_derived_state(undo)`. But `TakeMove()` does not call `restore_derived_state()`. It restores state through piece operations and direct king updates.

That means every searched move writes backup arrays that appear unused.

Recommended fix:

- Confirm no caller uses `restore_derived_state()`.
- Remove the backup fields and `save_derived_state()` call, or gate them behind debug if they are only for diagnostics.
- Keep tests focused on promotions, captures, en passant, castling, and illegal self-undo.

Why this is accuracy-safe: removing unused state writes should not change behavior.

Expected payoff: medium-small, but extremely low conceptual risk and helps cache footprint.

## Priority 8: Stop Counting TT Stats in the Hot Path by Default

Evidence: [src/transposition_table.hpp](src/transposition_table.hpp#L143-L157), [src/transposition_table.hpp](src/transposition_table.hpp#L181-L196)

Every TT probe increments `hits` or `misses`. Every store increments `writes`. That is useful during diagnostics, but it is hot-path mutation on every node.

Recommended fix:

- Compile stats behind `ENABLE_TT_STATS` or a runtime debug flag.
- For tournament/release builds, keep probes and stores as close to read/compare/write-entry as possible.

Why this is accuracy-safe: stats do not affect chess decisions.

Expected payoff: small but free. It also matters more once multithreading exists, because shared counters become cache-line contention.

## Priority 9: Improve TT Table Architecture After the Bound Fix

The current TT is one entry per index with depth-preferred replacement in [src/transposition_table.hpp](src/transposition_table.hpp#L99-L140). That is simple, but strong engines usually use clusters and generations.

Recommended after fixing node types:

- Use 4-entry clusters per cache line.
- Add generation/age so shallow current-search entries can replace stale deep entries when appropriate.
- Track usable hit rates by bound type, not just raw hit/miss.
- Consider storing partial key signatures if you want denser clusters, but do not sacrifice collision safety casually.

Why this is accuracy-safe if done conservatively: replacement policy changes what is cached, not what is legal or evaluated. It can still change search behavior under time controls, so gauntlet it.

## Priority 10: Incremental Eval and Pawn Hash

Evidence: [src/search.cpp](src/search.cpp#L88-L306)

The engine tracks material incrementally, but `evaluate()` still recomputes material plus PST by iterating all pieces. It also recomputes pawn structure every eval, and mobility every eval.

Recommended sequence:

1. Incremental PST score and phase. Store in `Position` and undo state. Use this as the base eval.
2. Pawn hash for isolated/doubled/passed pawn terms. Pawn structure changes less often than positions are evaluated.
3. Keep mobility non-incremental initially, but make it cheaper after real slider tables land.
4. Only then consider smooth tapered eval. That is an accuracy feature, not a pure speed feature.

Why this can be accuracy-safe: exact incremental maintenance can return the same eval. The risk is implementation bugs, not chess theory.

Validation:

- Add eval equality tests comparing incremental eval to the old full recomputation over random legal move sequences.
- Include make/take random walks so undo bugs surface.
- Keep mirror symmetry tests.

## Pruning and Search-Shape Feedback

The backlog shows a consistent pattern: pruning/order features often reduce nodes but fail gauntlets. That is a warning. Do not keep adding pruning until ordering and TT correctness are healthier.

Specific notes:

- Main-search SEE demotion already measured as fewer nodes but negative/neutral Elo in [docs/BACKLOG.md](docs/BACKLOG.md#L1048-L1154). Do not reattempt the same `SEE < 0` demotion unchanged. If revisited, use a more conservative threshold like `SEE < -100`, exempt checks, and measure.
- LMP reduced nodes but remained negative in [docs/BACKLOG.md](docs/BACKLOG.md#L1156-L1224). That suggests the first quiet moves are still not reliable enough. Staged ordering and TT fix should come first.
- Aspiration step b is not just a parameter problem. [docs/BACKLOG.md](docs/BACKLOG.md#L1226-L1275) correctly points at score volatility and ordering. Fix TT bound types before drawing more conclusions.
- Counter-move and continuation history attempts being neutral does not mean all history is useless. It may mean current score slots and ordering phases are too crude.

Accuracy-safe speed work should be mostly representation, lookup, caching, and TT correctness. Risky speed work is pruning. Treat those categories differently.

## Compiler and Build Notes

The release flags in [CMakeLists.txt](CMakeLists.txt#L114-L159) are already aggressive. More flags will not fix the architecture costs above.

Useful build-side work:

- Add a PGO preset. Chess search is branch-heavy, and PGO can help the compiler put hot paths where they belong.
- Keep a separate benchmark/tournament preset if you want to test `/guard:cf` and `/GS` overhead. Do not confuse security-hardening choices with engine architecture.
- Do not expect `/arch:AVX2` to matter much until the code actually uses vector-friendly data paths. Bitboard engines usually win more from table layout, branch behavior, and cache footprint than from broad SIMD.

## What I Would Not Spend Time On Yet

- More hand-written eval features. The backlog already shows several eval additions causing regression. Search depth and correctness are more urgent.
- Re-tuning null-move, LMR, LMP, aspiration, and futility all at once. You will not know which lever moved Elo.
- Micro-optimizing `pop_lsb()` or `setBit()` before fixing ray-walk sliders and piece lookup.
- Adding a bigger opening book and calling it engine strength.
- Treating a 20-game result as signal. The docs already contain examples where noise and opponent mismatch misled conclusions.

## Suggested Work Order

1. Fix TT node-type classification with `original_alpha`.
2. Add TT bound-type/cut counters, behind a stats flag.
3. Implement real rook/bishop attack lookup tables and update docs to stop claiming magic until it is true.
4. Add `board64` piece cache and make `at_sq64()` an array load.
5. Cache static eval inside `AlphaBeta()`.
6. Remove unused undo backups and other dead hot-path state writes.
7. Split move payload from ordering score; stop eager generation-time scoring.
8. Build a staged move picker without adding new pruning.
9. Improve TT layout with clusters/generations.
10. Add incremental PST/phase and pawn hash.
11. Revisit LMP, aspiration step b, and main-search SEE only after the above.

## Validation Plan

For each speed change, record both speed and chess safety:

- Correctness: perft suite, FEN round-trip, make/take random walk, hash consistency, eval symmetry.
- Fixed-depth speed: startpos depth 10/11, Kiwipete, tactical middlegame, endgame.
- Search quality: WAC300 and LCT2 EPD sweeps.
- TT health: bound-type stores, usable TT cutoffs, exact hits, lower-bound cutoffs, upper-bound cutoffs.
- Gauntlet: at least 200 games for small changes, 400+ when noise is high.

Use the same benchmark positions before and after. Track nodes and wall time separately. A change that lowers nodes but loses Elo is not a win; this repo has already seen that pattern repeatedly.

## Final Take

Claude helped build a lot of useful scaffolding here, but the architecture is still carrying tutorial-engine habits in the hot path. The most honest assessment is this: Huginn is close enough to be worth optimizing seriously, but not by sprinkling more pruning on top. Fix the TT metadata, make slider attacks actually table-driven, stop scanning bitboards for square occupants, and make move ordering lazy/staged. Those are the changes most likely to increase speed without giving back accuracy.