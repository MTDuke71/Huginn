# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Huginn is a UCI-compliant chess engine written in modern C++17/20. As of
the `pure-bitboard-engine` branch (Phase 4 of the rewrite), the data flow
is **pure bitboard**: per-piece bitboards are the sole source of truth for
movegen and evaluation. Squares are indexed 0–63 (a1=0, h8=63). The old
mailbox-120 board and piece lists were removed in Phase 4.8b; piece
location is derived from the bitboards via `Position::at_sq64()`.

**Current Status (`pure-bitboard-engine` branch, 2026-05-16):**
- ✅ **Functional UCI engine**: tested with Arena and direct UCI piping
- ✅ **Baseline tag**: `baseline-t16 = 533d0b9` (= t15 + BACKLOG #9 round 7 / #2:
  **king safety, finally converted** — reformulated the in-tree term to be
  Texel-tunable (removed the ≥2-attacker gate that made it zero on quiet
  positions and stalled hand-tuning at ~0 Elo; MTLChess-recipe king-zone units²/4,
  MG-only; weights + shelter now EVAL_PARAM). The KS weights moved off their seeds
  under tuning (N 2→3, B 2→4, shelter 18→21) = genuinely tunable at last; full
  829-param tune, MSE 0.05732→0.05717. **+10.1 Elo pooled 2000g vs t15** [AMD
  +20.52@1000g LOS 99.6% / Intel −0.35@1000g neutral] — AMD-strong, Intel
  non-regressive; the first king-safety ship in the program's history (the #41
  calibration study had pegged KS as the #1 lever). Converted far above its MSE
  drop — quiet-corpus MSE under-states KS. Prior: `baseline-t15 = cdcd31f`
  (= t14 + BACKLOG #9 round 6:
  NEW feature — threats (bonus per enemy piece attacked by a cheaper/more-
  dangerous attacker, by attacker→target class, tapered); full 824-param tune
  on the 725k corpus, MSE 0.05799→0.05732 (strongest new-term signal since the
  early rounds). **+54.2 ± 14.9 Elo pooled ~1018g vs t14** [AMD +50.26@536g
  LOS 100% / Intel +58.95@482g LOS 100%], both legs same-sign positive, both
  SPRT H1-accept — the largest eval-*term* ship of the program, a clean two-
  machine decision. **Also includes the #37 illegal-bestmove guard + #36
  PV-display fix** (so t15 won't forfeit on the board-desync bug during round-7
  gauntlets). Prior: `baseline-t14 = db3f1ef` (= t13 + BACKLOG #9 round 5:
  NEW feature — rook on the relative 7th rank, gated tapered bonus (enemy king
  on back rank OR enemy pawns on the rank); full 812-param tune on the 725k
  corpus, MSE 0.05806→0.05799, fitted MG 20 / EG 24. **Shipped on a sign-split
  as a deliberate methodology exception** [AMD +17.73@1000g LOS 99.0% / Intel
  −4.52@1000g LOS 27.7%, pooled +6.6 ± 10.6] — does NOT meet the usual same-
  sign two-machine bar, taken on the strong AMD leg + theoretical soundness +
  near-zero downside of a small gated eval term; see BACKLOG #9 round 5.
  Prior: `baseline-t13 = f90fd54` (= t12 + BACKLOG #9 round 4:
  NEW features — connected (phalanx/supported) + backward pawn terms, both
  tapered; full 810-param tune on the 725k corpus, MSE 0.0583→0.0580.
  **+18.9 ± 10.7 Elo pooled 2000g vs t12** [AMD +11.82@1000g LOS 93.9% /
  Intel +26.11@1000g LOS 99.96%], both legs same-sign positive — two-machine
  ship). Prior: `baseline-t12 = 1a0b3a1` (= t11 + BACKLOG #9 round 2:
  tapered (endgame) PSTs for the 5 non-king pieces + tunable mobility, full
  ~780-param re-tune on the 725k corpus, MSE 0.0596→0.0587. **+37.4 ± 17.9 Elo
  vs t11, LOS 100%, SPRT H1-accept @764g (AMD)** — single-machine decisive
  freeze; Intel leg skipped (sign-flip impossible at this magnitude). Prior:
  `baseline-t11 = 4f091c1` (= t10 + BACKLOG #9 first Texel
  tune: material MG/EG + all 6 PSTs + king-EG fit on the Zurichess quiet-labeled
  725k corpus, MSE 0.0642→0.0596; the hand-set VICE PSTs had never been tuned.
  `value_of()` decoupled onto a fixed canonical table so ordering/material don't
  drift. **+71.4 Elo pooled 863g vs t10** [AMD +88.2@350g / Intel +59.6@512g],
  both LOS 100%, both SPRT H1-accept — the largest single ship of the program).
  Prior: `baseline-t10 = 476d33c` (= t9 + BACKLOG #35 tapered-eval
  foundation: smooth `game_phase_256()` blend replaces the hard `is_endgame`
  boolean — mg/eg sums diverge only on the king PST, flag-off byte-identical to
  t9. No new tuned values. +39.5 Elo pooled 1448g vs t9 [AMD +45.86@602g /
  Intel +35.03@846g], both LOS 100%, both SPRT H1-accept — first eval-quality
  ship of the #35 program). Prior: `baseline-t9 = ca335c2` (= t8 + perf trio
  [triangular PV + input-poll throttle + eval mirror→XOR] + PV
  repetition truncate + Priority 6 static-eval cache + Priority 7 dead
  undo-state drop; +13.90 Elo pooled vs t8, LOS 99.6%, both machines
  clear 95% — pure-speed ship, byte-identical search at equal depth).
  Prior: `baseline-t8 = b9d63f8` (= t7 + BACKLOG #15 ply-tracked
  counter-move @1500; +7.1 Elo vs t7, LOS 91%, soft ship),
  `baseline-t7 = 304f2b7` (#28 repetition fixes: halfmove-clock lookback
  + TT-safe Zarkov single-rep draw; +7.6 Elo vs t6), `baseline-t6`
  (#27 winning-rep avoidance), `baseline-t5 = 3eab266` (P1a + TT-bound
  fix + magic bitboards).
- ✅ **Comprehensive test suite**: 197 GoogleTest cases
- ✅ **Strength**: **~1818 ± 30 Elo** (10+0.1, no book, CCRL-Blitz scale) as of
  baseline-t11, June 2026 — 3-anchor pooled MLE over 600 games vs Snowy 0.2
  (1868), CDrill 2000 (1949), MTLChess v0.3 (1984). Big jump from the old
  "~1500-1700" (that figure predates the t5→t11 stack: magic bitboards, TT-bound
  fix, tapered eval, Texel tune). Note real style non-transitivity (CDrill is a
  bogey −7.5pp; MTL/MORA favorable +9pp), so treat as ~1800–1850. See BACKLOG #5
  + [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for the calibration ladder
- 🔬 **Performance**: **~3.55 Mnps** single-threaded at depth 11 from
  startpos (post-#24 magic bitboards; up from ~2.06 Mnps pre-t5)

For the **live inventory** of which search and evaluation techniques are
implemented, see [SEARCH_AND_EVAL.md "Current state" sections](SEARCH_AND_EVAL.md#current-state--search).
For **what to work on next**, see [BACKLOG.md](BACKLOG.md). This file is
the orientation guide for new sessions; the other two are the source of
truth for state and priorities.

## Common Commands

### Building the Project

The project uses CMake with presets for consistent builds across environments.

**Quick start (MSVC/Windows):**
```powershell
# Configure using CMake preset (Release build)
cmake --preset msvc-x64-release

# Build all targets
cmake --build build/msvc-x64-release --config Release

# Build specific targets
cmake --build build/msvc-x64-release --config Release --target huginn
cmake --build build/msvc-x64-release --config Release --target huginn_tests
```

**Alternative build commands:**
```powershell
# GCC/Linux builds
cmake --preset gcc-x64-release
cmake --build build/gcc-x64-release

# Clean build (when needed)
Remove-Item -Recurse -Force build/msvc-x64-release
cmake --preset msvc-x64-release
```

### Running Tests

```powershell
# Build and run all tests
cmake --build build/msvc-x64-release --config Release --target check

# Run tests manually with CTest
cd build/msvc-x64-release
ctest --config Release --output-on-failure -V

# Run specific test patterns
ctest --config Release -R "perft|position"

# Run test executable directly
.\build\msvc-x64-release\bin\Release\huginn_tests.exe
```

### Running the Chess Engine

```powershell
# Main UCI Chess Engine
.\build\msvc-x64-release\bin\Release\huginn.exe

# Mirror evaluation test (symmetric evaluation verification)
cd mirror_test
.\run_mirror_test.ps1
```

### Performance Analysis

```powershell
# Generate assembly for performance analysis
cmake --preset msvc-x64-release-asm
cmake --build build/msvc-x64-release-asm --config Release --target generate_assembly
```

## Architecture Overview

### Core Components

- **UCI Interface** ([src/uci.cpp](../src/uci.cpp), [src/uci_utils.cpp](../src/uci_utils.cpp)):
  Full UCI protocol — position setup, `go` parsing, search control, real-time `info` output
- **Position Representation** — pure bitboard:
  - [src/position.cpp](../src/position.cpp) / [src/position.hpp](../src/position.hpp):
    `Position` class with per-piece bitboards (`std::array<std::array<Bitboard, _Count>, 2>`),
    `S_MOVE` system, and incremental Zobrist + material updates. Piece-on-square
    lookup (`at_sq64()`) is derived from the bitboards; there is no mailbox
    board and no piece lists (both removed in Phase 4.8b).
- **Move Generation** — two entry points, both backed by the bitboard core:
  - [src/movegen_bb.cpp](../src/movegen_bb.cpp) / [.hpp](../src/movegen_bb.hpp):
    bitboard-based generation — **the production movegen**
  - [src/movegen.cpp](../src/movegen.cpp) / [.hpp](../src/movegen.hpp):
    legacy `S_MOVELIST` entry points that delegate to `BitboardMoveGen`;
    retained for the move-list container and tests
  - [src/attack_tables.cpp](../src/attack_tables.cpp), [src/{knight,king,pawn}_lookup_tables.cpp](../src/):
    pre-computed attack tables; sliders use magic bitboards
- **Search Engine** ([src/search.cpp](../src/search.cpp)):
  see [SEARCH_AND_EVAL.md "Current state — Search"](SEARCH_AND_EVAL.md#current-state--search)
  for the full audited inventory. High-level: negamax + alpha-beta with PVS,
  iterative deepening, TT, quiescence with SEE pruning, null-move (R=4),
  RFP, futility, razoring, LMR (`log·log` table), check extension, IID,
  killers + history + MVV-LVA + counter-move (on @1500, BACKLOG #15),
  polyglot book.
- **Evaluation**: definitions in [src/evaluation.hpp](../src/evaluation.hpp)
  (constants + masks + PSTs); the actual `evaluate()` lives in
  [search.cpp:88](../src/search.cpp#L88). Hand-crafted,
  no NNUE. Phase-aware via 3-bucket switch (no smooth taper). Terms:
  material + PSTs (separate MG/EG king table), isolated/doubled/passed
  pawns, bishop pair, rook/queen on open/semi-open files, mobility,
  tempo, insufficient-material draw. See
  [SEARCH_AND_EVAL.md "Current state — Evaluation"](SEARCH_AND_EVAL.md#current-state--evaluation)
  for the full audited inventory.

### Key Design Patterns

- **S_MOVE System**: High-performance 25-bit packed move representation with integrated scoring
- **Single Representation**: per-piece bitboards are the sole board state; piece-on-square is derived via `at_sq64()`
- **Incremental Updates**: O(1) make/unmake operations with complete state restoration
- **Type Safety**: Extensive use of C++17 type-safe enums and constexpr functions
- **Performance Focus**: Zero-copy operations, efficient bit manipulation, magic-bitboard slider attacks
- **Comprehensive Testing**: 197 automated tests covering all major components with 100% pass rate

### Move System Architecture

**Production**: S_MOVE with 25-bit packed encoding (from/to/promotion + flags)
plus an in-band `score` field used for move ordering. Make/unmake is
`MakeMove`/`TakeMove` on `Position` with O(1) incremental updates to
the per-piece bitboards, Zobrist key, and material counts.

### Bitboard Infrastructure

- **Per-piece bitboards**: `std::array<std::array<Bitboard, PieceType::_Count>, 2>`
- **Magic bitboards** for slider attacks (rook / bishop), pre-computed
  attack tables for knight / king / pawn
- **Square indexing**: 0–63 (a1=0, h8=63); see [src/square.hpp](../src/square.hpp).
  No mailbox board — piece-on-square is derived from the bitboards via `at_sq64()`

## Development Guidelines

### Code Style
- Modern C++17/20 features preferred
- Type-safe enums over integer constants
- `constexpr` for compile-time computations
- Comprehensive debug assertions when `DEBUG` is defined
- Zero-copy operations and minimal memory allocations

### Testing Requirements
- All new functionality must have corresponding tests in `test/` directory
- **Current size**: ~197 GoogleTest cases across 34 files (verify the
  pass count by running the suite — it changes as features land)
- Perft tests for move generation validation
- Position validation with FEN round-trip testing
- Evaluation symmetry tests via [test/test_evaluation_symmetry.cpp](../test/test_evaluation_symmetry.cpp)
  and the mirror harness
- Tactical sweeps via WAC300 ([test/WAC300.epd](../test/WAC300.epd))
  and LCT2 ([test/lct2.epd](../test/lct2.epd))

### Performance Considerations
- **Current NPS**: **~3.55 Mnps** single-threaded (`go depth 11` from
  startpos, 2026-05-16 measurement, post-#24 magic bitboards). Now
  ~1.5× MTLChess_v0.3 (~2.33 Mnps). Speed-side parity reached; the
  remaining strength gap is **search-tree shape and eval quality**,
  not raw speed.
- **Strength priorities** are tracked in [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md)
  by tier; per-feature Elo deltas are gauntlet-measured at 100+ games
- Assembly generation available for micro-optimization analysis (see
  `cmake --preset msvc-x64-release-asm`)

### Build System Notes
- Uses CMake 3.22+ with preset support
- Multi-compiler support (MSVC, GCC)
- Automatic GoogleTest integration via FetchContent
- Assembly generation option for performance analysis
- Aggressive MSYS2 interference prevention for pure MSVC builds

## File Organization

- `src/`: Core engine source files
- `test/`: Comprehensive test suite (197 tests across 34 files)
- `docs/`: Extensive documentation including API reference, architecture guides
- `benchmark/`: Performance tracking and benchmark results
- `mirror_test/`: Evaluation symmetry validation
- `build/`: Build output directories (created by CMake)

## Important Constants

- **Quiescence Search**: Limited to 10 plies (`MAX_QUIESCENCE_DEPTH`) - prevents stack overflow
- **Null Move Pruning**: Enhanced with R=4 reduction and minimum depth of 5 - 1.4% performance gain
- **Board Representation**: per-piece bitboards; squares indexed 0–63 (a1=0, h8=63)
- **S_MOVE System**: 25-bit packed move encoding with integrated scoring
- **UCI Protocol**: Full implementation compatible with Arena, Fritz, ChessBase, all major GUIs

## Current Engine Status (`pure-bitboard-engine` branch)

The codebase is mid-rewrite to a pure-bitboard architecture. Phase 3 +
Phases 4.1-4.2 are done (data flow is bitboard-only; `BITBOARD_ENGINE`
shim removed). Sub-phases 4.3-4.10 remain on the plan.

### What works today
1. **UCI**: position setup, search control, real-time `info` output, EPD
   parsing
2. **Search**: negamax + alpha-beta + PVS, iterative deepening, TT,
   quiescence with SEE pruning, full pruning stack (null-move, RFP,
   futility, razoring, LMR with `log·log` table), check extension, IID,
   killers + history + counter-move (on @1500) + MVV-LVA
3. **Evaluation**: hand-crafted, phase-aware (3-bucket switch), full set
   of standard pawn-structure / piece-activity / mobility terms
4. **Books / TBs**: Polyglot wired and used; Syzygy WDL probe **wired and
   active** (BACKLOG #10 closed) — fires at leaf nodes (`depth <= 1`) when
   tablebases are loaded ([search.cpp:1200](../src/search.cpp#L1200)),
   plus a root probe. WDL results are mapped to Huginn's own score scale
   (`±(MATE-1000)` = ±28000, inside `INFINITE`=30000), not Fathom's
   `TB_VALUE_*` constants. TB scores are deliberately **not** stored in
   the TT (rule50 isn't in the zobrist key)
5. **Test suite**: ~197 GoogleTest cases + WAC300 / LCT2 EPD sweeps

### Where we are vs target strength
- Calibration baseline: ~1500-1700 Elo at 10+0.1 (huginn current vs
  MTLChess_v0.3 ~1984 Elo, 0-20 score). The cumulative shipped Tier 1+2
  stack is roughly +50-100 Elo over the `t1` baseline; see
  [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for live gauntlet numbers.
- Near-term roadmap is in [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md)
  Tier 1/2/3 tables; live priorities in [BACKLOG.md](BACKLOG.md).
- Notable gaps (not yet implemented): aspiration windows, singular
  extensions, ProbCut, continuation history, SEE in main search,
  smooth tapered eval, king safety, NNUE, lazy SMP.