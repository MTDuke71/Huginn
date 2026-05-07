# Performance Testing Directory

Perft (move-generation correctness + speed) testing utilities for
Huginn.

## Files

- **`perft_suite_vice.cpp`** — main perft test driver (reads stdin
  for a test selection, prints per-position results and a total time).
  This is what `perf_test.ps1` builds and runs.
- **`perft_suite.cpp`** — alternative perft suite implementation.
- **`perft_benchmark.cpp`** — perft benchmark variant.
- **`perf_test.ps1`** — automated build + run + log script.
- **`performance_tracking.txt`** — CSV log of historical perft times.

## Usage

### Automated test with logging

```powershell
.\perft\perf_test.ps1 -Description "Your optimization description"
```

This builds via the `msvc-x64-release` preset, runs `perft_suite_vice`
with a "1" (quick test) on stdin, parses the total time, and appends
a row to `performance_tracking.txt`.

### Manual build and run

```powershell
# Build the targets you need
cmake --build build/msvc-x64-release --config Release --target perft_suite_vice
cmake --build build/msvc-x64-release --config Release --target perft_suite
cmake --build build/msvc-x64-release --config Release --target perft_benchmark

# Run perft_suite_vice (interactive — pick test mode from menu)
.\build\msvc-x64-release\bin\Release\perft_suite_vice.exe

# Or pipe a selection in (1 = quick test, mirrors what perf_test.ps1 does)
"1" | .\build\msvc-x64-release\bin\Release\perft_suite_vice.exe
```

## Performance tracking format

`performance_tracking.txt` is appended to by `perf_test.ps1`:

```
Date, Git Commit, Optimization Description, Perft Quick Test Time (ms)
```

Use it for historical comparisons of perft-quick-test wall-clock
across commits.

## Note

These perft tools measure move-generation correctness and speed in
isolation. They do **not** measure search-strength (Elo). For that,
use the gauntlet scripts in the repo root:

- `test_huginn_vs_t3.bat` — current regression yardstick (vs the t3
  baseline, which is the post-#13 TT-mate ship).
- `test_huginn_vs_mtlchess_v03.bat` — external MTLChess v0.3 calibration.
- `test_huginn_vs_mora.bat` — external MORA calibration (~2191 Elo).
