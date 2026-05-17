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
3. Combined estimate: `fastchess.exe -pgnin` does **not** exist in
   this build, so pool manually. Read each machine's
   `Results of Huginn_current vs Huginn_t4` block (or the tail of its
   `.log`), sum the W / L / D across both runs, then:

   ```
   score = (W + D/2) / N
   Elo   = -400 * log10(1/score - 1)
   ```

## Notes

- **Each run is its own experiment.** Both bats delete that machine's
  `.pgn`/`.log` and use `append=false`, so games never accumulate
  across runs. Commit/push the result before the next run if you want
  to keep it (the following run overwrites it).
- Files are machine-tagged by name, so an AMD pull never conflicts
  with an Intel push (different paths) — clean git merges.
- Logs are committed too (per request). They are large and noisy;
  prune old ones if the repo gets heavy.
- t4 baseline is `6e3a761` (BACKLOG #18). Keep `huginn_t4.exe`
  byte-identical across both machines (build once, copy the binary —
  do not rebuild per-machine).

## History

| Date | Machine | Games | Result | Notes |
|---|---|---:|---|---|
| 2026-05-17 | AMD 7800X3D | 200 | **−38.37 ± 31.90 Elo vs t5**, LOS 0.86%, W38/L60/D102 | **#26 board64 cache — cross-machine disagreement.** AMD −38 / LOS 0.86% vs **Intel +12.17 / LOS 77%** on identical binaries (~50 Elo, machine-dependent). NOT poolable; uninitialised `board64` suspected. Reopened (BACKLOG #26) |
| 2026-05-17 | AMD 7800X3D | 200 | **t5 +79.53 ± 40.59 Elo over t4**, LOS ~100%, t5 W97/L52/D51 | frozen **t4 vs t5** baseline-delta (not current-vs-tN); full t5 stack = P1a + #23 + #24 magic bitboards |
| 2026-05-15 | AMD 7800X3D | 200 | **+40.13 ± 39.78 Elo**, LOS 97.75%, W71/L48/D81 | code = t4 + P1a + **#23 TT-bound fix**; ran hot vs Intel's +8.69 |
| 2026-05-15 | Intel 13700K | 200 | **+8.69 ± 39.12 Elo**, LOS 66.90%, W70/L65/D65 | #23 TT-bound fix; original lean ship signal (BACKLOG #23) |
| 2026-05-15 | AMD 7800X3D | 200 | **+1.74 ± 45.82 Elo**, LOS 52.98%, W84/L83/D33 | first AMD baseline; flat (CI swamps it — exactly the #19 motivation) |
| 2026-05-15 | Intel 13700K | 200 | **-5.21 ± 43.42 Elo**, LOS 40.65%, W77/L80/D43 | parallel run on the Intel box |
| 2026-05-11 | Intel 13700K | 200 | **+22.62 ± 44.20 Elo**, LOS 84.40%, W85/L72/D43 | original P1a ship measurement (BACKLOG #1) |

### Pooled (all three runs above, 600 games total)
**W 246 / L 235 / D 119**, score 50.92%, **~+6.4 Elo**, LOS ~69%.

The original Sunday +22 / LOS 84% was on the favorable side of the
noise band. Two subsequent independent runs (one on each machine,
both today) settled toward neutral-positive. P1a's true Elo is
probably ~+5 to +10, not +22. Ship decision wasn't wrong (the engine
IS slightly better, and the WAC tactical validation + bench evidence
are non-gauntlet positives), but the magnitude was overstated by the
single 200g sample.

### Pooled — #23 TT-bound fix (400 games, two machines)

Independent 200g runs of the **same #23 code** vs t4 (Intel + AMD):

- Intel: W70 / L65 / D65
- AMD:   W71 / L48 / D81
- **Pooled: W141 / L113 / D146**, score **53.5%**, **≈ +24.4 Elo**,
  LOS well above 95%.

The #23 fix shipped on Intel's lean LOS 67% (+8.69). The AMD run came
in hot (+40.13, LOS 97.75%); the 400g pool settles at **≈ +24 Elo** —
a real, meaningful gain that comfortably clears the ship threshold and
retroactively confirms the #23 ship decision. (Distinct code
generation from the P1a pool above — these cannot be merged with the
600g P1a sample.)

### Pooled — t5 over t4 (#24 full stack, 400 games, two machines)

Frozen **t4 vs t5** baseline-delta (P1a + #23 TT-fix + #24 magic
bitboards combined), t5 perspective:

- Intel (BACKLOG #24 current-vs-t4 @ #24 = t5): W82 / L39 / D79
- AMD   (t4-vs-t5 direct):                      W97 / L52 / D51
- **Pooled: W179 / L91 / D130**, score **61.0%**, **≈ +77.7 Elo**,
  LOS ≫ 99.99%.

Exceptionally tight cross-machine agreement (Intel +75.88, AMD
+79.53). The t5 stack is a rock-solid **~+78 Elo over t4** —
confirms the #24/#25 ship and the `baseline-t5 = 3eab266` freeze.

(Pooling method: see Workflow step 3 above — `-pgnin` isn't in this
fastchess build, so sum W/L/D and compute Elo manually.)
