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
| 64 | Full joint re-tune of the 841-vector (stale eval fit) | **OPEN (2026-08-05)** — eval params frozen since t30 `--only-new`; four selectivity ships landed underneath | feature/eval | high |
| 65 | Soft/hard time bounds (single-deadline TM) | **SHIPPED (2026-08-06)** — `ENABLE_SOFT_TIME_BOUND` default ON; +10.60 ± 10.57, LOS 97.54%, 2000g vs its own predecessor (fixed-N, SPRT did NOT reach the bound) | feature/search | high |
| 66 | Flat TB score = no progress gradient in won endgames | **OPEN (2026-08-05)** — WDL win returns a ply-independent 28000; engine shuffles at +280.00 with nothing to prefer a faster win | bug/search | medium |
| 68 | Gravity-bounded butterfly history | **PARKED FLAT (2026-08-07)** — −0.87 ± 17.08, 800g. Bounds magnitude, doesn't fix the one-sidedness | candidate/search | low |
| 69 | History malus only at beta cutoffs | **PARKED on cheap evidence (2026-08-07)** — +287% fixed-depth nodes, ordering collapses | candidate/search | low |
| 67 | Eval-discontinuity mining of match PGNs | **TOOLING (2026-08-05)** — triage front-end to the SF oracle; every gauntlet already carries the data | tooling/research | medium |
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

### #64: Full joint re-tune of the 841-vector — OPEN (2026-08-05)

**Origin: the Fable rematch diagnostic.** Fable 0.8 (a Rust engine built
against Huginn in two days, `Repos/Fable`) beats 2.3 by roughly +50 Elo. A
40-game match at 10+0.1, both single-threaded, plus the depth analysis below
localizes *where* that comes from — and it is not search.

Measured (`gauntlet/huginn23_vs_fable08_t1.pgn`, 40g, −52.51 ± 79.74, 42.50%):

| | depth/move | nodes/move | s/move |
|---|---|---|---|
| Fable 0.8 | 13.37 | **1.08 M** | 0.250 |
| Huginn 2.3 | 13.27 | **0.53 M** | 0.219 |

Head-to-head paired depth diff is **zero** (+0.22 / +0.03 / −0.07 ply at
filter caps 100/40/26 — sign flips, median +0.0 at every cut). ⚠ The naive
number is **+1.41 ply** and is wrong: Fable reports `depth 127` (its
`MAX_PLY`) on mate-terminated searches and never emits `seldepth` (`sd=0`
always). Any depth comparison against Fable must filter those; `tools/smp/
pgn_depth_compare.py` does not.

**So: Huginn reaches the same depth on half the nodes and still loses.** The
tree is strictly more efficient (matching the Kiwipete signature's 2.29×);
the deficit is leaf eval quality.

**The mechanism.** t30 threats were fit `--only-new` — six new params tuned,
the rest of the 841-vector deliberately **frozen** for clean SPRT attribution.
Correct for attribution, but the vector has had no joint re-tune underneath
the four selectivity ships that followed (t31 singular, t32 aspiration, t33
history-LMR, t34 TT aging), all of which reshape the tree the eval is fit
against. Fable ran two full 815-param joint re-tunes in two days and records
its threat fit (14/8, 15/2, −4/28) as diverging sharply from Huginn's (9/18,
10/4, −5/34) — engine-specific fit confirmed necessary by both labs
independently.

**Plan.** Full joint re-tune of all 841 params with the existing tuner (no new
code), then SPRT vs t34. Expect this to be the largest single eval lever
available; it is also the cheapest, since the tooling already exists.

### #65: Soft/hard time bounds — SHIPPED 2026-08-06

**Verdict (AMD box, 10+0.1, cc=4, 2000g, vs its OWN immediate predecessor —
same commit, `ENABLE_SOFT_TIME_BOUND=OFF`, Fathom matched):**

```
Elo: +10.60 +/- 10.57   nElo: 15.28 +/- 15.23   LOS: 97.54%
W501 L440 D1059   51.52%   DrawRatio 37.70%   PairsRatio 1.15
Ptnml(0-2): [48, 242, 377, 267, 66]
LLR: 1.70 (57.8%) of (-2.94, 2.94) [0.00, 10.00]
```

