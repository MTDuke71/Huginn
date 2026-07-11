# Baseline ladder

Full shipped-baseline history (newest first). Each `baseline-tN` tag is a
two-machine-confirmed (or logged-exception) ship. CLAUDE.md carries only the
current baseline + a pointer here; this file is the durable record.

Convention: baselines are built **per-machine** from the tag (never copy a
binary between boxes) and snapshotted as `huginn_tN.exe` in the fastchess dir.

---

### baseline-t27 — #57: legal-move ordinal for PVS/LMR + textbook PVS condition
= t26 + the 2026-07-09 audit's #57, shipped by flipping
`ENABLE_LEGAL_MOVE_ORDINAL` default ON (source + CMake option). Two defects
in one: (a) the move loop's pseudo-legal index `i` decided PVS first-move
treatment, the LMR lateness threshold/row, and fhf — illegal (pinned) entries
inflate it, so the first LEGAL move could get a null-window/reduced search;
(b) the PVS condition `i == 0 || alpha == best_score` gave every move after a
normal alpha improvement a full window — null-window PVS only engaged in
failing-low nodes. Fix: a searched-move ordinal incremented only after
successful `MakeMove()` drives all three sites; PVS is textbook (first legal
move full-window, all others null-window + fail-high re-search).

**AMD SPRT vs t26 (2026-07-11): H1 ACCEPT +29.98 ± 15.53** (nElo 42.31 ±
21.80), LOS 99.99%, 976g early-stop (LLR 2.97), 54.30% (W288/L204/D484),
Ptnml [22,99,184,139,44], PairsRatio 1.51. Notable: the test arm searches
+12.8% MORE fixed-depth nodes from startpos (7,484,807 vs 6,634,033, root
move d2d4 vs e2e4) and still gains at fixed time — the reshaped tree is
better, not smaller. Kiwipete d13 = 1,930,694 (−5.7%). **Shipped on AMD-only
accept** (user call, #51/#25-precedent; no Intel leg). Ship build reproduces
the test-arm signature exactly; 226/226 tests (1 by-design skip). Sixth
consecutive structural-search-bug win (#44/#45/#50/#51/#52/#57 lineage).

### baseline-t26 — 2026-07-09 audit criticals: check-aware qsearch (#52) + rule-50 TT guard (#53)
= t25 + both search-shape criticals from the independent 2026-07-09 code
audit, SPRT'd separately per the standard queue process (run-sheet:
[SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md)) and shipped together by
flipping both flag defaults ON (`ENABLE_QSEARCH_CHECK_EVASIONS`,
`ENABLE_RULE50_TT_GUARD` — source + CMake option + test mirror, all three,
each). Full findings/resolutions in [BACKLOG.md](BACKLOG.md) #52/#53.

- **#52 — check-aware quiescence** (the Elo carrier; same "blind at the
  horizon" soundness class as #44/#45). Qsearch detects check at entry; in
  check it skips stand-pat/delta/SEE and searches every evasion, returning
  `-MATE + ply` when none is legal; out of check the frontier gains quiet
  promotions; `info.ply` advances through qsearch (true mate distances +
  seldepth). **Two-machine SPRT vs t25 — both legs H1-accept:** Intel
  **+40.11 ± 18.18** (nElo 57.51), LOS 100%, 696g, 55.75% (W226/L146/D324),
  Ptnml [12,69,126,109,32], LLR 2.95; AMD **+44.67 ± 18.94** (nElo 65.80),
  LOS 100%, 610g, 56.39% (W188/L110/D312), Ptnml [7,66,97,112,23], LLR 2.97.
  Pooled **56.05% / 1306g ≈ +42 Elo**, remarkably machine-consistent.
- **#53 — rule-50-aware TT eligibility** (correctness ship, #50/#51
  precedent). No TT cutoff / no store when `halfmove_clock + depth >= 100`
  (subtree can reach the fifty-move boundary → scores are path-dependent);
  TT move still used for ordering. Blitz SPRT vs t25 was a **sign-split**:
  Intel −18.08 ± 15.32 (LOS 1.02%) / AMD +5.21 ± 14.37 (LOS 76.16%), both
  1000g round-capped, pooled 49.08% / 2000g ≈ −6 Elo — accepted as the cost
  of path-independent scores in long shuffle endgames (a #5-class conversion
  concern blitz never exercises). Shipped on correctness+tests by explicit
  user call, same as #50/#51.

