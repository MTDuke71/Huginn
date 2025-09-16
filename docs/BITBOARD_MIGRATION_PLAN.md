# Huginn Bitboard Migration Plan

## Executive Summary

Transition Huginn chess engine from piece lists + mailbox-120 to pure bitboard architecture over multiple phases, maintaining performance and stability throughout the migration.

## Current State Analysis

### Existing Infrastructure ✅
- **Bitboard types**: `uint64_t` based with `setBit`/`popBit` macros
- **Complete bitboards maintained**: `piece_bitboards[color][type]`, `color_bitboards[color]`, `occupied_bitboard`
- **Bitboard operations**: Basic set/clear/test operations available
- **Coordinate conversion**: `MAILBOX_MAPS.to64[]` for mailbox-120 → bitboard conversion

### Current Performance Baseline
- **MSVC**: 1.26M nodes/second, 16.8s perft
- **GCC**: 23.1s perft (37% slower than MSVC)
- **Move generation**: Piece list iteration (`pList[color][type][index]`)

### Missing Components ❌
- **Attack bitboards**: Sliding piece attack generation
- **Move generation**: Bitboard-based move loops
- **Magic bitboards**: For rook/bishop attacks (optional but recommended)
- **SIMD optimizations**: Hardware acceleration opportunities

## Migration Strategy

### Core Principles
1. **Gradual Migration**: One subsystem at a time to maintain stability
2. **Performance Monitoring**: Benchmark each phase to prevent regressions
3. **Dual Implementation**: Run both systems in parallel during transition
4. **Rollback Capability**: Ability to revert changes if performance drops

### Phase Breakdown

## Phase 0: Foundation Preparation (1-2 weeks)

### Objectives
- Complete bitboard infrastructure
- Establish performance benchmarks
- Create migration testing framework

### Tasks

#### 1. Enhance Bitboard Operations
```cpp
// Add missing bitboard utilities
namespace Bitboard {
    // Population count (hardware accelerated)
    inline int popcount(uint64_t bb) { return __builtin_popcountll(bb); }
    
    // Find first set bit (LSB)
    inline int lsb(uint64_t bb) { return __builtin_ctzll(bb); }
    
    // Find last set bit (MSB) 
    inline int msb(uint64_t bb) { return 63 - __builtin_clzll(bb); }
    
    // Iterate through set bits
    inline int pop_lsb(uint64_t& bb) {
        int sq = lsb(bb);
        bb &= bb - 1;  // Clear LSB
        return sq;
    }
}
```

#### 2. Attack Bitboard Infrastructure
```cpp
// Pre-computed attack tables
extern uint64_t knight_attacks[64];
extern uint64_t king_attacks[64];
extern uint64_t pawn_attacks[2][64];  // [color][square]

// Sliding piece attacks (to be implemented)
uint64_t rook_attacks(int square, uint64_t occupied);
uint64_t bishop_attacks(int square, uint64_t occupied);
uint64_t queen_attacks(int square, uint64_t occupied);
```

#### 3. Performance Testing Framework
```cpp
// Benchmark harness for comparing approaches
class MigrationBenchmark {
    void compare_attack_detection();
    void compare_move_generation();
    void measure_cache_performance();
};
```

## Phase 1: Attack Detection Migration (2-3 weeks)

### Objectives
Replace piece list based attack detection with bitboard implementation

### Current Implementation
```cpp
// src/attack_detection.cpp uses mailbox iteration
bool Huginn::SqAttacked(int sq, Color side, const Position& pos) {
    // Iterates through piece lists to find attackers
    for (int i = 0; i < pos.pCount[color_idx][type_idx]; ++i) {
        int piece_sq = pos.pList[color_idx][type_idx][i];
        // Check if piece attacks target square
    }
}
```

