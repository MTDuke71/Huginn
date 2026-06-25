# Doxygen documentation pass — progress tracker

Improving Doxygen `@file` / `@brief` / `@param` / `@return` coverage across the
source, **2 files per day** until finished. The codebase already uses
Doxygen-style comments; this pass fills the uneven gaps (the survey below was
the starting state: `@file` = has a file header, `@brief` = count of documented
declarations).

**Convention for this pass:** add a `@file` block to every file; `@brief` (+
`@param`/`@return` where non-trivial) to every public type and function; convert
existing `//` member comments to Doxygen `///<` trailing comments. Document the
interface (headers) before the implementation (.cpp).

Mark items `[x]` when done, with the date.

## Tier 1 — core (do first; highest reader value)

- [x] `position.hpp` — structs, Position class + members + methods (2026-06-14)
- [x] `search.hpp` — Engine, SearchInfo, MinimalLimits + search-core methods (2026-06-14)
- [x] `position.cpp` — @brief on all 14 Position methods (Zobrist incr/full,
      is_consistent, FEN in/out, reset/startpos/rebuild_counts, MakeMove/TakeMove
      with the legality-filter contract, null moves, perft, generate_all_moves)
      (2026-06-18)
- [x] `search.cpp` — DONE (2026-06-22, two sittings). Sitting 1: `@file` header
      (section map + INVARIANTS pointer) + the core four (`evaluate`/`AlphaBeta`/
      `quiescence`/`searchPosition`). Sitting 2: every remaining helper —
      integrity asserts, LMR table, `game_phase_256`, `king_safety_white_mg`,
      `repetition_count_in_history` (the #44 `pos.ply` note), the move-ordering
      heuristics (MVV-LVA / killers / history / counter / continuation),
      `pick_next_move`, IID, MaterialDraw/mirrorBoard, book + Syzygy probes.
      Completeness-swept (no undocumented `Engine::` left); 203/203 tests pass.

**Tier 1 COMPLETE** ✅ (position.hpp, search.hpp, position.cpp, search.cpp,
movegen_bb, chess_types, evaluation.hpp, move.hpp).
- [x] `movegen_bb.hpp` / `movegen_bb.cpp` — production movegen: pseudo-legal
      contract (castling the legal exception), special-move coverage, file-local
      castling helper; replaced stale pre-rewrite @file blocks (2026-06-17)
- [x] `chess_types.hpp` / `chess_types.cpp` — Color/PieceType/Piece enums +
      enumerators, pack/unpack + type predicates, castling flags, the EVAL_PARAM
      tuning macro + the three material-value tables (MG/EG/canonical) and their
      independence rationale, PieceTypeIter; legacy bits (PieceList, Offboard)
      tagged @deprecated. (.cpp was already complete.) (2026-06-17)
- [x] `evaluation.hpp` — finished: tapered-eval @file rewrite (was stale
      3-bucket), GamePhase enum + init/extern masks, and Doxygen on every
      EVAL_PARAM group (outposts/pawns/threats/KS), all 12 PSTs, and the
      structure masks (2026-06-17)
- [x] `move.hpp` — audit/finish: documented the two string utilities +
      debug_check_sq64_move, converted the MOVE_* mask/shift constants to `///<`,
      fixed the get_from/get_to return range (sq64 0-63, was 0-127) (2026-06-18)

## Tier 2 — supporting subsystems

- [x] `transposition_table.hpp` — rationale-first @file (replacement/aging gap),
      store/probe @brief (2026-06-14; prompted by the Fruit/Toga TT example → #42)
- [ ] `magic_bitboards.hpp` / `magic_bitboards.cpp`
- [ ] `bitboard.hpp` / `bitboard.cpp`
- [ ] `see.hpp` / `see.cpp`
- [ ] `uci.hpp` / `uci.cpp`
- [ ] `uci_utils.hpp` / `uci_utils.cpp`
- [ ] `polyglot_book.hpp` / `polyglot_book.cpp`
- [ ] `syzygy_tablebase.hpp` / `syzygy_tablebase.cpp`
- [ ] `zobrist.hpp` / `zobrist.cpp`
- [ ] `pvtable.hpp` / `pvtable.cpp`
- [ ] `input_checking.hpp` / `input_checking.cpp`
- [ ] `attack_tables.hpp` / `attack_tables.cpp`
- [x] `attack_detection.hpp` / `attack_detection.cpp` — square semantics,
      target-side pawn lookup, params/return/precondition (2026-06-17)
- [x] `movegen.hpp` / `movegen.cpp` — S_MOVELIST facade, scoring bands,
      legality/capture-filter contracts (2026-06-17)

## Tier 3 — small / leaf files

- [ ] `main.cpp`, `init.hpp` / `init.cpp`
- [ ] `square.hpp`, `bit_utils.hpp`
- [ ] `msvc_intrinsics.hpp`, `msvc_optimizations.hpp`

---

_Last updated: 2026-06-22 — search.cpp finished (sitting 2). **Tier 1 complete.**
Next up: Tier 2 supporting subsystems (magic_bitboards, bitboard, see, uci, …)._
