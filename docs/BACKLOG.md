# BACKLOG (Huginn 2.3+)

> **Lean working backlog** — open and in-progress items only, plus a deferred-
> ideas list. Full closed-item history lives in three archives:
> [BACKLOG-archive-2.0.md](BACKLOG-archive-2.0.md) (2.0-era, issues #1–#37),
> [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) (2.1/2.2-era, t16→t24: the
> #41 calibration study, #43 NMP round, #44 repetition fix, #45 futility fix +
> pruning-stack audit, #46–#50, and the 2026-07 SPRT queue that produced
> `baseline-t24`), and [BACKLOG-archive-2.2.md](BACKLOG-archive-2.2.md)
> (2.2/2.3-era, t25→t34: the 2026-07-09 audit criticals #52–#61, the
> SF18-gap-study selectivity program #62/#17-r2/#63, #42 TT aging/clusters,
> #31's hash sweep, and the full road-to-v2.3 release checklist incl. the
> final Stash recalibration). Issue numbers are preserved so cross-references
> into any archive still resolve.
>
> **Current baseline + strength:** see [CLAUDE.md](CLAUDE.md) (top-level
> status) and [BASELINE_LADDER.md](BASELINE_LADDER.md) (full per-baseline
> writeup, newest first) — not duplicated here.

## Version roadmap

Agreed 2026-07-13, confirmed at the v2.3 recalibration (2026-07-16):
**v2.3** = the recalibrated classical-engine peak (this cycle's release,
~2600–2680 CCRL-blitz — see `#5` below); **v2.4** = Lazy SMP
(infrastructure, same eval — note SMP breaks fixed-depth signature
ship-discipline, plan a new verification method, see `#40`); **v3.0** =
NNUE (identity change: trained net ships with the binary — deliberately
major-versioned, see `#39`). Don't fold SMP or NNUE work into 2.4's
boundary casually.

## Road to v2.3 — RELEASED (2026-07-16)

All six checklist items are done (full detail in
[BACKLOG-archive-2.2.md](BACKLOG-archive-2.2.md)): the selectivity/TT-aging
ladder (t31→t34), the LMP/hash-sweep closures, the WAC300/LCT2 hygiene
sweep (which surfaced and fixed a UCI position-rejection observability
gap), the `#60` parser-purity + randomized-invariant tests, and the final
Stash recalibration (~2625 ± 18, reported as a ~2600–2680 range — see `#5`
below for the full number and its caveats). Tagged `v2.3`; see
[CHANGELOG_2.3.md](CHANGELOG_2.3.md) for the shipped-feature ladder.
**Remaining:** snapshot the release build (per-machine, usual convention).

## Open / in-progress

| # | Title | Status | Type | Priority |
|---|-------|--------|------|----------|
| 37 | Board-desync illegal bestmove | **GUARDED + INSTRUMENTED**; root cause OPEN (needs repro) | bug | high |
| 9 / 35 | Texel eval program + tapered eval | **IN-PROGRESS, paused** — search/EBF work has dominated since #45; roadmap below | feature/eval | high |
| 5 | Recalibrate vs external opponents (CCRL scale) | **MEASURED @t34/pre-v2.3 (2026-07-16)** — ~2600–2680 CCRL-blitz, pooled ~2625 ± 18; next: drop stash19, use 20/21/21.2 | maintenance | medium |
| 34 | Pin/blocker-aware legal movegen | **OPEN** | speed/research | low |
| 39 | NNUE evaluation | **DEFERRED** (HCE first) — big lever | feature/eval | — |
| 40 | Lazy SMP / multithreading | **IN-PROGRESS (2026-08-04)** — phases 1–4 landed; `Threads=8` measures **+199 ± 26** vs 2.3 on one box; phase 5 (ship protocol) open | feature/speed | high |

### #37: Board-desync illegal bestmove — GUARDED + INSTRUMENTED, root cause OPEN

A rare, **time-dependent** make/unmake imbalance can leave the engine's internal
board desynced from the real root position, so the search can return a move that
is illegal in the actual position (observed as `b2b4` in round-6 vs t14 on
Intel; fastchess forfeits on it). Guarded (UCI boundary validates + substitutes,
never forfeits) and instrumented (`ENABLE_SEARCH_INTEGRITY_ASSERTS` diagnostics)
— see [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) for that work's detail.

- **Root cause still open.** Not reproducible cold / warm-TT / faithful replay —
  only under real time pressure. Audited every make/unmake site + the primitives;
  all balanced on inspection. **Next:** run an instrumented self-play / fastchess
  repro until it trips, then bisect from the captured context.
- **Follow-up hook (2026-07-04):** the #50 fix (Zobrist black-king-row OOB,
  shipped `baseline-t23`) removed a real TT-key-collision source. Zobrist
  collisions corrupting TT-move sourcing was one of the few mechanisms that
  could explain a rare illegal bestmove — worth re-testing whether #37 still
  repros post-#50 before investing further debugging time here.

