# Huginn Chess Engine Architecture Documentation

## Overview

Huginn represents a complete architectural rewrite that delivers dramatically improved chess playing strength and reliability. This document details the technical implementation and design decisions behind the current architecture.

## Architecture Components

### 1. Hybrid Evaluation System (`src/evaluation.hpp/cpp`)

Huginn implements a sophisticated hybrid evaluation system that combines the best of bitboard-based evaluation techniques with mailbox position representation.

#### Core Design Philosophy
- **Game Phase Awareness**: Dynamic evaluation adjustments based on material count and position characteristics
- **Comprehensive Analysis**: Multi-layered evaluation covering material, positional, tactical, and strategic factors
- **Performance Optimized**: Efficient evaluation suitable for deep search with ~70k-90k nps throughput

#### Game Phase Detection
```cpp
enum class GamePhase {
    OPENING,    // 24+ pieces remaining
    MIDDLEGAME, // 12-23 pieces remaining  
    ENDGAME     // <12 pieces remaining
};
```

The evaluation system dynamically adjusts weights and emphasis based on the detected game phase:
- **Opening**: Emphasizes development, king safety, and piece coordination
- **Middlegame**: Balances material, positional factors, and tactical opportunities
- **Endgame**: Prioritizes king activity, pawn promotion, and piece coordination

#### Evaluation Components

##### Material Evaluation
- **Base Values**: Pawn=100, Knight=300, Bishop=350, Rook=500, Queen=1000, King=10000
- **Phase Interpolation**: Piece values adjusted based on game phase
- **Imbalance Handling**: Sophisticated handling of material imbalances

##### Piece-Square Tables
- **All Pieces**: Comprehensive PSTs for pawns, knights, bishops, rooks, queens, and kings
- **Phase Dependent**: Different tables for opening/middlegame vs endgame
- **Positional Guidance**: Encourages piece placement on optimal squares

##### Pawn Structure Analysis
```cpp
// Pawn evaluation penalties and bonuses
inline constexpr int ISOLATED_PAWN_PENALTY = 15;
inline constexpr int DOUBLED_PAWN_PENALTY = 10;
inline constexpr int PASSED_PAWN_BONUS = 20; // varies by rank
```

- **Isolated Pawns**: Detection and penalty assessment (-15 cp)
- **Doubled Pawns**: Stacked pawn penalty (-10 cp)
- **Passed Pawns**: Progressive bonus based on advancement
- **Pawn Chains**: Support structure evaluation

##### King Safety Evaluation
- **Pawn Shield Analysis**: Evaluates pawn protection around the king
- **Attack Zone Evaluation**: Counts and weights enemy pieces attacking king vicinity
- **Castling Bonuses**: Incentivizes early king safety (+40 cp)
- **Endgame King Activity**: Encourages active king in endgames

##### Piece Activity Assessment
- **Knight Outposts**: Well-supported knights on strong squares (+25 cp)
- **Bishop Pairs**: Bonus for maintaining both bishops (+50 cp)
- **Rook Activity**: Open and semi-open file bonuses (+15 cp)
- **Development**: Early piece development bonuses (+15-20 cp in opening)

##### Mobility Evaluation
- **Dynamic Scoring**: Piece mobility weighted by game phase
- **Blocked Pieces**: Penalties for pieces with limited mobility
- **Central Control**: Bonuses for pieces controlling key central squares

### 2. Single-threaded Search Engine (`src/search.hpp/cpp`)

Huginn implements a robust single-threaded search engine that prioritizes reliability and correctness over threading complexity.

#### Core Search Algorithm
```cpp
int alpha_beta(Position& pos, int alpha, int beta, int depth, PVLine& pv);
int quiescence(Position& pos, int alpha, int beta, PVLine& pv);
```

##### Alpha-Beta Pruning
- **Minimax with Pruning**: Efficient tree search with alpha-beta cutoffs
- **Iterative Deepening**: Progressive depth increase from 1 to target depth
- **Depth Management**: Proper depth tracking and termination conditions

##### Quiescence Search
- **Tactical Extension**: Continues search in tactically sharp positions
- **Horizon Effect Prevention**: Avoids missing immediate tactical threats
- **Capture-only Search**: Extends search for captures and checks

#### Move Ordering Strategy
Huginn implements sophisticated move ordering to maximize alpha-beta efficiency:

1. **Hash Move**: Best move from transposition table (when available)
2. **Captures**: Ordered by MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
3. **Killer Moves**: Moves that caused cutoffs at the same depth
4. **History Heuristic**: Moves that have historically performed well
5. **Quiet Moves**: Remaining moves in natural order

