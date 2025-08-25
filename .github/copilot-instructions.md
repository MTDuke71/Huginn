# Huginn Chess Engine

Huginn is a C++17 chess engine with UCI interface, comprehensive test suite, and multiple demo applications. The engine uses mailbox-120 board representation, bitboards, and includes advanced move generation with parallelization support.

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively

### Bootstrap and Build Commands
- Install dependencies (Ubuntu/Debian):
  ```bash
  sudo apt-get update && sudo apt-get install -y build-essential cmake ninja-build libgtest-dev
  cd /usr/src/googletest && sudo cmake -B build -DCMAKE_BUILD_TYPE=Release && sudo cmake --build build --config Release && sudo cmake --install build
  ```
- Configure build:
  ```bash
  cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
  ```
- Build the project:
  ```bash
  cmake --build build
  ```
  - **NEVER CANCEL** - Build takes approximately 15-20 seconds. Set timeout to 120+ seconds.
- Run comprehensive test suite:
  ```bash
  ctest --test-dir build --output-on-failure -V
  ```
  - **NEVER CANCEL** - Test suite (195 tests) takes approximately 2-3 seconds. Set timeout to 60+ seconds.
- Convenience target (builds tests + runs them):
  ```bash
  cmake --build build --target check
  ```
  - **NEVER CANCEL** - Complete check takes approximately 3-5 seconds. Set timeout to 120+ seconds.

### Debug Build Configuration
- Enable debug assertions:
  ```bash
  cmake -S . -B build_debug -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_ASSERTIONS=ON
  cmake --build build_debug
  ```

## Running Applications

### UCI Chess Engine
- Main UCI engine:
  ```bash
  ./build/huginn
  ```
- Test with UCI commands:
  ```bash
  cat test_uci_commands.txt | ./build/huginn
  ```
- Expected UCI output includes:
  ```
  id name Huginn 1.0
  id author MTDuke71
  option name Hash type spin default 32 min 1 max 1024
  option name Threads type spin default 1 min 1 max 64
  option name Ponder type check default false
  uciok
  readyok
  info depth 1 nodes 1000 time 0 nps 0 score cp 31 pv b1a3
  bestmove b1a3
  ```

### Demo Applications
- Position display demo:
  ```bash
  ./build/huginn_demo
  ```
- Perft suite validation (quick test):
  ```bash
  echo "1" | ./build/perft_suite_demo
  ```
  - **NEVER CANCEL** - Perft tests can take 30+ seconds for quick mode. Set timeout to 300+ seconds.
- Move generation benchmark:
  ```bash
  ./build/parallel_benchmark
  ```
- Move decoding performance test:
  ```bash
  ./build/test_move_decoding
  ```

## Validation Requirements

### Manual Validation Steps
- **ALWAYS** test UCI interface functionality after making changes:
  ```bash
  # Test basic UCI commands
  echo -e "uci\nisready\nposition startpos\ngo movetime 1000\nquit" | ./build/huginn
  
  # Test position setup with moves
  cat test_uci_positions.txt | ./build/huginn
  ```
- **ALWAYS** run the demo applications to verify core functionality:
  ```bash
  echo | ./build/huginn_demo  # Should display chess positions
  ```
- **ALWAYS** run comprehensive test suite before completing changes:
  ```bash
  cmake --build build --target check
  ```

### Expected Test Results
- All 195 tests must pass
- Test execution time: 2-3 seconds
- No memory leaks or crashes
- UCI interface responds correctly to all standard commands

## Build Targets and Executables

### Primary Targets
- `huginn` - Main UCI chess engine
- `huginn_uci` - Alternative UCI interface binary
- `huginn_demo` - Position display demonstration
- `huginn_tests` - Complete test suite (195 tests)
- `perft_suite_demo` - Perft validation suite
- `parallel_benchmark` - Move generation performance testing
- `test_move_decoding` - Move encoding/decoding performance test

### Test Files Available
- `test_uci_commands.txt` - Basic UCI command sequence
- `test_uci_positions.txt` - UCI with position setup and moves
- `test_uci_debug.txt` - UCI debug mode testing
- `test/perftsuite.epd` - Comprehensive perft test positions

## System Requirements

### Dependencies
- C++17 compatible compiler (GCC 13.3.0+ tested)
- CMake 3.24+
- Ninja build system
- Google Test framework
- POSIX threads (automatically linked)

### Platform Support
- **Validated on**: Ubuntu 24.04 LTS (Linux kernel 6.11+)
- **Primary target**: MSYS2 UCRT64 on Windows (as documented in README.md)
- **Note**: Instructions here validated on Linux; Windows setup requires MSYS2 environment

## Architecture Overview

### Key Components
- **Board Representation**: Mailbox-120 (10x12 board with off-board sentinels)
- **Move Generation**: Enhanced move generator with parallel processing support
- **Position Management**: Incremental updates with Zobrist hashing
- **UCI Interface**: Full UCI protocol implementation with debug support
- **Validation System**: Comprehensive debug validation with integrity checking

### Important Source Files
- `src/main.cpp` - Main UCI engine entry point
- `src/uci.cpp` - UCI protocol implementation
- `src/position.cpp` - Core position management
- `src/movegen_enhanced.cpp` - Primary move generation system
- `src/debug.cpp` - Validation and debugging utilities
- `src/bitboard.cpp` - Bitboard operations and utilities

### Documentation
- Complete documentation in `docs/` directory
- API reference: `docs/API.md`
- Architecture guide: `docs/POSITION_AND_MOVEGEN_ARCHITECTURE.md`
- UCI implementation: `docs/UCI_IMPLEMENTATION.md`
- Debug validation: `docs/DEBUG_VALIDATION.md`

## Common Development Tasks

### Adding New Tests
- Place test files in `test/` directory
- Tests automatically discovered by Google Test
- Follow existing naming convention: `test_<component>.cpp`
- Include necessary headers from `src/` directory

### Debugging Issues
- Enable debug build: `-DENABLE_DEBUG_ASSERTIONS=ON`
- Use debug validation functions in `src/debug.cpp`
- Run position consistency checks after modifications
- Verify with perft suite for move generation changes

### Performance Testing
- Use `parallel_benchmark` for move generation performance
- Use `perft_suite_demo` for correctness validation
- Monitor test execution times (should remain under 5 seconds total)

## Critical Warnings

- **NEVER CANCEL** build operations - builds complete in 15-20 seconds
- **NEVER CANCEL** test operations - full test suite runs in 2-3 seconds  
- **NEVER CANCEL** perft operations - can take 30+ seconds but are essential for validation
- **ALWAYS** run full test suite before committing changes
- **ALWAYS** test UCI interface manually after modifications
- **ALWAYS** validate with demo applications to ensure core functionality works

Set appropriate timeouts: Build commands 120+ seconds, Test commands 60+ seconds, Perft commands 300+ seconds.