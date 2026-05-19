# BACKLOG #28 — PGN repetition conversion findings

Source: 400 games across `huginn_vs_t5_intel.pgn` + `huginn_vs_t5_amd.pgn` (baseline-t6 gauntlet).

## Pipeline

- 238 games ended in 3-fold repetition.
- 126 of those: Huginn_current played the clinching move.
- 20 of *those* had Huginn's own reported eval >= +300cp **and** a legal non-repeating alternative (candidates).
- Each candidate re-searched: current Huginn with full game history replayed (300ms) + Stockfish 18 depth-24 oracle on the bare pre-repetition position.

## Verdict tally (20 candidates)

- **REAL_BUG: 5** — Huginn still clinches the repetition AND Stockfish confirms the position is won.
- **FIXED_BY_T6: 11** — Stockfish confirms won, but current Huginn no longer repeats (t6 root-demotion now steers away). Regression-prevention guards.
- **ARTIFACT: 4** — Stockfish says the position is ~0/drawn; Huginn's large eval was inflated. Not a thrown win; discard.

## Key correction

An early harness bug (sending `quit` before `bestmove`, aborting the background `go`) made Huginn *look* like it still repeated everywhere. With a streaming UCI driver, most candidates are FIXED_BY_T6 or ARTIFACT. The genuine residual bug is **5/20**.

## Regression set (REAL_BUG)

### alt_exists (3) — a clearly winning non-repeating move exists; Huginn must stop emitting the clincher

| id | clincher | SF best | SF cp | PGN cp |
|---|---|---|---|---|
| huginn_vs_t5_intel.pgn-R19 | d7d8 | d7e6 | 419 | 400 |
| huginn_vs_t5_intel.pgn-R91 | a6a7 | f4d4 | 562 | 535 |
| huginn_vs_t5_amd.pgn-R32 | e5b8 | e5g5 | 756 | 550 |

### history_dependent (2) — SF's own best move IS the clincher; objectively best from the bare position but a 3-fold given game history. Needs explicit root repetition detection, not a bm test.

| id | clincher (=SF best) | SF cp | PGN cp |
|---|---|---|---|
| huginn_vs_t5_amd.pgn-R54 | h4h5 | 709 | 603 |
| huginn_vs_t5_amd.pgn-R87 | d2c2 | 565 | 390 |

## Recommended next action

The `alt_exists` subset is the clean, fixable bug class: in won positions with a winning non-repeating move, Huginn still selects the repetition. The t6 root-demotion fires only at >= +300cp *and* only when the repetition is the immediate root move; these cases either dip under threshold during the shuffle or repeat one ply deeper. Candidate fix: lower/condition the demotion and detect claimable repetition at the root explicitly (BACKLOG #28 candidate fixes 2-3), validated against this fixture + a t6 gauntlet.
