# Performance Testing Directory

This directory contains performance testing utilities and tracking for the Huginn chess engine.

## Files

### Core Performance Testing
- **`perft_suite_demo.cpp`** - Comprehensive perft test suite implementation
- **`perf_test.ps1`** - Automated performance testing script with logging
- **`performance_tracking.txt`** - Historical performance tracking data

## Usage

### Running Performance Tests

```powershell
# Run automated performance test with description
.\perft\perf_test.ps1 -Description "Your optimization description"

# Build and run perft suite manually
cmake --build build --target perft_suite_demo
.\build\perft_suite_demo.exe
```

### Performance Tracking

The `performance_tracking.txt` file contains historical performance data in CSV format:
```
Date, Git Commit, Optimization Description, Perft Quick Test Time (ms)
```

This data is automatically updated by `perf_test.ps1` and provides a historical view of optimization impact.

## Test Suite

The perft suite tests move generation correctness using standard chess positions and validates that the engine produces the correct number of moves at various depths.
