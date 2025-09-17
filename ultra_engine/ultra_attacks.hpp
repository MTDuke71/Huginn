/**
 * @file ultra_attacks.hpp
 * @brief Ultra-optimized attack table system for pure 64-bit coordinates
 * 
 * Provides blazing-fast attack computation for all piece types using
 * pre-computed lookup tables and magic bitboards. Zero coordinate conversions.
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#pragma once
#include <cstdint>
#include <array>
#include <algorithm>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace UltraEngine {

// ============================================================================
// BITBOARD UTILITIES
// Ultra-fast bit manipulation for 64-bit coordinates
// ============================================================================

// Get least significant bit (square index 0-63)
[[nodiscard]] inline int get_lsb(uint64_t bb) noexcept {
#ifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, bb);
    return static_cast<int>(index);
#else
    return __builtin_ctzll(bb);
#endif
}

// Pop least significant bit and return its index
[[nodiscard]] inline int pop_lsb(uint64_t& bb) noexcept {
    const int sq = get_lsb(bb);
    bb &= bb - 1; // Clear LSB
    return sq;
}

// Count number of set bits
[[nodiscard]] inline int popcount(uint64_t bb) noexcept {
#ifdef _MSC_VER
    return static_cast<int>(__popcnt64(bb));
#else
    return __builtin_popcountll(bb);
#endif
}

// ============================================================================
// ULTRA-FAST ATTACK TABLES
// All tables use pure 64-bit coordinates (0-63)
// ============================================================================

class UltraAttacks {
private:
    // Pre-computed attack tables
    static std::array<uint64_t, 64> knight_attacks_;
    static std::array<uint64_t, 64> king_attacks_;
    static std::array<uint64_t, 64> white_pawn_attacks_;
    static std::array<uint64_t, 64> black_pawn_attacks_;
    
    // Magic bitboard tables for sliding pieces
    static std::array<uint64_t, 64> rook_masks_;
    static std::array<uint64_t, 64> bishop_masks_;
    static std::array<uint64_t, 64> rook_magics_;
    static std::array<uint64_t, 64> bishop_magics_;
    static std::array<int, 64> rook_shifts_;
    static std::array<int, 64> bishop_shifts_;
    
    // Attack lookup tables (allocated dynamically for optimal memory usage)
    static uint64_t* rook_attacks_[64];
    static uint64_t* bishop_attacks_[64];
    
    static bool initialized_;

public:
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    static void initialize();
    static void cleanup();
    [[nodiscard]] static bool is_initialized() noexcept { return initialized_; }

    // ========================================================================
    // ULTRA-FAST ATTACK LOOKUPS
    // All functions are inline for maximum performance
    // ========================================================================
    
    // Knight attacks - single lookup
    [[nodiscard]] static inline uint64_t knight(int square) noexcept {
        return knight_attacks_[square];
    }
    
    // King attacks - single lookup
    [[nodiscard]] static inline uint64_t king(int square) noexcept {
        return king_attacks_[square];
    }
    
    // Pawn attacks - single lookup per color
    [[nodiscard]] static inline uint64_t pawn(int square, int color) noexcept {
        return (color == 0) ? white_pawn_attacks_[square] : black_pawn_attacks_[square];
    }
    
    // Rook attacks - magic bitboard lookup
    [[nodiscard]] static inline uint64_t rook(int square, uint64_t occupied) noexcept {
        const uint64_t blockers = occupied & rook_masks_[square];
        const uint64_t key = (blockers * rook_magics_[square]) >> rook_shifts_[square];
        return rook_attacks_[square][key];
    }
    
    // Bishop attacks - magic bitboard lookup
    [[nodiscard]] static inline uint64_t bishop(int square, uint64_t occupied) noexcept {
        const uint64_t blockers = occupied & bishop_masks_[square];
        const uint64_t key = (blockers * bishop_magics_[square]) >> bishop_shifts_[square];
        return bishop_attacks_[square][key];
    }
    
    // Queen attacks - combination of rook and bishop
    [[nodiscard]] static inline uint64_t queen(int square, uint64_t occupied) noexcept {
        return rook(square, occupied) | bishop(square, occupied);
    }

    // ========================================================================
    // ADVANCED ATTACK QUERIES
    // ========================================================================
    
    // Check if square is attacked by any piece of given color
    [[nodiscard]] static bool is_square_attacked(int square, int attacking_color, 
                                                 const uint64_t piece_boards[2][6], 
                                                 uint64_t all_occupied) noexcept;
    
    // Get all squares attacking a given square
    [[nodiscard]] static uint64_t get_attackers(int square, int attacking_color,
                                                const uint64_t piece_boards[2][6],
                                                uint64_t all_occupied) noexcept;

private:
    // ========================================================================
    // INITIALIZATION HELPERS
    // ========================================================================
    
    static void init_knight_attacks();
    static void init_king_attacks();
    static void init_pawn_attacks();
    static void init_sliding_attacks();
    
    // Magic bitboard generation
    static uint64_t generate_rook_mask(int square);
    static uint64_t generate_bishop_mask(int square);
    static uint64_t generate_rook_attacks(int square, uint64_t blockers);
    static uint64_t generate_bishop_attacks(int square, uint64_t blockers);
    static uint64_t find_magic(int square, int bits, bool is_rook);
};

// ============================================================================
// BITBOARD CONSTANTS AND UTILITIES
// ============================================================================

// File and rank masks for pure 64-bit coordinates
constexpr uint64_t FILE_A = 0x0101010101010101ULL;
constexpr uint64_t FILE_B = 0x0202020202020202ULL;
constexpr uint64_t FILE_C = 0x0404040404040404ULL;
constexpr uint64_t FILE_D = 0x0808080808080808ULL;
constexpr uint64_t FILE_E = 0x1010101010101010ULL;
constexpr uint64_t FILE_F = 0x2020202020202020ULL;
constexpr uint64_t FILE_G = 0x4040404040404040ULL;
constexpr uint64_t FILE_H = 0x8080808080808080ULL;

constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

// Get file/rank masks for any square
[[nodiscard]] constexpr uint64_t file_mask(int square) noexcept {
    return FILE_A << (square & 7);
}

[[nodiscard]] constexpr uint64_t rank_mask(int square) noexcept {
    return RANK_1 << ((square >> 3) * 8);
}

// Direction offsets for sliding pieces (pure 64-bit)
constexpr int NORTH = 8;
constexpr int SOUTH = -8;
constexpr int EAST = 1;
constexpr int WEST = -1;
constexpr int NORTH_EAST = 9;
constexpr int NORTH_WEST = 7;
constexpr int SOUTH_EAST = -7;
constexpr int SOUTH_WEST = -9;

// ============================================================================
// INLINE UTILITY FUNCTIONS
// ============================================================================

// Convert square to file/rank (0-7)
[[nodiscard]] constexpr int square_to_file(int square) noexcept { return square & 7; }
[[nodiscard]] constexpr int square_to_rank(int square) noexcept { return square >> 3; }
[[nodiscard]] constexpr int file_rank_to_square(int file, int rank) noexcept { return rank * 8 + file; }

// Check if square is on edge (for boundary checking)
[[nodiscard]] constexpr bool is_on_file_a(int square) noexcept { return (square & 7) == 0; }
[[nodiscard]] constexpr bool is_on_file_h(int square) noexcept { return (square & 7) == 7; }
[[nodiscard]] constexpr bool is_on_rank_1(int square) noexcept { return square < 8; }
[[nodiscard]] constexpr bool is_on_rank_8(int square) noexcept { return square >= 56; }

// Distance calculations
[[nodiscard]] constexpr int file_distance(int sq1, int sq2) noexcept {
    const int diff = square_to_file(sq1) - square_to_file(sq2);
    return (diff < 0) ? -diff : diff;
}

[[nodiscard]] constexpr int rank_distance(int sq1, int sq2) noexcept {
    const int diff = square_to_rank(sq1) - square_to_rank(sq2);
    return (diff < 0) ? -diff : diff;
}

[[nodiscard]] constexpr int chebyshev_distance(int sq1, int sq2) noexcept {
    const int file_dist = file_distance(sq1, sq2);
    const int rank_dist = rank_distance(sq1, sq2);
    return (file_dist > rank_dist) ? file_dist : rank_dist;
}

} // namespace UltraEngine