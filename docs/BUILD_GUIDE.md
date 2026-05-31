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

- `huginn.exe` — main chess engine (UCI)
- `huginn_tests.exe` — complete test suite (GoogleTest)
- `perft_suite.exe` — perft benchmarking
- `mirror_eval_test.exe` — evaluation symmetry tests

## Running Tests

```powershell
# Run all tests
.\build\msvc-x64-release\bin\Release\huginn_tests.exe

# Run via CTest (preferred for CI-style output)
cd build\msvc-x64-release
ctest --config Release --output-on-failure -V

# Run perft suite
cd perft
.\perf_test.ps1
```

Current state: **208/208 tests passing**.

## Search Optimizations

Most search features (LMR, NMP, futility pruning, razoring,
quiescence, IID, aspiration step a, TT, SEE in qsearch, ply-tracked
TT-mate adjustment, etc.) are **always-on** in the Release build —
they're not behind compile flags.

The two exceptions:

- `USE_MULTI_CUT` — multi-cut pruning, defined `#ifdef
  USE_MULTI_CUT` in `src/search.cpp`. Not defined in
  `CMakeLists.txt`, so it is **disabled** by default. Define it
  per-target to experiment.
- `ENABLE_PLY_TRACKED_COUNTERMOVE` / `ENABLE_PLY_TRACKED_TT_MATE` —
  bisection gates at the top of `src/search.cpp`. Default is
  TT-mate **on**, counter-move **off** (the 2c ship from BACKLOG #13;
  counter-move is gated off pending re-attempt at #15).

## Current Performance

- **NPS**: ~1.5M nodes/second (measured: `go depth 11` from startpos
  with book disabled).
- **Strength**: `baseline-t3` tag, +104 ± 62 Elo (LOS 99.98%) over
  `baseline-t2` at tc=10+0.1 / 100 games — see [BACKLOG.md](BACKLOG.md)
  #13 for the full bisection trail.

For day-to-day regression tracking against the t3 baseline use
`test_huginn_vs_t3.bat` (auto-rebuilds Huginn before the gauntlet).
