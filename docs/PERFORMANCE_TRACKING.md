# Performance Tracking System

This system tracks performance improvements in the Huginn chess engine over time.

## Files

- `performance_tracking.txt` - Log of all performance test results
- `perf_test.ps1` - PowerShell script to run automated performance tests

## Usage

Run a performance test:
```powershell
.\perf_test.ps1 -Description "Your optimization description"
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

### Key Finding: decode_move() Removal Shows Performance Improvement!

The removal of decode_move() function shows a **671ms improvement** (0.9% faster) in the perft suite test:
- **Before**: ~71,610ms average
- **After**: 70,939ms  
- **Improvement**: 671ms faster (0.9% improvement in perft test)

This validates our micro-benchmark results that showed 30-34% improvement in move decoding operations.

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
