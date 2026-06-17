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
- [ ] `position.cpp` (601 lines)
- [ ] `search.cpp` (2410 lines — **special: split across several days** — `@file`
      header + `evaluate`/`AlphaBeta`/`quiescence`/`searchPosition` first, then
      the pruning/ordering helpers in later sittings)
- [x] `movegen_bb.hpp` / `movegen_bb.cpp` — production movegen: pseudo-legal
      contract (castling the legal exception), special-move coverage, file-local
      castling helper; replaced stale pre-rewrite @file blocks (2026-06-17)
- [ ] `chess_types.hpp` / `chess_types.cpp`
- [ ] `evaluation.hpp` (constants/masks/PSTs — already 12 briefs; finish the rest)
- [ ] `move.hpp` (already 27 briefs — audit/finish)

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

_Last updated: 2026-06-17 — documented production bitboard movegen (movegen_bb)._
