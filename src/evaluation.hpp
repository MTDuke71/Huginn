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
EVAL_PARAM int BISHOP_PAIR_BONUS = 47;   // #9 round 3, Texel-tunable

/** @brief Tempo bonus — small advantage for the side to move (initiative). */
EVAL_PARAM int TEMPO_BONUS = 20;

// ============================================================================
// OPEN FILE BONUSES - VICE Part 81 implementation
// ============================================================================

/** @brief Rook bonus for controlling an open file (no pawns of either color) */
EVAL_PARAM int ROOK_OPEN_FILE_BONUS = 17;      

/** @brief Rook bonus for controlling a semi-open file (no own pawns, enemy pawns present) */
EVAL_PARAM int ROOK_SEMI_OPEN_FILE_BONUS = 16;  

/** @brief Queen bonus for controlling an open file */
EVAL_PARAM int QUEEN_OPEN_FILE_BONUS = 2;
EVAL_PARAM int QUEEN_SEMI_OPEN_FILE_BONUS = 5; // VICE value: QueenSemiOpenFile = 3

EVAL_PARAM int ISOLATED_PAWN_PENALTY = 12;  // #9 round 3, Texel-tunable
EVAL_PARAM int DOUBLED_PAWN_PENALTY = 11;

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
     -13,   2, -15,  -9,  -9,  18,  29, -12,
      -3,  -3,   5,   0,   6,   9,  24,  -2,
     -11,  -5,   6,  19,  19,   5,  -2, -22,
      -2,  20,  14,  29,  31,  19,  17, -19,
      -1, -15,   7, -11,  44,  69,  28, -21,
      48,  99,  31,  70,  67,  77, -41,-104,
       0,   0,   0,   0,   0,   0,   0,   0,};;

EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
     -90,  -6, -31, -11,   9,  -5,  -5, -16,
     -17, -30,   8,  21,  24,  25,  -1,   0,
     -19,   2,  16,  25,  32,  30,  34,  -9,
      -5,  11,  24,  15,  33,  21,  29,   1,
      -6,  22,  12,  54,  29,  64,  19,  28,
     -48,  48,  28,  50,  90, 109,  78,  36,
     -90, -42,  73,  23,  13,  62,   1, -22,
    -187, -67, -28, -21,  10, -97, -41,-110,};;

EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
     -20,  12,   4,   5,   6,  -1, -24, -18,
      19,  32,  29,  15,  24,  23,  45,  14,
       9,  25,  26,  14,  15,  38,  23,  13,
       2,  15,   8,  32,  31,   2,  12,  12,
      -5,   2,  15,  43,  33,  26,   7,   2,
     -31,  24,  29,  22,  25,  41,  25, -12,
     -37,  10, -24, -22,  23,  54,  22, -58,
     -30,  -8, -95, -69, -13, -29, -11,   0,};;

EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
       3,  10,  21,  25,  28,  18, -14,   3,
     -22,   4,   5,  12,  11,  11,   4, -49,
     -28,  -9,   0,   0,   3,   6,   3, -14,
     -24,  -8,  -4,   3,   8,   3,  11, -14,
     -12, -11,   0,  14,   9,  19,   5,   3,
       7,  18,  15,  17,   4,  20,  39,  13,
      27,  24,  47,  36,  50,  56,  23,  27,
      28,  21,  24,  26,  29,  19,  22,  18,};;

EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
       6,  14,  26,  41,   8,  -9, -11, -41,
     -17,  -3,  23,  24,  31,  28,   0,  15,
     -12,   8,  -5,  -5,  -4,   5,  16,  12,
      -9, -37, -19, -28, -14, -12, -12,  -4,
     -36, -41, -39, -40, -18,  -4,  -6, -11,
     -23, -21, -10, -19,  21,  39,  33,  45,
     -33, -57,  -9,   8, -19,  30,  25,  41,
     -25,  -1,   7,   9,  33,  34,  24,  36,};;

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
EVAL_PARAM std::array<int, 64> KING_TABLE = {
     -35,  27,   7, -93, -15, -55,  26,   6,
     -19,  -6, -32, -94, -74, -50, -11,   0,
     -30, -27, -39, -59, -73, -55, -38, -53,
     -81, -21, -58, -83, -80, -60, -62,-104,
     -57,  -4, -27, -67, -65, -30,  22, -75,
      29,  31,  24, -15,  -2,  73, 119,  -9,
      35,  41,  11,  63,   9,  26,  -8, -49,
     -25,  44,  47,   8, -33, -17,  12,  -1,};;

