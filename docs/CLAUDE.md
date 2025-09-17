# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Huginn is a **tournament-ready, production-quality chess engine** written in modern C++17/20. It implements the complete UCI (Universal Chess Interface) protocol and features a sophisticated hybrid evaluation system combining bitboard analysis with mailbox position representation. 

**Current Status (HugginMain Branch):**
- ✅ **Complete Chess Engine**: 232/232 tests passing (100% pass rate)
- ✅ **Tournament Ready**: Full UCI compliance, works with Arena, Fritz, ChessBase
- ✅ **Strong Performance**: ~220k nodes/second search speed with systematic optimizations
- ✅ **Advanced Evaluation**: Multi-factor position evaluation with game phase detection
- ✅ **Proven Reliability**: Stable production branch suitable for competitive play

The engine emphasizes systematic performance optimization with documented improvements totaling ~5.6% cumulative gain through measured enhancements.

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

- **UCI Interface** (`src/uci.cpp`, `src/uci_utils.cpp`): Complete UCI protocol implementation with full search control
- **Position Representation**: Advanced hybrid system using both mailbox-120 and comprehensive bitboards
  - `src/position.cpp`: Main Position class with S_MOVE system and incremental updates
  - `src/position_bitboard.cpp`: Full bitboard infrastructure for all piece types
  - `src/board120.cpp`: 10x12 mailbox representation with off-board sentinels
- **Move Generation**: High-performance move generation with multiple optimization strategies
  - `src/movegen_enhanced.cpp`: Main move generation with MVV-LVA ordering
  - `src/movegen_bitboard.cpp`: Bitboard-based move generation
  - `src/movegen_acceleration.cpp`: Performance-optimized variants
- **Search Engine** (`src/minimal_search.cpp`): Complete alpha-beta search with:
  - Quiescence search with depth limiting (10 plies max)
  - Enhanced null move pruning (R=4 reduction) 
  - History heuristic with aging mechanism
  - Iterative deepening and principal variation tracking
- **Evaluation** (`src/evaluation.cpp`): Advanced multi-factor evaluation system:
  - Material tracking with incremental updates
  - Piece-square tables for positional assessment
  - Pawn structure analysis (isolated, doubled, passed pawns)
  - King safety evaluation and piece activity scoring
  - Game phase detection (opening/middlegame/endgame)

### Key Design Patterns

- **S_MOVE System**: High-performance 25-bit packed move representation with integrated scoring
- **Dual Representation**: Position class maintains both mailbox-120 and full bitboards in sync
- **Incremental Updates**: O(1) make/unmake operations with complete state restoration
- **Type Safety**: Extensive use of C++17 type-safe enums and constexpr functions
- **Performance Focus**: Zero-copy operations, efficient bit manipulation, O(1) piece location via piece lists
- **Comprehensive Testing**: 232 automated tests covering all major components with 100% pass rate

### Move System Architecture

The engine implements two move systems for comparison:
- **VICE Tutorial System** (`MakeMove/TakeMove`): Tutorial-based implementation, 1.55x faster with 54.6% higher node throughput
- **Huginn System** (`make_move_with_undo`): Modern vector-based approach with dynamic move history and automatic resizing

**Current Production System**: S_MOVE with 25-bit packed encoding (from/to/promotion) providing 33% memory reduction and improved cache performance.

### Bitboard Infrastructure

- **Full Bitboard Support**: All piece types have dedicated bitboards (`std::array<std::array<Bitboard, PieceType::_Count>, 2>`)
- **Efficient Operations**: Pre-computed masks, bit manipulation macros, ASCII visualization
- **Hybrid Access**: Both mailbox (simplicity) and bitboard (speed) available simultaneously

## Development Guidelines

### Code Style
- Modern C++17/20 features preferred
- Type-safe enums over integer constants
- `constexpr` for compile-time computations
- Comprehensive debug assertions when `DEBUG` is defined
- Zero-copy operations and minimal memory allocations

### Testing Requirements
- All new functionality must have corresponding tests in `test/` directory
- **Current Status**: 232/232 tests passing (100% pass rate)
- Perft tests for move generation validation
- Position validation with FEN round-trip testing
- Performance regression testing with benchmarking framework
- Extended tactical test suites for engine strength validation

### Performance Considerations
- The engine tracks performance improvements systematically through detailed benchmarking
- All optimizations are measured with standardized benchmark positions
- **Current Performance**: ~220k nodes/second search speed (tournament-competitive)
- **Documented Improvements**: ~5.6% cumulative gain through systematic optimizations:
  - Quiescence search depth limiting: +2.0%
  - Enhanced history heuristic with aging: +1.4% 
  - Enhanced null move pruning (R=4): +1.4%
  - 100% bitboard representation: +0.8%
- Assembly generation available for micro-optimization analysis

### Build System Notes
- Uses CMake 3.22+ with preset support
- Multi-compiler support (MSVC, GCC)
- Automatic GoogleTest integration via FetchContent
- Assembly generation option for performance analysis
- Aggressive MSYS2 interference prevention for pure MSVC builds

## File Organization

- `src/`: Core engine source files
- `test/`: Comprehensive test suite (230+ tests)
- `docs/`: Extensive documentation including API reference, architecture guides
- `benchmark/`: Performance tracking and benchmark results
- `mirror_test/`: Evaluation symmetry validation
- `build/`: Build output directories (created by CMake)

## Important Constants

- **Quiescence Search**: Limited to 10 plies (`MAX_QUIESCENCE_DEPTH`) - prevents stack overflow
- **Null Move Pruning**: Enhanced with R=4 reduction and minimum depth of 5 - 1.4% performance gain
- **Board Representation**: 10x12 mailbox with squares 21-98 representing a1-h8
- **S_MOVE System**: 25-bit packed move encoding with integrated scoring
- **UCI Protocol**: Full implementation compatible with Arena, Fritz, ChessBase, all major GUIs

## Current Engine Status (HugginMain Branch)

**Production-Ready Chess Engine** with tournament-level capabilities:

### Completed Major Features ✅
1. **Complete UCI Implementation**: Position setup, search control, move parsing, real-time info output
2. **Advanced Search System**: Alpha-beta with quiescence, iterative deepening, principal variation tracking
3. **Sophisticated Evaluation**: Multi-factor analysis including material, piece-square tables, pawn structure, king safety
4. **Performance Optimizations**: Systematic improvements documented with concrete metrics
5. **Comprehensive Testing**: 232/232 tests passing with full coverage of core functionality

### Performance Achievements
- **Search Speed**: ~220k nodes/second (tournament-competitive)
- **Engine Strength**: Estimated ~1800-2000 Elo rating
- **Reliability**: 100% test pass rate ensures competitive stability
- **Memory Efficiency**: 33% reduction in move storage through S_MOVE system

### Systematic Optimization Results
The project demonstrates disciplined performance improvement through measured enhancements:
1. **Quiescence search depth limiting**: +2.0% performance gain + stack safety
2. **Enhanced history heuristic with aging**: +1.4% improvement through better move ordering  
3. **Enhanced null move pruning (R=4)**: +1.4% gain via aggressive branch elimination
4. **100% bitboard representation**: +0.8% improvement + foundation for future optimizations

**Total Documented Improvement**: ~5.6% cumulative performance gain with maintained functional correctness.