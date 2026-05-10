# Huginn API — pointer page

Huginn is a single-engineer chess engine. There is no maintained
public API document; the prior version of this file described a
pre-bitboard "MinimalEngine" without TT, qsearch, killers, history,
LMR, or NMP — none of which reflects the current engine.

If you're looking for an API surface, here's where to actually find it.

## External API: UCI

Huginn's only external interface is the **UCI protocol**. Anything a
chess GUI (Arena, Cute Chess, Fritz, etc.) or a fastchess wrapper
sees comes through stdin/stdout per the UCI spec.

- Implementation: [`src/uci.cpp`](../src/uci.cpp), [`src/uci_utils.cpp`](../src/uci_utils.cpp)
- Spec mirror: [UCI-Protocol-Specification.txt](UCI-Protocol-Specification.txt)
- Notable options: `Hash`, `OwnBook`, `SyzygyPath` (probe is gated
  off — see BACKLOG #10).

## Internal architecture (when you need to read code)

| Concern | Files |
|---|---|
| Search | [`src/search.cpp`](../src/search.cpp), [`src/search.hpp`](../src/search.hpp) |
| Evaluation | [`src/evaluation.cpp`](../src/evaluation.cpp) |
| Move generation | [`src/movegen.cpp`](../src/movegen.cpp), [`src/movegen_bb.cpp`](../src/movegen_bb.cpp) |
| Position (mailbox + bitboard) | [`src/position.cpp`](../src/position.cpp), [`src/position.hpp`](../src/position.hpp) |
| Move encoding (S_MOVE) | [`src/move.hpp`](../src/move.hpp), [`src/chess_types.hpp`](../src/chess_types.hpp) |
| TT (header-only) | [`src/transposition_table.hpp`](../src/transposition_table.hpp) |
| SEE | [`src/see.cpp`](../src/see.cpp) |
| Attack detection / tables | [`src/attack_detection.cpp`](../src/attack_detection.cpp), [`src/attack_tables.cpp`](../src/attack_tables.cpp) |

## Where decisions and rationale live

- [BACKLOG.md](BACKLOG.md) — every feature has an entry: what was
  tried, what shipped, what regressed, what's blocked. Read this
  before starting any change in search/eval.
- [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) — Tier 1-4 roadmap; the
  longer-horizon plan that BACKLOG entries reference.
- [POSITION_AND_MOVEGEN_ARCHITECTURE.md](POSITION_AND_MOVEGEN_ARCHITECTURE.md)
  — the architectural narrative. Currently being updated to reflect
  the pure-bitboard tip; trust the code over the prose where they
  disagree.
- [MOVEGEN_COMPARISON.md](MOVEGEN_COMPARISON.md) — comparison with
  MTLChess, useful when reasoning about move-generation choices.

## What is *not* documented anywhere

- Internal C++ class APIs (Position, MinimalEngine, S_MOVE helpers).
  Read the headers — they're small and self-explanatory. If you find
  a non-obvious invariant, add a one-line comment in code rather than
  a separate doc page.
- Performance numbers. They drift fast. Current ballpark
  (~1.5M nps, 208/208 tests) lives in [CLAUDE.md](CLAUDE.md) and gets
  refreshed when it materially changes.
