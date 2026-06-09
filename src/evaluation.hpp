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

EVAL_PARAM int MOBILITY_WEIGHT_DEFAULT = 5;   // mg, Texel-tunable (#9 round 2)
EVAL_PARAM int MOBILITY_WEIGHT_ENDGAME = 2;   // eg, Texel-tunable

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
     -13,   2, -15, -11,  -9,  18,  29, -12,
      -3,  -3,   5,  -3,   6,   9,  24,  -2,
     -11,  -5,   6,  16,  19,   5,  -2, -22,
      -2,  20,  14,  29,  31,  19,  17, -19,
       2,  -1,  28,  13,  46,  72,  39, -10,
      47, 107,  49,  70,  68,  77, -21,-100,
       0,   0,   0,   0,   0,   0,   0,   0,};;

EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
     -82,  -6, -26, -10,   9,  -5,  -5, -17,
     -13, -24,   8,  21,  24,  25,  -2,  -1,
     -18,   2,  16,  25,  32,  30,  34,  -9,
      -5,  10,  24,  15,  33,  21,  27,   1,
      -5,  20,  12,  54,  29,  60,  19,  25,
     -40,  48,  30,  49,  80, 107,  75,  32,
     -92, -39,  65,  21,  13,  59,   3, -28,
    -176, -63, -23, -21,  10, -82, -41, -99,};;

EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
     -13,  11,   4,   4,   4,  -1, -17, -17,
      15,  29,  27,  14,  24,  20,  45,  12,
       9,  26,  26,  14,  15,  38,  23,  12,
       2,  15,   8,  32,  31,   2,  12,  11,
      -5,   2,  15,  42,  33,  26,   6,   4,
     -31,  24,  29,  22,  22,  41,  25,  -7,
     -38,  10, -21, -21,  21,  53,  21, -58,
     -25,  -7, -95, -58, -10, -20,  -7,   2,};;

EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
       3,  10,  21,  25,  28,  18, -11,   3,
     -22,   4,   5,  12,  11,  11,   4, -49,
     -28,  -8,   0,   0,   3,   6,   3, -14,
     -22,  -8,   5,   3,   8,   3,   8, -10,
     -13,  -4,  13,  14,  12,  20,   6,   3,
       6,  19,  15,  19,   5,  20,  39,  13,
      27,  26,  47,  36,  48,  47,  22,  23,
      27,  21,  24,  25,  29,  18,  21,  22,};;

EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
       4,  14,  26,  42,   8,  -9, -15, -41,
     -17,  -4,  22,  24,  31,  28,  -2,  15,
     -12,   6,  -4,  -4,  -4,   5,  16,  11,
      -9, -36, -16, -29, -14, -12, -10,   2,
     -33, -38, -31, -42, -19,   0,  -5,  -9,
     -23, -21, -10, -17,  21,  39,  32,  46,
     -32, -57,  -9,  11,  -6,  29,  23,  39,
     -25,   1,   7,   9,  33,  27,  24,  36,};;

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
EVAL_PARAM std::array<int, 64> KING_TABLE = {
     -35,  27,   7, -94, -15, -55,  26,   6,
     -19,  -6, -32, -94, -74, -50, -11,   0,
     -31, -27, -40, -59, -73, -55, -38, -53,
     -79, -22, -58, -81, -80, -60, -62,-104,
     -55,  -4, -27, -67, -61, -29,  22, -77,
      30,  29,  13, -20,  -4,  72, 119,  -8,
      28,  32,  10,  62,   9,  25, -11, -39,
     -15,  28,  36,   9, -31, -18,   8,   3,};;

// KingE[64] - Endgame king table (encourages centralization)
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
     -60, -38, -16,  12, -15,   2, -35, -61,
     -22,  -2,  22,  39,  37,  26,   7, -21,
     -11,  13,  31,  42,  48,  37,  23,   2,
      -5,  12,  45,  51,  52,  45,  28,   8,
       6,  33,  44,  53,  49,  50,  35,  18,
      13,  30,  37,  32,  30,  51,  39,  17,
      -7,  23,  23,  19,  27,  46,  36,  20,
     -86, -33, -13, -15,  -5,  20,  11, -13,};;

// Endgame PSTs for the non-king pieces (#9 round 2 — tapered PSTs). Initialized
// as copies of the MG tables, so the eval is byte-identical to baseline-t11
// until the Texel tuner differentiates them. The eval uses these for the eg
// accumulator (search.cpp); the king already has MG + EG tables above.
EVAL_PARAM std::array<int, 64> PAWN_TABLE_EG   = {
       0,   0,   0,   0,   0,   0,   0,   0,
      14,   4,  18,  10,  18,  10,  -2,  -9,
       3,   6,  -1,   4,   6,   2,  -6,  -8,
      15,  14,   1,  -6,  -3,   0,   4,   2,
      35,  21,  12,  -5,  -3,   6,  16,  21,
      89,  90,  65,  34,  24,  32,  61,  71,
     142, 122, 101,  71,  79,  79, 126, 167,
       0,   0,   0,   0,   0,   0,   0,   0,};;
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE_EG = {
      -9, -25,  -3,  11,   0,   3, -22, -41,
     -17,  -2,  10,  14,  16,   6,   0, -23,
      -2,  17,  16,  34,  30,  12,   3,   6,
       7,  17,  37,  50,  38,  39,  28,   6,
       9,  28,  47,  43,  47,  35,  32,   9,
      -2,   7,  31,  31,  11,  13,  -1, -18,
       4,  16,  -6,  24,  13,  -8,  -2, -29,
     -38, -23,   8, -14,   1, -15, -41, -84,};;
EVAL_PARAM std::array<int, 64> BISHOP_TABLE_EG = {
     -10,  10,   2,  10,   9,   6,  11,   2,
       4,  -1,   7,  14,  13,  13,   4, -12,
       7,  13,  24,  24,  30,  11,  15,   5,
       8,  17,  27,  28,  18,  25,  11,   8,
      15,  25,  26,  24,  25,  21,  15,  12,
      22,  12,  17,  16,  13,  17,  20,  20,
      12,  11,  23,   3,  11,   5,  10,   7,
      -4,  -5,   9,  10,   8,   3,   3,  -9,};;
EVAL_PARAM std::array<int, 64> ROOK_TABLE_EG   = {
       3,  10,  13,   7,   6,  11,   7, -13,
       7,   4,   5,  10,   3,   7,   2,  12,
      12,  12,   3,   5,   4,   4,   6,  -4,
      18,  17,  16,  11,   8,   4,   4,   4,
      18,  13,  22,  12,  12,  16,   7,  14,
      19,  18,  16,  16,  12,  13,  10,  11,
      23,  26,  19,  23,  11,  18,  23,  23,
      23,  22,  23,  24,  25,  21,  20,  21,};;
EVAL_PARAM std::array<int, 64> QUEEN_TABLE_EG  = {
     -23, -50, -46, -66,  -9, -30, -28, -46,
     -18, -23, -40, -35, -32, -26, -31, -19,
     -13, -30,  -5,  -5,  -4,   4,  10,   6,
     -16,  15,  -5,  39,  14,  23,  37,  10,
      -8,   3,   3,  28,  38,  25,  43,  33,
     -22, -16, -11,  36,  29,  37,  28,  18,
     -22,   7,   5,  16,  25,  27,  29,  20,
     -24,   8,  12,   8,  30,  20,   9,  29,};;

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
