# Texel tuner (BACKLOG #9)

Tunes Huginn's evaluation (material + piece-square tables) by minimizing the
mean-squared error of `sigmoid(K · white_eval)` against game outcomes, over a
corpus of labeled quiet positions.

## Why

Hand-tuning eval terms one at a time hit a wall (#35: tapered material and king
safety both landed in the noise). Those terms are *structurally* correct but
un-tuned. Texel tuning fits them jointly against real game results.

## Workflow

1. **Get games** (PGN). Master games are ideal (clean result labels) — e.g. a
   ChessBase export filtered to ≥~2200 Elo, decisive+draws. Lichess works with
   a rating filter. Volume: ~hundreds of k games → ~1–2M positions is plenty.

2. **Extract labeled positions:**
   ```
   python extract_fens.py GAMES.pgn -o fens.txt \
       --min-elo 2200 --min-ply 16 --sample-per-game 10 --max-positions 2000000
   ```
   Output lines: `<result> <FEN>` where result ∈ {1.0, 0.5, 0.0} (White POV).
   Filters: skip opening plies, skip in-check positions, sample a few per game
   (decorrelate / avoid opening bias).

3. **Tune** (the engine must be built with the tuner target, which sets
   `-DHUGINN_TUNING` so the eval tables are mutable):
   ```
   cmake --build build/msvc-x64-release --config Release --target huginn_tuner
   ./build/msvc-x64-release/bin/Release/huginn_tuner.exe fens.txt
       [--positions N] [--k K] [--max-sweeps S]
   ```
   It fits K, runs coordinate-descent (per-param line search) until MSE
   converges, and prints paste-ready C++ tables.

4. **Bake + validate:** paste the printed `PIECE_VALUES_MG/EG` into
   `src/chess_types.hpp` and the PST tables into `src/evaluation.hpp`, rebuild
   the release engine, and run a **two-machine SPRT vs the current baseline**
   (`test_huginn_gauntlet.bat t11`). Only ship on a confirmed gain.

## How the parameterization works (no eval drift)

`src/chess_types.hpp` defines `EVAL_PARAM` / `EVAL_FN`: in the release build the
eval tables are `constexpr` (folded, zero cost); under `-DHUGINN_TUNING` they
become mutable `inline` globals the tuner overwrites. The tuner calls the engine's
*real* `Engine::evaluate`, so there is no second eval implementation to drift.

## Notes / future work

- Single-threaded for now. On very large corpora cap with `--positions` (a few
  hundred k tunes fine); parallelizing `mse()` or switching to a sparse
  feature-vector eval would speed up the full-corpus run.
- Currently tunes material (MG+EG, P–Q) + the 6 PSTs + king-EG. Natural
  extensions: separate EG PSTs for all pieces (tapered PSTs), mobility weights,
  king-safety weights — all already structurally present in the eval.
- `fens*.txt` (extracted corpora) are git-ignored; they're regenerable.
