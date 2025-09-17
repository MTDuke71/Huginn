/**
 * @file ultra_types.hpp
 * @brief Ultra-optimized chess engine types - pure 64-bit coordinates
 * 
 * Revolutionary chess engine design eliminating all coordinate conversion overhead.
 * Uses only 64-bit square indices (0-63) throughout the entire engine.
 * Target: 50%+ performance improvement over current engine.
 * 
 * @author MTDuke71 
 * @version Ultra 1.0
 */
#pragma once
#include <cstdint>
#include <array>

namespace UltraEngine {

// ============================================================================
// ULTRA-FAST MOVE REPRESENTATION
// Pure 64-bit coordinates - NO coordinate conversions anywhere!
// ============================================================================

/**
 * @brief Ultra-compact move encoding using ONLY 64-bit coordinates (0-63)
 * 
 * 22-bit encoding (vs current 25-bit with 120-square overhead):
 * Bits 0-5:   From square (6 bits: 0-63) 
 * Bits 6-11:  To square (6 bits: 0-63)
 * Bits 12-14: Captured piece (3 bits: 0-7, None/P/N/B/R/Q/K/Reserved)
 * Bits 15-17: Promotion piece (3 bits: 0-7)
 * Bit 18:     En passant flag
 * Bit 19:     Double pawn push flag  
 * Bit 20:     Castling flag
 * Bit 21:     Reserved for future use
 */
struct UltraMove {
    uint32_t data = 0;  // Only 22 bits used - ultra-compact!
    
    // Ultra-fast encoding - all inline for maximum performance
    constexpr UltraMove() = default;
    
    constexpr UltraMove(int from64, int to64, int captured = 0, int promoted = 0, 
                       bool ep = false, bool pawn_start = false, bool castle = false)
        : data((from64 & 0x3F) | 
               ((to64 & 0x3F) << 6) |
               ((captured & 0x7) << 12) |
               ((promoted & 0x7) << 15) |
               (ep ? (1 << 18) : 0) |
               (pawn_start ? (1 << 19) : 0) |
               (castle ? (1 << 20) : 0)) {}
    
    // Ultra-fast decoding - all constexpr for compile-time optimization
    [[nodiscard]] constexpr int from() const { return data & 0x3F; }
    [[nodiscard]] constexpr int to() const { return (data >> 6) & 0x3F; }
    [[nodiscard]] constexpr int captured() const { return (data >> 12) & 0x7; }
    [[nodiscard]] constexpr int promoted() const { return (data >> 15) & 0x7; }
    [[nodiscard]] constexpr bool is_ep() const { return data & (1 << 18); }
    [[nodiscard]] constexpr bool is_pawn_start() const { return data & (1 << 19); }
    [[nodiscard]] constexpr bool is_castle() const { return data & (1 << 20); }
    [[nodiscard]] constexpr bool is_capture() const { return captured() != 0; }
    [[nodiscard]] constexpr bool is_promotion() const { return promoted() != 0; }
    [[nodiscard]] constexpr bool is_quiet() const { return !is_capture() && !is_promotion(); }
    
    constexpr bool operator==(const UltraMove& other) const { return data == other.data; }
    constexpr bool operator!=(const UltraMove& other) const { return data != other.data; }
};

// ============================================================================
// ULTRA-FAST MOVE LIST
// Cache-friendly, stack-allocated move list for maximum performance
// ============================================================================

struct UltraMoveList {
    std::array<UltraMove, 256> moves;  // Stack allocated - no heap overhead
    int count = 0;
    
    // Ultra-fast move adding - all inline
    constexpr void add(const UltraMove& move) {
        moves[count++] = move;
    }
    
    constexpr void add(int from, int to, int captured = 0, int promoted = 0,
                      bool ep = false, bool pawn_start = false, bool castle = false) {
        moves[count++] = UltraMove(from, to, captured, promoted, ep, pawn_start, castle);
    }
    
    constexpr void clear() { count = 0; }
    constexpr UltraMove& operator[](int i) { return moves[i]; }
    constexpr const UltraMove& operator[](int i) const { return moves[i]; }
    constexpr UltraMove* begin() { return moves.data(); }
    constexpr UltraMove* end() { return moves.data() + count; }
};

// ============================================================================
// ULTRA-OPTIMIZED POSITION REPRESENTATION
// Pure bitboard storage with minimal memory footprint
// ============================================================================

struct UltraPosition {
    // Core bitboards - only what we absolutely need
    uint64_t pieces[2][6];        // [color][piece_type] - 96 bytes
    uint64_t occupied[2];         // [color] occupied squares - 16 bytes  
    uint64_t all_occupied;        // All pieces - 8 bytes
    