⚠ **Reported as a fixed-N positive, NOT an SPRT accept.** The LLR reached only
1.70 of the 2.94 bound before the 1000-round cap. That is not a failure of the
test: with bounds `[0, 10]` and a true value near 10.6, the hypothesis sits on
the H1 boundary, which is the slowest case an SPRT can be given. The 95% CI
`[0.03, 21.17]` excludes zero, but barely. Single machine — same
logged-exception class as t25 / t27 / t34 (user call), NOT two-machine
confirmed.

**It does mechanically what it was designed to do.** Measured against FableR
over 1000 games: clock utilisation **83.2% → 96.9%**, max/move **0.56 s →
1.67 s**, and median/move **down** 0.203 → 0.157 s — less on easy moves, far
more on hard ones. The head-to-head gap to FableR closed from ~49 to 35.6 Elo,
consistent with +10.6 plus FableR's own two commits.

Verification: fixed-depth signatures **bit-identical in both arms** (startpos
d14 = 3,481,582; Kiwipete d13 = 1,958,182), 315/315 tests, and on Kiwipete at
10+0.1 the candidate reaches depth 13 in 897 ms where the single-deadline arm
stops at depth 11 in 353 ms.

PGN: `gauntlet/softtime_vs_singledeadline.pgn`. Flip with
`-DENABLE_SOFT_TIME_BOUND=OFF`.

**Stress test — PASSED (2026-08-06, user-proposed).** The combination #65 had
never seen: **fast clock + SMP**. `softtime_t8` vs `singledeadline_t8`,
**5+0.05, `Threads=8` both sides, cc=1** (runbook §3 — `cc>1` with `Threads=8`
oversubscribes and makes wall-clock meaningless), 200g, TB both sides.

*Pass criterion was forfeits, not Elo* — 200g gives ±32, which cannot resolve a
~10 Elo effect, but it can resolve whether a 3× allowance flags under pressure.

- **Zero time forfeits, zero illegal moves, zero disconnects, zero crashes.**
  All 200 games ended on legitimate chess termination: 93 threefold, 52 Black
  mates, 46 White mates, 8 insufficient material, 1 stalemate. Warn-level log
  contains only startup INFO.
- Elo **+6.95 ± 32.38**, LOS 66.35%, 51.00% — same sign as the 10+0.1 result
  and its CI comfortably contains +10.60. No contradiction; no new information.
- Clock (budget ≈ 8.21 s at 64.2 moves): utilisation **85.1% → 97.7%**,
  max/move **0.28 s → 0.83 s**, median/move **down** 0.097 → 0.066 s.

Both maxima land exactly on the computed bounds — at 5+0.05, soft =
`5000/20 + 50/2` = **275 ms** (observed single-deadline max 0.28 s) and hard =
`min(3*275, safe_max, cap60, 2500)` = **825 ms** (observed candidate max
0.83 s). The formulas predict the measured behaviour to the millisecond at a
second, faster TC.

⚠ 97.7% utilisation is deliberately close to the edge. It is safe *by
construction* rather than by luck: a forfeit needs a single move to outlast the
remaining clock, and no move can exceed `time/2`. The guards scale with the
clock, so the margin holds as the clock drains. PGN:
`gauntlet/softtime_t8_stress_5p05.pgn`.

**Original analysis follows.**

### #65: Soft/hard time bounds — the measurement (2026-08-05)

