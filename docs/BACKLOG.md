# BACKLOG

## Quick Index

| # | Title | Status | Type | Priority |
|---|-------|--------|------|----------|
| 1 | Skip SEE-prune and LMR-reduce on check-giving moves (P1a) | **CLOSED** @ `2dbd856` | feature | high |
| 2 | Re-attempt king safety on top of mobility | **DEFERRED** — likely needs tapered eval first (see #35); KS on an untapered base poisons the endgame | feature | low |
| 3 | Continuation history | **PARKED** — 1-ply additive falsified (w16 neutral, w64 −9); flag off, kept in-tree | feature | high |
| 4 | Refresh `huginn_t3` baseline | **CLOSED** @ `2e97066` | maintenance | medium |
| 5 | Recalibrate vs external opponent (MTLChess) | **OPEN** | maintenance | medium |
| 6 | Lazy SEE in main-search capture ordering | **WIP (parked)** — attempt 2 `f75a830` pooled +2.08 neutral, reverted `66bce5d`; branch `wip/see-capture-ordering` | feature | low |
| 7 | LMP (Late Move Pruning) fix | **DEFERRED** | feature | low |
| 8 | Aspiration step (b) — narrow-window search | **DEFERRED** | feature | low |
| 9 | Texel-style tuner | **OPEN** | research | low |
| 10 | Wire up Syzygy tablebase probe | **CLOSED** @ `5347e6d` | feature | low |
| 11 | CLAUDE.md NPS figure is stale | **CLOSED** @ `b9cc1be` | maintenance | low |
| 12 | Fastchess hang at 80 games | **OPEN** | bug | low |
| 13 | Investigate dormant counter-move + TT-mate bugs | **CLOSED** @ `2e97066` | bug | high |
| 14 | Move-gen — bypass legality pre-filter | **CLOSED** @ `b1154c8` | feature/perf | low |
| 15 | Re-attempt counter-move heuristic | **CLOSED** — soft ship @ `b9d63f8` (+7.1 Elo, LOS 91%, 2-machine) | feature | medium |
| 16 | Contempt — penalize draw scores | **CLOSED** | feature | medium |
| 17 | Aspiration-window widening on score swings | **OPEN** | feature | medium |
| 18 | Refresh `huginn_t4` baseline | **CLOSED** @ `6e3a761` | maintenance | medium |
| 19 | Two-machine gauntlet workflow + SPRT | **IN-PROGRESS** | tooling | medium |
| 20 | Trapped-bishop eval pattern | **DEFERRED** | feature | low |
| 21 | PV continues past threefold repetition | **CLOSED** @ `5efaa78` | bug | low |
| 22 | Gauntlet results archive policy | **ESTABLISHED** | tooling | medium |
| 23 | TT bound classification bug fix | **CLOSED** @ `7d11f23` | bug | high |
| 24 | Real magic-bitboard slider attacks | **CLOSED** @ `3eab266` | feature/speed | high |
| 25 | Refresh `huginn_t5` baseline | **CLOSED** @ `3eab266` | maintenance | medium |
| 26 | `board64[64]` piece-on-square cache | **DEFERRED** | feature/speed | medium |
| 27 | Unorthodox early-queen PV (d1d3 / d8d6) | **DEFERRED** | evaluation | low |
| 28 | PGN-driven repetition conversion analysis | **CLOSED** — P1 @ `a21a037`, P2 @ `304f2b7` (`baseline-t7`, +7.6 Elo pooled) | research/bug | high |
| 29 | Fifty-move-rule search blindness | **P1 KEPT on correctness** @ `534b44c` (near-inert at 10+0.1; pooled −5.73 = variance). P2 (clock-scaled eval) DEFERRED | bug | low |
| 30 | Nondeterministic search from uninitialized history | **FIXED** — `search_history` was uninitialized; zero-init. Search now deterministic at fixed depth | bug | high |
| 31 | TT-size (`Hash`) SPRT sweep — 64 vs 128 vs 256 MB | **OPEN** | tuning | low |
| 32 | PEXT slider attacks (build-gated, fallback to magic) | **OPEN** | speed/research | low |
| 34 | Pin/blocker-aware legal movegen (SF `blockersForKing`) | **OPEN** | speed/research | low |
| 35 | Tapered eval + eval-gap closure vs Fruit 2.1 | **IN-PROGRESS** | feature/eval | high |
| 36 | Illegal move in displayed PV (TT-walk collision) | **OPEN** | bug | low |

**Status Legend:**
- **CLOSED**: Completed and shipped (or documented closure reason)
- **DEFERRED**: Valid idea, blocked or re-attempted but regressed; preserved for future
- **OPEN**: Unstarted, unblocked, ready to pursue
- **IN-PROGRESS**: Active work
- **ESTABLISHED**: Policy/infrastructure set up and documented

Single-file issue tracker. Each session opens here first.

## Next 3 Actions

1. **#19 Part A (SPRT)** — add `-sprt elo0=0 elo1=10 alpha=0.05 beta=0.05`
  to the t5/t6 gauntlet bats so borderline results converge faster and
  clear wins/losses stop early.
2. **NEXT TARGET — eval/search Elo headroom (TBD).** #28 (repetition)
  and #29 (fifty-move) closed the draw-blindness bug class. #29 proved
  the 50-move rule is a non-factor at 10+0.1, so the next lever is real
  strength: candidates below to be scouted — king safety (#2, deferred),
  continuation history (#3, deferred), aspiration windows (#17), SEE in
  main search (#6, deferred). Pick by Elo/effort after a fresh look.
3. **#17 score-swing verification re-search** — add full-window
  verification when iteration-to-iteration score swing exceeds threshold
  to stabilize aspiration behavior in sharp endgame transitions.

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

## Deferred — recently deferred (unblocking per-feature via specific fixes, not via one universal lever)

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

## Backlog — high priority (open + deferred)

### #26: `board64[64]` piece-on-square cache — DEFERRED (negative gauntlet, no bug)

- status: **DEFERRED 2026-05-17** — reverted in this commit after the
  400g pool came in net-negative AND the invariant walker (`b8cd310`)
  ruled out the obvious desync bug. See "What we actually know" below.
- priority: was medium-high (speed win, low risk); now low — likely
  needs profiling evidence before retry
- ~~REOPENED 2026-05-17~~  (the cross-machine framing was a hypothesis,
  not a finding; the invariant test ruled it out — see below)
- ~~closed code-side (2026-05-17, shipped pending 200g gauntlet)~~
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
  Ptnml(0-2) [4, 23, 40, 28, 5]. Lean positive, DrawRatio 56.5%.
- 200g AMD gauntlet (2026-05-17, same conditions):
  **−38.37 ± 31.90 Elo, LOS 0.86%**, W38/L60/D102, score 44.50%,
  Ptnml(0-2) [10, 24, 46, 18, 2]. Decisive negative.

**Headline: the two machines disagree on the sign** (Intel +12.17 /
LOS 77% vs AMD −38.37 / LOS 0.86%). Naive pool ≈ −13 Elo, score
48.1%. The disagreement is wider than typical for byte-identical
binaries at 200g each, but **not impossibly so**: with a true Elo
near zero or slightly negative and per-sample SD ~20-25, two samples
landing 25 Elo above and below the mean is a low-but-not-vanishing
joint probability (~few percent). Whether this is "machine-dependent
bug" or "antipodal noise around small true Elo" can't be decided from
just these two samples — but see "What we actually know" below.

**What we actually know (post-investigation).**

Worried the cache was desyncing on a special-move path the bench
misses, we built `test/test_position_invariant.cpp` (committed at
`b8cd310`) that walked every legal move on five diverse FENs
(startpos, Kiwipete, promotion-rich, EP available, both-sides-castle)
to depths 3-4, asserting `board64[sq64]` matches the
bitboard-derived piece after **every** MakeMove and **every**
TakeMove. **All 5 cases PASSED.** The cache was correct across
promotion, en passant, castling, and the corresponding undo paths.

Additional checks:
- No `Position` copy / pass-by-value / `memcpy` paths exist in
  `src/` (verified by grep) — board64 zero-init via brace-init
  `std::array<Piece, 64>{}` is sufficient; there's no construction
  path that could leave it uninitialized.
- `MakeNullMove` / `TakeNullMove` don't touch pieces, so they
  correctly leave board64 alone.
- `set_from_fen` calls `reset()` (zeros board64) and
  `rebuild_counts()` (rebuilds board64 from the bitboards).

So the "uninitialized `board64`" hypothesis from the original
REOPENED framing is **not supported by evidence**. The cache really
was correct.

**Most parsimonious explanation now.** The +12% NPS bench gain is
real (instruction-count savings on `at_sq64`), but at TC=10+0.1 the
engine is bottlenecked by total work-per-node, not by this one
function. The +64 bytes that `board64` adds to `Position`'s
memory footprint costs as much in L1/L2 access overhead (across the
~80 `Position::at*` calls per node and the hot-path traversal of
the Position struct itself) as the array load saves vs the bitboard
scan. Net true Elo is likely near zero or slightly negative; the
pooled −13 is consistent with that. The Intel/AMD swing is then
just two samples on opposite sides of the small true value.

This **falsifies a default intuition**: "always cache a hot lookup"
is not free when the lookup is already cache-resident and cheap
(`at_sq64`'s 6-iteration loop is ~10 ANDs + branches from already-hot
cache lines, well under a nanosecond on modern CPUs). The
1-Elo-per-1%-NPS slope observed for #23 (+5% NPS → +24 pooled) and
#24 (+52% NPS → +78 pooled) does **not** generalize to caches that
displace surrounding hot data.

**Revert (this commit):**
- Removed `board64` field + all mutator updates from `position.hpp`.
- Restored bitboard-scan `at_sq64` (with a comment block recording
  the failed attempt + revert reason).
- Removed `board64.fill` from `reset()` and the rebuild loop from
  `rebuild_counts()`.
- Removed `test/test_position_invariant.cpp` from CMakeLists and
  from disk (it references `pos.board64` which no longer exists).
  The test design is documented in `b8cd310`'s commit message + this
  entry for future-self if any cache is re-attempted.

**Revisit triggers** (when to attempt again):
- A profiling pass (callgrind / VTune / Intel PCM) shows `at_sq64`
  consuming a measurable fraction (≥5%) of search time. Currently
  nothing suggests that.
- The `Position` struct gets a memory-layout reorganization that
  reduces footprint elsewhere — a cache could land in freed cache
  capacity at neutral total size.
- A smaller cache variant (e.g., 4-bit per square in 32 bytes
  instead of 8-bit in 64 bytes) might fit the same usage at half
  the memory cost and tip the trade-off.

**Methodology note.** This is the first case in this session where
"speed bench up" did **not** convert to "gauntlet Elo up." The
1-Elo-per-1%-NPS slope is a heuristic, not a law — works for hot
functions that are NOT already cache-resident, breaks down for ones
that are. Going forward, if bench shows < 15% NPS gain and the
function being optimized was already small/cheap, treat the
expected Elo gain as 0 ± noise rather than projecting linearly.

---

### #24: Real magic-bitboard slider attacks (CLOSED)

- status: **closed @ `3eab266`** (2026-05-16; shipped after cross-machine pool)
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

### #23: TT bound classification bug — fix (CLOSED)

- status: **closed @ `7d11f23`** (2026-05-15; shipped after pooled validation)
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

- status: **CLOSED — soft ship @ `b9d63f8`** (2026-05-31). Counter-move
  enabled @ score 1500. Kept on two-machine replication despite pooled
  LOS 91% (just under the 95% bar) — see closure note below.
- priority: was medium
- type: feature / research
- est: 1 session
- links: #13 closure (2c shipping commit), #3 (continuation history shares
  the search_stack plumbing)

**Closure (2026-05-31) — re-tested @1500 on baseline-t7, two machines:**

| Leg | Score | Elo | LOS | Ptnml(0-2) |
|---|---|---|---|---|
| Intel 13700KF | 51.05% | +7.30 ± 14.7 | 83.5% | [26,119,187,144,24] |
| AMD 7800X3D | 51.00% | +6.95 ± 14.5 | 82.7% | [21,123,201,125,30] |
| **Pooled 2000g** | 51.02% | **+7.12 [-3.2, +17.4]** | **91.24%** | [47,242,388,269,54] |

Pooled raw W479 / L438 / D1083 (tc=10+0.1, noob_3moves.epd, 64 MB hash).

**Why shipped despite LOS 91% (under the 95% bar):**
- **Two-machine agreement is tight** (+7.30 vs +6.95). Cross-machine
  reproducibility — the exact property whose *absence* sank #26 (Intel
  +12 / AMD −38 → reverted) — is present here. Both legs independently
  land on ~+7, so "true effect ≤ 0" is unlikely (that's why LOS is 91,
  not 50).
- **Replicates the t4 result** (+8.7 Elo, 2026-05-09) on an entirely
  different baseline (t5 TT-bound fix + magic bitboards + t6/t7 rep work
  all shipped between). A small effect surviving a baseline change is the
  signature of a real one.
- **Precedent**: P1a (#1) shipped at LOS 84% with corroborating WAC
  evidence. Two-machine replication at LOS 91% is stronger corroboration.
- The SPRT [elo0=0, elo1=10] structurally can't accept a ~+7 effect (it
  sits between the hypotheses; Intel LLR stalled at 0.47/2.94). The
  fixed-games pooled LOS is the meaningful readout, not the SPRT verdict.

**Settings shipped:** `ENABLE_PLY_TRACKED_COUNTERMOVE = 1`, counter-move
ordering score **1500** (just above the history range, below promotions).
Score 1500 was confirmed (again) superior to 15000 — the latter's −10 Elo
on t4 was not re-tested on t7. Baseline NOT refreshed (t7 stays current;
+7 < the +50 refresh trigger). WIP branch `experiment/counter-move-1500`
retired — the change is now on main.

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

**Clean 1-ply re-implementation (2026-06-01), on the `baseline-t9` tip.**
The #13/#30 ply-tracking bugs that sank the 2005-05 attempts are long
fixed, so this is a genuine fresh attempt. Gated behind
`ENABLE_CONTINUATION_HISTORY` (default on) for A/B. Design:
- 1-ply only (counter-move history): table keyed
  `[prevPiece][prevTo][curPiece][curTo]`, `int16_t`, heap `std::vector`
  (~1.38 MB). Updated with the same `+/-depth^2` bonus/penalty as
  butterfly history; aged `/4` per search.
- Blended **additively** into quiet-move ordering on top of the existing
  counter-move (1500) bonus — counter-move kept as-is for isolation.

Two findings worth keeping:
1. **Heap allocation is mandatory.** As a by-value Engine member the
   1.38 MB table overflowed the 1 MB Windows stack (a test constructs
   `Engine` by value) → `0xC00000FD`. Same footprint class that killed
   the `board64` cache (#26). Moved to a heap `std::vector<int16_t>`.
2. **A 1:1 blend is inert.** Huginn's butterfly `search_history`
   accumulates unbounded over *all* parent contexts, so each entry is
   ~100-1000x larger than a single-context conthist entry and swamps it
   — byte-identical search at weight 1. Verified live only after adding
   `CONTHIST_ORDER_WEIGHT` (start **16**) + a `CONTHIST_ORDER_CAP` (8000)
   to keep hot quiets inside the history band. At weight 16, depth-11
   Kiwipete: 363,757 nodes vs 364,401 off (~0.18% fewer), deterministic,
   same PV/eval. WAC-50 @5s: 48/50, no tactical regression.

**Primary tuning knob = `CONTHIST_ORDER_WEIGHT`.** If the first SPRT is
flat, sweep the weight before concluding conthist doesn't pay — the
additive-on-unbounded-butterfly scaling makes the signal weight-
sensitive. If still flat, the fallback design is bounded "history
gravity" updates for both tables so a 1:1 sum is principled (deferred —
it would modify the gauntlet-proven butterfly history, conflating two
changes). Gauntlet: this commit (flag on) vs `baseline-t9`, both
machines per the per-machine baseline rule.

**Weight 16 SPRT result (2026-06-02): NEUTRAL, not shippable.**
1000g/machine vs `baseline-t9`, 10+0.1:
- Intel: **-0.35 ± 14.72** Elo, LOS 48.2%, Ptnml [27,123,204,116,30],
  DrawRatio 40.8%, PairsRatio 0.97 — dead flat.
- AMD: **+6.6** Elo. Pooled ≈ +3 Elo (±~10), and the two machines
  disagree in sign → noise around zero, below the cross-machine ship bar.
The near-symmetric, draw-heavy pentanomial (WW30/LL27, WD116/LD123) says
the two builds played almost the same chess — consistent with the
pre-commit measurement that w16 changed only ~0.18% of nodes. **Most
likely too weak to alter move selection**, NOT evidence conthist is
useless. Caveat: the amplification probe showed even w1000 moved only
~0.7% of nodes, so the additive-on-unbounded-butterfly term is
structurally capped — a weight bump has limited headroom.
**Now testing `CONTHIST_ORDER_WEIGHT = 64`** (w64 Kiwipete d11: 363,561
nodes vs 363,757 @ w16 vs 364,401 off — engages marginally more). If w64
is also flat, that confirms the structural cap → escalate to the
bounded-gravity redesign or shelve.

**Weight 64 SPRT result (2026-06-02): NEGATIVE — hypothesis falsified.**
- Intel: **-9.04 ± 13.98** Elo, nElo -13.93, LOS 10.2%,
  Ptnml [26,130,204,124,16], LLR -1.57. AMD: worse still.
- vs w16 [27,123,204,116,30]: the **WW pairs collapsed 30 → 16** while LL
  held — raising the weight specifically destroyed the double-win
  outcomes. More conthist influence = worse play, on both machines.

**Conclusion: the 1-ply *additive* design is the wrong recipe for Huginn,
falsified — not "too weak."** The weight sweep (off≈0 → w16≈0 → w64 −9)
is monotone-down: when forced to influence ordering, conthist's noisy
1-ply guesses override BOTH butterfly AND the proven counter-move (its
8000 cap outranks the 1500 counter-move bonus, and at w64 it does so on
raw conthist ≥ 24 — frequently). The stale-guess risk the user flagged
shows up exactly here. **PARKED**: `ENABLE_CONTINUATION_HISTORY` set to 0
(flag-off byte-identical to baseline-t9, 364,401 nodes Kiwipete d11),
code kept in-tree.

**If revisited, do it RIGHT, not bigger:** (a) bounded "history gravity"
updates so butterfly + conthist are co-scaled in [-MAX,+MAX] and a 1:1
sum is principled; (b) **subsume** the counter-move into conthist rather
than stacking additively (they are the same idea — the current design
has them fighting); (c) consider multi-ply (1+2) only after 1-ply earns
its keep. This is a deliberate redesign on proven code, not a tuning
knob — schedule it as its own session, not a quick re-attempt.

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

## Backlog — medium priority (open + deferred)

### #31: TT-size (`Hash`) SPRT sweep — 64 vs 128 vs 256 MB

- status: **OPEN** (added 2026-05-30) — unblocked by `0326eae`
- priority: low
- type: tuning
- est: 1 session (3 SPRT legs, two-machine pooled per #19)

**Unblocked 2026-05-30 (`0326eae`).** The UCI `Hash` option was a no-op —
acknowledged but never wired, so the TT was effectively hardcoded at
64 MB regardless of what fastchess/GUIs sent. `0326eae` extracted
`TranspositionTable::resize_mb()` (`src/transposition_table.hpp`),
advertised `option name Hash type spin default 64 min 1 max 4096`, and
wired the setoption handler to resize + clamp. The knob is now testable.

**Why this was worth opening (perft surprise → cache discussion,
2026-05-30).** The perft tracking run had the Intel i7-13700KF beating
the AMD 7800X3D (~9.5 s vs ~11.3 s), which looked backwards vs the
"X3D is great for chess" lore. Resolution: perft is a compute-bound,
cache-resident movegen loop — it rewards raw clock/IPC (Intel's faster
P-cores win), not L3 capacity. The X3D's 96 MB V-cache only pays off
on the workload it was never tested on here: **engine search with a TT
large enough to be latency-bound**. At the current 64 MB the entire TT
fits inside the X3D's 96 MB L3 (best case for it) but spills to DRAM on
the 13700KF (~30 MB L3).

**Hypothesis to test:**
- At short TC (10+0.1, our SPRT standard) the TT barely fills 64 MB
  before the move is made, so 128/256 MB is likely **neutral-to-slightly-
  negative** (added cache-miss latency, no hit-rate gain). Same pattern
  as #26 — "bigger/cached isn't free when the data is already hot."
- **X3D-specific watch:** keep size ≤ ~96 MB to stay L3-resident on the
  AMD box. Pushing past 96 MB should hurt the AMD leg more than the
  Intel leg (Intel already spills) — if the two machines disagree by
  size, that's the cache boundary showing up, not noise.

**Plan:**
1. Build once; run three SPRT legs per machine via the #19 two-machine
   pooled workflow: `Hash=64` (control) vs `128`, vs `256`, each
   `elo0=0 elo1=10 alpha=0.05 beta=0.05`, current vs current with only
   the `setoption Hash` line differing.
2. Set `Hash` **before** `isready`/`position` — `resize_mb()` reallocs
   and discards entries, so mid-game resize would wipe the table.
3. If any size clears LOS ≥ 95% positive, bump the default in
   `Engine`'s ctor (`src/search.hpp:90`, currently `tt_table(64)`).
   Otherwise record neutral and keep 64 MB (and note the X3D-boundary
   finding for the lore file).

**Note:** lower priority than strength features — this is a tuning knob
with a likely-neutral prior at our TC, but it's cheap now that the
option works, and the cross-machine cache-boundary question is
independently interesting.

### #32: PEXT slider attacks (build-gated, fallback to magic)

- status: **OPEN** (future investigation, added 2026-05-31)
- priority: low
- type: speed / research
- est: ~1 session (impl + init-verifier reuse) + 1 gauntlet

**Idea.** Replace the magic-bitboard slider index (multiply + fixed
shift) with a BMI2 **PEXT** index (`_pext_u64(occupancy, mask)`) for
`rook_attacks` / `bishop_attacks`. PEXT extracts exactly the masked
occupancy bits into a dense index, so attack tables are **exactly
sized** — no over-allocation from fixed magic shifts. Smaller tables =
better L1/L2 residency, which is the cache-locality lever from #31 (and
the perft/X3D thread). Current code is pure magic
([src/magic_bitboards.cpp](src/magic_bitboards.cpp)); `git grep` confirms
no PEXT/PDEP anywhere in `src/`.

**Why it's viable NOW (the AMD-vs-Intel angle).** PEXT was historically
an Intel-only win: fast (~3c) on Intel, microcoded and brutally slow
(~18c) on AMD **pre-Zen3** — which is exactly why magic (vendor-neutral
multiply) was the right call when #24 landed. But **both current gauntlet
boxes have fast PEXT**: 7800X3D (Zen 4, ~3c) and 13700KF (Raptor Lake,
~3c). The historical reason to avoid it is gone on our hardware.

**Constraints / design:**
- **Build-gated.** Wrap the PEXT path in `#ifdef __BMI2__` (or a CMake
  option) with the existing magic path as the fallback, so it never
  regresses on an older/slow-PEXT AMD box. `/arch:AVX2` is already set
  (CMakeLists.txt:130), so BMI2 is available in the current build.
- Reuse the init-time **verifier** from #24 (walks every (square,
  occupancy-subset) vs a ray-walk reference) — it's table-source-agnostic
  and would validate the PEXT tables for free.