### Target Implementation
```cpp
bool Huginn::SqAttackedBB(int sq, Color side, const Position& pos) {
    uint64_t enemy_pieces = pos.color_bitboards[size_t(side)];
    uint64_t target_bit = 1ULL << sq;
    
    // Check pawn attacks
    if (pawn_attacks[1-size_t(side)][sq] & pos.piece_bitboards[size_t(side)][size_t(PieceType::Pawn)])
        return true;
        
    // Check knight attacks  
    if (knight_attacks[sq] & pos.piece_bitboards[size_t(side)][size_t(PieceType::Knight)])
        return true;
        
    // Check sliding pieces with magic bitboards
    uint64_t rook_queens = pos.piece_bitboards[size_t(side)][size_t(PieceType::Rook)] |
                          pos.piece_bitboards[size_t(side)][size_t(PieceType::Queen)];
    if (rook_attacks(sq, pos.occupied_bitboard) & rook_queens)
        return true;
        
    // Similar for bishops and king
    return false;
}
```

### Performance Target
- **Goal**: 10-20% improvement in attack detection speed
- **Fallback**: If performance degrades, implement hybrid approach

## Phase 2: Pawn Move Generation (2-3 weeks)

### Objectives
Convert pawn move generation from piece list iteration to bitboard operations

### Current Implementation
```cpp
// Iterates through pawn piece list
for (int i = 0; i < pos.pCount[color_idx][PAWN]; ++i) {
    int from = pos.pList[color_idx][PAWN][i];
    // Generate moves for individual pawn
}
```

### Target Implementation
```cpp
void generate_pawn_moves_bb(const Position& pos, S_MOVELIST& list) {
    Color us = pos.side;
    uint64_t our_pawns = pos.piece_bitboards[size_t(us)][size_t(PieceType::Pawn)];
    uint64_t empty = ~pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[1 - size_t(us)];
    
    // Single pawn pushes
    uint64_t single_pushes = (us == Color::White) ? 
        (our_pawns << 8) & empty : 
        (our_pawns >> 8) & empty;
        
    // Double pawn pushes
    uint64_t double_pushes = (us == Color::White) ?
        (single_pushes << 8) & empty & RANK_4 :
        (single_pushes >> 8) & empty & RANK_5;
        
    // Captures
    uint64_t left_captures = (us == Color::White) ?
        (our_pawns << 7) & enemies & ~FILE_H :
        (our_pawns >> 9) & enemies & ~FILE_H;
        
    // Convert bitboards to move list
    while (single_pushes) {
        int to = pop_lsb(single_pushes);
        int from = (us == Color::White) ? to - 8 : to + 8;
        add_move(from, to, QUIET, list);
    }
    // Similar for other move types...
}
```

### Performance Target
- **Goal**: 15-25% improvement in pawn move generation
- **Pawn moves**: Currently 20.3% of generation time

## Phase 3: Sliding Piece Migration (3-4 weeks)

### Objectives
Implement magic bitboard attacks for rook, bishop, and queen

### Magic Bitboard Implementation
```cpp
// Magic bitboard structure
struct MagicEntry {
    uint64_t mask;      // Relevant occupancy bits
    uint64_t magic;     // Magic number
    uint64_t* attacks;  // Pointer to attack table
    int shift;          // Bits to shift
};

extern MagicEntry rook_magics[64];
extern MagicEntry bishop_magics[64];

inline uint64_t rook_attacks(int sq, uint64_t occupied) {
    MagicEntry& m = rook_magics[sq];
    occupied &= m.mask;
    occupied *= m.magic;
    occupied >>= m.shift;
    return m.attacks[occupied];
}
```

### Move Generation Conversion
```cpp
void generate_rook_moves_bb(const Position& pos, S_MOVELIST& list) {
    Color us = pos.side;
    uint64_t our_rooks = pos.piece_bitboards[size_t(us)][size_t(PieceType::Rook)];
    uint64_t our_pieces = pos.color_bitboards[size_t(us)];
    
    while (our_rooks) {
        int from = pop_lsb(our_rooks);
        uint64_t attacks = rook_attacks(from, pos.occupied_bitboard);
        attacks &= ~our_pieces;  // Can't capture own pieces
        
        while (attacks) {
            int to = pop_lsb(attacks);
            MoveType type = (pos.occupied_bitboard & (1ULL << to)) ? CAPTURE : QUIET;
            add_move(from, to, type, list);
        }
    }
}
```