Huginn has **one** deadline. [uci_utils.cpp:243](../src/uci_utils.cpp#L243)
computes `alloc = time_ms/20 + inc_ms/2`, and [uci.cpp:559](../src/uci.cpp#L559)
turns it into a single `info.stop_time` that aborts the search mid-iteration.

At 10+0.1 that is `10000/20 + 100/2` = **550 ms**, and across 2,455 observed
moves Huginn never once exceeded **0.56 s**. It is structurally incapable of
spending longer on a critical position, no matter how sharp.

Fable ([search.rs:156-160](../../Fable/src/search.rs#L156-L160)) splits it:

```
alloc = t/30 + inc*3/4        = 408 ms
soft  = min(alloc, t/2)       = 408 ms   -> do not START another iteration
hard  = min(alloc*3, (t-30)/2) = 1224 ms -> abort mid-search
```

Observed Fable max: **1.23 s**. Both formulas reproduce their engines' measured
maxima to the millisecond.

**The counter-intuitive part:** Huginn's base allocation is 35% *larger* than
Fable's soft bound (550 vs 408 ms), yet Huginn uses less total clock —
**13.42 s vs 15.41 s per game against a ~16.1 s budget (83.2% vs 95.5%)**. The
difference is structural, not generosity: Fable's soft bound only prevents
*starting* a new iteration, so an iteration already begun runs to 3× soft and
completes. Huginn kills it and plays the previous depth's move. Peak-to-mean
ratios: Fable 4.9×, Huginn 2.6×. The surplus concentrates in moves 1–9 (Fable
1.5–1.7× Huginn), converging to parity by move ~20.

**Plan.** Add a soft bound behind a flag: `soft` = current `alloc` checked at
the root iteration boundary; `hard` = `min(3*soft, safe_max, cap60, time/2)`
keeps the existing mid-search abort.

⚠ **The `time/2` clamp is load-bearing, not decoration.** `cap60` was never
binding pre-#65 (`time/20 + inc/2` cannot approach 60% of a normal clock), so
it only ever fired on near-zero clocks. A 3× allowance changes that:
`3*soft >= cap60` once `time <= 3.33*inc` — below ~333 ms at a 0.1 s increment
— which would let one move eat 60% of a scramble clock. Halving bounds the
cost of a bad estimate to a move rather than the game. Inert in normal play
(5000 ms vs a 1650 ms hard bound at 10+0.1). Fable uses `(t-30)/2` for exactly
this reason; **port the clamp, not just the multiplier.**

**Follow-up, gated separately (do NOT fold into this candidate — one
functional change per SPRT):** *best-move instability scaling.* Multiply
`soft` by ~1.3–1.8× when the root best move has changed across the last few
iterations, and shrink it when the same move has been stable for many — i.e.
explicit criticality detection layered on the implicit kind this candidate
provides. Both refinements above came from Fable's lab; they are on its own
candidate list too, so expect a race.

⚠ The current allocation is **not** arbitrary — [uci_utils.cpp:240](../src/uci_utils.cpp#L240)
records it as gauntleted at t21 for **+127 Elo**. But that ship fixed *time
forfeits* (a catastrophic bug); the `/20` divisor and the single-deadline
structure were never swept on their own. This changes the structure, not the
safety margin. Watch time-forfeit count as a hard gate on any candidate.

**Signature note:** a time-only change must leave fixed-depth node counts
**bit-identical** (startpos d14 = 3,481,582). That makes `signature.py` a free
and complete correctness pre-check here — unusually clean for a search change.

### #66: Flat TB score gives no progress gradient — OPEN (2026-08-05)

**Found by reading the #65 SPRT PGN** (`gauntlet/softtime_vs_singledeadline.pgn`,
moves 72–79 of a won ending; both arms are Huginn, so both probe TB).

`probe_tablebase_wdl` returns a **ply-independent** score:
[syzygy_tablebase.cpp:185](../src/syzygy_tablebase.cpp#L185) maps `TB_WIN` to
`MATE - 1000` = **28000** (UCI renders it `+280.00`), and
[search.cpp:2148](../src/search.cpp#L2148) returns it verbatim — no `- ply`.

So **every move that preserves the win scores exactly 28000.** There is no
gradient to prefer mating over shuffling:

```
72. Qd4+  +280.00/9        <- plateau
73. Nc3+  +280.00/8        <- plateau
74. Qd1+  +280.00/8  Kxf2  <- Black captures; score unmoved
75. Ne4+  +280.00/8  Kxe3  <- Black captures again; score unmoved
76. Qc2   +M7/10           <- mate enters the horizon: gradient appears
79. Qf2#                   <- finishes in three
```

Four moves of aimless checks while **shedding material to captures**, then an
immediate finish the moment a real mate score existed. It converted here, but
the failure modes are real: the **50-move rule** can draw a TB-won position
while the engine shuffles at +280.00, and enough material shedding drops the
position out of the TB win altogether.

**Why the two sides report differently** (the observation that surfaced this).
WDL tablebases are **distance-free by construction** — win/draw/loss, never
"win in N" — so the *winning* side cannot report a mate distance from a TB hit
at all; it must find the mate in its own tree. The *losing* side reports real
mate scores (`-M12`, `-M8`) at shallow depth because its tree is forced and
narrow: in check with a handful of legal replies, the whole sequence fits
inside the horizon. Verifying a mate against you is cheap; proving one across
all your own alternatives is expensive. This is NOT one side outsearching the
other, and it is not itself the bug — the flat plateau is.

**Fix (cheap, standard).** Make the TB score ply-sensitive exactly as mate
scores are:

```
TB_WIN  ->  MATE - 1000 - ply
TB_LOSS -> -MATE + 1000 + ply
```

Bound check: `ASPIRATION_MATE_BOUND` is 27000
([search.cpp:3308](../src/search.cpp#L3308)); even at ply 128 the score is
27872, still above it, so aspiration keeps treating TB scores as out-of-range
exactly as today. Does not interact with the deliberate "TB scores are never
stored in the TT" rule (#53 / rule50 keying) — that stays.

⚠ **Signature note:** unlike #65 this is NOT time-only. It changes returned
scores, so fixed-depth node counts WILL move on any TB-reachable position and
`signature.py` cannot be used as a pass/fail gate here. Gate on the TB tests
plus a direct A/B. Verification idea beyond SPRT: a won-endgame suite scored
on *moves-to-mate*, which measures the actual defect (Elo may barely move —
these positions are already won — while 50-move draws are the real leak).

**Correspondence note:** this is a Huginn-specific bug; FableR cannot have it,
having no tablebases at all. Reported to their lab in the interest of the
shared ledger, matching the two Fable bugs their letter received.

### #67: Eval-discontinuity mining of match PGNs — TOOLING (2026-08-05)

Every gauntlet PGN already carries both engines' per-move eval, depth, seldepth
and node count. Mining the **eval step functions** — the plies where a game
goes from near-equal to decided — locates the moments games were actually lost,
and (uniquely) says *which side was blind and for how long*.

Worked example that motivated this (`softtime_vs_singledeadline.pgn`, evals
normalized to White's POV):

| ply | White | Black | reading |
|---|---|---|---|
| 32 | −0.03 (d11) | −0.18 (d9) | agree, near equal |
| 33 | **+1.17** (d11) | +0.07 (d9) | **disagree by 1.10** |
| 34 | +0.97 (d11) | +1.17 (d10) | Black finally agrees |

The step lands on White's `gxf5`; **Black's eval lagged two plies**, still
calling it equal one ply after the position had turned. Note Black was
searching **2 plies shallower at every one of those moves**. If "the side that
eats the swing is the side searching shallower" holds across a corpus, that is
direct evidence for the depth→strength chain #65 rests on — a mechanism, not
just an Elo delta.

**Signals to extract**
1. Δeval per ply per engine (White-normalized); flag `|Δ| >= ~0.75`.
2. Cross-engine disagreement — adjacent-ply evals should roughly negate; a
   persistent gap says one side is blind and which.
3. **Lag** — plies until the overturned side's own eval catches up. Long lag =
   deep structural blindness; this is exactly the #52 check-blind-qsearch
   signature.
4. Attribution + persistence — swings that revert are search instability;
   swings that persist to the result are real. Bucket by move number and arm.

⚠ **Mate/TB scores must be clamped before differencing.** `+M7` and the
`+280.00` TB verdict (#66) otherwise manufacture enormous fake swings and
dominate every statistic.

⚠ **An engine's own eval is not ground truth.** A step can be a genuine
blunder, ordinary horizon resolution (nobody erred), an eval bug, or
aspiration/TT instability — indistinguishable from the PGN alone. So this is
**not a verdict machine**: it is a triage front-end to the oracle already in
`tools/analyze_played_vs_engines.py`, which sweeps ~6,837 positions at SF depth
22 in 4–4.5 h. Mining first cuts that to the ~50–100 positions that decided
games. Precedent: `tools/SF_winning_Huginn_losing.csv`.

**Two distinct payoffs.** On a *self-play A/B* (#65) both arms are near-
identical, so asymmetry in who commits the swings is attributable to the change
alone — the mechanism check Elo cannot give. On a *cross-engine* match
(FableR) the clustering of swings says which eval terms Huginn misprices,
making the #64 thesis concrete instead of assumed.

This is the "audit structures, not deltas" method that produced this ladder's
largest gains (#44, #45, #52) — and it comes free with every gauntlet already
run. Tool: `tools/pgn_swing_mine.py`.

### #68 / #69: butterfly-history rework — the measured defect, and why the
### obvious fixes made it worse (2026-08-07)

**Origin.** FableR shipped gravity/malus history for **+129.8 ± 30.5**, its
largest single gain ever, off our own per-move diagnosis. That prompted the
question: does Huginn have the same headroom?

**Huginn already has maluses** ([search.cpp](../src/search.cpp)
`penalize_search_history`), so that gain does not transfer wholesale. But the
*formulation* differs in two ways, and one of them turns out to be a real,
measured defect.

#### The defect: #63's LMR modulation is effectively one-directional

`search_history` accumulates raw ±depth² with no bound, aged only ×7/8 every
third ID depth. The **bonus** fires only when a quiet improves alpha; the
**malus** fires on every quiet that does not — which in an all-node is every
quiet there is. Bonuses are rare, maluses ubiquitous, nothing bounds either.

Measured (`test/test_history_saturation.cpp`, one search each):

| position | max | min | > +4096 | < −4096 |
|---|---|---|---|---|
| startpos d12 | +18,758 | −66,522 | **3** | **119** |
| Kiwipete d12 | +1,929 | −125,564 | **0** | **138** |
| Kiwipete d14 | +15,613 | −510,135 | **6** | **269** |

`#63` thresholds these against a **fixed** `HISTORY_LMR_GRAIN = 4096`. Entries
below −grain outnumber those above by ~40×, so the "proven-good quiet, reduce
one ply LESS" half of the feature almost never fires while its counterpart
fires constantly. **Half of a shipped feature is dormant.** That is a genuine
find of the #44/#45/#52 latent-bug class.

⚠ An earlier draft of the test asserted the positive half was *unreachable*.
The test failed and falsified it — 3, 0 and 6 entries do clear +grain. It now
asserts the **ratio**, not an absolute. Claim trimmed to what was measured.

#### Both obvious fixes made the tree dramatically worse

| arm | startpos d14 nodes | vs 2.4 |
|---|---|---|
| 2.4 baseline | 3,481,582 | — |
| **#68** gravity-bounded (`±HISTORY_MAX` = 16384) | 4,878,930 | **+40%** |
| **#69** malus only to quiets tried before a cutoff | 13,491,453 | **+287%** |
| both | 9,064,266 | +160% |

- **#68 bounds magnitude but not the asymmetry.** After the change min pins at
  exactly −16384 and max falls to +5,240, yet `>+grain` vs `<−grain` is still
  2/115, 0/133, 4/237. The imbalance is set by *when* updates fire, which
  gravity does not touch. A first draft of the source comment claimed gravity
  would "largely cancel the frequency asymmetry"; re-measuring refuted it and
  the comment now says so.
  Plausible cost: ordering uses raw history as its score, so unbounded values
  carried fine-grained discrimination among bad quiets that ±16384 compresses.
- **#69 is decisively bad.** Removing the broad malus leaves most quiets near
  zero, so both quiet ordering *and* #63's "reduce more" branch lose their
  signal — a 3.9× tree. **Parked on cheap evidence; no gauntlet spent.**

**The real lesson: Huginn's unconventional scheme is load-bearing.** The broad
all-node malus looks wrong by textbook standards and is what makes both the
quiet ordering and #63's negative branch work at all. FableR's +129.8 came from
having *no* malus to having one; Huginn is already past that point, on a
different local optimum.

**#68 GAUNTLETED — flat, parked (2026-08-07).** `huginn_hist68` vs 2.4,
10+0.1, cc=4, 800g: **−0.87 ± 17.08**, LOS 46.03%, 49.88%, LLR −0.41 (no
bound). Neither better nor worse.

⚠ **And the +40% fixed-depth node inflation cost nothing measurable.** That is
now the third data point that node count at fixed depth is a poor strength
predictor on this ladder — t27 shipped at **+12.8% nodes and stronger**, #68 is
+40% and flat. Fixed-depth nodes are a *signature* (bit-identity gate) and a
*mechanism* probe; they are not an Elo proxy, and a candidate should not be
parked on node count alone. #69's +287% remains a reasonable cheap park because
of its magnitude and because its mechanism is understood, but that judgement is
weaker than it looked before this result.

**Untested alternative, cheaper than either:** leave the distribution alone and
re-scale `HISTORY_LMR_GRAIN` (or make it relative to the observed spread) so
#63's positive branch can fire against the distribution that actually exists.
That treats the measured defect without disturbing the ordering signal.

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

#### Training data: the existing PGN archive is NOT a training corpus (2026-08-05)

Measured, so the decision is not a rule of thumb. `gauntlet/` holds **102
PGNs, 73,797 games, 235 MB ≈ ~8M raw plies** — and `tools/texel/extract_fens.py`
already distils all of it to `fens_gauntlet.txt`: **19,825 positions**. That
ratio is the whole answer. Three reasons, worst first:

1. **Effective diversity, not raw volume.** Successive plies of a game are
   near-duplicates; every game starts from `noob_3moves.epd` (~3 moves of
   opening variety); and much of the archive is *near-identical engine
   versions* playing near-identical positions — that is what an A/B IS. NNUE
   wants ~1e7 positions minimum, ~1e8 to be competitive. We are 1–2 orders
   short on count and worse on independent information.
2. **Wrong label type.** PGN evals come from a **time-limited** search at
   wildly varying depth (8 → 25+ within one game), so identical positions get
   inconsistent labels — that teaches the net the variance of our time
   management, not chess. Training wants a *consistent* budget (fixed nodes or
   fixed depth). Mate scores and the `#66` `+280.00` TB verdicts must also be
   stripped.
3. **On-policy coverage only.** These are positions Huginn *chose* to enter. A
   net trained on them never learns to evaluate what Huginn currently avoids —
   which is exactly where the eval is wrong. Real generators inject random
   opening plies specifically to escape the engine's own preferences.

**Keep the archive** — it is a good *held-out validation set* (real outcomes,
never trained on), the `K` sigmoid-fit corpus, and the #67 diagnostic input.
`extract_fens.py` is reusable plumbing; only scale and label type are wrong.

**#40 IS A PREREQUISITE FOR #39.** Data generation is embarrassingly parallel.
At ~3.5 Mnps and 5k nodes/position: ~700 positions/sec/thread → **1e8 positions
≈ 40 h single-threaded, ≈ 5 h at `Threads=8`.** The v2.4-SMP-then-v3.0-NNUE
roadmap ordering is therefore load-bearing, not just sequencing.

**Generation must be chunked and resumable — 6-hour sessions, kicked off at
night, checked in the morning** (user requirement, 2026-08-05). Design
consequences for the `gensfen`-equivalent, all cheaper to build in than to
retrofit:
- **Wall-clock budget flag** (`--max-hours 6`) that finishes the current game
  and exits *cleanly*. Do not rely on killing the process — a kill mid-write
  truncates the output.
- **Append-only output + a manifest** (positions written, sessions run, seeds
  used) so a new session continues instead of restarting. Same resumable
  pattern `run_cc_analysis.bat` already uses for the SF oracle sweep.
- **Distinct seed per session**, recorded in the manifest. Without this, seven
  nights produce one night of unique data seven times — the single most
  expensive mistake available here.
- **Dedup across sessions** (position hash); opening randomisation will collide
  near the root.
- Note: at `Threads=8` the full 1e8 fits in ONE ~5 h overnight window, so the
  chunked path matters mainly if generation runs single-threaded, if the target
  grows, or if the box is shared with gauntlets (it usually is).

**SEPARATE POSITION GENERATION FROM LABELLING.** The two halves have completely
different reuse lifetimes, and fusing them throws away the expensive half:

| component | engine-specific? | reuse |
|---|---|---|
| positions (FEN) | no | universal — across engines AND across our own versions |
| game result (WDL) | no | reusable; quality scales with generator strength |
| eval score | **yes** | scale + search-coupling are ours; re-labelling is cheap |

Eval is engine-specific in two ways: **scale** (our `+1.00` != another engine's;
this is what the Texel `K` sigmoid fit calibrates) and **search coupling** —
RFP / futility / razoring margins are denominated in HCE centipawns, so a net
trained to a foreign scale silently miscalibrates every pruning margin. FableR
confirmed the same principle from the other side: its threat fit (14/8, 15/2,
−4/28) diverged sharply from ours (9/18, 10/4, −5/34) on identical terms,
because each was fit against its own search.

Consequences:
- Store positions/results and eval labels in **separate, joinable** files
  (position hash as key). Re-labelling then costs a pass, not a regeneration.
- This is what makes **iterative training** affordable — the standard NNUE loop
  is: train net v1 on HCE labels, re-label the SAME corpus with net v1 + search,
  train v2, repeat. Impossible to do cheaply if labels are baked into the
  position file.
- It also survives **#64**: a full 841-vector re-tune changes our eval, which
  invalidates *labels* but not *positions*. Fusing them would mean regenerating
  40 h of corpus for an eval change.
- Cross-engine data reuse is standard practice (Leela data trains many engines;
  public SF binpacks are widely consumed). A stronger teacher's evals beat
  self-labelling — that is distillation.

**DECIDED (user, 2026-08-05): Stockfish is an acceptable teacher.** The
project-values question above is settled — importing a stronger engine's
evaluation knowledge is in scope. Same call covers open co-development with
FableR (shared cooperation/competition), so a **shared position corpus between
the two engines is viable**: positions are engine-agnostic, and only labels are
ours.

⚠ **This may delete the 40 h generation step entirely — check before running
it.** If a Stockfish/Leela teacher is acceptable, the fastest path is not to
generate at all but to consume an existing **public pre-labelled dataset**
(SF binpacks, Leela data), which are already 1e9+ positions, already labelled
by a far stronger teacher, and already diversity-filtered. Cost/benefit before
committing overnight runs:

| path | cost | label quality | notes |
|---|---|---|---|
| public dataset | download | **best** (SF/Leela) | needs a binpack/`.bin` reader; format is tied to the trainer |
| SF-labelled own positions | generation + SF pass | very good | keeps our position distribution |
| self-labelled (HCE) | ~40 h / ~5 h at t8 | capped near our own ceiling | fully ours; the from-scratch path |

**Licensing — checked, and we are clear (2026-08-05).** Attribution is the
user's stated condition ("as long as we credit whatever external source we
use"), but for NNUE data the binding constraint is the *license*, not the
attribution convention. Stockfish is **GPLv3**, and the SF project's position
is that nets trained on SF-generated data are derivative works that must also
be GPLv3 — the point actually litigated in the ChessBase dispute (settled
2022). Lc0 is GPLv3 as well.

**Huginn already ships GPLv3** (`LICENSE`), so consuming SF- or Leela-derived
data or nets creates **no copyleft conflict**. This would have been a genuine
blocker under MIT/Apache. Requirements reduce to: record provenance in-repo —
dataset name, version/URL, and the net/teacher used — and keep the GPLv3
notice intact. (Not legal advice; this is the community-standard reading.)

⚠ **FableR should check its own license before consuming the same data.** It
has **no LICENSE file** as of today (`Repos/Fable`: no LICENSE, no `license`
field in `Cargo.toml`), which defaults to all-rights-reserved. That is fine
for a private lab engine, but it matters for two things now on the table: the
shared position corpus above, and publishing to GitHub (the rename to FableR
implies intent to publish). If FableR ingests SF-derived labels, it inherits
GPLv3 obligations. Flagged for their lab in the correspondence — cheaper to
choose a license now than to relicense after a net ships.

Practical route for a first net: public data + an existing trainer (`bullet` is
the common Rust one) gets a working net fastest, and the engine-side work
(quantisation, incremental accumulator updates in make/unmake, AVX-512 path) is
the same regardless of data source — that integration, not the data, is the
real #39 lift. Revisit self-generation only if the position distribution turns
out to matter (our search coupling) or for later self-play iterations.

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
- **Hash sweep at `Threads=8` — MEASURED 2026-08-06: the 64 MB default is
  inadequate at t8.** Not yet gauntleted for Elo, but the mechanism is
  established and it already changes the release recommendation.

  `hashfull` (permille) over a **continuous game** — no `ucinewgame` between
  moves, which is how the TT really behaves (#46 clears only on `ucinewgame`):

  | regime | Hash | move 1 → 15 |
  |---|---|---|
  | blitz 550 ms, **t1** | 64 MB | 14 → 272 (never fills — `#31`'s null was right) |
  | blitz 550 ms, **t8** | 64 MB | 126 → **779**, still climbing |
  | blitz 550 ms, t8 | 256 MB | 18 → 299 |
  | LTC 3000 ms, t8 | 64 MB | 414 → **998 by move 7**, pegged thereafter |
  | LTC 3000 ms, t8 | 256 MB | 177 → 742 |

  ⚠ **Our own SMP legs were measured under TT starvation.** AMD +199.05 and
  Intel +172.78 both ran at 10+0.1 / `Threads=8` / 64 MB — i.e. against a table
  ~78% loaded by move 15. Whatever the true t8 gain is, it was measured on a
  starved table; a re-measure at adequate Hash can only be read as a *different*
  configuration, not as a correction to those numbers.

  **Methodology warning, the real lesson here.** The first instrument
  (`tools/smp/hash_ab.py`, 40 positions, `ucinewgame` per position) reported
  **flat**: 64/128/256/1024 MB all within ±0.25 ply of each other at t8,
  every delta smaller than its own 95% CI. That was a true measurement of
  *within-search* pressure and a completely misleading answer to the question
  asked, because it resets the table before every position. Real games
  accumulate. The tool documents this limitation — and the limitation turned
  out to be the entire result. **A hash sweep must be measured across a
  continuous game, never per-position.**

  **GAUNTLETED 2026-08-06 — and the saturation inference was WRONG. Default
  stays 64 MB.**

  `Hash=128` vs `Hash=64`, same binary both sides, `Threads=8`, 10+0.1, cc=1,
  500g: **−18.08 ± 21.85**, LOS **5.18%**, 47.40%, zero anomalies. The CI
  includes zero, so this is not proof 128 MB is worse — but there is no case
  whatsoever for raising the default, and a ~95% lean that it would hurt.

  **Measured mechanism (this is the part that survives).** Raw throughput at
  `Threads=8`, movetime 3000, median NPS of 3:

  | Hash | NPS | vs 64 MB |
  |---|---|---|
  | 64 MB | 13.52 M | — |
  | 128 MB | 13.15 M | **−2.7%** |
  | 256 MB | 12.90 M | −4.6% |
  | 1024 MB | 12.87 M | −4.8% |

  Bigger tables cost throughput and the cost plateaus near −4.7% — the shape of
  falling out of cache. This box is a **Ryzen 7 7800X3D with 96 MB L3
  V-Cache**: a 64 MB TT largely fits, 128 MB does not. `#31`'s own runner
  header predicted exactly this ("the AMD 7800X3D's 96 MB L3 V-Cache holds a
  meaningful fraction of a small TT, the Intel box's L3 does not, so the
  optimal size may differ per machine").

  ⚠ **Therefore this result is machine-specific and must not be generalised.**
  On a normal L3 (~30 MB, the Intel box) 64 MB is *already* out of cache, so
  the throughput cliff sits elsewhere and more Hash may well help. An Intel leg
  is the one genuinely informative follow-up; until then the release should
  document the trade-off rather than prescribe a size.

  **Lesson: mechanism is not Elo.** The saturation data was real and correctly
  measured — 64 MB pegs at 998‰ by move 7 at LTC/t8 — and the inference drawn
  from it ("therefore raise the Hash") was still wrong. A full table with TT
  aging (`#34`) evicts stale entries perfectly well; the collision cost it
  implies never showed up, while the cache cost of avoiding it did. Same family
  as `#53` (correct ≠ Elo-positive). This is precisely why a *default* must not
  be changed on a mechanism argument alone.

  **#31 stays closed, now for both thread counts on this box.** PGN:
  `gauntlet/hash128_vs_hash64_t8.pgn`.
- **Scaling curve (t2, t4 vs t1).** Elo per thread count instead of the single
  t8 point. ~3h per rung at cc=1. Depth data predicts ≈ +0.72 / +1.25 ply.
- ~~**Intel leg.**~~ **DONE** (`d5f9c5b`): `smp_t8` vs 2.3, **+172.78 ± 25.51**,
  400g. Cross-machine agreement holds in sign and rough magnitude against AMD's
  +199.05 ± 25.63 — which is the signal to look for; per §6 of the runbook the
  two legs are deliberately **not pooled**, since `Threads=8` is physically a
  different workload on different silicon.

**Also verified (2026-08-06): #65 soft/hard time bounds behave under SMP.**
`soft_stop_time` is deliberately NOT propagated to helpers — the soft bound is
a driver-only decision, and the driver's return fires `group_stop_` immediately.
Kiwipete @ 10+0.1 (soft 550 ms / hard 1650 ms): `Threads=1` → d13 @ 945 ms,
`Threads=2` → d13 @ 906 ms, `Threads=8` → d14 @ 816 ms. Time stays bounded as
threads rise and t8 buys a ply; nothing runs away to the hard deadline. Covered
by `SmpPoolTest.HelpersContributeUnderALiveSoftTimeBound`, which asserts the
coarse property the other pool tests cannot (they are all depth-limited, so a
time-handling change that crippled helpers under a clock would pass every one
of them). ⚠ Scope stated honestly: propagating the field would *not* strand
helpers for long — each would stop at its own next iteration boundary past the
soft bound, near where the driver stops anyway. It stays driver-only for the
semantics, not because the alternative is catastrophic.

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
