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
