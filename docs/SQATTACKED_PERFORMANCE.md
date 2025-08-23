# SqAttacked Function Performance Analysis

## Current Implementation Analysis

### Performance Characteristics

The current `SqAttacked` function has these performance characteristics:

1. **Worst Case**: ~56 board accesses (8 knight checks + 8 king checks + up to 7×4=28 sliding attacks + 4 pawn checks + 8 boundary checks)
2. **Average Case**: ~15-25 board accesses (early termination on first attack found)
3. **Best Case**: ~4-8 board accesses (pawn or knight attack found immediately)

### Current Bottlenecks

1. **Linear Scanning**: Sliding pieces scan up to 7 squares in 8 directions
2. **Redundant Board Access**: `pos.at()` function call overhead for each square
3. **No Early Termination Order**: Checks all piece types in fixed order
4. **No Piece List Utilization**: Doesn't leverage existing piece lists for targeted checking

## Optimization Strategies

### 1. **Piece List Optimization** (Highest Impact - 3-10x speedup)

Instead of scanning empty squares, use piece lists to check only actual pieces:

```cpp
// Current: Check if knights attack square (scans 8 empty squares)
for (int delta : KNIGHT_DELTAS) {
    int knight_sq = sq + delta;
    if (is_playable(knight_sq)) {
        Piece p = pos.at(knight_sq);
        if (type_of(p) == PieceType::Knight && color_of(p) == attacking_color) {
            return true;
        }
    }
}

// Optimized: Only check actual knights
int knight_count = pos.pCount[int(attacking_color)][int(PieceType::Knight)];
for (int i = 0; i < knight_count; ++i) {
    int knight_square = pos.pList[int(attacking_color)][int(PieceType::Knight)][i];
    // Check if this knight attacks the target square using precomputed lookup
    if (knight_attacks[knight_square] & (1ULL << target_square_64)) {
        return true;
    }
}
```

### 2. **Precomputed Attack Tables** (High Impact - 2-5x speedup)

Create lookup tables for non-sliding pieces:

```cpp
// Global precomputed tables (one-time initialization)
extern uint64_t knight_attacks[64];
extern uint64_t king_attacks[64];
extern uint64_t pawn_attacks[2][64];  // [color][square]

// Usage in SqAttacked
bool knight_can_attack = knight_attacks[knight_sq64] & (1ULL << target_sq64);
```

### 3. **Optimized Piece Type Ordering** (Medium Impact - 1.5-2x speedup)

Check most common attacking pieces first:

```cpp
// Order by frequency: Pawns > Knights > Kings > Queens > Rooks > Bishops
// Pawns attack diagonally (most common in middle game)
// Knights are active in opening/middle game
// Kings are always present but limited range
```

### 4. **Sliding Piece Ray Attacks** (High Impact - 2-4x speedup)

Use ray attacks with occupancy masking:

```cpp
// Precomputed ray attacks from each square in each direction
extern uint64_t ray_attacks[64][8];  // [square][direction]

bool rook_attacks_square(int rook_sq, int target_sq, uint64_t occupancy) {
    int direction = get_direction(rook_sq, target_sq);
    if (direction == -1) return false;  // Not on same rank/file
    
    uint64_t ray = ray_attacks[rook_sq][direction];
    uint64_t blockers = ray & occupancy;
    
    // Find first blocker
    int first_blocker = (direction < 4) ? 
        lsb(blockers) : msb(blockers);
    
    return target_sq <= first_blocker;  // Target is before first blocker
}
```

### 5. **Cache-Friendly Memory Access** (Low-Medium Impact - 1.2-1.5x speedup)

Minimize memory indirection:

```cpp
// Store pieces directly in optimized arrays
struct FastPosition {
    uint64_t piece_bb[12];  // [piece_type_color] bitboards
    int piece_squares[12][16];  // Direct square arrays
    int piece_counts[12];
};
```

## Recommended Implementation

Here's a hybrid optimized version balancing complexity and performance:

```cpp
inline bool SqAttacked_Optimized(int sq, const Position& pos, Color attacking_color) {
    if (!is_playable(sq)) return false;
    
    int target_sq64 = MAILBOX_MAPS.to64[sq];
    if (target_sq64 < 0) return false;
    
    int color_idx = int(attacking_color);
    
    // 1. Check pawns first (most common attacks)
    int pawn_count = pos.pCount[color_idx][int(PieceType::Pawn)];
    for (int i = 0; i < pawn_count; ++i) {
        int pawn_sq = pos.pList[color_idx][int(PieceType::Pawn)][i];
        if (pawn_attacks_square(pawn_sq, sq, attacking_color)) {
            return true;
        }
    }
    
    // 2. Check knights (using piece list + precomputed attacks)
    int knight_count = pos.pCount[color_idx][int(PieceType::Knight)];
    for (int i = 0; i < knight_count; ++i) {
        int knight_sq = pos.pList[color_idx][int(PieceType::Knight)][i];
        int knight_sq64 = MAILBOX_MAPS.to64[knight_sq];
        if (knight_sq64 >= 0 && (KNIGHT_ATTACKS[knight_sq64] & (1ULL << target_sq64))) {
            return true;
        }
    }
    
    // 3. Check king (always exactly 1)
    int king_sq = pos.king_sq[color_idx];
    if (king_sq >= 0) {
        int king_sq64 = MAILBOX_MAPS.to64[king_sq];
        if (king_sq64 >= 0 && (KING_ATTACKS[king_sq64] & (1ULL << target_sq64))) {
            return true;
        }
    }
    
    // 4. Check sliding pieces (rooks, bishops, queens)
    // Use occupancy bitboards for efficient ray attacks
    uint64_t occupancy = pos.get_occupancy_bitboard();
    
    // Check rooks and queens for rank/file attacks
    for (PieceType pt : {PieceType::Rook, PieceType::Queen}) {
        int piece_count = pos.pCount[color_idx][int(pt)];
        for (int i = 0; i < piece_count; ++i) {
            int piece_sq = pos.pList[color_idx][int(pt)][i];
            if (rook_attacks_square(piece_sq, sq, occupancy)) {
                return true;
            }
        }
    }
    
    // Check bishops and queens for diagonal attacks
    for (PieceType pt : {PieceType::Bishop, PieceType::Queen}) {
        int piece_count = pos.pCount[color_idx][int(pt)];
        for (int i = 0; i < piece_count; ++i) {
            int piece_sq = pos.pList[color_idx][int(pt)][i];
            if (bishop_attacks_square(piece_sq, sq, occupancy)) {
                return true;
            }
        }
    }
    
    return false;
}
```

## Expected Performance Improvements

1. **Piece List Usage**: 3-10x speedup (especially in endgames with few pieces)
2. **Precomputed Knight/King Tables**: 2-3x speedup for those piece types
3. **Optimized Piece Order**: 1.5-2x speedup on average
4. **Ray Attack Tables**: 2-4x speedup for sliding pieces

**Overall Expected Speedup**: 5-20x faster than current implementation

## Implementation Priority

1. **Phase 1**: Add piece list usage (biggest win, minimal complexity)
2. **Phase 2**: Add precomputed knight/king attack tables
3. **Phase 3**: Optimize sliding piece attacks with ray tables
4. **Phase 4**: Fine-tune piece checking order

The piece list optimization alone would provide the biggest performance gain with minimal code complexity increase.
