/**
 * @file evaluation.hpp
 * @brief Chess position evaluation system with game phase awareness and tactical pattern recognition
 * 
 * Implements a sophisticated evaluation function that assesses chess positions through multiple
 * game phases (opening, middlegame, endgame) using material balance, piece-square tables,
 * tactical bonuses, and positional factors. The evaluation provides the foundation for the
 * engine's search algorithm to distinguish between chess positions.
 * 
 * ## Architecture Overview
 * 
 * **Game Phase Detection:**
 * - Opening: High piece count (28+ points), development-focused evaluation
 * - Middlegame: Medium piece count (16-27 points), tactical pattern emphasis  
 * - Endgame: Low piece count (<16 points), king activity and pawn promotion focus
 * 
 * **Evaluation Components:**
 * - **Material Balance**: Piece values with phase-dependent adjustments
 * - **Piece-Square Tables**: Positional bonuses for optimal piece placement
 * - **Tactical Patterns**: Outposts, open files, bishop pairs, king safety
 * - **Pawn Structure**: Passed pawns, isolated pawns, pawn chains
 * - **King Safety**: Attack patterns, shelter evaluation, exposure penalties
 * 
 * **Performance Characteristics:**
 * - Speed: ~50,000 evaluations per second on modern hardware
 * - CPU Usage: ~2% of total engine time during search
 * - Precision: Centipawn granularity with smooth phase transitions
 * 
 * **Design Philosophy:**
 * Based on VICE tutorial evaluation principles with Huginn-specific enhancements:
 * - Incremental evaluation updates during move making/unmaking
 * - Cache-friendly data structures for rapid position assessment
 * - Tuned parameters from game analysis and engine testing
 * 
 * @author MTDuke71  
 * @version 1.2
 * @see evalPosition() for main evaluation entry point
 * @see docs/EVALUATION_DESIGN.md for detailed algorithm documentation
 */
#pragma once

#include "../src/position.hpp"
#include "../src/chess_types.hpp"
#include <array>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

// Castling rights constants - Used for evaluation of castling availability
#define CASTLE_WK 1   // White kingside castling available
#define CASTLE_WQ 2   // White queenside castling available  
#define CASTLE_BK 4   // Black kingside castling available
#define CASTLE_BQ 8   // Black queenside castling available

