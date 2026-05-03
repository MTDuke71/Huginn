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

## Open — high priority

### #1: Skip SEE-prune and LMR-reduce on check-giving moves

- status: open / unblocked
- priority: high
- type: feature
- est: half-session (~30-40 lines + tests)
- links: [SEARCH_AND_EVAL.md#priority-1](SEARCH_AND_EVAL.md), `src/minimal_search.cpp` qsearch + LMR blocks

**Evidence:** WAC.009 — huginn at depth 10 says −223 cp ("losing"),
MTL_v0.5 at depth 26 says +28991 ("forced mate"). Expected move is
`Bh2+`. Huginn's LMR is reducing exactly the move that would force the
mate. Pattern repeats across 11/24 of the regressions vs old-huginn
and roughly half of the 51 missed-vs-MTL positions.

**Plan:**
1. `gives_check()` helper via make/SqAttacked-on-enemy-king/unmake.
2. Qsearch SEE-prune: skip prune if `gives_check()`.
3. LMR block: skip reduction if `gives_check()` (or set R=0).
4. Re-run WAC test → expect 5-15 of the 24 regressions to flip to passes.
5. Gauntlet vs `huginn_t2` → expect +20-40 Elo.

**Why now:** unblocked (SEE qsearch shipped at `1cce8de`), highest
leverage of any open item, fast verification via WAC.

---

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
