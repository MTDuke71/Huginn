# BACKLOG

Single-file issue tracker. Each session opens here first.

Format per entry:
- **status**: open / in-progress / blocked / closed
- **priority**: high / medium / low
- **type**: feature / bug / maintenance / research
- **est**: rough effort (hours/sessions)
- **evidence/notes**: why this matters, what unblocks it

Closed items keep their entry with `status: closed @ <commit>`. Don't
delete — the rationale stays useful as history.

The long-term roadmap (Tier 1-4 features, architectural plans) lives in
[SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md). This file is for *specific
actions with concrete state* — what's blocked, what's ready, what bugs
exist, what conditions to watch.

---

## Open — recently deferred (blocked by move-ordering quality)

**Four** features now share this fingerprint: tactically-correct
implementation, real WAC tactical-solving improvement, neutral-to-
strongly-negative gauntlet Elo. #1, #2, #7, #8 all regressed despite
individually-sound diagnoses. The 2026-05-04 king-safety re-attempt
on top of mobility was the last falsifying experiment: original
hypothesis was "mobility was the missing companion" — but the gauntlet
landed at -126 Elo at 40 games, *worse* than the original KS-only
runs (-21 / -3 / -17). Mobility is not the missing companion. **Move
ordering quality is** — the engine cannot reliably distinguish
"moves that need full attention" from "moves safe to reduce or prune."

