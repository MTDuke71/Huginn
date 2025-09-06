#include "evaluation.hpp"
#include "attack_detection.hpp"
#include "movegen_enhanced.hpp"
#include <algorithm>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

namespace Huginn {

namespace EvalParams {

// VICE Part 78: Runtime-initialized evaluation masks (0:58, 1:30, 2:32)
std::array<uint64_t, 64> WHITE_PASSED_PAWN_MASKS;
std::array<uint64_t, 64> BLACK_PASSED_PAWN_MASKS;

// VICE Part 78: Initialize pawn bitmasks (2:19)
void init_evaluation_masks() {
    // Initialize all masks to 0
    WHITE_PASSED_PAWN_MASKS.fill(0);
    BLACK_PASSED_PAWN_MASKS.fill(0);
    
    // Iterate through each square on the board (4:48)
    for (int sq = 0; sq < 64; ++sq) {
        int file = sq % 8;
        int rank = sq / 8;
        
        // White passed pawn masks (0:58)
        // For each white pawn, mark squares that must be clear for it to be passed
        uint64_t white_mask = 0ULL;
        
        // Check all ranks in front of this pawn
        for (int r = rank + 1; r <= 7; ++r) {
            // Same file - pawn must advance
            white_mask |= (1ULL << (r * 8 + file));
            
            // Adjacent files - enemy pawns that could capture
            if (file > 0) {
                white_mask |= (1ULL << (r * 8 + (file - 1)));
            }
            if (file < 7) {
                white_mask |= (1ULL << (r * 8 + (file + 1)));
            }
        }
        WHITE_PASSED_PAWN_MASKS[sq] = white_mask;
        
        // Black passed pawn masks (2:32)
        // For each black pawn, mark squares that must be clear for it to be passed
        uint64_t black_mask = 0ULL;
        
        // Check all ranks behind this pawn (from black's perspective)
        for (int r = rank - 1; r >= 0; --r) {
            // Same file - pawn must advance
            black_mask |= (1ULL << (r * 8 + file));
            
            // Adjacent files - enemy pawns that could capture  
            if (file > 0) {
                black_mask |= (1ULL << (r * 8 + (file - 1)));
            }
            if (file < 7) {
                black_mask |= (1ULL << (r * 8 + (file + 1)));
            }
        }
        BLACK_PASSED_PAWN_MASKS[sq] = black_mask;
    }
}

} // namespace EvalParams

namespace PawnStructure {

// Check if a pawn is isolated (no friendly pawns on adjacent files)
bool is_isolated_pawn(const Position& pos, int sq64) {
    int file = sq64 % 8;
    
    // Get pawn bitboards for the current player
    uint64_t pawns = pos.piece_bitboards[WHITE_PAWN] | pos.piece_bitboards[BLACK_PAWN];
    
    // Check adjacent files for any pawns
    uint64_t adjacent_files = 0ULL;
    if (file > 0) {
        adjacent_files |= EvalParams::FILE_MASKS[file - 1];
    }
    if (file < 7) {
        adjacent_files |= EvalParams::FILE_MASKS[file + 1];
    }
    
    return (pawns & adjacent_files) == 0;
}

// Check if a pawn is passed using the runtime-initialized masks
bool is_passed_pawn(const Position& pos, int sq64, bool is_white) {
    uint64_t blocking_mask;
    uint64_t enemy_pawns;
    
    if (is_white) {
        blocking_mask = EvalParams::WHITE_PASSED_PAWN_MASKS[sq64];
        enemy_pawns = pos.piece_bitboards[BLACK_PAWN];
    } else {
        blocking_mask = EvalParams::BLACK_PASSED_PAWN_MASKS[sq64];
        enemy_pawns = pos.piece_bitboards[WHITE_PAWN];
    }
    
    // Pawn is passed if no enemy pawns block its path
    return (enemy_pawns & blocking_mask) == 0;
}

// Evaluate overall pawn structure
int evaluate_pawn_structure(const Position& pos) {
    int score = 0;
    
    // Evaluate white pawns
    uint64_t white_pawns = pos.piece_bitboards[WHITE_PAWN];
    while (white_pawns) {
        int sq64 = __builtin_ctzll(white_pawns);
        white_pawns &= white_pawns - 1; // Clear the lowest set bit
        
        if (is_passed_pawn(pos, sq64, true)) {
            score += 50; // Passed pawn bonus
        }
        
        if (is_isolated_pawn(pos, sq64)) {
            score -= EvalParams::ISOLATED_PAWN_PENALTY;
        }
    }
    
    // Evaluate black pawns
    uint64_t black_pawns = pos.piece_bitboards[BLACK_PAWN];
    while (black_pawns) {
        int sq64 = __builtin_ctzll(black_pawns);
        black_pawns &= black_pawns - 1; // Clear the lowest set bit
        
        if (is_passed_pawn(pos, sq64, false)) {
            score -= 50; // Passed pawn penalty (for opponent)
        }
        
        if (is_isolated_pawn(pos, sq64)) {
            score += EvalParams::ISOLATED_PAWN_PENALTY; // Opponent isolated pawn is good for us
        }
    }
    
    return score;
}

} // namespace PawnStructure

} // namespace Huginn
