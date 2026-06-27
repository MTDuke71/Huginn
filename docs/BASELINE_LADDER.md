# Baseline ladder

Full shipped-baseline history (newest first). Each `baseline-tN` tag is a
two-machine-confirmed (or logged-exception) ship. CLAUDE.md carries only the
current baseline + a pointer here; this file is the durable record.

Convention: baselines are built **per-machine** from the tag (never copy a
binary between boxes) and snapshotted as `huginn_tN.exe` in the fastchess dir.

---

### baseline-t21 — TT-clear-on-newgame (#46) + time-management fix (#47)
= t20 + two fixes surfaced by watching a 5+2 Arena game (t20 vs MTLChess v0.5):
the TT was 100% full from game 2 on, and Huginn finished 50 moves with **2:48
unused** while the opponent had 0:40.
- **#46 — clear TT + search tables on `ucinewgame`.** `reset()` (the only prior
  newgame action) just reset two flags; it also runs per-`go`, so the clear lives
  in the ucinewgame handler, not reset(). Stops stale prior-game entries being
  probed on a transposition (the #44 warm-TT hazard, across games; no aging yet).
- **#47 — use the clock.** The iteration-start gate bailed at `budget/4` (assumed
  the next iteration costs 3× cumulative elapsed), leaving ~75% of the clock
  unused → now `budget/2` (EBF≈2: a new iteration ≈ doubles cumulative time).
  Sudden-death/increment allocation uses `inc/2` not `inc/4`. The per-move
  `0.6×remaining` cap + checkup hard-stop still prevent flagging.
- Both were **invisible to the blitz A/B ladder** (they hit t19/t20 equally, so
  they cancelled) — same blind spot as #44/#45; caught only by watching real play.
- **Validation — t21 vs t20 (10+0.1, 400g): +126.97 ± 24.60 Elo, 67.5%, LOS 100%**
  (186W/46L/168D, Ptnml [1,17,62,81,39]), **zero time-forfeits** (all terminations
  normal). Single-machine decisive freeze (sign-flip impossible at this magnitude;
  precedent t12) — Intel confirms on push. 203/203 tests pass. The +127 at *blitz*
  is mostly #47: the gate bug was TC-independent, so reclaiming the unused clock
  buys a deeper search every move at every TC. LTC (60+0.6) rating pending →
  feeds the v2.2 release. Artifact: `gauntlet/t21_vs_t20.pgn`.

### baseline-t20 — move-level futility (#45): latent search-correctness bug fix
= t19 + replaced node-level futility (`return alpha` before the move loop when
`eval + (100+50·depth) ≤ alpha`, depth ≤3 — which bailed the WHOLE node incl.
interior **PV nodes** and tactical replies) with **move-level** pruning: inside
the loop skip only *quiet, non-promotion, non-checking* moves, still searching
every tactic; raise `best_score` to the futility bound so the fail-low / TT
upper-bound stays correct. One ~10-line change behind `ENABLE_MOVE_LEVEL_FUTILITY`
(default flipped ON). **This was a latent search-correctness bug invisible across
the entire t5→t19 ladder** — every baseline carried the identical node-level
futility, so it cancelled in every incremental A/B and only surfaced when the
futility *structure* itself was A/B'd (cf. #44).

**The largest single gain in the program's history.**
- **Two-machine SPRT vs t19 (10+0.1) — both legs H1-accept:** AMD **+345.15 ±
  60.79**, 87.94%/170g, LOS 100%, Ptnml [0,0,9,23,53]; Intel **+355.00 ± 71.85**,
  88.53%/171g, LOS 100%, Ptnml [0,2,8,17,58]. Magnitude (~6× any prior ship)
  triggered a sanity gate, NOT shipped on the SPRT alone.
- **Binary/source audit (Intel):** `huginn_t19.exe` byte-identical to the tag,
  `current` carries only the flag, clean isolation — not a build artifact.
- **External anchors (AMD, 200g each) — confirm it's real, not self-play:**
  control t19 vs Stash 12.0 = 51.25% (Stash not crippled, harness sound); candidate
  vs Stash 12.0 (1886) **90.5%**, Stash 13.0 (1972) **86.75%**, Stash 17.0 (2298)
  **56.75% / +47** (clean, non-saturated pin → ~2345). Cross-family 3-way RR:
  vs MTLChess v0.5 (2314) **61.0%** (~2392 on the MTL ladder, +9pp-favorable
  non-transitivity), vs v0.3 (1984) **88.2%**, and v0.3–v0.5 landed 12.5% = exactly
  its 330-Elo gap (anchors calibrated).
