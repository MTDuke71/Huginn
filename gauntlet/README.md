# gauntlet/ — two-machine match results (git-shuttled)

PGN + fastchess logs are committed here so results pass between the
two gauntlet machines over git instead of manual file copy.

## Machines

| Machine | CPU | repo path | bat | outputs |
|---|---|---|---|---|
| AMD | Ryzen 7 7800X3D (8C/16T) | `C:\Users\m_lad\Repos\Huginn` | `test_huginn_vs_t8.bat` | `*_amd.pgn` / `*_amd.log` |
| Intel | i7-13700K | `C:\Users\m_lad\Documents\Repos\Huginn` | `test_huginn_vs_t8.bat` | `*_intel.pgn` / `*_intel.log` |

**One self-configuring bat now serves both machines** —
`test_huginn_vs_t8.bat` auto-detects the box (repo path from `%~dp0`,
CPU vendor → `_intel`/`_amd` suffix), so there is no longer a separate
`_amd` variant. It runs **current Huginn vs the frozen `huginn_t8.exe`**
(`git tag baseline-t8`), `tc=10+0.1`, concurrency 4, **SPRT
`elo0=0 elo1=10`** with a 500-round cap = up to 1000 games each, into
machine-tagged files. 1000 + 1000 = **2000 games per experiment** (or
fewer if the SPRT stops early). The per-tier bats
(`test_huginn_vs_t4..t7`) are kept for historical regression checks;
**use `test_huginn_vs_t8.bat` going forward**.

## Workflow

1. On each machine: run its bat (each picks independent random
   openings, so the two halves are independent samples).
2. `git add gauntlet/ && git commit && git push` from each; `git pull`
   on the other to collect both halves.
3. Combined estimate: `fastchess.exe -pgnin` does **not** exist in
   this build, so pool manually. Read each machine's
   `Results of Huginn_current vs Huginn_t7` block (or the tail of its
   `.log`), sum the W / L / D across both runs, then:

   ```
   score = (W + D/2) / N
   Elo   = -400 * log10(1/score - 1)
   ```

   For the pooled **pentanomial**, sum the `Ptnml(0-2)` buckets across
   machines. If recomputing from a PGN, pair the two games of each pair
   by their `[Round]` tag — **not** file order, which is completion
   order under concurrency and scrambles the wings.

## Reading the fastchess result block

Each run ends with a summary like this (the candidate is engine A,
"current"; the baseline is engine B, `t8`):

```
Results of Huginn_current vs Huginn_t8 (10+0.1, 1t, 64MB, noob_3moves.epd):
Elo: 1.39 +/- 14.93, nElo: 2.01 +/- 21.53
LOS: 57.24 %, DrawRatio: 41.80 %, PairsRatio: 0.98
Games: 1000, Wins: 241, Losses: 237, Draws: 522, Points: 502.0 (50.20 %)
Ptnml(0-2): [28, 119, 209, 109, 35], WL/DD Ratio: 0.42
LLR: -0.25 (-8.4%) (-2.94, 2.94) [0.00, 10.00]
```

**Header `(10+0.1, 1t, 64MB, noob_3moves.epd)`** — the match conditions:
`tc=10+0.1` = 10 s base clock + 0.1 s added per move; `1t` = 1 search
thread; `64MB` = transposition-table hash per engine; `noob_3moves.epd`
= the opening book (each game starts from a random 3-move opening so the
sample isn't all the same line). Both engines always get identical
conditions.

**`Games / Wins / Losses / Draws / Points`** — the plain scoreboard, from
the candidate's view. `Points = Wins + Draws/2` (241 + 522/2 = 502.0).
`Points / Games` = the score rate, `50.20 %` here. Win/Loss/Draw are
*per game*.

**`Elo: 1.39 +/- 14.93`** — the strength difference converted from the
score rate: `Elo = -400·log10(1/score - 1)`. `+1.39` means the candidate
is ~1.4 Elo stronger; `± 14.93` is the **95 % confidence half-width**, so
the true value is very likely in `[-13.5, +16.3]`. A CI that straddles 0
means "not statistically distinguishable from no change."

**`nElo: 2.01 +/- 21.53`** — *normalized* Elo: the same difference
expressed in units of the result's spread (standard deviations of the
pair scores) instead of logistic Elo. It's the scale SPRT power actually
depends on, and it lets you compare experiments run at different draw
rates. Bigger |nElo| = a more *resolvable* effect, not necessarily more
Elo. You can mostly ignore it day-to-day; logistic Elo + CI is the
headline.

