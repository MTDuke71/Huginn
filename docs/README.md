# Huginn Documentation

Index of documentation for the Huginn chess engine. Some files in this directory pre-date the move to a pure-bitboard engine and may still describe the old hybrid mailbox/bitboard design — verify against the current code in `src/` before relying on any specific claim.

## Reference

- [API.md](API.md) — public API and usage
- [BUILD_GUIDE.md](BUILD_GUIDE.md) — MSVC build with CMake presets
- [WSL_BUILD_GUIDE.md](WSL_BUILD_GUIDE.md) — GCC/WSL build
- [UCI_IMPLEMENTATION.md](UCI_IMPLEMENTATION.md) — UCI protocol
- [CHANGELOG.md](CHANGELOG.md) — release history
- [ToDo.md](ToDo.md) — open work
- [CLAUDE.md](CLAUDE.md) — repo notes for Claude (currently stale; pending rewrite for `pure-bitboard-engine`)

## Architecture

- [ARCHITECTURE.md](ARCHITECTURE.md) — high-level engine architecture
- [POSITION_AND_MOVEGEN_ARCHITECTURE.md](POSITION_AND_MOVEGEN_ARCHITECTURE.md) — position representation and move generation
- [BITBOARD_IMPLEMENTATION.md](BITBOARD_IMPLEMENTATION.md) — bitboard internals
- [BITBOARD_MIGRATION_PLAN.md](BITBOARD_MIGRATION_PLAN.md) — migration plan from mailbox to bitboard
- [UML_ARCHITECTURE.md](UML_ARCHITECTURE.md) — UML diagrams
- [EVALUATE_FLOWCHART.md](EVALUATE_FLOWCHART.md) — evaluation flow

## Special-move validation

- [CASTLING_VALIDATION.md](CASTLING_VALIDATION.md)
- [EN_PASSANT_VALIDATION.md](EN_PASSANT_VALIDATION.md)
- [FEN_GENERATION.md](FEN_GENERATION.md)
- [DEBUG_VALIDATION.md](DEBUG_VALIDATION.md)

## Performance

- [PERFORMANCE_TRACKING.md](PERFORMANCE_TRACKING.md) — methodology and tracked numbers
- [OPTIMIZATION_ANALYSIS.md](OPTIMIZATION_ANALYSIS.md)
- [OPTIMIZATION_SUMMARY.md](OPTIMIZATION_SUMMARY.md)
- [MOVE_DECODING_ANALYSIS.md](MOVE_DECODING_ANALYSIS.md)
- [assembly_analysis.md](assembly_analysis.md)
- [PROFILING_TOOLS_USAGE.md](PROFILING_TOOLS_USAGE.md)