// KingE[64] - Endgame king table (encourages centralization)
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
     -60, -38, -16,  11, -15,   2, -35, -61,
     -22,  -2,  22,  39,  37,  26,   7, -21,
     -13,  13,  31,  42,  48,  37,  23,   2,
      -5,  13,  44,  52,  54,  45,  29,   8,
       4,  33,  43,  53,  49,  50,  35,  18,
      10,  29,  33,  30,  29,  50,  39,  18,
     -13,  19,  22,  16,  27,  46,  35,  20,
     -91, -36, -21, -15,  -5,  20,  10, -20,};;

// Endgame PSTs for the non-king pieces (#9 round 2 — tapered PSTs). Initialized
// as copies of the MG tables, so the eval is byte-identical to baseline-t11
// until the Texel tuner differentiates them. The eval uses these for the eg
// accumulator (search.cpp); the king already has MG + EG tables above.
EVAL_PARAM std::array<int, 64> PAWN_TABLE_EG   = {
       0,   0,   0,   0,   0,   0,   0,   0,
      14,   4,  18,  10,  18,  10,  -2,  -9,
       3,   6,  -1,   4,   6,   2,  -6,  -8,
      15,  14,   1,  -6,  -3,   0,   4,   2,
      35,  21,  12,  -5,  -3,   2,  16,  21,
      77,  75,  49,  17,   1,  12,  44,  57,
     150, 124, 107,  75,  80,  80, 129, 172,
       0,   0,   0,   0,   0,   0,   0,   0,};;
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE_EG = {
       0, -25,   0,  10,   1,   4, -22, -41,
     -13,   0,  12,  14,  16,   6,   2, -23,
       2,  20,  16,  34,  30,  12,   3,   6,
       7,  20,  37,  50,  38,  39,  30,   6,
      11,  28,  47,  43,  47,  33,  32,   7,
       1,   4,  31,  29,   9,   7,  -3, -21,
       4,  17,  -8,  22,  11, -11,  -4, -28,
     -30, -19,   8, -13,   1, -14, -41, -82,};;
EVAL_PARAM std::array<int, 64> BISHOP_TABLE_EG = {
      -3,  11,   2,  10,  10,   6,  14,   3,
       3,  -1,   8,  14,  13,  13,   1, -15,
       6,  13,  24,  24,  30,  11,  15,   6,
      10,  17,  27,  28,  18,  25,  10,   9,
      15,  25,  26,  24,  25,  21,  15,  15,
      22,  12,  15,  14,  12,  15,  18,  24,
      14,  12,  24,   4,  11,   5,  10,   8,
      -2,  -5,   9,  11,   9,   3,   1, -12,};;
EVAL_PARAM std::array<int, 64> ROOK_TABLE_EG   = {
       3,  10,   8,   7,   0,  11,  10, -13,
       7,   4,   5,  10,   3,   7,  -1,  15,
      12,  12,   3,   5,   4,   4,   5,  -3,
      19,  17,  18,  11,   7,   4,   5,   6,
      20,  17,  25,  12,  12,  16,   9,  16,
      20,  18,  16,  15,  12,  13,  11,  11,
      23,  23,  19,  21,   7,  18,  24,  22,
      23,  21,  23,  20,  25,  22,  19,  21,};;
EVAL_PARAM std::array<int, 64> QUEEN_TABLE_EG  = {
     -27, -56, -52, -66, -13, -30, -32, -48,
     -21, -24, -42, -38, -36, -26, -34, -19,
     -13, -38,  -4,  -6,  -4,   4,   9,   4,
     -17,  24,   2,  44,  14,  23,  38,  21,
       5,  13,  16,  31,  44,  25,  43,  40,
     -22, -14, -11,  39,  33,  34,  28,  18,
     -22,   7,  14,  20,  45,  28,  25,  20,
     -24,  10,  12,  10,  30,  20,  11,  28,};;

EVAL_PARAM std::array<int, 8> PASSED_PAWN_BONUS = { 0, 2, 5, 21, 41, 85, 103, 200 };

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