- Keep `bishop_attacks` / `rook_attacks` signatures unchanged; only the
  internal index computation swaps. Every caller stays untouched.

**Expectation (set realistically).** On fast-PEXT hardware PEXT bitboards
are typically **neutral-to-slightly-faster** than magic — the win is
mostly cache (smaller tables), not raw lookup. Could easily land in the
#26 bucket ("NPS up, Elo flat at our TC") since slider lookup is already
cache-resident and cheap. Treat expected Elo as ~0 ± noise; the real
question is the **bench/NPS + cache-miss profile** (VTune/uProf, per the
new BUILD_GUIDE profiling section), not gauntlet Elo. Worth doing as a
speed/architecture experiment + a confirmation that it doesn't regress,
not as a strength lever.

**Revisit trigger / sequencing.** Low priority vs strength work. Natural
to pair with #31 (both are cache experiments) and to measure with the
profiling tooling once that's set up.

### #33: WAC300 hard-position set — 30 deterministic-failure positions

- status: **OPEN** (added 2026-06-02)
- priority: medium (focused tactical improvement target)
- type: investigation / strength
- est: per-feature spot-check (~25 min/run), ongoing — close when
  WAC stabilises ≥95% or pause if 30 prove genuinely depth-limited

**Stable hard-position set.** With t9 + Tier 1-4 cleanup + conthist
OFF (`#define ENABLE_CONTINUATION_HISTORY 0`), WAC300 scores
**270/300 = 90.0%** at 5s/position, and **the same exact 30 positions
fail on both gauntlet boxes** — verified by separate runs on the AMD
7800X3D and Intel 13700KF at the same time control. Same total, same
identical failure set, including matching engine moves where the
engine picked the wrong best-line. This is the strongest possible
cross-machine determinism check at the tactical level: the search
tree at fixed 5s is bit-identical in observable output across both
machines. **The 30 are genuinely hard, not per-machine luck.**

