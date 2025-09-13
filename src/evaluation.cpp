/**
 * @file evaluation.cpp
 * @brief Chess position evaluation implementation
 * 
 * Implements position evaluation functions for the Huginn chess engine, including
 * material counting, piece-square tables, and advanced positional features.
 * The evaluation is based on the VICE tutorial approach with runtime-initialized
 * evaluation masks for passed pawn detection and other positional patterns.
 * 
 * ## Evaluation Components
 * - **Material Balance**: Basic piece values (P=100, N/B=300, R=500, Q=900)
 * - **Piece-Square Tables**: Positional bonuses for piece placement
 * - **Passed Pawns**: Detection using precomputed bitmasks
 * - **King Safety**: Basic king exposure evaluation
 * - **Mobility**: Piece mobility considerations
 * 
 * ## Performance Features
 * - Pre-computed evaluation masks for faster passed pawn detection
 * - Runtime initialization of position-dependent lookup tables
 * - Optimized evaluation ordering for alpha-beta cutoffs
 * 
 * @author MTDuke71
 * @version 1.2
 * @see position.hpp for Position structure
 */
#include "evaluation.hpp"

namespace Huginn {

namespace EvalParams {

// VICE Part 78: Runtime-initialized evaluation masks (0:58, 1:30, 2:32)
std::array<uint64_t, 64> WHITE_PASSED_PAWN_MASKS;
std::array<uint64_t, 64> BLACK_PASSED_PAWN_MASKS;

/**
 * @brief Initialize evaluation bitmasks for positional analysis
 * 
 * Sets up precomputed bitmasks for passed pawn detection and other positional
 * patterns. This function must be called once during engine initialization before
 * using the evaluation function. Based on VICE Part 78 tutorial approach.
 * 
 * Creates masks for:
 * - White passed pawn detection (squares in front and adjacent files)
 * - Black passed pawn detection (squares behind and adjacent files)
 * - Fast bitboard-based passed pawn evaluation
 */
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

} // namespace Huginn
