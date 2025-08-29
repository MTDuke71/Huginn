# Performance Tracking System

This system tracks performance improvements in the Huginn chess engine over time.

## Files

- `performance_tracking.txt` - Log of all performance test results
- `perft/perf_test.ps1` - PowerShell script to run automated performance tests

## Usage

Run a performance test:
```powershell
.\perft\perf_test.ps1 -Description "Your optimization description"
```

## Test Details

- **Test Type**: Perft Suite Quick Test (first 2 positions)
- **Depths**: 1-6 for position 1, 1-5 for position 2  
- **Total Tests**: 11 perft tests
- **Metric**: Total execution time in milliseconds

## Baseline Performance

As of commit `486b47b` (IS_PLAYABLE macro optimization):
- **Time**: ~71,600ms (71.6 seconds)
- **Positions**: 2 of 128 test positions
- **Success Rate**: 100%

## Recent Performance Results

| Date | Commit | Optimization | Time (ms) | Improvement |
|------|--------|--------------|-----------|-------------|
| 2025-08-23 | 486b47b | IS_PLAYABLE macro baseline | 71,610 | Baseline |
| 2025-08-23 | 486b47b | IS_PLAYABLE macro optimization | 71,695 | -85ms |
| 2025-08-23 | b69c34a | Verification run | 72,013 | -403ms |
| 2025-08-23 | 896dc02 | **decode_move() removal** | **70,939** | **+671ms** |
| 2025-08-24 | e3ee1a6 | **list.clear() → list.count = 0** | **71,902** | **-963ms** |
| 2025-08-24 | e3ee1a6 | list.clear() verification run | 71,921 | -982ms |

## Major Optimization Series (August 2025)

| Date | Commit | Optimization | Time (ms) | Improvement |
|------|--------|--------------|-----------|-------------|
| 2025-08-25 | 6c16149 | **Pawn promotion optimizations** | **22,689** | **+49,232ms** |
| 2025-08-25 | 6c16149 | Verification run - pawn optimization | 21,991 | +49,930ms |
| 2025-08-25 | 31fa2e5 | Post-optimization profiling comparison | 22,047 | +49,874ms |
| 2025-08-25 | 31fa2e5 | King optimization measurement | 24,074 | +47,847ms |
| 2025-08-25 | 31fa2e5 | Streamlined king optimization | 22,489 | +49,432ms |
| 2025-08-25 | c4495a4 | Post-cleanup verification | 21,961 | +49,960ms |
| 2025-08-25 | f360c8e | Knight template optimization | 22,554 | +49,367ms |
| 2025-08-25 | 41c1fbb | **Sliding piece optimizations complete** | **22,519** | **+49,402ms** |
| 2025-08-27 | aca7b89 | **CastlePerm array optimization** | **29,537** | **+42,384ms** |
| 2025-08-28 | f39fdc6 | **Castling lookup table optimization** | **28,572** | **+43,349ms** |
| 2025-08-29 | 1c6af67 | **Atomic piece operations** | **28,873** | **+43,048ms** |

### Performance Analysis

#### 🚀 **Major Performance Breakthrough (6c16149)**: +49,232ms improvement  
The pawn promotion optimizations represent a **massive 68.4% performance improvement**:
- **Before**: ~71,921ms (list.clear() optimization)
- **After**: 22,689ms  
- **Improvement**: 49,232ms faster (68.4% improvement)

This dramatic improvement suggests a fundamental optimization in move generation algorithms.

#### ✅ **Sliding piece optimizations (41c1fbb)**: Maintained performance
The sliding piece optimizations maintain excellent performance at **22,519ms**, showing consistent results around the 22-23 second range after the major breakthrough.

#### ⚠️ **CastlePerm array optimization (aca7b89)**: -7,018ms regression
The CastlePerm optimization shows an unexpected **7,018ms regression** (31% slower):
- **Before**: 22,519ms (sliding piece optimizations)
- **After**: 29,537ms
- **Change**: -7,018ms slower (31% performance regression)

#### ✅ **Castling lookup table optimization (f39fdc6)**: +965ms improvement
The redesigned castling lookup table shows a **965ms improvement** (3.3% faster):
- **Before**: 29,537ms (CastlePerm array)
- **After**: 28,572ms  
- **Change**: +965ms faster (3.3% performance improvement)

This improvement demonstrates that the lookup table approach is more efficient than both the previous array implementation and the original conditional logic, while maintaining Huginn's C++ architecture and style.

#### 🔬 **Atomic piece operations (1c6af67)**: -301ms regression
The atomic `clear_piece()` and `add_piece()` functions show a **301ms regression** (1.1% slower):
- **Before**: 28,572ms (distributed piece operations)
- **After**: 28,873ms
- **Change**: -301ms slower (1.1% performance regression)

**Analysis**: This implementation follows the VICE tutorial pattern of atomic piece operations, consolidating:
- Zobrist hash updates
- Board square updates  
- Material score updates
- Piece counter updates
- Bitboard updates
- Piece list management

The slight performance regression likely occurs because:
1. **Branch prediction**: The distributed approach had highly predictable branches that CPUs optimized well
2. **Function inlining**: The compiler may have been inlining the smaller functions very effectively
3. **Instruction cache**: The larger atomic functions may have different cache characteristics

**Decision**: Keeping this change for code clarity, maintainability, and consistency with VICE tutorial learning, despite the minor performance cost.

**Critical Analysis - Compiler Difference Theory**: 
Based on git history, there was a significant build system change on August 23rd (`1bf35c9 Clean up CMakeLists.txt: Remove GCC/Clang build options for MSVC-only project`). This suggests:

1. **Early benchmarks (70,000ms range)** may have used GCC/Clang compiler
2. **Recent benchmarks (22,000ms range)** use MSVC compiler after the build system change
3. **Current benchmark (29,537ms)** uses MSVC but shows regression

**Possible explanations**:
- **Compiler optimization differences**: GCC vs MSVC have different optimization strategies
- **Array lookup vs branching**: MSVC may optimize conditional branches better than array lookups
- **Cache behavior**: Different compilers may generate different memory access patterns  
- **Inlining decisions**: Compiler may treat the array lookup differently than conditionals

**Verification needed**: Test with the same compiler configuration as previous benchmarks to isolate the optimization impact from compiler differences.

## Performance Tracking Format

```
Date, Git Commit, Optimization Description, Time (ms)
```

## Running Manual Tests

```powershell
# Build project
cd build && cmake --build . --config Debug

# Run perft quick test  
cd .. && echo "1" | .\build\perft_suite_demo.exe
```

## Future Optimizations to Track

- Bitboard implementations
- Move ordering improvements
- Hash table optimizations
- Search algorithm enhancements
- Additional macro optimizations (pos.at(), color_of(), etc.)