**The 30:** WAC.002, 029, 055, 071, 080, 081, 090, 091, 100, 131,
141, 145, 163, 196, 213, 222, 230, 237, 241, 243, 247, 248, 256,
265, 277, 283, 287, 291, 293, 297.

Logs: `test/wac_test_log_20260602_102823.txt` (full),
`test/wac_failed_positions_20260602_104225.txt` (just the 30 +
positional FEN + expected SAN + got-move).

**Failure-category breakdown** (informal, for targeted work):

- **Knight tactics (6):** 071 `Nxa7+`, 090 `Nxg7`, 196 `Nb4`,
  256 `Nf5`, 283 `Ng5`, 293 `Nfg5`. Long-standing pattern flagged in
  `SEARCH_AND_EVAL.md` ("5 of 11 hard positions are knight tactics"
  from the older WAC runs). Knight forks resist deep search because
  LMR steals the ply where the fork resolves into material gain.
- **Multi-ply forcing checks (6):** 055 `Qxg7+`, 163 `Qg2+`,
  213 `Rxh7+`, 237 `Rc1+`, 241 `Qxh7+`, 248 `Qc5+`. Sacrificial check
  sequences where the immediate sac doesn't obviously recover material
  — need check-extension depth on the *follow-up*, not just the
  candidate move.
- **Quiet / positional (4):** 080 `Ra1`, 230 `Rb4`, 243 `Qe2`,
  291 `h3`. Engine prefers a tactical alternative; missing eval term
  or insufficient depth for the positional payoff to register in
  alpha-beta.