- **Strength: ~2350–2390 CCRL-ladder (≈ +510 over t19's ~1834).** Crosses a
  strength class. Artifacts: `gauntlet/mlf_vs_t19_*.pgn`, `mlf_vs_stash{12,13,17}.pgn`,
  `t19_vs_stash12_control.pgn`, `rr_mlf_mtl03_mtl05.pgn`. Pending: LTC (60+0.6)
  confirmation overnight → then the `v2.2` release.

### baseline-t19 — safe mobility (#9 round 9)
= t18 + per-piece-type mobility weights over a *safe* area (exclude own-occupied
+ enemy-pawn-attacked squares; the queen also excludes enemy-minor-attacked, the
#41 Queen-error cluster). 8 tunable weights `{KNIGHT,BISHOP,ROOK,QUEEN}_MOBILITY_{MG,EG}`
under `ENABLE_SAFE_MOBILITY` (default ON). Full 841-param re-tune (K=1.520),
MSE 0.057102→0.056857.
**Two-machine SPRT vs t18 — both legs same-sign positive:** AMD +5.91 ± 17.81,
LOS 74.2% (W349/L332/D319); Intel +10.43 ± 17.39, LOS 88.0% (W338/L308/D354,
Ptnml [50,106,163,126,55]); pooled 51.18% / 2000g. Neither leg clears 95%, but
tight cross-machine agreement clears the cross-machine-agreement ship bar (cf.
the #15 ship at 91%). The MSE drop did not fully convert — the bulk was the
joint re-fit, not the term — corroborating #41 that eval breadth isn't the gap.

### baseline-t18 = ab37a0d — mate-distance pruning (#43 sub-lever 3)
= t17 + at node entry clamp [α,β] to the mate envelope (best = MATE−ply, worst =
−MATE+ply); if it collapses the node can't beat a known mate, so cut. Sound,
cheap, move-for-move identical to t17 outside mate lines (where it steers to
shorter mates, saves nodes). Added behind a default-OFF flag (`a36bb96`), then
the complexity-gate two-machine SPRT vs t17 **passed:** AMD +14.95 ± 17.56, LOS
95.3% and Intel +10.08 ± 17.17, LOS 87.5% (1000g, W346/L317/D337, Ptnml
[41,117,177,102,63], LLR 0.64) — both positive. Default flipped ON.

### baseline-t17 = 9906fec — repetition-detector bug fix (#44)
= t16 + `repetition_count_in_history` used the grow-only `move_history` buffer
*size* instead of the current path length `pos.ply`; during deep search the
buffer over-counts, sliding the scan window off the real predecessors, so a true
3-fold read as a non-repetition at the deepest iteration — and with a warm TT
serving a stale winning score the engine drew won games (a concrete cause of the
#5 conversion weakness). The board never desynced; only the rep counter. Fix:
`history_len = pos.ply`. **+62 Elo self-play vs t16** (AMD, LOS 100%, H1 @482g);
**+48 external** (42.58% vs Stash 12.0 → Huginn ≈ 1834 CCRL, gap to Stash 12
~halved). NMP verification (#43) was bundled then **rejected** by an isolation
test (NMP-off ahead +14.6 — no benefit). Single-machine ship — clean bug fix.
Deterministic repro: `tools/repro_repetition_44.py`.

### baseline-t16 = 533d0b9 — king safety, finally converted (#9 round 7 / #2)
= t15 + reformulated the in-tree KS term to be Texel-tunable (removed the
≥2-attacker gate that made it zero on quiet positions and stalled hand-tuning at
~0 Elo; MTLChess-recipe king-zone units²/4, MG-only; weights + shelter now
EVAL_PARAM). The KS weights moved off their seeds under tuning (N 2→3, B 2→4,
shelter 18→21) = genuinely tunable at last; full 829-param tune, MSE
0.05732→0.05717. **+10.1 Elo pooled 2000g vs t15** [AMD +20.52@1000g LOS 99.6% /
Intel −0.35@1000g neutral] — AMD-strong, Intel non-regressive; first king-safety
ship in the program's history (the #41 study had pegged KS as the #1 lever).
Converted far above its MSE drop — quiet-corpus MSE under-states KS.

### baseline-t15 = cdcd31f — threats (#9 round 6)
= t14 + bonus per enemy piece attacked by a cheaper/more-dangerous attacker (by
attacker→target class, tapered); full 824-param tune on the 725k corpus, MSE
0.05799→0.05732 (strongest new-term signal since the early rounds). **+54.2 ±
14.9 Elo pooled ~1018g vs t14** [AMD +50.26@536g LOS 100% / Intel +58.95@482g
LOS 100%], both same-sign positive, both SPRT H1-accept — the largest eval-*term*
ship of the program, a clean two-machine decision. Also includes the #37
illegal-bestmove guard + #36 PV-display fix (so t15 won't forfeit on the
board-desync bug during round-7 gauntlets). Huginn 2.1 (`v2.1`) = this content.

### baseline-t14 = db3f1ef — rook on the relative 7th (#9 round 5)
= t13 + gated tapered bonus (enemy king on back rank OR enemy pawns on the rank);
full 812-param tune, MSE 0.05806→0.05799, fitted MG 20 / EG 24. **Shipped on a
sign-split as a deliberate methodology exception** [AMD +17.73@1000g LOS 99.0% /
Intel −4.52@1000g LOS 27.7%, pooled +6.6 ± 10.6] — does NOT meet the usual
same-sign bar; taken on the strong AMD leg + soundness + near-zero downside of a
small gated term.

### baseline-t13 = f90fd54 — connected + backward pawns (#9 round 4)
= t12 + connected (phalanx/supported) + backward pawn terms, both tapered; full
810-param tune, MSE 0.0583→0.0580. **+18.9 ± 10.7 Elo pooled 2000g vs t12** [AMD
+11.82@1000g LOS 93.9% / Intel +26.11@1000g LOS 99.96%], both same-sign positive
— two-machine ship.

### baseline-t12 = 1a0b3a1 — tapered EG PSTs + tunable mobility (#9 round 2)
= t11 + tapered (endgame) PSTs for the 5 non-king pieces + tunable mobility, full
~780-param re-tune, MSE 0.0596→0.0587. **+37.4 ± 17.9 Elo vs t11, LOS 100%, SPRT
H1-accept @764g (AMD)** — single-machine decisive freeze; Intel leg skipped
(sign-flip impossible at this magnitude).

### baseline-t11 = 4f091c1 — first Texel tune (#9)
= t10 + material MG/EG + all 6 PSTs + king-EG fit on the Zurichess quiet-labeled
725k corpus, MSE 0.0642→0.0596; the hand-set VICE PSTs had never been tuned.
`value_of()` decoupled onto a fixed canonical table so ordering/material don't
drift. **+71.4 Elo pooled 863g vs t10** [AMD +88.2@350g / Intel +59.6@512g], both
LOS 100%, both SPRT H1-accept — the largest single ship of the program. Strength
anchor: ~1818 ± 30 CCRL-Blitz (3-anchor MLE).

### baseline-t10 = 476d33c — tapered-eval foundation (#35)
= t9 + smooth `game_phase_256()` blend replaces the hard `is_endgame` boolean —
mg/eg sums diverge only on the king PST, flag-off byte-identical to t9. No new
tuned values. **+39.5 Elo pooled 1448g vs t9** [AMD +45.86@602g / Intel
+35.03@846g], both LOS 100%, both SPRT H1-accept — first eval-quality ship of the
#35 program.

### baseline-t9 = ca335c2 — perf trio + PV/cache cleanups
= t8 + [triangular PV + input-poll throttle + eval mirror→XOR] + PV repetition
truncate + static-eval cache + dead undo-state drop; **+13.90 Elo pooled vs t8,
LOS 99.6%**, both machines clear 95% — pure-speed ship, byte-identical search at
equal depth.

### baseline-t8 = b9d63f8 — ply-tracked counter-move (#15)
= t7 + counter-move heuristic on @1500; +7.1 Elo vs t7, LOS 91%, soft ship.

### baseline-t7 = 304f2b7 — repetition fixes (#28)
halfmove-clock lookback + TT-safe Zarkov single-rep draw; +7.6 Elo vs t6.

### baseline-t6 — winning-rep avoidance (#27)
(commit not recorded.)

### baseline-t5 = 3eab266 — P1a + TT-bound fix + magic bitboards
The foundation under the t10→t19 eval program: magic-bitboard sliders, the
TT-bound fix, and P1a.

---

_Pre-t5 history (the 2.0-era stack) lives in
[BACKLOG-archive-2.0.md](BACKLOG-archive-2.0.md). Add new baselines to the top of
this file and update the one-line current-baseline note in CLAUDE.md._
