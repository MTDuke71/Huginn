# Hug> **🎉 Version 1.1** - Major release featuring hybrid evaluation and dramatically improved chess playing strength  

## Huginn: Odin's Thought, in Every Move

![Huginn Logo]### Option B: Use vcpkg for GoogleTest

```powershell
# Install vcpkg (if not already installed)
vcpkg install gtest:x64-windows
```

Then in `CMakeLists.txt`:

```cmake
find_package(GTest CONFIG REQUIRED)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)  # or GTest::gtest_main if you omit test_main.cpp
```uginn.png)

## Acknowledgments

Huginn was inspired by and built upon the foundation of the **VICE Chess Engine** by Richard Allbert (Bluefever Software). VICE (Video Instructional Chess Engine) provided an excellent educational framework for understanding chess engine programming fundamentals, including board representation, move generation, and search algorithms. While Huginn has evolved significantly with its own hybrid evaluation system, modern C++ architecture, and advanced optimizations, we gratefully acknowledge VICE as the foundational inspiration that made this project possible.

You can find the original VICE engine and instructional video series on YouTube: [VICE Chess Engine Playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)


## MSVC Build Instructions (Visual Studio 2022)

### Prerequisites
1. **Visual Studio 2022** with C++ workload installed
2. **CMake 3.22+** (included with Visual Studio or install separately)
3. **Git** for version control

### Quick Start
```powershell
# Open a PowerShell terminal or Visual Studio Developer Command Prompt
cd d:\repos\Huginn_New\Huginn

# Configure using CMake preset (Release build)
cmake --preset msvc-x64-release

# Build all targets
cmake --build build/msvc-x64-release --config Release

# Or build specific targets
cmake --build build/msvc-x64-release --config Release --target huginn
cmake --build build/msvc-x64-release --config Release --target huginn_tests
```

## Alternative: Use VS Code

1. Open the project folder in VS Code
2. Install the **CMake Tools** extension
3. Select the `msvc-x64-release` preset
4. Build using **Ctrl+Shift+P** → "CMake: Build"

### Run the Programs

```powershell
# Main UCI Chess Engine
.\build\msvc-x64-release\bin\Release\huginn.exe

# Run all tests
.\build\msvc-x64-release\bin\Release\huginn_tests.exe

# Mirror evaluation test (symmetric evaluation verification)
cd mirror_test
.\run_mirror_test.ps1
```

## Features

### UCI Chess Engine

- **Complete UCI Protocol**: Compatible with Arena, Fritz, ChessBase, and all UCI chess GUIs
- **Hybrid Evaluation System**: Advanced evaluation combining Engine2's bitboard analysis with mailbox position representation
- **Single-threaded Alpha-Beta Search**: Robust search with quiescence, iterative deepening, and move ordering
- **Realistic Position Assessment**: Accurate centipawn evaluations and mate detection (~70k-90k nps)
- **Enhanced Move Encoding**: Proper UCI move output (d2d4, e2e4) replacing legacy "0000" move bugs
- **Game Phase Detection**: Opening/middlegame/endgame evaluation adjustments for optimal play strength

### Chess Engine Architecture

- **Hybrid Evaluation Engine**: Combines advanced bitboard evaluation techniques with mailbox position representation
- **Comprehensive Position Analysis**: Material counting, pawn structure, piece activity, king safety, and mobility evaluation
- **Game Phase Awareness**: Dynamic evaluation adjustments based on opening/middlegame/endgame detection
- **Single-threaded Search**: Alpha-beta pruning with quiescence search, iterative deepening, and sophisticated move ordering
- **Principal Variation Collection**: Complete PV lines showing tactical sequences and candidate moves
- **Mailbox-120 Board Representation**: 10x12 board with off-board sentinels for efficient boundary checking
- **Modern C++17**: Type-safe enums, constexpr functions, and std::array for performance
- **Bitboard Support**: 64-bit bitboards with visualization and manipulation macros
- **Piece List Optimization**: Fast piece iteration without board scanning (up to 10 pieces per type)

### Move System

- **Dual Move Implementation**: Both VICE Tutorial (MakeMove/TakeMove) and Huginn (make_move_with_undo) systems
- **VICE Tutorial Videos #41-42**: Complete implementation of standard chess programming move making/unmaking
- **Performance Advantage**: VICE approach is 1.55x faster with 54.6% higher node throughput
- **Dynamic Move History**: Vector-based move history with automatic resizing and minimal memory overhead
- **Comprehensive Move Encoding**: From/to/promotion packed into single integer
- **Attack Detection**: Fast square attack detection for all piece types with proper blocking
- **Castling Rights**: Centralized constants and utility functions
- **Position Hashing**: Zobrist hashing for transposition tables

### Development Tools

- **Debug Assertions**: Detailed error reporting with file/line information when DEBUG defined
- **Comprehensive Testing**: 183+ test cases covering all major components including FEN generation
- **Position Validation**: Multi-layer integrity checking with castling validation and FEN round-trip verification
- **Bitboard Visualization**: ASCII board display with rank/file borders