**`LOS: 57.24 %`** — **Likelihood Of Superiority**: the probability the
candidate is *truly* stronger than the baseline given this data (= the
fraction of the CI that lies above 0). 50 % = a coin flip; >95 % is the
usual "real improvement" bar. `57.24 %` = barely leaning positive,
basically inconclusive.

**`DrawRatio: 41.80 %`** — fraction of *pairs* (not games) that came out
**level**, i.e. scored exactly 1.0 over the two-game pair. That's the
middle pentanomial bucket: `209 / 500 = 41.8 %`. Note this is a
pair-level "tie" rate and is *not* the per-game draw rate (which is
`522/1000 = 52.2 %`) — a common point of confusion.

**`PairsRatio: 0.98`** — winning pairs over losing pairs:
`(Ptnml[3]+Ptnml[4]) / (Ptnml[0]+Ptnml[1]) = (109+35)/(28+119) = 0.98`.
Above 1.0 means more pairs went the candidate's way than the baseline's;
`0.98` is a hair under even.

**`Ptnml(0-2): [28, 119, 209, 109, 35]`** — the **pentanomial**, the
heart of the stats. Games are run in **pairs**: the same opening played
twice with colors reversed, which cancels color/opening luck and roughly
halves the variance vs counting games independently. Each pair is
bucketed by the candidate's combined score over its two games:

| bucket | pair score | meaning | count |
|---|---|---|---|
| 0 | 0.0 | lost both (LL) | 28 |
| 1 | 0.5 | one loss, one draw | 119 |
| 2 | 1.0 | level: win+loss **or** draw+draw | 209 |
| 3 | 1.5 | one win, one draw | 109 |
| 4 | 2.0 | won both (WW) | 35 |

Sum = 500 pairs = 1000 games. The **wings** (buckets 0/1 vs 3/4) are what
move Elo; a symmetric spread = a dead heat. This is what we sum across
machines to pool, and pairing must be by `[Round]` tag (see Workflow).

**`WL/DD Ratio: 0.42`** — inside the level bucket (the 209), how many
were *decisive-but-level* pairs (one Win + one Loss) vs *double draws*
(Draw + Draw). `0.42` = far more double-draws than win/loss pairs → a
quiet, drawish matchup (near-peer engines trading quick draws), not a
sharp one trading blows.

**`LLR: -0.25 (-8.4%) (-2.94, 2.94) [0.00, 10.00]`** — the **SPRT**
(Sequential Probability Ratio Test) state:
- `[0.00, 10.00]` = the two hypotheses being tested: **H0 = +0 Elo** (no
  improvement) vs **H1 = +10 Elo**. Set by `-sprt elo0=0 elo1=10`.
- `-0.25` = the current log-likelihood ratio — which hypothesis the data
  favors so far. Negative leans toward H0 (no gain), positive toward H1.