namespace Huginn {

/**
 * @namespace Huginn::EvalParams
 * @brief Evaluation parameters and constants for position assessment
 *
 * Contains all tunable parameters used by the evaluation function, including
 * piece values, positional bonuses, and game phase thresholds. These values
 * have been tuned through engine testing and analysis of master games.
 */
namespace EvalParams {

// ============================================================================
// GAME PHASE DETECTION - Determines evaluation strategy based on material count
// ============================================================================

/** @brief Material threshold for remaining in opening phase (stay development-focused longer) */
inline constexpr int GAME_PHASE_OPENING_THRESHOLD = 28;  

/** @brief Material threshold for entering endgame phase (king activity becomes crucial) */
inline constexpr int GAME_PHASE_MIDDLEGAME_THRESHOLD = 16;  

// ============================================================================
// TACTICAL BONUSES - Positional and tactical pattern recognition
// ============================================================================

/** @brief Minimum rank for white knight outpost consideration (4th rank+) */
inline constexpr int WHITE_KNIGHT_OUTPOST_MIN_RANK = 3;

/** @brief Maximum rank for black knight outpost consideration (5th rank+) */
inline constexpr int BLACK_KNIGHT_OUTPOST_MAX_RANK = 4;

/** @brief Bonus for knights on strong outpost squares (protected, can't be attacked by pawns) */
inline constexpr int KNIGHT_OUTPOST_BONUS = 25;

/** @brief Bonus for having both bishops (bishop pair advantage) */
inline constexpr int BISHOP_PAIR_BONUS = 50;

/** @brief Tempo bonus — small advantage for the side to move (initiative). */
inline constexpr int TEMPO_BONUS = 10;

// ============================================================================
// OPEN FILE BONUSES - VICE Part 81 implementation
// ============================================================================

/** @brief Rook bonus for controlling an open file (no pawns of either color) */
inline constexpr int ROOK_OPEN_FILE_BONUS = 10;      

/** @brief Rook bonus for controlling a semi-open file (no own pawns, enemy pawns present) */
inline constexpr int ROOK_SEMI_OPEN_FILE_BONUS = 5;  

/** @brief Queen bonus for controlling an open file */
inline constexpr int QUEEN_OPEN_FILE_BONUS = 5;
inline constexpr int QUEEN_SEMI_OPEN_FILE_BONUS = 3; // VICE value: QueenSemiOpenFile = 3

inline constexpr int ISOLATED_PAWN_PENALTY = 10;  // VICE Part 80: Matches original VICE value
inline constexpr int DOUBLED_PAWN_PENALTY = 20;    // Increased penalty

inline constexpr int MOBILITY_WEIGHT_DEFAULT = 5;
inline constexpr int MOBILITY_WEIGHT_ENDGAME = 2;

inline constexpr int DEVELOP_BONUS_DEFAULT = 40;     // Much higher penalty for undevelopment
inline constexpr int DEVELOP_BONUS_OPENING = 60;     // Strong opening development bonus
inline constexpr int DEVELOP_BONUS_ENDGAME = 0;

// VICE Part 82/83: Material draw detection and king evaluation 
// Endgame threshold: approximately equivalent to Rook + Knight + Bishop (about 1150)
// Lowered from 1300 to prevent engine from sacrificing material to force draws
inline constexpr int ENDGAME_MATERIAL_THRESHOLD = 1150;

// ============================================================================
// KING SAFETY (#35 Experiment 3)
// MG-only: the tapered eval blends this out toward the endgame, where the king
// should be active (an untapered KS term sank the #2 attempt at -126 Elo).
// Magnitudes are a conservative first cut — Texel-tune later (#9). Replaces the
// four dead constants (KING_SHIELD_MULTIPLIER / KING_ATTACK_PENALTY /
// CASTLE_BONUS / STUCK_PENALTY) that were defined but never referenced.
// ============================================================================

// Per-attacker weight, per king-ring square attacked, indexed by PieceType
// (None, Pawn, Knight, Bishop, Rook, Queen, King). Heavy pieces weigh more.
inline constexpr std::array<int, size_t(PieceType::_Count)> KS_ATTACK_WEIGHT = {
    0,  // None
    0,  // Pawn  (pawn king-ring pressure folded into shelter, not attack units)
    2,  // Knight
    2,  // Bishop
    3,  // Rook
    5,  // Queen
    0   // King
};

// Non-linear danger conversion: danger = min(units*units / DIVISOR, CAP), and
// only when >= 2 distinct attackers hit the ring (a lone attacker is rarely a
// real threat — the classic king-safety gate).
inline constexpr int KS_ATTACK_DIVISOR = 8;
inline constexpr int KS_ATTACK_CAP     = 500;
inline constexpr int KS_MIN_ATTACKERS  = 2;

// Shelter: penalty per open file on or adjacent to the king's file (no own pawn
// anywhere on that file). Applied regardless of attacker count.
inline constexpr int KS_OPEN_FILE_PENALTY = 18;

// Texel-tuned on Zurichess quiet-labeled (725k pos), #9. rank1 top -> rank8 bottom.
EVAL_PARAM std::array<int, 64> PAWN_TABLE = {
      0,   0,   0,   0,   0,   0,   0,   0,
      2,   4,  -6,  -9,  -4,  14,  12,  -8,
      3,   3,   3,   0,   6,   5,   6,  -5,
      5,   6,   4,  10,   9,   1,   2,  -7,
     23,  22,  13,  13,  14,  12,  17,   9,
     64,  67,  52,  29,  32,  44,  57,  49,
    124, 122,  87,  74,  76,  80,  88, 113,
      0,   0,   0,   0,   0,   0,   0,   0};

EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
    -36,  -8, -14,   1,   3,  -2,  -8, -33,
    -16, -11,   7,  20,  22,  17,  -1,  -9,
    -16,   7,  16,  30,  31,  28,  21, -10,
     -2,  15,  30,  30,  36,  29,  28,   1,
      4,  23,  33,  49,  35,  47,  24,  15,
    -14,  22,  31,  39,  37,  40,  28,  -3,
    -28,   1,  22,  22,  12,  14,   0, -29,
    -88, -32,  -1, -17,   2, -36, -44, -93};

EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
    -12,  10,   2,   9,   7,   1,   1,  -9,
      8,  19,  18,  14,  22,  18,  31,   1,
      5,  20,  24,  20,  21,  26,  19,   8,
      6,  16,  18,  31,  25,   9,  12,  10,
      8,  10,  24,  32,  28,  25,   9,   9,
     -1,  18,  20,  18,  17,  22,  24,   6,
     -5,  11,   7,  -3,  15,  21,  14, -24,
    -10,  -5, -15,  -4,   3,  -4,   1,  -7};

EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
      3,  10,  17,  18,  17,  15,  -6,  -1,
    -10,   4,   5,  10,   7,   9,   2, -25,
     -7,   3,   2,   3,   4,   4,   3, -12,
      2,   9,  12,   9,   7,   4,   5,  -3,
     11,   9,  19,  13,  12,  19,   6,  11,
     16,  20,  17,  18,  10,  14,  18,  10,
     27,  26,  30,  29,  22,  27,  23,  23,
     26,  22,  23,  24,  26,  19,  19,  20};

EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
     -6,  -7,   4,  29,   1, -16, -21, -42,
    -18, -10,  10,  10,  17,   9, -16,   1,
    -13,  -3,  -4,  -4,  -4,   4,  10,   6,
    -15, -19, -12, -14,  -3,   0,   4,   1,
    -22, -25, -16, -15,   8,  12,  15,   1,
    -24, -19, -10,   6,  27,  40,  32,  38,
    -32, -37,  -3,  13,  11,  29,  28,  33,
    -23,   4,   8,   9,  31,  22,  18,  34};

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
EVAL_PARAM std::array<int, 64> KING_TABLE = {
    -34,  27,   7, -95, -18, -55,  26,   6,
    -19,  -7, -33, -94, -74, -50, -11,   0,
    -30, -27, -39, -56, -73, -53, -37, -53,
    -78, -31, -60, -82, -72, -60, -58,-104,
    -59,  -2, -28, -67, -62, -30,  30, -76,
     31,  25,   8, -22,  -8,  64, 118,  -8,
     22,  30,   9,  50,   7,  23,  -2, -30,
     -9,  15,  19,  -1, -25, -13,   3,   2};

// KingE[64] - Endgame king table (encourages centralization)
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
    -60, -35, -13,  12, -15,   2, -35, -61,
    -22,   2,  22,  39,  37,  25,   7, -23,
    -11,  13,  31,  40,  46,  35,  23,   0,
     -3,  16,  44,  51,  49,  42,  27,   6,
     10,  36,  46,  52,  49,  50,  32,  17,
     15,  33,  38,  32,  32,  51,  39,  18,
     -1,  29,  26,  21,  28,  46,  35,  21,
    -83, -27, -12, -14,  -7,  18,  16, -14};

inline constexpr std::array<int, 8> PASSED_PAWN_BONUS = {0,  5,  10,  20,
                                                         35, 60, 100, 200};

// VICE Part 77: Evaluation masks for pawn structure analysis (2:00)
// These bitboards help identify passed pawns and isolated pawns

// File masks: represent all squares on a specific file (A-H)
inline constexpr std::array<uint64_t, 8> FILE_MASKS = {
    0x0101010101010101ULL, // A-file
    0x0202020202020202ULL, // B-file  
    0x0404040404040404ULL, // C-file
    0x0808080808080808ULL, // D-file
    0x1010101010101010ULL, // E-file
    0x2020202020202020ULL, // F-file
    0x4040404040404040ULL, // G-file
    0x8080808080808080ULL  // H-file
};

// Rank masks: represent all squares on a specific rank (1-8)
inline constexpr std::array<uint64_t, 8> RANK_MASKS = {
    0x00000000000000FFULL, // Rank 1
    0x000000000000FF00ULL, // Rank 2
    0x0000000000FF0000ULL, // Rank 3
    0x00000000FF000000ULL, // Rank 4
    0x000000FF00000000ULL, // Rank 5
    0x0000FF0000000000ULL, // Rank 6
    0x00FF000000000000ULL, // Rank 7
    0xFF00000000000000ULL  // Rank 8
};

// Isolated pawn masks: adjacent files to check for supporting pawns
inline constexpr std::array<uint64_t, 8> ISOLATED_PAWN_MASKS = {
    0x0202020202020202ULL, // A-file: only B-file adjacent
    0x0505050505050505ULL, // B-file: A and C files
    0x0A0A0A0A0A0A0A0AULL, // C-file: B and D files
    0x1414141414141414ULL, // D-file: C and E files
    0x2828282828282828ULL, // E-file: D and F files
    0x5050505050505050ULL, // F-file: E and G files
    0xA0A0A0A0A0A0A0A0ULL, // G-file: F and H files
    0x4040404040404040ULL  // H-file: only G-file adjacent
};

// White passed pawn masks: squares that must be clear for a white pawn to be passed
// These will be initialized at runtime by init_evaluation_masks()
extern std::array<uint64_t, 64> WHITE_PASSED_PAWN_MASKS;

// Black passed pawn masks: squares that must be clear for a black pawn to be passed  
// These will be initialized at runtime by init_evaluation_masks()
extern std::array<uint64_t, 64> BLACK_PASSED_PAWN_MASKS;

// VICE Part 78: Initialize evaluation masks (2:19)
void init_evaluation_masks();

} // namespace EvalParams

enum class GamePhase {
    Opening,
    Middlegame,
    Endgame
};
// HybridEvaluator removed: keep EvalParams and GamePhase for Engine

} // namespace Huginn