## Documentation

Complete documentation is available in the [`docs/`](docs/) directory:

- **[📚 Documentation Index](docs/README.md)** - Complete guide to all documentation
- **[🔧 API Reference](docs/API.md)** - Complete API and usage guide  
- **[🏗️ Architecture](docs/POSITION_AND_MOVEGEN_ARCHITECTURE.md)** - Core system design
- **[📊 Performance Tracking](docs/PERFORMANCE_TRACKING.md)** - Optimization results and tracking
- **[🎯 VICE Tutorial Implementation](docs/VICE_TUTORIAL_41_42_IMPLEMENTATION.md)** - Complete VICE Videos #41-42 implementation with performance analysis
- **[✅ Validation Systems](docs/DEBUG_VALIDATION.md)** - Testing and validation documentation

### Quick Links

- [Performance Analysis](docs/DECODE_MOVE_ANALYSIS.md) - Why micro-optimizations translate to real performance
- [Optimization History](docs/DECODE_MOVE_REMOVAL.md) - Recent modernization improvements  
- [Implementation Details](docs/BITBOARD_IMPLEMENTATION.md) - Technical implementation guides
- **Square Conversion**: Seamless translation between 64-square and 120-square indexing
- **FEN Support**: Complete bidirectional FEN parsing and generation with validation

### Performance Optimizations

- **Zero Copy Operations**: Direct array access and bit manipulation
- **Fast Piece Location**: O(1) piece finding via piece lists
- **Efficient Memory Layout**: Compact data structures and minimal indirection

### Demo Applications

- **huginn.exe**: Main chess engine application (v1.1) with hybrid evaluation and single-threaded search
- **huginn_legacy.exe**: Original engine preserved for comparison and regression testing
- **huginn_v1.1.exe**: Versioned copy of the main chess engine
- **debug_demo.exe**: Comprehensive validation system demonstration with passing/failing scenarios
- **castling_demo.exe**: Interactive castling validation showing detection of various inconsistency types
- **fen_demo.exe**: FEN generation demonstration showing round-trip conversion capabilities
- **sq_attacked_demo.exe**: Attack detection visualization showing piece attack patterns and blocking

### Running Tests (CTest)

```powershell
# Build test binary (from project root)
cmake --build build/msvc-x64-release --config Release --target huginn_tests

# Run tests with CTest
cd build/msvc-x64-release
ctest --config Release --output-on-failure -V

# Run specific test patterns
ctest --config Release -R "perft|position"
```

### Convenience: `check` target

After the first configure, you can just:

```powershell
cmake --build build/msvc-x64-release --config Release --target check
```

> This builds `huginn_tests` and then runs `ctest`.

---

## GoogleTest Options

### Option A: Vendored (recommended for MSVC)

- Add googletest source at `external/googletest/`.
- In your top-level `CMakeLists.txt`:

  ```cmake
  # Vendored googletest
  add_subdirectory(external/googletest)

  enable_testing()
  include(GoogleTest)

  add_executable(huginn_tests
    test/test_board120.cpp
    test/test_chess_types.cpp
    test/test_perft.cpp
    test/test_main.cpp    # provides main()
  )
  target_compile_features(huginn_tests PRIVATE cxx_std_17)
  target_include_directories(huginn_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(huginn_tests PRIVATE GTest::gtest)

  # Discover at test time (prevents NOT_BUILT placeholders)
  set(GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
  gtest_discover_tests(huginn_tests DISCOVERY_TIMEOUT 30)

  # Convenience target
  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
    DEPENDS huginn_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
  ```

### Option B: Use MSYS2’s packaged googletest

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gtest
```

Then in `CMakeLists.txt`:

```cmake
find_package(GTest CONFIG REQUIRED)
target_link_libraries(huginn_tests PRIVATE GTest::gtest)  # or GTest::gtest_main if you omit test_main.cpp
```

> Avoid mixing system gtest **headers** with vendored **libs** (or vice-versa).

---

## Alternative PowerShell Workflow

For direct PowerShell usage with MSVC:

```powershell
# Clean build
Remove-Item -Recurse -Force build/msvc-x64-release
cmake --preset msvc-x64-release
cmake --build build/msvc-x64-release --config Release --target huginn
cmake --build build/msvc-x64-release --config Release --target huginn_tests

# Run tests
cd build/msvc-x64-release
ctest --config Release --output-on-failure -V
```

---

## Notes & Tips

- **CMake Presets**: The project uses CMake presets for consistent builds across environments.
- **Cleaning**: If builds get weird, delete the build directory and reconfigure.
  - PowerShell: `Remove-Item -Recurse -Force build/msvc-x64-release`
  - Command Prompt: `rmdir /s build\msvc-x64-release`
  - PowerShell: `Remove-Item -Recurse -Force build`
- **Where to add tests**: `test/*.cpp` (not `src/`). Targets: `huginn` (app) and `huginn_tests` (tests).
- **Debug asserts**: Enable in Debug builds; they’re your defensive-programming guardrails.
