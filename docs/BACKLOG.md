# BACKLOG (Huginn 2.1+)

> **Lean working backlog** — open and in-progress items only, plus a deferred-
> ideas list. Full closed-item history lives in two archives:
> [BACKLOG-archive-2.0.md](BACKLOG-archive-2.0.md) (2.0-era, issues #1–#37) and
> [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) (2.1/2.2-era, t16→t24: the
> #41 calibration study, #43 NMP round, #44 repetition fix, #45 futility fix +
> pruning-stack audit, #46–#50, and the 2026-07 SPRT queue that produced
> `baseline-t24`). Issue numbers are preserved so cross-references into either
> archive still resolve.
>
> **Current baseline + strength:** see [CLAUDE.md](CLAUDE.md) (top-level
> status) and [BASELINE_LADDER.md](BASELINE_LADDER.md) (full per-baseline
> writeup, newest first) — not duplicated here.

## Open / in-progress

| # | Title | Status | Type | Priority |
|---|-------|--------|------|----------|
| 9 / 35 | Texel eval program + tapered eval | **IN-PROGRESS, paused** — t10→t19 shipped (see archive); roadmap continues below | feature/eval | high |
| 37 | Board-desync illegal bestmove | **GUARDED + INSTRUMENTED**; root cause OPEN (needs repro) | bug | high |
| 42 | TT aging + clusters (Fruit/Toga design) | **INCONCLUSIVE** — idea 1 tested, weak positive lean, LTC check recommended; idea 2 (clusters) untried | feature/search | medium |
| 5 | Recalibrate vs external opponents (CCRL scale) | **OPEN** | maintenance | medium |
| 31 | TT-size (`Hash`) SPRT sweep | **OPEN** | tuning | low |
| 34 | Pin/blocker-aware legal movegen | **OPEN** | speed/research | low |
| 39 | NNUE evaluation | **DEFERRED** (HCE first) — big lever | feature/eval | — |
| 40 | Lazy SMP / multithreading | **DEFERRED** (HCE first) — big lever | feature/speed | — |

### #9 / #35: Eval program — Texel tuning + tapered eval (IN-PROGRESS, paused)

The main strength thread. A `game_phase_256` tapered eval foundation (#35) plus
a Texel tuner (`tools/texel/`, `bake.py` rewrites headers from the tuner dump,
verified exact) drive successive full-vector re-tunes / new-feature rounds on
the 725k Zurichess quiet-labeled corpus. Shipped ladder (full round-by-round
detail in [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md)):

- t10: tapered-eval foundation · t11: material+PSTs+king-EG · t12: tapered EG
  PSTs + tunable mobility · t13: connected+backward pawns · t14: rook-on-7th
  (sign-split exception) · t15: threats (largest eval-term ship) · t16: king
  safety (first KS ship) · t19: safe mobility (cross-machine-agreement ship)

Method note (learned across rounds): **new-feature MSE converts to Elo better
than re-fit MSE**; SPRT decides every round. King safety (t16) is the
exception that proves a corollary: quiet-corpus MSE UNDER-states terms that
pay in sharp positions.

**Paused since 2026-06-27:** the #45 pruning-stack audit (now closed, see
archive) redirected the active thread from eval to search-soundness, which
dwarfed any eval term in the same period (#45 +510, #47 +127 vs the ~+30 from
t16–t19 combined). Round 7+ HCE roadmap remains, priority order set by the
#41 calibration study (king safety → threats round 2 → safe mobility →
outposts; king safety, safe mobility, and outposts are done — see archive):

- **Threats round 2** — extend the +54 t15 cluster: hanging pieces (attacked
  *and* undefended), pawn-push threats, threat-by-king. Same machinery,
  proven. **Not yet attempted.**
- **Passed-pawn refinements** — king distance to the passer (own + enemy),
  blockade, rook-behind-passer. **Deprioritized**: #41 shows balanced-endgame
  play is already solid (fair-fight cp-loss 13.3). Not yet attempted.
- **Lower priority:** doubled-rooks / blind-pig follow-up to t14's
  rook-on-7th; space (safe squares behind own centre pawns); rook-on-king-file;
  specific endgame recognizers (KPK, KRKP).

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

### #42: TT aging + clusters (Fruit/Toga design) — INCONCLUSIVE (2026-07-03)

Huginn's TT (`transposition_table.hpp`) is single-entry-per-index,
depth-preferred replacement, no aging. It persists across searches
(`clearForSearch` clears the PV table, not the TT), so with depth-preferred-only
replacement a deep entry stored early in a game can squat in its slot
indefinitely — new shallow stores can't evict it (`depth >=` fails), crowding
out fresh results. Two portable ideas from the Fruit/Toga TT:

