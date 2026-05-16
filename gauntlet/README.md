# gauntlet/ — two-machine match results (git-shuttled)

PGN + fastchess logs are committed here so results pass between the
two gauntlet machines over git instead of manual file copy.

## Machines

| Machine | CPU | repo path | bat | outputs |
|---|---|---|---|---|
| AMD | Ryzen 7 7800X3D (8C/16T) | `C:\Users\m_lad\Repos\Huginn` | `test_huginn_vs_t4_amd.bat` | `*_amd.pgn` / `*_amd.log` |
| Intel | i7-13700K | `C:\Users\m_lad\Documents\Repos\Huginn` | `test_huginn_vs_t4.bat` | `*_intel.pgn` / `*_intel.log` |

Both bats run **current Huginn vs the frozen `huginn_t4.exe`**
(commit `6e3a761`, `git tag baseline-t4`), `tc=10+0.1`,
concurrency 4, 100 rounds = 200 games each, identical engine names,
appending to their own machine-tagged files. 200 + 200 = **400 games
per experiment**.

## Workflow

1. On each machine: run its bat (each picks independent random
   openings, so the two halves are independent samples).
2. `git add gauntlet/ && git commit && git push` from each; `git pull`
   on the other to collect both halves.
3. Combined estimate from the pooled PGNs:

   ```
   fastchess.exe -pgnin gauntlet/huginn_vs_t4_amd.pgn ^
                 -pgnin gauntlet/huginn_vs_t4_intel.pgn
   ```

   (or just sum the W/L/D tallies for a quick sanity check).

## Notes

- `append=true`: re-running a bat **adds** games to that machine's
  file. Delete the machine's `.pgn`/`.log` before a run if you want a
  fresh experiment rather than a cumulative one.
- Files are machine-tagged by name, so an AMD pull never conflicts
  with an Intel push (different paths) — clean git merges.
- Logs are committed too (per request). They are large and noisy;
  prune old ones if the repo gets heavy.
- t4 baseline is `6e3a761` (BACKLOG #18). Keep `huginn_t4.exe`
  byte-identical across both machines (build once, copy the binary —
  do not rebuild per-machine).

## History

| Date | Machine | Games | Result vs t4 | Notes |
|---|---|---:|---|---|
| 2026-05-15 | AMD 7800X3D | 200 | **+1.74 ± 45.82 Elo**, LOS 52.98%, W84/L83/D33 | first AMD baseline; flat (CI swamps it — exactly the #19 motivation) |
