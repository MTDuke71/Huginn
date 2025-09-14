# Huginn Build Guide

## Recommended Build Method

Use the **msvc-x64-release** preset for consistent builds:

```powershell
# Quick build script
.\build_release.ps1

# Manual build
cmake --preset msvc-x64-release
cmake --build --preset msvc-x64-release --config Release
```

## Output Locations

All executables are built to: `build\msvc-x64-release\bin\Release\`

- `huginn.exe` - Main chess engine  
- `huginn_tests.exe` - Complete test suite
- `perft_suite_vice.exe` - Performance testing
- `mirror_eval_test.exe` - Evaluation symmetry tests

## Running Tests

```powershell
# Run all tests
.\build\msvc-x64-release\bin\Release\huginn_tests.exe

# Run performance benchmark
cd benchmark
python benchmark.py

# Run perft tests  
cd perft
.\perf_test.ps1
```

## Search Optimizations

The engine includes these optimizations (controlled by compilation flags):

- **USE_RAZORING**: Depth reduction when eval+margin < alpha
- **USE_MULTI_CUT**: Early pruning after multiple beta-cutoffs
- **USE_LMR**: Late move reduction
- **USE_FUTILITY_PRUNING**: Forward pruning in leaf nodes
- **USE_NULL_MOVE**: Null move pruning

All optimizations are enabled by default and verified through the test suite.

## Benchmark Results

Current performance with all optimizations:

- Average NPS: ~1,280,000 nodes/second
- All 246 tests passing
- Razoring and Multi-cut statistics available in search output