### #9 / #35: Eval program — Texel tuning + tapered eval (IN-PROGRESS, paused)

The main strength thread. A `game_phase_256` tapered eval foundation (#35) plus
a Texel tuner (`tools/texel/`, `bake.py` rewrites headers from the tuner dump,
verified exact) drive successive full-vector re-tunes / new-feature rounds on
the 725k Zurichess quiet-labeled corpus. Shipped ladder (full round-by-round
detail in [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) /
[BACKLOG-archive-2.2.md](BACKLOG-archive-2.2.md)):

- t10: tapered-eval foundation · t11: material+PSTs+king-EG · t12: tapered EG
  PSTs + tunable mobility · t13: connected+backward pawns · t14: rook-on-7th
  (sign-split exception) · t15: threats (largest eval-term ship) · t16: king
  safety (first KS ship) · t19: safe mobility (cross-machine-agreement ship) ·
  t30: threats round 2 (pooled +17.0 two-machine)

Method note (learned across rounds): **new-feature MSE converts to Elo better
than re-fit MSE**; SPRT decides every round. King safety (t16) is the
exception that proves a corollary: quiet-corpus MSE UNDER-states terms that
pay in sharp positions.

**Paused since 2026-06-27:** the #45 pruning-stack audit (now closed, see
archive) redirected the active thread from eval to search-soundness, which
dwarfed any eval term in the same period (#45 +510, #47 +127 vs the ~+30 from
t16–t19 combined). Round 7+ HCE roadmap remains, priority order set by the
#41 calibration study (king safety → threats round 2 → safe mobility →
outposts; king safety, safe mobility, outposts, and threats round 2 are all
done — see archives). **Re-diagnosed a third time vs t34 (2026-07-15)**,
same reading: SF move-match 48.5%→51.8%, fair cp-loss 15.6→12.3cp, middlegame
over-optimism 29.0→22.7cp, this time WITH a real depth jump (+3.1 ply) —
search/EBF work (t22→t34) keeps dominating the one eval ship (threats round
2) in this window; full numbers in
[BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md#41-played-game-calibration-study--round-7-priority-evidence-done-2026-06-14)
and [BACKLOG-archive-2.2.md](BACKLOG-archive-2.2.md).

- **Passed-pawn refinements** — king distance to the passer (own + enemy),
  blockade, rook-behind-passer. **Deprioritized**: #41 shows balanced-endgame
  play is already solid (fair-fight cp-loss 13.3). Not yet attempted.
- **Lower priority:** doubled-rooks / blind-pig follow-up to t14's
  rook-on-7th; space (safe squares behind own centre pawns); rook-on-king-file;
  specific endgame recognizers (KPK, KRKP).

### #5: Recalibrate vs external opponents (OPEN)

Re-anchor the CCRL-Blitz-scale Elo estimate. Scripts: `test_huginn_gauntlet.bat
<snowy|cdrill|mtl03|mora|mtl05|mtl06|stash11..stash21|stash154|stash212>`; the
2.0 archive has the t11 3-anchor MLE (~1818). Full round-robin + Stash-ladder
calibration detail (round-robins, findings, the score-compression/
conversion-weakness lead) is in
[BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) under this item's original
write-up — key finding carried forward:

- **Score compression / monotonic anchor drift (2026-06-15):** the per-anchor
  estimate *climbs* with opponent strength (Huginn over-performs a flat Elo
  model vs strong opponents, under-performs vs weak ones — 99 draws vs Stash
  11.0). Reads as a **conversion weakness** — solid vs stronger engines but
  doesn't convert wins vs weaker ones. Points at drawishness-scaling /
  50-move-conversion items (drawishness-scaling now tested+parked, see the
  Deferred/parked list below — a weaker-anchor retest is the natural next step
  for that item, which would also feed back into this one).
- **Reproduced at the road-to-v2.3 recalibration (2026-07-16):** the same
  climb-with-opponent-strength pattern, on a completely different anchor
  bracket. Stash 19/20 (weaker, 2473/2509) pinned Huginn at ~2604; Stash 21
  (stronger, 2714) pinned it ~72 Elo higher (~2676) — same direction as the
  June finding, now confirmed on a second, much higher-strength anchor pair.
  Two recurrences on non-overlapping anchor sets makes "it's just this one
  anchor's rating being off" a weaker explanation than the standing
  conversion-weakness read. **Latest measurement: ~2600–2680 CCRL-blitz,
  pooled ~2625 ± 18** (Stash 19.0 +132.28/68.17%/300g → ~2605; Stash 20.0.1
  +93.95/63.20%/500g → ~2603; Stash 21.0 −38.37/44.50%/300g → ~2676). Full
  numbers: [BASELINE_LADDER.md](BASELINE_LADDER.md).
- **Next:** drop Stash 19 (least informative rung now), use 20/21/21.2 as the
  three-anchor bracket (Stash 21.2 = CCRL 2785, wired into the bat as
  `stash212` but not yet run). Re-run the Stash-anchored round-robin to full
  2400g (prior run stalled at 2049g on a sleep interrupt) with sleep disabled
  (`powercfg /change standby-timeout-ac 0`) to tighten CIs if pursuing the
  older round-robin method instead. Then fold the refreshed estimate into
  CLAUDE.md (already done for the 2026-07-16 measurement above).

## Big levers (deferred — HCE first)

The two architectural jumps that dwarf any single HCE term. Both deliberately
deferred (2026-06-13) to exhaust pure-HCE headroom first — but recorded here so
the strategic picture is explicit.

### #39: NNUE evaluation (deferred)

- The per-node *quality* lever — hundreds of Elo, and the honest path to closing
  the gap to Fruit/Stockfish-class strength. Replaces (or augments) the HCE
  `evaluate()` with a small efficiently-updatable network.
- **Measured 2026-07-12** ([SF18_GAP_STUDY.md](SF18_GAP_STUDY.md)): on
  Kiwipete, SF18's depth-1 NNUE eval is closer to the converged truth than
  Huginn's depth-16 HCE result, and Huginn's static eval carries the wrong
  sign until depth 6. Same study: Huginn is ~2× faster in raw nps but its
  tree is 112× larger at equal depth (EBF ~1.90 vs ~1.37) — the two levers
  in one number each.
- **Why it fits this project:** preserves single-thread determinism (the
  fixed-depth / cross-machine-bit-identical test methodology survives), and it
  improves the thing we actually care about — move quality per node.
- **Cost:** a multi-session build — training-data generation (self-play at depth,
  labelled), the net + quantisation, and incremental-update integration into
  make/unmake. Highest conceptual lift of the options; lowest methodology cost.
- **Hardware fit (2026-06-26):** this is also where the AMD box's **AVX-512** earns
  its keep — accumulator updates + int8/int16 dot products map to `AVX512-VNNI`
  (`VPDPBUSD`) for a big inference speedup. The current HCE engine doesn't vectorize
  (see the AVX-512 note under #32 in the archive), so NNUE is the project that
  justifies an AVX-512 code path. (Intel consumer 12–14th gen lacks AVX-512 →
  the fast path would be a build-gated AMD variant with an AVX2 fallback.)

### #40: Lazy SMP / multithreading — IN PROGRESS (started 2026-08-04)

**Branch:** `smp-phase1-lockless-tt`. Kickoff decisions (2026-08-04): proceed
without first root-causing #37 (it stays guarded + instrumented; accepted cost
is that a future desync report is ambiguous between #37 and a new SMP race),
and land each prerequisite refactor as its own signature-gated commit rather
than folding them into a threading branch.

**Phase ladder** — the two refactors are semantics-preserving, so each is gated
on the `baseline-t34` fixed-depth signatures (startpos d14 = 3,481,582 / cp 31
/ e2e4; Kiwipete d13 = 1,958,182 / cp −85 / e2a6) and expected to measure
*zero*. Both were run as fixed-round gauntlets rather than SPRT: the question
is "is it not worse", not "is it better", and an early-stopping SPRT returns a
loose CI on a true null.

| Phase | Commit | Signatures | Tests | Measurement |
|---|---|---|---|---|
| 1. Lockless-safe TT (Hyatt XOR key) | `3ea44d2` | identical | 295/295 | **+0.12 ± 8.29** vs t34, LOS 51.1%, 3000g (NPS +0.1%) |
| 2. Shared TT/book across Engines | `50439da` | identical | 300/300 | **+4.75 ± 8.41** vs t34, LOS 86.6%, 3000g (NPS −0.45%) |
| 3. Spawn helper threads (`SearchPool`) | `3d836d8` | identical @ `Threads=1` | 310/310 | see scaling below |
| 4. Re-advertise `Threads` option | `fa8a655` | identical @ `Threads=1` | 310/310 | — |
| 3r2. Real per-thread diversification | `429cefd` | identical @ `Threads=1` | 310/310 | **+199.05 ± 25.63** @ t8 vs 2.3, LOS 100%, 400g |
| 5. Ship gate (default arm) | — | identical @ `Threads=1` | 310/310 | **+1.04 ± 14.91** vs 2.3, 1000g · **+2.78 ± 14.80** vs phase-2 build, 1000g |

**`Threads=1` is bit-identical to `baseline-t34` at every phase** (startpos d14
= 3,481,582 / cp 31 / e2e4; Kiwipete d13 = 1,958,182 / cp −85 / e2a6). That
contract is what carries the ship discipline now that fixed-depth node counts
stop being a comparable unit above one thread — the shipped default is
single-threaded, so the default binary is provably unchanged.

#### The r1 diversification bug (worth remembering)

Phase 3 r1 skewed only the **start depth** by `thread_id % 2` — exactly TWO
distinct schedules, so at 8 threads workers 1/3/5/7 all ran one identical
schedule and 2/4/6 another. Every helper past the second was re-deriving a
result another thread was already computing. It was invisible to the test
suite (all the correctness properties held) and only showed up as a *scaling*
measurement:

| Kiwipete d17, median of 3 | t1 | t2 | t4 | t8 |
|---|---|---|---|---|
| r1 time-to-depth | 1.00× | 1.49× | 1.40× | **1.49×** (saturated) |
| r1 nodes | 34.05M | 45.39M | 87.33M | **147.39M** |
| r2 time-to-depth | 1.00× | 1.73× | 2.38× | **3.61×** |
| r2 nodes | 34.05M | 26.98M | 48.29M | **48.23M** |

Fixed by Stockfish's iteration-skipping (19 distinct size/phase pairs) plus the
per-thread aspiration skew the design specified but r1 never implemented.
**Lesson: a correct-but-non-diversified Lazy SMP passes every functional test
and still wastes the hardware.** Scaling has to be measured explicitly.

#### Depth scaling (r2), two independent methods

Controlled A/B — identical position, fixed 1000ms, only `Threads` varying, 40
positions evenly sampled from `CC_Games.pgn`'s 1929 eligible, median of 3:

| | t1 | t2 | t4 | t8 |
|---|---|---|---|---|
| mean depth | 13.57 | 14.30 | 14.82 | 15.43 |
| vs t1 | — | **+0.72** | **+1.25** | **+1.85** ply |
| deeper / equal / shallower | — | 27/12/1 | 35/5/0 | **38/2/0** |

Real games (400g vs 2.3, 48,925 annotated moves, paired by game+move): **+1.92
ply** at t8, deeper on 79% of moves, shallower on 6.4%. Node use 3.04M/move vs
0.54M (5.6×); `s/move` identical at 0.22 (time management undisturbed).

The two methods agree to **0.07 ply**. Move-pairing is confounded in principle
(White's move N and Black's move N are different positions — a deep quiet
position vs a forced recapture), but at 24k pairs with colour-balanced openings
it washes out, so the free-with-any-gauntlet method is usable.

> ⚠ **The +199 Elo does NOT move Huginn's ~2625 CCRL number.** CCRL's main
> Blitz / 40-15 lists run engines single-core, so a `Threads=8` gain does not
> apply there. What was measured is *Huginn on 8 cores vs Huginn on 1 core* —
> the maximum-possible SMP delta by construction (at equal thread counts the
> difference is zero). It belongs in release notes as a **multicore-scaling
> figure**, not as a strength increase, and must not be added to the calibrated
> rating (see `#5`).

#### Phase 5: ship protocol (open)

**Release-claim framing — DECIDED (2026-08-04, user call):** report the
multicore gain as a **separate figure**, not folded into the rating. The
calibrated number stays "~2625 CCRL-blitz, single-core" and SMP ships as a
distinct scaling claim ("Huginn on 8 cores vs 1 core: +199 ± 26 Elo"). Any
phrasing that lets the two be added is wrong on every CCRL list.

**Ship gate: PASSED (2026-08-04).** `Threads=1` (default) vs 2.3, 1000g, cc=4:
**+1.04 ± 14.91**, LOS 55.5%, 50.15%. And the stronger, baseline-free version —
phase 4 @ `Threads=1` vs the **phase-2 binary**, isolating exactly
`3d836d8 + fa8a655 + 429cefd` — **+2.78 ± 14.80**, LOS 64.4%, 1000g. The
threading commits cost nothing at the shipped default.

> **A false negative happened first, and the lesson is worth keeping.** The
> first run of this gate returned **−17.39 ± 14.01, LOS 0.74%** — an apparently
> significant regression. It was wrong. Two causes: (a) engine-spawning
> measurement scripts were run on the box *during* the timed match (including a
> `Threads=4` search), and (b) ordinary variance — the two runs differ by ~1.8σ.
> Diagnosis order that worked, cheapest first: node counts + NPS across all
> three binaries (identical → not throughput), then a diff of the
> single-threaded path (every phase-3/4 change provably inert at `thread_id==0`),
> then the **baseline-free A/B against the immediate predecessor**, which
> settled it without needing to resolve the apparent t34-vs-2.3 discrepancy at
> all. **Never run anything on a box hosting a timed gauntlet**, and prefer an
> A/B against the immediate predecessor over a chain of comparisons through a
> shared baseline — transitive chains accumulate error and invite chasing
> phantom mechanisms.

**Why fixed-depth bit-identity is not sufficient as a ship gate:** it
guarantees the same *tree*, not the same *strength under a clock*. It cannot
see throughput, time management, or per-search overhead. The `Threads=1`
signature remains a necessary cheap check that the search is unchanged, but the
clock-based A/B against the immediate predecessor is what actually gates a ship.

**Original gate rationale:** the arm that actually ships is `Threads=1`, which is
a *different arm* from the one measured at +199. It is bit-identical to t34 at
fixed depth, so the gate tests what fixed depth cannot: whether the pool
wrapper, group-stop plumbing, and per-checkup node-mirror stores cost anything
under a real clock. `Threads=1` (default, unset) vs 2.3, 1000g, cc=4 — both
sides single-threaded, so cc=4 is legitimate here. **Nothing merges to `main`
without this.** At 1000g the CI is ≈ ±14 Elo (vs ±8 at 3000), so it bounds a
regression at ~14 rather than ~8 — accepted as adequate for a null test (user
call).

**Queued, not yet run:**
- **Hash sweep at `Threads=8`.** At t8 the engine searches ~5.6× the nodes in
  the same wall-clock, so TT pressure is ~5.6× higher against an unchanged
  64 MB default. The optimal Hash at t8 is very likely larger. Concrete
  mechanism, precedent in `#31`, and plausibly free Elo on top of the +199 —
  it also changes what the release should recommend to users.
- **Scaling curve (t2, t4 vs t1).** Elo per thread count instead of the single
  t8 point. ~3h per rung at cc=1. Depth data predicts ≈ +0.72 / +1.25 ply.
- **Intel leg.** Every baseline in the ladder has two machines; SMP is the most
  hardware-sensitive thing shipped so far (core count, cache, SMT), so
  cross-machine agreement matters more here than usual.

**Also open:** one unreproduced test flake (single ctest run right after a
build; 9 subsequent runs clean — see `429cefd`), and `#37`'s attribution
ambiguity is now live: a future desync report can no longer be cleanly assigned
between the old single-threaded bug and a new SMP race.

Neither gauntlet result can be a real gain — both arms are byte-identical at
fixed depth, so there is no mechanism for one. What the pair actually
establishes is the **measurement noise floor at 3000 games: ≈ ±8 Elo**, which
is the number to judge phase 3 against. (Phase 2's +4.75 also drifted up from
+1.21 at 2018 games during a window when the box may have been in use — read
the CI bound, not the central value.)

- **Phase 1** (`transposition_table.hpp`): the table's storage cell became
  `TTSlot` — two relaxed atomic `uint64`s holding `key ^ payload` and
  `payload`. A read torn across a concurrent store decodes to a bogus key and
  is reported as a miss. Without it a torn read hands `best_move` from an
  unrelated position to `pick_next_move` and singular extensions.
- **Phase 2** (`search.hpp`): scoping showed the planned Engine→SearchWorker
  split was unnecessary — an `Engine` **already is** the right per-thread unit
  (history, killers, counter-moves, PV table, node counter, stop flag are all
  correctly private to a worker). Only the TT and book are genuinely shared, so
  they were lifted out behind a second constructor
  `Engine(TranspositionTable&, PolyglotBook&, SyzygyTablebase*)` with the
  members becoming references. +224/−6 lines instead of a sprawling refactor,
  and every existing call site (incl. all 18 `Engine engine;` test sites) is
  untouched. New `test_smp_shared_resources.cpp` pins both halves of the
  contract.

**Phase 3 design:** [SMP_PHASE3_DESIGN.md](SMP_PHASE3_DESIGN.md). Three things
that draft surfaced, all of which would have been bugs in a naive "wrap
`searchPosition` in threads" implementation:
  1. `clearForSearch` calls `tt_table.new_search()`, and the TT search date is
     **global** (#42). Once per worker would bump it N times per search, making
     worker 0's entries look stale to worker 1 *within the same search* — quietly
     gutting the aging that shipped at t34.
  2. `checkup()`'s input block (`on_input` / `read_input`) is main-thread-only
     ([input_checking.hpp:30](../src/input_checking.hpp#L30)); a helper reaching
     it could consume a GUI command. Needs an explicit `polls_input` opt-out.
  3. The root book/TB probe, `info` emission, and the iteration-start time gate
     are all driver-only, so `searchPosition` must split into driver + worker loop.

  Resolved while drafting: **Fathom is thread-safe as built** — it guards probes
  with real mutexes and `TB_NO_THREADS` is not defined anywhere in our build.

**Why it was deferred until now** (the original writeup, still the live risk list):

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
     **Measured on SF18 (2026-07-12,
     [SF18_GAP_STUDY.md](SF18_GAP_STUDY.md)):** 8 threads = ~6.4× node
     throughput but 2.5× *longer* wall-clock to the same nominal depth (the
     d32 tree grows 14×) — fixed-depth is not a comparable unit under SMP.
  3. **Highest engineering risk** — needs a race-tolerant (lockless XOR-key) TT,
     per-thread history/killers, a clean multi-thread stop, and forces
     **concurrency-1 gauntlets** (cc=4 would oversubscribe the CPU and make
     10+0.1 wall-clock meaningless) → ~4× throughput hit per box. Concurrency
     bugs are the worst class to debug — and #37 (a *single*-threaded desync) is
     still open.
- ~~**Decision:** revisit after the HCE roadmap is mined out (or alongside NNUE).~~
  **Superseded 2026-08-04** — the HCE roadmap was declared largely mined out at
  the v2.3 release, so this is now the active v2.4 workstream (see the phase
  ladder above). Catch 1 (buys Elo with hardware) is accepted as the point of
  the release; catch 2 is answered by the `Threads=1` bit-identity contract;
  catch 3 is what the phase ladder de-risks.

## Deferred / parked ideas

One-liners; full detail + evidence in the archives
([2.0](BACKLOG-archive-2.0.md) / [2.1](BACKLOG-archive-2.1.md) /
[2.2](BACKLOG-archive-2.2.md)).

- **#2 — King safety on top of mobility.** Shipped (t16) — see archive.
- **#3 — Continuation history.** 1-ply additive falsified (w16 neutral, w64 −9);
  flag off, kept in-tree (`experiment/continuation-history`).
- **#7 — LMP (late move pruning).** Deferred 2026-04, re-tested and
  **RE-PARKED 2026-07-14** (`candidate/lmp-r2` vs t34, two-machine pooled
  **−0.87 ± 10.33, LOS ≈ 46.6%** — AMD +4.17 ± 14.16 / Intel −6.60 ± 15.09, a
  sign-split that pools to dead-flat): the two-era verdict (−254→−56 vs t1;
  ≈0 vs t34 with the full t24–t33 ordering stack + the never-tested non-PV
  gate) shows ordering improvements converge LMP essentially to zero —
  Huginn's shallow tree is already thin without it. In-tree behind
  `ENABLE_LMP` (default OFF); a future revisit needs a genuinely new ordering
  signal (e.g. a working conthist redesign), not more threshold dialing.
- **#8 — Aspiration step (b), narrow-window search.** Folded into #17 and
  shipped as `#17-r2` (`baseline-t32`) — see archive-2.2.
- **#20 — Trapped-bishop eval pattern.** Tested 2026-07-03 (CPW locks,
  tuner-wired seeds 100/120 + 50/60) — **PARKED, clean two-machine neutral**
  (AMD +2.08/LOS 60.8%, Intel −5.21/LOS 24.7%, both noise-level, exactly the
  "~1–3 Elo, needs large N" outcome expected). Park-for-Texel-retune: a full
  re-tune with these params exposed is the better long-term path if revisited.
- **#26 — `board64[64]` piece-on-square cache.** Deferred (a prior sign-split;
  perf/cache changes carry more downside risk than gated eval terms).
- **#27 — Unorthodox early-queen PV** (d1d3 / d8d6). Deferred (evaluation).
- **#29 — Fifty-move-rule search blindness.** Direct terminal detection was
  kept on correctness; audit #53 (shipped) reopened and resolved the
  propagated-score / TT-contamination gap. P2 (clock-scaled eval) remains
  deferred (near-inert at 10+0.1).
- **#32 — PEXT slider attacks.** Tested 2026-07-03/04 — **PARKED, 3–5%
  SLOWER on the AMD 7800X3D/Zen4 box** (not the expected win; memory-latency-
  bound hot path, not ALU-bound). Bit-identical node counts confirm
  correctness. Infrastructure (build-gated PEXT + verified magic fallback)
  kept behind the flag for a future revisit on different hardware.
- **#42b — TT clusters (4-way, Fruit/Toga design).** Parked 2026-07-14 —
  two-TC flat (AMD blitz +0.69, Intel LTC −2.78) once idea 1 (TT aging)
  shipped; 4-way geometry adds nothing on top. In-tree behind
  `ENABLE_TT_CLUSTERS` (default OFF) for a post-NNUE revisit.
- **Drawishness scaling (`mul[]`)** — opposite-coloured bishops →½,
  pawnless-minor-up→⅛ (KNNK etc. left to the existing `MaterialDraw()`
  short-circuit). Tested 2026-07-03 — **PARKED, two-machine flat neutral vs
  t23** (AMD +1.04/LOS 55.4%, Intel +4.52/LOS 71.9%), exactly as predicted:
  self-play vs an equal-strength opponent is the least sensitive test for a
  #5 conversion-weakness fix. **Revisit vs a weaker external anchor** (Stash
  11/12, the #5 draw-prone pairing) rather than self-play if pursued further.

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
    (The 2026-07 pruning-stack audit — razoring, RFP, the #45 knobs — ran this
    exact play against t23; all four checked out sound or neutral. See archive.)
  - **Principle: fewer, better, sound > many, noisy, fighting.** "Demote breadth"
    (#41 + the MTLChess comparison, archive) is this gate applied.
- **Two-machine gauntlet workflow (#19, established):**
  `test_huginn_gauntlet.bat <tN|calib> [rounds] [cc]` auto-detects CPU vendor,
  tags PGNs `_intel`/`_amd`, runs SPRT [0,10] for baselines. Keep cc=4 on both
  boxes for poolable runs. Pentanomial pooling pairs games by `[Round]` tag.
- **Commit directly to `main`** (no feature branch unless asked); push only when
  asked. Gauntlet results: `gauntlet/*_<machine>.{log,pgn}` + a BACKLOG/commit
  summary carrying Elo/LOS/Ptnml.
- **Baselines are built per-machine** from the `baseline-tN` tag — never copy one
  box's binary to the other.
