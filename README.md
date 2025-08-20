# Huginn  
_Huginn: Odin’s Thought, in Every Move_

![Huginn Logo](images/Huginn.png)

## MSYS2 UCRT64 C++ Build Instructions

### Prerequisites
1. Install MSYS2: https://www.msys2.org/
2. Open **MSYS2 MinGW UCRT64** shell (not the plain MSYS shell).
3. Update & install toolchain:
   ```sh
   pacman -Syu
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain cmake ninja
   ```

### Build (CMake + Ninja)
```sh
# In the MSYS2 MinGW UCRT64 shell
cd /d/repos/Huginn
mkdir -p build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Run the program
```sh
./build/huginn.exe
```

## Features

### Chess Engine Architecture
- **Mailbox-120 Board Representation**: 10x12 board with off-board sentinels for efficient boundary checking
- **Modern C++17**: Type-safe enums, constexpr functions, and std::array for performance
- **Bitboard Support**: 64-bit bitboards with visualization and manipulation macros
- **Piece List Optimization**: Fast piece iteration without board scanning (up to 10 pieces per type)

### Move System
- **Dynamic Move History**: Vector-based move history with automatic resizing and minimal memory overhead
- **Comprehensive Move Encoding**: From/to/promotion packed into single integer
- **Attack Detection**: Fast square attack detection for all piece types with proper blocking
- **Castling Rights**: Centralized constants and utility functions
- **Position Hashing**: Zobrist hashing for transposition tables

### Development Tools
- **Debug Assertions**: Detailed error reporting with file/line information when DEBUG defined
- **Comprehensive Testing**: 82+ test cases covering all major components including FEN generation
- **Position Validation**: Multi-layer integrity checking with castling validation and FEN round-trip verification
- **Bitboard Visualization**: ASCII board display with rank/file borders
- **Square Conversion**: Seamless translation between 64-square and 120-square indexing
- **FEN Support**: Complete bidirectional FEN parsing and generation with validation

### Performance Optimizations
- **Zero Copy Operations**: Direct array access and bit manipulation
- **Fast Piece Location**: O(1) piece finding via piece lists
- **Efficient Memory Layout**: Compact data structures and minimal indirection

### Demo Applications
- **huginn.exe**: Main chess engine application
- **debug_demo.exe**: Comprehensive validation system demonstration with passing/failing scenarios
- **castling_demo.exe**: Interactive castling validation showing detection of various inconsistency types
- **fen_demo.exe**: FEN generation demonstration showing round-trip conversion capabilities
- **sq_attacked_demo.exe**: Attack detection visualization showing piece attack patterns and blocking

### Running Tests (CTest)
```sh
# Build test binary
cmake --build build --target huginn_tests
# Run tests
ctest --test-dir build --output-on-failure -V
```

### Convenience: `check` target
After the first configure, you can just:
```sh
cmake --build build --target check
```
> This builds `huginn_tests` and then runs `ctest`.

---

## GoogleTest Options

### Option A: Vendored (recommended for Windows/MinGW)
- Add googletest source at `external/googletest/`.
- In your top-level `CMakeLists.txt`:
  ```cmake
  # Vendored googletest
  set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)  # robust on MinGW
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

## PowerShell (alternate workflow)
If you prefer PowerShell outside MSYS2:

```powershell
# Clean build
Remove-Item -Recurse -Force build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target huginn
cmake --build build --target huginn_tests
ctest --test-dir build -C Debug --output-on-failure -V
```

---

## Notes & Tips
- **Generator**: Using Ninja keeps toolchains consistent and avoids Visual Studio/MinGW mismatches.
- **Cleaning**: If builds get weird, delete the build dir and reconfigure.
  - MSYS2: `rm -rf build`
  - PowerShell: `Remove-Item -Recurse -Force build`
- **Where to add tests**: `test/*.cpp` (not `src/`). Targets: `huginn` (app) and `huginn_tests` (tests).
- **Debug asserts**: Enable in Debug builds; they’re your defensive-programming guardrails.
