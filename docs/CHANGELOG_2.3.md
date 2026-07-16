# Huginn 2.3 — Changelog (2.2 → 2.3)

**Released 2026-07-16.** Tagged `v2.3`; the release binary is snapshotted
separately per the usual per-machine convention.

2.3 = 2.2 content (`baseline-t21`) + thirteen baselines (t22–t34) + the
road-to-v2.3 item-5 UCI hygiene changes (always-visible position-rejection
diagnostics, `parse_go_command` extraction) — both search-behavior-
identical to `t34` (verified byte-for-byte: startpos d14 = 3,481,582 / cp
31 / e2e4, unchanged) — plus the `id name Huginn 2.3` version bump.

## Headline

- **Thirteen baselines shipped, t22 → t34** — an informal running sum of
  each individually-SPRT'd blitz delta (10+0.1, two-machine where run) is
  **≈ +275 Elo** (t22 through t33; NOT a single controlled multi-step
  test — each step was gauntleted against its immediate predecessor, not
  against t21 directly, so treat this as an order-of-magnitude estimate,
  the same caveat 2.2's changelog used for its own summed deltas).
  `baseline-t34` (TT aging) shipped separately on an **LTC-only** verdict
  (60+0.6, +15.99 ± 17.00) after a flat/inconclusive blitz leg, so it's
  kept out of the blitz sum above — different time control, not
  comparable.
- **External calibration: ~2571 ± 19 CCRL-blitz at `t26`** (measured
  2026-07-11, Stash 19.0/20.0.1 anchors) → **~2600–2680 (pooled ~2625 ±
  18) at `t34`/pre-2.3** (measured 2026-07-16, Stash 19.0/20.0.1/21.0 —
  full numbers, and why this is a range and not a point,
  in [BASELINE_LADDER.md](BASELINE_LADDER.md)). Net gain **≈ +33 to +54**,
  well under the naive ~+90 the t27→t34 self-play addend (≈ +122) would
  suggest at t26's ~0.86 compression — this ladder's selectivity-heavy
  gains compressed harder (~0.3–0.4) going external. The weak/strong
  anchor split (19/20 agree within 2 Elo; 21 pins ~72 Elo higher) also
  **reproduces BACKLOG #5's 2026-06-15 "score compression" finding** — the
  per-anchor pin climbing with opponent strength — on an entirely
  different, higher-strength anchor pair. **Next recalibration: drop
  Stash 19, use 20/21/21.2.**
- **Mix shifted from 2.2's "three latent bugs" story to a more even split**:
  three correctness fixes (`#50` Zobrist OOB, `#51` history-index collision,
  `#59` EP-key semantics), one bundled audit pair (`#52`/`#53`), one pure
  speed win (`#48`/`#49`), one SPRT-queue pair (`#6`/`#44` follow-up), and
  **the SF18-gap-study selectivity program** — four genuine search-shape
  features in a row (`#57` legal-move ordinal, `#62` singular extensions,
  `#17-r2` aspiration re-ship, `#63` history-modulated LMR) plus `#58`
  (SEE pin legality) — against exactly **one** new eval term (`#9` threats
  round 2). Corroborated three separate ways this session (see below).

## The story

Where 2.1→2.2 was three bugs hiding in plain sight, 2.2→2.3 is dominated by
**closing the effective-branching-factor gap** the
[SF18_GAP_STUDY.md](SF18_GAP_STUDY.md) measured (Huginn ~1.90/ply vs SF18's
~1.37) — extensions, aspiration, and history-modulated reductions, the
classic selectivity trio, each shipping on two-machine same-sign agreement.
Alongside it, three more of the same latent-bug species as 2.2's `#44`/`#45`
turned up and got fixed unconditionally (`#50`, `#51`, `#59`) — this ladder
is not yet done finding them.

**Corroborating evidence gathered this session (road-to-v2.3 item 5,
2026-07-15), independent of the SPRT ladder above:**

- **WAC300 tactical-solving: 274/300 (91.3%, 2026-06-03 `t9`/`t10`-era) →
  294/300 (98.0%, `t34`)** — 21 of 26 old failures fixed, 1 new miss
  investigated and found to be a dual-solution WAC-300 entry, not a
  regression. **LCT2 (Louguet Chess Test II) run for the first time: 21/35
  (60.0%)**, refined to ~22/35 via a Stockfish depth-18 cross-check (one
  dual-solution, two noise-level, 11 genuine misses banked as a future
  reference pool). Full writeup:
  [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md#wac300-tactical-solving-snapshot-2026-07-15-baseline-t34--current).
- **`#41` played-game re-diagnosis vs the real `t21` (2.2) baseline**, 6649
  correspondence-game positions vs Stockfish 17.1: SF move-match
  48.5%→51.8%, fair-fight cp-loss 15.6→12.3, middlegame over-optimism
  29.0→22.7cp — **and this time with a genuine +3.1 ply depth increase**
  (vs the t15→t21 leg's near-flat +0.6 ply), i.e. the selectivity stack is
  buying real additional depth at fixed time, not just recovering
  blind spots. [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md).
- **Test infrastructure hardened** (`#60` leftovers): UCI go/position
  parser tests now assert exact results instead of `EXPECT_NO_THROW`
  (caught the class of bug that let a malformed EPD castling flag go
  undetected — see below); new randomized make/unmake/null-move invariant
  tests (5 seed positions × 200 walks × 40 plies). 295/295 tests green.
- **Found + fixed along the way**: a malformed castling flag in
  `lct2.epd` was silently rejected by `validate_uci_position` (correct,
  per `#54`'s transactional design) with zero visible signal, because the
  rejection diagnostic was gated behind `debug_mode`. Fixed the
  observability gap too — `uci.cpp`'s position-rejection messages always
  print as `info string` now, not just under `debug_mode`.

## Per-baseline

| Tag | Change | Result vs prior |
|-----|--------|-----------------|
| t22 | **Speed pair** — kill the double TT probe (`#48`) + fuse king-safety attacker scan into mobility (`#49`), both behavior-identical (+11.0% nps startpos d15) | Pooled +16.3 (52.35%/2000g), both legs positive |
| t23 | **`#50` Zobrist black-king-row OOB fix** — `Piece[12][64]` was one row short of the 13-row scheme; the black king's row read 64 `U64`s past the array (1 phantom slot hit ASLR heap noise, 5 more silently zeroed → real TT key collisions), carried by every baseline ever shipped | AMD +33.97 ± 16.60, LOS 100% (872g) — single-machine decisive freeze |
| t24 | **SPRT-queue winners** — SEE good/bad capture ordering split (`#6`) + root two-fold draw-avoidance (`#44` follow-up), combined | AMD +48.84 ± 20.36 / Intel +66.33 ± 23.61, both H1-accept |
| t25 | **`#51` history-heuristic piece-index collision fix** — `% 13` on the packed `Piece` enum folded `BlackKing=14` into `WhitePawn`'s row, same species as `#50` | AMD +19.48 ± 15.00, LOS 99.46% — single-machine accept |
| t26 | **2026-07-09 audit criticals** — check-aware quiescence (`#52`, the Elo carrier) + rule-50-aware TT eligibility (`#53`, correctness ship) | Pooled ≈ +42 Elo (56.05%/1306g), both legs H1-accept |
| t27 | **`#57` legal-move ordinal for PVS/LMR + textbook PVS condition** — pseudo-legal loop index had been driving first-move treatment and LMR lateness | AMD +29.98 ± 15.53, LOS 99.99% — single-machine accept |
| t28 | **`#58` pin-aware first recapture in SEE** — pinned defenders no longer make winning captures look losing to qsearch's hard prune | Pooled +7.2 ± 10.5, LOS ≈ 91% |
| t29 | **`#59` en-passant key semantics** — EP right normalized at the source (MakeMove + set_from_fen), fixing missed threefolds and decorative-EP TT splits | AMD regression gate +8.34 ± 15.32, LOS 85.73% — clean gate + correctness ship |
| t30 | **`#9` threats round 2** — hanging units, safe pawn-push threats, hanging units in the king's ring (Texel `--only-new` fit) — the one new eval term this ladder | Pooled +17.03 ± 10.40, LOS ≈ 99.9% |
| t31 | **`#62` singular extensions** — the SF18-gap-study EBF lever: reduced-depth exclusion search extends the TT move one ply when singular | Pooled +14.90 ± 10.62, LOS ≈ 99.7% |
| t32 | **`#17-r2` aspiration windows at the root (re-ship)** — attempt 1 was H0-rejected at t15 before the soundness fixes above; re-tested clean | Pooled +14.46 ± 10.61, LOS ≈ 99.6% |
| t33 | **`#63` history-modulated LMR** — butterfly-history score adjusts the static reduction table by ±1 ply | Pooled +13.63 ± 10.72, LOS ≈ 99.4% |
| t34 | **`#42` idea 1: date-based TT aging** — 6-bit search date fixes the depth-preferred-only squatting problem; **first LTC-verdict ship** (blitz flat/inconclusive, LTC positive) | Intel LTC +15.99 ± 17.00, LOS 96.77% (60+0.6, 500g) |

## Notes

- No architectural change — still pure-bitboard hand-crafted eval,
  single-thread. `2.4` is reserved for Lazy SMP (`#40`), `3.0` for NNUE
  (`#39`); see the version roadmap in
  [BACKLOG.md](BACKLOG.md#version-roadmap) (full road-to-v2.3 checklist
  detail archived in
  [BACKLOG-archive-2.2.md](BACKLOG-archive-2.2.md#road-to-v23-release-checklist--complete)).
- 295/295 GoogleTest cases pass (1 by-design skip).
- `#42` idea 2 (4-way TT clusters) was tested and parked (two-TC flat) —
  in-tree behind `ENABLE_TT_CLUSTERS` (default OFF) for a post-NNUE
  revisit. `#7` (LMP re-test) was re-parked on a two-machine sign-split —
  in-tree behind `ENABLE_LMP` (default OFF). `#31` (Hash-size sweep) came
  back flat on both boxes; the `Hash` UCI default stays at 64MB.
- Methodology lesson continues to bank: this ladder found three more
  latent structural bugs (`#50`, `#51`, `#59`) the same way 2.2 found its
  three — by auditing long-standing structures, not just by tuning deltas.
