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
- [x] `magic_bitboards.hpp` / `magic_bitboards.cpp` — storage globals, ray-walker,
      mask builders, subset enumeration, magic finder, init (2026-06-22)
- [x] `bitboard.hpp` / `bitboard.cpp` — free bit ops (popcount/lsb/print) +
      magic-slider delegates (2026-06-22)
- [x] `see.hpp` / `see.cpp` — SEE_PIECE_VALUE independence rationale (Doxygen),
      file-local helpers (piece_type_on, attackers_to) (2026-06-22)
- [x] `uci.hpp` / `uci.cpp` — uci.hpp: class @brief + member `///<` + method briefs;
      uci.cpp already had @file + ~24 briefs (audited) (2026-06-28)
- [x] `uci_utils.hpp` / `uci_utils.cpp` — @file + parse_uci_move contract (2026-06-22)
- [x] `polyglot_book.hpp` / `polyglot_book.cpp` — .cpp @file (own-Zobrist note);
      hpp already documented (2026-06-22)
- [x] `syzygy_tablebase.hpp` / `syzygy_tablebase.cpp` — @file (FATHOM gating,
      not-in-TT note) + member `///<`; class methods already documented (2026-06-22)
- [x] `zobrist.hpp` / `zobrist.cpp` — globals `///<`, init/compute @brief,
      .cpp @file + relocated compute() doc (2026-06-22)
- [x] `pvtable.hpp` / `pvtable.cpp` — PVEntry/PVTable members + methods; .cpp @file (2026-06-22)
- [x] `input_checking.hpp` / `input_checking.cpp` — @file + briefs present, audited (2026-06-28)
- [x] `attack_tables.hpp` / `attack_tables.cpp` — @file + briefs present, audited (2026-06-28)
- [x] `attack_detection.hpp` / `attack_detection.cpp` — square semantics,
      target-side pawn lookup, params/return/precondition (2026-06-17)
- [x] `movegen.hpp` / `movegen.cpp` — S_MOVELIST facade, scoring bands,
      legality/capture-filter contracts (2026-06-17)

## Tier 3 — small / leaf files

- [x] `main.cpp` (+@file), `init.hpp` (+@file) / `init.cpp` (had @file + briefs) (2026-06-28)
- [x] `square.hpp` (function/enum briefs), `bit_utils.hpp` (exemplary @file covering its
      shims, audited) (2026-06-28)
- [x] `msvc_intrinsics.hpp`, `msvc_optimizations.hpp` — @file + briefs present, audited (2026-06-28)

---

**PASS COMPLETE** ✅ (2026-06-28). All three tiers done: every `src/` file has an
`@file` block and `@brief` on its public types/functions; member `//` comments
converted to `///<`. Comment-only throughout; final build clean, 203/203 tests pass.

_History: Tier 1 (2026-06-14→22), Tier-2 main batch (2026-06-22), Tier-2 remainder
+ Tier-3 leaf files (2026-06-28)._
