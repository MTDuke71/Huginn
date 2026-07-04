# BACKLOG archive — 2.1/2.2 era (t16 → t24)

Full detail for every item **closed** during the Huginn 2.1/2.2 development
arc: shipped baselines t16 through t24, the #41 calibration study, the #43
NMP round, the #44 repetition fix (+ its 2026-07 follow-up), the #45
move-level-futility fix and the pruning-stack audit it spawned, the #48/#49
speed pair, the #50 Zobrist OOB fix, and the 2026-07-02→04 SPRT queue (10
branches screened, 2 shipped as `baseline-t24`).

For issues #1–#37 (the 2.0-era history), see
[BACKLOG-archive-2.0.md](BACKLOG-archive-2.0.md). For currently open items,
see [BACKLOG.md](BACKLOG.md). Issue numbers are preserved so cross-references
from either file still resolve.

## Quick index

| # | Title | Status |
|---|-------|--------|
| 6 | Lazy SEE → full SEE good/bad capture ordering split | **SHIPPED** → `baseline-t24` |
| 9 / 35 | Texel eval program rounds 7–9 (king safety, threats round 2 partial, safe mobility, outposts) | **SHIPPED** t16/t19 (round detail below); roadmap continues in BACKLOG.md |
| 17 | Aspiration windows at the root | **REJECTED** @ t15 |
| 20 | Trapped-bishop eval pattern | **TESTED, PARKED** — clean two-machine neutral |
| 32 | PEXT slider attacks | **TESTED, PARKED** — 3–5% slower on this box |
| 38 | Displayed PV continues past the fifty-move rule | **FIXED** |
| 41 | Played-game calibration study (+ MTLChess v0.6 comparison) | **DONE** |
| 43 | NMP soundness/refinement round | **DONE** (sub-lever 3 shipped t18; others rejected/parked) |
| 44 | Repetition detector used buffer size, not ply | **FIXED** → t17; follow-up **SHIPPED** → t24 |
| 45 | Move-level (Fruit-style) futility vs node-level | **SHIPPED** → t20; pruning-stack audit below |
| 45-audit | Razoring off | **PARKED** — sign-split |
| 45-audit | Reverse-futility PV guard | **REJECTED** |
| 45 knob a | Futility envelope `<=3`→`<=2` | **PARKED** — clean neutral |
| 45 knob b | Futility PV guard | **REJECTED** |
| 46 | TT not cleared between games | **SHIPPED** → t21 |
| 47 | Time management under-uses the clock | **SHIPPED** → t21 |
| 48 | Kill the double TT probe | **SHIPPED** → t22 |
| 49 | Fuse king-safety attacker scan into the mobility pass | **SHIPPED** → t22 |
| 50 | Zobrist table OOB read (black king row) | **SHIPPED** → t23 |
| — | drawishness scaling (`mul[]`) | **TESTED, PARKED** — flat vs equal-strength t23 |
| — | SPRT queue (2026-07-02→04): the 10-branch screen that produced t24 | **CLOSED** |
| — | Historical "Current state" snapshot (2026-06-18, t19-era) | superseded by CLAUDE.md/BASELINE_LADDER.md |

---

## Historical "Current state" snapshot (as of 2026-06-18, superseded)

