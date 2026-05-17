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

## Open — recently deferred (unblocking per-feature via specific fixes, not via one universal lever)

**Four** features in this section share a fingerprint: tactically-
correct implementation, real WAC tactical-solving improvement,
neutral-to-strongly-negative gauntlet Elo. #1, #2, #7, #8 all
regressed despite individually-sound diagnoses. The original
diagnosis was "weak move ordering" and the original prescribed
unblocker was **#3 (continuation history)**.

**Status as of 2026-05-12 — two informative datapoints have falsified
the original model:**

| Date | Feature tested | Type | Result | Interpretation |
|---|---|---|---|---|
| 2026-05-11 | #1 P1a (LMR-exempt-check) | **specific** tactical fix | +22.62 ± 44 Elo / 200g vs t4, LOS 84% — **shipped** | The deferred-section regression *can* recover when re-attempted on better-ordered tip. ~+26 Elo swing from the same code's 2026-05-02 measurement vs t2. |
| 2026-05-12 | #3 continuation history (clean) | **generic** ordering bonus | -1.74 ± 40 Elo / 200g vs t4, LOS 46.6% — **deferred** | The universal-unblocker hypothesis is wrong. With current ordering quality, another additive prev-move bonus is redundant. |

**The deferred section unblocks per-feature via specific search-quality
work, not via one generic lever.** P1a was the proof of concept;
continuation history was the falsifying experiment. The remaining
deferred features (#2/#7/#8) likely need either further cumulative
ordering improvements *or* their own specific fixes (not from the
generic-bonus toolbox: counter-move, continuation history, history
already saturated).

**Strategy update:**
- **#7 LMP** original -56 Elo. By the +25 Elo "P1a lift" heuristic, would land around -31. Worth one cheap re-test (`tier1-stack-broken` cherry-pick + 200g) since the LMP-specific implementation may have benefitted more than P1a from cumulative ordering improvements.
- **#8 aspiration b** re-tested 2026-05-11, still -24/-42. Bottleneck is inter-iteration score stability, not parameter tuning. Wait for further search-quality work before retrying.
- **#2 king safety** original -126 Elo. Too far to recover from cumulative lift alone; needs Texel tuning (#9) or fundamentally different implementation.
- **#3 continuation history** deferred until a different scoring formula or a different temporal signal (2-ply follow-up, e.g.) is worth trying.

Highest-EV next experiments: **#7 LMP re-attempt** (cheap test, completes the deferred-trio re-attempt sweep), then **#17 aspiration re-search on swings** (small scope, addresses #8's score-stability bottleneck), then **#6 lazy SEE in main-search** (specific search-shape lever, similar to P1a in spirit).

### #1: Skip SEE-prune and LMR-reduce on check-giving moves — P1a CLOSED

- status: **P1a closed @ `2dbd856` (2026-05-11)** — +22.62 ± 44.20 Elo /
  200g vs t4, LOS 84.4%. P1b stays deferred (-28 Elo delta in original
  attempt, unlikely to recover).
- priority: was high; P1a shipped, P1b deferred
- type: feature
- attempted: 2026-05-02 (original), 2026-05-11 (re-attempt)
- links: [SEARCH_AND_EVAL.md#priority-1](SEARCH_AND_EVAL.md), `src/search.cpp`
  LMR block

**P1a closure (2026-05-11):** Re-derived the lazy `gives_check` predicate
and added it as a third skip condition to the LMR eligibility check
(after `!in_check && !is_capture && !is_promotion`). Short-circuit
evaluation ensures the predicate is only computed for moves that
would otherwise be reduced.

Implementation: `auto gives_check = [&]() { int opp_king =
pos.king_sq[int(pos.side_to_move)]; return opp_king >= 0 &&
SqAttacked(opp_king, pos, !pos.side_to_move); };` After MakeMove,
side_to_move has flipped to the opponent, so opp_king is the right
target and `!pos.side_to_move` (us, the side that just moved) is the
attacker. Lambda fires lazily via && short-circuit.

Result: **+22.62 ± 44.20 Elo / 200g vs t4, LOS 84.4%**
(85W / 72L / 43D, 53.25% score). Compare to the original
2026-05-02 measurement of -3.47 / LOS 46% vs t2 — a ~+26 Elo swing
for the same code change, attributable to the cumulative move-
ordering improvements shipped since (#13 TT-mate +104, #10 TB +16,
#16 contempt +40). LOS 84% is below the strict ~95% threshold but
the +22 cp point estimate is well above noise, the EV is clearly
positive, and the WAC-side validation (16 more tactics solved,
+5.3pp; specifically WAC.009 finding Bh2+ at depth 8) provides
non-gauntlet evidence the change does what it's designed to do.

**P1b (qsearch SEE-prune exempts check captures) NOT included.** The
original attempt landed at -28 Elo delta from the P1a-only baseline.
The cost of MakeMove + SqAttacked + TakeMove per SEE-pruned capture
in the hot qsearch loop is real; sacrificial check captures are
rare in actual games and usually pruned by other mechanisms when
worthwhile. Keep it deferred.

---

**Below: pre-shipping context preserved for history.**

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

### #26: `board64[64]` piece-on-square cache (CLOSED, pending gauntlet)

- status: **closed code-side** (2026-05-17, shipped pending 200g gauntlet)
- priority: was medium-high (speed win at every depth, low risk)
- type: speed
- source: GPT performance review (`PERFORMANCE_ARCHITECTURE_REVIEW.md`,
  Priority 3).

**What was happening.** `Position::at_sq64(s64)` did a bitboard scan
to derive the piece on a given square: bit test against
`occupied_bitboard`, then `color_bitboards`, then a 6-iteration loop
over `piece_bitboards[c][t]` until the matching piece type was found.
Worst case ~10 bitboard ANDs + branches per call. Called from
`MakeMove` / `TakeMove`, slider movegen capture tagging, MVV-LVA
scoring, TT move validation, history scoring — every search node hit
this function many times.

**The fix.** New `board64[64]` array in `Position`, mirrors the piece
bitboards as a piece-per-square cache. `at_sq64()` becomes a single
array load. Every Position mutator (`set`, `move_piece`, `clear_piece`,
`add_piece`) updates `board64` in lock-step with the bitboards;
`reset()` zeros it; `rebuild_counts()` re-derives it from the piece
bitboards (so FEN-loaded positions and any code path that bypasses
the per-square mutators stay consistent).

**Why this is accuracy-safe.** `board64` is a derived cache — every
read is replaced by an equivalent array load, every write keeps the
two views in sync. No move / eval / search decision changes.

**Bench evidence** (depth 11 startpos, OwnBook off, single thread,
post-#24 t5 baseline):

| Metric | t5 (post-#24) | post-#26 | Δ |
|---|---:|---:|---:|
| Depth-11 time | 263 ms | **234 ms** | **−11%** |
| Depth-11 NPS | 3.55 Mnps | **3.98 Mnps** | **+12%** |
| Depth-11 nodes | 932,229 | 932,234 | bit-identical (±5, input-poll noise) |

Smaller per-call savings than #24 but the function is called many
more times per node, so the aggregate gain is meaningful and
compounds at every depth.

**Cumulative trajectory** (this session + the t4→t5 jump):

| Version | Time @ d11 | NPS @ d11 |
|---|---:|---:|
| pre-#23 (mid-May) | 2285 ms | 2.21 Mnps |
| post-#23 | 516 ms | 2.33 Mnps |
| post-#24 (= t5) | 263 ms | 3.55 Mnps |
| **post-#26** | **234 ms** | **3.98 Mnps** |

pre-#23 → post-#26: **9.8× faster** to depth 11, **+80% NPS**. Plus
the t5 stack already shipped ~+78 Elo over t4 (#25).

**Validation.**
- All 208 GoogleTests pass.
- Perft suite passes (movegen bit-identical).
- d11 startpos PV and node count match the t5 binary exactly
  (mod ±5-node input-polling noise from faster wall-clock).
- 200g Intel gauntlet (2026-05-17, tc=10+0.1, noob_3moves.epd):
  **+12.17 ± 31.80 Elo, LOS 77.42%**, W47/L40/D113, score 51.75%,
  Ptnml(0-2) [4, 23, 40, 28, 5]. Lean positive; DrawRatio 56.5%
  is unusually high (t5 and #26 are close in strength → draws
  dominate, which tightens the CI from typical ±41 to ±32).
- AMD 200g pool pending to push LOS above 95% (predicted pooled
  CI ~±22; if AMD lands similar +10-15 the pooled LOS exceeds
  95% with point estimate ~+12).

**Expected vs measured.** Predicted +15-30 Elo from +12% NPS;
measured +12.17. At the low edge of the band, consistent with
the linear "1 Elo per 1% NPS" pattern observed for #24
(+52% NPS → ~+50 Elo single-Intel) — this slope appears
reliable for accuracy-safe speed wins at TC=10+0.1.

---

### #24: Real magic-bitboard slider attacks (CLOSED, pending gauntlet)

- status: **closed code-side** (2026-05-16, shipped pending 200g gauntlet)
- priority: was high (latent — docs lied; code was ray-walking)
- type: speed / correctness
- source: GPT performance review (`PERFORMANCE_ARCHITECTURE_REVIEW.md`,
  Priority 2), verified against `src/bitboard.cpp:96-165`.

**The lie that wasn't.** `MOVEGEN_COMPARISON.md` and `CLAUDE.md` both
claimed Huginn used magic bitboards for sliders. `bishop_attacks()` /
`rook_attacks()` in `src/bitboard.cpp` actually called
`generate_ray_attacks()` four times each: switch on direction, loop
up to 7 steps, bounds-check + occupancy-test per step. This was a
runtime ray walker dressed up as "magic" in the prose. Every search
node paid this cost through movegen, attack detection, SEE, and
mobility eval — four hot paths simultaneously.

**The fix.** New `src/magic_bitboards.{hpp,cpp}` implementing plain
magic with fixed per-piece shifts (rook 52, bishop 55) and ~2.25 MB
of attack tables. `bishop_attacks` / `rook_attacks` in `bitboard.cpp`
now delegate to `Magic::magic_*_attacks()`; every caller stays
unchanged. Magic numbers are **found at init via a deterministic
xorshift64 PRNG seeded with a hardcoded constant** — functionally
equivalent to hardcoded magics (deterministic across runs/machines)
but expressed as ~30 lines of generator instead of 128
hand-transcribed uint64s. Init adds ~200 ms one-time; subsequent
lookups are O(1) (mask + multiply + shift + indexed load).

**Init verifier.** After finding magics, exhaustively walks every
(square, every subset of mask) and confirms the magic result matches
a ray-walk reference. `std::abort` on any mismatch — better to fail
loudly at startup than silently corrupt search.

**Bench evidence** (depth 11 startpos, OwnBook off, single thread,
post-#23 ray-walk baseline):

| Metric | Pre-#24 (ray-walk) | Post-#24 (magic) | Δ |
|---|---:|---:|---:|
| Depth-11 time | 516 ms | **263 ms** | **−49%** |
| Depth-11 NPS | 2.33 Mnps | **3.55 Mnps** | **+52%** |
| Depth-11 nodes | 1,203,168 | 932,229 | −22% |
| Init cost | ~0 | +200 ms | one-time |

The NPS jump is the headline: a 52% speed increase on every slider
attack call, compounding across four hot paths. Unlike #23 (which
mostly helped depths 10+), this gain applies at *every* depth — so
TC=10+0.1 should see the full benefit since shallow searches benefit
just as much as deep ones.

**Validation.**
- All 208 GoogleTests pass.
- Perft suite passes (movegen bit-identical, as expected for a
  pure-implementation swap verified against the ray walker).
- Init-time verifier walks 64 × max(2^12, 2^9) = ~290K (square,
  occupancy) pairs and confirms magic == ray-walk for every one.
- 200g Intel gauntlet (2026-05-16, tc=10+0.1, noob_3moves.epd):
  **+75.88 ± 41.00 Elo, LOS 99.99%**, W82/L39/D79, score 60.75%,
  Ptnml(0-2) [4, 20, 26, 29, 21]. Combined #23+#24 stack vs t4;
  marginal #24 contribution ≈ +50 Elo over the #23 pooled ~+24.
- 200g AMD gauntlet (2026-05-17, direct frozen t4 vs t5):
  **t5 +79.53 ± 40.59 Elo over t4, LOS ~100%**, t5 W97/L52/D51,
  score 61.25%. Near-identical to Intel's +75.88.
- **Pooled 400g (Intel + AMD): t5 W179/L91/D130, score 61.0%,
  ≈ +77.7 Elo over t4, LOS ≫99.99%.** Exceptionally tight
  cross-machine agreement confirms the #24/#25 ship and the
  `baseline-t5 = 3eab266` freeze. See `gauntlet/README.md`.

**Why this converts cleanly (unlike #23).** #23's bench gain was
concentrated at depth 10+, but TC=10+0.1 plays mostly at depth 9-10,
so only a fraction of the bench delta reached Elo. #24's 52% NPS gain
applies at *every* depth — every search node touches slider attacks
through movegen, attack detection, SEE, or mobility eval. The bench
prediction (~+50 Elo from +52% NPS) matched the gauntlet marginal
almost exactly.

**Doc accuracy.** After this lands, the magic-bitboard claims in
`POSITION_AND_MOVEGEN_ARCHITECTURE.md` (the "Attack set sources"
diagram I added in `cd3d56f`), `MOVEGEN_COMPARISON.md`, and
`CLAUDE.md` are finally truthful. No doc edits needed — the code now
matches what the prose has been claiming for months.

### #23: TT bound classification bug — fix (CLOSED, pending gauntlet)

- status: **closed code-side** (2026-05-15, shipped pending 200g gauntlet)
- priority: was latent-high (never knew it was broken)
- type: bug
- source: GPT performance review (`PERFORMANCE_ARCHITECTURE_REVIEW.md`,
  Priority 1), verified against `src/search.cpp:1519-1593`.

**The bug.** Inside `AlphaBeta`, the move loop raises the local
`alpha` variable whenever a move beats it (`alpha = score`). At the
end of the loop, node-type classification used the **mutated** alpha:

```cpp
if (best_score <= alpha)       node_type = UPPER_BOUND;
else if (best_score >= beta)   node_type = LOWER_BOUND;
else                           node_type = EXACT;
```

After the loop raises alpha to `best_score`, `best_score <= alpha`
is trivially true. Consequence:

| Node behavior | What alpha looked like at line 1587 | Stored as | Should have been |
|---|---|---|---|
| All-fail-low | unchanged at original_alpha | UPPER | UPPER ✓ |
| Alpha-improving (exact PV) | raised to best_score | UPPER | **EXACT** ✗ |
| Beta cutoff | raised to best_score (≥ beta) | UPPER | **LOWER** ✗ |

So **EXACT and LOWER_BOUND entries were structurally unreachable**.
Every alpha-improving node and every beta-cutoff was mis-stored as
UPPER_BOUND. UPPER bounds are the weakest TT pruning material (they
can only cut when probed score ≤ alpha), so the TT was doing roughly
1/3 of its potential work for years.

**The fix.** Capture `original_alpha` before the move loop, use it
(not the mutated `alpha`) in classification. One-line behavior
change, bit-equivalent for movegen.

**Bench evidence** (depth 11 startpos, OwnBook off, single thread):

| Metric | Pre-fix | Post-fix | Δ |
|---|---:|---:|---:|
| Nodes @ d11 | 5,062,280 | 1,203,168 | **−76%** (4.2×) |
| Time @ d11 | 2285 ms | 516 ms | **−78%** (4.4×) |
| TT writes @ d11 | 503,909 | 110,567 | **−78%** |
| NPS | 2.21 Mnps | 2.33 Mnps | basically flat |

NPS is unchanged because this isn't a raw-speed fix — it's a
tree-shape fix. The TT is finally cutting on entries that were
previously stored with bogus labels. Both versions converge on
`d2d4 score cp 25` at depth 11.

**Validation.**
- All 208 GoogleTests pass.
- Perft suite (8 tests covering startpos d1-d3 + Kiwipete d1-d2) passes
  — confirms movegen unchanged.
- 200g Intel gauntlet (2026-05-15, tc=10+0.1, noob_3moves.epd):
  **+8.69 ± 39.12 Elo, LOS 66.90%**, W70/L65/D65, score 51.25%,
  Ptnml(0-2) [8, 26, 32, 21, 13]. Modest positive — much smaller than
  the bench delta would predict.
- 200g AMD gauntlet (2026-05-15, same conditions):
  **+40.13 ± 39.78 Elo, LOS 97.75%**, W71/L48/D81, score 55.75%,
  Ptnml(0-2) [9, 17, 29, 32, 13]. Ran hot relative to Intel.
- **Pooled 400g (Intel + AMD): W141/L113/D146, score 53.5%,
  ≈ +24.4 Elo, LOS ≫95%.** Confirms the ship decision — the lean
  Intel signal was on the low side of the noise band; the true effect
  is a solid ~+24 Elo. See `gauntlet/README.md` for the pooled
  breakdown.

**Why the bench/Elo divergence?** The fix's biggest node-count wins
are at depth 10-11. At depths 7-9 the post-fix tree is near-flat or
slightly worse (different TT contents reorder some lines). At
tc=10+0.1 the engine typically reaches **depth 9-10** in the
middlegame — exactly the depth band where the fix is mixed/marginal.
The 4× depth-11 win is largely theoretical at this TC.

**Pattern to file:** infrastructure fixes can over-promise at fast
TC. The fix is correct (TT bounds now match alpha-beta semantics),
the bench is real (4.2× node reduction at d11 is reproducible), but
TC-bound Elo only converts in proportion to the depth band the
engine actually plays at. A longer TC (e.g. 60+0.6) would likely
show a much larger Elo because deeper search benefits compound.

**Ship decision: SHIPPED.** LOS 67% is lean positive, the fix is
objectively more correct than the buggy code it replaced (TT
metadata now matches alpha-beta semantics), and there's no
regression evidence — perft + 208 tests + positive LOS all clean.
Reverting because "Elo gain was modest" would be the wrong call.

**Why this dwarfs the recent deferral pattern.** Counter-move, LMP,
continuation history, lazy SEE — every recently-deferred experiment
was operating inside a search whose TT was throwing away most of its
pruning power. Some of those experiments may behave differently on
top of the fixed TT. Worth keeping the experiment branches around
(`experiment/counter-move-1500`, `experiment/lmp-quiet-count`,
`experiment/continuation-history`, `experiment/lazy-see-main`,
`experiment/aspiration-step-b`) and revisiting after this gauntlet
lands.

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
`git restore src/search.cpp`. Diff was:
- `++/--info.ply` around recursions in main loop, null move, IID,
  root (`searchPosition`), test entry (`Engine::search`).
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
`#define ENABLE_PLY_TRACKED_COUNTERMOVE 0` in `src/search.cpp`.
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

**Attempt 2 (2026-05-09, on top of t4 = TT-mate + TB + contempt):**
Step 1 + 2a executed. Both score variants vs t4 at 200g / tc=10+0.1 /
concurrency 4:

| Variant | Score slot | Elo vs t4 | LOS |
|---|---|---|---|
| 15000 | between promotions (25K+) and history (~1K) | -10.43 ± 43.28 | 31.7% |
| 1500  | just above history (~1K)                    | +8.69 ± 41.18  | 66.1% |

The 15K → 1500 shift moved the eval by +19 Elo (suggestive but not
statistically conclusive at 200g per leg). Best variant (1500) is
positive in expectation but at LOS 66% — well below the ~95%
threshold for a confident ship.

**Pattern:** counter-move helps slightly when treated as a small
history-style ordering bonus, hurts slightly when treated as a
separate promoted tier between promotions and history. Consistent
with counter-move being conceptually history-flavored (the move that
worked last time we were here) rather than worthy of its own tier.

**Deferred (2026-05-09):** the 1500 variant is too noisy a signal
to ship at LOS 66%. WIP preserved on branch
`experiment/counter-move-1500` (commit `7a66f56`) for future
revival. Search-stack write infrastructure stays in place on
`pure-bitboard-engine` (it costs ~one store per recursion, no Elo
penalty, and is *required* for #3 continuation history regardless
of whether counter-move's gate is on).

**Remaining hypothesis paths if/when we revisit:**
- Tighter measurement at 1500 (400-500g gauntlet) — convert LOS
  66% into a real signal one way or the other.
- Score variants 700 (in-history-range), 5000 (intermediate), or a
  sweep to find the local optimum.
- prev_move-as-parameter approach (the original step-3 fallback) —
  decouples counter-move from `info.ply`, possibly different
  interaction with the rest of the stack.
- Counter-move table aging (depth² bonus aging like history) —
  currently the table is only cleared per-search, not aged within
  iterative deepening.

---

### #3: Continuation history (Tier 2 #11) — deferred (universal-unblocker hypothesis falsified)

- status: deferred 2026-05-12 (was: high-priority unblocker)
- priority: was high, downgraded
- type: feature
- est: 1 session (re-attempted; clean implementation lives on branch)
- links: [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) Tier 2 #11
- WIP branch: `experiment/continuation-history` (commit `182ec87`)
- attempted: 2026-05-05 (first; conflated with ply-tracking bug), 2026-05-12 (clean re-attempt)

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

**Re-attempt 2026-05-12 (clean implementation on top of t4):**
Implemented fresh as a heap-allocated `std::unique_ptr<int[]>` of
size 13×64×13×64 (~2.7 MB). Indexed by
`[prev_piece][prev_to_64][cur_piece][cur_to_64]`. Read in
`pick_next_move`'s quiet branch, additive on top of `search_history`.
Updated on quiet beta-cutoff with `depth²` bonus. Aged ÷4 at
`clear_search_tables` (search start) and ×7/8 every 3 depths during
iterative deepening (paired with `age_search_history`).

**Gauntlet result:** **-1.74 ± 40.32 Elo / 200g vs t4, LOS 46.6%**.
79W / 80L / 41D, score 49.75%. Dead-center neutral. The textbook
+30-50 Elo did **not** materialize.

**Findings:**
- Same shape as #15 counter-move re-attempt: history-style additive
  bonuses using `info.search_stack[info.ply - 1]` as the prev-move
  signal are marginal-at-best in this search stack.
- **The "universal unblocker" hypothesis is falsified.** With the
  current ordering quality (TT/PV/IID/captures/killers/history/P1a),
  another additive prev-move bonus is redundant. #3 is *not* the
  lever that opens up #7/#8/#2.
- The deferred section unblocks **per-feature via specific search-
  quality work** (the 2026-05-11 P1a result was the proof) rather
  than via a single generic ordering boost.

**Deferred.** WIP preserved on `experiment/continuation-history`
(commit `182ec87`) for future revival. Cherry-pickable when we have
reason to believe the heuristic will land positive (e.g., after
other ordering work shifts the equilibrium, or with a different
scoring formula).

**Remaining hypothesis paths if/when we revisit:**
- Scaling tweaks: scale the depth² bonus by some constant, cap the
  per-entry magnitude, or use linear depth bonus instead of depth².
- Different signal: try 2-ply continuation
  `[prev_prev_piece][prev_prev_to][prev_piece][prev_to]` (the "follow-up"
  rather than "counter") to see if a deeper temporal signal is more
  discriminative.
- Wait for more ordering improvements to land, then re-test. The
  break-even may shift as other heuristics mature.
- Bonus-vs-history-only ordering: try replacing search_history's
  contribution with continuation_history's for a side-by-side, rather
  than additive.

---

## Open — medium priority

### #19: Two-machine gauntlet workflow + SPRT — planned for weekend revisit

- status: in progress (2026-05-15) — Part B largely done on the AMD box; Part A (SPRT) still open
- priority: medium
- type: tooling / infrastructure
- est: ~1 hour (Part A) + ~30-45 min one-time (Part B)

**Progress update (2026-05-15) — AMD 7800X3D worker stood up:**
- `test_huginn_vs_t4_amd.bat` created in the repo root: AMD-machine
  variant with the correct `HUGINN_REPO=C:\Users\m_lad\Repos\Huginn`
  (the Intel bat hard-codes `Documents\Repos`, which does not exist on
  this box). Concurrency held at **4 to match the Intel bat** so the
  two 200g halves pool into a homogeneous 400g sample; separate
  `huginn_vs_t4_amd.pgn` / `fastchess_t4_amd.log` so each half is
  machine-identifiable before merge. Engine names + frozen t4 binary
  kept identical to the Intel bat for clean pooling.
- Current Huginn and the frozen t4 baseline (commit `6e3a761`, built in
  an isolated `git worktree`) both configured `-DENABLE_FATHOM=ON` and
  verified UCI-runnable on the 7800X3D — no SIGILL (the AVX-512 SIGILL
  was Intel Raptor Lake disabling AVX-512; Zen 4 is unaffected, and
  these are freshly local builds regardless).
- `baseline-t4` tag created locally at `6e3a761` — #18 documented it
  but the tag only existed on the Intel box; now reproducible here.
- First 200-game current-vs-t4 baseline match running on the AMD box.
- Setup-checklist status (Part B below): items 1-3 + 5 effectively
  satisfied on the AMD box; item 4 (Syzygy `c:\TB\`) still optional.
- Still open: Part A (`-sprt` mode in both bats) and exercising the
  PGN-merge composition step once both machines have a run on disk.

**Goal:** double gauntlet throughput (13700K + 7800X3D in parallel)
plus statistical efficiency via SPRT early-stop. Borderline LOS
results from recent features (P1a 84%, counter-move 66%, today's
LMP) would either clear or fail the ≥95% threshold faster, and
clearer-cut results would stop early instead of running the full
200g.

**Part A — `-sprt` mode in `test_huginn_vs_t4.bat` (do first, free
throughput win on existing hardware):**

Replace the fixed `-rounds 100 -repeat` with fastchess SPRT mode:

```
-sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -rounds 500 -repeat
```

This runs games until the test statistic crosses the H1 (LOS ≥ 95%
that true Elo > 10) or H0 (LOS ≤ 5%) bound, or hits the rounds cap.
Clear positives may stop at 100-200g; clear negatives similarly;
borderline runs can use up to the cap. Same wall-clock as today
when result is borderline, much faster when result is clear.

Tune `elo1` based on what's "shippable" — 10 is conservative;
some engines use 0 (any positive) for new features. Tune `alpha/
beta` for confidence (0.05/0.05 = standard 95%).

**Part B — Add the 7800X3D as a second worker:**

The 7800X3D's 96 MB 3D V-cache is actually a strong fit for chess
TT-bound workloads — likely matches or slightly beats 13700K
per-thread despite the lower clock.

Setup checklist (one-time):
1. Install MSVC Build Tools + CMake (same versions as 13700K for
   bit-equivalent builds).
2. Clone repo, build via `cmake --preset msvc-x64-release
   -DENABLE_FATHOM=ON` then `cmake --build ...`. Verify 208/208
   tests pass.
3. Replicate `fastchess-windows-x86-64/` folder: `fastchess.exe`,
   `noob_3moves.epd`, `huginn_t1.exe` / `t2.exe` / `t3.exe` /
   `t4.exe`, `mtlchessV3.exe`, `MORA110.exe`, `src/performance.bin`.
   ~5 MB total.
4. (Optional, for TB-on testing) Copy `c:\TB\` Syzygy files.
5. Copy `test_huginn_vs_t4.bat`; edit `HUGINN_REPO` path to its
   local checkout.

**Workflow once both are set up:**
- Kick off the same SPRT gauntlet on both machines simultaneously.
- Each picks independent openings via `order=random` (uses
  separate random streams).
- When both stop (each at its own SPRT decision or rounds cap),
  concat the two PGN files and run
  `fastchess.exe -pgnin merged.pgn` for the combined Elo /
  LOS, or just add the W/L/D tallies for a quick sanity check.

**Expected throughput improvement:**
- 13700K solo at concurrency 4: ~480g/hour
- + 7800X3D at concurrency 4 (V-cache parity): ~500-550g/hour
- Combined: ~1000g/hour, roughly 2×

**Statistical impact:**
- 200g gauntlet CI today: ±42 Elo
- 400g effective (two machines, 200g each): ±30 Elo
- 800g effective (two machines, 400g each): ±21 Elo

With SPRT on top, borderline results can run to ~400-800g for
tighter CI in the same wall-clock you'd spend on one 200g today.

**Sequencing:**
1. Add `-sprt` to `test_huginn_vs_t4.bat` this weekend (free on
   13700K alone).
2. Then set up 7800X3D worker.
3. Compose: SPRT on each, run in parallel, merge PGNs.

**Why not OpenBench:** the distributed SPRT framework is overkill
for a one-engineer project without community contributors. The big
OpenBench wins (worker farms, web UI for many concurrent tests)
don't apply here. DIY two-machine + SPRT captures ~90% of the
benefit with ~10% of the setup cost.

---

### #22: Gauntlet results archive (off-repo, Intel-master) — POLICY

- status: established 2026-05-15
- priority: medium (workflow infrastructure)
- type: tooling / methodology
- location: `C:\Users\m_lad\HuginnGauntletArchive\` (outside Huginn repo)
- pairs with: #19 (two-machine workflow), in-repo `gauntlet/` shuttle folder

**Problem this solves:** the in-repo `gauntlet/` folder is the
two-machine shuttle — it overwrites every run (`append=false`, files
deleted at start). That keeps git diffs clean (one experiment per
commit) and avoids ever-growing PGN files in the repo, but **loses
the historical record**. After three runs of the same experiment,
the first two results are gone unless somebody captured them
elsewhere.

The off-repo archive is that "elsewhere." Persistent, owned by the
Intel master (the one machine that has both halves of every pull),
keeps the repo clean.

**Layout:**

```
C:\Users\m_lad\HuginnGauntletArchive\
├── YYYY-MM-DD_<label>\
│   ├── huginn_vs_t<n>_amd.pgn        (~700 KB per 200g)
│   ├── huginn_vs_t<n>_intel.pgn      (~700 KB per 200g)
│   ├── fastchess_t<n>_amd.log
│   ├── fastchess_t<n>_intel.log
│   └── summary.md                    (REQUIRED — see template below)
├── 2026-05-15_p1a_vs_t4_baseline\
└── ...
```

**`summary.md` template** (required for every archived experiment):

```markdown
# YYYY-MM-DD — `<label>`

One-line description of what was tested.

## What was tested
- Engine under test: `<commit>` (one-line description of what this
  commit changes vs the reference)
- Reference engine: `<tag/commit>`
- Effective code delta: <what's actually different>

## Setup
- Time control, concurrency, opening book, round count.

## Per-machine results
Table: date | machine | games | W/L/D | score | Elo ± CI | LOS

## Pooled (Ng across N runs)
- Combined W/L/D
- Pooled Elo and LOS

## Interpretation
Short paragraph: what does this tell us; was it surprising; what
follow-ups (if any).

## Why archived
Per the policy: meaningful baselines, ships, anomalies. NOT routine
failed/deferred experiments (the BACKLOG entry handles those).
```

**Archive policy — when to keep, when to skip:**

| Archive | Skip |
|---|---|
| First two-machine cycle on a new ship-candidate | Routine deferral with clear LOS < 50% |
| Baseline reference for a long-running experiment | Failed runs where the BACKLOG entry captures everything |
| Anomalies worth re-investigating later | Quick sanity-check sniff tests |
| Magnitude-correction pools (multiple runs of same experiment) | Aborted runs / known-bad binaries |
| Long-TC verifications | Re-runs that match an existing archived result |

The BACKLOG entry for each feature is the **authoritative narrative**
(why, what, hypothesis paths, outcome). The archive is the
**evidence locker** for the small number of experiments where raw
data may still matter later. Most BACKLOG entries don't need an
archive companion.

**Workflow for archiving an experiment:**

After both machines finish a pair of runs and both halves are pulled
into the Intel master's `gauntlet/`:

1. Decide: is this worth archiving (per policy above)?
2. `mkdir C:\Users\m_lad\HuginnGauntletArchive\YYYY-MM-DD_<label>`
3. Copy `huginn_vs_*.pgn` + `fastchess_*.log` from `gauntlet/` to
   the new folder.
4. Write `summary.md` from the template above.
5. (Future) maybe add `archive_gauntlet.bat` to automate steps 2-4
   with a label argument; for now the manual flow is fine.

**First archived experiment (2026-05-15):**
`2026-05-15_p1a_vs_t4_baseline\` — the magnitude-correction pool
that revised P1a from "+22 Elo / LOS 84%" (Sunday's single 200g)
to "+6 Elo / LOS 69%" (600g pooled across both machines + Sunday).
First full validation of the two-machine workflow end-to-end.

---

### #25: Refresh `huginn_t5` baseline when cumulative ≥ +50 over t4 — CLOSED

- status: closed @ `3eab266` (2026-05-16)
- tag: `baseline-t5 = 3eab266`
- priority: was medium
- type: maintenance

**Triggered by** the cumulative ship of P1a + #23 + #24 crossing
the +50 Elo threshold over t4 in a single session:
- #1 P1a (LMR-exempt-check) shipped at `2dbd856`: **~+6 Elo pooled
  600g** (single-200g originally +22 but settled to +6 on
  three-machine pool — see gauntlet/README.md history)
- #23 TT bound classification fix shipped at `7d11f23`: **~+24
  Elo pooled 400g**, LOS >>95%
- #24 real magic-bitboard slider attacks shipped at `3eab266`:
  **+75.88 ± 41 / 200g Intel vs t4, LOS 99.99%** (measures the
  combined stack; marginal #24 contribution ≈ +50 Elo on top of
  the pre-#24 cumulative)

The #24 single-200g result alone clears the +50 threshold with
LOS 99.99%, so AMD pool verification was skipped — the ship
direction is unambiguous.

**What shipped:**
- `git tag baseline-t5 3eab266`
- `huginn_t5.exe` copied to `fastchess-windows-x86-64/` from
  `build/msvc-x64-release/bin/Release/huginn.exe` (the #24 magic
  bitboards build with Fathom, 210,944 bytes).
- `test_huginn_vs_t5.bat` (Intel) and `test_huginn_vs_t5_amd.bat`
  (AMD) added, mirroring the t4 versions. Same `-DENABLE_FATHOM=ON`
  configure, same concurrency 4, same `tc=10+0.1`, same opening
  book — so future gauntlets stay apples-to-apples.

**Bench delta** (depth 11 startpos, OwnBook off):

| | t4 (ray-walk) | t5 (magic) | Δ |
|---|---:|---:|---|
| NPS | ~2.3 Mnps | **~3.55 Mnps** | **+52%** |
| Time-to-d11 | ~2280 ms (pre-#23) / ~516 ms (post-#23) | **263 ms** | **−88% vs t4 / −49% vs post-#23** |

The total speed gain stacks the #23 tree-shape win and the #24
raw-throughput win — together they give us ~4.3× faster wall-clock
to depth 11 vs the t4 baseline.

**Going forward:** `test_huginn_vs_t4.bat` and `test_huginn_vs_t4_amd.bat`
are now superseded but kept around for historical regression checks;
new search/eval work should use t5. The next baseline refresh fires
when cumulative gain over t5 hits +50 Elo.

**Notable doc-correctness side-effect:** the magic-bitboard claims
in `MOVEGEN_COMPARISON.md`, `CLAUDE.md`, and the "Attack set
sources" diagram I added to `POSITION_AND_MOVEGEN_ARCHITECTURE.md`
in `cd3d56f` are now truthful — the code finally matches what the
prose has claimed for months.

---

### #18: Refresh `huginn_t4` baseline when cumulative ≥ +50 over t3 — CLOSED

- status: closed @ `6e3a761` (2026-05-09)
- tag: `baseline-t4 = 6e3a761`
- priority: was medium
- type: maintenance

**Triggered by** the cumulative #10 + #16 ship crossing the +50 Elo
threshold:
- #10 (TB integration) shipped at `5347e6d`: **+15.65 ± 42 / 200g
  vs t3, LOS 77%**
- #16 (contempt 25 cp) shipped at `6e3a761`: **+40.13 ± 41 / 200g
  vs t3, LOS 97.5%** — combined cumulative

**What shipped:**
- `git tag baseline-t4 6e3a761`
- `huginn_t4.exe` copied to `fastchess-windows-x86-64/` from
  `build/msvc-x64-release/bin/Release/huginn.exe` (the contempt-on
  build with Fathom).
- `test_huginn_vs_t4.bat` added (mirrors `test_huginn_vs_t3.bat`,
  notation=san, concurrency 4, default 100 rounds = 200 games).
- t4 baseline includes the Syzygy TB integration; the bat configures
  with `-DENABLE_FATHOM=ON` so current Huginn matches.

**Going forward:** `test_huginn_vs_t3.bat` is now superseded but
kept around for historical regression checks; new search/eval work
should use t4. The next baseline refresh fires when cumulative gain
over t4 hits +50 Elo.

---

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

**Chain validation (2026-05-07, `test_t_chain_tournament.bat`):**
3-way round-robin t1 vs t2 vs t3, 200 games per engine
(50 rounds × 3 pairings × 2 colors), tc=10+0.1, concurrency 2.
Tournament-average-relative Elos:

| Engine | Elo (vs avg) | ± | Score |
|---|---|---|---|
| t3 | +77.7 | 45.9 | 61.0% |
| t2 | −10.4 | 45.7 | 48.5% |
| t1 | −66.8 | 44.6 | 40.5% |

Pairwise differentials vs prior gauntlets:

| Pair | Observed | Expected | Δ |
|---|---|---|---|
| t2 − t1 | +56.4 | +52.5 (baseline-t2 closing) | +3.9 |
| t3 − t2 | +88.1 | +104 (#13 closing) | −15.9 |
| t3 − t1 | +144.5 | +156 (additive) | −11.5 |

All differentials inside the per-leg noise band (~±46 Elo at 200g).
Chain is additive within statistical limits — no non-transitive
interaction detected. Validates the "~+118 Elo of recent search
work" claim from the t-chain. Slight t3−t2 under-performance vs the
original +104 reading is consistent with the 100g CI ±62 plus
possible concurrency=2-vs-1 compression.

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
- **2026-05-09, post-#10 (TB) + post-#16 (contempt), vs `mtlchessV3.exe`
  (100g):** 2W/95L/3D, **-576 ± 250 Elo, LOS 0%**. Wide CI overlaps
  the 2026-05-06 reading at [-542, -326]; can't statistically claim
  a regression. But the score percentage dropped from 7.5% → 3.5%
  which is the kind of move you'd expect from contempt's textbook
  downside: against a much-stronger opponent (MTL ~+400 Elo over
  Huginn), draw-by-repetition opportunities are precious, and
  contempt biases away from accepting them. The +40 Elo gain vs t3
  (LOS 97.5%) is the trustworthy measurement for regression tracking
  on peer baselines; the MTL number is consistent with contempt
  being net-negative against opponents we have no realistic chance
  of beating, which is its known caveat.

  **Strategic implication:** contempt is well-suited to the
  t-chain ladder (peer baselines, contempt helps vs near-equal
  opponents) but should be revisited (drop, reduce, or make
  asymmetric / root-only) if a future regime requires playing
  much-stronger opponents in tournaments.

Trustworthy gauge of recent Huginn progress remains the internal
t-chain (t1 → t2 → t3 → t4), not external MTL calibration.

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

### #16: Contempt — penalize draw scores when engine thinks it's losing — CLOSED

- status: closed (2026-05-09)
- priority: was medium
- type: feature / search robustness
- final result: **+40.13 ± 40.68 Elo / 200g vs t3, LOS 97.5%**
  (89W/66L/45D, score 55.75%). Marginal contribution over the
  post-#10 baseline of +15.65 / LOS 77% is roughly +24 Elo;
  marginal CI is wide at 200g but the LOS jump 77% → 97.5% is
  the more reliable signal.

**Motivating evidence (2026-05-08):** game where Huginn (white) was
materially down in Q+P vs Q+3P endgame at FEN
`8/8/1kp1Q3/2p5/5P1K/p2q4/8/8 w - - 0 70`. Fresh search of this
position converges to **-287 cp** at depth 12-13 with `Qa2` (block
the a-pawn) as best move. In the actual game, the engine searched
to depth 14 and reported `Kg5 0.00/14` — and the `0.00` was *not*
noise; it was the engine finding what it believed to be a forced
3-fold-repetition / 50-move-rule draw line. The engine then
committed to Kg5 because every alternative scored worse than 0.00
against the position's true -290-ish baseline. Black deviated from
the "forced" repetition, promoted the a-pawn, won the game.

**Mechanism this fix targets:** when the engine is losing, it has
no incentive to look hard at non-drawing alternatives — a 0.00
draw line dominates a -290 cp non-drawing line. So the search
optimistically commits to any apparent draw, even if the
opponent's "forced" cooperation isn't actually forced. The fix:
penalize draw scores from the engine's own perspective when its
material/eval indicates it's losing, so the search prefers playing
out a slightly-better-than-losing line to a fragile "draw."

**Implementation sketch:**
- Add a `contempt` parameter (cp value, e.g. 25-50).
- At repetition / 50-move-rule / stalemate score returns, instead
  of returning `0`, return `-contempt` from the engine's POV when
  the engine is the side considering taking the draw.
- Equivalently: in the search, when the score-to-return would be
  exactly 0 due to a draw rule, subtract `contempt` if it's the
  engine's move that triggers the draw, add `contempt` if it's the
  opponent's.
- Tune via gauntlet — typical contempt values are 10-50 cp;
  too high makes the engine refuse legitimate draws when actually
  drawn.

**Caveat:** contempt is contentious in modern engines because at
the top of the search tree the engine is supposed to play the move
that gives the best result, and "best result" against a draw-only
line should be 0.5 not 0. But for this exact failure mode (engine
prefers fragile draws to playing-on-when-losing), small contempt
is well-known to help.

**Closure (2026-05-09):** shipped at `CONTEMPT = 25` cp at the top
of `src/search.cpp`. Applied at three draw-score sites:
- `evaluate()`: insufficient-material draw → `-CONTEMPT`
- AlphaBeta: 3-fold repetition draw → `-CONTEMPT`
- AlphaBeta: stalemate after no-legal-moves → `-CONTEMPT`

The single-value approach (always `-CONTEMPT`, no per-side
tracking) works because in negamax the score is from side-to-move's
POV, and both sides equally prefer to win — both view a draw as
slightly disliked, and negation handles the perspective flip
correctly. Standard simple implementation; can be made asymmetric
(only at root) or material-dependent later if tuning suggests it.

Gauntlet result: **+40.13 ± 40.68 Elo / 200g vs t3, LOS 97.5%**
(89W/66L/45D, 55.75% score). Marginal contribution over the
post-#10 baseline of +15.65 / LOS 77% is roughly +24 Elo; the LOS
jump 77% → 97.5% is the more reliable signal at 200g sample size.
208/208 unit tests pass; KPK TB probe still returns the correct
mate score (cp 28000) — contempt doesn't perturb non-draw paths.

`CONTEMPT = 25` was the first value tried and worked on the first
attempt. No tuning sweep was done; values in the 10-50 cp range are
all defensible per the literature. If a future gauntlet shows the
engine refusing legitimate draws when actually drawn, drop to 10-15.

---

### #17: Aspiration-window widening / re-search on large score swings

- status: open / unblocked
- priority: medium
- type: feature / search robustness
- est: 1 session

**Motivating evidence (2026-05-08):** same game as #16. The in-game
search reported Kg5 with `0.00/14`. A fresh search of the same
position at the same nominal depth had eval around `-280` to `-290`
and chose `Qa2`. The 280-cp gap implies that during iterative
deepening the in-game search saw the eval collapse from "negative
several pawns" (depth 10-12) to "exact zero" (depth 14) — a swing
that *should* have triggered a re-search to verify, but evidently
didn't. The engine just took the depth-14 result and played.

**Mechanism this fix targets:** large score swings between
iterative-deepening iterations are diagnostic of either a real
tactical discovery (good — we want to find these) or a search
artifact (bad — TT pollution, horizon effect, false repetition).
Re-searching at full window with extended time on big swings lets
the engine verify before committing. If the swing was real, the
re-search confirms; if it was an artifact, the re-search uncovers
the truth.

**Implementation sketch:**
- After each iterative-deepening iteration completes, compare the
  new score to the prior depth's score.
- If `|new - prev| > SWING_THRESHOLD` (e.g. 100-150 cp), and not
  already at max depth, kick off a verification search at the same
  depth with full window and a small time bonus.
- If the verification confirms the swing, accept it. If it
  contradicts, take the verification's score (which had more time
  and a clean window).
- Threshold and time-bonus values are gauntlet-tunable.

**Relation to #8:** #8 is about aspiration-window step (b) — using
narrow windows on the *next* iteration assuming small score
changes. This (#17) is the inverse: detecting when the assumption
is wrong and re-searching. Could be implemented as an aspiration
fail-low/fail-high handler in #8's framework when that lands.

**Likely impact:** small in noise-dominated middlegames, larger
in endgames where horizon effects matter (exactly the failure mode
that hit Kg5).

---

## Open — low priority

### #27: Unorthodox early-queen PV (d1d3 / d8d6) — deferred

- status: **deferred** (2026-05-17, Option D selected)
- priority: low (cosmetic / opening-only symptom; doesn't show in
  middlegame or endgame)
- type: evaluation quality

**Symptom.** Depth-11 startpos search converges on
`d2d4 d7d5 d1d3 d8d6` (PV from the post-#26 bench). The d1→d3 and
d8→d6 queen moves are bad opening chess — they expose the queen to
harassment before minors are developed.

**Root cause** (`src/evaluation.hpp:172`, `QUEEN_TABLE`). The queen
PST has a +5 cp gradient toward center ranks 3-6 / files c-f, with
−5 on the back rank. Net **+10 cp incentive** to move queen d1→d3 or
d8→d6. There is **no compensating term** for early-queen development
(no "minors out first" heuristic, no phase-gated queen PST, no
mobility-quality scoring that distinguishes "queen has many safe
squares" from "queen on a square enemy minors will hit").

At our search depth of 9-10 plies in tc=10+0.1 middlegame play, the
engine can't see far enough into "queen harassed, lost tempi, lost
the resulting position" to override the immediate +10 cp PST gain.
A deeper search (MTL likely reaches depth 12-13 at the same TC)
would find the punishment naturally.

**Fix options considered:**

| Option | Effort | Estimated Elo | Risk |
|---|---|---|---|
| A. Flatten queen PST center to 0 | 5 min | +5-15 | low |
| B. Tempered queen PST + back-rank bonus, phase-gated | 30 min | +10-25 | low-medium |
| C. Minor-development penalty (-15-25 cp if queen off back rank but minors home) | 60-90 min | +15-30 | medium |
| D. Defer; rely on future depth/pruning improvements | 0 | +0 directly | n/a |

**Why D (deferred).** The early-queen PV is a *symptom* of search
depth, not a fundamental eval flaw. Adding a hand-tuned penalty fixes
the immediate cosmetic issue but doesn't address the underlying
"engine can't see deep enough" problem — and the penalty itself can
mis-fire in positions where an early queen move IS the correct play
(early-attack openings, gambits, some defensive setups). Investing
in tree-shape improvements (aspiration windows that work post-#23 TT
fix, LMP retry, continuation history retry) targets the real
limitation. The d1d3 / d8d6 line is also opening-only — it goes away
once we're out of book-ish positions, so its real-game cost is small.

**When to revisit.** If a future tree-shape ship pushes effective
search depth to 11+ at tc=10+0.1 and the PV is *still* d1d3/d8d6,
that means the PST gradient is the real driver and Option A or B
becomes worth shipping. Or if a strong external opponent (MTL,
MORA) starts winning specifically on early-queen exploitation, the
eval fix moves up in priority.

---

### #20: Trapped-bishop eval pattern (a7/h7/a2/h2) — deferred

- status: deferred (2026-05-13)
- priority: low
- type: feature
- est: half-session (impl) + tuning iterations
- WIP branch: `experiment/trapped-bishop` (commit `c4a330e`)

**Pattern:** classic textbook trap — a white bishop on h7 with black
pawns on g6 AND f7 has no safe retreat. Eventually lost for a pawn
or two over the search horizon. Eval flags this directly with a
fixed penalty per matching pattern. Four mirror patterns:
- WB on a7 trapped by BP on b6 + c7
- WB on h7 trapped by BP on g6 + f7
- BB on a2 trapped by WP on b3 + c2
- BB on h2 trapped by WP on g3 + f2

**Implementation:** four bitboard AND-mask comparisons in `evaluate()`
right after the bishop-pair bonus. Added `TRAPPED_BISHOP_PENALTY = 150 cp`
to `EvalParams`. Mirror-symmetric (208/208 unit tests pass including
all 8 eval-symmetry tests).

**Gauntlet result vs t4 (200g, tc=10+0.1, concurrency 4):**

  **-41.89 ± 44.80 Elo, LOS 3.13%**
  69W / 93L / 38D, score 44.0%

Clear regression — 96.87% confidence it's truly negative.

**Hypotheses for the regression** (all speculative without instrumentation):
- **Penalty too large at 150 cp.** Stockfish uses ~50 mg / 50 eg for
  this exact pattern. Could be over-correcting.
- **Search-shape disruption.** Even when the bishop can escape via
  Bg8, the static eval at every node shows -150, biasing the search
  to over-prioritize the escape over better alternatives elsewhere.
  E.g., the engine plays Bg8 when a better tactical option existed.
- **Rare-but-impactful.** Pattern fires in <5% of games but when it
  fires the engine plays disruptively across the whole game.
- **The pattern is too narrow.** It only fires when the bishop is
  ALREADY on h7/a7 — but real "trapped" situations also include
  approaches *toward* the trap that the engine should avoid. This
  doesn't catch those.

**Deferred** with WIP preserved on `experiment/trapped-bishop`
(commit `c4a330e`).

**Remaining hypothesis paths if/when we revisit:**
- Lower penalty (try 50 or 75 instead of 150). Match Stockfish's value.
- Add a "but can the bishop escape?" check — only penalize if the
  escape square (g8 / a8 / g1 / a1) is occupied or attacked.
- Broaden the pattern set: knight on a8/h8/a1/h1, rook on h1 behind
  king after castling failures.
- Defer until other eval work (king safety re-attempt, mobility
  tuning) lands — the strategic effect may interact better with a
  more sophisticated eval baseline.

---

### #6: Lazy SEE in main-search capture ordering — deferred (2026-05-13)

- status: deferred (2026-05-13)
- priority: low
- type: feature
- est: half-session
- links: [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) Tier 2 #10 partial
- WIP branch: `experiment/lazy-see-main` (commit `59b0fad`)

**Status:** SEE is wired in qsearch (commit `1cce8de`) but main-search
capture ordering still uses pure MVV-LVA. Splitting captures into
`SEE ≥ 0` and `SEE < 0` buckets in `pick_next_move` would push losing
trades below quiets. Eager SEE during move generation costs too much
(measured at king-safety attempt time); lazy SEE at pick time is the
standard approach.

**Attempt 1 (2026-05-13, on top of t4 = TT-mate + TB + contempt + P1a):**

Implemented split:
- Good captures (SEE >= 0): score = 1,000,000 + MVV-LVA (existing)
- Bad captures (SEE  < 0): score = MVV-LVA (below history-positive quiets)

SEE computed once per capture during the initial scoring pass; cached
in `move.score` for subsequent selection-sort calls.

**Bench (depth 10 startpos): 868k nodes / 384 ms** — ~50% fewer
nodes than baseline. Tree shape materially smaller at the same NPS.

**Gauntlet result vs t4 (200g, tc=10+0.1, concurrency 4):**

  **-15.65 ± 48.09 Elo, LOS 26.03%**
  78W / 87L / 35D, score 47.75%

Neutral-trending-negative. Smaller tree but worse decisions on net.

**Diagnosis — textbook asymmetry between qsearch SEE and main-search
SEE:**

- **qsearch SEE *prunes*** the move entirely (skip): saves a small
  amount of time per losing capture, no decision risk because qsearch
  is for static-eval refinement.
- **main-search SEE *reorders*** (still searches, just later): saves
  nothing if the move is needed; potentially harms when the SEE-losing
  capture is a real tactical sacrifice (Bxh7+, knight sacs,
  queen-takes-defended-but-forks).

Static SEE doesn't see follow-up tactics. By demoting SEE<0 captures
after positive-history quiets, we examine them too late in some
positions where they're the actual best move.

**Deferred.** WIP preserved on `experiment/lazy-see-main` (commit
`59b0fad`).

**Remaining hypothesis paths if/when we revisit:**
- **More conservative threshold**: only demote captures where
  SEE < -100 (or some other negative threshold) instead of SEE < 0.
  Preserves "obviously bad" demotion without affecting "borderline"
  cases that might be tactical sacrifices.
- **Don't demote captures that give check**: check-giving captures
  drive forcing sequences regardless of static SEE. Add a
  `gives_check()` exemption like P1a does for LMR.
- **Different demotion magnitude**: instead of `score = MVV-LVA`,
  try `score = -1000 + MVV-LVA` (well below history but still
  positively ordered). May change interaction with the search.
- **Combine with main-search SEE *pruning*** (vs ordering): at
  certain depths, actually skip SEE<0 captures entirely (mirror
  of qsearch). More aggressive but cleaner trade-off than reorder.

---

### #7: LMP fix — Tier 3 revisit

- status: re-deferred (2026-05-13)
- priority: low
- type: feature
- est: 1 session (impl on branch `experiment/lmp-quiet-count`,
  needs further upstream search-quality work to land positive)
- links: [SEARCH_AND_EVAL.md#lmp-fix-attempts](SEARCH_AND_EVAL.md),
  defer commit `5ada94c`
- WIP branch: `experiment/lmp-quiet-count` (commit `a715ef4`)

**State:** Two iterations attempted originally (depth-1-2 incl, then
min-depth-3 only). Both regressed (-254, -56 Elo). The per-node
`quiet_count` plumbing was the necessary fix.

**Originally blocked by:** #3 (continuation history) — but that
hypothesis is now falsified (see #3 closure 2026-05-12). LMP unblocks
the same way as other deferred features: via incremental cumulative
search-quality improvements, not via one specific lever.

**Attempt 2 (2026-05-13, on top of t4 = TT-mate + TB + contempt + P1a):**

Implemented the `quiet_count`-based LMP fresh on top of t4, with
`LMP_MIN_DEPTH = 3`, `LMP_MAX_DEPTH = 6`, threshold = `4 + depth²`
(= 13, 20, 29, 40 at d=3..6). Pruning fires before MakeMove to save
the make-then-unmake cost.

Bench (depth 10 startpos): **1.07M nodes / 462 ms** — a clean 40%
node reduction from the no-LMP path. The pruning **IS doing real
work**; the freed time just isn't translating to better Elo at
fast TC.

Gauntlet results vs t4 at tc=10+0.1, concurrency 4:

| Run | W/L/D | Score | Elo | LOS |
|---|---|---|---|---|
| Run 1 (200g) | 76 / 80 / 44 | 49.0% | -6.95 ± 41.33 | 37.0% |
| Run 2 (200g) | 71 / 88 / 41 | 45.75% | -29.60 ± 41.08 | 7.7% |
| **Combined (400g)** | **147 / 168 / 85** | **47.4%** | **~-18 ± 30** | ~negative |

**+49 Elo lift** from original -56 to current -7 (run 1) or -18
(combined). Biggest swing of any deferred-trio re-attempt yet
(vs +25 for P1a, +25 for aspiration b). The cumulative ordering
work IS lifting LMP, just not by enough to clear zero.

**Pattern across all deferred-trio re-attempts:**
- #1 P1a: original -3 → +22 (lift +25) → **shipped**
- #8 aspiration b: original -49 → -24 (lift +25) → deferred
- #7 LMP: original -56 → ~-18 (lift +38-49) → **deferred**
- #2 king safety: original -126 → not yet tested; predicted ~-78-100

So we have a consistent lift mechanism but the original starting
depth determines where each lands. Only P1a was close enough to
zero to clear.

**Re-deferred.** WIP preserved on `experiment/lmp-quiet-count`
(commit `a715ef4`) for future revival.

**Remaining hypothesis paths if/when we revisit:**
- Wait for further ordering improvements to land; the lift trend
  may push LMP positive after 1-2 more search-quality features.
- Less aggressive threshold (e.g., `6 + depth²` or `LMP_MIN_DEPTH = 4`)
  to reduce false-pruning rate.
- Combine with continuation history score in `pick_next_move` once
  CH ships (if it ever does) — better quiet-move ordering would
  make LMP's "first K quiet moves contain the best ones" assumption
  more reliable.

---

### #8: Aspiration step (b) — Tier 3 revisit

- status: re-deferred (2026-05-11)
- priority: low
- type: feature
- est: 1 session
- links: [SEARCH_AND_EVAL.md#aspiration-step-b-deferred](SEARCH_AND_EVAL.md)
- WIP branch: `experiment/aspiration-step-b` (commit `fa6c66e`)

**State:** Step (a) (root PVS with fail-high break) shipped at `28cb2cd`
(+13.9 Elo). Step (b) (actual aspiration window) tried at two tunings,
both regressed (-75, -49 Elo). Same root cause as #7: weak move
ordering causes too many fail-low/fail-high re-searches that cost more
than the tighter-window cutoffs save.

**Originally blocked by:** #3 (continuation history). Possibly also
#2 (king safety) for more stable inter-iteration scores.

**Attempt 2 (2026-05-11, on top of t4 = TT-mate + TB + contempt + P1a):**
Re-implemented step (b) v2 (delta×2 widening, no beta-pull on
fail-low, no alpha-pull on fail-high — only widen the failing side).
Tried two parameter sets vs t4 at 200g / tc=10+0.1 / concurrency 4:

| Variant | INITIAL_DELTA | MIN_DEPTH | Elo vs t4 | LOS |
|---|---|---|---|---|
| v2 | 50  | 4 | -24.36 ± 41.83 | 12.5% |
| v3 | 100 | 6 | -41.89 ± 37.07 |  1.2% |

v3 was strictly worse — wider initial window meant less benefit
when in-window, higher min-depth meant aspiration didn't help at
the early depths where TT was building up.

**Pattern matches #1 P1a:** v2's -24 vs original -49 is a +25 Elo
swing, same magnitude as P1a's recovery (-3 → +22 = +25). The
cumulative t2→t4 ordering work IS lifting step (b), just not by
enough to land positive — starting point was deeper in the hole
than P1a was.

**Re-deferred.** WIP preserved on branch
`experiment/aspiration-step-b` (commit `fa6c66e`) for future revival.

**Remaining hypothesis paths if/when we revisit:**
- Wait for #3 (continuation history) and more ordering improvements
  to accumulate. Each +25 Elo of ordering lift makes step (b) ~25 Elo
  closer to positive; another 1-2 features should close the gap.
- Try **adaptive delta** based on iteration's score volatility (the
  Stockfish approach: small delta when scores are stable, larger
  when they swing).
- Try **score-trend-aware widening** (asymmetric, e.g., widen alpha
  more on fail-low if score has been dropping, less if rising).
- Try **incremental window** (start tight at low depth, grow with
  depth) instead of fixed initial delta.

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

### #10: Wire up Syzygy tablebase probe — CLOSED

- status: closed (2026-05-08)
- final commit: `5347e6d`
- final result: **+15.65 ± 42 Elo / 200g vs t3, LOS 77%** (neutral within noise)
- priority: was low
- type: feature

**Took dramatically longer than the BACKLOG's "1 hour, gate already
exists" estimate.** Closure was a nine-commit fix-stack across two
phases: integration wire-up, then a regression hunt after the first
gauntlet measurement showed -63 Elo.

**Phase 1 — Integration wire-up** (`d79900a`). The original gate-flip
exposed five distinct integration problems that all had to be fixed
before the engine would even produce the right TB score on a
hand-coded smoke test:

1. **Default-OFF Fathom**: `ENABLE_FATHOM` defaulted off in
   `CMakeLists.txt`; Huginn was using the stub implementation that
   could never probe. Left default-OFF; opt in via `-DENABLE_FATHOM=ON`.
2. **Default path inconsistency**: `src/syzygy_tablebase.cpp` had
   `d:\\TB\\` as the internal default while UCI advertised `c:\\TB\\`.
   Unified to `c:\\TB\\`.
3. **Fathom MSVC C4717 stack-overflow warnings**: `tbprobe.c:422`
   does `#include "tbchess.c"`, and tbchess.c lines 31-33 redefine
   `popcount`/`lsb`/`poplsb` macros to map to the helper API. The
   redefinitions leak into the rest of `tbprobe.c`, turning every
   popcount call (including those inside `tb_probe_wdl_impl`) into
   self-recursive infinite loops. First TB-probable position hung the
   engine. Fixed with `#pragma push_macro` / `#pragma pop_macro`
   around the include.
4. **`unsigned`-vs-`uint64_t` truncation in probe_wdl**: bitboard
   accumulators were declared `unsigned` (32-bit). Pieces on ranks 5-8
   (sq64 ≥ 32) silently truncated to zero, so every probe returned
   `TB_RESULT_FAILED`. Fixed to `uint64_t`.
5. **The actual gate flip**: removed `if (false && ...)` at
   `src/search.cpp:1218`.

After phase 1: KPK smoke test returned `cp 28000` at depth 1 (TB_WIN
encoded as MATE-1000) — the first proof of life — and 208/208 unit
tests passed.

**Phase 2 — Regression hunt** (`adb043d` → `c37f991` → `7a2311e` →
`5347e6d`). With phase 1 working at the unit-test level, the first
100g gauntlet vs t3 measured **-63 Elo, LOS 0.79%**. A 200g re-run
at -57.86 / LOS 0.16% confirmed it was a real regression, not noise.
Four more bugs got peeled off:

| Commit | Fix | Gauntlet result (200g unless noted) |
|---|---|---|
| `d79900a` | Phase 1 (initial wire-up) | -63 Elo / 100g, LOS 0.79% |
| `adb043d` | Use safe wrapper `tb_probe_wdl` (rule50/castling guards) — `tb_probe_wdl_impl` doesn't take those params, so positions with castling rights or non-zero halfmove silently got the no-castling/rule50=0 reading and corrupted TT | -57.86, LOS 0.16% |
| `c37f991` | `can_probe` fast-path via `popcount(occupied_bitboard)` instead of iterating 120 squares calling `pos.at()` per leaf | -48.96, LOS 0.98% |
| `7a2311e` | Don't TT-cache TB results (halfmove_clock isn't part of zobrist; cached TB scores returned for any rule50 value); classify CURSED_WIN/BLESSED_LOSS as draws (≈ ±1) instead of full mates | -24.36, LOS 14.4% |
| `5347e6d` | Build TB probe inputs directly from `piece_bitboards` instead of iterating 120 squares — same sq64 layout, no per-square translation needed | **+15.65, LOS 76.9%** |

**Net swing: +79 Elo across the four follow-up fixes.** Final
position is neutral within the per-leg ±42 Elo CI at 200g.

**Lessons / patterns visible in the fix-stack:**
- Mailbox-era code paths (`pos.at()` loops over all 120 squares)
  hidden inside what looked like cold paths still hit the hot search
  loop because they ran at every depth-≤-1 leaf. The `can_probe` fix
  alone recovered ~9 Elo. Worth grepping for similar 120-iterate
  patterns elsewhere when chasing future overhead.
- Probing TBs and storing the result in the regular TT is a known
  pitfall when halfmove_clock isn't in the zobrist key — Stockfish
  doesn't cache them either. We shouldn't have copied the original
  caching pattern without thinking.
- CURSED_WIN/BLESSED_LOSS misclassification was the most surprising
  contributor — a small fraction of probes return these, but treating
  them as full mates pollutes search-tree shape under fastchess's
  standard 50-move-rule adjudication.

**To use TB at runtime:** configure with `-DENABLE_FATHOM=ON` (option
defaults OFF for clean builds). `test_huginn_vs_t3.bat` and successors
already handle this. Default path is `c:\\TB\\`; override via the UCI
`SyzygyPath` option. Without Fathom enabled, the engine falls back to
the stub which cleanly reports "not available" and plays without TB.

**Strength impact:** measured at neutral within noise at tc=10+0.1.
Most games at fast time controls end before reaching the ≤5-piece
slice where TB fires. Matters more at long time controls and in
adjudication-free games. The integration is correct and ready;
revisit the strength question if/when we test at tc=60+0.6 or longer.

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
   `generate_all_moves`: `Engine::search`, `AlphaBeta` main
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

### #21: PV continues past threefold repetition (cosmetic, pollutes analysis)

- status: open (filed 2026-05-15)
- priority: low
- type: bug

**State:** During the 2026-05-15 AMD t4 baseline gauntlet
(`huginn_vs_t4_amd.pgn`), fastchess logged repeated
`Warning; PV continues after threefold repetition` from both builds,
e.g. `info depth 20 score cp 25 ... pv h7g8 f4e3 f7e5 e3f4 e5f7 f4e3
f7e5 …` — a 2-cycle move loop padded out to the full PV array, with a
static score and depth still climbing 17→20.

**Diagnosis (likely, unconfirmed):** `get_pv_line`
([minimal_search.cpp:597](src/minimal_search.cpp#L597)) walks the
PV/TT chain without a repetition guard, so in a drawn-by-repetition
line it follows the cycle until the 64-slot PV array fills. The
reported `score cp 25` equals the shipped contempt value (#16,
`CONTEMPT = 25`), which indicates the search itself *is* correctly
scoring the position as a (contempt-adjusted) draw — only the printed
PV and the still-incrementing depth are garbage.

**Impact:** none on playing strength (the draw is scored correctly).
Cosmetic in GUIs, but it **pollutes any PV/depth/score-based
analysis** — e.g. the WAC depth-comparison tables in
[SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md): any position that resolves
to a repetition will show a meaningless looping PV and an inflated
depth at a flat contempt score.

**Action:** in `get_pv_line`, stop appending once the reconstructed
line hits a position key seen earlier in the same walk (or once the
half-move/repetition state would be a draw). Low priority — fix when
next touching PV extraction or analysis tooling.

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