Continuation history (#3) is the universal unblocker for all four.
When it lands, expect this entire section to become re-attemptable.

### #1: Skip SEE-prune and LMR-reduce on check-giving moves — DEFERRED

- status: blocked (re-attempt after #3 lands)
- priority: was high; now low until unblock condition met
- type: feature
- attempted: 2026-05-02 / 2026-05-03 (this session, two iterations)
- links: [SEARCH_AND_EVAL.md#priority-1](SEARCH_AND_EVAL.md), `src/minimal_search.cpp`
  qsearch + LMR blocks

**Original evidence still valid.** WAC.009 — huginn at depth 10 says
−223 cp, MTL_v0.5 at depth 26 says +28991 (forced mate). Expected move
`Bh2+`. Huginn's LMR was reducing the exact move that drives the mate.
Pattern: 11/24 of the regressions vs old-huginn and roughly half of
the 51 missed-vs-MTL positions involve checks.

**What we did and what happened:**

| Variant | What changed | WAC | Gauntlet vs t2 (100g) |
|---|---|---|---|
| P1a | LMR exempts moves that give check | 84.3% (+5.3pp from baseline) | -3.47 Elo, LOS 46% |
| P1a opt + P1b | + qsearch SEE-prune exempts check captures | not re-measured | **-31.35 Elo, LOS 18%** |

**The diagnosis was correct.** WAC tactical-solving rate jumped 5.3pp
(16 more positions solved, 13 of them checks — exactly the target
class). On WAC.009 specifically, huginn now finds Bh2+ at depth 8
with mate in 11 (vs not seeing it at all up through depth 10
previously). The fix does what it was designed to do.

**The fix doesn't translate to gauntlet Elo.** Same shape as #7 (LMP)
and #8 (aspiration step b): tactical-sight improvements that don't
convert to game-play strength. Two probable mechanisms:

1. **Time redistribution.** Skipping LMR on check-moves means those
   moves consume more time per node. Same total clock budget, less
   depth elsewhere → tactical wins offset by positional losses.
2. **P1b actively hurt** (-28 Elo delta). The cost of MakeMove +
   SqAttacked + TakeMove on every SEE-pruned capture is real overhead
   in a hot qsearch loop, and sacrificial-check captures in qsearch
   rarely pay out in actual games (they were already pruned earlier
   in the search by other mechanisms when worthwhile).

**Why it's blocked, not just rejected.** All four deferred features
share the same root cause: weak move ordering. With current move
ordering, the engine cannot reliably tell which check-giving move (or
which late quiet, or which aspiration-bound subtree) is actually the
critical one. Continuation history (#3) directly addresses this. Once
ordering is strong enough that the top-K moves contain the right
candidates, all four deferred features should yield positive Elo.

**Re-attempt plan when unblocked by #3:**
1. Cherry-pick just P1a (LMR-exempts-check) — the optimized form with
   the lazy `gives_check` lambda. This was the half that did neutral,
   not negative.
2. Re-run WAC and gauntlet vs whatever-baseline-is-current.
3. If positive, commit. Skip P1b again unless WAC shows specific
   capture-with-check positions that P1a alone misses; those are
   rarer than the analysis initially suggested.

**Code archive:** the P1a-optimized + P1b combined diff is in the
reflog from this session. Recoverable via `git reflog` if/when needed.

---

### #2: Re-attempt king safety on top of mobility — DEFERRED

- status: blocked (re-attempt after #3 lands)
- priority: was high; now low until unblock condition met
- type: feature
- attempted: 2026-05-04 (this re-attempt) + 2026-04-30 (original 3 iterations)
- worktree: `C:\Users\m_lad\Repos\Huginn.worktrees\agents-backlog-review-next-steps`
  branch `agents/backlog-review-next-steps`
- links: [SEARCH_AND_EVAL.md#king-safety-attempt](SEARCH_AND_EVAL.md)
  defer note from commit `30aba92`

**Hypothesis tested:** "King safety needs mobility's position-based
penalty interaction to pay off (less mobile = harder to escape attacks)."
Original 3 iterations landed at -21 / -3 / -17 Elo. With mobility now
shipped (commit `626257a`), the v2 settings should compound positively.

**Result of re-attempt (2026-05-04):**
- Implementation: presence-based attack zone, `DANGER_MAX = 200`, the
  same v2 settings that scored best in the original attempt; freshly
  added file-exposure penalties.
- 208/208 unit tests pass; eval-symmetry test confirms mirror-equivalence.
- Gauntlet vs `huginn_t2`: **-126 Elo at 40 games**, called early. Worse
  than any of the original three KS iterations.

**Hypothesis falsified.** Mobility was *not* the missing companion. The
gauntlet result is significantly worse than the original isolated runs,
which suggests mobility and king safety actively *anti-compound* under
current conditions — they push the engine in different directions when
deciding move priorities, and the search doesn't have the ordering
quality to reconcile them.

**Why it's blocked, not rejected.** Same root cause as #1, #7, #8:
weak move ordering. The eval is correctly identifying king-safety
threats, but the search can't sequence the moves to address them
(or to ignore false-positive threats the eval surfaces). Continuation
history (#3) addresses this directly.

**Code state:** the implementation lives uncommitted in the worktree at
`C:\Users\m_lad\Repos\Huginn.worktrees\agents-backlog-review-next-steps`
on branch `agents/backlog-review-next-steps`. Cherry-pickable when #3
ships. Worktree binary `huginn_ks.exe` is in the fastchess folder.

**Re-attempt plan when unblocked by #3:**
1. Cherry-pick the king-safety eval changes from the worktree branch.
2. Re-run gauntlet vs whatever-baseline-is-current.
3. If positive, commit. If still neutral, the issue is deeper than
   move ordering and may require a Texel-style tuner (#9) to resolve
   parameter mis-tuning.

---

## Open — high priority

### #13: Investigate dormant counter-move + TT-mate-adjustment latent bugs — CLOSED

- status: **closed** (2026-05-06, TT-mate variant shipped)
- priority: was high
- type: bug / research

**Discovered 2026-05-05** while attempting #3. The engine has two
latent bugs that have been silently dormant since the start:

1. **`info.ply` is never incremented across recursive AlphaBeta calls.**
   It's set to 0 once at search start and stays 0. This silently
   disables:
   - The counter-move heuristic (guarded by `info.ply > 0`)
   - The TT mate-distance adjustment (uses `info.ply` for store/probe)
2. **The "fix" makes things significantly worse.** Adding an RAII
   `PlyScope` guard that correctly tracks ply produced **-164 Elo at
   100 games (LOS 0%)** vs `huginn_t2`. Fixing the dormancy without
   re-tuning the now-active code regresses the engine.

Most likely cause: **counter-move score 700K is misordered**. With
counter-moves dormant, ordering was: TT > PV > IID > captures (1M+) >
killers (800-900K) > promotions (25K-90K) > history. With the fix
active, counter-moves at 700K slide between killers and promotions —
beating queen-promotion (90K) for tactical priority. That's almost
certainly wrong; promotions should beat counter-moves.

There may be additional issues with TT mate-adjustment correctness
that surface only when it actually fires.

**Why high priority:** #3 (continuation history) was meant to be the
universal unblocker for the four-feature deferred section. But #3's
plumbing depends on `info.search_stack[info.ply - 1]` which only works
when `info.ply` tracks correctly. Fixing #13 cleanly unblocks #3 —
and likely also #1/#2/#7/#8 indirectly via better move ordering once
the latent code is properly tuned.

**Attempt 2 (2026-05-06):** Applied the prescribed fix —

1. Inline `++info.ply` / `--info.ply` around every recursive AlphaBeta
   call (main loop, null-move, IID, root, test entrypoint).
2. Wrote search_stack[ply] before each recursion, with a null S_MOVE()
   marker for null-move so the child's counter-move guard
   (`previous_move.move != 0`) skips it.
3. Lowered counter-move score from 700K → 15K (slot below all
   promotions, above the history range).
4. Fixed leaf mate return from `-MATE + (info.max_depth - depth)` →
   `-MATE + info.ply` so it stays consistent with the TT store/probe
   adjustment that uses `info.ply` (the two diverge when check
   extensions bump depth, corrupting the TT for check-extended mate
   paths).

**Result:** **-113.94 ± 63.96 Elo at 60g vs t2, LOS 0.01%, stopped
early.** Better than attempt 1's -164 (the score+mate-distance fixes
recovered ~50 Elo) but still clearly regressive. 208/208 unit tests
pass; failure is gameplay-only.

**What this rules out:** the 700K counter-move score was not the
sole cause. The mate-distance leaf encoding was inconsistent
(real bug, fixed) but fixing it was not sufficient. The remaining
regression source is somewhere we haven't pinpointed — most likely
candidates:

- **TT pollution from now-active mate-distance writes.** Even with
  consistent leaf encoding, there may be other paths where mate
  scores get stored without the `+= info.ply` adjustment, or where
  `info.ply` at store time differs from `info.ply` at the leaf
  (e.g., null-move recursion writes at parent's ply but the child's
  leaf was at a different effective depth).
- **Counter-move at 15K may still be wrong.** Maybe should be
  history-range (1000-ish) or even lower. Or counter-move scoring
  may be conceptually broken in a way the score doesn't fix.
- **Something else activated by `info.ply > 0`** that we didn't audit.

**Plan for next session (resume here 2026-05-07):**
1. Snapshot the regressive change as a stash/branch for reference,
   then revert clean. (Working tree already cleaned 2026-05-06.)
2. **Bisect the activations.** Re-apply the patch but gate each
   activation independently:
   - 2a. Ply tracking only, counter-move + TT-mate stays guarded by
     a separate flag (`if (info.ply_tracked && info.ply > 0)` etc.).
     Test: same Elo as t2 (info.ply tracks but nothing fires)?
   - 2b. Add ply-tracked-counter-move only. Test.
   - 2c. Add ply-tracked-TT-mate only. Test.
   - 2d. Combinations. Find which activation regresses.
3. **Step 6 fallback (from original plan):** if bisection
   inconclusive or all individual activations regress, drop the
   `info.ply` machinery entirely and pass `prev_move` as an explicit
   AlphaBeta parameter (`AlphaBeta(..., S_MOVE prev_move = S_MOVE())`).
   This sidesteps ply tracking completely; only the counter-move
   heuristic uses prev_move; TT mate-distance stays dormant.
4. **TT cap-on-mate-store:** when storing mate scores, clamp to
   ensure `store_score < MATE` and `store_score > -MATE` even after
   `+= info.ply`. Prevents the >MATE corruption case if check
   extensions still cause encoding drift somewhere.

**Code state:** changes were uncommitted and reverted via
`git restore src/minimal_search.cpp`. Diff was:
- `++/--info.ply` around recursions in main loop, null move, IID,
  root (`searchPosition`), test entry (`MinimalEngine::search`).
- search_stack writes at each recursion site.
- Counter-move score 700K → 15K at line ~899.
- Mate leaf return changed at line ~1497.

**Resolution (2026-05-06):** four-way bisection on top of ply tracking
isolated the regressors and rescued one feature. Each variant ran
100g vs `huginn_t2` at tc=10+0.1.

| Variant | Counter-move | TT-mate | Elo vs t2 | LOS |
|---|---|---|---|---|
| 2a | off | off | +10 ± 61 | 63% |
| 2b | 15K | off | +96 ± 66 | 99.9% |
| **2c (shipped)** | **off** | **on** | **+104 ± 62** | **99.98%** |
| 2d | 15K | on | +31 ± 52 | 88% |
| attempt 2 (history) | 700K | on | -114 ± 64 | 0.01% |

**Findings:**
- Attempt 2's regressor was specifically the **700K counter-move
  score** placing counter-moves above queen-promotion (90K). With it
  at 15K (below promotions, above history), counter-move alone is
  +96 Elo (2b).
- The cap-clamp on mate-store (`store_score >= MATE → MATE-1`) added
  this session was the difference between attempt 2's TT-mate failure
  and 2c's success at the same conceptual config. Attempt 2 had no
  cap, allowing TT corruption from `+= info.ply` overflow.
- **Counter-move + TT-mate anti-compound** (2d at +31 vs ~+100 each
  alone). Likely a search-shape interaction — 2d had 25 draws vs
  17-19 for the singles. Cause not yet pinpointed.

**Shipped (2c):** TT-mate active (probe-adjust + store-adjust + cap
clamp + leaf-encoding via `info.ply`), counter-move gated off via
`#define ENABLE_PLY_TRACKED_COUNTERMOVE 0` in `src/minimal_search.cpp`.
Counter-move re-attempt tracked separately as #15.

**Triggers:** +104 ≥ +50, so #4 (refresh `huginn_t3` baseline) fires.

---

### #15: Re-attempt counter-move heuristic on top of 2c

- status: open / unblocked
- priority: medium
- type: feature / research
- est: 1 session
- links: #13 closure (2c shipping commit), #3 (continuation history shares
  the search_stack plumbing)

**Evidence (from #13 bisection, 2026-05-06):**
- 2b (counter-move @ 15K only): +96 ± 66 Elo, LOS 99.9%
- 2c (TT-mate only, shipped):  +104 ± 62 Elo, LOS 99.98%
- 2d (both at 2b/2c settings): +31 ± 52 Elo, LOS 88%

The 2d combo gives back ~65-73 Elo vs either feature alone. CIs
overlap at 100g so noise can't be ruled out, but the trajectory is
consistent. 2d also drew significantly more (25 vs 17-19) which
suggests a real search-shape interaction, not pure noise.

**Hypothesis space:**
- **Score interaction:** 15K may be wrong on top of TT-mate. With
  TT-mate active, mate-distance scores propagate further; counter-move
  may be over-promoting moves that lose to forced sequences. Try
  history-range (1000-1500) or even lower.
- **Search-stack lifetime bug:** `search_stack[ply]` is only written
  in the main-loop recursion path. Null-move recursion now writes a
  null marker (per the 2a-2d patch); IID, root, and test entry write
  the move being recursed. But there may be paths where the parent's
  search_stack[ply] entry leaks into a sibling's view. Audit.
- **Counter-move table aging:** `counter_moves` is never aged, only
  cleared per-search via `clear_search_tables()`. Might want depth²
  bonus aging like history.
- **Approach swap:** drop ply-tracked counter-move and pass `prev_move`
  as an explicit AlphaBeta parameter (the original step-6 fallback).
  Decouples counter-move from `info.ply`, simplifies reasoning.

**Plan:**
1. Flip `ENABLE_PLY_TRACKED_COUNTERMOVE` to 1 → 100g gauntlet. If
   matches 2d (+31), proceed.
2. Try score variants: 1500, 5000 (sub-killer), 50000 (above queen
   promotion but below killers), each with a 100g gauntlet.
3. If still anti-compound at all scores, try the prev_move parameter
   approach as a clean-slate test.
4. If positive, ship on top of 2c. Triggers #4 again if cumulative
   crosses next +50 threshold.

---

### #3: Continuation history (Tier 2 #11) — universal unblocker

- status: open / unblocked (ply tracking landed in #13 / 2c)
- priority: high
- type: feature
- est: 1 session
- links: [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) Tier 2 #11
- attempted: 2026-05-05 (this session)

**Result of first attempt (2026-05-05):** Implemented continuation
history table + scoring contribution + aging + PlyScope guard for
the prerequisite info.ply tracking. Combined gauntlet vs `huginn_t2`:
**-147 Elo / LOS 0%**. Isolation test (PlyScope-only, no continuation
history): **-164 Elo / LOS 0%**. The `PlyScope` fix alone is the
regressor — see #13 for the root-cause analysis.

**Why this is the highest-priority remaining feature.** Four features
in this backlog (#1, #2, #7, #8) regressed in gauntlet despite
individual diagnoses being correct. The shared root cause is
move-ordering quality. Continuation history addresses this directly.
Now that #13 has landed (2026-05-06), #3 is implementable and the
entire "recently deferred" section is re-attemptable.

**Caveat from #13 anti-compounding result:** counter-move + TT-mate
anti-compounded (+31 Elo combined vs +96/+104 individually). Watch
for similar interaction when continuation history layers on top of
TT-mate. If #3 alone is positive but combined with anything else
regresses, the fix is ordering-table tuning, not feature deletion.

**Plan now that #13 has landed:**
1. Add `int continuation_history[13][64][13][64]` (heap-allocated to
   avoid stack-overflow on test instances; ~2.7 MB).
2. Update on quiet-move beta cutoff with `depth²` bonus, indexed by
   `[prev_piece][prev_to_64][cur_piece][cur_to_64]`.
3. Score contribution in pick_next_move's quiet-move branch, additive
   on top of search_history.
4. Age each search start (divide by 4) like search_history.
5. Test alone vs t2 (success criterion: any positive Elo, not
   regressing). Then re-attempt the deferred trio in order:
   #1 (check-pruning, optimized P1a only) → #7 (LMP, with the
   `quiet_count` plumbing from `tier1-stack-broken`) → #8 (aspiration
   step b) → #2 (king safety) — each with a fresh gauntlet on the
   new tip.

The implementation details from the 2026-05-05 attempt are documented
above. Cherry-pickable when #13 unblocks.

---

## Open — medium priority

### #4: Refresh `huginn_t3` baseline when cumulative ≥ +50 over t2 — CLOSED

- status: closed @ `2e97066` (2026-05-06)
- tag: `baseline-t3 = 2e97066`
- priority: was medium
- type: maintenance

**Triggered by** the #13/2c TT-mate ship (+104 ± 62 Elo vs t2,
LOS 99.98% / 100g) — single feature crossed the +50 threshold.

**What shipped:**
- `git tag baseline-t3 2e97066`
- `huginn_t3.exe` copied to `fastchess-windows-x86-64/` from
  `build/msvc-x64-release/bin/Release/huginn.exe` (the 2c build).
- `test_huginn_vs_t3.bat` added (mirrors `test_huginn_vs_t2.bat`).

**Follow-ups still pending (low priority):**
- `SEARCH_AND_EVAL.md` tooling section update to reference t3.
- `test_epd_diff.py` default opponent → t3.
- `test_huginn_vs_t2.bat` is now superseded but kept around for
  historical regression checks; new work should use t3.

---

### #5: Recalibrate vs external opponent after each major fix

- status: open / recurring
- priority: medium
- type: maintenance
- est: 5 minutes per run (script auto-rebuilds)

**Recent MTLChess v0.3 runs:**
- **2026-04-30, post-mobility, vs old `mtlchess003.exe` (20g):** 2W/17L/1D,
  ~−340 Elo. Real progress from 0W/20L/0D the run before, but
  still firmly behind.
- **2026-05-06, post-2c TT-mate ship, vs rebuilt `mtlchessV3.exe` (100g):**
  5W/89L/6D, **-424 ± 118 Elo, LOS 0%**. Apparent regression vs the
  prior datapoint is opponent-side: the rebuild (Zig 0.16.0-dev,
  modern LLVM, no AVX-512) is materially stronger than the SIGILL'd
  pre-migration `mtlchess003.exe` it replaced — call it ~150-200 Elo.
  Trustworthy gauge of recent Huginn progress is the internal t-chain
  (t1 → t2 → t3 = ~+118 Elo across mobility / movegen / TT-mate),
  not this external comparison.

**Machine-migration breakage + recovery (2026-05-06):** all seven
pre-existing `mtlchess*.exe` binaries in the fastchess folder fail
with SIGILL on the new machine — a 13th-gen i7-13700KF (Raptor Lake).
Cause is AVX-512 in the prior-machine builds; Intel disabled AVX-512
on consumer Raptor Lake P-cores. **Rebuilt MTLChess v0.3 from source
on this box as `mtlchessV3.exe`** (UCI handshake confirms `id name
MTLChess v0.3`). Other versions (v0.4, v0.5, v0.6, etc.) are still
unrunnable until similarly rebuilt.

**Anchors:**
- **Primary (works):** MTLChess v0.3 / `mtlchessV3.exe`, rated ~1984.
  Script `test_huginn_vs_mtlchess_v03.bat`. Last datapoint
  (2026-04-30, post-mobility, on the prior machine): -340 Elo. Good
  rung for current Huginn strength.
- **Secondary (works, too strong):** MORA / `MORA110.exe`, rated
  ~2191. Script `test_huginn_vs_mora.bat`. Useful once Huginn
  closes most of the v0.3 gap; right now Huginn is comfortably losing.
- **Broken pending rebuild:** `mtlchess003.exe`, `mtlchess002.exe`,
  `mtlchess.exe`, `mtlchess_v05.exe`, `mtlchess_v06.exe`,
  `MTLChess_v03_before.exe`, `MTLChess_v04_after.exe`. Rebuild path
  same as v0.3: build from source with `-march=alderlake` or
  explicit AVX2-only flags.

**Cadence:** run `test_huginn_vs_mtlchess_v03.bat 50` after each
shipped feature expected to be ≥ +10 Elo. Watch the trajectory —
when we cross +0 Elo vs MTL v0.3, that anchor is graduated and MORA
becomes the new rung.

**Caveat on `test_huginn_calibration.bat`:** the multi-target
calibration script hardcodes `mtlchess003.exe` for `mtl03` (broken)
and `mtlchess_v05.exe` for `mtl05` (broken). Only its `mora` key
works. Update or replace if needed; the standalone bats
(`test_huginn_vs_mtlchess_v03.bat` and `test_huginn_vs_mora.bat`)
sidestep this.

---

## Open — low priority

### #6: Lazy SEE in main-search capture ordering

- status: open / unblocked
- priority: low
- type: feature
- est: half-session
- links: [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) Tier 2 #10 partial

**Status:** SEE is wired in qsearch (commit `1cce8de`) but main-search
capture ordering still uses pure MVV-LVA. Splitting captures into
`SEE ≥ 0` and `SEE < 0` buckets in `pick_next_move` would push losing
trades below quiets. Eager SEE during move generation costs too much
(measured at king-safety attempt time); lazy SEE at pick time is the
standard approach.

**Why low priority:** the qsearch wiring captured the bulk of the SEE
Elo gain (+24 marginal). Main-search ordering is incremental on top.
Worth doing eventually but not blocking anything.

---

### #7: LMP fix — Tier 3 revisit

- status: blocked
- priority: low
- type: feature
- est: 1 session (impl already done in `tier1-stack-broken` tag,
  needs tuning)
- links: [SEARCH_AND_EVAL.md#lmp-fix-attempts](SEARCH_AND_EVAL.md),
  defer commit `5ada94c`

**State:** Two iterations attempted (depth-1-2 incl, then min-depth-3
only). Both regressed (-254, -56 Elo). The per-node `quiet_count`
plumbing is correct; the issue is move-ordering quality.

**Blocked by:** #3 (continuation history) — need stronger ordering so
that "first K quiet moves" actually contains the best tactical choices.
Pruning the rest is then safe.

**On unblock:** cherry-pick the `quiet_count` change from
`tier1-stack-broken`, set `LMP_MIN_DEPTH = 3`, gauntlet.

---

### #8: Aspiration step (b) — Tier 3 revisit

- status: blocked
- priority: low
- type: feature
- est: 1 session
- links: [SEARCH_AND_EVAL.md#aspiration-step-b-deferred](SEARCH_AND_EVAL.md)

**State:** Step (a) (root PVS with fail-high break) shipped at `28cb2cd`
(+13.9 Elo). Step (b) (actual aspiration window) tried at two tunings,
both regressed (-75, -49 Elo). Same root cause as #7: weak move
ordering causes too many fail-low/fail-high re-searches that cost more
than the tighter-window cutoffs save.

**Blocked by:** #3 (continuation history). Possibly also #2 (king
safety) for more stable inter-iteration scores.

**On unblock:** re-apply step (b) v2 (delta×=2, no beta-pull on
fail-low). Test against current tip.

---

### #9: Texel-style tuner

- status: open / blocked (no tuner infra yet)
- priority: low
- type: research
- est: large (1-3 sessions to bring up; ongoing use thereafter)

**Evidence:** King-safety v1→v2→v3 hand-tuning hit a ceiling at ~0
Elo across 3 iterations. The implementation is correct (v1→v2 = +18
Elo from a real bug fix); further gains are tuning-bound.

**Plan:** standard Texel tuning — a corpus of a few-million labelled
positions (game outcomes), fit eval params via gradient descent on
`sigmoid(eval) → outcome`. Unlocks: better king-safety, better
mobility weights, better LMR table values, etc.

**Why low priority:** infrastructure investment without immediate
return. Worth doing once #1, #2, #3 have shipped and we're
parameter-bound on multiple eval features simultaneously.

---

### #10: Wire up Syzygy tablebase probe

- status: open / unblocked
- priority: low
- type: feature
- est: 1 hour (gate already exists)

**State:** Probe code at `src/minimal_search.cpp:1094` is gated by
`if (false && ...)` — fully disabled. Default `SyzygyPath` is
`c:\TB\`. If TB files are installed there, removing the `false &&`
gate enables it.

**Estimated impact:** depends entirely on whether `.rtbw/.rtbz` files
are installed. With 5-piece tables (small download), endgame play
becomes near-perfect. Less impactful for opening/middlegame play.

---

### #11: CLAUDE.md NPS figure is stale — CLOSED

- status: closed @ `b9cc1be` (2026-05-05)
- priority: low
- type: maintenance

**State:** `CLAUDE.md` claimed "~220k nodes/second" in three places.
Replaced with ~1.5M nodes/second on this machine (measured at depth 11
startpos with book disabled: 4.58M nodes / ~2.9s). One of the three
sites now also notes the bench command. The prior box measured 2.0-2.3
Mnps; using ~1.5M as the documented figure since it's what the user
will reproduce on the current hardware.

---

### #14: Move-gen — bypass legality pre-filter at search call sites — CLOSED

- status: closed @ `b1154c8` (2026-05-05)
- priority: was low
- type: feature / perf

**Result (2026-05-05):** Shipped on bench evidence. Depth 11 startpos
(book off): 4.58M nodes / 2904ms (1.58 Mnps) → 3.70M nodes / 1659ms
(2.23 Mnps). **+41% NPS / +43% wall-clock**, slightly above the 35%
spike estimate. Node count and PV shifted (e2e4-line cp 25 → d2d4
cp 30) due to a free correctness improvement in qsearch capture
ordering: the new pseudo-caps generator preserves the bitboard
generator's pre-move MVV-LVA score, instead of `generate_all_caps`'s
post-MakeMove re-scoring where `pos.at(from)==None` zeroed the LVA
term.

**What shipped:**
1. Added `generate_all_caps_pseudo` (pseudo-legal captures, no
   Make/Unmake filter) to `movegen.{hpp,cpp}`.
2. Converted four search call sites from `generate_legal_moves` →
   `generate_all_moves`: `MinimalEngine::search`, `AlphaBeta` main
   loop, IID, `searchPosition` root. Each tracks `legal_count` and
   handles mate detection after the loop.
3. AlphaBeta main loop: mate/stalemate detection moved from pre-loop
   `if (move_list.count == 0)` → post-loop `if (legal_count == 0 &&
   !stopped && !quit)` pattern.
4. Qsearch: switched `generate_all_caps` → `generate_all_caps_pseudo`.
   The existing `if (pos.MakeMove(m) != 1) continue;` already filters
   illegals inline. SEE-on-pseudo-legal is safe — illegal captures
   either get pruned by SEE or rejected by MakeMove with no harm.
5. Left book-move legality check at `searchPosition:1675` alone —
   one-shot, not a hot path, not worth the change risk.

**Validation:** 208/208 unit tests pass. Mate-in-1 detection works
(`Ra8#` found at depth 2). Mate-already position returns clean
`bestmove 0000` with no crash.

**Gauntlet vs `huginn_t2` (100g, 10+0.1):** **+10.43 ± 56.20 Elo,
LOS 64.32%**, 41W/38L/21D. Wide CI but non-regressive — confirms
the bench-evidence shipping logic. Cumulative position vs t2 has
recovered from -17 (post-mobility era at `6865379`) to +10 here.
Not yet at the +50 trigger for refreshing t3 baseline (#4).

---

## Open — bugs

### #12: Fastchess hang at 80 games during mobility gauntlet

- status: open / unreproduced (one-off?)
- priority: low (medium if recurs)
- type: bug

**State:** During the mobility gauntlet (`test_huginn_vs_t2.bat 50`)
on 2026-04-30, fastchess became unresponsive after 80 of 100 games.
Result is the 80-game sample (+4.34 Elo, LOS 54.86%). Could be:
- One-off (cosmic ray, OS scheduling, polyglot book load races)
- Real engine hang on a specific position
- Fastchess bug

**Action:** if recurs in a future gauntlet, capture engine state
(stack trace, last position from PGN, last engine output line) and
investigate. Until then, ignore.

---

## Closed (kept as history)

### LMR table tuning — log(d)·log(m)/2 lookup
- closed: commit `66685f3` (2026-04-30)
- result: +14 Elo marginal, +52.5 cumulative vs `baseline-t1`, LOS 95.68%

### SEE qsearch pruning
- closed: commit `1cce8de` (2026-04-30)
- result: +24 Elo marginal, +38.4 cumulative vs `baseline-t1`, LOS 88.98%

### Aspiration step (a) — root PVS with fail-high break
- closed: commit `28cb2cd` (2026-04-30)
- result: +13.9 Elo vs `baseline-t1`, LOS 65%

### Mobility evaluation
- closed: commit `626257a` (2026-04-30)
- result: +4.34 Elo / 80g vs `baseline-t2`, LOS 54.86%

### `wac_test.py` fixes (castling SAN, temp-file leak, OwnBook off)
- closed: commit `f539927` (2026-04-30)

### `wac_test.py` — `--engine` CLI flag
- closed: commit `01bad2a` (2026-04-30)

### Refresh `baseline-t2`
- closed: commit `2ce78d1` (2026-04-30)
- tag: `baseline-t2 = 66685f3`

### Movegen filter — remove Position copy + preserve capture scoring
- closed: commit `6865379` (2026-05-05)
- bench: depth 11 startpos ~2.7s → ~2.5s (~7-10% per-node speedup, same nodes/PV)
- gauntlet vs t2: -17 ± 73 Elo (inside CI; bench-validated, gauntlet too noisy at 100g)
- shipped because change is correctness-preserving (perft 100%) and removes literal duplicate work + a latent capture-scoring bug
- follow-up available: surgical search-side change (search uses `generate_all_moves` directly + legal-count mate detection) is ~3-4× the win — spike measured 35% faster at depth 11 startpos. Tracked as #14 below.
