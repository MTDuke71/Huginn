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

#1, #7 (LMP), and #8 (aspiration step b) all regressed in gauntlet
testing despite individual diagnoses being correct. Common cause:
weak move ordering. All three should unblock when continuation
history (#3) lands.

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

## Open — high priority

### #2: Re-attempt king safety on top of mobility

- status: open / unblocked
- priority: high
- type: feature
- est: 1 session (cherry-pick + tune + gauntlet)
- links: [SEARCH_AND_EVAL.md#king-safety-attempt](SEARCH_AND_EVAL.md)
  defer note from commit `30aba92`

**Evidence:** Three iterations of king safety landed at -21 / -3 / -17
Elo. v2 (presence-based attack count, `DANGER_MAX = 200`) was best.
Hypothesis from defer note: king safety needs mobility's position-based
penalty interaction to pay off (less mobile = harder to escape attacks).

**Plan:**
1. Re-apply v2's eval params on top of current tip (post-mobility).
2. Re-run gauntlet vs `huginn_t2`.
3. If positive, commit. If still neutral, push back into Tier 3 with
   a note that mobility wasn't the missing companion.

**Unblocked since:** mobility shipped (commit `626257a`).

---

## Open — medium priority

### #3: Continuation history (Tier 2 #11)

- status: open / unblocked
- priority: medium
- type: feature
- est: 1 session
- links: [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) Tier 2 #11

**Evidence:** Two-ply move history (`prev_move → this_move`) for ordering.
Compounds with SEE (better tactical move ordering surfaces winning
sequences earlier). Stockfish-class engines all have it.

**Why important here:** prerequisite for revisiting #4 (LMP) and #5
(aspiration step b). Both deferred features need stronger move ordering
to pay off; continuation history is the cheapest way to get there.

**Plan:** add `cmh[piece][to][piece2][to2]` table. Update on quiet-move
beta cutoff with `depth²` bonus. Use as scoring contribution in
`pick_next_move`. Test alone vs t2.

---

### #4: Refresh `huginn_t3` baseline when cumulative ≥ +50 over t2

- status: open / blocked (waiting on cumulative gain trigger)
- priority: medium
- type: maintenance
- est: 5 minutes (tag + build + copy + new vs-t3 bat)
- links: [SEARCH_AND_EVAL.md#next-steps](SEARCH_AND_EVAL.md) #8

**Trigger:** any single feature that pushes the gauntlet vs `huginn_t2`
to ≥ +50 Elo, OR cumulative across multiple features. Today the only
shipped feature on top of t2 is mobility (+4). #1 and #2 should each
push us past the trigger.

**Plan:** when triggered, mirror what we did for `baseline-t2`:
```
git tag baseline-t3 <commit>
build → copy huginn_t3.exe to fastchess folder
write test_huginn_vs_t3.bat
update tooling section in SEARCH_AND_EVAL.md
update test_epd_diff.py default opponent to t3
```

---

### #5: Recalibrate vs MTLChess_v0.3 after each major fix

- status: open / recurring
- priority: medium
- type: maintenance
- est: 5 minutes per run (script auto-rebuilds)

**Last run (2026-04-30, post-mobility):** 2W/17L/1D — Huginn at ~−340
Elo vs MTL_v0.3. Real progress from 0W/20L/0D the run before, but
still firmly behind.

**Cadence:** run after each shipped feature that's expected to be ≥
+10 Elo. Watch the trajectory — when we cross +0 Elo, MTL_v0.3 stops
being the rung and MORA (2191) becomes the next target.

**Command:** `test_huginn_calibration.bat mtl03 50` — auto-rebuilds.

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

### #11: CLAUDE.md NPS figure is stale

- status: open
- priority: low
- type: maintenance
- est: 5 minutes

**State:** `CLAUDE.md` claims "~220k nodes/second" multiple times.
Actual measurement at depth 11 startpos shows ~2.0-2.3 Mnps — off by
~10×. This is a documentation-correctness issue, not a strength one.

**Plan:** find the lines in CLAUDE.md, replace with current measured
figure, note the bench command (`go depth 11` from startpos).

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
