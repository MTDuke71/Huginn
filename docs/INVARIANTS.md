# Invariants & contracts

Load-bearing facts that span functions — the things that look fine locally but
are wrong globally. Doxygen tells you *what a function does*; this file tells you
*what must stay true across the codebase*. **Read the relevant section before
changing search, eval, or make/unmake.**

Why this exists: BACKLOG #44 was an *unwritten* invariant violated — the
repetition detector used `move_history.size()` when the contract is "path length
is `pos.ply`" (see below). The engine drew won games for it. One line in this
file would have prevented it.

> Anchors are given as `file: symbol` (stable) rather than line numbers (drift).
> If something here contradicts the code, the code wins — fix this file.

---

## Board & move representation

- **sq64 everywhere.** Squares are indices 0–63, `a1=0 … h8=63`. There is no
  mailbox-120 board anymore; piece-on-square is derived from the per-piece
  bitboards via `Position::at_sq64()`. `Piece::Offboard` is a dead mailbox
  sentinel — don't reintroduce offboard logic.
- **Move from/to are sq64 0–63.** The 7-bit fields in `S_MOVE` can *physically*
  hold 0–127, so a stale/`-1` index silently masks into a wrong square instead
  of an obvious error. `debug_check_sq64_move()` guards this in debug builds.
  (`src/move.hpp`)
