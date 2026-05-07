# Huginn Documentation

Index of documentation for the Huginn chess engine. Most surviving
docs reflect the current `pure-bitboard-engine` branch state. A few
listed under *Reference* still need a content refresh — flagged
inline.

## Active trackers

- [BACKLOG.md](BACKLOG.md) — single-file issue tracker. Open / closed
  features, deferred items, current priorities. Always start here.
- [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) — Tier 1-4 roadmap for
  search and evaluation features.
- [CLAUDE.md](CLAUDE.md) — repo notes used by Claude Code when
  working in this tree.

## Reference

- [API.md](API.md) — public API and usage. **Stale**: still describes
  a pre-bitboard `MinimalEngine` with no TT/qsearch. Pending rewrite.
- [BUILD_GUIDE.md](BUILD_GUIDE.md) — MSVC build with CMake presets.
- [WSL_BUILD_GUIDE.md](WSL_BUILD_GUIDE.md) — GCC/WSL build.
- [UCI-Protocol-Specification.txt](UCI-Protocol-Specification.txt) —
  external UCI protocol spec, mirrored for offline reference.

## Architecture

- [POSITION_AND_MOVEGEN_ARCHITECTURE.md](POSITION_AND_MOVEGEN_ARCHITECTURE.md)
  — position representation and move generation. Currently framed
  as "hybrid mailbox + bitboard"; pending reframe as primary
  bitboard with mailbox auxiliary.
- [BITBOARD_IMPLEMENTATION.md](BITBOARD_IMPLEMENTATION.md) — bitboard
  internals.
- [MOVEGEN_COMPARISON.md](MOVEGEN_COMPARISON.md) — comparison of
  Huginn's movegen with MTLChess. Current as of 2026-05-05.