**Ship verification (combined build, both flags ON):** startpos d14 =
**6,634,033** / cp 27 / e2e4 — byte-identical to the #52 solo arm (the
rule-50 guard cannot fire from startpos at d14: clock+depth never reaches
100). Mate-in-1 discriminator (`7k/8/5KQ1/8/8/8/8/8 w`) → `mate 1 / g6g7` at
depth 1. Rule-50 oracle equality: warm clock-98→clock-0 re-search returns
`cp 1211 / h2d6`, exactly matching a fresh-process search of the clock-0
position. Tests: **216 pass / 1 by-design skip (217 total)**. Net vs t25 at
blitz ≈ the #52 number (#53 ~neutral); the t26 self-play delta is expected
around +35–40 Elo pooled.

**External calibration (2026-07-11, AMD box): t26 ≈ 2571 ± 19 CCRL-blitz.**
Two new same-author anchors added to the Stash ladder (10+0.1, 500g each,
cc=1, noob_3moves.epd):
- vs **Stash 19.0 (CCRL 2473)**: **+102.97 ± 25.74** (nElo 129.38), 64.40%
  (W256/L112/D132), Ptnml [11,23,90,63,63] → pin ~2576.
  PGN `gauntlet/huginn_vs_stash19_amd.pgn`.
- vs **Stash 20.0.1 (CCRL 2509)**: **+56.07 ± 27.03** (nElo 64.41), 58.00%
  (W217/L137/D146), Ptnml [25,32,83,58,52] → pin ~2565.
  PGN `gauntlet/huginn_vs_stash20_amd.pgn`.

