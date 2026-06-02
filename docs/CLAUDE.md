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
- ✅ **Baseline tag**: `baseline-t9 = ca335c2` (= t8 + perf trio
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
- ⚠️ **Strength**: ~1500-1700 Elo at 10+0.1 vs MTLChess_v0.3 (~1984 Elo);
  see [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for the live calibration
  ladder
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
4. **Books / TBs**: Polyglot wired and used; Syzygy probe code present
   but gated off (`if (false && …)` at search.cpp:1218)
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