- **Multi-option choose-your-route (4):** 100 (`Be3, b6+`),
  277 (`Rg3, Rxg2`), 287 (`Qg4, Qh5`), 297 (`Bxg2, Bxh2+`). Solutions
  list 2+ acceptable best moves; engine picks neither — exactly the
  case singular-move detection / extension would catch ("no other
  move is competitive at reduced depth").
- **Single-best tactic, miscellaneous (10):** 002 `Rxb2`, 029 `c6`,
  081 `Bd6`, 091 `Be6`, 131 `Re8`, 141 `Qxf4`, 145 `Re8`,
  222 `Bf6`, 247 `Rxb5`, 265 `exf6`.

**How to use this as a focused-improvement signal:**

1. Run `cd test && python wac_test.py 300 -t 5` (~25 min on AMD) on
   the candidate build. Compare the failed-position set to the 30
   above.
2. A change that **flips 3-5 of the 30** is real signal at the
   tactical-resolution level. Cross-machine determinism gives this
   teeth: the 30 are stable, so any flip is the change solving
   something genuinely harder, not noise.
3. Much cheaper than a 1000g SPRT (~25 min vs ~80 min) and **orthogonal
   to gauntlet Elo**: WAC measures tactical resolution at depth,
   SPRT measures move quality at tournament TC. An eval change can
   flip WAC failures without moving Elo at 10+0.1, and vice versa.
4. **Cross-machine sanity is automatic.** Both gauntlet boxes run
   identical search at fixed time, so a WAC-affecting feature should
   show the same flips on both. AMD/Intel divergence on which of the
   30 newly solve/fail = #26-class machine-dependent behaviour to
   investigate.

**Candidate features expected to flip some of the 30** (hypothesis
seeds, not commitments):

- **King-zone knight-attack term** (#2 king safety revisit). Would
  weight positions where a knight reaches an attack square near the
  enemy king. Plausible flips: the 6 knight tactics, plus a few of
  the forcing checks where the sac sequence ends in knight-fork.
- **Check extension on the candidate move's *follow-up*,** not just
  the candidate itself. Today's check extension may stop too early
  in the multi-ply Qxg7+ / Rxh7+ / Qxh7+ continuations.
- **Recapture extension at low depth.** Would deepen the sacrificial
  check positions by one more ply at the cost the search currently
  refuses.
- **Singular extension** (Tier 3 work). The 4 multi-option positions
  are exactly the case singular catches — "no other move is
  competitive at reduced depth, extend the candidate."
- **History-aware LMR** that doesn't reduce moves leading to mate
  threats in the recent search. The forcing-check failures are
  exactly where LMR's depth cut bites.

**Not a single deliverable.** Tracking item for the hard set itself
plus a workflow note. Close when post-improvement WAC stabilises
above ~95% (matches the MTLChess_v0.5 reference), or pause if the
30 prove genuinely depth-limited and the engine's better marginal
ROI is search shape (deeper trees, better ordering) rather than
tactical surgery.

#### vs Stockfish 18 — refined target (added 2026-06-02)

Cross-engine comparison: same 300 positions, same 5 s/pos, AMD
7800X3D. Stockfish 18 (AVX-512 build, 16 threads, NNUE) at
~20–40 Mnps. Huginn (1 thread, post-t9 + Tier 1-4 cleanup, conthist
OFF) at ~3.5 Mnps.

Logs: `test/wac_test_log_20260602_120720.txt` (SF18 detailed),
`test/wac_failed_positions_20260602_*.txt` (the failed-position
companion files for each engine).

**Note on harness:** running SF required fixing `test/wac_test.py` —
the old batched-`subprocess.run` driver let engines that poll stdin
mid-search (SF) see the buffered `quit` / stdin-EOF and abort
without searching, returning a depth-0 instant move (commit
`c9ce270`). Huginn happened to mask the bug because it only reads
stdin between checkup intervals. After the fix, both engines drive
correctly via interactive Popen UCI; `--threads N` was added so SF
can use all 16 cores on the gauntlet boxes for the comparison.

**Headline numbers:**

| Engine | Solved | Rate |
|---|---:|---:|
| Huginn current (1 thread) | 270/300 | 90.0% |
| Stockfish 18 (16 threads, NNUE) | 280/300 | 93.3% |

SF18's 93.3% is *lower than expected* (would have predicted ~97-99%
given Stockfish's strength). The reason is mostly the WAC test
itself, not SF: many WAC "expected best moves" are dated and
narrowly specified — SF often finds equally-winning alternatives the
database doesn't list. The headline numbers are less useful than the
**intersection analysis below**.

**Intersection (the more useful data):**

| Bucket | Count | Practical meaning |
|---|---:|---|
| Both solve | 259 | Easy bulk — no engineering target |
| **Both fail** | **9** | Genuinely depth-bound — SF18 at 100 M nodes can't find these in 5 s either |
| **SF-only fail** | **11** | Almost certainly "WAC-expected-too-strict" — SF picks an unlisted but still-winning move. Huginn "wins" these by happening to pick the WAC-listed move at lower depth. *Not* a strength signal. |
| **Huginn-only fail** | **21** | The actual engineering target. SF solves these at 5 s, so they're reachable with stronger search; the gap is feature/ordering work. |

**The 9 both-fail positions** (genuinely horizon-bound, no eval term
will flip these — needs deeper search or singular-extension-style
horizon expansion):

| WAC | Expected | Huginn played | SF played | Note |
|---|---|---|---|---|
| 002 | `Rxb2` | `Rb6` | `Rb7` | Canonical horizon case — both engines pick "build b-file pressure" over the sacrifice; SF reports `score cp 497` for `Rb7` at depth 20 |
| 131 | `Re8` | `Rf6` | `Rf6` | single-best tactic, both miss |
| 145 | `Re8` | `Rf3` | `Rf3` | single-best tactic |
| 230 | `Rb4` | `Rh7` | `Rb5` | quiet rook lift; SF lands closer (b-file but wrong rank) |
| 241 | `Qxh7+` | `Qf6` | `Qf6` | multi-ply forcing check — even SF doesn't extend deep enough |
| 247 | `Rxb5` | `Rg5` | `Rg5` | sacrifice both miss the same way |
| 248 | `Qc5+` | `Qd5` | `Qb6` | forcing check |
| 291 | `h3` | `c7` | `c3` | quiet positional move |
| 293 | `Nfg5` | `Be3` | `Nb5` | **knight tactic SF18 also misses at 16 threads** — striking; pure depth-bound |

**The 21 Huginn-only failures** (SF solves these; flipping them
requires closing some of the SF gap — feature/ordering work):

`029, 055, 071, 080, 081, 090, 091, 100, 141, 163, 196, 213, 222,
237, 243, 256, 265, 277, 283, 287, 297`

Of these, 5 are knight tactics SF solves (`071 Nxa7+`, `090 Nxg7`,
`196 Nb4`, `256 Nf5`, `283 Ng5`). Huginn's 6 knight-tactic failures
split as 5 in this bucket + 1 (WAC.293 `Nfg5`) in the depth-bound
both-fail bucket — meaning **5/6 of Huginn's knight failures are
reachable**, not depth-bound. King-safety / knight-attack eval work
plausibly flips this subset.

**Revised candidate-feature ranking** (replaces the original
hypothesis-seeds list above; this is evidence-based once we have the
SF intersection):

| Candidate | Likely impact | Why |
|---|---|---|
| **Singular extension** (Tier 3 work) | **Highest ROI** | Directly addresses the "one move uniquely deserves depth" pattern. The both-fail set (9) includes the canonical case (WAC.002 `Rxb2`) where shallow eval prefers a positional alternative; singular extension is the textbook fix. Also targets multi-option positions (100, 277, 287, 297) in the 21-set. |
| **History-aware LMR / improved move ordering** (continuation history done right; conthist re-attempt with a sane weight curve) | High | The 21-set is reachable at 5 s with stronger search — these are mostly cases where the right move is *findable* but Huginn doesn't reach it in time. Better ordering = effective deeper search at the same NPS. |
| **King-zone knight-attack eval term** (#2 king safety revisit) | Medium | Five of the 6 knight-tactic failures are in the reachable (Huginn-only) set, suggesting eval support would help here. Lower priority than singular ext / ordering because SF's NNUE handles these implicitly; a hand-crafted eval term may not capture the right pattern. |
| **Check extension on candidate's *follow-up*** (not just the candidate) | Medium | Several forcing checks in the 21-set (055, 213) — extension may close. Less useful for the both-fail forcing checks (241, 248) since SF also fails them with massive extension budget. |
| **Recapture extension at low depth** | Low | Speculative — would deepen sacrifice sequences but SF's failure on 247 (`Rxb5`) suggests this class needs more than +1 ply. |
| **PEXT / TT-size sweep (#32, #31)** | Marginal here | Pure NPS levers help both buckets slowly. Won't move the needle on the depth-bound 9. |

**Updated closure conditions:**
- **Tier-1 target**: flip 10 of the 21 Huginn-only failures via
  singular extension and/or better move ordering. That would bring
  Huginn to 280/300 = 93.3% — exact parity with SF's total and with SF
  on the *non-depth-bound* subset (291 positions, both at 280) — much
  more honest than "stabilise above 95%". (Each flip = +0.33pp; ~6 flips
  reaches ~92%.)
- **Tier-2 stretch**: flip 1-2 of the 9 both-fail via raw depth wins
  (PEXT, TT scaling, ordering compounding). Each flip there is a
  meaningful "we now see something SF18 doesn't at 5 s" claim.
- **Not a goal**: closing the 11 SF-only failures. Those reflect WAC
  test-database narrowness, not engine strength.

### #34: Pin/blocker-aware legal movegen (Stockfish `blockersForKing`)

- status: **OPEN** (idea parked 2026-06-03)
- priority: low (speed/architecture experiment, not a strength lever;
  much bigger eval/search issues come first)
- type: speed / research
- est: ~1 session for the perft-side fast path + perft validation;
  the search-side variant is larger and separate

**Idea.** Adopt Stockfish's `st->blockersForKing[c]` / `st->pinners[~c]`
precomputation: once per node, derive the set of own pieces pinned to
the king (king-blockers) and the enemy sliders pinning them. Legality
of a move then becomes an O(1) test — a non-king, non-EP move by a
non-pinned piece is always legal; a pinned piece is legal iff its
destination stays on the king↔pinner ray — replacing per-move king-
attack scans.

**Where Huginn stands today.** Movegen is pure pseudo-legal
([movegen_bb.cpp:25](../src/movegen_bb.cpp#L25)); it has no pin
awareness. Legality is decided by make/unmake + `SqAttackedBB(king)`
([position.cpp:404](../src/position.cpp#L404)). Two consumers:
1. **`generate_legal_moves`** ([movegen.cpp:25](../src/movegen.cpp#L25))
   — make/unmake purely to test legality, then undo. Used by perft and
   the mate/stalemate `legal_moves` count at
   [search.cpp:1795](../src/search.cpp#L1795).
2. **Search** — already pseudo-legal: `generate_all_moves` + inline
   `if (pos.MakeMove(m) != 1) continue;` ([search.cpp:1388](../src/search.cpp#L1388)).
   This path's pre-filter was *already removed* in **#14** (closed
   `b1154c8`), so the legality test now piggybacks on a `MakeMove`
   that has to happen anyway for recursion.

**So the remaining win is narrower than it first looks:**
- **Perft / `generate_legal_moves` (the real target).** Here make/unmake
  is pure overhead — blockers let ~90%+ of moves skip it entirely.
  Memory/diagnosis says **perft is movegen-bound**, so this is where the
  gain lands.
- **Search.** #14 already captured the easy win. Blockers would only
  save (a) *making* the handful of illegal moves per node, and (b) the
  `SqAttackedBB` inside `MakeMove` — but (b) requires changing
  `MakeMove`'s contract to a "trusted-legal" make that skips the in-check
  test. Invasive, modest payoff, and at 10+0.1 the engine is work-per-
  node bound (cf. #26) — likely a #26-class "NPS up, Elo flat" result.

**Correctness traps (why this is not a quick edit):**
1. **En passant** — the captured pawn isn't on the from→to ray, so the
   pin test can't classify it. Always route EP through full make/unmake.
2. **In check** — blockers don't resolve evasions; the fast path is
   invalid when `pos.in_check()`. Huginn has no `generate<EVASIONS>`, so
   in-check nodes fall back to per-move checks.

**Recommended low-risk shape (if revisited):** don't rewrite movegen
into fully-legal generation. Add `compute_blockers(pos, us)` →
pinned-bitboard + pin rays, then bolt a fast-path filter onto
`generate_legal_moves`: in-check / EP / king moves fall back to the
existing `MakeMove` oracle; non-pinned pieces are accepted with no make;
pinned pieces are accepted iff `to` stays on the from–king line. Keeps
`MakeMove` as the correctness fallback, and the perft suite catches any
pin/EP bug instantly. Only escalate to a trusted-legal `MakeMove` for
the search path if the perft-side win proves worth the larger change.

**Expectation.** Real perft speedup; search Elo ~0 ± noise at our TC.
Worth it mainly as a perft/movegen-architecture lever, sequenced behind
the eval/search strength work (#33 tactical set, #17 aspiration, king
safety) that has far higher strength ROI.

### #35: Tapered eval + eval-gap closure vs Fruit 2.1

- status: **IN-PROGRESS** (opened 2026-06-03)
- priority: **high** — this is the largest identified strength lever; the
  eval is where Huginn's move-quality gap lives
- type: feature / eval
- est: tapered-eval foundation ~1 session + gauntlet; full table is
  multi-session

**Trigger: Fruit 2.1 gauntlet wipeout.** Dropped `fruit_fast.exe` into the
fastchess folder for a relative-strength read (10+0.1, noob_3moves,
OwnBook off):
- huginn_t9 vs Fruit, 40g: **0 W / 40 L / 0 D (0.0%)**, every game by mate.
- huginn_current vs Fruit, 20g: **0 W / 20 L / 0 D (0.0%)**, same.

Fruit 2.1 ≈ 2780 CCRL vs Huginn ≈ 1600 — a ~1000-1200 Elo gap, so 0% is
expected and yields no measurement gradient (useless as a calibration
sparring partner; the MTLChess ~1984 ladder is the right yardstick). PGNs:
`gauntlet/huginn_vs_fruit.pgn` (40g), `gauntlet/huginn_vs_fruit_10r.pgn`.

**The diagnostic signal (not the score itself).** In the PGN traces
**Huginn searched DEEPER (d10-15) than Fruit (d8-12) and still got mated
every game.** The gap is **move quality per node = evaluation**, not
speed or depth. This is consistent with CLAUDE.md ("the remaining gap is
search-tree shape and eval quality, not raw speed") and the post-#24
speed-parity finding. Search-shape experiments (#3 conthist, #6 lazy SEE)
have washed out near-neutral; eval is the under-invested axis.

**Eval feature gap — Huginn vs Fruit 2.1** (Huginn claims verified against
the code this session; ✅/❌ = present/absent):

| Feature | Huginn | Fruit 2.1 | Impact |
|---|---|---|---|
| Tapered eval (op→eg blend) | ❌ hard boolean `is_endgame` flips only king-PST + mobility weight ([search.cpp:127](../src/search.cpp#L127),[:148](../src/search.cpp#L148),[:274](../src/search.cpp#L274)) | ✅ every term has op/eg, smooth phase 0–256 | **Huge** |
| Tapered material values | ❌ single `PIECE_VALUES_MG` ([chess_types.hpp:166](../src/chess_types.hpp#L166)) | ✅ Pawn 80→90, etc. | High |
| King safety | ❌ **dead code** — `KING_SHIELD_MULTIPLIER`/`KING_ATTACK_PENALTY`/`CASTLE_BONUS`/`STUCK_PENALTY` defined ([evaluation.hpp:126-130](../src/evaluation.hpp#L126-L130)) but referenced nowhere; KS = king-PST only | ✅ shelter + storm + multi-attacker non-linear | **Huge** |
| Drawishness scaling | ❌ none | ✅ `mul[]` 0–16: opp-bishops ½, KNNK→0, "1 minor up no pawns"→⅛ | High |
| Endgame recognizers | ❌ only KK/KNK/KBK draw ([search.cpp:322](../src/search.cpp#L322)) | ✅ KPK, KRKP, KQKP, KBPKB, wrong rook-pawn… | Med |
| Passed pawns | basic rank bonus ([search.cpp:185](../src/search.cpp#L185)) | ✅ + unstoppable/free/king-distance | Med |
| Pawn structure | isolated, doubled, passed | + backward, candidate passers | Med |
| Mobility | flat square count ([search.cpp:273](../src/search.cpp#L273)) | per-piece tuned op/eg weights | Low-Med |
| Rook on open file | ✅ | ✅ | — |
| Rook on 7th / king-file | ❌ | ✅ | Low-Med |
| Bishop pair | ✅ +50 | ✅ +50 | — |
| Trapped/blocked pieces | ❌ | ✅ | Low |
| Pawn/material hash tables | ❌ | ✅ | perf only |

**Sequencing — tapered eval FIRST, and why it's the unblocker:**

1. **Tapered eval (phase 0–256 smooth blend).** Foundational — it's a
   multiplier on every other term. Low-risk: clean correctness test
   (phase=256 ≡ today's MG eval, phase=0 ≡ EG eval; mirror-symmetry must
   still hold). Partial infra already exists (`KING_TABLE_ENDGAME` + the
   `is_endgame` phase concept), so this is an *extension*, not a rebuild.
   **Key insight: this likely explains why #2 king safety regressed −126
   Elo.** A KS penalty on an untapered eval applies at full strength in
   the endgame, where the king should be *active* and KS should fade to
   ~0 — so KS poisons EG play. Fruit's whole eval is phase-blended exactly
   so KS can vanish in the EG. Tapering is the prerequisite that revives
   #2.
2. **Tapered material values.** Trivial once #1's blend machinery exists —
   give `PIECE_VALUES` an EG column.
3. **King safety (Fruit's shelter + storm + multi-attacker model),** now on
   a tapered base — NOT the presence-zone that failed at −126 (#2).
4. **Drawishness scaling (`mul[]`).** Independent, High-impact, cheap.

**Why this should convert (unlike the recent neutral experiments).** The
washed-out experiments (#3, #6, #26) were search-shape / speed levers that
TC-bound Elo barely reflected. Eval-quality changes of this magnitude
(missing KS, no tapering) are first-order move-selection changes — they
should show real Elo vs t9 at 10+0.1. Measure each step vs `baseline-t9`,
two machines, like every other ship. Don't refactor the whole table at
once: land tapered eval, gauntlet it, then layer terms one at a time so
each Elo delta is attributable.

**Experiment 1 — tapered-eval foundation (`ENABLE_TAPERED_EVAL`, 2026-06-03).**
Replaced the hard `is_endgame` boolean (material ≤ 1150 flips king-PST + mobility
weight) with a smooth `game_phase_256()` blend (`search.cpp`: `mg_pst`/`eg_pst`
sums diverge only on the king table; combine `(mg·phase+eg·(256−phase))/256`).
**No new tuned values** — material stays MG; only the king-PST + mobility
*transitions* are smoothed. Flag-off path is byte-identical to t9. 197/197 tests
pass incl. all 8 eval-symmetry cases.
- AMD 40g eyeball vs t9: +79.53 ± 95.81, LOS 95.85% (W17/L8/D15).
- **AMD SPRT [elo0=0,elo1=10] vs t9: H1 ACCEPTED @ 602g — +45.86 ± 19.43 Elo,
  LOS 100%, LLR 2.96**, W192/L113/D297 (56.56%), Ptnml [9,55,116,90,31].
  Branch `experiment/tapered-eval` @ `476d33c`, pushed. The 40g +80 regressed
  to mean; true effect ~+46 Elo — decisive, comparable to the #24 magic ship,
  from a foundation change with **zero new tuned values**. PGN
  `gauntlet/huginn_vs_t9_amd.pgn`.
- The win is **removing the eval discontinuity** (a capture crossing the 1150
  threshold instantly swapped king table + mobility weight = double-digit-cp
  jump → search instability) plus earlier graduated king-activation.
- **Intel SPRT vs t9: H1 ACCEPTED @ 846g — +35.03 ± 16.91 Elo, LOS 100%, LLR
  2.97**, W273/L188/D385 (55.02%), Ptnml [26,64,172,121,40]. Commit `33f773b`,
  PGN `gauntlet/huginn_vs_t9_intel.pgn`.
- **POOLED two-machine (1448g): W465/L301/D682, 55.66% ≈ +39.5 Elo.** Both
  machines independently H1-accepted at LOS 100% — the cross-machine agreement
  that #26 lacked. **DECISIVE SHIP.** Foundation = commit `476d33c`; freeze as
  **baseline-t10** (binary snapshot `huginn_t10c.exe` → `huginn_t10.exe`).
- Next: Experiment 2 = tapered material values (`PIECE_VALUES_EG`, already
  drafted) measured vs t10, then Exp 3 = king safety (Fruit
  shelter+storm+attacker) on the tapered base.

**Experiment 2 — tapered material (`ENABLE_TAPERED_MATERIAL`, DRAFTED 2026-06-03).**
Added `PIECE_VALUES_EG` (P120/N315/B340/R530/Q940 vs MG P100/N320/B330/R500/Q900 —
standard directions, conservative first-cut, Texel-tune later #9). The eg
accumulator uses EG material; mg keeps MG; blended by the #35 phase machinery.
Flag-off → eg uses MG → byte-identical to the foundation. Built, 197/197 tests
pass, symmetry holds. Static-eval validation: inert at full material (startpos
diff 0), engages in reduced material (KP −2P: Δ−40 = 2×20 exact; R+P: Δ−17;
R+2P: Δ+64) — magnitudes match the blend math. **Uncommitted draft** on top of
`476d33c`; NOT pushed (keeps the Intel foundation SPRT clean). Snapshot
`huginn_t10c.exe` = baseline-t10 binary, the isolation opponent. Foundation is
now confirmed + frozen as baseline-t10, so Experiment 2 is **ready to gauntlet
vs t10** (`test_huginn_vs_t10.bat`).

**WAC tactical corroboration (exp1+exp2 build, 5s/pos, AMD).** Independent of
gauntlet Elo, the eval work improves tactical resolution:
- WAC300: **274/300 (91.3%)**, up from baseline-t9's 270/300 (#33). Net +4: 7 of
  the old hard-30 flipped solved (071/080/081/091/145/248/277), 3 new misses
  (180/226/242).
- WAC201 (curated sound subset, new `test/wac201_test.py` + `wac201.epd`):
  **185/201 (92.0%)**. The reduced set drops the busted/dual/trivial WAC-300
  positions for a more honest tactical signal going forward. (Note: the file
  has 201 records — `wc -l` undercounts because WAC.300 has no trailing
  newline; fixed the suite default/cap from 200→201 in `943faf1`.)

**Experiment 2 status (2026-06-03): eyeball POSITIVE, two-machine SPRT running.**
Committed `3b498f9` (source), pushed. Opponent = `huginn_t10.exe` (baseline-t10).
- **AMD 40g eyeball vs t10: +52.51 ± 83.75 Elo, LOS 89.86%**, W12/L6/D22 (57.5%),
  Ptnml [1,3,8,5,3]. Not the small lever feared — a second real-looking eval
  gain. PGN `gauntlet/huginn_exp2_vs_t10_amd.pgn`.
- **AMD SPRT vs t10: INCONCLUSIVE @ 1000g (full cap) — +7.99 ± 15.46 Elo,
  LOS 84.48%, LLR 0.51** (never crossed ±2.94), W259/L236/D505 (51.15%),
  Ptnml [30,126,165,149,30]. PGN `gauntlet/huginn_vs_t10_amd.pgn`.
- **The 40g eyeball (+52) was noise** (±84 CI). Real effect ~+8 Elo — marginal,
  not a clear ship. Likely because the foundation already tapers the king PST
  (dominant phase term), so material tapering adds little on top, and the EG
  magnitudes were a conservative first cut. **Lesson: skip the 40g eyeball for
  small eval terms; it's uninformative — go straight to SPRT.**
- **Intel SPRT vs t10: −2.43 ± 15.01 Elo** (1000g, W250/L257/D493, 49.65%).
  Commit `f3838c2`, PGN `gauntlet/huginn_vs_t10_intel.pgn`.
- **POOLED 2000g: W509/L493/D998, 50.40% ~+2.8 Elo — MACHINES DISAGREE IN SIGN**
  (AMD +7.99 / Intel −2.43). Same #26 fingerprint (board64: Intel +12 / AMD −38
  → reverted); the *opposite* of a real ship (#15 counter-move agreed
  +7.3/+6.95). **Verdict: NOISE around zero, NOT shippable with these values.**
- **DECISION: PARK tapered material** (`ENABLE_TAPERED_MATERIAL 0`, keep code
  in-tree like conthist #3). The foundation already taperes the dominant phase
  term (king PST), leaving little for material tapering to add; blind EG-value
  hand-tuning isn't worth the gauntlet hours without a Texel tuner (#9). t10
  stays the clean confirmed baseline. Revisit when #9 lands (joint Texel tune
  of material+PST EG values is the principled way to extract this).
- **Next: Experiment 3 = king safety** (Fruit shelter+storm+multi-attacker),
  measured vs t10 — the "Huge" gap-table item, currently dead code, now on a
  tapered base so it fades correctly in the endgame (the #2 failure mode).

### #36: Illegal move in displayed PV (TT-walk collision) — cosmetic

- status: **OPEN** (logged 2026-06-03)
- priority: low — cosmetic (display only; `bestmove` is always legal)
- type: bug

**Symptom.** fastchess occasionally logs `Warning; Illegal PV move - move <m>
from Huginn_tXX` (~1 per few hundred games at 10+0.1). Seen on baseline-t10
during the #35 Exp 2 SPRT. **More frequent on the Intel box than AMD.**

**Mechanism.** The displayed PV = triangular-PV prefix (search-truth, length
≤ search depth) **+ a TT-walk extension** that lengthens it past the searched
depth ([search.cpp:2088](../src/search.cpp#L2088), commit `72b19f5`).
Signature: a PV longer than `depth` — e.g. `depth 3 ... pv g3h4 e2e1 d4d3 e1d2
h5h4 d2d3` (6 moves; the illegal `h5h4` is the 5th = in the TT-walk tail). The
walk probes each reached position's TT entry and appends its `best_move`; on a
**hash collision** that entry belongs to a *different* position, so the move is
bogus for the walked line. The guard ([search.cpp:2099-2105](../src/search.cpp#L2099-L2105))
only checks the from-square holds a side-to-move piece, then trusts `MakeMove`
— which assumes pseudo-legal input and only verifies king safety, so a collided
move on a correctly-coloured from-square slips through. **The PV walk is the
only place a raw TT move is *applied* without being cross-checked against the
generated move list** (the main search only uses the TT move for *ordering*
within the legal list, and its per-move `MakeMove()!=1` guard filters illegals
— so this is PV-display-specific, not a search-correctness bug).

**Why more on Intel (hypothesis).** Intel's higher NPS reaches more nodes /
greater depth at the same TC → more TT traffic and a fuller table → more
collisions and more TT-walk firing. Confirm by correlating warning rate with
avg depth/NPS (not yet measured).

**Severity: cosmetic.** Only the displayed PV string is wrong. `bestmove` =
PV[0] is always legal and is what's played; the board is fully restored by the
`TakeMove` loop ([search.cpp:2111](../src/search.cpp#L2111)); search behaviour
and Elo are unaffected; both engines can emit it so the SPRT is not biased.

**Fix (after #35 part 2; ~5 lines).** In the PV walk, before appending a TT
move, validate it against the position's generated legal moves (generate moves,
confirm `tt_move` is present) instead of the weak from-square-colour check.
Pure display change (no search/Elo impact), but it rebuilds the binary — don't
do it mid-SPRT; batch with the next baseline build.

### #19: Two-machine gauntlet workflow + SPRT

- status: in progress (2026-05-28) — Part A landed on t6 scripts; two-machine pooling active
- priority: medium
- type: tooling / infrastructure
- est: ~1 hour (Part A) + ~30-45 min one-time (Part B)

**Baseline policy update (2026-05-28):** `baseline-t6` is now the
active regression baseline. Active gauntlets should use
`test_huginn_vs_t6*.bat` only. `test_huginn_vs_t5*.bat` is legacy and
kept for historical/regression archaeology, not routine tuning.

**Progress update (2026-05-28) — first t6 SPRT pooled result (1000+1000):**
- Part A completed on active scripts: `test_huginn_vs_t6.bat` and
  `test_huginn_vs_t6_amd.bat` now run with
  `-sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -rounds 500 -repeat`.
- Intel leg (1000g): W239 / L244 / D517, score 49.75%,
  Elo -1.74 ± 14.53, LOS 40.73%.
- AMD leg (1000g): W247 / L240 / D513, score 50.35%.
- Pooled (2000g): W486 / L484 / D1030, score 50.05%.

Interpretation: dead-even against t6 (as expected for current vs
baseline sanity runs). The new workflow is validated and ready for
feature tests (notably #28 Part 2).

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

## Backlog — low priority (open + deferred)

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

- status: **WIP (parked, low priority)** — attempt 2 on t7 was
  Elo-neutral and reverted (2026-05-30); attempt 1 on t4 was −15.65.
- priority: low
- type: feature
- est: half-session
- links: [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) Tier 2 #10 partial
- WIP branches: `wip/see-capture-ordering` (attempt 2, commit `f75a830`,
  revive via `git cherry-pick f75a830`); `experiment/lazy-see-main`
  (attempt 1, commit `59b0fad`)

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

**Attempt 2 (2026-05-30, on t7 = baseline-t6 + #28 repetition stack):
Elo-neutral, reverted.** Re-implemented the same SEE bucket split in
`pick_next_move` (commit `f75a830`), but picker-safe placement: losing
captures (`SEE < 0`, promotions/EP exempt) drop the 1M base to bare
MVV-LVA, sinking *below killers* rather than below quiets (the
`pick_next_move` selection-sort floors at `best_score = -1`, so negative
scores can't be ordered — the t4 attempt's "below quiets" goal isn't
reachable without also fixing the picker).
- A 200g Intel smoke test read **+33 Elo** — pure small-sample variance.
- Pooled 2000g vs t7 (with #29+#30 in the stack): **+2.08 Elo, both
  machines exactly +2.08 / 50.30%**, pentanomial t = +0.40, LOS ~65% —
  statistically neutral. SEE ordering shows no measurable gain on the
  stronger t7 base (vs −15.65 on t4: the better the surrounding ordering,
  the closer to neutral) while adding a per-capture `see()` cost.
- Reverted (`66bce5d`); parked as low-priority WIP on
  `wip/see-capture-ordering`. Lesson: node-count-at-fixed-depth was
  useless as a pre-screen here because the engine was nondeterministic
  at the time (see #30, fixed after).

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

### #28: PGN-driven repetition conversion analysis

- status: **CLOSED — P1 @ `a21a037` (2026-05-18), P2 @ `304f2b7`
  (2026-05-29, shipped as `baseline-t7`).** Part 1 (halfmove-clock-
  bounded repetition lookback) shipped on correctness grounds at zero
  Elo cost. Part 2 (TT-safe Zarkov single-rep draw, gated on winning
  eval) shipped at **+7.6 ± ~10.5 Elo pooled** (both machines positive)
  — correctness fix that also tested positive. **Part 2 history**
  (bug class 2): attempt 1 (Zarkov-rule `isRepetition`)
  fixed 7/10 of the fixture but regressed −40 Elo (likely TT
  pollution from path-dependent draw scores) and was reverted.
  **Attempt 2 (TT-safe Zarkov, `34c336e`+`996c3f8`): Elo-neutral**
  — −1.6 ± ~10.5 over 2000g pooled (Intel −3.13, AMD literal 0); the
  TT-safe early-return killed the −40 regression (TT-pollution
  hypothesis confirmed) but the rule adds no measurable Elo.
  **Attempt 2b (`304f2b7`)**: narrowed the single-rep draw to
  clearly-winning positions only — fixture-neutral (6/10 = broad).
  **Pooled t6 +7.6 Elo** (Intel +9.4 / AMD +5.9, both machines
  positive, LOS ~92%, 2000g) — a ~+9 Elo swing over the broad neutral.
  Positive-lean + correctness, just under the 95% ship bar. See the
  Part 2 detail below.
- pooled t6 gauntlet (400g, two machines @ 10+0.1 vs `huginn_t6`):
  - Intel 200g: +19.13 ± 32.82 Elo, LOS 87.45%, W47/L36/D117, Draw 43.0%
  - AMD 200g: +3.47 ± 34.84 Elo, LOS 57.78%, W49/L47/D104, Draw 52.0%
  - **Pooled: W96/L83/D221 (51.6%), ~+11 Elo** — statistical dead
    heat to mildly positive; **no regression, no draw-heavy
    regression** (vs ~60% draws in the t6-vs-t5 baseline). A
    correctness refinement that pays its own way: kept on
    correctness grounds (fixes provably-thrown wins) rather than as
    a measured Elo gain.
- priority: high
- type: research / search bug
- est: bug-class-1 done; bug-class-2 ~1-2 sessions

**Findings (2026-05-18) — full writeup in
[../tools/repetition_findings.md](../tools/repetition_findings.md):**

Pipeline (tooling, all under `tools/`):
- `repetition_analysis.py` — parses both t5 PGNs, extracts 3-fold
  draws, reconstructs the pre-repetition position + start FEN + full
  UCI move list, flags Huginn-clinched winning-repeat candidates.
- `repetition_research.py` — replays each candidate's full history
  into current Huginn (300ms, so the t6 root-demotion can fire) and
  probes **Stockfish 18 @ depth 24** on the bare position as an
  objective oracle (Stockfish copied to the fastchess dir).
- `repetition_regression_build.py` — emits the history-aware
  regression fixture `tools/repetition_regression.json`.

Numbers: 238 three-fold draws / 400 games; 126 Huginn-clinched; 20
were Huginn-clinched with reported eval ≥ +300cp **and** a legal
non-repeating alternative. Re-search verdicts on those 20:

| Verdict | N | Meaning |
|---|---|---|
| **ARTIFACT** | 4 | Stockfish says position is ~0/drawn — Huginn's big eval was inflated. Not a thrown win. Discard. |
| **FIXED_BY_T6** | 9 | Stockfish confirms won, but current Huginn no longer repeats. The t6 fix already handles these — keep as regression guards. |
| **REAL_BUG** | 7 | Huginn *still* clinches the repetition and Stockfish confirms the position is won. The genuine residual bug. |

The 7 REAL_BUG split into two sub-classes:
- **alt_exists (5)** — a clearly winning *non-repeating* move exists
  (SF best ≠ clincher); Huginn picks the repetition anyway. Cleanest
  fixable class. `intel-R8` (KQ vs K, +M), `intel-R95` (KR vs Kp,
  +1196), `intel-R19` (+419), `intel-R91` (+562), `amd-R32` (+756).
- **history_dependent (2)** — SF's own best move *is* the clincher
  (objectively best from the bare position, but a 3-fold given game
  history): `amd-R54` (+709), `amd-R87` (+565). These need explicit
  root claimable-repetition detection, not a `bm` test.

**Method note / correction:** an early harness bug (piping `quit`
before reading `bestmove`, which aborts the engine's background `go`)
made Huginn falsely appear to still repeat on *every* candidate. A
streaming UCI driver fixed it; the honest residual is 7/20, not 20/20.
The t6 conversion fix is more effective than the raw "draws went up"
observation suggested — most large-eval 3-fold endings are now either
artifacts or already-corrected.

**Bug-class-1 fix applied (2026-05-18) — `src/search.cpp`
`isRepetition`:** the original hypothesis (eval dips under +300cp)
was *falsified* by a direct probe: all 7 REAL_BUG positions have
Huginn static eval ≥ +300 (320–1370cp; added an `eval` UCI command
for this). The real root cause for the worst cases is that
`isRepetition` only scanned the **last 12 plies** (`start_check =
size - 12`), a conservative perf hack. Slow long-period shuffles
evade it: `intel-R8` (KQ vs K) had its prior occurrences 16 and 22
plies back, so the t6 demotion *never fired* and the engine drew a
forced mate.

Fix: bound the lookback by `pos.halfmove_clock` instead of a fixed
12. Any irreversible move (capture / pawn / castle) resets the
halfmove clock and makes the current position structurally
unreachable again, so a zobrist match within the last
`halfmove_clock` plies is necessarily a *true* repetition — the
wider scan can only find real threefolds earlier, never false ones.

Result on the regression fixture (history replayed, 300ms):
**REAL_BUG 7 → 5** — `intel-R8` (thrown KQ-vs-K mate) and
`intel-R95` (thrown KR ending) now avoid the repetition.
FIXED_BY_T6 9 → 11, ARTIFACT 4 → 4 (no FIXED→REAL backslide), all
194 unit tests pass.

**Intel t6 gauntlet (2026-05-18, 200g @ 10+0.1 vs `huginn_t6`):**
+19.13 ± 32.82 Elo, LOS 87.45%, W47 / L36 / D117 (52.75%),
DrawRatio **43.0%**. The change widens in-search repetition
detection (search.cpp:1373) — exactly the surface that could
reintroduce the draw-heavy regression the tree-wide variant showed
— but the run is positive with *fewer losses than wins* and a
draw rate well below the ~60% of the t6-vs-t5 baseline, so the
feared drawishness did **not** materialize.

The AMD 200g pool (`fdbc9b3`) returned +3.47 ± 34.84 Elo, LOS
57.78%, W49/L47/D104 — a dead heat. Pooled 400g: W96/L83/D221
(51.6%), ~+11 Elo. LOS doesn't reach the ~95% Elo-ship bar, but
the change is a *correctness* fix (it stops the engine drawing
provably-won games — confirmed on the regression fixture, e.g. a
thrown KQ-vs-K mate) at **zero measured Elo cost and no draw
regression**, so **Part 1 ships on correctness grounds**.

**Remaining: bug class 2 (5 cases, still open).** For
`intel-R19`/`intel-R91`/`amd-R32` (alt_exists) and
`amd-R54`/`amd-R87` (history_dependent) the demotion *does* fire
(eval ≥ +300, repetition within window) yet the engine still
clinches: every non-repeating alternative also scores ≤ the
−200 clamp because the opponent forces the cycle within Huginn's
300ms horizon while Stockfish needs depth 24 to see the win. A
−200 *penalty* can't help when there is no >−200 alternative to
switch to. Next action: replace/augment the flat penalty with a
*positive* progress signal or a search extension in clearly-won
positions, plus explicit root claimable-repetition detection
(candidate fixes 1-2). Validate against
`tools/repetition_regression.json` *and* a fresh t6 gauntlet.

**Post-fix harvest (2026-05-18) — Part 1 holds, Part 2 sharpened.**
Re-ran the pipeline on the 400 *post-fix* `vs t6` gauntlet games
(Huginn_current = `a21a037`, Part 1 already shipped): 220 3-fold
draws, 104 Huginn-clinched, 15 winning-repeat candidates. Oracle
re-classification (Stockfish, **bounded `go movetime 3000`** — a
fixed depth wedged the harness for 31 min on a locked pawn
endgame; added a watchdog + movetime bound):

- **FIXED_BY_T6 10 / REAL_BUG 5 / ARTIFACT 0.** Part 1 is
  empirically holding in real post-fix play (10/15 won-position
  repetitions now avoided in re-search).
- The 5 residual REAL_BUG (`amd-R25` alt_exists; `amd-R50`/`R62`/
  `R86`/`R95` history_dependent) are genuine bug-class-2 cases that
  survive Part 1 in real games. **4 of 5 are history_dependent**
  (Stockfish's own best move *is* the clincher) — strong evidence
  that the dominant residual failure is "objectively-best move that
  happens to complete a 3-fold given game history", which a score
  penalty fundamentally cannot fix. This pushes **explicit root
  claimable-repetition detection** (candidate fix 2) ahead of the
  positive-progress-signal idea as the Part 2 lead approach.

Master Part 2 fixture is now `tools/repetition_regression.json`
(10 cases, deduped by pre_fen, `source_set`-tagged): 5 `t5_vs_t5`
(pre-fix) + 5 `t6_post_fix` (survived Part 1 — highest signal).
Rebuilt by `tools/repetition_regression_merge.py`; raw per-source
snapshots in `repetition_*_t5.json` / `repetition_*_t6.json`.

**Attempt 1 — Zarkov-rule `isRepetition` (2026-05-18): REJECTED,
−40 Elo.** Per the Chess Programming Wiki "Repetitions" page the
mainstream design is the Zarkov rule: a true threefold is always a
draw, and a *single* repetition is a draw once `ply_from_root > 2`
(makes the search tree a DAG so the won side, via `-CONTEMPT`,
routes around shuffles instead of only seeing the draw at the
literal 3-fold — which is exactly Huginn's sub-standard
`count >= 3` gate, the structural cause of bug class 2).

Implemented it (single in-tree rep = draw past root+2; threefold
always). Bench results:
- 194/194 unit tests pass.
- **Regression fixture: 7/10 fixed** (was 0/10), incl. 4/6
  history_dependent — the class the `−200` root penalty cannot
  touch. The 3 residuals (`t5-R87`, `t6-R62`, `t6-R95`, all
  history_dependent) are beyond-horizon *conversion* failures: the
  search now correctly scores the cycle a draw but cannot find the
  win in 300 ms (separate endgame-technique problem, not a
  repetition bug — CPW does not claim to solve this).
- **Intel t6 gauntlet: −40.13 ± 33.51 Elo, LOS 0.88%,
  W38/L61/D101, DrawRatio 40.0%.** Decisive regression.

Diagnostic: the failure is *not* drawishness (draws actually fell
to 40%) — **losses surged** (L61 vs ~36 at Part 1). Signature of
search corruption, not passivity. Leading hypothesis: a
repetition draw score is **path-dependent**, but Huginn writes
search scores into the **path-independent** transposition table.
Under `count >= 3` such stores were rare; scoring *single* in-tree
reps as `-CONTEMPT` floods the TT with draw scores keyed by
zobrist that then poison non-repeating paths transposing to the
same key → broad eval corruption → more losses. Mainstream
engines pair single-rep detection with **not storing those scores
in the TT** (or flagging them unusable). Code reverted to the
Part 1 shipped state; not committed to `main`.

**Part 2 next attempt:** redo the Zarkov rule *with* TT-safe
handling — do not write (or do not probe) TT entries whose score
originates from a single-repetition draw node, or only treat the
single-rep-as-draw inside the PV/while not eligible for TT cutoff.
Re-validate on the fixture (expect ≥7/10) *and* a t6 gauntlet
(must clear the −40 and not regress draws). The 3 conversion
residuals are out of scope for repetition work — file under
endgame technique if they persist.

**Attempt 2 — TT-safe Zarkov rule (2026-05-28): Elo-neutral,
REJECT-clean.** Re-implemented the single-repetition-as-draw rule
(`34c336e`) but moved the repetition test to a TT-safe early return:
`repetition_count_in_history` is computed and any single-rep
(`count >= 2 && ply > 2`) or threefold node returns `-CONTEMPT`
*before* the TT probe/store, so path-dependent draw scores never
enter the path-independent TT. Also bumped
`WINNING_REPETITION_DRAW_SCORE` −200 → −800.

Pooled t6 SPRT (2000g @ 10+0.1 vs `huginn_t6`, 1t, noob_3moves.epd),
bounds `[0.00, 10.00]`:
- Intel 1000g: −3.13 ± 14.94 Elo, LOS 34.07%, W226/L235/D539
  (49.55%), Ptnml [26, 139, 181, 126, 28], LLR −0.79.
- AMD 1000g (`996c3f8`): **literal 0 Elo**, W240/L240/D520 (50.00%),
  Ptnml [27, 123, 199, 125, 26].
- **Pooled: W466/L475/D1059 (49.775%), −1.6 ± ~10.5 Elo**, pooled
  Ptnml [53, 262, 380, 251, 54], pentanomial t ≈ −0.29. Statistically
  zero on both machines independently.

(Pentanomial correction, 2026-05-29: the AMD/pooled Ptnml above were
re-derived pairing games by `[Round]` tag. Earlier values
[24,129,200,117,30] / [50,268,381,243,58] used file-order pairing,
which is wrong under fastchess concurrency — games are written in
completion order, not pair order. W/L/D, score and Elo are unaffected;
only the pentanomial wings were off. See
[[feedback-pentanomial-pair-by-round]].)

Read: the TT-safe early-return **fixed the −40 disaster** exactly as
the attempt-1 TT-pollution hypothesis predicted (−40 → −1.6) — that
is the real finding. But the Zarkov single-rep rule itself buys **no
Elo** in t6 conditions, so it does not ship as a gain. Decision
pending: keep the TT-safety refactor (`repetition_count_in_history`
+ return-before-probe, sound and free) and drop the behavioral change
(single-rep draw + the −800 score), vs. revert the whole commit.

Next-target lead: the Intel log is full of `PV continues after
fifty-move rule` with Huginn reporting +744–767cp in positions dead
drawn by the 50-move rule. That is **draw blindness on the 50-move
path**, untouched by this (3-fold-only) patch, and is the more
promising Part 2/3 surface than further repetition tuning.

Provenance note: AMD results arrive via push from the AMD box
(`996c3f8`), so the on-disk `gauntlet/*_amd.*` files are only current
after `git fetch && git pull` — always sync before parsing them.

**Attempt 2b — narrowed gate (option 1, `304f2b7`): queued for
gauntlet.** Decision from the keep-vs-revert debate: don't reject a
neutral change that fixes provable bugs, but shrink its footprint. The
single-rep draw now fires only when the side to move is clearly winning
(`evalPosition(pos) >= WINNING_REPETITION_AVOID_THRESHOLD`); threefold
stays an unconditional draw. Coherent with the rule's purpose (the WON
side routes around shuffles), and confines the speculative single-rep
scoring to the eval band where the bug actually lives.

Fixture (new `tools/repetition_fixture_check.py`, replays each case into
the build and checks clincher-vs-avoid; no oracle needed since positions
are pre-confirmed won): **narrow 6/10 == broad 6/10 — fixture-neutral.**
The earlier "7/10" was from attempt 1's different fixture, never the
broad TT-safe build on this 10-case master. The 1-case narrow/broad
difference (R54 vs R62, both history_dependent) is search-time
instability at fixed movetime, not signal; R32 (alt_exists) repeats in
both builds (static eval +528 > gate; a conversion/horizon residual the
rule cannot reach). 194/194 unit tests pass.

**Attempt 2b result — pooled t6 SPRT (2000g @ 10+0.1, round-paired):
+7.6 Elo, both machines positive.**
- Intel 1000g: +9.38 ± 14.65, LOS 89.56%, W259/L232/D509 (51.35%),
  Ptnml [27, 109, 202, 134, 28], LLR 0.73.
- AMD 1000g (`1d20303`): +5.91, W250/L233/D517 (50.85%),
  Ptnml [27, 122, 190, 129, 32].
- **Pooled: W509/L465/D1026 (51.10%), +7.6 ± ~10.5 Elo**, pooled
  Ptnml [54, 231, 392, 263, 60], pentanomial t ≈ +1.43.

The narrowing thesis is confirmed: vs the broad attempt-2 (−1.6,
t ≈ −0.29) this is a **~+9 Elo swing at identical fixture coverage
(6/10)** — confining the single-rep draw to winning positions removed
a diffuse drag the board-wide version carried. Both machines agree on
direction. **Pooled LOS ~92% is just under the ~95% ship bar and the
SPRT LLR has not crossed 2.94**, but the change now clears the Part 1
precedent (correctness fix shipped at neutral) with room to spare:
positive-leaning Elo *and* the provable-bug fix. Decision: ship on
correctness+positive-lean, or extend one more pooled round for a clean
SPRT stamp.

**State:** `baseline-t6` shipped the first repetition-related fix:
root moves that immediately repeat while the root side is clearly
ahead (`>= +300cp`) are demoted, and PV reconstruction now stops at
repeated position keys. The gauntlet result was a success:

- Intel 13700K vs t5: W45 / L36 / D119, **+15.65 +/- 33.21 Elo**,
  LOS 82.31%.
- AMD 7800X3D vs t5: W48 / L32 / D120, **+27.85 +/- 31.01 Elo**,
  LOS 96.19%.
- Pooled: W93 / L68 / D239, score 53.125%, **~+21.8 Elo**.
- Tag: `baseline-t6` at `a31d7a8`.

The improvement came mostly from fewer losses, not fewer draws. Draws
increased versus the t5 cleanup baseline, so this did **not** solve
the original observation: some games still show large reported evals
ending in 3-fold repetition.

**Evidence to mine:** after the first Intel t6 run, a quick PGN scan
still found high-eval 3-fold endings in `gauntlet/huginn_vs_t5_intel.pgn`
(examples included final comments like `+280.00`, `+21.85`, `-12.25`,
`+11.40`, etc.). Some are likely opponent-side or mate-score artifacts,
so do not assume every large value means Huginn threw away a win.

**Next action:** build a small analysis tool around actual PGN data:

1. Parse `gauntlet/huginn_vs_t5_intel.pgn` and
   `gauntlet/huginn_vs_t5_amd.pgn`.
2. Extract every game ending in `Draw by 3-fold repetition`.
3. For each, record: round, side/engine to move on the final move,
   final SAN/UCI if recoverable, final eval/depth/comment, result,
   full move list, and whether Huginn_current or Huginn_t5 made the
   repetition-clinching move.
4. Reconstruct the position a few plies before the final repetition
   and output FEN + legal alternatives. Prefer using engine movegen or
   a chess library rather than ad-hoc SAN parsing.
5. Re-search those positions with current Huginn at fixed depth/time
   and compare:
   - Does it choose the repetition?
   - Is root static eval actually winning?
   - Are non-repeating alternatives present and scored close enough?
   - Is the big PGN eval a real score, a mate/tablebase artifact, or
     just PV pollution from the opponent baseline?
6. Turn confirmed "Huginn repeats while winning with alternatives"
   cases into a targeted regression set before touching search again.

**Candidate fixes after data exists:**

- Penalize repeated-position candidates by root-side eval only when the
  candidate move itself is the repetition-clincher and the engine has a
  non-repeating alternative inside a sane score band.
- Detect twofold/claimable repetition at root more explicitly instead
  of relying on `isRepetition()` after MakeMove.
- Tune thresholds (`+300cp`, `-200cp`) only against the extracted
  positions plus a normal t6 gauntlet; avoid broad tree-wide repetition
  scoring for now. A quick 20-game pilot of the tree-wide variant was
  neutral and very draw-heavy (3W / 3L / 14D), so that path is suspect.

---

### #29: Fifty-move-rule search blindness

- status: **PART 1 KEPT on correctness grounds @ `534b44c`
  (2026-05-29).** Pooled t7 SPRT was −5.73 Elo (Intel −12.86 / AMD
  +1.39, pentanomial t = −1.08), **but this is variance, not a
  regression** — see the gauntlet read below. **PART 2 (clock-scaled
  eval) DEFERRED** — equally unmeasurable at this TC.
- priority: low (correctness done; no measurable strength lever here)
- type: search bug
- est: P1 done; P2 deferred

**The bug.** `halfmove_clock` appeared in `src/search.cpp` only in the
#28 repetition lookback and the mirror/TB helpers — **`AlphaBeta` and
`quiescence` had no fifty-move-rule draw check at all.** The engine
could not see a 50-move draw: it scored dead-drawn positions at their
material eval (e.g. +744–767cp in KR/KQ endings the t6/t7 Intel logs
flagged with `PV continues after fifty-move rule`, 22 such warnings in
one 1000g run). Blind to the draw, it had no incentive to make progress
and shuffled won games into the rule.

**Part 1 fix (`src/search.cpp` AlphaBeta).** Added a TT-safe early
return in the same block as the repetition handling: if
`halfmove_clock >= 100` and the side to move is **not** in check, return
`-CONTEMPT`. Placed before the TT probe/store because a 50-move draw
score is path-dependent (`halfmove_clock` is not in the zobrist key, per
the TB note) and must never be cached. The not-in-check guard preserves
checkmate-takes-precedence: an in-check node falls through so movegen
distinguishes mate from a legal escape. Quiescence needs no check — it
is entered at `depth==0` *after* this block, and only explores captures
(which reset the clock).

Validation: KQ-vs-K at `halfmove=100` now scores **+25cp (draw)** vs
**+1008cp** for the same position at `halfmove=0` (control) — previously
both read ~+1000. 194/194 unit tests pass.

**Gauntlet read (2026-05-29) — kept on correctness; the −5.73 is
variance, proven two ways.** Pooled t7 SPRT 2000g: Intel −12.86
[34,134,193,113,26], AMD +1.39 [26,121,198,133,22], pooled W456/L489/
D1055 (49.18%), pentanomial t = −1.08.
- **The code almost never runs.** Reconstructing the halfmove clock for
  all 1000 Intel games: only **2 reached the ≥100 threshold, both
  draws — zero wins, zero losses**. Median max halfmove clock is **9**
  (these ~1600-Elo engines trade constantly, resetting the clock); only
  8/1000 games even reached 80. The 50-move rule is a near-non-factor
  at 10+0.1.
- **The only diff is a gated branch.** `baseline-t7 → 534b44c` is
  exclusively the fifty-move block, guarded by `halfmove_clock >= 100`
  — false in ~998/1000 games, so the binaries play identically there.
  Code that does not execute cannot cost 13 Elo. The Intel −12.86 is a
  ~0.85σ excursion; AMD +1.39 is the clean read, matching every prior
  correctness fix on that box. The SPRT `[0,10]` trending to reject only
  means "not a +10 gain" — expected for a correctness fix, not harmful.

Decision: **keep** (provably correct, provably inert, insurance for
longer TC / endgame-heavy play); **do not bump the baseline** (t7 stays
the strength baseline, #29 rides in the tree); the experiment simply had
near-zero power to measure this change.

**Part 2 — anticipate the draw via clock-scaled eval — DEFERRED.** The
idea (mirror Stockfish: scale static eval toward 0 as `halfmove_clock`
rises, e.g. `eval * (100 - rule50)/100`, so the engine makes progress
before the draw enters the horizon — no forced/sub-optimal pawn moves)
is sound, but it would be **equally unmeasurable at 10+0.1**: if the
rule is reached in 0.2% of games, scaling toward it won't move Elo
either. Revisit only alongside longer-TC or endgame-suite testing.

---

### #30: Nondeterministic search from uninitialized history — FIXED

- status: **FIXED (2026-05-29).** Zero-initialized `search_history`.
- priority: high (correctness; reproducibility)
- type: bug (uninitialized memory)

**Symptom.** At fixed depth (`go depth N`), the same position could
return a different best move and score run-to-run. Demonstrated on KQK
(`8/8/8/4k3/8/3QK3/8/8 w`): best move flipped d3e4/d3d4/d3d7 with scores
1012–1051 across runs. The user asked the key question — "is the best
move varying too?" — which it was.

**Diagnosis (ruled out, then nailed).** Not the queued-`quit` test
harness (persisted with `quit` delayed past the search), not time
management (`go depth` runs in `infinite` mode, bypassing the 5s
checkup cap), not TT garbage (`resize()` zero-inits, probe requires a
full 64-bit key match), not all-TB-probes (KRvK/KPvK/7-pc endgames were
deterministic). The tell: KQK is **all quiet queen moves with ~20
near-equal winning options**, so its result is decided entirely by
quiet-move ordering — i.e. `search_history`.

**Root cause.** `int search_history[13][64];` (search.hpp) was a raw
array member never initialized by the constructor, and
`clear_search_tables()` *ages* it (`/4`) rather than zeroing — so the
first search read per-process heap **garbage**, and that garbage drove
quiet-move ordering. Garbage differs per process (allocator/ASLR) →
nondeterministic ordering → nondeterministic results wherever quiet
ordering is decisive (KQK), invisible where a clear best move dominates
(Kiwipete, normal middlegames).

**Fix.** `int search_history[13][64] = {};` (default member init).
After: KQK returns d3d7/cp1050 every run; a non-TB middlegame is
bit-identical (same move, score, and node count 5527609 every run).
194/194 unit tests pass. Verified across an 8-position sweep (startpos,
Sicilian, Kiwipete, two KQK variants, rook endgame, sharp middlegame,
rook+pawns) — **all fully deterministic** (move + score + node count).
The only remaining run-to-run variation is intentional **opening-book**
weighted-random move selection (`OwnBook`, disabled in gauntlets); with
`OwnBook=false`, startpos is bit-identical too. No sibling uninit bugs:
`killers`/`counter_moves` are zeroed (not aged) by `clear_search_tables`,
`mvv_lva_scores`/`search_stack` are constructor-initialized — history was
unique because it was aged, not zeroed.

**Impact.** The Engine is constructed once per process, so in real games
the garbage only tainted the *first* search per game (history then
accumulates) — modest game impact, but every fresh-process fixed-depth
measurement was affected, which is why node-count benchmarking was
unusable. Fixing it restores deterministic fixed-depth benchmarking and
removes a (small) noise/Elo source. Plausibly related to the #26
"uninitialised `board64`" cross-machine disagreement — same bug class.
A residual ~0.01% node-count jitter on KQK only (move/score stable) is a
benign leaf-level TB/checkup artifact, not search-affecting.

---

### #21: PV continues past threefold repetition — CLOSED

- status: closed @ `5efaa78` / shipped in `baseline-t6` (2026-05-17)
- priority: was low
- type: bug

**State:** During the 2026-05-15 AMD t4 baseline gauntlet
(`huginn_vs_t4_amd.pgn`), fastchess logged repeated
`Warning; PV continues after threefold repetition` from both builds,
e.g. `info depth 20 score cp 25 ... pv h7g8 f4e3 f7e5 e3f4 e5f7 f4e3
f7e5 …` — a 2-cycle move loop padded out to the full PV array, with a
static score and depth still climbing 17→20.

**Diagnosis:** `PVTable::get_pv_line` walked the PV/TT chain without a
repetition guard, so in a drawn-by-repetition line it followed cycles
until the requested PV length filled. The reported `score cp 25`
often matched the shipped contempt value (#16, `CONTEMPT = 25`),
which indicated the search itself was frequently scoring the position
as a contempt-adjusted draw while the printed PV was garbage.

**Fix:** `PVTable::get_pv_line` now seeds a `seen_keys` set from the
actual game history plus the reconstructed PV walk, and stops
appending once the next made move reaches a repeated position key.

**Result:** In the Intel t6 gauntlet log, current Huginn produced zero
`PV continues after threefold repetition` warnings; all remaining
warnings came from frozen `Huginn_t5`. This is closed as a PV/log
pollution bug. The separate playing-strength question of converting
winning repetition draws remains open as #28.

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