The two pins agree within 11 Elo (inverse-variance pool 2571 ± 19); zero
Huginn timeouts (Stash had one). Externally measured gain since v2.2/t21
(~2434 LTC): **≈ +137** — vs +160 summed self-play deltas, a mild ~0.86
compression (with the blitz-vs-LTC scale caveat). Both anchors are
non-saturated (58–64%), so the pins are trustworthy; next recalibration
rung when Huginn approaches ~2650 would be Stash 21+ or a second engine
family for a style cross-check (BACKLOG #5).

### baseline-t25 — #51: History-heuristic piece-index collision fix
= t24 + a one-line-class correctness fix, shipped directly to `main` ahead of
any flag/branch/SPRT-queue process (bug, not a tunable feature — see
[BACKLOG.md #51](BACKLOG.md) for the full writeup). Found while comparing
Huginn's move-ordering design against a sibling JS engine built for an
unrelated competition.

Four call sites (`update_search_history`, `penalize_search_history`,
`pick_next_move`'s quiet-move scoring, and the gated
`ENABLE_CONTINUATION_HISTORY` helper) indexed `search_history[13][64]` /
`continuation_history` with `static_cast<int>(piece) % 13`, where `Piece` is
packed as `(color<<3)|type` (`WhitePawn=1..WhiteKing=6`,
`BlackPawn=9..BlackKing=14`) rather than a dense 0..12 range. The modulo
folded `BlackKing=14 → 1`, the same row as `WhitePawn`, so quiet White-pawn
moves and quiet Black-king moves to the same square shared one history
cell (and `BlackQueen=13 → 0`, silently wasting a row). Same species of bug
as **#50** (Zobrist black-king-row OOB) but narrower in mechanism — the
correct 13-row convention (`type_of(piece) + (Black ? 6 : 0)`, White 1-6 /
Black 7-12 / row 0 unused) already existed in `zobrist.hpp` and just hadn't
been reused here; the two schemes had silently diverged.

**Fix:** new `history_piece_row()` helper (`search.cpp`, anonymous
namespace) replacing all four `% 13` call sites, reusing `zobrist.hpp`'s
proven-correct convention instead of reinventing a broken one. No compile
flag — this was a straightforward indexing bug with one correct answer, not
tunable behavior. Clean Release build; 205/205 tests pass (204 run + 1
by-design skip).

**Signature check:** `go depth 14` from startpos — 8,406,631 nodes (t25) vs
8,461,833 nodes (t24), same bestmove/PV (`d2d4`, expected — king moves don't
enter the startpos-d14 PV, so this only confirms the fix is live, not a
strength signal).

**AMD SPRT vs t24 (2026-07-07):** 10+0.1, 1t, 64MB, noob_3moves.epd, cc=4,
1000 games (hit the `-rounds 500` cap before LLR crossed either SPRT bound
— LLR 1.88 of [-2.94, 2.94]). **+19.48 ± 15.00 Elo**, nElo +28.03 ± 21.53,
**LOS 99.46%**, W280/L224/D496 (52.80%), DrawRatio 40.20%. CI excludes
zero. Larger than expected for a collision this narrow on paper — the
shared row corrupts ordering for the far more common White-pawn quiet
moves too, not just the rare Black-king ones, so the practical impact was
broader than "rare edge case" suggested.

**Shipped on AMD-only accept** (user call, given the clear CI/LOS margin) —
no Intel leg run. Mirrors t23's own precedent: that baseline also shipped
on a single-machine SPRT accept (AMD, +33.97 ± 16.60, LOS 100%, @872g)
ahead of the Intel leg, since #50 was likewise a correctness bug rather
than a tunable feature.

### baseline-t24 — SPRT queue winners: SEE ordering (#6) + root two-fold avoidance (#44 f/u)
= t23 + the two confirmed winners from the 2026-07-02/04 SPRT queue (10 branches
screened, see [BACKLOG-archive-2.1.md's SPRT queue section](BACKLOG-archive-2.1.md)
for all 8 parked/rejected items and the full
[SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md) procedure). Built as
`candidate/t24` (cherry-picked both feature commits onto
`main`, resolved a textual flag-block merge conflict — no logic overlap, the
two features touch different functions), verified, then merged.

- **#6 — SEE good/bad capture split** (`ENABLE_SEE_ORDER_SPLIT`, default ON).
  In `pick_next_move`, captures with SEE < 0 drop to strictly below every
  quiet move instead of scoring with MVV-LVA above killers; SEE ≥ 0 captures
  keep their normal high priority. Promotions and en passant exempt (mirrors
  quiescence's own SEE-prune exemptions). The prior #6 attempt ("lazy SEE")
  was ~neutral; this full split is a different, much stronger mechanism.
  Two-machine SPRT vs t23 — **both legs H1-accept:** AMD **+49.54 ± 20.55**
  (nElo +68.84), 57.08% (W189/L106/D291, 586g), LOS 100%, Ptnml
  [11,55,99,96,32], LLR 2.97; Intel **+29.02 ± 15.17** (nElo +41.48), 54.17%
  (W279/L196/D521, 996g), LOS 99.99%, Ptnml [20,102,197,133,46], LLR 2.98.
- **#44 follow-up — root two-fold draw-avoidance** (`ENABLE_ROOT_TWOFOLD_AVOID`,
  default ON). Extends the root winning-repetition clamp from the rule
  threefold (≥3) to a single repetition (≥2), so a won engine routes around
  a repeating shuffle one move earlier — closing the one gap the #28-Part-2
  Zarkov single-rep rule didn't cover (the root child at `info.ply==1`).
  Provably inert without game history (startpos node counts identical to
  t23); verified via `RootTwofoldAvoid.WinningRootRoutesAroundSingleRepetition`,
  which directly confirms a stale warm-TT repetition bait is routed around.
  Two-machine SPRT vs t23 — **cross-machine agreement** (neither leg resolves
  alone, but both clear the t19 safe-mobility ship precedent of +5.91/+10.43):
  AMD **+12.51 ± 15.22** (nElo +17.73), 51.80% (W269/L233/D498, 1000g), LOS
  94.68%, Ptnml [24,117,201,115,43], LLR 1.06; Intel **+7.99 ± 15.12** (nElo
  +11.39), 51.15% (W268/L245/D487, 1000g), LOS 85.01%, Ptnml
  [32,109,193,136,30], LLR 0.53.
- **Combined-candidate validation (`candidate/t24`, both flags ON) vs t23 —
  two-machine SPRT, both legs H1-ACCEPT:** AMD **+48.84 ± 20.36** (nElo
  +68.81), 56.98% (W186/L105/D289, 580g), LOS 100%, Ptnml [8,59,99,92,32],
  LLR 2.98 (crossed the upper bound); Intel **+66.33 ± 23.61** (nElo +93.61),
  59.43% (W159/L76/D205, 440g), LOS 100%, Ptnml [7,34,75,77,27], LLR 2.97
  (crossed the upper bound). **Pooled: W345/L181/D494 = 58.04% / 1020g.**
- **Undershoot guard (AMD): clean.** Combined (+48.84) sits essentially at
  see-ordering's solo AMD number (+49.54), not below it — healthy
  sub-additivity vs the naive sum (+62.05), not a masked negative interaction.
  **Intel showed the opposite pattern** — combined (+66.33) notably exceeds
  see-ordering's solo Intel number (+29.02), i.e. apparent positive synergy
  between the two winners on that box. Neither pattern triggers a concern:
  the guard only fires on undershoot (a winner turning negative in company,
  the #44+NMP precedent), and both machines show the combine outperforming
  or matching its best single component.
- Verified: both-flags-OFF arm reproduces `baseline-t23`'s exact signature
  (startpos d14 = 14,306,844 nodes, byte-for-byte); both-flags-ON signature
  (8,461,833 nodes / d2d4) matches `see-ordering`'s standalone number exactly
  (confirms `root-twofold-avoid` composes cleanly, no leakage into ordinary
  search); 204/205 tests pass (1 by-design skip — the opposite-arm test pair).
- **Eight items screened in the same queue did NOT make t24** — razoring-off
  (sign-split, parked), rfp-pv-guard (two-machine reject), futility-depth2
  (clean neutral, parked), futility-pv-guard (two-machine reject — caught and
  fixed a silent-flag-off CMake bug mid-queue, see
  [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md)), tt-aging (inconclusive,
  LTC check recommended), drawishness-scaling (flat vs equal-strength t23, as
  predicted — needs a weaker external anchor), trapped-bishop (clean neutral,
  SF-class magnitude, park-for-Texel-retune), pext (3–5% *slower* on this
  Zen4 box, not faster). Full numbers for each in
  [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md).

### baseline-t23 — #50: Zobrist black-king row out-of-bounds fix
= t22 + a one-line correctness fix, shipped directly to `main` ahead of the
SPRT queue (bug, not a feature — see
[BACKLOG-archive-2.1.md #50](BACKLOG-archive-2.1.md) for the full
root-cause writeup). Every keying site computes the piece-table row as
`int(PieceType) + (Black ? 6 : 0)` — White rows 1–6, Black rows 7–12, row 0
unused, **13 rows total** — but `Zobrist::Piece` was declared
`[PIECE_NB=12][64]`. The **black king's row (12) read 64 `U64`s past the end
of the array**:
- One phantom slot (black king on **f4**) landed on an **ASLR'd heap
  pointer**, injecting a fresh per-process value into every key with the
  black king there — the source of the BACKLOG #50 Kiwipete d14 node-count
  nondeterminism (±~1k/4.7M run-to-run; startpos never walks the black king
  to f4, so it stayed deterministic and the bug went unnoticed for the whole
  t5→t22 ladder).
- Five more phantom slots (black king on **b1/c1/d1/g4/h4**) read **constant
  zero** — the black king contributed NOTHING to the key on those squares, so
  two positions differing only in that placement **hashed identically: real
  TT key collisions**, a correctness hazard beyond nondeterminism, carried by
  every baseline ever shipped (same latent-bug class as #44 and #45).
- Time/input polling and Syzygy TB were each exonerated experimentally before
  the OOB read was found (per-node key traces + a per-process dump of the key
  table pinned the exact phantom slots and their contents).

**Fix:** `PIECE_NB` 12→13, unconditional — **no compile flag.** This was pure
undefined behavior with exactly one correct value, not a tunable behavior, so
it didn't go through the flag/branch/SPRT-queue process other backlog items
use; it landed as a direct commit to `main` (`150ea1f`, merged via PR #22).
15× Kiwipete d14 confirmed constant post-fix (was wobbling); 203/203 tests.

**Signature shift (expected, not a bug):** `init_zobrist()` draws keys
sequentially — `Piece[][]` then `Side`/`Castle[]`/`EpFile[]` — so inserting a
whole extra row before those shifts every key drawn after it, not just the
black king's. Fixed-depth node counts moved even at positions that never
approach the phantom squares: startpos `go depth 14` was 12,035,479 nodes /
cp 27 under t22, now **14,306,844 nodes / cp 24** under t23 (same bestmove
e2e4 both times — the search logic is unchanged, only the hash constants
moved). d15 = 21,844,725. **Kiwipete is now also deterministic** (1,639,166 @
d13, 3/3 runs) and usable as a second verification anchor going forward.

- **AMD SPRT vs t22 (10+0.1, 1t, 64MB, noob_3moves.epd): H1 ACCEPTED @872g —
  +33.97 ± 16.60** (nElo +47.55 ± 23.06), 54.87% (W267/L182/D423), **LOS
  100.00%**, DrawRatio 36.70%, PairsRatio 1.60, Ptnml(0-2) [20,86,160,129,41],
  LLR 3.01 — resolved decisively inside the cap. Artifacts
  `gauntlet/fix50_vs_t22_amd.pgn` + `fix50_fastchess_t22_amd.log`.
- **Single-machine decisive freeze** (LLR resolved, not just a lean) — Intel
  confirms on push, same pattern as the t21 ship. Third #44/#45-class latent
  structural bug to convert to real Elo (#44 ≈ +48–76, #45 ≈ +345–355) —
  smaller here (+34), but the same lesson: **audit long-standing structures
  periodically, not just new deltas** (every incremental A/B on this ladder
  carried the collision bug in both arms, so it never surfaced until the
  structure itself — the Zobrist table — was examined directly).
- **Ten more candidate branches** (`copilot/fix50-for-*`, rebased onto t23
  from the pre-#50 `experiment/*` originals — feature diffs verified
  byte-identical) are queued for gauntlet screening toward the next baseline,
  `t24`. Full procedure: [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md).
- **Follow-up hook:** re-test #37 (illegal bestmove) — zobrist collisions
  corrupting TT-move sourcing were a candidate mechanism, and this fix removed
  one concrete collision source.

### baseline-t22 — speed pair: #48 double-TT-probe kill + #49 KS/mobility fusion
= t21 + two **behavior-identical** nps optimizations from the 2026-06-28 uProf
profile. No search/eval behavior change: startpos d15 node counts byte-identical
across every run of every arm (18,223,597), 203/203 tests incl. eval symmetry.
- **#48 — kill the double TT probe.** `AlphaBeta` probed the TT at node entry
  AND `pick_next_move` re-probed just to fetch the ordering move → two
  cache-miss-prone probes/node (uProf: pick_next_move #2 self-time). The TT
  move is now a parameter fed from the node-entry probe; quiescence (no entry
  probe) keeps exactly one probe, hoisted pre-loop. Alone: +0.8% nps startpos
  d15 / +4.0% Kiwipete d14.
- **#49 — fuse the king-safety attacker scan into the mobility pass.**
  `king_safety_white_mg` recomputed a magic attack set for every enemy B/R/Q
  (queen ×2) for both kings at every eval (~3% of time) — the exact sets the
  mobility loop already computes. King-zone attacker units are now accumulated
  inside the mobility loop (`ks_accum`, both branches); `king_danger_mg()`
  finalizes (square/cap/shelter) at the taper; the standalone scan is deleted.
  Alone: +8.2% / +5.5% — above the uProf estimate.
- **Combined interleaved A/B: +11.0% nps (startpos d15) / +6.3% (Kiwipete d14).**
- **Two-machine SPRT vs t21 (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each) —
  both legs positive, cross-machine-agreement ship:**
  - AMD: **+14.60 ± 15.69** (nElo +20.07), 52.10%, LOS 96.62%, W285/L243/D472,
    Ptnml [36,104,175,152,33], LLR 1.23. `gauntlet/huginn_vs_t21_amd.pgn`.
  - Intel: **+18.08 ± 14.44** (nElo +27.03), 52.60%, LOS 99.31%, W260/L208/D532,
    Ptnml [17,110,216,118,39], LLR 1.83. `gauntlet/huginn_vs_t21_intel.pgn`.
  - Pooled: W545/L451/D1004 = **52.35% / 2000g** (≈ +16.3), Ptnml
    [53,214,391,270,72].
- Validates the #48/#49 judging bar: a pure-nps bump converts to Elo at fixed
  time with zero behavior change (the speed reaches the next iteration depth).
  Side discovery logged as **#50**: Kiwipete fixed-depth node counts wobble
  ±~1k/4.7M run-to-run (pre-existing at t21; startpos is clean).

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
  buys a deeper search every move at every TC.
- **LTC rating (60+0.6, AMD, 1000g): t21 vs Stash 17.0 (2298) = 68.65% / +136.16
  ± 19.87, LOS 100%** (567W/194L/239D, Ptnml [20,46,136,137,161]) → **t21 ≈ ~2434
  LTC**. Markedly above t20's *blitz* 56.75%/+47 (~2345) vs the same anchor — the
  longer clock is exactly where #47 pays. **Zero Huginn time-forfeits** (the 4 in
  the run were all Stash flagging, 0.4%) → the aggressive clock use is safe at LTC.
  This is the v2.2 rating number. Artifacts: `gauntlet/t21_vs_t20.pgn`,
  `gauntlet/ltc_t21_vs_stash17.pgn` (the LTC games — conversion/endgame audit
  material: 219 of 239 draws are 3-folds, cf. #5).

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