- `(-2.94, 2.94)` = the **decision bounds** (from `alpha=beta=0.05`,
  = `±ln(19)`). Cross the **lower** bound → stop and **accept H0** ("not
  a +10 gain"); cross the **upper** → **accept H1** ("≥ +10 confirmed").
- `(-8.4%)` = progress toward a bound (`-0.25 / 2.94`). Stay between the
  bounds until the round cap → **inconclusive**, and you fall back to the
  fixed-games Elo/LOS readout.

A neutral change (like our perf batch) can *never* hit H1 (+10) and only
slowly drifts toward H0, so it almost always runs to the cap inconclusive
— that's expected, and why "did we regress?" is answered by the pooled
CI/LOS, not by the SPRT verdict.

## Notes

- **Each run is its own experiment.** Both bats delete that machine's
  `.pgn`/`.log` and use `append=false`, so games never accumulate
  across runs. Commit/push the result before the next run if you want
  to keep it (the following run overwrites it).
- Files are machine-tagged by name, so an AMD pull never conflicts
  with an Intel push (different paths) — clean git merges.
- Logs are committed too (per request). They are large and noisy;
  prune old ones if the repo gets heavy.
- **Current baseline is `baseline-t8` = `b9d63f8`** (BACKLOG #15
  counter-move @1500 enabled; +7.1 Elo pooled vs t7). **Each machine
  builds its own `huginn_t8.exe` from the `baseline-t8` tag** with
  `-DENABLE_FATHOM=ON` — the same per-machine build environment the bat
  uses to rebuild the *candidate* from source, so both engines in the
  match are compiled identically on that box. Do **not** copy one box's
  binary to the other; the tag (= shared source) is what guarantees the
  two legs play the same baseline. Prior baselines: t7 `304f2b7`,
  t6 `5efaa78`, t4 `6e3a761` (BACKLOG #18).

## History

| Date | Machine | Games | Result | Notes |
|---|---|---:|---|---|
| 2026-05-31 | AMD 7800X3D | **1000 (SPRT)** | **+15.30 ± 14.32 Elo vs t8**, **LOS 98.20%**, W251/L207/D542 | **Perf trio + static-eval cache + dead undo-writes drop + PV repetition truncate** (HEAD `ca335c2` = `baseline-t8` + perf trio + Priority 6 + Priority 7 + #21 PV fix, 7 files +171/−61). SPRT `elo0=0 elo1=10`, LLR **+1.49 (50.5% to H1)** → halfway to H1 acceptance, hit cap. **First AMD 1000g this session to clear LOS 95% on a single run**; CI [+1, +30] — lower bound above zero. Pentanomial [20,110,206,134,30] — clear positive skew (30 best vs 20 worst, 134 W-pair vs 110 L-pair). **Static-eval cache (Priority 6) is the likely Elo mover**: skips redundant `evaluate()` during the RFP/razoring/futility/null-move prune cascade. Awaiting Intel pool — ship-grade single-machine signal. |
|---|---|---:|---|---|
| 2026-05-31 | AMD 7800X3D | **1000 (SPRT)** | **+3.82 ± 13.64 Elo vs t8**, LOS 70.87%, W230/L219/D551 | **Search perf trio** (HEAD `d6f9463` = `baseline-t8` + triangular PV + input-poll throttle + mirror→XOR + LSB micro-cleanup; 5 files +115/−35). SPRT `elo0=0 elo1=10`, LLR +0.09 (2.9%) → essentially neutral, inconclusive at cap. CI [−10, +17]; pentanomial [24,101,237,116,22] with 47.4% draws — classic near-peer + speed-only shape (NPS up but extra search doesn't convert at this TC). Wings near-symmetric, 116 W-pair > 101 L-pair — slight lean, awaiting Intel pool. |
| 2026-05-31 | Intel 13700KF | **1000 (SPRT)** | **+1.39 ± 14.93 Elo vs t8**, LOS 57.24%, W241/L237/D522 | **Search perf trio** (HEAD `d6f9463`, same binary as the AMD row above). SPRT `elo0=0 elo1=10`, LLR −0.25 (−8.4%) in bounds → neutral, inconclusive at cap. CI [−13.5, +16.3]; pentanomial [28,119,209,109,35], 52.2% draws — same speed-only shape as AMD, 109 W-pair vs 119 L-pair near-symmetric. Pools with the AMD row → see Pooled section. |
| 2026-05-31 | Intel 13700KF | **1000 (SPRT)** | **+7.30 ± 14.67 Elo vs t7**, LOS 83.54%, W249/L228/D523 | **BACKLOG #15** counter-move @1500 (HEAD `b9d63f8`). SPRT LLR +0.47 in bounds (16% to H1) → inconclusive at cap. Pentanomial [26,119,187,144,24]. Pools with the AMD row below → **SHIP as `baseline-t8`** (see Pooled section). |
| 2026-05-31 | AMD 7800X3D | **1000 (SPRT)** | **+6.95 ± 14.48 Elo vs t7**, LOS 82.68%, W230/L210/D560 | **BACKLOG #15** ply-tracked counter-move heuristic re-enabled at order-score 1500 (HEAD `b9d63f8` + lookup_tables/perft cleanup on top, all Elo-neutral). SPRT `elo0=0 elo1=10`, LLR +0.44 in bounds (15% of way to H1) → inconclusive at cap. CI [−7, +21]; pentanomial [21,123,201,125,30] — slight +current lean (30 best vs 21 worst, 125 W-pair vs 123 L-pair). Drift is the right sign; pooled with Intel below. |
| 2026-05-30 | Intel 13700K | **1000 (SPRT)** | **+2.08 ± 13.86 Elo vs t7**, LOS 61.60%, W221/L215/D564 | **#6+#29+#30 stack** (HEAD `2e5c7f7` = `baseline-t7` + 50-move + SEE-ordering + determinism fix). Ptnml [18,127,209,123,23]. Pools with the AMD row below → **+2.08 Elo, both machines exactly 50.30%, neutral**. A 200g Intel smoke read +33 (variance). **Decision: revert #6 SEE-ordering (`66bce5d`, no measured gain + per-capture cost), keep #29/#30 (correctness).** #6 parked WIP on `wip/see-capture-ordering`. |
| 2026-05-29 | AMD 7800X3D | **1000 (SPRT)** | **+2.08 ± 15.05 Elo vs t7**, LOS 60.70%, W246/L240/D514 | **#6+#29+#30 stack** (HEAD `2e5c7f7`, +51/−10 src vs `baseline-t7`; note: binary included #6 SEE-ordering, not #30 alone). SPRT `elo0=0 elo1=10`, LLR −0.17 in bounds → inconclusive at cap. CI [−13, +17]; Pentanomial [27,129,186,127,31]. Pools with Intel row above (pooled t=+0.40). **Same bug *class* as #26 (uninitialised read), much smaller blast radius** — history affects ordering only (self-corrects via iterative deepening), no #26-style cross-machine swing. #29/#30 kept on correctness; #6 reverted. |
| 2026-05-29 | AMD 7800X3D | **1000 (SPRT)** | **+1.39 ± 14.39 Elo vs t7**, LOS 57.51%, W236/L232/D532 | **BACKLOG #29** TT-safe fifty-move-rule draw in AlphaBeta (HEAD `534b44c`, +20/−1 src vs `baseline-t7`). SPRT `elo0=0 elo1=10`, LLR −0.24 in bounds → inconclusive at cap. CI [−13, +16]; same shape as the prior TT-safe correctness fixes (#28 +2.43, TT-safe rep −0.00). Pentanomial near-symmetric [26,121,198,133,22]. **Zero detectable strength cost; ship on correctness.** |
| 2026-05-29 | Intel 13700K | **1000 (SPRT)** | **+9.38 ± 14.65 Elo vs t6**, LOS 89.56%, W259/L232/D509 | **#28 Part 2 attempt 2b — narrow-gate Zarkov draw** (candidate `304f2b7`). SPRT `elo0=0 elo1=10`, LLR +0.73. Ptnml [27,109,202,134,28]. Pools with the AMD row below → **SHIP as `baseline-t7`**. |
| 2026-05-29 | AMD 7800X3D | **1000 (SPRT)** | **+5.91 ± 15.03 Elo vs t6**, LOS 77.96%, W250/L233/D517 | **#28 Part 2 attempt 2b — narrow-gate Zarkov draw** (HEAD `24f9fdb`/`304f2b7`, +69/−17 src). SPRT `elo0=0 elo1=10`, LLR +0.29 drifting toward H1 → inconclusive at cap. Lean positive (~+3 to +6 marginal vs TT-safe baseline), **not the −40 wide-gate disaster** of attempt 1; doesn't ship solo (CI straddles 0) — pools with Intel row above. |
| 2026-05-28 | AMD 7800X3D | **1000 (SPRT)** | **−0.00 ± 14.61 Elo vs t6**, LOS 50.00%, W240/L240/D520 | **TT-safe repetition handling** (HEAD `34c336e`, = baseline-t6 + #28 + TT-safe repetition, +58/−17 src). SPRT `elo0=0 elo1=10`, LLR −0.41 in bounds (inconclusive → cap). Literal zero at 1000g (coin flip down to the last game); CI [−15, +15]. Pure correctness, **zero Elo cost**. |
| 2026-05-28 | AMD 7800X3D | **1000 (SPRT)** | **+2.43 ± 15.52 Elo vs t6**, LOS 62.07%, W247/L240/D513 | **BACKLOG #28** halfmove-clock-bounded repetition lookback (HEAD `d04ee3e`, same +25/−5 src as `a21a037`). SPRT `elo0=0 elo1=10`, LLR −0.13 in bounds (inconclusive → hit 500-round cap). 1000g settles the question: **Elo-neutral with high power** (CI [−13, +18]); supersedes the 200g +3.47 ± 35 prelim. Keep on correctness grounds. |
| 2026-05-17 | Intel 13700K | 200 | **+15.65 ± 33.21 Elo vs t5**, LOS 82.31%, W45/L36/D119 | **`codex/reduce-winning-repetition-draws`** `5efaa78` (= main + root winning-repetition avoidance + PV repetition guard). Pools with AMD row below -> `baseline-t6`. |
| 2026-05-17 | AMD 7800X3D | 200 | **+27.85 ± 31.01 Elo vs t5**, LOS 96.19%, W48/L32/D120 | Same `5efaa78` repetition-draw fix. Clears 95% on a single 200g; pools cleanly with Intel to ship as `baseline-t6`. |
| 2026-05-17 | Intel 13700K | 200 | **≈ +1.74 Elo vs t5**, LOS ~52%, W45/L44/D111 | **`codex/remove-mailbox120-cleanup`** `fabf4ae` (24e51e4 + full 120-table removal / sq64-direct iteration, −1181 LOC). Dead heat. Pools with AMD row → codex Pooled section. |
| 2026-05-17 | AMD 7800X3D | 200 | **+12.17 ± 32.16 Elo vs t5**, LOS 77.16%, W46/L39/D115 | Same codex `fabf4ae` cleanup. Lean positive, no regression, same sign as Intel. Pools with Intel row → codex Pooled section. |
| 2026-05-17 | Intel 13700K | 200 | **−1.74 ± 29.95 Elo vs t5**, LOS 45.46%, W41/L42/D117 | **Mailbox 120→64 migration** (Zobrist `cbb615b` + S_MOVE/king_sq Stages 0–3, HEAD `24e51e4`). Statistical dead heat — footprint-neutral. Pools with the AMD row → see Pooled section. |
| 2026-05-17 | AMD 7800X3D | 200 | **+19.13 ± 32.82 Elo vs t5**, LOS 87.45%, W53/L42/D105 | Same `24e51e4` mailbox-elimination stack. Lean positive, no regression. Pools with the Intel row → see Pooled section. |
| 2026-05-17 | AMD 7800X3D | 200 | **−38.37 ± 31.90 Elo vs t5**, LOS 0.86%, W38/L60/D102 | **#26 board64 cache — cross-machine disagreement.** AMD −38 / LOS 0.86% vs **Intel +12.17 / LOS 77%** on identical binaries (~50 Elo, machine-dependent). NOT poolable; uninitialised `board64` suspected. Reopened (BACKLOG #26) |
| 2026-05-17 | AMD 7800X3D | 200 | **t5 +79.53 ± 40.59 Elo over t4**, LOS ~100%, t5 W97/L52/D51 | frozen **t4 vs t5** baseline-delta (not current-vs-tN); full t5 stack = P1a + #23 + #24 magic bitboards |
| 2026-05-15 | AMD 7800X3D | 200 | **+40.13 ± 39.78 Elo**, LOS 97.75%, W71/L48/D81 | code = t4 + P1a + **#23 TT-bound fix**; ran hot vs Intel's +8.69 |
| 2026-05-15 | Intel 13700K | 200 | **+8.69 ± 39.12 Elo**, LOS 66.90%, W70/L65/D65 | #23 TT-bound fix; original lean ship signal (BACKLOG #23) |
| 2026-05-15 | AMD 7800X3D | 200 | **+1.74 ± 45.82 Elo**, LOS 52.98%, W84/L83/D33 | first AMD baseline; flat (CI swamps it — exactly the #19 motivation) |
| 2026-05-15 | Intel 13700K | 200 | **-5.21 ± 43.42 Elo**, LOS 40.65%, W77/L80/D43 | parallel run on the Intel box |
| 2026-05-11 | Intel 13700K | 200 | **+22.62 ± 44.20 Elo**, LOS 84.40%, W85/L72/D43 | original P1a ship measurement (BACKLOG #1) |

### Pooled — search perf trio / no baseline (2000 games, two machines)

Candidate `d6f9463` (= `baseline-t8` + triangular PV + input-poll
throttle + eval mirror→XOR) vs frozen **t8**, `tc=10+0.1`, 1t, 64 MB
hash, `noob_3moves.epd`, round-paired pentanomials:

- Intel 13700KF: W241 / L237 / D522  (+1.39, LOS 57.2%)  [28,119,209,109,35]
- AMD 7800X3D:   W230 / L219 / D551  (+3.82, LOS 70.9%)  [24,101,237,116,22]
- **Pooled: W471 / L456 / D1073**, score **50.38%**, **+2.61 Elo
  [−7.5, +12.7]**, LOS **69.34%**, pooled Ptnml [52,220,446,225,57],
  pentanomial t ≈ +0.51.

**Verdict: KEEP on main; baseline stays `t8` (do NOT promote).** Both
machines land small-positive and same-sign (+1.39 / +3.82) — the tight
cross-machine agreement (the reproducibility whose *absence* reverted
#26) confirms a clean non-regression. Every change is move-for-move
identical to t8 at equal depth; the only real delta is ~7% NPS
(input-poll throttle ~5% + eval mirror XOR ~2.7%), which at 10+0.1
surfaces as a sub-noise +2.6 Elo. This is **not a strength ship** — there
is no Elo change to isolate — so cutting a `baseline-t9` would only force
every future experiment to re-clear this +2.6 for nothing. Baseline
therefore stays `t8`; these commits ride on main as a perf + PV-display
correctness batch. SPRT [0,10] can't resolve a ~neutral effect (LLR
wandered the bounds on both boxes), so the fixed-games pooled LOS 69% is
the readout. The triangular PV also fixed deep-PV truncation and the
"PV continues after repetition" GUI warnings (display-only; `7f723b6`
adds the print-time repetition guard on top, not in this binary). See
[docs/PROFILE_OBSERVATIONS.md](../docs/PROFILE_OBSERVATIONS.md).

### Pooled — #15 counter-move @1500 / baseline-t8 (2000 games, two machines)

Counter-move heuristic enabled @ order-score 1500 (HEAD `b9d63f8`) vs
frozen **t7**, `tc=10+0.1`, 1t, 64 MB hash, `noob_3moves.epd`,
round-paired pentanomials:

- Intel 13700KF: W249 / L228 / D523  (+7.30, LOS 83.5%)  [26,119,187,144,24]
- AMD 7800X3D:   W230 / L210 / D560  (+6.95, LOS 82.7%)  [21,123,201,125,30]
- **Pooled: W479 / L438 / D1083**, score **51.02%**, **+7.12 Elo
  [−3.2, +17.4]**, LOS **91.24%**, pooled Ptnml [47,242,388,269,54].

**Verdict: SHIP as `baseline-t8`.** Soft ship: pooled LOS 91% is just
under the 95% bar, but the two machines agree tightly (+7.30 vs +6.95) —
the cross-machine reproducibility whose *absence* reverted #26. The
effect also reproduces the original t4 measurement (+8.7, 2026-05-09) on
an entirely different baseline. SPRT [0,10] can't accept a ~+7 effect
(between the hypotheses), so the fixed-games pooled LOS is the readout.
Baseline refreshed to t8 so future experiments isolate the next change
rather than re-counting this +7. See BACKLOG #15.

### Pooled — #28 Part 2 narrow-gate Zarkov / baseline-t7 (2000 games, two machines)

Candidate `304f2b7` (= `baseline-t6` + #28 Part 1 + TT-safe repetition
handling + **single-rep Zarkov draw gated on winning eval**) vs frozen
**t6**, `tc=10+0.1`, 1t, `noob_3moves.epd`, round-paired pentanomials:

- Intel 13700K: W259 / L232 / D509  (+9.38, LOS 89.56%)  [27,109,202,134,28]
- AMD 7800X3D:  W250 / L233 / D517  (+5.91, LOS 77.96%)  [27,122,190,129,32]
- **Pooled: W509 / L465 / D1026**, score **51.10%**, **+7.6 ± ~10.5 Elo**,
  pooled Ptnml [54,231,392,263,60], pentanomial t ≈ +1.43.

**Verdict: SHIP as `baseline-t7`.** Both machines independently
positive, no cross-machine disagreement. A **~+9 Elo swing** over the
broad attempt-2 gate (−1.6, neutral) at identical fixture coverage
(6/10) — confining the single-rep draw to winning positions removed the
diffuse drag the board-wide version carried. Pooled LOS ~92% is just
under the 95% bar, but the change ships on combined grounds: a
positive-leaning Elo gain *plus* a correctness fix (stops provably-won
games being thrown to repetition), comfortably clearing the Part 1
precedent (correctness shipped at neutral).

### Pooled — #27 winning-repetition root avoidance / baseline-t6 (400 games, two machines)

`codex/reduce-winning-repetition-draws` `5efaa78` (= `main`
`35a5f22` + root-side winning repetition avoidance + PV repetition
guard) vs frozen **t5**:

- Intel 13700K: W45 / L36 / D119  (+15.65, LOS 82.31%)
- AMD 7800X3D:  W48 / L32 / D120  (+27.85, LOS 96.19%)
- **Pooled: W93 / L68 / D239**, score **53.125%**, **~+21.8 Elo**.

**Verdict: SHIP as `baseline-t6`.** Both machines are positive with
no cross-machine disagreement. The score gain came mostly from fewer
losses rather than fewer draws (draw count rose), but the pooled
result is a clean strength improvement and the PV repetition guard
eliminates current-side `PV continues after threefold repetition`
warnings in the Intel log.

### Pooled — codex 120-removal cleanup (400 games, two machines)

`codex/remove-mailbox120-cleanup` `fabf4ae` (= the `24e51e4` mailbox
stack **plus** deletion of the dead [120] tables and sq64-direct
iteration in zobrist / polyglot / mirrorBoard, −1181 LOC) vs frozen
**t5**:

- Intel 13700K: W45 / L44 / D111  (≈ +1.74, LOS ~52%)
- AMD 7800X3D:  W46 / L39 / D115  (+12.17, LOS 77.16%)
- **Pooled: W91 / L83 / D226**, score **51.0%**, **≈ +7.0 Elo**,
  LOS ≈ 78%.

**Verdict: KEEP.** Same pattern as the mailbox-migration pool below —
both halves same side (+1.7 / +12.2, no cross-machine disagreement,
the anti-#26), pooling to a lean **+7 Elo**. A pure deletion/cleanup
refactor landing neutral-to-slightly-positive is the expected and
desired outcome: the win is architectural (no 120 representation
left in the hot path) at zero Elo cost. Below the 95% ship bar for
an Elo *claim* but well clear of any regression concern.

### Pooled — mailbox 120→64 migration (400 games, two machines)

Current = full mailbox-elimination stack at `24e51e4` (Zobrist Piece
`[120]→[64]` `cbb615b` + S_MOVE/`king_sq` 120→64 Stages 0–3 +
search-table shrink) vs frozen **t5**:

- Intel 13700K: W41 / L42 / D117  (−1.74, LOS 45.46%)
- AMD 7800X3D:  W53 / L42 / D105  (+19.13, LOS 87.45%)
- **Pooled: W94 / L84 / D222**, score **51.25%**, **≈ +8.7 Elo**,
  LOS ≈ 77%.

**Verdict: KEEP.** No regression and — unlike BACKLOG #26 (Intel +12 /
AMD −38, opposite signs, ~50 Elo split) — **no cross-machine
disagreement**: both halves sit neutral-to-positive (−2 / +19, same
side) and pool cleanly to a lean **+8.7 Elo**. Below the 95% LOS ship
bar for an Elo *claim*, but the refactor's goal was architectural
(single 64-square representation, ~85 KB less hot footprint) at zero
Elo cost — delivered, with a slight positive lean and correctness
proven independently (perft d6 = 119060324, Kiwipete d5, 208 gtests,
400 legal gauntlet games).

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