    // Minimal game state - pack into single 64-bit value for cache efficiency
    union GameState {
        uint64_t packed;
        struct {
            uint8_t to_move : 1;           // Side to move (1 bit)
            uint8_t castle_rights : 4;     // Castling rights (4 bits) 
            uint8_t ep_square : 6;         // En passant square 0-63 (6 bits)
            uint8_t halfmove : 7;          // Halfmove clock (7 bits = 0-127)
            uint16_t fullmove;             // Fullmove number (16 bits)
            uint32_t hash_key;             // Zobrist hash (32 bits)
        };
    } state;
    
    // Constructor - initialize to starting position
    UltraPosition();
    
    // Core operations - all optimized for pure 64-bit coordinates
    void clear();
    bool make_move(const UltraMove& move);
    void unmake_move(const UltraMove& move, const GameState& prev_state);
    
    // Ultra-fast piece queries using pure bitboards
    [[nodiscard]] constexpr int piece_at(int sq64) const {
        const uint64_t mask = 1ULL << sq64;
        for (int color = 0; color < 2; ++color) {
            if (!(occupied[color] & mask)) continue;
            for (int piece = 0; piece < 6; ++piece) {
                if (pieces[color][piece] & mask) return piece;
            }
        }
        return -1; // Empty square
    }
    
    [[nodiscard]] constexpr bool is_occupied(int sq64) const {
        return all_occupied & (1ULL << sq64);
    }
    
    [[nodiscard]] constexpr bool is_white_piece(int sq64) const {
        return occupied[0] & (1ULL << sq64);
    }
    
    [[nodiscard]] constexpr bool is_black_piece(int sq64) const {
        return occupied[1] & (1ULL << sq64);
    }
};

// ============================================================================
// PIECE TYPE CONSTANTS - Optimized for ultra-fast lookups
// ============================================================================

constexpr int PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5;
constexpr int WHITE = 0, BLACK = 1;

// ============================================================================
// ULTRA-FAST ATTACK TABLES
// Pre-computed attack tables for all pieces using 64-bit coordinates
// ============================================================================

extern std::array<uint64_t, 64> ULTRA_KNIGHT_ATTACKS;
extern std::array<uint64_t, 64> ULTRA_KING_ATTACKS;
extern std::array<uint64_t, 64> ULTRA_PAWN_ATTACKS[2];  // [color]

// Magic bitboard tables for sliding pieces
extern std::array<uint64_t, 64> ULTRA_ROOK_MASKS;
extern std::array<uint64_t, 64> ULTRA_BISHOP_MASKS;
extern std::array<uint64_t*, 64> ULTRA_ROOK_ATTACKS;
extern std::array<uint64_t*, 64> ULTRA_BISHOP_ATTACKS;
extern std::array<uint64_t, 64> ULTRA_ROOK_MAGICS;
extern std::array<uint64_t, 64> ULTRA_BISHOP_MAGICS;

// Ultra-fast attack lookups - all inline for maximum performance
[[nodiscard]] constexpr uint64_t ultra_knight_attacks(int sq) {
    return ULTRA_KNIGHT_ATTACKS[sq];
}

[[nodiscard]] constexpr uint64_t ultra_king_attacks(int sq) {
    return ULTRA_KING_ATTACKS[sq];
}

[[nodiscard]] constexpr uint64_t ultra_pawn_attacks(int sq, int color) {
    return ULTRA_PAWN_ATTACKS[color][sq];
}

[[nodiscard]] inline uint64_t ultra_rook_attacks(int sq, uint64_t occupied) {
    const uint64_t blockers = occupied & ULTRA_ROOK_MASKS[sq];
    const uint64_t key = (blockers * ULTRA_ROOK_MAGICS[sq]) >> (64 - 12); // 12-bit index
    return ULTRA_ROOK_ATTACKS[sq][key];
}

[[nodiscard]] inline uint64_t ultra_bishop_attacks(int sq, uint64_t occupied) {
    const uint64_t blockers = occupied & ULTRA_BISHOP_MASKS[sq];
    const uint64_t key = (blockers * ULTRA_BISHOP_MAGICS[sq]) >> (64 - 9); // 9-bit index
    return ULTRA_BISHOP_ATTACKS[sq][key];
}

[[nodiscard]] inline uint64_t ultra_queen_attacks(int sq, uint64_t occupied) {
    return ultra_rook_attacks(sq, occupied) | ultra_bishop_attacks(sq, occupied);
}

} // namespace UltraEngine