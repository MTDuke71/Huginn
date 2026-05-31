# Profile Observations — Very Sleepy, 2026-05-31

Sampling-profiler capture of a single-threaded search, used to pick the
next round of speed work. Companion to
[PERFORMANCE_ARCHITECTURE_REVIEW.md](PERFORMANCE_ARCHITECTURE_REVIEW.md)
(which tracks the architectural priority list); this file records one
concrete measurement and what was done off the back of it.

## Capture setup

- **Tool**: Very Sleepy (CPU sampling profiler, Windows).
- **Binary**: `huginn.exe`, **RelWithDebInfo** config (`/O2` + PDB —
  optimized code *with* symbols; a plain Release build emits no PDB and
  the profile is all raw addresses). Built via
  `cmake --build build/msvc-x64-release --config RelWithDebInfo --target huginn`.
- **Workload**: `go` search from `startpos` with the opening book
  disabled (`setoption name OwnBook value false`), ~20 s wall.
- **Raw data**: [../benchmark/profile_20260531_very-sleepy.csv](../benchmark/profile_20260531_very-sleepy.csv).

> Reading the CSV: ignore the `ILT+NNNN(...)` rows — those are
> incremental-link thunks (duplicate stubs, `[unknown]` source), not real
> samples. Use the rows that have an actual source file.

## Hotspots (by self / exclusive time)

| Function | Self % | Incl % | Notes |
|---|---|---|---|
| `evaluate` (search.cpp) | **19.4%** | 37.2% | #1 cost centre — over a third of all time |
| `AlphaBeta` | 12.2% | ~100% | recursion root, expected |
| `pick_next_move` | **7.3%** | 12.0% | selection-sort move picking |
| slider attacks (`rook_attacks` + `bishop_attacks`) | **7.0%** | — | magic lookups |
| `generate_all_caps_pseudo` | 4.1% | 13.5% | qsearch capture generation |
| `GetNumberOfConsoleInputEvents` + `PeekConsoleInputA` | **6.0%** | — | GUI input polling (Windows console syscalls) |
| `PVTable::probe_move` | 3.4% | — | side-table PV probe (move ordering + reconstruction) |
| `mirror_square_64` | 2.5% | — | per-piece square flip in the PST lookup |
| `MakeMove` / `TakeMove` / `move_piece_sq64` | ~5% combined | — | normal make/unmake |

`quiescence` is **51% inclusive** — half of all time is in qsearch, so
capture-gen and SEE efficiency there matter as much as the main eval.

## Acted on in this batch

All three are behaviour-neutral to the search tree (verified: 197/197
tests pass, eval-symmetry suite green, identical nodes/score/PV at
depth 14):

1. **Input-poll throttle** (`checkup`). The console poll is ~5 µs and was
   firing every 2048 nodes → ~6% of total time. Decoupled it from the
   time check: time management keeps its 2048-node cadence (cheap
   `steady_clock`), the GUI-input poll is gated ~16× coarser
   (`INPUT_CHECK_MASK = 32767`). `StopInterruptsQuickly*` tests still
   green, so stop responsiveness is intact. **≈ −5% runtime.**
2. **`mirror_square_64` → `sq ^ 56`** in the PST lookup. The mirror is
   exactly an XOR of the rank bits; the old out-of-line call wasn't being
   inlined (it shows as its own profiler symbol). Replaced with a
   per-colour flip mask hoisted out of the inner loop. Provably identical
   evals (XOR ≡ old formula). **≈ +2.7% NPS measured** (3.42M → 3.51M at
   depth 14), matching the 2.5% prediction.
3. **PV reconstruction → triangular PV.** Not a speed item but found in
   the same session: the printed PV collapsed to a single move at high
   depth because it was rebuilt from the overwrite-always side
   `PVTable`, which thrashes once node counts explode. Now collected
   inline during search (exact, full-length). Also removes most of the
   `probe_move` reconstruction cost. See `pv_line`/`pv_length` in
   `SearchInfo`.

## Candidates not yet taken (ranked)

1. **`evaluate` itself (37% inclusive).** Biggest pie slice. Next steps:
   cache static eval per node (avoids recomputation in RFP/futility/
   razoring that all call `evalPosition` — overlaps
   PERFORMANCE_ARCHITECTURE_REVIEW Priority 6), then incremental PST/phase
   (Priority 10). The per-piece PST loop and the pawn-structure scan are
   the in-function hotspots.
2. **`pick_next_move` (7.3% self).** Selection sort re-scans the move list
   each call. Overlaps the staged-move-picker work (Priority 5).
3. **`PVTable::probe_move` ordering use (3.4%).** The side table is still
   probed for a move-ordering hint (priority 2,000,000, behind the TT
   move). Likely redundant with the TT hash move — but removing it
   **changes move ordering → strength**, so it needs a gauntlet, not a
   free swap. Separate experiment.
4. **qsearch (51% inclusive).** Capture generation + SEE pruning quality
   is where half the time goes; worth a dedicated pass.