- **Baseline:** `baseline-t19` — **safe mobility** (#9 round 9): replaced the
  flat square-count × weight with per-piece-type weights over a *safe* area
  (exclude own-occupied + enemy-pawn-attacked squares; queen also excludes
  enemy-minor-attacked, the #41 Queen-error cluster), 8 tunable weights
  (`{KNIGHT,BISHOP,ROOK,QUEEN}_MOBILITY_{MG,EG}`) under `ENABLE_SAFE_MOBILITY`.
  Full 841-param re-tune (K=1.520) MSE 0.057102→0.056857. Two-machine SPRT vs
  t18 (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each) — both legs same-sign
  positive: **AMD +5.91 ± 17.81, LOS 74.2%** (W349/L332/D319) and **Intel
  +10.43 ± 17.39, LOS 88.0%** (W338/L308/D354, Ptnml [50,106,163,126,55],
  PairsRatio 1.16). Pooled W687/L640/D673 = **51.18% / 2000g**. Neither leg
  clears the 95% bar, but tight cross-machine agreement (both positive, +6 to
  +10) clears the cross-machine-agreement ship bar — a small keeper, like the
  #15 ship at 91%. The MSE drop did *not* fully convert (the bulk was the joint
  re-fit, not the term) — another datapoint that eval *breadth* isn't the gap
  (#41 / MTLChess). Prior: `baseline-t18 = ab37a0d` — **mate-distance pruning**
  (#43 sub-lever 3): at node entry clamp [α,β] to the mate envelope and cut if it
  collapses. Sound, cheap, move-for-move identical to t17 outside mate lines.
  Two-machine SPRT vs t17 **passed**: **AMD +14.95 ± 17.56, LOS 95.3%** and
  **Intel +10.08 ± 17.17, LOS 87.5%** (1000g, W346/L317/D337, Ptnml
  [41,117,177,102,63], WL/DD 2.00) — both positive. First #43 sub-lever to ship.
  Prior: `baseline-t17 = 9906fec` — the **#44
  repetition fix** (rep detector used the grow-only `move_history` buffer size
  instead of `pos.ply`, so deep iterations miscounted 3-folds and the engine drew
  won games). **+62 self-play vs t16 (AMD, H1 @482g)**; externally **+48 Elo /
  42.58% vs Stash 12.0 → Huginn ≈ 1834 CCRL** (gap to Stash 12 ~halved). NMP
  verification (#43) was bundled then rejected by an isolation test (no benefit).
  Prior: `baseline-t16 = 533d0b9` (round-7 king safety, +10.1 pooled).
- **Release: Huginn 2.1** (`v2.1`) = baseline-t15 content. **~+490 Elo over
  Huginn 2.0** — pooled two-machine advertising gauntlet (10+0.1, no book,
  600 games): Intel +517.7 ± 70 / AMD +468.0 ± 54, combined **536W / 3L / 61D
  = 94.4%**, both LOS 100%. That's the full stack since the 2.0 stamp
  (2026-04-26): magic bitboards, TT-bound fix, pure-bitboard rewrite, tapered
  eval, the Texel program (t10→t15), threats, and the illegal-move guard.
- **Baseline:** `baseline-t16 = 533d0b9` — round-7 **king safety** (#2/#41):
  reformulated tunable (gate removed → fires on quiet positions; weights moved
  under the tuner), MSE 0.05732→0.05717. **+10.1 Elo pooled 2000g vs t15** [AMD
  +20.52@1000g LOS 99.6% / Intel −0.35 neutral] — AMD-strong, Intel
  non-regressive; the **first king-safety ship in the program's history**.
  Per-machine build from the tag; `huginn_t16.exe` snapshotted. Prior:
  `baseline-t15 = cdcd31f` (round-6 threats, +54.2 pooled; + #37 guard + #36 fix).
- **Architecture:** pure bitboard; magic-bitboard sliders; tapered eval
  (`game_phase_256`); Texel-tuned material/PSTs/mobility/pawn-structure/threats/
  **king safety**. ~3.55 Mnps single-thread.
- **Active thread (2026-06-27): the pruning-stack audit (#45 et seq).** #45
  move-level futility passed SPRT on box 1 at **+345 Elo / 88% / LOS 100%** — a
  latent search-correctness bug (node-level futility pruned PV + ≤3-ply tactical
  nodes) invisible across the whole t5→t19 ladder. Pending 2nd-box confirm → flip
  flag ON + tag **t20**. This redirects the active thread from eval to **auditing
  the rest of the forward-pruning stack** (razoring, reverse futility, the #45
  depth/PV knobs) — same era, same node-level style, now prime leak suspects (see
  the pruning-stack audit thread under #45). The **#9/#35 eval program is paused**:
  round 10 (threats round 2) remains the top *eval* lever but eval breadth was
  already in diminishing returns, and a +345 search-correctness class dwarfs it.
- **Direction (2026-06-13): push pure HCE as far as it goes before reaching for
  multithreading or NNUE.** The +490 over 2.0 was mostly foundational repair
  (structural bugs, a never-tuned eval) and won't recur — but pure-HCE engines
  (Fruit 2.1 ≈ 2780 CCRL, Toga, early Stockfish) prove the HCE ceiling sits
  *hundreds* of Elo above Huginn's ~1818. So the road ahead is incremental HCE
  terms (see the round-7+ roadmap under #9/#35), with the two big architectural
  levers (#39 NNUE, #40 Lazy SMP) deliberately deferred.

## #9 / #35: Eval program rounds 7–9 detail (SHIPPED — round-by-round)

- **King safety, reformulated for the tuner** *(SHIPPED → baseline-t16, `533d0b9`)* —
  #2 was unblocked by the tapered base. The in-tree term was neutral and
  *excluded* from tuning because the ≥2-attacker non-linear gate fired too
  rarely in quiet positions to constrain. **Fix: removed the gate (fires on ≥1
  attacker, MTLChess units²/4 shape, MG-only), made the attacker weights +
  open-file shelter EVAL_PARAM.** The tuner then *moved* them off their seeds
  (N 2→3, B 2→4, shelter 18→21) = genuinely tunable at last.
  - **Round-7 SPRT vs t15 (533d0b9): SHIPPED, pooled +10.1 Elo / 2000g.**
    AMD: **+20.52 ± 15.17, LOS 99.61%** (W282/L223/D495, Ptnml [23,112,187,139,39]).
    Intel: **−0.35 ± 14.1, 49.95%** (W238/L239/D523, Ptnml [24,123,203,130,20],
    `gauntlet/huginn_vs_t15_ks_intel.pgn`). Pooled W520/L462/D1018 = 51.45%,
    Ptnml [47,235,390,269,59].
  - **AMD-strong / Intel-neutral — a keeper, the first king-safety ship ever.**
    Not a clean same-sign decisive (Intel ≈0, not positive), but milder than the
    t14 rook-on-7th exception (there Intel was −4.5): Intel here is non-regressive
    and AMD is much stronger (+20.5/LOS 99.6%). Converted **far above its MSE
    drop** (−0.00014) → confirms quiet-corpus MSE under-states KS.
  - **Round-8 hooks:** (a) re-run the #41 harness on t16 to confirm KS shrank the
    middlegame over-rating / recovered the king-attack bucket (~40% of blunders);
    (b) the Intel-neutrality leaves headroom — safe-checks / a king-danger table
    are natural KS extensions if a later round wants more.
- **Threats round 2** — extend the +54 t15 cluster: hanging pieces (attacked
  *and* undefended), pawn-push threats, threat-by-king. Same machinery, proven.
  **Not yet attempted** — remains on the live roadmap, see BACKLOG.md.
- **Mobility refinement (safe mobility)** *(SHIPPED → baseline-t19,
  2026-06-18)* — replaced flat square-count × weight with
  per-piece-type weights over a safe area (exclude own + enemy-pawn-attacked
  squares; queen also excludes enemy-minor-attacked, the #41 Queen-error
  cluster). 8 tunable weights (`{KNIGHT,BISHOP,ROOK,QUEEN}_MOBILITY_{MG,EG}`),
  `ENABLE_SAFE_MOBILITY` (default ON). Full 841-param re-tune (K=1.520) **MSE
  0.057102→0.056857** (−0.000245, ~10× the outpost round; bake verified exact).
  Symmetric, 203/203 tests pass.
  - **Two-machine SPRT vs t18 (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each) —
    SHIPPED, both legs same-sign positive (cross-machine-agreement bar).**
    - AMD: **+5.91 ± 17.81**, 50.85%, LOS 74.2%, W349/L332/D319, LLR 0.18 —
      positive lean, CI crosses zero. `gauntlet/safemob_vs_t18_amd.pgn`.
    - Intel: **+10.43 ± 17.39** (nElo +12.93), 51.50%, LOS 88.0%, W338/L308/D354,
      Ptnml [50,106,163,126,55], PairsRatio 1.16, LLR 0.65.
      `gauntlet/huginn_vs_t18_intel.pgn`.
    - Pooled W687/L640/D673 = **51.18% / 2000g**. Neither leg clears 95% LOS, but
      tight cross-machine agreement (both positive, +6 to +10) clears the
      cross-machine-agreement ship bar — a small keeper, like the #15 ship at 91%.
  - **The MSE drop did NOT fully convert** (the bulk of the −0.000245 was the
    flat joint re-fit, not the mobility term) — another datapoint that eval
    *breadth* isn't Huginn's gap (#41 / MTLChess). Flag stays ON; t18 params
    superseded.
- **Outposts** *(KEPT 2026-06-17 on a sign-split — logged exception; #41 Knight-error cluster)* —
  knight/bishop on an advanced square supported by an own pawn that enemy
  adjacent-file pawns can no longer challenge by advancing. Added tapered,
  Texel-exposed bonuses (`KNIGHT_OUTPOST_BONUS_MG/EG`,
  `BISHOP_OUTPOST_BONUS_MG/EG`) with coverage for challengeable-vs-true holes
  plus color symmetry. Full 833-param tune on `tools/texel/fens_quiet.txt`
  (725k, K=1.520) moved MSE 0.057119 -> 0.057102; fitted values:
  knight 33/11, bishop 28/7.
  - **Two-machine gauntlet vs t17 (2026-06-17, 10+0.1, 1t, 64MB,
    noob_3moves.epd, 1000g each) — SIGN-SPLIT, KEPT as a logged exception.**
    - Intel: **+9.04 ± 16.88** (nElo +11.54), 51.30%, LOS 85.32%, W336/L310/D354,
      Ptnml [40,116,180,106,58], LLR 0.54 — positive lean, CI crosses zero.
    - AMD: **−6.25 ± 17.61** (nElo −7.66), 49.10%, LOS 24.30%, W321/L339/D340,
      Ptnml [60,110,171,106,53], LLR −1.05 — negative lean, also unresolved.
    - **Pooled ≈ neutral (~+1.4).** Does NOT meet the same-sign two-machine bar —
      an opposite-sign split, like the t14 rook-on-7th exception.
    - **Decision (2026-06-17): KEEP** — code reviewed correct (support + rank
      gate + adjacent-file hole check, all colour-symmetric; the dedicated
      symmetry test + the 8-case mirror suite pass), near-zero downside, and it
      targets the #41 Knight-error cluster. Logged as a deliberate exception,
      **not a clean ship**. The marginal magnitude corroborates #41 / MTLChess:
      eval *breadth* is not Huginn's gap. Lives on `main` atop t17 (not separately
      tagged); fold into the next baseline tag.
    - Artifacts: `gauntlet/huginn_vs_t17_amd.pgn` + `gauntlet/fastchess_t17_amd.log`
      — the log also validates the #38 PV fix (289 fifty-move-PV warnings from
      t17, **0 from current**; that warning was the run's only warning class).
- **Passed-pawn refinements** — king distance to the passer (own + enemy),
  blockade, rook-behind-passer. Endgame Elo — **deprioritized**: #41 shows
  balanced-endgame play is already solid (fair-fight cp-loss 13.3). **Not yet
  attempted** — remains on the live roadmap, see BACKLOG.md.
- **Drawishness scaling (`mul[]`)** — opposite-coloured bishops →½, KNNK→0,
  "a minor up, no pawns"→⅛. High impact, cheap, independent; kills the
  "scores a dead-drawn endgame as winning" class. **Not** sigmoid-tunable (it's
  a final-eval multiplier) — hand-set from known values + SPRT to confirm.
  **Tested 2026-07-03 (`copilot/fix50-for-drawishness-scaling`,
  `ENABLE_DRAWISHNESS_SCALING`) — PARKED, two-machine flat neutral, exactly
  as predicted.** Implemented: pure-OCB ×64/128, pawnless-favoured-side-up-
  ≤-a-minor ×16/128 (KNNK etc. deliberately left to `MaterialDraw()`, which
  already short-circuits it — not double-handled). Two-machine SPRT vs t23
  (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each): AMD **+1.04 ± 15.10**
  (nElo +1.49), 50.15% (W240/L237/D523), LOS 55.38%, Ptnml
  [32,116,201,119,32], LLR −0.29; Intel **+4.52 ± 15.25** (nElo +6.38),
  50.65% (W256/L243/D501), LOS 71.94%, Ptnml [32,116,191,129,32], LLR 0.11.
  Both dead-flat. **Correctly predicted before the run:** this term targets
  the #5 conversion weakness (drawing WEAKER opponents), so self-play vs an
  equal-strength t23 is the least sensitive test available — mirror games
  rarely reach the exact drawish material configs this term discounts, and
  even when they do, both players are equally strong so the "converting a
  won-but-drawish endgame" scenario barely arises. Flag verified genuinely
  live (not a repeat of the futility-pv-guard silent-off bug) via a direct
  eval A/B on a pure-OCB position (73 vs 127 cp, roughly the intended
  halving) plus the Intel implementer's independent confirmation (OCB +3P
  endgame: cp 150 scaled vs cp 300 unscaled). **Not folded into t24 on
  current evidence** — a future revisit should test vs a weaker external
  anchor (Stash 11/12, the #5 draw-prone pairing) rather than self-play.
- **Lower priority:** doubled-rooks / blind-pig follow-up to t14's rook-on-7th;
  space (safe squares behind own centre pawns); rook-on-king-file; specific
  endgame recognizers (KPK, KRKP); trapped-bishop (#20, tested + parked, see
  below).

## #41: Played-game calibration study — round-7 priority evidence (DONE 2026-06-14)

Analyzed **Huginn 2.1 vs Stockfish 17.1 over 6,649 unique positions** from a
correspondence-game variation tree (`CC_Games.pgn`). Per position: SF depth-22
best + eval (ground truth), Huginn's move at 1.5 s, **SF's eval of Huginn's own
move** (mover-POV, so cp-loss = `sf_eval − sf_eval_after_hug`), plus phase /
material / tactical tags. Reusable harness (resumable, ~4 h for the full tree):
`tools/analyze_played_vs_engines.py`, `tools/summarize_cc.py`,
`run_cc_analysis.bat` → `tools/cc_analysis.csv`.

**Findings** (on *fair fights* = |material| ≤ 200 ∧ |SF eval| ≤ 300 cp, the
meaningful subset — raw endgame/quiet cp-loss is inflated by decided deep-
analysis lines and was discarded):

- **Positional, not tactical.** Huginn matches SF's best move **71 % in tactical
  positions** (SF-best is a capture/check) but only **34 % in quiet** ones. The
  gap is positional move-selection = eval quality.
- **Systematic over-optimism, worst in the middlegame.** Huginn over-rates the
  positions it steers into (its own eval − SF's truth of its move): opening +30,
  **middlegame +44**, endgame +21 cp. Consistent optimism is the fingerprint of
  an engine that under-weights danger to its own king and the opponent's
  resources — a king-safety / threats shaped hole.
- **Most "errors" are depth, not eval.** Of fair-fight move-choice misses, only
  **27 % are genuine eval-blindness** (Huginn over-rated its move ≥ 75 cp); 73 %
  are moves it evaluated ~correctly that SF (d22) simply out-searched (Huginn ran
  at ~d11). The dramatic 400–567 cp tail is largely tactical-depth — **not** an
  HCE target; don't chase it.
- **Balanced endgames are fine** (fair-fight cp-loss 13.3) → no endgame priority.
- Eval-blind errors cluster by piece: **Queen / Pawn / Knight**.

**Conclusion → round-7 order (now data-backed):** king safety (the systematic
middlegame optimism + quiet-move weakness *is* the KS hole) → threats round 2
(the other half of the optimism: under-credited opponent resources) → safe
mobility (the Queen cluster) → outposts (the Knight cluster; low-risk warm-up).
Passed-pawn / drawishness drop in priority. See the round-7+ roadmap under
#9/#35. **Validation hook:** a shipped KS term should measurably shrink the
middlegame over-rating — re-run the harness on a larger game set to confirm.

**RE-DIAGNOSIS vs t21 (2026-06-27) — the conclusion PARTIALLY FLIPS: search
soundness, not eval, was the bigger lever.** Re-ran the harness on the same 6649
positions (`run_cc_analysis_t21.bat`, reusing the t15 SF ground truth by FEN;
`tools/cc_analysis_t21.csv` vs the saved `cc_analysis20260614.csv`). t15→t21 deltas:
- **Huginn==SF 42.8%→48.5% (+5.6)**; middlegame 40.4→47.7 (+7.3), quiet 33.6→39.2
  (+5.7), tactical 70.7→76.2 (+5.5); **fair cp-loss 23.9→15.6 (−8.3)**.
- **Over-optimism (the #41 fingerprint): middlegame 44.2→29.0cp (−15.1)**, opening
  29.6→17.9, endgame 21.3→15.6. **Mean depth only +0.6 ply.**
- **Reading:** t15→t21 is overwhelmingly *search* (#45 futility +510, #47 clock
  +127 dwarf the ~+30 from the t16–t19 eval ships), yet it cut the middlegame
  over-optimism #41 had labelled an *eval* hole by a third, and lifted *quiet*-move
  match — at near-constant depth (so it's soundness/recovered-tactics, not deeper
  search). **So a large share of what #41 attributed to eval quality was actually
  the broken search.** This is the data behind re-prioritising the **pruning-stack
  audit + deferred-fix re-tests over threats-round-2** — #41's "lead with eval"
  order was set on the buggy search. **Not a full flip:** quiet (39%) still trails
  tactical (76%) and middlegame optimism is still 29cp, so positional eval retains
  *some* headroom — just less than #41 implied. Caveat: cumulative t15→t21, not an
  isolated search-only delta (no t19 cc-baseline exists), but the eval ships were
  tiny vs the search fixes, so the attribution holds.

## MTLChess v0.6 source comparison (2026-06-14) — adds a *search* track to #41

Read MTLChess v0.6's `search.zig` + `eval.zig` as an answer key (it's the same
author/tooling, magic-bitboard-native, and sits ~+500 above 2.1; round-robin
#5). It reframes round 7.

- **Their eval is *simpler* than Huginn's — and still wins.** MTLChess v0.6 =
  material + tapered PST + king safety. **No mobility, no pawn structure, no
  bishop pair, no rook-on-file, no threats, no outposts**, all hand-tuned. So
  eval *breadth* is decisively not the gap → **demote outposts / more pawn terms
  further** (corroborates #41 dropping passed-pawn/drawishness).
- **King safety converges as the #1 eval lever** (both #41's middlegame
  over-optimism *and* MTLChess's +116 v0.6 jump). Their recipe is dead simple and
  directly portable: king-zone = king + 8 neighbours; sum attacker hits ×
  weight (N/B=2, R=3, Q=5); penalty = `min(units²/4, 500)`, **MG-only, no
  shelter / no safe-checks**. Simpler than Huginn's stalled hand-tuned attempts —
  start by porting this, then optionally extend.
- **The search track attacks #41's 73% "depth, not eval" share** — but its first
  lever just failed. #41 found most misses are Huginn out-searched (d11 vs d22).
  MTLChess has depth-buying levers Huginn lacks:
  - **Aspiration windows (#17)** — **REJECTED @ t15, −33.8 Elo H0 (2026-06-14).**
    Regressed even on the strong eval base (the "stronger eval unblocks it"
    hypothesis is falsified); reverted. **Tempers the whole search track:**
    search-*efficiency* levers don't convert at 10+0.1 for an eval-bound engine —
    the eval track (king safety) is the higher-confidence bet. See #17.
  - **SEE-based capture ordering (#6, parked)** — good captures (SEE≥0) score
    *above* killers, **bad captures (SEE<0) below quiets**. Huginn orders by
    MVV-LVA, so it tries losing captures early → wasted nodes. (Their parked #6
    was "lazy SEE"; this is the full good/bad split.) They also fixed a SEE loop
    bug — audit Huginn's SEE for correctness while there.
  - These won't reach d22, but they narrow the Huginn-vs-MTLChess search gap and
    lift the share eval can't touch.
- **Pruning audit:** MTLChess prunes *less* — null R=2/3 (depth-scaled) vs
  Huginn's **R=4**, and **no** futility/razoring/IID/counter-move. Huginn's
  heavier stack at R=4 may over-prune / leak tactics. Worth checking a sound-lean
  config against the current one.

**Revised round 7 (after aspiration's rejection): lead with eval.** Eval track is
now the clear priority: **king safety** (port the MTLChess recipe) — agreed #1
from both studies, and the #17 result removes aspiration from contention. The
search track is demoted: aspiration rejected; **SEE capture ordering (#6)** is the
one remaining search lever worth a look (different mechanism — move-ordering
quality, not window efficiency — but its own history is only ~neutral, so treat
it as low-priority after king safety, not parallel to it). **Update (2026-06-15):
the Stash v12/v13 changelog re-opens the search track with a higher-confidence
lever — `NMP soundness/refinement` (#43).** Unlike aspiration/SEE (efficiency &
ordering, mixed record), #43 is a *soundness/adaptive-pruning* class: Huginn's
flat R=4 with no verification is a genuine over-pruning smell, and Stash bought
+282 CCRL across v11→v13 almost entirely from NMP+LMR and their refinements.

## #43: NMP soundness/refinement round (DONE — sub-lever 3 SHIPPED t18; #1 rejected, #2 parked — 2026-06-15)

**Highest-confidence search lever — pointed at by three independent sources:**
#41 (gap is "depth, not eval" — 73% of misses), the MTLChess comparison (their
strength is well-tuned simple search, not eval breadth), and the **Stash v12/v13
changelog** (+196 CCRL from *adding* NMP+LMR, then +86 from *refining* them —
+282 total, zero new eval). The lesson: Huginn already carries the techniques
(NMP, LMR, futility, razoring, IID) yet sits ~200 Elo under Stash v13 — so the
gap is search **calibration/soundness**, not missing features. This round audits
Huginn's null-move pruning, the prime over-pruning suspect.

**Current Huginn NMP (audited 2026-06-15):** flat `R=4`
([search.cpp:1502](../src/search.cpp#L1502)), min depth 5, `has_non_pawn_material`
guard, **no verification search**, **no eval/depth scaling**, returns `beta` on
cutoff ([search.cpp:1535](../src/search.cpp#L1535)). Aggressive *and*
uncalibrated. (LMR, by contrast, is already *ahead* of Stash v13 — `log·log`
table, depth-gated ≥3, exempts checks/captures/promotions
[search.cpp:1655](../src/search.cpp#L1655) — so LMR is **not** part of this round.)

**Three sub-levers, each its own fixed-depth + fixed-time SPRT (per the
complexity gate):**
1. **NMP verification search** *(lead — the soundness fix)*. Huginn has none;
   Stash had one since v12 (re-search without null at high depth on a fail-high),
   v13 added a `beta > 5000` zugzwang-mate recheck. Flat-R=4 + no-verification is
   the genuine tactical-leak smell. Fixed-depth gain here = "plugged a leak"
   (the most valuable outcome); a fixed-time-only gain = pure speed.
   - **IMPLEMENTED 2026-06-15 — `ENABLE_NMP_VERIFICATION`, default OFF
     (`main` byte-identical).** On a null fail-high, if `depth >= 10` **and**
     `game_phase_256(pos) <= 96` (low non-pawn material — zugzwang is an endgame
     thing), re-search the actual position at `depth − R` with null disabled
     (same node, no ply bump); confirm the cutoff only if it also fails high,
     else fall through. 197/197 tests pass on the ON build. **Cost (fixed-depth
     node count):** the material gate was essential — *ungated* it cost **+53%
     nodes** in a middlegame; **phase-gated it's 0% in the middlegame and −19%
     (node-negative, TT-warming) in a rook endgame**, same best moves. Now
     genuinely SPRT-worthy. Tunables if neutral: the depth-10 / phase-96
     gates, the `depth − R` verify depth. Build the test arm with
     `-DENABLE_NMP_VERIFICATION=1` (see `build/msvc-nmpon`).
   - **Fixed-time SPRT vs t16 (AMD, 10+0.1, 2026-06-15): +11.82 ± 15.16,
     51.7% (269W/235L/496D), LOS 93.7%, LLR 0.98 (inconclusive @1000g).**
     Positive lean, not a regression — strong for a pure-soundness term that
     only fires in low-material endgames (49.6% draws). Point estimate sits
     *above* the elo1=10 target but the CI is wide; SPRT didn't resolve.
     Ptnml [25,123,178,141,33]. PGN `gauntlet/huginn_nmpver_vs_t16_amd.pgn`.
   - **REJECTED (2026-06-16) by a clean isolation test.** After the #44
     repetition fix landed, ran NMP-off vs NMP-on with *both arms carrying #44*
     (AMD, 10+0.1, 1000g) to isolate the NMP delta: **NMP-OFF ahead +14.60 ±
     16.65, 52.1%, LOS 95.75%** (LLR 1.15, inconclusive). So NMP-on is
     neutral-to-slightly-harmful — no measurable benefit, leans ~−14. The
     earlier +11.8 vs t16 was noise / pre-#44 context. The bundled +62 vs t16
     was **all #44** (see #44). Flag defaulted OFF; code kept behind it for a
     possible re-formulation (eval/depth-scaled R, cheaper verify depth). The
     fixed-depth-12 arm was abandoned mid-run (pivoted to the #44 bug it
     surfaced). **Lesson:** a soundness term that only fires in endgames and
     adds a re-search tax needs a real benefit to clear its cost — this one
     didn't. Sub-lever 3 (MDP) shipped (t18); sub-lever 2 (scaled R) untried.
2. **Eval/depth-scaled R** — *PARKED 2026-06-22 (neutral on both machines),
   `ENABLE_SCALED_NMP_R` default OFF.* Replaced flat R=4 with `R = 2 + depth/4`
   (+1 when static eval − beta ≥ 200), clamped — prunes less near the leaf, more
   at depth, the MTLChess profile. Works as designed (**27% fewer nodes at fixed
   depth d13**) but the two-machine SPRT vs t19 was neutral on both boxes:
   **AMD +0.00** (328W/328L/344D, LOS 50%, LLR −0.41) and **Intel +3.82 ± 17.5**
   (334W/323L/343D, LOS 66.6%, LLR −0.02, inconclusive); pooled ≈ +1.9 — no
   benefit. The extra depth the savings buy is cancelled by the tactics the
   heavier deep reduction skips. Flat R=4 is already well-calibrated; the
   MTLChess "prunes less + wins" edge is elsewhere, not its NMP R. Code kept
   behind the flag for a possible re-formulation (different base/div, eval-only
   scaling). Artifacts: `gauntlet/nmpr_vs_t19_amd.pgn`,
   `gauntlet/huginn_vs_t19_intel.pgn`. **Lesson:** a cheap, data-backed
   search-param test that came back cleanly neutral on both machines — the
   checkpoint working as intended (cost: one constant + two gauntlets, not an
   eval round).
3. **Mate-distance pruning** — *SHIPPED as `baseline-t18 = ab37a0d` (2026-06-17),
   `ENABLE_MATE_DISTANCE_PRUNING` default ON.* Clamp α/β to the mate envelope
   (`MATE − ply` / `−MATE + ply`) at node entry (after the draw checks, before the
   TT probe), cut if it collapses. Provably sound — verified identical moves/scores
   on/off (mate-in-1: 215 vs 377 nodes; quiet d12: same move/score, ~1.7% fewer
   nodes); 203/203 tests pass. Added behind the flag (a36bb96, default OFF), then
   the complexity-gate **two-machine SPRT vs t17 passed**: **AMD +14.95 ± 17.56,
   52.15%, LOS 95.28%** (LLR 1.10) and **Intel +10.08 ± 17.17, 51.45%, LOS 87.5%**
   (1000g, W346/L317/D337, Ptnml [41,117,177,102,63], WL/DD 2.00, LLR 0.64) — both
   inconclusive at cap but clearly positive, tight cross-machine agreement. Shipped
   on soundness + positive lean (zero downside — cannot change a non-mate result).
   The **first #43 sub-lever to ship**: sub-lever 1 (NMP verification) was rejected,
   sub-lever 2 (scaled R) remains untried.

**Caveat (why this isn't a slam dunk):** search levers have a mixed Huginn record
— #17 aspiration −34, #6 SEE ~neutral (at the time). But those were
*efficiency/ordering* levers; #43 is *soundness/adaptive*. The R=3 pruning-
soundness test the complexity gate already calls for folds in here naturally.

## #44: Repetition detector used buffer size, not ply — FIXED (2026-06-16); follow-up SHIPPED (2026-07-03)

**Symptom:** Huginn drew clearly-won games by 3-fold repetition — shuffling in a
+6.8 (or rook-up) position into a draw, often playing the repeating move in
~0.001s. A concrete mechanism behind the #5 conversion weakness; surfaced in two
Stash-RR games. **Now has a deterministic repro** (`tools/repro_repetition_44.py`)
— a first for this bug family (#37 was never reproducible).

**Root cause (one line).** `repetition_count_in_history` used
`move_history.size()` as the history length. But `move_history` is a *reusable
high-water-mark buffer*: `MakeMove` does `resize(ply+1)` (grow-only) and
`TakeMove` never shrinks it ([position.cpp:266](../src/position.cpp#L266)). So
during deep search its size exceeds the current path length `pos.ply`, and the
entries past `pos.ply` are **stale undos from deeper/sibling lines**. The
`halfmove_clock`-bounded scan window was computed from the inflated size, so it
slid off the real predecessors — a *true* 3-fold read as a non-repetition at the
deepest iteration (the one that picks the move). The board itself never desynced
(zobrist key constant across iterations) — only the rep counter was wrong.

**Trigger:** only with a **warm TT** — a stale *winning* score is cached for the
position before it became a 3-fold, and with detection failing nothing overrides
it. Cold (single search) the engine finds the win; replaying the moves in one
process (as any GUI does) reproduces the draw. The instrumentation nailed it:
`repcnt` was correct (2/3) at shallow iterations and collapsed to 1 as `hist`
inflated (116→128) while the root key stayed constant.

**Fix:** `const int history_len = pos.ply;` (was `move_history.size()`) — the
current path is exactly `move_history[0 .. pos.ply)`. One line + the `< 6` guard.
Verified: the warm-TT repro now plays the winning `h6h7` at both rep points
(was `Kg6`→draw); 197/197 tests pass.

**Status:** FIXED in-tree, **gauntlet-confirmed (AMD)**. Changes rep detection
used throughout search (root draw-avoidance, in-tree single-rep/threefold, PV
truncation), so it is NOT byte-identical.
- **SPRT vs t16 (AMD, 10+0.1) — bundled with #43 NMP-on: +61.92 ± 23.32,
  58.82% (180W/95L/207D), LOS 100%, LLR 2.99 → H1 ACCEPTED @482g.** Ptnml
  [9,37,91,68,36]. PGN `gauntlet/huginn_fix_nmpon_vs_t16_amd.pgn`. The package
  was #44 + #43-NMP together; since NMP-alone was only +11.8 (inconclusive),
  **#44 carries ~+50 of the +62** — by far the bigger lever. One of the largest
  jumps in the program, and it resolved in <½ the game cap.
- **Attribution (2026-06-16):** the isolation gauntlet (NMP-off vs NMP-on, both
  with #44) put NMP-off +14.6 ahead, so **#44 alone ≈ +76 vs t16** (+62 bundle
  + 14.6 NMP drag) — by far the largest single fix in the program. NMP
  verification was dragging it down and is rejected (#43).
- **baseline-t17 = the #44 fix alone (NMP off).** AMD-confirmed via the +62
  bundle, the isolation test, AND an **external anchor**: t17 vs **Stash 12.0
  (1886)** scored **42.58% / −51.92 ± 24.5 (600g, AMD)** → Huginn ≈ **1834
  CCRL**, up from 36.0% / ~1786 pre-#44 (June RR). **+6.6pp = ~+48 external
  Elo**, gap to Stash 12 ~halved (100→52). The external +48 vs the +62/+76
  self-play is the usual self-play inflation (#5) — **+48 is the honest gain.**
  Single-machine accepted (clean bug fix); t17 is the shipped baseline.

**Related:** #28 (TT-safe repetition handling — this is the missing piece: the
detector it relies on was miscounting), #5 (conversion weakness — one concrete
cause), #37 (also a make/unmake-buffer subtlety, but that one *does* desync the
board; distinct bug).

**Follow-up — SHIPPED 2026-07-03 (`copilot/fix50-for-root-twofold-avoid`,
`ENABLE_ROOT_TWOFOLD_AVOID`):** extended the root draw-avoidance to a winning
*single* repetition (rep count ≥2, not just the rule-threefold ≥3), so a won
engine routes around the shuffle one move earlier — the root child at
`info.ply==1` was the one gap the existing #28-Part-2 Zarkov single-rep rule
didn't cover. Provably inert without game history (startpos node counts
identical to t23; verified via the dedicated
`RootTwofoldAvoid.WinningRootRoutesAroundSingleRepetition` unit test, which
directly confirms a stale warm-TT repetition bait is routed around).
**Two-machine SPRT vs t23 (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each) —
cross-machine-agreement ship:** AMD **+12.51 ± 15.22** (nElo +17.73), 51.80%
(W269/L233/D498), LOS 94.68%, Ptnml [24,117,201,115,43], LLR 1.06; Intel
**+7.99 ± 15.12** (nElo +11.39), 51.15% (W268/L245/D487), LOS 85.01%, Ptnml
[32,109,193,136,30], LLR 0.53. Pooled W537/L478/D985 = **51.48% / 2000g**.
Neither leg resolves individually, but both same-sign positive and both LOS
values clear the t19 safe-mobility ship precedent (+5.91/LOS 74.2%, +10.43/LOS
88.0%) — a clean small keeper. **Combined into `baseline-t24`** (see the SPRT
queue section below).

## #45: Move-level (Fruit-style) futility vs current node-level — SHIPPED t20 (2026-06-17→27); pruning-stack audit CLOSED (2026-07-04)

A pruning-soundness experiment (sibling to #43), prompted by comparing Huginn's
futility against the Fruit 2.1 reference (`Repos/fruit_21/src/search_full.cpp`).

**Current Huginn futility (pre-fix, [search.cpp:1741](../src/search.cpp#L1741)):**
`depth <= 3 && !in_check && !isRoot`; if `static_eval + (100 + 50*depth) <=
alpha`, **`return alpha`** — prunes the WHOLE node *before* the move loop. So it
skips **every** reply, including captures, promotions, and checks; quiescence at
the leaf is the only backstop.

**Fruit 2.1 futility (`search_full.cpp:697`):** **move-level** — inside the move
loop, `continue` (skip just that move) when `depth == 1 && node_type != NodePV
&& !in_check && new_depth == 0 && !move_is_tactical && !move_is_dangerous &&
!move_is_check`. Margin a flat **100cp**. So it prunes only *quiet,
non-near-promotion, non-checking* moves and still searches all tactics. (Notable:
Fruit ships it **disabled by default**, `UseFutility = false`.)

**Three ways Huginn's is more aggressive / riskier:**
1. **Node-level vs move-level** — Huginn's `return alpha` drops the node incl.
   tactical replies; a depth-3 node with a low static eval but a rook-winning
   capture or a check available is pruned unseen. *The tactical-leak risk, and
   the one that matters* (same class as #44).
2. **depth ≤ 3 vs depth 1** — Fruit only trusts futility at the frontier (one
   ply from qsearch); Huginn extends two plies deeper under a bigger margin.
3. **No PV-node guard** — Huginn excludes only `isRoot`; Fruit excludes all PV
   nodes (`node_type != NodePV`; PVS equivalent here is `beta > alpha + 1`).

**IMPLEMENTED 2026-06-26 — `ENABLE_MOVE_LEVEL_FUTILITY`, default OFF (`main`
byte-identical to t19).** First cut isolates *only* the node→move change: when the
node-level test fires (`eval + (100+50·depth) <= alpha`, `depth<=3`, `!in_check`,
`!isRoot`) it no longer `return alpha`s; instead the move loop skips a move only
when it is quiet (`!capture && !promotion`) **and** `!gives_check()` (the same
post-MakeMove check the LMR exemption uses), raising `best_score` to the futility
bound so the fail-low / TT upper-bound stays correct. **Kept the existing
`depth<=3` envelope and `!isRoot` (not full-PV) guard** so the SPRT measures the
tactical-leak fix alone; depth-narrowing (`==1`/`<=2`) and a PV-node guard were
follow-up knobs (see the pruning-stack audit below). 203/203 tests pass on both arms.

- **Fixed-depth tactical diagnostic (complexity gate, local) — strongly positive.**
  WAC300 @ depth 10, both arms scored by SAN-match (`tools`-style harness):
  **OFF(node) 143/299 → ON(move) 184/299 = +41 solves** (55 MISS→OK, 14 OK→MISS)
  for **+2.0% nodes** (8.72M→8.90M). The MISS→OK set is overwhelmingly the
  captures/checks node-level futility was pruning unseen (Qxf8+, Nxf7, Rxh7+,
  Qxh2+, Rxg2+, Bxh7+, Qxg6+, Nxe4 …) — direct confirmation of the leak.
- **Two-machine SPRT vs t19 (10+0.1, 1t, 64MB, noob_3moves.epd) — BOTH LEGS
  H1-ACCEPT at an anomalously large magnitude (sanity-checked, then VALIDATED).**
  - **AMD: H1 accepted @170g — +345.15 ± 60.79 Elo** (nElo +548.87), 87.94%
    (149.5/170g, W136/L7/D27), LOS 100%, DrawRatio 10.6%, Ptnml [0,0,9,23,53]
    (zero pair losses, PairsRatio inf), LLR 2.95.
  - **Intel: H1 accepted @170g — +355.00 ± 71.85 Elo** (nElo +498.14 ± 52.23),
    88.53% (W137/L6/D27, Points 150.5), LOS 100%, DrawRatio 9.41%, PairsRatio
    37.50, Ptnml [0,2,8,17,58], LLR 2.97. Artifacts `gauntlet/mlf_vs_t19_intel.pgn`
    (171g) + `gauntlet/mlf_fastchess_t19_intel.log`. All terminations `normal`.
  - **⚠ The magnitude failed a first-pass sanity check, so we did NOT ship on the
    SPRT alone.** +350 Elo is ~6× the largest prior ship (#44 +62/+76). Required
    two further checks (both PASSED).
- **Check 1 — Binary + source audit (Intel, 2026-06-27): PASSED. Clean t19 vs
  t19+move-level-futility isolation, NOT a build artifact.** Signature = `go depth
  14` from startpos, 1t, `OwnBook=false`, default hash (deterministic node counts):
  1. **`source current` (flags OFF) ≡ t19** — `git diff baseline-t19..HEAD -- src/`
     is *only* Doxygen comments + two default-OFF flag guards; **0** non-comment
     code changes elsewhere. The only behavioural delta available to the test arm
     is the futility flag.
  2. **fastchess `huginn_t19.exe` IS the tag** — byte-for-byte identical signature
     to a fresh `baseline-t19` build: d12 4,683,084 / d13 6,833,542 / d14 10,357,442
     nodes, cp 18/26/37, identical PVs, ~3.5 Mnps.
  3. **fastchess `current` genuinely carries the flag** — signature *diverges* from
     t19 (d14 12,035,479 nodes = +16%, cp 27, PV `e2e4 e7e5 b1c3 b8c6 …`). Not an
     accidental t19 rebuild.
- **Check 2 — External anchor (AMD, 2026-06-27): PASSED. The +350 is real, not a
  self-play mirage.** `current` vs **Stash 12.0** (1886, the calibrated AMD anchor)
  with a t19-vs-Stash-12 **control** alongside (200g each, same settings):
  - **t19 control: 51.25% / +8.69 ± 47.6** (W83/L78/D39, Ptnml [18,17,30,12,23]) —
    t19 plays Stash 12 dead even, exactly as a ~1834-class engine should. So
    **Stash is NOT crippled and the harness is sound.**
  - **`current`: 90.50% / +391.57 ± 59.0** (W166/L4/D30, Ptnml [0,0,5,28,67]); all
    200 terminations normal (Stash mated in 166). Same-box, same-opponent delta over
    t19 ≈ **+383 Elo**, agreeing with the +345/+355 self-play. External Elo does NOT
    self-inflate, so the corroboration is decisive.
  - **Rating bracket (AMD, 200g each) — two anchors AGREE at ~2285 Stash-ladder Elo:**
    vs **Stash 12.0** (1886) 90.50% / +391.57 → ~2277; vs **Stash 13.0** (1972)
    86.75% / +326.42 ± 55.05 (156W/9L/35D, all normal) → ~2298. Both saturate, but
    the two estimates agreeing within ~20 is the real check. ≈ **+450 over t19's
    ~1834**. Artifacts `gauntlet/mlf_vs_stash12.pgn`, `gauntlet/mlf_vs_stash13.pgn`,
    `gauntlet/t19_vs_stash12_control.pgn`.
- **Status: ALL CHECKS PASSED → SHIP.** This *reclassifies the item from a "pruning
  refinement" to a LATENT SEARCH-CORRECTNESS BUG FIX* (hence bug/search): node-level
  `return alpha` pruned interior **PV nodes** (only `isRoot` excluded) + tactical
  nodes ≤3-ply, without searching a single capture/promotion/check. Like #44, it was
  **invisible across the entire t5→t19 ladder** and only surfaced when the futility
  *structure* was A/B'd. **The largest single gain in program history.** Flipped
  flag default → ON and tagged `baseline-t20`.

### Pruning-stack audit (opened by the #45 result, CLOSED 2026-07-04)

If node-level futility leaked ~+350, the sibling forward-pruning levers — all
written in the same era, same node-level style — became prime suspects for the
same latent-leak class. Audited each behind its own flag, two-machine SPRT vs t23:

- **Razoring** (`RAZORING_MARGIN=400`, depth 2–4, `!in_check`, `!isRoot`): drops
  depth by 1 when `eval + 400 < alpha`. Node-level, no PV guard, fires to depth
  4 — same shape as the bug just fixed. Highest suspicion.
  **Tested 2026-07-02 (`copilot/fix50-for-razoring-off`, `ENABLE_RAZORING`) —
  PARKED, sign-split.** Two-machine SPRT vs t23 (10+0.1, 1t, 64MB,
  noob_3moves.epd, 1000g each), razoring fully removed: AMD **+5.56 ± 14.83**
  (nElo +8.08), 50.80% (W258/L242/D500), LOS 76.89%, Ptnml [28,115,200,127,30],
  LLR 0.25 — undecided, barely positive-of-center. Intel **−10.08 ± 15.90**
  (nElo −13.67), 48.55% (W244/L273/D483), LOS 10.67%, Ptnml
  [39,134,180,111,36], LLR −1.54 — undecided, leaning negative. Both CIs span
  zero, opposite signs — a clean sign-split, does not clear the ship bar.
  **Razoring earns its keep**: the highest-suspicion pruning lever from the
  #45-era audit turns out sound at this TC. Branch parked.
- **Reverse futility / static null** (`REVERSE_FUTILITY_MARGIN=80`/ply, depth
  ≤6, returns `eval - margin` ≥ beta): returns beta-side; checked whether it
  prunes PV nodes.
  **Tested 2026-07-02/03 (`copilot/fix50-for-rfp-pv-guard`,
  `ENABLE_RFP_PV_GUARD`) — REJECTED, two-machine H0.** Two-machine SPRT vs t23:
  AMD **−12.86 ± 15.41** (nElo −18.00), 48.15% (W235/L272/D493, 1000g), LOS
  5.07%, Ptnml [43,123,185,126,23], LLR −1.91 (undecided at cap, trending H0);
  Intel **H0 ACCEPTED −22.47 ± 15.32** (nElo −32.35), 46.77% (W206/L268/D486,
  960g), LOS 0.20%, Ptnml [39,127,193,99,22], LLR −2.95 (crossed the lower
  bound, decisive). Same-sign negative both legs. **Reading: RFP was NOT
  hiding a #45-class leak at PV nodes** — exempting it there just gives up
  cheap beta cutoffs the search actually needed, despite cutting the tree
  20–43% at fixed depth (fewer nodes, worse Elo). Reverse futility checks out
  as sound as-is; branch rejected.
- **The two #45 knobs deliberately left on the table** (now that the node→move
  change alone won big): narrow the depth envelope (`<=3` → `==1`/`<=2`) and
  add a **PV-node guard** (`beta > alpha + 1`, Fruit's exact recipe).
  - **Knob (a) tested 2026-07-03 (`copilot/fix50-for-futility-depth2`) —
    PARKED, clean two-machine neutral.** Two-machine SPRT vs t23 (10+0.1,
    1t, 64MB, noob_3moves.epd, 1000g each): AMD **+2.43 ± 15.34** (nElo
    +3.42), 50.35% (W262/L255/D483), LOS 62.21%, Ptnml [34,114,197,121,34],
    LLR −0.13; Intel **−2.78 ± 15.48** (nElo −3.87), 49.60% (W257/L265/D478),
    LOS 36.23%, Ptnml [36,119,197,113,35], LLR −0.73. Both as flat as a
    result gets — exactly the "may have already saturated" outcome flagged
    when the knob was parked. Costs +27.6% nodes at fixed depth (startpos
    d15, 27.87M vs t23's 21.84M) for ~0 Elo either direction — dead weight.
  - **Knob (b) tested 2026-07-02/03 (`copilot/fix50-for-futility-pv-guard`)
    — REJECTED, two-machine H0.** Two-machine SPRT vs t23: AMD **−12.86 ±
    15.50** (nElo −17.89), 48.15% (W222/L259/D519, 1000g), LOS 5.17%, Ptnml
    [39,136,173,127,25], LLR −1.89 (undecided, trending H0); Intel **H0
    ACCEPTED −27.33 ± 16.79** (nElo −38.72), 46.07% (W180/L245/D403, 828g),
    LOS 0.07%, Ptnml [35,116,164,77,22], LLR −2.95 (crossed the lower bound,
    decisive). Same-sign negative both legs, same verdict as sibling
    rfp-pv-guard: exempting ALL PV nodes from futility nearly doubles the
    tree (+90% startpos d15, 41.56M vs 21.84M) yet loses ~27 Elo — Fruit's
    exact PV-exemption recipe doesn't transfer cleanly onto Huginn's
    PVS-window proxy for PV-node detection. Futility's PV-node pruning
    earns its keep; rejected. **Note:** this branch initially had a
    CMakeLists.txt bug that silently compiled the flag as OFF in a reused
    build directory (caught before any games ran, fixed, verified — see
    [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md)'s trap note); the
    result above is from the corrected build.

**Conclusion:** of the four pruning-stack-audit suspects, none hid a #45-class
leak — razoring and RFP both check out as sound (razoring: sign-split =
inconclusive-but-not-harmful; RFP: actively earns its keep), and both #45
follow-up knobs (depth-narrowing, PV-guard) were either neutral or harmful.
The audit's hypothesis (same-era, same-style code hiding more leaks) did not
pan out beyond the original #45 find — a useful negative result.

## #37: Board-desync illegal bestmove — GUARDED + INSTRUMENTED, root cause OPEN

*(Item remains open — see [BACKLOG.md](BACKLOG.md) for current status. Detail
of the guard/instrumentation work, done 2026-06-16, is recorded here since it's
long-settled infrastructure, not an active thread.)*

A rare, **time-dependent** make/unmake imbalance can leave the engine's internal
board desynced from the real root position, so the search can return a move that
is illegal in the actual position (observed as `b2b4` in round-6 vs t14 on
Intel; fastchess forfeits on it).

- **Guarded** (`390ea13`): the UCI boundary validates the returned move against
  a freshly generated legal list for the clean pre-search position; on a miss it
  substitutes a legal move (never forfeit), restores the clean board, and logs
  the exact FEN + offending move. So the engine can no longer *emit* an illegal
  move. (#36, the cosmetic PV-display variant, closed separately at `a9173ad`.)
- **Instrumented** (2026-06-16): `Position::is_consistent()` validates derived
  caches, material, king squares, `at_sq64()` agreement, and full Zobrist
  recomputation against the per-piece bitboards. Search now has optional
  `ENABLE_SEARCH_INTEGRITY_ASSERTS` diagnostics that assert after make/unmake
  boundaries and verify recursive child searches return the position unchanged.
  Default Release builds keep this OFF (no Elo/NPS impact); enable with
  `-DENABLE_SEARCH_INTEGRITY_ASSERTS=ON` for repro runs.

## SPRT queue — branches off baseline-t23 (2026-07-02, CLOSED 2026-07-04)

**Queue closed. `baseline-t24` = t23 + see-ordering (#6) + root-twofold-avoid
(#44 f/u), both two-machine H1-accepted as a combined candidate (AMD +48.84±
20.36 LOS 100% @580g; Intel +66.33±23.61 LOS 100% @440g — pooled W345/L181/
D494 = 58.04% / 1020g). See [BASELINE_LADDER.md](BASELINE_LADDER.md) for the
full t24 writeup.**

Eleven branches implemented + adversarially verified in one session, originally
off `baseline-t22`. **#50 (below) shipped directly to `main` ahead of the queue
— a correctness bug, not a feature — and `main` was tagged `baseline-t23`
immediately after.** Because #50 shifts the `init_zobrist()` RNG draw sequence
(inserting a whole extra table row changes every subsequently-drawn key, not
just the black-king ones), every remaining branch was **rebased onto t23** by
GitHub Copilot as `copilot/fix50-for-*` — diff-verified byte-identical feature
content to the originals (feature diff vs `experiment/*`, excluding
`zobrist.hpp`, is empty for all 10). Full procedure + the t23 signature numbers:
[SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md).

| Branch | Item | Flag | d14 nodes vs old t22 12.04M | Note |
|---|---|---|---|---|
| ~~`experiment/fix-nondet-50`~~ → **`main` (`baseline-t23`)** | #50 | *(none — shipped unconditionally)* | n/a (t23 IS the new reference) | **SHIPPED** — latent correctness bug (TT collisions), #44/#45 class. AMD H1 @872g, +33.97±16.60, LOS 100% |
| ~~`copilot/fix50-for-see-ordering`~~ → **`main` (`baseline-t24`)** | #6 | `ENABLE_SEE_ORDER_SPLIT` | −41% vs t23 | **SHIPPED — two-machine H1-accept.** AMD +49.54±20.55, LOS 100% @586g; Intel +29.02±15.17, LOS 99.99% @996g. First clean two-machine win of the queue. |
| `copilot/fix50-for-razoring-off` | #45-audit | `ENABLE_RAZORING` (**default 0 = test arm**) | 10.57M (−12%) | **PARKED** — sign-split: AMD +5.56±14.83 (LOS 76.9%) / Intel −10.08±15.90 (LOS 10.7%), both CIs span 0. Razoring earns its keep. |
| `copilot/fix50-for-rfp-pv-guard` | #45-audit | `ENABLE_RFP_PV_GUARD` | 7.89M (−34%) | **REJECTED — two-machine H0.** AMD −12.86±15.41 (LOS 5.07%); Intel **H0 ACCEPTED −22.47±15.32, LOS 0.20%** @960g (LLR crossed −2.94). |
| `copilot/fix50-for-futility-depth2` | #45 knob a | `ENABLE_FUTILITY_DEPTH2` | +27.6% d15 nodes vs t23 | **PARKED — clean two-machine neutral.** AMD +2.43±15.34 (LOS 62.21%); Intel −2.78±15.48 (LOS 36.23%). |
| `copilot/fix50-for-futility-pv-guard` | #45 knob b | `ENABLE_FUTILITY_PV_GUARD` | 38.3M d14 (post-CMake-fix; +90% d15) | **REJECTED — two-machine H0.** AMD −12.86±15.50 (LOS 5.17%); Intel **H0 ACCEPTED −27.33±16.79, LOS 0.07%** @828g. |
| `copilot/fix50-for-tt-aging` | #42 | `ENABLE_TT_AGING` | ~same (startpos) | **INCONCLUSIVE, weak positive lean.** AMD +0.69±15.24 (LOS 53.56%); Intel +11.12±15.01 (LOS 92.71%). LTC check recommended — see BACKLOG.md, item remains open. |
| `copilot/fix50-for-drawishness-scaling` | roadmap | `ENABLE_DRAWISHNESS_SCALING` | ~same (startpos) | **PARKED — two-machine flat neutral vs t23 (as predicted).** AMD +1.04±15.10 (LOS 55.38%); Intel +4.52±15.25 (LOS 71.94%). |
| ~~`copilot/fix50-for-root-twofold-avoid`~~ → **`main` (`baseline-t24`)** | #44 f/u | `ENABLE_ROOT_TWOFOLD_AVOID` | same (inert w/o history) | **SHIPPED — cross-machine agreement.** AMD +12.51±15.22 (LOS 94.68%), Intel +7.99±15.12 (LOS 85.01%). |
| `copilot/fix50-for-trapped-bishop` | #20 | `ENABLE_TRAPPED_BISHOP` | ~same | **PARKED — clean two-machine neutral, as predicted.** AMD +2.08±14.96 (LOS 60.77%); Intel −5.21±14.91 (LOS 24.65%). |
| `copilot/fix50-for-pext` | #32 | `ENABLE_PEXT` | identical (verified) | **PARKED on this box — PEXT is 3–5% SLOWER, not faster** (AMD 7800X3D/Zen4, interleaved A/B). Node counts bit-identical (203/203 tests). |

## #50: Zobrist black-king row OOB — SHIPPED to `main` = `baseline-t23` (2026-07-02)

Surfaced during the #48/#49 verification runs as a run-to-run Kiwipete d14
node-count wobble (±~1k in 4.7M; startpos byte-identical); present at t21.
**Root cause (2026-07-02, found by the diagnosis agent via per-node key traces
+ a per-process dump of the key tables):** an **out-of-bounds read of the
Zobrist piece table.** Every keying site (`position.hpp` make/unmake
primitives, `Zobrist::compute`, `Position::is_consistent`) computes
`row = int(PieceType) + (Black ? 6 : 0)` with Pawn=1..King=6 — White rows
1–6, Black rows 7–12, row 0 unused = **13 rows** — but `zobrist.hpp` declared
`Zobrist::Piece[PIECE_NB=12][64]`. The **black king (row 12) reads 64 U64s
past the table** — whatever globals the linker parked there:

- Phantom slot 29 (black king on **f4**) lands on an **ASLR'd heap pointer** →
  a fresh per-process value XORed into every key with the black king on f4 →
  per-process TT collision patterns → the run-to-run wobble. Startpos d14
  never walks the black king to f4 (stays deterministic); Kiwipete's deep
  check-extension lines do.
- Phantom slots 1–3/30/31 (black king on **b1/c1/d1/g4/h4**) read **constant
  0** — the black king contributes NOTHING to the key on those squares, so
  positions differing only in that placement **hash identically: genuine TT
  key collisions**, a correctness hazard beyond nondeterminism (same latent
  class as #44/#45, invisible to every incremental A/B because both arms
  carried it). Remaining phantom slots read adjacent constants (Side/Castle/
  EpFile + attack-table data) — deterministic but unintended key material
  that can alias other hash components.
- **Exonerated experimentally:** time/input (checkup compiled to a bare
  return → wobble persists), Syzygy (stub + gate), #47 clock reads
  (output-only under `go depth`).

**Fix:** `PIECE_NB` 12→13, **shipped unconditionally, no flag** (pure UB with a
single correct value, not a tunable behavior). Verified: 15× Kiwipete d14 now
constant, tests green. Node counts legitimately differ from t22 (black-king
keys changed → different TT interactions). **AMD SPRT vs t22: H1 ACCEPTED
@872g — +33.97 ± 16.60, LOS 100%** (54.87%, W267/L182/D423, Ptnml
[20,86,160,129,41], LLR 3.01; artifacts `gauntlet/fix50_*`). Third
#44/#45-class latent bug to convert to Elo. **This is `baseline-t23`.**

## #48/#49: Speed pair — SHIPPED `baseline-t22` (2026-07-01)

- **#48 — Kill the double TT probe (move ordering).** `pick_next_move` takes
  the TT move as a parameter (0 = none) from `AlphaBeta`'s node-entry probe;
  quiescence (which has no entry probe) does one pre-loop probe — same probe
  count as before there, one fewer everywhere else. Verified identical
  startpos d15 node counts (18,223,597, deterministic both arms), 203/203
  tests, interleaved A/B nps +0.8% (startpos d15) / +4.0% (Kiwipete d14).
  Original analysis: uProf (2026-06-28, time-based sampling of a Kiwipete
  `go movetime` workload) put `pick_next_move` at #2 self-time (behind
  `evaluate`, ahead of `AlphaBeta`). It does NOT re-score per pick (scores
  once at `move_num==0`, then cheap selection scans — sound). The real cost:
  `AlphaBeta` already probes the TT at node entry and has `tt_best_move`,
  then `pick_next_move` probes the TT **again** only to get the ordering move
  — two cache-miss-prone probes per node.
- **#49 — Fuse king-safety attacker scan into the mobility pass.** King-zone
  attacker units are accumulated in the mobility loop via a per-side
  `ks_accum` lambda (both the safe-mobility and flat fallback branches);
  the new `king_danger_mg(pos, c, units)` applies the square/cap/shelter
  finalization at the taper; the standalone scan is gone. Verified identical
  startpos d15 node counts (18,223,597, deterministic both arms), 203/203
  tests incl. the symmetry suite; interleaved A/B nps +8.2% (startpos d15) /
  +5.5% (Kiwipete d14) — above the ~3% uProf estimate (the fused loop also
  drops redundant per-piece popcount setup, and the queen's two magic lookups
  per king are gone). Original analysis: `king_safety_white_mg`'s
  `danger_for` lambda computed a magic slider attack set for every enemy
  bishop/rook/queen (queen = two lookups), for both kings — a magic lookup
  for every slider on the board, every eval, at every leaf ≈ ~3% of total
  time. Those exact attack sets are already computed by the mobility term.
- **Two-machine SPRT vs t21 (batched, 10+0.1, 1t, 64MB, noob_3moves.epd) —
  SHIPPED t22, both legs positive (cross-machine-agreement bar):**
  AMD **+14.60 ± 15.69** (nElo +20.07), 52.10% (W285/L243/D472), LOS 96.62%,
  Ptnml [36,104,175,152,33], LLR 1.23, `gauntlet/huginn_vs_t21_amd.pgn`;
  Intel **+18.08 ± 14.44** (nElo +27.03), 52.60% (W260/L208/D532), LOS 99.31%,
  Ptnml [17,110,216,118,39], LLR 1.83, `gauntlet/huginn_vs_t21_intel.pgn`.
  Pooled W545/L451/D1004 = **52.35% / 2000g**.

## #46/#47: TT-clear-on-newgame + time-management fix — SHIPPED `baseline-t21` (2026-06-27)

- **#46 — TT not cleared between games (`ucinewgame`).** `reset()` only reset
  2 flags (and runs per-`go`), so the TT/history were never wiped on newgame
  → stale prior-game entries probed on transpositions (cross-game #44
  hazard; "100% full from game 2"). Clear moved into the ucinewgame handler.
- **#47 — Time management under-uses the clock.** Iteration-start gate bailed
  at budget/4 (assumed next iter = 3× elapsed), leaving ~75% of the clock
  unused (finished a 5+2 game +2:48 on the clock); → budget/2 (EBF≈2) +
  alloc inc/4→inc/2. Caught by watching a real game — invisible to blitz A/B
  (hit both arms equally).
- **Bundled validation: +126.97 ± 24.60 vs t20 (10+0.1, 400g, LOS 100%, zero
  forfeits).**

## #38: Displayed PV continues past the fifty-move rule (FIXED 2026-06-16)

- **Status:** fixed as a pure display change (zero Elo impact): the PV-display
  walk now keeps the move that reaches `halfmove_clock >= 100`, then truncates
  any triangular-PV or TT-walk tail after it. This mirrors the repetition
  truncation logic and prevents fastchess warnings like `PV continues after
  fifty-move rule`.
- **Regression guard:** `PVDisplay.TruncatesTTHorizonAtFiftyMoveRule` seeds a
  stale same-key TT continuation behind a forced halfmove-99 root move and
  verifies the emitted UCI `info ... pv` line stops after the draw-creating move.

## #17: Aspiration windows at the root — REJECTED @ t15 (2026-06-14)

Implemented (`228817b`, MTLChess-guided): ±50cp window around the previous
iteration's score, geometric (×2) widening on fail, full window at shallow
depths. Wrapped the existing root PVS loop. 197/197 tests pass.

**SPRT vs t15 (AMD, 10+0.1): H0 ACCEPTED @ 660g — −33.8 ± 18.0 Elo, LOS 0.01%,
LLR −2.98**, W123/L187/D350 (45.15%), Ptnml [28,94,131,68,9].
**SPRT vs t15 (Intel, 10+0.1): H0 ACCEPTED @ 756g — −29.95 ± 17.5 Elo, LOS 0.04%,
LLR −2.96**, W165/L230/D361 (45.70%), Ptnml [35,103,147,78,15]. Same-sign
two-machine rejection — the regression is real and machine-independent (PGN
`gauntlet/huginn_vs_t15_intel.pgn`).

**Reverted; preserved on branch `experiment/aspiration-root` (`228817b`).** This
is the **key new datapoint**: aspiration regresses even on the much stronger t15
base — the "a smoother/stronger eval will stabilize inter-iteration scores and
unblock aspiration" hypothesis is **falsified at t15**. Consistent with the #8
history (−24…−75 across tunings) and with the #41 finding that Huginn's gap is
*eval quality*, not search efficiency: a narrow-window speed lever can't pay when
the re-search tax from score swings exceeds the pruning saved at d~10. **Lesson:
search-efficiency levers are the wrong track for an eval-bound engine at this
TC.** Related: #8 (aspiration step b, parked).

## #20: Trapped-bishop eval pattern — TESTED, PARKED (2026-07-03)

Scoped (2026-06-14) from the CPW-engine `EvalBishop` reference: six
square+enemy-pawn locks per side — A7/H7 (b6/g6), B8/G8 (c7/f7), A6/H6
(b5/g5, lighter penalty) and the a2/h2 mirrors. Real signal mobility *can't*
capture: a cornered bishop is lost for a pawn, but Huginn's mobility
(`bishop_attacks(sq,occ) & ~own`) still credits it for "seeing" enemy squares.
**Skip CPW's `RETURNING_BISHOP`** (bonus for a bishop on f1/c1 with own king on
g1/b1) — it rewards an *undeveloped* bishop and fights `DEVELOP_BONUS`.

**Implemented + tested 2026-07-03 (`copilot/fix50-for-trapped-bishop`,
`ENABLE_TRAPPED_BISHOP`) — PARKED, clean two-machine neutral exactly as
predicted.** Six square+enemy-pawn locks per side, two tapered tiers
(`P_BISHOP_TRAPPED_A7_MG/EG` seeded 100/120, `P_BISHOP_TRAPPED_A6_MG/EG`
seeded 50/60), wired into `tools/texel/tuner.cpp`. `RETURNING_BISHOP`
deliberately not ported, per the original scoping note. Flag verified
genuinely live (207/207 tests incl. 4 new `EvalTrappedBishop` cases; a direct
eval A/B confirmed the penalty fires: bishop trapped a7/pawn-b6 = 204cp vs the
same position with the pawn on b5 (bishop free) = 250cp, a 46cp discount in
the right direction). **Two-machine SPRT vs t23 (10+0.1, 1t, 64MB,
noob_3moves.epd, 1000g each):** AMD **+2.08 ± 14.96** (nElo +3.00), 50.30%
(W252/L246/D502), LOS 60.77%, Ptnml [26,131,183,131,29], LLR −0.17; Intel
**−5.21 ± 14.91** (nElo −7.53), 49.25% (W243/L258/D499), LOS 24.65%, Ptnml
[34,121,195,126,24], LLR −1.04. Mild opposite lean, both legs noise-level —
exactly the "~1–3 Elo, needs large N" outcome this item always expected. **Not
folded into t24 on current evidence**; a full Texel re-tune with these params
exposed (rather than the hand-set seed values) remains the better long-term
path if revisited.

## #32: PEXT slider attacks — TESTED, PARKED (2026-07-03/04)

Build-gated with a magic fallback (BMI2 boxes). **AVX-512 assessment
(2026-06-26): not a lever for the current HCE engine.** The build already
compiles `/arch:AVX2` (MSVC) + `-march=native -mtune=native` (GCC/Clang), so
AVX2 + per-CPU tuning is *on*; no free SIMD on the table. AVX-512 wins when
you do the same op on 8+ independent 64-bit lanes at once — but Huginn's hot
paths (movegen, eval, search) are latency-bound and branchy: bitboard ops are
single scalar instructions, `popcount`/`ctz` already map to one hardware
instruction each and can't batch, and magic-slider lookups are single loads
(AVX-512 gather is *slower*). Verdict: the HCE engine doesn't vectorize;
AVX-512's home run is NNUE inference (#39), not a standalone speed item. PEXT
was still identified as the one real modern-AMD speed lever: BMI2, fast on
Zen4, can beat magic-multiply. **Caveat:** consumer Intel 12th–14th gen has
AVX-512 fused off, so any AVX-512 build is AMD-leg-only.

**Implemented + tested 2026-07-03/04 (`copilot/fix50-for-pext`,
`ENABLE_PEXT`) — PARKED, the expected win did NOT materialize on this box.**
`_pext_u64` table indexing replaces the magic multiply+shift for rook/bishop
lookups; init fills tables with pext ordering instead of the magic search;
`verify_or_die()` validates the active indexing against the ray-walker at
startup either way. 203/203 tests pass (perft would catch any attack-table
error instantly) and node counts are **bit-exact identical** between arms as
required (startpos d15 = 21,844,725, Kiwipete d14 = 4,665,682, both arms).
**Interleaved A/B nps (AMD 7800X3D/Zen4, 5×5 runs): PEXT is 3–5% SLOWER, not
faster** — startpos −2.87%, Kiwipete −4.90%. Contrary to the "BMI2 beats
magic-multiply on Zen4" assumption: the hot path here is memory-latency-bound
(the attack-table load dominates), not ALU-bound, so replacing a cheap
multiply+shift with a single `pext` doesn't help when the bottleneck is the
load itself — and the extra instruction-decode/scheduling overhead of `pext`
can net slightly negative. **Not adopted on this machine.** An Intel-side
check would need its own measurement, but low prior given this result — the
correctness/portability infrastructure (build-gated PEXT with a verified
magic fallback) is sound and kept behind the flag for future revisit on
different hardware.
