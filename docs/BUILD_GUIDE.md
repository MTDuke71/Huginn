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

Current state: **197/197 tests passing**.

## Search Optimizations

Most search features (LMR, NMP, futility pruning, razoring,
quiescence, IID, aspiration step a, TT, SEE in qsearch, ply-tracked
TT-mate adjustment, etc.) are **always-on** in the Release build —
they're not behind compile flags.

The exception:

- `ENABLE_PLY_TRACKED_COUNTERMOVE` / `ENABLE_PLY_TRACKED_TT_MATE` —
  bisection gates at the top of `src/search.cpp`. TT-mate is **on**
  (the 2c ship from BACKLOG #13). Counter-move (@ score 1500) is
  currently **on** as the BACKLOG #15 re-attempt vs t7 (commit
  `b9d63f8`) — **revert to off if the pooled SPRT isn't clearly
  positive** (LOS ≥ 95%).

## Current Performance

- **NPS**: ~3.5M nodes/second (measured: `go depth 11` from startpos
  with book disabled; post-#24 magic bitboards).
- **Strength**: `baseline-t3` tag, +104 ± 62 Elo (LOS 99.98%) over
  `baseline-t2` at tc=10+0.1 / 100 games — see [BACKLOG.md](BACKLOG.md)
  #13 for the full bisection trail.

For day-to-day regression tracking against the t3 baseline use
`test_huginn_vs_t3.bat` (auto-rebuilds Huginn before the gauntlet).

## Profiling

Two different questions, two different tool classes — pick by intent:

- **"Where do the cycles go?"** (hot functions by self-time — usually
  what you want before optimizing) → **sampling** profilers.
- **"Which functions are called most often?"** (literal invocation
  counts + call graph) → **instrumentation** profilers.

**Always profile the optimized Release build**, never Debug (Debug
numbers lie). Make sure **symbols are emitted** or every frame shows up
as `huginn.exe!0x…`:
- MSVC Release already produces a `.pdb` next to the exe; keep it
  alongside `huginn.exe` so names resolve. (Add `/Zi` compile + `/DEBUG`
  link if a `.pdb` is missing — neither disables optimization.)
- GCC/WSL: build `-O3 -g -fno-omit-frame-pointer` (the `-g` and
  frame-pointer keep stacks readable without touching codegen speed).

### Tools (ranked for this setup)

| Tool | Class | Platform | Use it for |
|---|---|---|---|
| **Very Sleepy** | sampling | Windows, free | 5-minute "what's hot." No rebuild — just needs the `.pdb`. Best first look. |
| **Intel VTune** | sampling | 13700KF, free | Deep hotspots **+ cache misses / branch mispredicts / IPC**. Answers the memory-vs-compute-bound question (BACKLOG #31). |
| **AMD uProf** | sampling | 7800X3D, free | VTune-equivalent on the AMD box; run the same workload both machines and compare. |
| **VS Performance Profiler** (Alt+F2) | both | Windows, full VS | CPU Usage (hotspots) **and** Instrumentation (exact call counts). Attaches to the Release exe — the broken debug preset doesn't matter. |
| **callgrind** + KCachegrind | instrumentation | WSL/Linux | Precise per-function **call counts** + call graph. ~20–50× slowdown → use a *small* perft (depth 5), not a long search. |
| **perf** + flamegraph | sampling | WSL/Linux, free | Near-zero-overhead hotspots; `perf record -g ./huginn …` then `perf report` or a flamegraph. |
| **gprof** | instrumentation | GCC | Crude flat profile + call counts. Build `-pg -O3`, run once, `gprof huginn gmon.out`. |

### Driving a profiler — a deterministic, single-threaded workload

The engine is single-threaded and CPU-bound, so sampling works well.
Feed it a fixed search (exercises movegen + eval + search together) and
keep the run long enough to collect samples:

```powershell
# ~multi-second search from startpos — drives the whole engine
"uci`nisready`nsetoption name OwnBook value false`nposition startpos`ngo depth 14`nquit" |
  .\build\msvc-x64-release\bin\Release\huginn.exe
```

For **movegen-only** profiling (make/unmake, magic lookups, attack gen),
drive perft instead via `perft/perf_test.ps1`. For **call-count** runs
under callgrind, keep it tiny (`perft` depth 5) so the 20–50× slowdown
stays tolerable.

Expected hot functions, as a sanity check that symbols resolved:
movegen (`generate_*_moves`), magic slider lookups, `MakeMove`/`TakeMove`,
`evaluate`, and `pick_next_move` ordering.
