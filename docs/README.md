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

- [API.md](API.md) — pointer page for finding the actual API surface
  (UCI on the outside; `src/` files for each internal concern).
- [BUILD_GUIDE.md](BUILD_GUIDE.md) — MSVC build with CMake presets.
- [WSL_BUILD_GUIDE.md](WSL_BUILD_GUIDE.md) — GCC/WSL build.
- [UCI-Protocol-Specification.txt](UCI-Protocol-Specification.txt) —
  external UCI protocol spec, mirrored for offline reference.

## Architecture

- [POSITION_AND_MOVEGEN_ARCHITECTURE.md](POSITION_AND_MOVEGEN_ARCHITECTURE.md)
  — `Position` struct (bitboard-primary), `S_MOVE` encoding, the
  movegen pipeline, and the `MakeMove`/`TakeMove` protocol.
- [BITBOARD_IMPLEMENTATION.md](BITBOARD_IMPLEMENTATION.md) — bitboard
  primitives, square-indexing, and how `Position` uses them.
- [MOVEGEN_COMPARISON.md](MOVEGEN_COMPARISON.md) — comparison of
  Huginn's movegen with MTLChess. Current as of 2026-05-05.
