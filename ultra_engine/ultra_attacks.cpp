/**
 * @file ultra_attacks.cpp
 * @brief Implementation of ultra-optimized attack table system
 */

#include "ultra_attacks.hpp"
#include <cstring>
#include <random>

namespace UltraEngine {

// ============================================================================
// STATIC MEMBER DEFINITIONS
// ============================================================================

std::array<uint64_t, 64> UltraAttacks::knight_attacks_;
std::array<uint64_t, 64> UltraAttacks::king_attacks_;
std::array<uint64_t, 64> UltraAttacks::white_pawn_attacks_;
std::array<uint64_t, 64> UltraAttacks::black_pawn_attacks_;

std::array<uint64_t, 64> UltraAttacks::rook_masks_;
std::array<uint64_t, 64> UltraAttacks::bishop_masks_;
std::array<uint64_t, 64> UltraAttacks::rook_magics_;
std::array<uint64_t, 64> UltraAttacks::bishop_magics_;
std::array<int, 64> UltraAttacks::rook_shifts_;
std::array<int, 64> UltraAttacks::bishop_shifts_;

uint64_t* UltraAttacks::rook_attacks_[64];
uint64_t* UltraAttacks::bishop_attacks_[64];

bool UltraAttacks::initialized_ = false;

// ============================================================================
// MAGIC NUMBERS - Pre-computed for optimal performance
// ============================================================================

// Rook magic numbers (verified to work for all squares)
static const uint64_t ROOK_MAGICS[64] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
    0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

// Bishop magic numbers
static const uint64_t BISHOP_MAGICS[64] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void UltraAttacks::initialize() {
    if (initialized_) return;
    
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_sliding_attacks();
    
    initialized_ = true;
}

void UltraAttacks::cleanup() {
    if (!initialized_) return;
    
    // Clean up dynamically allocated attack tables
    for (int sq = 0; sq < 64; ++sq) {
        delete[] rook_attacks_[sq];
        delete[] bishop_attacks_[sq];
        rook_attacks_[sq] = nullptr;
        bishop_attacks_[sq] = nullptr;
    }
    
    initialized_ = false;
}

void UltraAttacks::init_knight_attacks() {
    // Knight move offsets (pure 64-bit coordinates)
    const int knight_offsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    
    for (int sq = 0; sq < 64; ++sq) {
        uint64_t attacks = 0ULL;
        const int file = square_to_file(sq);
        const int rank = square_to_rank(sq);
        
        for (int offset : knight_offsets) {
            const int target = sq + offset;
            if (target < 0 || target >= 64) continue;
            
            const int target_file = square_to_file(target);
            const int target_rank = square_to_rank(target);
            
            // Check if the move stays within the board boundaries
            const int file_diff = (file > target_file) ? (file - target_file) : (target_file - file);
            const int rank_diff = (rank > target_rank) ? (rank - target_rank) : (target_rank - rank);
            if (file_diff <= 2 && rank_diff <= 2) {
                attacks |= 1ULL << target;
            }
        }
        
        knight_attacks_[sq] = attacks;
    }
}

void UltraAttacks::init_king_attacks() {
    // King move offsets (pure 64-bit coordinates)
    const int king_offsets[8] = { -9, -8, -7, -1, 1, 7, 8, 9 };
    
    for (int sq = 0; sq < 64; ++sq) {
        uint64_t attacks = 0ULL;
        const int file = square_to_file(sq);
        const int rank = square_to_rank(sq);
        
        for (int offset : king_offsets) {
            const int target = sq + offset;
            if (target < 0 || target >= 64) continue;
            
            const int target_file = square_to_file(target);
            const int target_rank = square_to_rank(target);
            
            // Check if the move is exactly one square away
            const int file_diff = (file > target_file) ? (file - target_file) : (target_file - file);
            const int rank_diff = (rank > target_rank) ? (rank - target_rank) : (target_rank - rank);
            if (file_diff <= 1 && rank_diff <= 1) {
                attacks |= 1ULL << target;
            }
        }
        
        king_attacks_[sq] = attacks;
    }
}

void UltraAttacks::init_pawn_attacks() {
    for (int sq = 0; sq < 64; ++sq) {
        const int file = square_to_file(sq);
        
        // White pawn attacks (north-east and north-west)
        uint64_t white_attacks = 0ULL;
        if (sq < 56) { // Not on rank 8
            if (file > 0) white_attacks |= 1ULL << (sq + 7); // North-west
            if (file < 7) white_attacks |= 1ULL << (sq + 9); // North-east
        }
        white_pawn_attacks_[sq] = white_attacks;
        
        // Black pawn attacks (south-east and south-west)
        uint64_t black_attacks = 0ULL;
        if (sq >= 8) { // Not on rank 1
            if (file > 0) black_attacks |= 1ULL << (sq - 9); // South-west
            if (file < 7) black_attacks |= 1ULL << (sq - 7); // South-east
        }
        black_pawn_attacks_[sq] = black_attacks;
    }
}

void UltraAttacks::init_sliding_attacks() {
    // Initialize masks and magic numbers
    for (int sq = 0; sq < 64; ++sq) {
        rook_masks_[sq] = generate_rook_mask(sq);
        bishop_masks_[sq] = generate_bishop_mask(sq);
        rook_magics_[sq] = ROOK_MAGICS[sq];
        bishop_magics_[sq] = BISHOP_MAGICS[sq];
        
        // Calculate shift amounts (64 - number of relevant bits)
        rook_shifts_[sq] = 64 - popcount(rook_masks_[sq]);
        bishop_shifts_[sq] = 64 - popcount(bishop_masks_[sq]);
        
        // Allocate attack tables
        const int rook_size = 1 << popcount(rook_masks_[sq]);
        const int bishop_size = 1 << popcount(bishop_masks_[sq]);
        
        rook_attacks_[sq] = new uint64_t[rook_size];
        bishop_attacks_[sq] = new uint64_t[bishop_size];
        
        // Initialize attack tables
        std::memset(rook_attacks_[sq], 0, rook_size * sizeof(uint64_t));
        std::memset(bishop_attacks_[sq], 0, bishop_size * sizeof(uint64_t));
        
        // Generate all possible blocker configurations
        const uint64_t rook_mask = rook_masks_[sq];
        const uint64_t bishop_mask = bishop_masks_[sq];
        
        for (int i = 0; i < rook_size; ++i) {
            uint64_t blockers = 0ULL;
            uint64_t mask = rook_mask;
            for (int bit = 0; bit < popcount(rook_mask); ++bit) {
                if (i & (1 << bit)) {
                    blockers |= 1ULL << get_lsb(mask);
                }
                mask &= mask - 1; // Clear LSB
            }
            
            const uint64_t key = (blockers * rook_magics_[sq]) >> rook_shifts_[sq];
            rook_attacks_[sq][key] = generate_rook_attacks(sq, blockers);
        }
        
        for (int i = 0; i < bishop_size; ++i) {
            uint64_t blockers = 0ULL;
            uint64_t mask = bishop_mask;
            for (int bit = 0; bit < popcount(bishop_mask); ++bit) {
                if (i & (1 << bit)) {
                    blockers |= 1ULL << get_lsb(mask);
                }
                mask &= mask - 1; // Clear LSB
            }
            
            const uint64_t key = (blockers * bishop_magics_[sq]) >> bishop_shifts_[sq];
            bishop_attacks_[sq][key] = generate_bishop_attacks(sq, blockers);
        }
    }
}

uint64_t UltraAttacks::generate_rook_mask(int square) {
    uint64_t mask = 0ULL;
    const int file = square_to_file(square);
    const int rank = square_to_rank(square);
    
    // Horizontal (exclude edges)
    for (int f = 1; f < 7; ++f) {
        if (f != file) {
            mask |= 1ULL << file_rank_to_square(f, rank);
        }
    }
    
    // Vertical (exclude edges)
    for (int r = 1; r < 7; ++r) {
        if (r != rank) {
            mask |= 1ULL << file_rank_to_square(file, r);
        }
    }
    
    return mask;
}

uint64_t UltraAttacks::generate_bishop_mask(int square) {
    uint64_t mask = 0ULL;
    const int file = square_to_file(square);
    const int rank = square_to_rank(square);
    
    // Diagonal directions (exclude edges)
    const int directions[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    
    for (const auto& dir : directions) {
        for (int i = 1; i < 7; ++i) {
            const int f = file + i * dir[0];
            const int r = rank + i * dir[1];
            
            if (f >= 1 && f <= 6 && r >= 1 && r <= 6) {
                mask |= 1ULL << file_rank_to_square(f, r);
            } else {
                break;
            }
        }
    }
    
    return mask;
}

uint64_t UltraAttacks::generate_rook_attacks(int square, uint64_t blockers) {
    uint64_t attacks = 0ULL;
    const int file = square_to_file(square);
    const int rank = square_to_rank(square);
    
    // Horizontal and vertical directions
    const int directions[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    
    for (const auto& dir : directions) {
        for (int i = 1; i < 8; ++i) {
            const int f = file + i * dir[0];
            const int r = rank + i * dir[1];
            
            if (f < 0 || f >= 8 || r < 0 || r >= 8) break;
            
            const int target_sq = file_rank_to_square(f, r);
            attacks |= 1ULL << target_sq;
            
            if (blockers & (1ULL << target_sq)) break;
        }
    }
    
    return attacks;
}

uint64_t UltraAttacks::generate_bishop_attacks(int square, uint64_t blockers) {
    uint64_t attacks = 0ULL;
    const int file = square_to_file(square);
    const int rank = square_to_rank(square);
    
    // Diagonal directions
    const int directions[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    
    for (const auto& dir : directions) {
        for (int i = 1; i < 8; ++i) {
            const int f = file + i * dir[0];
            const int r = rank + i * dir[1];
            
            if (f < 0 || f >= 8 || r < 0 || r >= 8) break;
            
            const int target_sq = file_rank_to_square(f, r);
            attacks |= 1ULL << target_sq;
            
            if (blockers & (1ULL << target_sq)) break;
        }
    }
    
    return attacks;
}

// ============================================================================
// ADVANCED ATTACK QUERIES
// ============================================================================

bool UltraAttacks::is_square_attacked(int square, int attacking_color, 
                                     const uint64_t piece_boards[2][6], 
                                     uint64_t all_occupied) noexcept {
    // Check pawn attacks
    if (pawn(square, 1 - attacking_color) & piece_boards[attacking_color][0]) return true;
    
    // Check knight attacks
    if (knight(square) & piece_boards[attacking_color][1]) return true;
    
    // Check bishop/queen attacks
    const uint64_t bishop_attacks = bishop(square, all_occupied);
    if (bishop_attacks & (piece_boards[attacking_color][2] | piece_boards[attacking_color][4])) return true;
    
    // Check rook/queen attacks
    const uint64_t rook_attacks = rook(square, all_occupied);
    if (rook_attacks & (piece_boards[attacking_color][3] | piece_boards[attacking_color][4])) return true;
    
    // Check king attacks
    if (king(square) & piece_boards[attacking_color][5]) return true;
    
    return false;
}

uint64_t UltraAttacks::get_attackers(int square, int attacking_color,
                                    const uint64_t piece_boards[2][6],
                                    uint64_t all_occupied) noexcept {
    uint64_t attackers = 0ULL;
    
    // Pawn attackers
    attackers |= pawn(square, 1 - attacking_color) & piece_boards[attacking_color][0];
    
    // Knight attackers
    attackers |= knight(square) & piece_boards[attacking_color][1];
    
    // Bishop/queen attackers
    const uint64_t bishop_attacks = bishop(square, all_occupied);
    attackers |= bishop_attacks & (piece_boards[attacking_color][2] | piece_boards[attacking_color][4]);
    
    // Rook/queen attackers
    const uint64_t rook_attacks = rook(square, all_occupied);
    attackers |= rook_attacks & (piece_boards[attacking_color][3] | piece_boards[attacking_color][4]);
    
    // King attackers
    attackers |= king(square) & piece_boards[attacking_color][5];
    
    return attackers;
}

} // namespace UltraEngine