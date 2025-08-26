# Huginn Chess Engine - Changelog

## Version 1.1 (August 2025)

### 🚀 Major Release - Engine3 Integration

**Engine Architecture Overhaul:**
- **New Engine3 System**: Complete rewrite with hybrid evaluation and single-threaded search
- **Hybrid Evaluation**: Adapted Engine2's advanced bitboard evaluation for mailbox position representation
- **Fixed Critical Bugs**: Resolved evaluation perspective errors (+578 cp → realistic assessments)
- **Move Encoding Fix**: Proper UCI move output (d2d4, c4c5) instead of broken "0000" moves

**Technical Improvements:**
- **Search Engine**: Single-threaded alpha-beta with quiescence search and iterative deepening
- **Move Ordering**: Captures, killer moves, and history heuristic for improved search efficiency
- **Evaluation Components**: 
  - Material counting with piece-square tables
  - Pawn structure analysis (doubled, isolated, passed pawns)
  - Piece activity and mobility scoring
  - King safety evaluation
  - Game phase detection (opening/middlegame/endgame)

**Performance Gains:**
- **Speed**: ~70k-90k nodes per second (single-threaded)
- **Accuracy**: Realistic position evaluations (-50 cp vs previous +578 cp bug)
- **Stability**: Proper search termination and PV collection
- **UCI Compliance**: Full UCI protocol support with correct move formatting

**File Structure:**
- `Engine3_src/`: New engine implementation
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
