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
- See [CLAUDE.md](CLAUDE.md) for the full baseline ladder and
  [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for the live search/eval inventory.

## Open / in-progress

| # | Title | Status | Type | Priority |
|---|-------|--------|------|----------|
| 9 / 35 | Texel eval program + tapered eval | **IN-PROGRESS** — t10→t15 shipped; round 7 next | feature/eval | high |
| 37 | Board-desync illegal bestmove | **GUARDED**; root cause OPEN (needs repro) | bug | high |
| 5  | Recalibrate vs external opponents (CCRL scale) | **OPEN** | maintenance | medium |
| 17 | Aspiration-window widening on score swings | **OPEN** | feature | medium |
| 31 | TT-size (`Hash`) SPRT sweep | **OPEN** | tuning | low |
| 32 | PEXT slider attacks (build-gated) | **OPEN** | speed/research | low |
| 34 | Pin/blocker-aware legal movegen | **OPEN** | speed/research | low |
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

**Round 7 candidates:** king-safety weights via the harness (the non-linear
term hand-tuning couldn't crack — #2 is now unblocked since the base is
tapered), doubled rooks / blind-pig follow-up to t14, mobility re-fit on fresh
self-play data from the stronger engine, or a re-tune on a larger corpus.
Method note (learned across rounds): **new-feature MSE converts to Elo better
than re-fit MSE**; SPRT decides every round. Full round-by-round detail in the
archive (PRODUCTION TUNE 1–6).

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

### #5: Recalibrate vs external opponents (OPEN)

Re-anchor the CCRL-Blitz-scale Elo estimate now that the eval program has added
~+150 Elo since the last calibration (t11 ≈ 1818). Calibration ladder + scripts
(`test_huginn_gauntlet.bat <snowy|cdrill|mtl03|mora|mtl05>`) in the archive (#5).

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