#### Principal Variation Collection
```cpp
struct PVLine {
    std::vector<S_MOVE> moves;
    void clear();
    void add_move(S_MOVE move);
    std::string to_uci() const;
};
```

- **Complete PV Lines**: Collects and reports best move sequences
- **UCI Integration**: Properly formatted PV output for chess GUIs
- **Tactical Insight**: Shows engine's primary line of play

#### Search Features
- **Time Management**: Respects search time limits and can be stopped gracefully
- **Node Counting**: Accurate statistics for performance analysis
- **Search Extensions**: Handles special cases like check evasion
- **Mate Detection**: Accurate identification and scoring of mate positions

### 3. UCI Interface Integration (`huginn3_main.cpp`)

Huginn provides complete UCI (Universal Chess Interface) protocol compliance for integration with chess GUIs.

#### UCI Protocol Support
- **Engine Identification**: Reports as "Huginn Chess Engine v1.1"
- **Position Management**: Handles startpos, FEN, and move sequences
- **Search Commands**: go depth/time/nodes/infinite with proper handling
- **Real-time Information**: depth, nodes, time, nps, score, pv reporting

#### Move Encoding
```cpp
static std::string move_to_uci(const S_MOVE& move);
```

Huginn provides reliable move encoding that replaces the broken "0000" output of legacy systems:
- **Proper UCI Format**: e2e4, d7d5, e7e8q (promotion), e1g1 (castling)
- **Special Move Handling**: Castling, en passant, and promotion correctly encoded
- **Consistency**: All moves properly formatted for external consumption

## Performance Characteristics

### Search Performance
- **Node Rate**: 70,000-90,000 nodes per second (single-threaded)
- **Search Depth**: Typically reaches depth 5-8 in tournament time controls
- **Cut-off Efficiency**: High alpha-beta pruning rate due to effective move ordering
- **Memory Usage**: Efficient with minimal memory allocation during search

### Evaluation Performance
- **Evaluation Speed**: Fast enough for deep search without bottlenecking
- **Positional Accuracy**: Realistic assessment of complex positions
- **Game Phase Detection**: Efficient classification with minimal overhead

## Design Decisions and Trade-offs

### Single-threaded Architecture
**Advantages:**
- **Reliability**: No threading bugs or race conditions
- **Debugging**: Easier to debug and maintain
- **Consistency**: Predictable performance characteristics
- **Simplicity**: Clean, understandable codebase

**Trade-offs:**
- **Performance**: Lower node rate compared to optimized multi-threaded engines
- **Hardware Utilization**: Doesn't fully utilize multi-core systems

### Hybrid Evaluation Approach
**Advantages:**
- **Best of Both Worlds**: Combines bitboard evaluation sophistication with mailbox simplicity
- **Maintainability**: Easier to understand and modify than pure bitboard evaluation
- **Extensibility**: Simple to add new evaluation features

**Trade-offs:**
- **Performance**: Slightly slower than optimized bitboard-only evaluation
- **Complexity**: More complex than simple material counting

## Future Development Opportunities

### Search Enhancements
- **Transposition Table**: Hash table for position caching and move ordering
- **Search Extensions**: Check extensions, singular extensions
- **Late Move Reductions**: Reduce search depth for unlikely moves
- **Null Move Pruning**: Skip a move to detect zugzwang positions

### Evaluation Improvements
- **Tactical Patterns**: Pattern recognition for common tactical motifs
- **Endgame Knowledge**: Specialized endgame evaluation routines
- **Piece Coordination**: Evaluation of piece harmony and cooperation
- **Dynamic Positional Factors**: Time-dependent positional assessments

### Performance Optimizations
- **Bitboard Integration**: Hybrid approach with more bitboard techniques
- **SIMD Instructions**: Vectorized operations for bulk calculations
- **Memory Optimization**: Cache-friendly data structures and access patterns

## Conclusion

Huginn represents a significant advancement in the Huginn chess engine project, delivering:

1. **Reliable Chess Playing**: Accurate position evaluation and sound move selection
2. **Clean Architecture**: Maintainable, extensible codebase
3. **UCI Compliance**: Full integration with chess GUIs and tournaments
4. **Performance**: Competitive search speed and evaluation quality

The hybrid evaluation system and single-threaded search provide an excellent foundation for future enhancements while delivering immediate improvements in chess playing strength and reliability.
