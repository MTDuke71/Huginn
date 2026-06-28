# Huginn 2.2 — Changelog (2.1 → 2.2)

**Released 2026-06-28.** 2.2 = 2.1 content (`baseline-t15`) + six baselines
(t16–t21). The binary is identical to `baseline-t21`; the only diff vs t21 is the
UCI id string.

## Headline

- **Advertising gauntlet (2.2 vs 2.1, self-play, 10+0.1) — two-machine, both legs
  SPRT H1-accept, LOS 100%:**
  - **AMD: +556.84 ± 116.20, 96.10%** (143W / 1L / 10D), Ptnml [0,0,1,10,66], @154g.
  - **Intel: +470.4, 93.75%** (140W / 0L / 20D), Ptnml [0,0,1,18,61], LLR 2.98 @160g.
  - **Pooled: 283W / 1L / 30D = 94.9% over 314 games (~+508 Elo).** One loss total.
- **External calibration: ~1834 → ~2434 CCRL-ladder** — 2.1 sat ~1770–1834; 2.2
  (t21) scores **68.65% / +136 vs Stash 17.0 (2298) over 1000g at 60+0.6 → ~2434**,
  and t20 pinned ~2345–2390 at blitz (Stash 17 56.75% / MTLChess v0.5 61%). The
  engine crossed a strength class (club → expert/CM-range).
- Self-play *here* ≈ external, because the gains were **correctness bugs** (they
  hurt vs every opponent roughly equally), not eval-breadth tuning.

## The story

The bulk of 2.1→2.2 is **three latent bugs invisible to the incremental SPRT
ladder** — every baseline t5–t19 carried them, so they cancelled in every A/B and
only surfaced by stepping *outside* the tune-and-compare loop (watching real games,
auditing long-standing structures). The #41 played-game re-diagnosis confirmed the
theme: **search soundness, not eval breadth, was the hidden lever.**

## Per-baseline

| Tag | Change | Result vs prior |
|-----|--------|-----------------|
| t16 | **King safety** (#9 r7 / #2) — reformulated Texel-tunable (gate removed, MG-only) | +10.1 pooled 2-machine vs t15 |
| t17 | **#44 repetition fix** — rep detector used the grow-only `move_history` size, not `pos.ply`; with a warm TT it drew won games by miscounting 3-folds | +48 external (≈1834 CCRL) |
| t18 | **Mate-distance pruning** (#43 s3) — clamp α/β to the mate envelope | +~12 2-machine |
| t19 | **Safe mobility** (#9 r9) — per-piece-type weights over a safe area | +~8 2-machine |
| t20 | **Move-level futility** (#45) — *latent search-correctness bug*: node-level `return alpha` bailed whole nodes incl. PV + ≤3-ply tactical replies; now skips only quiet non-checking moves | **AMD +345 / Intel +355** (both ~88%, LOS 100%); audited + externally validated. Largest single gain in program history |
| t21 | **TT-clear on newgame (#46) + time-management fix (#47)** — TT was never cleared between games; the iteration gate left ~half the clock unused | **+127 vs t20** (10+0.1, LOS 100%, zero time-forfeits) |

## Notes

- No architectural change — still pure-bitboard hand-crafted eval, single-thread.
  3.0 is reserved for NNUE (#39) / Lazy SMP (#40).
- 203/203 GoogleTest cases pass.
- Methodology lesson banked: incremental A/B SPRT is structurally blind to bugs
  both arms share — audit load-bearing structures periodically, don't only tune.
