# Huginn Chess Engine - GitHub Copilot Instructions

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively

### Environment Setup
- Install system dependencies:
  ```bash
  sudo apt-get update
  sudo apt-get install -y libgtest-dev build-essential cmake
  cd /usr/src/googletest
  sudo cmake -B build -DCMAKE_BUILD_TYPE=Release
  sudo cmake --build build --config Release
  sudo cmake --install build
  ```

### Build Process
- Configure and build the repository:
  ```bash
  cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
  cmake --build build --config Release
  ```
  - Build takes **1 minute 15 seconds**. NEVER CANCEL. Set timeout to 120+ seconds.
- Debug build (optional):
  ```bash
  cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_ASSERTIONS=ON
  cmake --build build_debug --config Debug
  ```
  - Debug build takes **56 seconds**. NEVER CANCEL. Set timeout to 90+ seconds.

### Testing
- Run comprehensive test suite:
  ```bash
  ctest --test-dir build --output-on-failure -V
  ```
  - Tests take **1.25 seconds** for 195 tests. NEVER CANCEL. Set timeout to 30+ seconds.
- Use convenience target (builds tests + runs them):
  ```bash
  cmake --build build --target check
  ```
  - Check target takes **58 seconds total**. NEVER CANCEL. Set timeout to 90+ seconds.

### Code Quality
- Format code (if needed):
  ```bash
  clang-format -i src/*.cpp src/*.hpp test/*.cpp
  ```
- Static analysis:
  ```bash
  clang-tidy src/*.cpp -- -I src -std=c++17
  ```

## Key Applications

### Main UCI Chess Engine
- **huginn**: Complete UCI chess engine compatible with chess GUIs
- Run basic UCI commands:
  ```bash
  echo -e "uci\nisready\nposition startpos\ngo movetime 1000\nquit" | ./build/huginn
  ```
- Test with FEN positions and moves:
  ```bash
  echo -e "uci\ndebug on\nisready\nposition startpos moves e2e4 e7e5\ngo movetime 500\nquit" | ./build/huginn
  ```

### Demo Applications
- **huginn_demo**: Position display demo (WARNING: may segfault - known issue)
- **parallel_benchmark**: Move generation multithreading benchmark - working correctly
- **test_move_decoding**: Performance testing for move decoding - working correctly
- **perft_suite_demo**: Perft testing (requires EPD file not included in repo)

## Validation Scenarios

### Always test the following after making changes:
1. **Build Validation**: Ensure both Release and Debug builds complete successfully
2. **Test Suite**: Run all 195 tests and verify they pass
3. **UCI Engine Functionality**: Test basic UCI commands with file input:
   ```bash
   cat test_uci_positions.txt | ./build/huginn
   ```
4. **Move Generation**: Verify the engine returns valid moves and handles positions correctly
5. **Debug Mode**: Test UCI engine with debug mode enabled to ensure logging works
6. **Search Commands**: Test infinite search with stop command

### Manual Testing Checklist
- [ ] Build completes without errors in under 2 minutes
- [ ] All 195 tests pass in under 30 seconds  
- [ ] UCI engine responds correctly to basic commands
- [ ] UCI engine handles FEN positions and move sequences
- [ ] UCI engine debug mode provides detailed logging
- [ ] Parallel benchmark executes without crashes

## Build System Details

### CMake Targets
- **huginn**: Main UCI chess engine
- **huginn_uci**: Alternative UCI interface
- **huginn_demo**: Position display demo
- **huginn_tests**: Comprehensive test suite (195 tests)
- **parallel_benchmark**: Move generation benchmarking
- **perft_suite_demo**: Perft validation suite
- **test_move_decoding**: Move decoding performance tests
- **check**: Convenience target that builds tests and runs them

### Build Types
- **Release**: Optimized production build (recommended, ~1:15 build time)
- **Debug**: Development build with assertions (~56s build time)
- **RelWithDebInfo**: Release with debug symbols

### Dependencies
- C++17 compiler (GCC 13.3+ or Clang)
- CMake 3.24+
- GoogleTest (system installation required)
- Threads library (pthread)

## Known Issues

### Working Applications
- ✅ UCI engine (huginn) - fully functional
- ✅ Test suite (huginn_tests) - all 195 tests pass
- ✅ Parallel benchmark - working correctly
- ✅ Move decoding performance test - working correctly

### Applications with Issues
- ⚠️ huginn_demo - segmentation fault on startup (known issue)
- ⚠️ perft_suite_demo - requires missing EPD file (test/perftsuite.epd)

### Workarounds
- Use the main huginn UCI engine for chess functionality testing
- Use huginn_tests for comprehensive validation
- Skip huginn_demo if you encounter segfaults

## Performance Characteristics

### Timing Expectations
- CMake configure: < 1 second
- Release build: 1 minute 15 seconds ± 10 seconds
- Debug build: 56 seconds ± 10 seconds  
- Test suite: 1.25 seconds for 195 tests
- Check target: 58 seconds total (build + test)

### Search Performance
- UCI engine provides progressive depth reporting
- Search typically reaches depth 6 in 500ms
- Engine provides realistic node counts and timing

## Repository Structure

### Source Code
- `src/`: Main engine source files (C++17)
- `test/`: Comprehensive test suite (GoogleTest)
- `perft/`: Performance testing code

### Documentation
- `docs/`: Extensive documentation (API, architecture, performance)
- `README.md`: Build instructions and project overview

### Build Artifacts (created during build)
- `build/`: Release build directory
- `build_debug/`: Debug build directory (optional)

## CI/CD Integration

The repository includes GitHub Actions workflow (`.github/workflows/cmake-multi-platform.yml`) that:
- Runs on Ubuntu with GCC and Clang
- Installs GoogleTest dependencies
- Builds the project
- Runs the complete test suite
- Expected to pass consistently when following these instructions

## Critical Reminders

- **NEVER CANCEL** build operations - they complete in under 2 minutes
- **ALWAYS** validate changes with the test suite before committing
- **VALIDATE** UCI engine functionality after making engine changes
- **SET TIMEOUTS** appropriately: 120s for builds, 30s for tests, 90s for check target
- **USE** the working applications (huginn, huginn_tests, parallel_benchmark) for validation
- **SKIP** problematic demos (huginn_demo) that may segfault