### Performance Target
- **Goal**: 20-30% improvement in sliding piece generation
- **Sliding pieces**: Currently 45%+ of generation time

## Phase 4: Knight and King Migration (1-2 weeks)

### Objectives
Complete migration of remaining piece types

### Implementation
```cpp
void generate_knight_moves_bb(const Position& pos, S_MOVELIST& list) {
    Color us = pos.side;
    uint64_t our_knights = pos.piece_bitboards[size_t(us)][size_t(PieceType::Knight)];
    uint64_t our_pieces = pos.color_bitboards[size_t(us)];
    
    while (our_knights) {
        int from = pop_lsb(our_knights);
        uint64_t attacks = knight_attacks[from] & ~our_pieces;
        
        while (attacks) {
            int to = pop_lsb(attacks);
            MoveType type = (pos.occupied_bitboard & (1ULL << to)) ? CAPTURE : QUIET;
            add_move(from, to, type, list);
        }
    }
}
```

## Phase 5: Cleanup and Optimization (2-3 weeks)

### Objectives
- Remove piece list infrastructure
- Optimize bitboard operations with SIMD
- Final performance tuning

### Infrastructure Removal
```cpp
// Remove from Position class:
// std::array<PieceList, 2> pList;
// std::array<std::array<int, 2>, 7> pCount{};

// Keep only:
// std::array<std::array<Bitboard, int(PieceType::_Count)>, 2> piece_bitboards{};
// std::array<Bitboard, 2> color_bitboards{};
// Bitboard occupied_bitboard{};
```

### SIMD Optimizations
```cpp
// Parallel bitboard operations
void update_multiple_bitboards_simd(uint64_t* bitboards, uint64_t mask, int count) {
    // Use AVX2 for parallel updates
}
```

## Risk Mitigation

### Performance Monitoring
- **Benchmark after each phase**: Compare perft and search speed
- **Regression threshold**: >5% slowdown triggers rollback
- **A/B testing**: Run both implementations in parallel

### Rollback Strategy
```cpp
#define USE_BITBOARD_MOVES 1  // Feature flag for each component
#ifdef USE_BITBOARD_MOVES
    generate_pawn_moves_bb(pos, list);
#else  
    generate_pawn_moves_legacy(pos, list);
#endif
```

### Testing Strategy
- **Unit tests**: Individual bitboard operations
- **Integration tests**: Full move generation accuracy
- **Performance tests**: Speed comparison at each phase
- **Correctness verification**: Perft validation

## Expected Outcomes

### Performance Projections
- **Phase 1**: 10-20% attack detection improvement
- **Phase 2**: 15-25% pawn move improvement  
- **Phase 3**: 20-30% sliding piece improvement
- **Overall**: 25-40% total move generation speedup

### Target Performance (Conservative)
- **MSVC**: 1.6M nodes/second (27% improvement)
- **Perft**: 13-14 seconds (15-20% improvement)
- **Memory**: Reduced by ~30% (remove piece lists)

### Modern Chess Engine Standards
- **Stockfish**: ~5-10M nodes/second (reference point)
- **Competitive range**: 2-5M nodes/second for strong engines

## Implementation Timeline

- **Phase 0**: 2 weeks (Foundation)
- **Phase 1**: 3 weeks (Attack detection)  
- **Phase 2**: 3 weeks (Pawn moves)
- **Phase 3**: 4 weeks (Sliding pieces)
- **Phase 4**: 2 weeks (Knight/King)
- **Phase 5**: 3 weeks (Cleanup)

**Total**: ~17 weeks (4+ months)

## Success Metrics

### Technical Metrics
- ✅ All perft tests pass at each phase
- ✅ No regression in search correctness
- ✅ 25%+ improvement in move generation speed
- ✅ Code complexity reduction (fewer data structures)

### Strategic Benefits
- 🚀 **Modern architecture** aligned with top engines
- 🚀 **SIMD optimization** opportunities unlocked
- 🚀 **Reduced memory footprint** for better cache performance
- 🚀 **Future-proof foundation** for advanced techniques

This migration plan positions Huginn for significant performance gains while maintaining the stability and correctness that makes it a strong chess engine!