# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Huginn is a UCI-compliant chess engine written in modern C++17/20. As of
the `pure-bitboard-engine` branch (Phase 4 of the rewrite), the data flow
is **pure bitboard**: per-piece bitboards are the sole source of truth for
movegen and evaluation. Squares are indexed 0–63 (a1=0, h8=63). The old
mailbox-120 board and piece lists were removed in Phase 4.8b; piece
location is derived from the bitboards via `Position::at_sq64()`.

**Current Status (`pure-bitboard-engine` branch, 2026-05-16):**
- ✅ **Functional UCI engine**: tested with Arena and direct UCI piping
- ✅ **Baseline tag**: `baseline-t34` — **#42 idea 1: date-based TT aging**
  (`ENABLE_TT_AGING`, default ON; road-to-2.3 item 2). A 6-bit search date
  packed in the TT entry's node_type byte; `new_search()` bumps the global
  date per search; stale-dated entries are evictable regardless of depth;
  probe hits re-date. Fixes the depth-preferred-only squatting problem (a
  deep entry stored early in a game blocked its slot forever). **Shipped on
  the LTC verdict** (pre-registered rule): blitz was inconclusive at t23
  (AMD +0.69 flat / Intel +11.12 lean) but the 60+0.6 leg — where searches
  per game accumulate the staleness aging fixes — came in **+15.99 ± 17.00,
  LOS 96.77%, 500g** (52.30%, DrawRatio 51.6%). ⚠ Signature: first-search
  d14 = **3,481,582** IDENTICAL to t33 by design (aging acts only from the
  second search of a process on) — verify this arm via the 8 gated TT tests
  (`test_transposition_table.cpp`), not node counts; 282/283 green (1
  by-design skip). **Full writeup:** [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t33` — **#63 history-modulated LMR**
  (`ENABLE_HISTORY_LMR`, default ON; road-to-2.3 item 1). At the LMR site
  the mover's butterfly-history score adjusts the static `log·log` table
  reduction by ±1 ply (grain ±4096, `[1, depth−2]` clamps still apply):
  proven-good quiets are reduced less, history-hated quiets more. Fourth
  straight ship of the SF18-study selectivity program (#62 → #17-r2 → #63).
  **Two-machine SPRT vs t32, both legs positive: AMD +8.69 ± 15.27 (LOS
  86.78%) / Intel +18.43 ± 15.05 (LOS 99.19%), pooled +13.63 ± 10.72, LOS
  ≈ 99.4%, 2000g** — widest per-leg spread of the series, both positive.
  Signature: startpos d14 = **3,481,582** / cp 31 / e2e4 (−38.6% vs t32 —
  the biggest fixed-depth cut of the series); Kiwipete d13 = 1,958,182 /
  cp −85 / e2a6; 274/275 green (1 by-design skip). **Full writeup:**
  [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t32` — **#17-r2 aspiration windows at the
  root** (`ENABLE_ASPIRATION`, default ON). From depth ≥ 6 the root searches
  a `[prev−50, prev+50]` window around the previous iteration's score; a
  fail-low/high widens that side ×2 around `best_score`, snapping to the full
  window past delta 800 or on mate-range scores. RE-TEST ship: attempt 1 was
  H0-rejected at t15 (−33.8) and reverted, but that verdict predates the
  #44/#50/#52/#57/#58 soundness fixes that caused the inter-iteration score
  instability aspiration thrashes on — the contaminated-verdict hypothesis
  (#45 precedent) predicted today's outcome. **Two-machine SPRT vs t31, both
  legs positive: AMD +12.51 ± 15.12 (LOS 94.78%) / Intel +16.34 ± 14.88 (LOS
  98.45%), pooled +14.46 ± 10.61, LOS ≈ 99.6%, 2000g.** Signature: startpos
  d14 = **5,669,691** / cp 33 / e2e4 (−13.9% vs t31 — cheaper root
  iterations); Kiwipete d13 = 2,768,609 / cp −88 / e2a6; 271/272 green (1
  by-design skip). **Full writeup:** [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t31` — **#62 singular extensions: the
  SF18-gap-study EBF lever** (`ENABLE_SINGULAR_EXT`, default ON). At a
  non-root, non-check node with depth ≥ 8 whose TT entry has a
  LOWER_BOUND/EXACT non-mate score at `tt_depth >= depth−3` and a best move,
  a reduced-depth `(depth−1)/2` exclusion search of every *other* move at a
  null window below `tt_score − 2·depth`; fail-low ⇒ the TT move is singular
  ⇒ searched one ply deeper (no TT cut/store, no null-move, no PV write at
  exclusion nodes). **Two-machine SPRT vs t30, both legs positive: AMD
  +12.17 ± 15.39 (LOS 93.97%) / Intel +17.39 ± 14.67 (LOS 99.01%), pooled
  +14.90 ± 10.62, LOS ≈ 99.7%, 2000g** — first search-shape ship since t27.
  Signature: startpos d14 = **6,583,846** / cp 24 / e2e4 (−20.7% vs t30's
  8,298,375 — sharper tree, forced lines verified deeper: Kiwipete seldepth
  30 vs 25); 268 tests green (1 by-design skip). **Full writeup:**
  [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t30` — **#9 threats round 2: hanging /
  pawn-push / king-ring eval terms** (`ENABLE_THREATS_R2`, default ON).
  Three eval threat classes layered on t15's threats round 1: hanging units
  (attacked + undefended), safe pawn-push threats, hanging units in our
  king's ring. Params Texel-fitted `--only-new` (six new params fit, rest of
  the 841-vector frozen for clean SPRT attribution): HANGING 9/18, PAWN_PUSH
  10/4, BY_KING −5/34 (MG/EG). **Two-machine SPRT vs t29, both legs positive:
  AMD +9.73 ± 14.88 (LOS 90.02%) / Intel +24.01 ± 14.55 (LOS 99.94%), pooled
  +17.0 ± 10.4, LOS ≈ 99.9%, 2000g** — largest eval-term ship since t15.
  Signature: startpos d14 = **8,298,375** / cp 26 / e2e4 (+51% vs t29's
  5,485,978 — the new terms reshape the tree); 265 tests green (1 by-design
  skip). **Full writeup:** [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t29` — **#59: en-passant key semantics.**
  The EP right is normalized at the source (`MakeMove` + `set_from_fen`
  store it only when a side-to-move pawn can pseudo-capture — Polyglot /
  X-FEN convention), fixing missed threefolds and TT splits from decorative
  EP squares; the Polyglot book-key EP check was also one rank off (book
  keys wrong in exactly the capturable-EP positions) — now verified against
  the spec anchor keys. Unconditional fix (#50/#51 precedent). **AMD
  regression gate vs t28: clean, +8.34 ± 15.32, LOS 85.73%, 1000g.**
  Signature: startpos d14 = **5,485,978** / cp 26 / e2e4 (−23% — decorative
  EP no longer splits transpositions).
  Prior: `baseline-t28` — **#58: pin-aware first recapture in
  SEE** (`ENABLE_SEE_LEGALITY`, default ON): pinned defenders no longer make
  winning captures look losing to qsearch's hard prune (pin-line captures
  still count; deeper swap plies stay geometric). Two-machine SPRT vs t27
  both ~neutral-positive — AMD +5.56 / Intel +8.69, **pooled +7.2 ± 10.5,
  LOS ≈ 91%** — shipped on cross-machine agreement + correctness (user
  call). Signature: startpos d14 = 7,128,502 / cp 30 / e2e4.
  Prior: `baseline-t27` — **#57: legal-move ordinal for
  PVS/LMR + textbook PVS condition** (`ENABLE_LEGAL_MOVE_ORDINAL`, default
  ON). The move loop's pseudo-legal index drove PVS first-move treatment,
  LMR lateness/row, and fhf (pinned entries inflate it), and the PVS
  condition `i == 0 || alpha == best_score` made every move after an alpha
  improvement full-window — null-window PVS only engaged in failing-low
  nodes. Fixed with a searched-move ordinal + textbook PVS. **AMD SPRT vs
  t26: H1 ACCEPT +29.98 ± 15.53, LOS 99.99%, 976g early-stop** — shipped on
  AMD-only accept (user call, #51 precedent). Signature: startpos d14 =
  7,484,807 / cp 23 / d2d4 (+12.8% fixed-depth nodes yet stronger at fixed
  time — the reshaped tree is better, not smaller). **Full writeup (all baselines):**
  [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t26` — **2026-07-09 audit criticals #52 +
  #53, shipped together** (flags `ENABLE_QSEARCH_CHECK_EVASIONS` +
  `ENABLE_RULE50_TT_GUARD`, both now default ON). **#52 check-aware
  quiescence** (the Elo carrier — same "blind at the horizon" soundness
  class as #44/#45): qsearch detects check at entry; in check no
  stand-pat/delta/SEE, every evasion searched, `-MATE + ply` when none is
  legal; quiet promotions join the frontier; `info.ply` advances through
  qsearch. **Two-machine SPRT vs t25, both legs H1-accept: Intel +40.11 ±
  18.18 @696g / AMD +44.67 ± 18.94 @610g (both LOS 100%), pooled ≈ +42 Elo
  over 1306g.** **#53 rule-50-aware TT eligibility** (#29 follow-up): no TT
  cutoff/store when `halfmove_clock + depth >= 100`; blitz SPRT sign-split
  (Intel −18.08 / AMD +5.21, pooled ≈ −6 Elo / 2000g) — **shipped on
  correctness+tests by user call** (#50/#51 precedent; the payoff is
  path-independent scores in long shuffle endgames). Ship build verified:
  startpos d14 = 6,634,033 (= #52 solo arm, rule-50 guard inert at low
  clocks by design), mate-in-1 discriminator `mate 1 g6g7` at d1, rule-50
  warm/fresh oracle equality (`cp 1211 / h2d6`); 216/217 tests (1 by-design
  skip). **Full writeup (all baselines):** [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t25` — **#51: history-heuristic
  piece-index collision fix.** `search_history[13][64]` (and the gated
  continuation-history table) indexed with `static_cast<int>(piece) % 13`
  on the packed `Piece` enum (`color<<3|type`), which folds `BlackKing=14`
  into `WhitePawn`'s row (`14 % 13 == 1`) — quiet White-pawn and Black-king
  moves to the same square shared one history cell. Same bug species as
  #50, found while comparing Huginn's design against a sibling JS engine.
  Fixed via a `history_piece_row()` helper reusing `zobrist.hpp`'s already-
  correct 13-row convention. Fixed unconditionally, no flag — shipped
  directly to `main` ahead of the flag/branch/SPRT-queue process, same as
  #50. **AMD SPRT vs t24: +19.48 ± 15.00, LOS 99.46%, 1000g** (rounds cap
  reached before LLR crossed the SPRT bound, but CI excludes zero) —
  **shipped on AMD-only accept** (user call), no Intel leg, mirroring t23's
  own single-machine-accept precedent below.
  Prior: `baseline-t24` — **SPRT queue winners: SEE good/bad
  capture ordering (#6) + root two-fold draw-avoidance (#44 follow-up).**
  Screened 10 candidate branches off t23 (8 parked/rejected — razoring-off,
  rfp-pv-guard, futility-depth2, futility-pv-guard, tt-aging,
  drawishness-scaling, trapped-bishop, pext — full numbers in
  [BACKLOG.md](BACKLOG.md)); the two winners combined into `candidate/t24`,
  verified (both-OFF reproduces t23 exactly; both-ON matches see-ordering's
  standalone signature; 204/205 tests), then merged. **Two-machine SPRT of
  the combined candidate vs t23 — both legs H1-ACCEPT:** AMD **+48.84 ± 20.36**
  (LOS 100%, 580g, LLR 2.98) / Intel **+66.33 ± 23.61** (LOS 100%, 440g, LLR
  2.97) — pooled 58.04% / 1020g. Undershoot guard clean on both legs (no
  masked negative interaction; Intel even shows the two winners outperforming
  either alone). **Full writeup (all baselines):** [BASELINE_LADDER.md](BASELINE_LADDER.md).
  Prior: `baseline-t23` — **#50: Zobrist black-king row out-of-bounds fix**
  (`Piece[PIECE_NB=12][64]` was one row short of the 13-row piece-index
  scheme; the black king's row read 64 `U64`s past the table, producing both
  the Kiwipete nondeterminism and real TT key collisions on 5 other squares,
  carried by every baseline ever). Fixed unconditionally, no flag. **AMD SPRT
  vs t22: H1 accepted @872g, +33.97 ± 16.60, LOS 100%.** The fix shifted the
  Zobrist RNG draw sequence, so fixed-depth signatures moved (startpos d14:
  12,035,479→14,306,844 nodes) — see
  [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md) for the reference
  numbers. Prior:
  `baseline-t22` — **speed pair #48 (double-TT-probe kill) + #49 (king-safety
  scan fused into the mobility pass)**, both verified behavior-identical
  (byte-identical fixed-depth node counts, 203/203 tests) for **+11% nps
  startpos / +6% Kiwipete** combined. Two-machine SPRT vs t21, both legs
  positive — **AMD +14.60 ± 15.69 (LOS 96.6%) / Intel +18.08 ± 14.44 (LOS
  99.3%)**, pooled 52.35% / 2000g — cross-machine-agreement ship. Prior:
  `baseline-t21` — **TT-clear-on-newgame (#46) + time-management fix (#47)**,
  both surfaced by watching a real 5+2 game; **+126.97 ± 24.60 vs t20** (10+0.1,
  400g, LOS 100%, zero time-forfeits). **Full history in
  [BASELINE_LADDER.md](BASELINE_LADDER.md).** Recent: t34 TT aging ·
  t33 history-modulated
  LMR · t32 aspiration windows ·
  t31 singular extensions · t30 threats round 2 ·
  t29 EP key semantics · t28 SEE pin legality · t27 legal-move
  ordinal PVS/LMR · t26 check-aware
  qsearch + rule-50 TT guard · t25 history-heuristic
  collision fix · t24 SEE ordering + root-twofold · t23 zobrist OOB fix ·
  t22 speed pair · t21 TT-clear +
  time-mgmt · t20 move-level futility · t19 safe mobility · t18
  mate-distance pruning · t17 #44 repetition fix · t16 king safety · t15 threats.
- ✅ **Comprehensive test suite**: 225 GoogleTest cases (224 run + 1 by-design
  skip — the `RootTwofoldAvoid` opposite-arm test pair) plus a registered
  `perft_quick` CTest job (startpos + Kiwipete to depth 5). `--target check`
  runs all of it and fails on empty discovery (#60)
- ✅ **Strength**: **pre-`v2.3` (`t34` + UCI-only changes) ≈ 2600–2680
  CCRL-blitz, pooled point estimate ~2625 ± 18** (2026-07-16, AMD box,
  10+0.1, cc=1) — three-anchor calibration, road-to-v2.3 item 6: Stash
  19.0 (2473) +132.28 ± 33.41 (68.17%, 300g, user-stopped) → ~2605; Stash
  20.0.1 (2509) +93.95 ± 26.75 (63.20%, 500g) → ~2603; Stash 21.0 (2714)
  −38.37 ± 33.22 (44.50%, 300g, user-stopped) → ~2676. **Reported as a
  range, not a false-precision point:** the 19/20 pair agrees within ~2
  Elo of each other, but Stash 21 — a different engine generation, not
  just a rating-ladder rung — pins ~72 Elo higher, a real lean (~1.8
  combined SE) rather than noise, the same non-transitivity pattern seen
  elsewhere on this ladder (CDrill bogey, MTL favorable). Zero Huginn
  timeouts (Stash 21.0 had one). Externally measured gain since t26's 2571
  is **≈ +33 to +54** against **≈ +122 summed self-play (t27→t34)** — a
  much stronger compression (~0.3–0.4) than t26's own ~0.86, i.e. this
  round's selectivity-heavy gains (extensions, aspiration, LMR modulation,
  TT aging) transferred to external play less linearly than prior rounds.
  **Next recalibration: drop Stash 19 (least informative rung now), use
  20/21/21.2 as the three-anchor bracket** (user call, 2026-07-16). Full
  writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md). Prior: **t26 ≈ 2571 ±
  19 CCRL-blitz** (2026-07-11, AMD box, 10+0.1, 500g per anchor, cc=1) —
  two-anchor inverse-variance pool of
  Stash 19.0 (2473) +102.97 ± 25.74 (64.40%) → ~2576 and Stash 20.0.1
  (2509) +56.07 ± 27.03 (58.00%) → ~2565; the two pins agree within 11 Elo
  (same-author anchor family, so style non-transitivity is minimized).
  Zero Huginn timeouts. Measured externally that's **≈ +137 since v2.2/t21**
  (blitz-vs-LTC caveat) against +160 summed self-play — mild ~0.86
  compression. Prior: **t20 ≈ 2350–2390 CCRL-ladder** (2026-06-27, ≈ +510 over t19) —
  non-saturated pins: Stash 17.0 (2298) 56.75%/+47 → ~2345, MTLChess v0.5 (2314)
  61% → ~2392 (+9pp-favorable MTL non-transitivity). **t21 (+127 self-play over t20,
  clock-usage fix) at LTC: vs Stash 17.0 (2298) 68.65% / +136 ± 20 over 1000g at
  60+0.6 → ~2434** — markedly above t20's blitz ~2345 vs the same anchor (the longer
  clock is where #47 pays); zero Huginn time-forfeits. This is the `v2.2` rating.
  t19 was ~1834 (t17 vs Stash 12.0 42.58%, +48 from #44). The earlier 3-anchor MLE was **~1818 ±
  30 Elo** (10+0.1, no book) as of
  baseline-t11, June 2026 — 3-anchor pooled MLE over 600 games vs Snowy 0.2
  (1868), CDrill 2000 (1949), MTLChess v0.3 (1984). Big jump from the old
  "~1500-1700" (that figure predates the t5→t11 stack: magic bitboards, TT-bound
  fix, tapered eval, Texel tune). Note real style non-transitivity (CDrill is a
  bogey −7.5pp; MTL/MORA favorable +9pp), so treat as ~1800–1850. See BACKLOG #5
  + [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for the calibration ladder
- 🔬 **Performance**: **~3.55 Mnps** single-threaded at depth 11 from
  startpos (post-#24 magic bitboards; up from ~2.06 Mnps pre-t5)

For the **live inventory** of which search and evaluation techniques are
implemented, see [SEARCH_AND_EVAL.md "Current state" sections](SEARCH_AND_EVAL.md#current-state--search).
For **what to work on next**, see [BACKLOG.md](BACKLOG.md). **Before changing
search, eval, or make/unmake, read [INVARIANTS.md](INVARIANTS.md)** — the
load-bearing contracts that span functions (colour symmetry, `pos.ply` vs
`move_history.size()`, TT path-independence, the three value tables, flag/ship
discipline). This file is the orientation guide for new sessions; the other
three are the source of truth for state, priorities, and contracts.

## Common Commands

### Building the Project

The project uses CMake with presets for consistent builds across environments.

**Quick start (MSVC/Windows):**
```powershell
# Configure using CMake preset (Release build)
cmake --preset msvc-x64-release

# Build all targets
cmake --build build/msvc-x64-release --config Release

# Build specific targets
cmake --build build/msvc-x64-release --config Release --target huginn
cmake --build build/msvc-x64-release --config Release --target huginn_tests
```

**Alternative build commands:**
```powershell
# GCC/Linux builds
cmake --preset gcc-x64-release
cmake --build build/gcc-x64-release

# Clean build (when needed)
Remove-Item -Recurse -Force build/msvc-x64-release
cmake --preset msvc-x64-release
```

### Running Tests

```powershell
# Build and run all tests
cmake --build build/msvc-x64-release --config Release --target check

# Run tests manually with CTest
cd build/msvc-x64-release
ctest -C Release --output-on-failure -V

# Run specific test patterns
ctest -C Release -R "perft|position"

# Run test executable directly
.\build\msvc-x64-release\bin\Release\huginn_tests.exe
```

### Running the Chess Engine

```powershell
# Main UCI Chess Engine
.\build\msvc-x64-release\bin\Release\huginn.exe

# Mirror evaluation test (symmetric evaluation verification)
cd mirror_test
.\run_mirror_test.ps1
```

### Performance Analysis

```powershell
# Generate assembly for performance analysis
cmake --preset msvc-x64-release-asm
cmake --build build/msvc-x64-release-asm --config Release --target generate_assembly
```

## Architecture Overview

### Core Components

- **UCI Interface** ([src/uci.cpp](../src/uci.cpp), [src/uci_utils.cpp](../src/uci_utils.cpp)):
  Full UCI protocol — position setup, `go` parsing, search control, real-time `info` output
- **Position Representation** — pure bitboard:
  - [src/position.cpp](../src/position.cpp) / [src/position.hpp](../src/position.hpp):
    `Position` class with per-piece bitboards (`std::array<std::array<Bitboard, _Count>, 2>`),
    `S_MOVE` system, and incremental Zobrist + material updates. Piece-on-square
    lookup (`at_sq64()`) is derived from the bitboards; there is no mailbox
    board and no piece lists (both removed in Phase 4.8b).
- **Move Generation** — two entry points, both backed by the bitboard core:
  - [src/movegen_bb.cpp](../src/movegen_bb.cpp) / [.hpp](../src/movegen_bb.hpp):
    bitboard-based generation — **the production movegen**
  - [src/movegen.cpp](../src/movegen.cpp) / [.hpp](../src/movegen.hpp):
    legacy `S_MOVELIST` entry points that delegate to `BitboardMoveGen`;
    retained for the move-list container and tests
  - [src/attack_tables.cpp](../src/attack_tables.cpp), [src/{knight,king,pawn}_lookup_tables.cpp](../src/):
    pre-computed attack tables; sliders use magic bitboards
- **Search Engine** ([src/search.cpp](../src/search.cpp)):
  see [SEARCH_AND_EVAL.md "Current state — Search"](SEARCH_AND_EVAL.md#current-state--search)
  for the full audited inventory. High-level: negamax + alpha-beta with PVS,
  iterative deepening, TT, quiescence with SEE pruning, null-move (R=4),
  RFP, futility, razoring, LMR (`log·log` table), check extension, IID,
  killers + history + MVV-LVA + counter-move (on @1500, BACKLOG #15),
  polyglot book.
- **Evaluation**: definitions in [src/evaluation.hpp](../src/evaluation.hpp)
  (constants + masks + PSTs); the actual `Engine::evaluate()` lives in
  [src/search.cpp](../src/search.cpp). Hand-crafted, no NNUE. **Tapered** via
  `game_phase_256()` (smooth MG/EG blend, not the legacy 3-bucket switch). Terms:
  material + PSTs (separate MG/EG king table), isolated/doubled/passed
  pawns, bishop pair, rook/queen on open/semi-open files, mobility,
  tempo, insufficient-material draw. See
  [SEARCH_AND_EVAL.md "Current state — Evaluation"](SEARCH_AND_EVAL.md#current-state--evaluation)
  for the full audited inventory.

### Key Design Patterns

- **S_MOVE System**: High-performance 25-bit packed move representation with integrated scoring
- **Single Representation**: per-piece bitboards are the sole board state; piece-on-square is derived via `at_sq64()`
- **Incremental Updates**: O(1) make/unmake operations with complete state restoration
- **Type Safety**: Extensive use of C++17 type-safe enums and constexpr functions
- **Performance Focus**: Zero-copy operations, efficient bit manipulation, magic-bitboard slider attacks
- **Comprehensive Testing**: 197 automated tests covering all major components with 100% pass rate

### Move System Architecture

**Production**: S_MOVE with 25-bit packed encoding (from/to/promotion + flags)
plus an in-band `score` field used for move ordering. Make/unmake is
`MakeMove`/`TakeMove` on `Position` with O(1) incremental updates to
the per-piece bitboards, Zobrist key, and material counts.

### Bitboard Infrastructure

- **Per-piece bitboards**: `std::array<std::array<Bitboard, PieceType::_Count>, 2>`
- **Magic bitboards** for slider attacks (rook / bishop), pre-computed
  attack tables for knight / king / pawn
- **Square indexing**: 0–63 (a1=0, h8=63); see [src/square.hpp](../src/square.hpp).
  No mailbox board — piece-on-square is derived from the bitboards via `at_sq64()`

## Development Guidelines

### Code Style
- Modern C++17/20 features preferred
- Type-safe enums over integer constants
- `constexpr` for compile-time computations
- Comprehensive debug assertions when `DEBUG` is defined
- Zero-copy operations and minimal memory allocations

### Testing Requirements
- All new functionality must have corresponding tests in `test/` directory
- **Current size**: ~203 GoogleTest cases across 34 files (verify the
  pass count by running the suite — it changes as features land)
- Perft tests for move generation validation
- Position validation with FEN round-trip testing
- Evaluation symmetry tests via [test/test_evaluation_symmetry.cpp](../test/test_evaluation_symmetry.cpp)
  and the mirror harness
- Tactical sweeps via WAC300 ([test/WAC300.epd](../test/WAC300.epd))
  and LCT2 ([test/lct2.epd](../test/lct2.epd))

### Performance Considerations
- **Current NPS**: **~3.55 Mnps** single-threaded (`go depth 11` from
  startpos, 2026-05-16 measurement, post-#24 magic bitboards). Now
  ~1.5× MTLChess_v0.3 (~2.33 Mnps). Speed-side parity reached; the
  remaining strength gap is **search-tree shape and eval quality**,
  not raw speed.
- **Strength priorities** are tracked in [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md)
  by tier; per-feature Elo deltas are gauntlet-measured at 100+ games
- Assembly generation available for micro-optimization analysis (see
  `cmake --preset msvc-x64-release-asm`)

### Build System Notes
- Uses CMake 3.22+ with preset support
- Multi-compiler support (MSVC, GCC)
- Automatic GoogleTest integration via FetchContent
- Assembly generation option for performance analysis
- Aggressive MSYS2 interference prevention for pure MSVC builds

## File Organization

- `src/`: Core engine source files
- `test/`: Comprehensive test suite (197 tests across 34 files)
- `docs/`: Extensive documentation including API reference, architecture guides
- `benchmark/`: Performance tracking and benchmark results
- `mirror_test/`: Evaluation symmetry validation
- `build/`: Build output directories (created by CMake)

## Important Constants

- **Quiescence Search**: Limited to 10 plies (`MAX_QUIESCENCE_DEPTH`) - prevents stack overflow
- **Null Move Pruning**: Enhanced with R=4 reduction and minimum depth of 5 - 1.4% performance gain
- **Board Representation**: per-piece bitboards; squares indexed 0–63 (a1=0, h8=63)
- **S_MOVE System**: 25-bit packed move encoding with integrated scoring
- **UCI Protocol**: Full implementation compatible with Arena, Fritz, ChessBase, all major GUIs

## Current Engine Status (`pure-bitboard-engine` branch)

The codebase is mid-rewrite to a pure-bitboard architecture. Phase 3 +
Phases 4.1-4.2 are done (data flow is bitboard-only; `BITBOARD_ENGINE`
shim removed). Sub-phases 4.3-4.10 remain on the plan.

### What works today
1. **UCI**: position setup, search control, real-time `info` output, EPD
   parsing
2. **Search**: negamax + alpha-beta + PVS, iterative deepening, TT,
   quiescence with SEE pruning, full pruning stack (null-move, RFP,
   futility, razoring, LMR with `log·log` table), check extension, IID,
   killers + history + counter-move (on @1500) + MVV-LVA
3. **Evaluation**: hand-crafted, phase-aware (3-bucket switch), full set
   of standard pawn-structure / piece-activity / mobility terms
4. **Books / TBs**: Polyglot wired and used; Syzygy WDL probe **wired and
   active** (BACKLOG #10 closed) — fires at leaf nodes (`depth <= 1`) when
   tablebases are loaded ([search.cpp:1200](../src/search.cpp#L1200)),
   plus a root probe. WDL results are mapped to Huginn's own score scale
   (`±(MATE-1000)` = ±28000, inside `INFINITE`=30000), not Fathom's
   `TB_VALUE_*` constants. TB scores are deliberately **not** stored in
   the TT (rule50 isn't in the zobrist key)
5. **Test suite**: ~203 GoogleTest cases + WAC300 / LCT2 EPD sweeps

### Where we are vs target strength
- Calibration baseline: ~1500-1700 Elo at 10+0.1 (huginn current vs
  MTLChess_v0.3 ~1984 Elo, 0-20 score). The cumulative shipped Tier 1+2
  stack is roughly +50-100 Elo over the `t1` baseline; see
  [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) for live gauntlet numbers.
- Near-term roadmap is in [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md)
  Tier 1/2/3 tables; live priorities in [BACKLOG.md](BACKLOG.md).
- Notable gaps (not yet implemented): aspiration windows, singular
  extensions, ProbCut, continuation history, SEE in main search,
  smooth tapered eval, king safety, NNUE, lazy SMP.