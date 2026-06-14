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
- **Baseline:** `baseline-t15 = cdcd31f` — round-6 threats (+54.2 ± 14.9 Elo
  pooled vs t14: AMD +50.26 / Intel +58.95, both LOS 100%) **plus the #37
  illegal-bestmove guard + #36 PV-display fix**. The largest eval-*term* ship
  of the program. Per-machine build from the tag; `huginn_t15.exe` snapshotted.
- **Architecture:** pure bitboard; magic-bitboard sliders; tapered eval
  (`game_phase_256`); Texel-tuned material/PSTs/mobility/pawn-structure/threats.
  ~3.55 Mnps single-thread.
- **Active thread:** the **#9 / #35 eval program** — round 7 is next.
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
| 9 / 35 | Texel eval program + tapered eval | **IN-PROGRESS** — t10→t15 shipped; round 7 next | feature/eval | high |
| 37 | Board-desync illegal bestmove | **GUARDED**; root cause OPEN (needs repro) | bug | high |
| 38 | Displayed PV continues past fifty-move rule | **OPEN** — cosmetic | bug | low |
| 5  | Recalibrate vs external opponents (CCRL scale) | **OPEN** | maintenance | medium |
| 17 | Aspiration-window widening on score swings | **OPEN** | feature | medium |
| 31 | TT-size (`Hash`) SPRT sweep | **OPEN** | tuning | low |
| 32 | PEXT slider attacks (build-gated) | **OPEN** | speed/research | low |
| 34 | Pin/blocker-aware legal movegen | **OPEN** | speed/research | low |
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

Method note (learned across rounds): **new-feature MSE converts to Elo better
than re-fit MSE** (re-fitting existing terms hit a floor at round 3, flat);
SPRT decides every round. Full round-by-round detail in the archive
(PRODUCTION TUNE 1–6).

**HCE roadmap (round 7+).** Remaining hand-crafted-eval levers, roughly by
Elo-per-effort. Add a new term → wire it into `collect_params()` → tune →
bake → two-machine SPRT. Each is its own round; pick by appetite for
risk-vs-yield.

- **Outposts** *(cleanest next win)* — knight/bishop on a hole the enemy can't
  hit with a pawn, supported by an own pawn. Constants (`KNIGHT_OUTPOST_BONUS`,
  the rank bounds) are already defined in `evaluation.hpp` but **dead/unused** —
  so it's part new-term, part cleanup. Cleanly tunable, low risk, ~+5–20.
- **King safety, reformulated for the tuner** *(marquee — biggest single gap)* —
  #2 is now unblocked (tapered base). The in-tree term is neutral and was
  *excluded* from tuning because the ≥2-attacker non-linear gate fires too
  rarely in quiet positions to constrain. Reformulate it more continuously
  (per-attacker-linear + shelter + safe-checks) so it fires often enough to
  Texel-fit. Highest ceiling (~+30–60 if cracked), highest tuning risk.
- **Threats round 2** — extend the +54 t15 cluster: hanging pieces (attacked
  *and* undefended), pawn-push threats, threat-by-king. Same machinery, proven.
- **Mobility refinement** — currently flat square-count × weight; go per-piece-
  type and restrict the area to *safe* squares (exclude enemy-pawn-attacked).
- **Passed-pawn refinements** — king distance to the passer (own + enemy),
  blockade, rook-behind-passer. Endgame Elo.
- **Drawishness scaling (`mul[]`)** — opposite-coloured bishops →½, KNNK→0,
  "a minor up, no pawns"→⅛. High impact, cheap, independent; kills the
  "scores a dead-drawn endgame as winning" class. **Not** sigmoid-tunable (it's
  a final-eval multiplier) — hand-set from known values + SPRT to confirm.
- **Lower priority:** doubled-rooks / blind-pig follow-up to t14's rook-on-7th;
  space (safe squares behind own centre pawns); rook-on-king-file; specific
  endgame recognizers (KPK, KRKP); trapped-bishop (#20, parked).

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

### #17: Aspiration-window widening (OPEN)

Replace the full-width root window with an aspiration window around the previous
iteration's score, widening on fail-high/low. Prereq groundwork (PVS, the
dormant fail-high break at root) is already in place. Related parked work:
#8 (aspiration step b).

### #31 / #32 / #34: Speed / research (OPEN, low)

- **#31** — SPRT sweep of `Hash` (64 vs 128 vs 256 MB) at the current strength.
- **#32** — PEXT slider attacks, build-gated with a magic fallback (BMI2 boxes).
- **#34** — SF-style `blockersForKing` pin/blocker-aware legal movegen (drop the
  per-move `MakeMove` legality filter for pinned-piece fast paths).

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
  Deferred (`experiment/trapped-bishop`).
- **#26 — `board64[64]` piece-on-square cache.** Deferred (a prior sign-split;
  perf/cache changes carry more downside risk than gated eval terms).
- **#27 — Unorthodox early-queen PV** (d1d3 / d8d6). Deferred (evaluation).
- **#29 — Fifty-move-rule search blindness.** P1 kept on correctness; P2
  (clock-scaled eval) deferred (near-inert at 10+0.1).

## Conventions

- **Ship bar:** same-sign two-machine SPRT H1-accept (AMD + Intel), or tight
  cross-machine agreement for a small effect. Sign-splits are parked by default
  (t14 rook-on-7th was a logged exception, not the rule).
- **Commit directly to `main`** (no feature branch unless asked); push only when
  asked. Gauntlet results: `gauntlet/*_<machine>.{log,pgn}` + a BACKLOG/commit
  summary carrying Elo/LOS/Ptnml.
- **Baselines are built per-machine** from the `baseline-tN` tag — never copy one
  box's binary to the other.
