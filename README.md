# Huginn 2.2

<p align="center">
  <img src="images/Huginn.png" alt="Huginn Logo" width="320">
</p>

### *Huginn: Odin's Thought, in Every Move*

Huginn is a UCI-compliant chess engine in modern C++17/20 — a **pure-bitboard**,
single-threaded, hand-crafted-evaluation engine. It runs in any UCI GUI (Arena,
Cute Chess, BanksiaGUI, ChessBase, …).

**Strength: ~2434 CCRL-Blitz-ladder scale** (v2.2 / `baseline-t21`, measured vs the
Stash engine ladder). Version 2.2 is **~+508 Elo over 2.1** — a pooled two-machine
gauntlet (AMD +557 / Intel +470) scored 94.9% with a single loss in 314 games.

➡️ **[Download the latest release](https://github.com/MTDuke71/Huginn/releases/latest)**
 · **[2.2 changelog](docs/CHANGELOG_2.2.md)** · **[baseline ladder](docs/BASELINE_LADDER.md)**

---

## Features

**Search** ([src/search.cpp](src/search.cpp))
- Negamax / alpha-beta with **PVS**, iterative deepening, and a transposition table
- Quiescence search with **SEE** pruning
- Pruning stack: null-move (R=4), reverse futility, **move-level futility**,
  razoring, **mate-distance pruning**
- **LMR** with a `log·log` reduction table, check extension, IID
- Move ordering: TT move, MVV-LVA, killers, history, counter-move
- **Polyglot** opening book + **Syzygy** WDL tablebase probing

**Evaluation** ([src/evaluation.hpp](src/evaluation.hpp) + `Engine::evaluate` in [src/search.cpp](src/search.cpp))
- **Tapered** (smooth `game_phase_256` MG↔EG blend), fully **Texel-tuned** on a
  725k quiet-labeled corpus
- Material + piece-square tables (separate MG/EG king table)
- Pawn structure: isolated, doubled, passed, connected, backward
- Bishop pair, rook/queen on open & semi-open files, rook on the 7th
- Mobility (safe-area weighted), threats, king safety, tempo, insufficient-material draw

**Architecture**
- **Pure bitboard**: per-piece bitboards are the sole source of truth; piece-on-square
  is derived via `Position::at_sq64()`. No mailbox board, no piece lists.
- Squares indexed **0–63** (a1 = 0, h8 = 63); **magic bitboards** for slider attacks
- **S_MOVE**: 25-bit packed move encoding with O(1) incremental make/unmake
  (bitboards, Zobrist key, material)
- **~3.55 Mnps** single-threaded; modern C++17/20 (type-safe enums, `constexpr`)

**Quality**
- **203 GoogleTest** cases + WAC300 / LCT2 tactical EPD sweeps + evaluation-symmetry
  (mirror) tests
- Illegal-move guard at the UCI bestmove boundary

## UCI options

| Option | Default | Notes |
|--------|---------|-------|
| `Hash` | 64 MB | transposition table size |
| `Threads` | 1 | single-threaded engine |
| `OwnBook` | **false** | set `true` to use the Polyglot book (2.1 defaulted this on) |
| `BookFile` | `src/performance.bin` | Polyglot book path |
| `SyzygyPath` | `c:\TB\` | Syzygy tablebase directory |

> The release ships `performance.bin`. To use it, place it next to the exe and set
> `OwnBook=true`; otherwise Huginn plays bookless by default.

---

## Build (MSVC / Visual Studio 2022)

**Prerequisites:** Visual Studio 2022 (C++ workload), CMake 3.22+, Git.

```powershell
# from the repository root
cmake --preset msvc-x64-release
cmake --build build/msvc-x64-release --config Release            # all targets
# or a single target:
cmake --build build/msvc-x64-release --config Release --target huginn
```

GCC/Linux: `cmake --preset gcc-x64-release && cmake --build build/gcc-x64-release`.
See [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md) (and [docs/WSL_BUILD_GUIDE.md](docs/WSL_BUILD_GUIDE.md)) for details and GoogleTest setup.

### Run

```powershell
.\build\msvc-x64-release\bin\Release\huginn.exe          # UCI engine
.\build\msvc-x64-release\bin\Release\huginn_tests.exe    # full test suite
```

### Test (CTest)

```powershell
# builds huginn_tests, then runs ctest
cmake --build build/msvc-x64-release --config Release --target check

# or directly
cd build/msvc-x64-release
ctest -C Release --output-on-failure
ctest -C Release -R "perft|position"   # filter by pattern
```

---

## Documentation

- **[Documentation index](docs/README.md)**
- **[Search & evaluation inventory](docs/SEARCH_AND_EVAL.md)** — what's implemented, by tier
- **[Baseline ladder](docs/BASELINE_LADDER.md)** — every shipped version, what it added, its SPRT result
- **[Backlog](docs/BACKLOG.md)** — current state, priorities, and the experiment log
- **[Invariants](docs/INVARIANTS.md)** — load-bearing cross-function contracts (read before changing search/eval/make-unmake)
- **[API reference](docs/API.md)** · **[Position & movegen architecture](docs/POSITION_AND_MOVEGEN_ARCHITECTURE.md)** · **[Bitboard implementation](docs/BITBOARD_IMPLEMENTATION.md)**

## Acknowledgments

Huginn was inspired by and built upon the foundation of the **VICE Chess Engine** by
Richard Allbert (Bluefever Software). VICE (Video Instructional Chess Engine) provided
an excellent educational framework for chess-engine fundamentals — board representation,
move generation, and search. While Huginn has since evolved into its own pure-bitboard
architecture with a tapered, Texel-tuned evaluation and a modern C++ codebase, we
gratefully acknowledge VICE as the foundational inspiration that made this project possible.

VICE engine and instructional series: [VICE Chess Engine Playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)