1. **Date-based aging** *(the main win)* — each entry carries a `date`; a
   global date bumps once per search; `age` = searches-elapsed. Replace by
   value `age*256 - depth` (prefer old + shallow). **Tested 2026-07-03**
   (`copilot/fix50-for-tt-aging`, `ENABLE_TT_AGING`): two-machine SPRT vs t23
   (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each): AMD **+0.69 ± 15.24**
   (LOS 53.56%, dead flat); Intel **+11.12 ± 15.01** (LOS 92.71%, real lean,
   undecided). Same-sign positive, but AMD too flat for clean cross-machine
   agreement (cf. t19's both-legs-above-+5 precedent). Behavior verified
   genuinely live (211/211 tests incl. 8 aging-specific cases; a direct
   multi-search-in-one-process A/B with a small hash showed the arms diverging
   from the second search onward). **Reading:** aging's value should
   concentrate in LONG games (many searches per game accumulate more staleness
   for it to fix) — blitz may genuinely understate it. **Next: one LTC leg
   (60+0.6, like the t21 validation) before a final park/ship call.**
2. **Clusters** — each index addresses an N-way contiguous cluster (probe
   scans for a lock match; store replaces the least-valuable slot). Fewer
   collision evictions → higher hit rate, at the cost of a small per-probe
   scan. **Untried.**

Pairs naturally with #31's Hash sweep (test aging + size together).

### #5: Recalibrate vs external opponents (OPEN)

Re-anchor the CCRL-Blitz-scale Elo estimate. Scripts: `test_huginn_gauntlet.bat
<snowy|cdrill|mtl03|mora|mtl05|mtl06>`; the 2.0 archive has the t11 3-anchor
MLE (~1818). Full round-robin + Stash-ladder calibration detail (round-robins,
findings, the score-compression/conversion-weakness lead) is in
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
- **Next:** re-run the Stash-anchored round-robin to full 2400g (prior run
  stalled at 2049g on a sleep interrupt) with sleep disabled (`powercfg /change
  standby-timeout-ac 0`) to tighten CIs; `stash14` (2060) optional for
  headroom. Then fold the refreshed estimate into CLAUDE.md.

### #31 / #34: Speed / research (OPEN, low)

- **#31** — SPRT sweep of `Hash` (64 vs 128 vs 256 MB) at the current
  strength. Pairs with #42's TT-aging test (test aging + size together).
- **#34** — SF-style `blockersForKing` pin/blocker-aware legal movegen (drop
  the per-move `MakeMove` legality filter for pinned-piece fast paths).

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
- **Hardware fit (2026-06-26):** this is also where the AMD box's **AVX-512** earns
  its keep — accumulator updates + int8/int16 dot products map to `AVX512-VNNI`
  (`VPDPBUSD`) for a big inference speedup. The current HCE engine doesn't vectorize
  (see the AVX-512 note under #32 in the archive), so NNUE is the project that
  justifies an AVX-512 code path. (Intel consumer 12–14th gen lacks AVX-512 →
  the fast path would be a build-gated AMD variant with an AVX2 fallback.)

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

One-liners; full detail + evidence in the archives
([2.0](BACKLOG-archive-2.0.md) / [2.1](BACKLOG-archive-2.1.md)).

- **#2 — King safety on top of mobility.** Shipped (t16) — see archive.
- **#3 — Continuation history.** 1-ply additive falsified (w16 neutral, w64 −9);
  flag off, kept in-tree (`experiment/continuation-history`).
- **#7 — LMP (late move pruning) fix.** Deferred (`experiment/lmp-quiet-count`).
- **#8 — Aspiration step (b), narrow-window search.** Deferred; folds into #17
  (rejected — see archive).
- **#20 — Trapped-bishop eval pattern.** Tested 2026-07-03 (CPW locks,
  tuner-wired seeds 100/120 + 50/60) — **PARKED, clean two-machine neutral**
  (AMD +2.08/LOS 60.8%, Intel −5.21/LOS 24.7%, both noise-level, exactly the
  "~1–3 Elo, needs large N" outcome expected). Park-for-Texel-retune: a full
  re-tune with these params exposed is the better long-term path if revisited.
- **#26 — `board64[64]` piece-on-square cache.** Deferred (a prior sign-split;
  perf/cache changes carry more downside risk than gated eval terms).
- **#27 — Unorthodox early-queen PV** (d1d3 / d8d6). Deferred (evaluation).
- **#29 — Fifty-move-rule search blindness.** P1 kept on correctness; P2
  (clock-scaled eval) deferred (near-inert at 10+0.1).
- **#32 — PEXT slider attacks.** Tested 2026-07-03/04 — **PARKED, 3–5%
  SLOWER on the AMD 7800X3D/Zen4 box** (not the expected win; memory-latency-
  bound hot path, not ALU-bound). Bit-identical node counts confirm
  correctness. Infrastructure (build-gated PEXT + verified magic fallback)
  kept behind the flag for a future revisit on different hardware.
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