- **`move_history` is a grow-only high-water-mark buffer, NOT a stack.**
  `MakeMove` does `move_history.resize(ply+1)` (grow only); `TakeMove` decrements
  `ply` but never shrinks the buffer. So **`move_history.size()` = the deepest
  ply ever reached this search, which is ≥ the current depth.** The current path
  is exactly `move_history[0 .. pos.ply)`. **Use `pos.ply` for path length, never
  `.size()`.** (This was #44; see `repetition_count_in_history` in `src/search.cpp`.)
- **Make/unmake must balance.** Every `MakeMove`/`MakeNullMove` is paired with a
  `TakeMove`/`TakeNullMove`; the board, Zobrist key, castling/ep, and clocks must
  be byte-restored. An imbalance corrupts `pos.ply` (and thus rep detection) even
  when the board *looks* right. #37 (board-desync) is the open instance of this
  class — instrument with `ENABLE_SEARCH_INTEGRITY_ASSERTS`.
- **Zobrist is incremental.** Piece add/remove XORs happen in the make/unmake
  primitives; `update_zobrist_for_move` folds only side/castling/ep. After any
  *non-incremental* edit (FEN load, `rebuild_counts`), call `update_zobrist_key`.

## Evaluation (`Engine::evaluate`, `src/search.cpp`)

- **Colour symmetry is mandatory.** `evaluate(pos) == -evaluate(mirror(pos))`.
  Every new term's white/black paths must mirror exactly (square flip + side
  swap; no rank-direction asymmetry that isn't mirrored). Enforced by
  `EvaluationSymmetryTest` (`test/test_evaluation_symmetry.cpp`) and per-term
  mirror tests. **A new eval term that breaks the mirror suite is a bug, full
  stop** — fix the symmetry, don't weaken the test.
- **Compute white-positive, negate for the side to move at the end.** Terms add
  to `mg_pst`/`eg_pst` (or `score`) as White − Black; the final return negates
  for Black to move.
- **Tapered eval.** Phase-dependent terms carry separate `_MG`/`_EG` values and
  are blended via `game_phase_256()` (256 = opening … 0 = bare kings). Add a
  term's MG part to the mg accumulator and EG to the eg accumulator; the blend
  happens once at the combine. (King safety is MG-only by design — it must fade
  to 0 in the endgame, or it regresses; see #35 Exp 3.)
- **Three independent value tables — do NOT merge them.** Eval material
  (`EvalParams::PIECE_VALUES_MG/EG`, Texel-tuned, *drifts*), `SEE_PIECE_VALUE`
  (`src/see.hpp`), and the MVV-LVA table (`Engine::init_mvv_lva`) each have their
  own copy. They start at the classic 100/320/330/500/900 *by coincidence*, not
  contract. Coupling capture ordering/pruning to eval tuning injects noise and
  breaks MVV-LVA's king=0 special case. (See the note in `src/chess_types.hpp`.)
- **`EVAL_PARAM` = tunable, `inline constexpr` = fixed structure.** `EVAL_PARAM`
  values are exposed to the Texel tuner (mutable under `-DHUGINN_TUNING`) and
  fitted; `constexpr` values are deliberate fixed structure. New tunable terms
  must be added to the tuner's `collect_params()` *and* dump (`tools/texel/tuner.cpp`).
- **`PIECE_VALUES` (the canonical fixed table) drives `value_of()`** — move
  ordering + incremental material. It is *not* the tuned `PIECE_VALUES_MG`; keep
  them separate so ordering doesn't drift with eval tuning.

## Search (`Engine::AlphaBeta` / `searchPosition`, `src/search.cpp`)

- **TT keys are path-independent; repetition & 50-move are path-dependent.** A
  TT entry is keyed by position only and carries no history. Repetition and
  50-move draw scores depend on the path to the node, so they are **returned
  before any TT probe/store** (and never cached). Putting a path-dependent score
  through the TT is how a winning score gets served for a now-drawn position
  (the warm-TT half of #44).
- **Mate scores encode as `MATE − ply`** (`MATE = 29000`, `INFINITE = 30000`).
  They must be ply-adjusted on TT store *and* probe (`ENABLE_PLY_TRACKED_TT_MATE`)
  so a "mate in N" from one depth reads correctly at another. Mate-distance
  pruning clamps `[α,β]` to `±(MATE − ply)`.
- **Node-entry order matters.** Current order: draw checks (rep, 50-move) →
  mate-distance pruning → TT probe → static-eval-based pruning (RFP, null move,
  futility, razoring) → move loop (LMR, etc.). Pruning that returns a bound must
  respect the (possibly MDP-tightened) window.
- **A winning side must route around repetitions.** The root draw-avoidance
  (`searchPosition`) scores a repetition the winning side enters as
  `WINNING_REPETITION_DRAW_SCORE`, so a won engine doesn't shuffle into a 3-fold.
  This depends entirely on correct rep counting (see `pos.ply` above).
- **Pruning is heuristic; tactics must survive it.** Futility/razoring/null-move
  trade soundness for depth. When changing them, watch for tactical leaks
  (the #45 concern: node-level futility prunes whole nodes incl. captures/checks).

## Flags, build, and shipping

- **New search/eval features go behind an `ENABLE_*` flag, default OFF, with the
  flag-OFF path byte-identical to the current baseline.** Build the ON arm with
  `-DENABLE_X=1`. (Eval *re-tunes* are the exception: the baked params change, so
  "flag-off byte-identical" applies to the code path, not the values.)
- **Ship bar = same-sign two-machine SPRT** (AMD + Intel), or tight
  cross-machine agreement for a small effect. Sign-splits are parked by default
  (logged exceptions: t14 rook-on-7th, t18-era outposts). On ship, flip the flag
  default to 1; reject → revert.
- **Baselines are built per-machine from the `baseline-tN` tag** — never copy one
  box's binary to the other. Snapshot as `huginn_tN.exe` in the fastchess dir.
- **The test suite + symmetry suite must pass** before any ship (currently 203
  GoogleTest cases). `cmake --build … --target check`.
- **MSVC env:** direct `cl`/`cmake` calls need `vcvars64.bat` sourced first if
  `INCLUDE` isn't set (a bare `cl` on PATH without env will fail with "no include
  path"). The gauntlet/tuner builds wrap this.

## Calibration & Texel tuning

- **K is fixed while weights are tuned.** The sigmoid is
  `1/(1 + 10^(−K·eval/400))`; K (≈1.520 for this eval+corpus) is fit once then
  frozen — tuning K and weights together is degenerate. Re-fit K only if the
  centipawn scale changes (e.g. rescaled material). (`tools/texel/`)
- **Bake-verify every tune.** After `bake.py`, rebuild the tuner and run
  `--max-sweeps 0`; the reported start MSE must equal the tuner's converged MSE,
  proving the bake was exact.
- **Self-play Elo inflates (~1.5×) vs the external CCRL scale.** A vs-baseline
  gauntlet number is optimistic; anchor real strength against rated opponents
  (the Stash ladder, #5). New HCE terms convert MSE→Elo better than re-fits of
  existing terms (which sit at a floor).

---

_Keep this lean and current. When a new contract emerges (or one here is
falsified), edit this file in the same change — a stale invariant is worse than
none. Linked from CLAUDE.md so it rides into context._
