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

- **USE_RAZORING**: Depth reduction when eval+margin < alpha ❌ *DISABLED*
- **USE_MULTI_CUT**: Early pruning after multiple beta-cutoffs ❌ *DISABLED*
- **USE_LMR**: Late move reduction ✅ *ENABLED*
- **USE_FUTILITY_PRUNING**: Forward pruning in leaf nodes ✅ *ENABLED*
- **USE_NULL_MOVE**: Null move pruning ✅ *ENABLED*

Razoring and Multi-cut optimizations have been disabled for performance testing.

## Benchmark Results

Current performance with optimized search:

- Average NPS: ~1,292,000 nodes/second (with razoring/multi-cut disabled)
- All 246 tests passing
- Search statistics available in engine output