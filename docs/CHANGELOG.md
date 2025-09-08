# Huginn Chess Engine - Changelog

## Version 1.1 (August 2025)

### 🚀 Major Release - Engine3 Integration

This release represents a complete architectural overhaul of the Huginn chess engine, delivering dramatically improved chess playing strength and reliability.

#### **🎯 Engine Architecture Overhaul**
- **Complete Engine3 Rewrite**: Brand new chess engine architecture replacing legacy implementation
- **Hybrid Evaluation System**: Advanced bitboard evaluation techniques adapted for mailbox position representation
- **Single-threaded Design**: Clean, reliable search implementation without threading complexity
- **Production Ready**: Full UCI protocol compliance for tournament and GUI integration

#### **🧠 Advanced Evaluation System (`Engine3_src/hybrid_evaluation.hpp/cpp`)**
- **Game Phase Detection**: Dynamic evaluation adjustments for opening/middlegame/endgame phases
- **Comprehensive Material Analysis**: Sophisticated piece values with positional bonuses
- **Pawn Structure Evaluation**: 
  - Passed pawn detection and scoring
  - Isolated pawn penalties (-15 cp)
  - Doubled pawn penalties (-10 cp)
  - Pawn shield analysis for king safety
- **Piece Activity Assessment**:
  - Knight outposts (+25 cp bonus)
  - Bishop pair bonuses (+50 cp)
  - Rook open file bonuses (+15 cp)
  - Development incentives (+15-20 cp in opening)
- **King Safety Analysis**: Attack zone evaluation with pawn shield assessment
- **Mobility Scoring**: Dynamic piece mobility evaluation with phase-dependent weights

#### **🔍 Robust Search Engine (`Engine3_src/simple_search.hpp/cpp`)**
- **Alpha-Beta Pruning**: Efficient minimax search with alpha-beta pruning
- **Quiescence Search**: Tactical extension search preventing horizon effect
- **Iterative Deepening**: Progressive depth increase from 1 to target depth
- **Sophisticated Move Ordering**:
  - Hash moves (from transposition table)
  - MVV-LVA capture ordering (Most Valuable Victim - Least Valuable Attacker)
  - Killer move heuristic
  - History heuristic for quiet moves
- **Principal Variation Collection**: Complete PV lines showing best tactical sequences
- **Time Management**: Proper handling of time controls and search limits

#### **🐛 Critical Bug Fixes**
- **Evaluation Perspective Error**: Fixed major bug where positions showed +578 cp instead of realistic evaluations
- **Move Encoding Failure**: Replaced broken "0000" move output with proper UCI moves (d2d4, e2e4, etc.)
- **Search Instability**: Eliminated search termination issues and infinite loops
- **Mate Detection**: Accurate mate scoring (+/-32000 cp) and proper mate detection

#### **⚡ Performance Improvements**
- **Search Speed**: 70,000-90,000 nodes per second (single-threaded architecture)
- **Evaluation Quality**: Realistic position assessments replacing evaluation bugs
- **Move Generation**: Proper UCI move formatting and encoding
- **Search Stability**: Consistent search behavior with proper termination
- **Memory Efficiency**: Clean, leak-free implementation with optimal data structures

#### **🏗️ Build System and Testing Updates**
- **Updated CMakeLists.txt**: Engine3 now builds as main `huginn.exe` executable
- **Migrated Test Suite**: All 243 tests updated to use Engine3 components instead of legacy
- **Legacy Preservation**: Original engine available as `huginn_legacy.exe` for comparison
- **Versioned Executables**: `huginn_v1.1.exe` for specific version identification

#### **📁 File Structure Changes**
- **Engine3_src/**: New engine implementation directory
  - `simple_search.hpp/cpp`: Single-threaded search engine with alpha-beta and quiescence
  - `hybrid_evaluation.hpp/cpp`: Comprehensive position evaluation system
- **huginn3_main.cpp**: UCI interface wrapper (now main huginn executable)
- **Legacy Components**: Preserved in `src/` for `huginn_legacy.exe`

#### **🎮 UCI Interface Enhancements**
- **Real-time Search Information**: Depth, nodes, time, NPS, score, and PV reporting
- **Proper Move Encoding**: All moves correctly formatted as UCI strings
- **Time Management**: Respects search limits and can be stopped gracefully  
- **Engine Identification**: Reports as "Huginn Chess Engine v1.1"

#### **🧪 Quality Assurance**
- **Comprehensive Testing**: All existing tests pass with Engine3 integration
- **Mate Detection Verification**: Correctly identifies and reports checkmate positions
- **Position Analysis**: Realistic evaluation of complex positions (famous mate-in-1 tests)
- **UCI Compliance**: Full protocol compatibility with chess GUIs

#### **📊 Before vs After Comparison**
| Metric | Legacy Engine | Engine3 v1.1 |
|--------|---------------|---------------|
| **Position Assessment** | +578 cp (wrong) | -50 cp (realistic) |
| **Move Output** | "0000" (broken) | "d2d4" (proper UCI) |
| **Search Stability** | Unstable termination | Stable with proper limits |
| **Mate Detection** | Inconsistent | Accurate +/-32000 cp |
| **Performance** | Variable with bugs | Consistent 70k-90k nps |
| **Code Quality** | Complex threading | Clean single-threaded |

---

## Version 1.0 (Previous)
  - `simple_search.hpp/cpp`: Search engine with alpha-beta and quiescence
  - `hybrid_evaluation.hpp/cpp`: Comprehensive position evaluation
- `huginn3_main.cpp`: UCI interface (now main huginn executable)
- Legacy engine preserved as `huginn_legacy`

**Compatibility:**
- Maintains all existing position/move generation systems
- Backward compatible with existing FEN/PGN workflows
- Enhanced UCI output with proper timing and node statistics

---

## Version 1.0 (Previous)

**Features:**
- Basic chess engine with move generation
- UCI interface
- Position evaluation
- Search functionality
- Multi-threading support

**Known Issues (Fixed in 1.1):**
- Evaluation perspective errors
- Move encoding producing "0000" moves
- Search instability
- Performance limitations
