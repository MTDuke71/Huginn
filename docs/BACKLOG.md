# BACKLOG (Huginn 2.1+)

> **Lean working backlog** — open and in-progress items only, plus a deferred-
> ideas list. The full 2.0-era history (every closed item, with full detail,
> evidence, and the round-by-round eval program) lives in
> [BACKLOG-archive-2.0.md](BACKLOG-archive-2.0.md). Issue numbers are preserved
> so cross-references into the archive still resolve.

## Current state (2026-06-13)

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
- **Active thread:** the **#9 / #35 eval program** — round 8 is next (threats
  round 2 or safe mobility, per the #41 roadmap).
- **Direction (2026-06-13): push pure HCE as far as it goes before reaching for
  multithreading or NNUE.** The +490 over 2.0 was mostly foundational repair
  (structural bugs, a never-tuned eval) and won't recur — but pure-HCE engines
  (Fruit 2.1 ≈ 2780 CCRL, Toga, early Stockfish) prove the HCE ceiling sits
  *hundreds* of Elo above Huginn's ~1818. So the road ahead is incremental HCE
  terms (see the round-7+ roadmap under #9/#35), with the two big architectural
  levers (#39 NNUE, #40 Lazy SMP) deliberately deferred.
- See [CLAUDE.md](CLAUDE.md) for the full baseline ladder and
  [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for the live search/eval inventory.

## Open / in-progress

| # | Title | Status | Type | Priority |
|---|-------|--------|------|----------|
| 9 / 35 | Texel eval program + tapered eval | **IN-PROGRESS** — t10→t16 shipped (t16 = king safety, #2); round 8 next | feature/eval | high |
| 41 | Played-game calibration study (round-7 evidence + harness) | **DONE** (2026-06-14) — sets round-7 order | research/eval | high |
| 43 | NMP soundness/refinement round (verification + scaled R + MDP) | **OPEN** (2026-06-15) — Stash-v13 + #41 + complexity-gate all point here | feature/search | high |
| 37 | Board-desync illegal bestmove | **GUARDED**; root cause OPEN (needs repro) | bug | high |
| 38 | Displayed PV continues past fifty-move rule | **OPEN** — cosmetic | bug | low |
| 5  | Recalibrate vs external opponents (CCRL scale) | **OPEN** | maintenance | medium |
| 17 | Aspiration windows at the root | **REJECTED** @ t15 (−33.8 Elo, H0) — reverted, parked | feature | low |
| 31 | TT-size (`Hash`) SPRT sweep | **OPEN** | tuning | low |
| 32 | PEXT slider attacks (build-gated) | **OPEN** | speed/research | low |
| 34 | Pin/blocker-aware legal movegen | **OPEN** | speed/research | low |
| 42 | TT aging (dates) + clusters (Fruit/Toga design) | **OPEN** — idea, pairs with #31 | feature/search | low |
| 39 | NNUE evaluation | **DEFERRED** (HCE first) — big lever | feature/eval | — |
| 40 | Lazy SMP / multithreading | **DEFERRED** (HCE first) — big lever | feature/speed | — |
| 19 | Two-machine gauntlet workflow + SPRT | **ESTABLISHED** (reference) | tooling | — |

### #9 / #35: Eval program — Texel tuning + tapered eval (IN-PROGRESS)

The main strength thread. A `game_phase_256` tapered eval foundation (#35) plus
a Texel tuner (`tools/texel/`, `bake.py` rewrites headers from the tuner dump,
verified exact) drive successive full-vector re-tunes / new-feature rounds on
the 725k Zurichess quiet-labeled corpus. Shipped ladder:

- t10: tapered-eval foundation (+39.5 pooled vs t9)
- t11: first Texel tune — material + all PSTs + king-EG (+71.4 pooled vs t10)
- t12: tapered EG PSTs + tunable mobility (+37.4 AMD vs t11)
- t13: connected + backward pawns (+18.9 pooled vs t12)
- t14: rook-on-7th (sign-split +6.6, shipped as a logged exception)
- t15: **threats** (+54.2 pooled vs t14 — largest eval-term ship)
- t16: **king safety** (+10.1 pooled vs t15 [AMD +20.5 LOS 99.6% / Intel −0.35] —
  first KS ship; converted far above its MSE drop)

Method note (learned across rounds): **new-feature MSE converts to Elo better
than re-fit MSE** (re-fitting existing terms hit a floor at round 3, flat);
SPRT decides every round. **King safety (t16) is the exception that proves a
corollary: quiet-corpus MSE UNDER-states terms that pay in sharp positions** —
KS's tiny −0.00014 MSE drop converted to +20 on AMD, because king danger matters
most in the attacking positions a quiet-labeled corpus excludes. Full
round-by-round detail in the archive (PRODUCTION TUNE 1–6).

**HCE roadmap (round 7+).** Remaining hand-crafted-eval levers. Add a new term →
wire it into `collect_params()` → tune → bake → two-machine SPRT. Each is its
own round.

**Priority order is now data-backed — set by the #41 calibration study
(2026-06-14): king safety → threats round 2 → safe mobility → outposts.** The
study found Huginn's gap is *positional move-selection in balanced middlegames*
(systematic +44cp over-optimism), not tactics or endgames — which is exactly the
king-safety/threats shaped hole. Outposts stays the lowest-risk warm-up, but KS
is the bigger lever.

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
- **Mobility refinement (safe mobility)** — currently flat square-count × weight;
  go per-piece-type and restrict the area to *safe* squares (exclude enemy-pawn-
  attacked). #41 ties the Queen-error cluster to this (stop crediting a queen on
  a square enemy minors attack).
- **Outposts** *(low-risk tuner warm-up; #41 Knight-error cluster)* —
  knight/bishop on a hole the enemy can't hit with a pawn, supported by an own
  pawn. Constants (`KNIGHT_OUTPOST_BONUS`, the rank bounds) are already defined
  in `evaluation.hpp` but **dead/unused** — part new-term, part cleanup. Cleanly
  tunable, low risk, ~+5–20.
- **Passed-pawn refinements** — king distance to the passer (own + enemy),
  blockade, rook-behind-passer. Endgame Elo — **deprioritized**: #41 shows
  balanced-endgame play is already solid (fair-fight cp-loss 13.3).
- **Drawishness scaling (`mul[]`)** — opposite-coloured bishops →½, KNNK→0,
  "a minor up, no pawns"→⅛. High impact, cheap, independent; kills the
  "scores a dead-drawn endgame as winning" class. **Not** sigmoid-tunable (it's
  a final-eval multiplier) — hand-set from known values + SPRT to confirm.
- **Lower priority:** doubled-rooks / blind-pig follow-up to t14's rook-on-7th;
  space (safe squares behind own centre pawns); rook-on-king-file; specific
  endgame recognizers (KPK, KRKP); trapped-bishop (#20, parked).

### #41: Played-game calibration study — round-7 priority evidence (DONE 2026-06-14)

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

### MTLChess v0.6 source comparison (2026-06-14) — adds a *search* track to #41

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

### #43: NMP soundness/refinement round (OPEN, 2026-06-15)

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
     genuinely SPRT-worthy. **NEXT: fixed-depth + fixed-time two-machine SPRT;
     ship → flip the default to 1.** Tunables if neutral: the depth-10 / phase-96
     gates, the `depth − R` verify depth. Build the test arm with
     `-DENABLE_NMP_VERIFICATION=1` (see `build/msvc-nmpon`).
2. **Eval/depth-scaled R** — replace flat 4 with e.g. `R = 4 + (eval−beta)/200`
   (capped) and/or `+ depth/6` (Stash v13: "more aggressive reductions when eval
   is way above beta").
3. **Mate-distance pruning** — clamp α/β to mate bounds at node entry, cut if
   α≥β. Cheap, sound, free; Huginn does TT mate-score adjustment but no
   node-entry MDP. Bonus: faster mate-finding helps the **conversion weakness**
   the #5 Stash RR exposed (Huginn under-converts won positions vs weaker
   engines).

**Caveat (why this isn't a slam dunk):** search levers have a mixed Huginn record
— #17 aspiration −34, #6 SEE ~neutral. But those were *efficiency/ordering*
levers; #43 is *soundness/adaptive*. The R=3 pruning-soundness test the
complexity gate already calls for folds in here naturally. Two-machine SPRT to
ship, like any change.

### #37: Board-desync illegal bestmove — GUARDED, root cause OPEN

A rare, **time-dependent** make/unmake imbalance can leave the engine's internal
board desynced from the real root position, so the search can return a move that
is illegal in the actual position (observed as `b2b4` in round-6 vs t14 on
Intel; fastchess forfeits on it).

- **Guarded** (`390ea13`): the UCI boundary validates the returned move against
  a freshly generated legal list for the clean pre-search position; on a miss it
  substitutes a legal move (never forfeit), restores the clean board, and logs
  the exact FEN + offending move. So the engine can no longer *emit* an illegal
  move. (#36, the cosmetic PV-display variant, closed separately at `a9173ad`.)
- **Root cause still open.** Not reproducible cold / warm-TT / faithful replay —
  only under real time pressure. Audited every make/unmake site + the primitives;
  all balanced on inspection. **Next:** instrument a debug build with a per-node
  board-integrity assertion (bitboard self-consistency + `at_sq64` agreement),
  run self-play until it trips, then bisect from the captured FEN/move. The
  shipped diagnostic will hand us a deterministic repro the next time it fires.

### #38: Displayed PV continues past the fifty-move rule (cosmetic)

- status: **OPEN** — cosmetic (display-only; `bestmove` is always legal, zero
  Elo impact). Exact analogue of the closed #21 (threefold-rep PV truncation).
- **Symptom:** the PV-display walk truncates at *repetition* (the seen-set) but
  not at the *50-move-rule* horizon, so a PV that runs into a 50-move draw still
  prints past it (fastchess: `Warning; PV continues after fifty-move rule`).
- **Evidence:** in the t15/2.1 vs MTLChess v0.3 calibration (300 games), this
  was Huginn's **only** remaining warning class — 7 occurrences across 2 games
  (0 illegal-move, 0 illegal-PV, 0 threefold-PV; #37/#36/#21 all clean).
- **Fix (~3 lines):** in the PV-display loop ([search.cpp](../src/search.cpp)),
  stop appending once `halfmove_clock` would reach 100 along the walk, mirroring
  the repetition seen-set truncation. Pure display change — batch with the next
  baseline build (don't rebuild mid-gauntlet).

### #5: Recalibrate vs external opponents (OPEN)

Re-anchor the CCRL-Blitz-scale Elo estimate. Scripts: `test_huginn_gauntlet.bat
<snowy|cdrill|mtl03|mora|mtl05|mtl06>`; the archive (#5) has the t11 3-anchor
MLE (~1818).

**Round-robin (2026-06-14, Intel, 10+0.1, no book, 1200g, fresh MTLChess builds):**

| Engine | RR Elo | Score |
|---|---|---|
| MTLChess v0.6 | +259.5 | 81.7% |
| MTLChess v0.5 | +143.8 | 69.6% |
| MTLChess v0.3 | −154.1 | 29.2% |
| Huginn 2.1 | −245.4 | 19.6% |

- Internal spacing (clean — all fresh builds): 2.1 is **~91 Elo below v0.3**;
  v0.3→v0.5 = ~298; v0.5→v0.6 = ~116. (The direct 2.1-vs-v0.3 head-to-head
  agreed: −74 ± 37, 39.5%.)
- **Scale check:** RR v0.3→v0.5 (~298) ≈ the CCRL gap (1984→2314 = 330) → the
  fresh builds are roughly consistent with their CCRL ratings (~10% compressed).
- **Absolute (two-anchor fit, v0.3=1984 / v0.5=2314):** Huginn 2.1 ≈ **~1880
  CCRL-Blitz**. The +9pp MTL-*favorable* non-transitivity pulls the transitive
  estimate lower (~1820–1850).
- **Finding:** on the external scale 2.1 is only ~+60 over t11 (1818) despite
  ~+130 in self-play — self-play Elo inflates. Corroborates the "+490 over 2.0
  was foundational repair; future HCE gains will be harder-won" direction.
- **Confound:** the fresh `mtlchess_v03` is NOT the old build behind the
  historical 36.8% record, so the before/after vs that record is invalid.
  **Next:** fresh-`v03` vs old-`mtlchessV3` head-to-head to quantify the shift,
  and/or broaden to non-MTL anchors (MORA ~2189, rebuild CDrill/Snowy) for a
  multi-point MLE.

**Stash calibration ladder (2026-06-14) — the preferred fix for the anchor
problem.** The current external anchors (MTLChess/CDrill/Snowy/MORA) are
sparse and *style-non-transitive* (CDrill −7.5pp bogey, MTL +9pp favorable),
so the transitive MLE smears across ~1820–1850. **Stash** removes that
confound: one actively-maintained, UCI-clean engine family with ~30 CCRL-rated
releases (v8 ≈1090 → v36 ≈3399), giving a *dense, same-author, monotonic*
ladder. Bracket Huginn (~1820) tightly and fit a local slope instead of
extrapolating across unrelated engines. CCRL-Blitz table (★ = CCRL estimate,
prefer ranked versions as primary anchors): v11 **1690**, v12 1886, v13
**1972**, v14 2060, v17 2298, v19 2473, v21 2714 …

- **Have on disk + wired into the gauntlet (3-anchor bracket):** `stash11`
  (Stash 11.0, **1690**), `stash12` (Stash 12.0, **1886** — closest, ~+66 over
  Huginn), `stash13` (Stash 13.0, **1972**). All three in the
  `test_huginn_gauntlet.bat` calib table and UCI-verified (real games, no
  stalls). **Stash is the calibration instrument going forward; CPW-engine was
  evaluated and rejected** (2014 hobby engine, init access-violation +
  first-search hang under UCI — forfeits every game; see the dropped entry in
  the gauntlet bat).

**Stash-anchored round-robin (2026-06-15, Intel, 10+0.1, cc=4) — Huginn ≈ 1770
CCRL.** 4-engine RR (Huginn 2.1 fresh build `6787430` + Stash 11/12/13), planned
2400g, **2049 games completed** (~342 per pairing) before the box slept and the
run stalled (all engines flagged on time at once = system suspend, not an engine
bug; orphaned processes were killed and the partial result kept). Artifacts:
`gauntlet/roundrobin_huginn_stash_calib_2026-06-15_intel.{pgn,log,standings.log}`.

| Engine | CCRL | RR Elo (±) | Score |
|---|---|---|---|
| Stash 13.0 | 1972 | +157 ± 22 | 71.2% |
| Stash 12.0 | 1886 | +51 ± 21 | 57.4% |
| **Huginn 2.1** | **~1770** | **−70 ± 19** | **40.1%** |
| Stash 11.0 | 1690 | −135 ± 21 | 31.4% |

Huginn head-to-head (342g each): vs Stash 11.0 **52.2%** (+15 → 1705), vs Stash
12.0 **36.0%** (−100 → 1786), vs Stash 13.0 **31.3%** (−137 → 1835).
- **Estimate: Huginn ≈ 1770 CCRL-Blitz** (direct-H2H 3-anchor mean ~1775;
  fastchess global MLE ~1755; band 1755–1786). **~45 below the old ~1818** —
  expected and healthier: the old number leaned on MTL (+9pp favorable). Stash
  is a clean neutral ladder; its internal RR spacing (v11→v13 = 297) matched
  CCRL (282, ~5% stretch), validating the ladder.
- **Finding — score compression / monotonic anchor drift.** The per-anchor
  estimate *climbs* with opponent strength (1705 → 1786 → 1835): Huginn
  over-performs a flat Elo model vs strong Stash (31.3% vs ~22% predicted) and
  under-performs vs weak Stash (52.2% vs ~59%). Reads as a **conversion
  weakness** — solid vs stronger engines but doesn't convert wins vs weaker
  ones (99 draws vs Stash 11.0). A more actionable lead than the raw rating;
  points at the drawishness-scaling / 50-move-conversion items already in the
  roadmap. The PGN holds the Huginn-vs-Stash-11 draws for a conversion audit.
- **Next:** re-run to full 2400g with sleep disabled (`powercfg /change
  standby-timeout-ac 0`) to tighten CIs; `stash14` (2060) optional for headroom.
  Then fold ~1770 into CLAUDE.md (replacing the ~1818 figure).

### #17: Aspiration windows at the root — REJECTED @ t15 (2026-06-14)

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

### #31 / #32 / #34 / #42: Speed / research (OPEN, low)

- **#31** — SPRT sweep of `Hash` (64 vs 128 vs 256 MB) at the current strength.
- **#32** — PEXT slider attacks, build-gated with a magic fallback (BMI2 boxes).
- **#34** — SF-style `blockersForKing` pin/blocker-aware legal movegen (drop the
  per-move `MakeMove` legality filter for pinned-piece fast paths).
- **#42 — TT aging + clusters (Fruit/Toga design).** Huginn's TT
  (`transposition_table.hpp`) is **single-entry-per-index, depth-preferred
  replacement, NO aging**. It persists across searches (`clearForSearch` clears
  the PV table, not the TT), so with depth-preferred-only replacement a deep
  entry stored early in a game can squat in its slot indefinitely — new shallow
  stores can't evict it (`depth >=` fails) — crowding out fresh results. Two
  portable ideas from the Fruit/Toga TT:
  1. **Date-based aging** *(the main win)* — each entry carries a `date`; a global
     date bumps once per search; `age` = searches-elapsed. Replace by value
     `age*256 - depth` (prefer old + shallow), so stale-but-deep entries become
     cheap to evict while still usable until overwritten. Fixes the squat problem
     without ever clearing the table.
  2. **Clusters** — each index addresses an N-way contiguous cluster (probe scans
     for a lock match; store replaces the least-valuable slot). Fewer collision
     evictions → higher hit rate, at the cost of a small per-probe scan.
  **Priority/caveat:** search-quality lever, and those have washed out lately
  (#17 aspiration −34, #26). Counter-point: the #23 TT *bound-fix* was +24, so TT
  isn't a dead lever — but a replacement-policy *refinement* is more marginal than
  a correctness bug. #41 says the gap is eval, not search, so this sits behind the
  eval roadmap. Good "search-quality round" candidate; pairs naturally with #31's
  Hash sweep (test aging + size together). Needs two-machine SPRT like any ship.

### #19: Two-machine gauntlet workflow (ESTABLISHED — reference)

`test_huginn_gauntlet.bat <tN|calib> [rounds] [cc]` auto-detects CPU vendor,
tags PGNs `_intel`/`_amd`, runs SPRT [0,10] for baselines. Keep cc=4 on both
boxes for poolable runs. Pentanomial pooling pairs games by `[Round]` tag. Full
notes in the archive (#19, #22).

## Big levers (deferred — HCE first)

The two architectural jumps that dwarf any single HCE term. Both deliberately
deferred (2026-06-13) to exhaust pure-HCE headroom first — but recorded here so
the strategic picture is explicit.

### #39: NNUE evaluation (deferred)

- The per-node *quality* lever — hundreds of Elo, and the honest path to closing
  the gap to Fruit/Stockfish-class strength. Replaces (or augments) the HCE
  `evaluate()` with a small efficiently-updatable network.
- **Why it fits this project:** preserves single-thread determinism (the
  fixed-depth / cross-machine-bit-identical test methodology survives), and it
  improves the thing we actually care about — move quality per node.
- **Cost:** a multi-session build — training-data generation (self-play at depth,
  labelled), the net + quantisation, and incremental-update integration into
  make/unmake. Highest conceptual lift of the options; lowest methodology cost.

### #40: Lazy SMP / multithreading (deferred)

- The biggest *Elo-at-tournament-hardware* lever short of NNUE: ~**+100–200 Elo**
  on the 8-core gauntlet boxes (currently idle — a cc=4 gauntlet just runs 4
  separate single-thread games, never one strong parallel search). Lazy SMP is a
  near-boilerplate algorithm, so lowest *conceptual* risk.
- **But three real catches** (the reason it's deferred, not done):
  1. **Buys Elo with hardware, not smarts** — per-node quality is unchanged.
     Legitimate for CCRL placement and GUI use, but orthogonal to the eval gap.
  2. **Breaks the determinism the test methodology leans on** — Lazy SMP is
     non-deterministic by construction, killing the bit-identical-at-fixed-depth
     ship checks and the cross-machine WAC determinism (#33-class).
  3. **Highest engineering risk** — needs a race-tolerant (lockless XOR-key) TT,
     per-thread history/killers, a clean multi-thread stop, and forces
     **concurrency-1 gauntlets** (cc=4 would oversubscribe the CPU and make
     10+0.1 wall-clock meaningless) → ~4× throughput hit per box. Concurrency
     bugs are the worst class to debug — and #37 (a *single*-threaded desync) is
     still open.
- **Decision:** revisit after the HCE roadmap is mined out (or alongside NNUE).

## Deferred / parked ideas

One-liners; full detail + evidence in [the archive](BACKLOG-archive-2.0.md).

- **#2 — King safety on top of mobility.** Deferred pending tapered eval — **now
  unblocked** (t10+ is tapered). Strong round-7 candidate; tune KS weights via
  the Texel harness rather than by hand (hand-tuning hit a ~0-Elo ceiling).
- **#3 — Continuation history.** 1-ply additive falsified (w16 neutral, w64 −9);
  flag off, kept in-tree (`experiment/continuation-history`).
- **#6 — Lazy SEE in main-search capture ordering.** Attempt 2 pooled +2.08
  (neutral), reverted; branch `wip/see-capture-ordering`.
- **#7 — LMP (late move pruning) fix.** Deferred (`experiment/lmp-quiet-count`).
- **#8 — Aspiration step (b), narrow-window search.** Deferred; folds into #17.
- **#20 — Trapped-bishop eval pattern** (a7/h7/a2/h2 + enemy pawn locks).
  Deferred (`experiment/trapped-bishop`). **Scoped (2026-06-14) from the
  CPW-engine `EvalBishop` reference:** six square+enemy-pawn locks per side —
  A7/H7 (b6/g6), B8/G8 (c7/f7), A6/H6 (b5/g5, lighter penalty) and the a2/h2
  mirrors. Real signal mobility *can't* capture: a cornered bishop is lost for a
  pawn, but Huginn's mobility (`bishop_attacks(sq,occ) & ~own`) still credits it
  for "seeing" enemy squares. **Port as direct bitboard mask tests** (six `if`s
  per side, no per-square switch / no piece loop) + two tapered `EVAL_PARAM`s
  (`P_BISHOP_TRAPPED_A7`, `..._A6`) so the tuner fits them like the pawn terms.
  **Skip CPW's `RETURNING_BISHOP`** (bonus for a bishop on f1/c1 with own king on
  g1/b1) — it rewards an *undeveloped* bishop and fights `DEVELOP_BONUS`; if ever
  tried, SPRT it standalone, never bundled with the trapped block.
  **Expectation: small (~1–3 Elo, SF-class), needs large SPRT N** — batch with
  another eval change or run after the higher-leverage round-8 terms. Lowest
  priority of the bishop work; the higher-ROI bishop term is a "bishop-pawns /
  bad-bishop" penalty (own pawns on the bishop's colour), not the corner trap.
- **#26 — `board64[64]` piece-on-square cache.** Deferred (a prior sign-split;
  perf/cache changes carry more downside risk than gated eval terms).
- **#27 — Unorthodox early-queen PV** (d1d3 / d8d6). Deferred (evaluation).
- **#29 — Fifty-move-rule search blindness.** P1 kept on correctness; P2
  (clock-scaled eval) deferred (near-inert at 10+0.1).

## Conventions

- **Ship bar:** same-sign two-machine SPRT H1-accept (AMD + Intel), or tight
  cross-machine agreement for a small effect. Sign-splits are parked by default
  (t14 rook-on-7th was a logged exception, not the rule).
- **Complexity gate (fixed-depth vs fixed-time + ablation) — run BEFORE each new
  eval/search round.** Guards against over-engineering: eval terms that don't
  earn their noise, and pruning that trims real tactics ("trim the tree in
  vain"). *Context:* Huginn is NOT depth-starved by eval — it runs ~3.55 Mnps vs
  MTLChess's ~2.33 and prunes harder (R=4 + futility/razoring/IID vs their R=2/3,
  none), yet loses. So MTLChess's simpler-but-stronger eval is **correlation, not
  causation**: the gap is term *quality*/search *shape*, not eval *cost*. The
  failure modes that ARE real here: (a) mis-tuned/correlated terms add eval noise
  that also degrades move ordering + LMR/futility margins; (b) over-aggressive
  pruning reaches more depth on a tree missing the move that mattered. Diagnose,
  don't guess:
  1. **Fixed-depth vs fixed-time SPRT — the key test.** Gains at fixed-depth but
     loses at fixed-time → costs more speed than it returns (true over-engineering).
     Loses at both → the term is just wrong/noisy. Neutral at fixed-depth → adds
     nothing but risk. Buckets every change cleanly.
  2. **Periodic ablation sweep** — before adding term N+1, SPRT-disable an existing
     cluster (mobility, all pawn-structure). MTLChess proves these *can* be
     near-neutral; if removing one doesn't hurt, it's dead weight adding noise.
  3. **nps-per-term regression + average-depth telemetry** at the test TC — if
     breadth creeps up and depth drifts down with no Elo, the line is crossed.
  - **Pruning-soundness audit** (the "trim in vain" lever): periodically test a
    sound-lean config (R=3, lighter razoring) at fixed time. A gain means the
    tree-trimming is over-aggressive — likely worth more than any new eval term.
  - **Principle: fewer, better, sound > many, noisy, fighting.** "Demote breadth"
    (#41 + the MTLChess comparison) is this gate applied.
- **Commit directly to `main`** (no feature branch unless asked); push only when
  asked. Gauntlet results: `gauntlet/*_<machine>.{log,pgn}` + a BACKLOG/commit
  summary carrying Elo/LOS/Ptnml.
- **Baselines are built per-machine** from the `baseline-tN` tag — never copy one
  box's binary to the other.
