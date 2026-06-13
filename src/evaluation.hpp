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
EVAL_PARAM int BISHOP_PAIR_BONUS = 45;   // #9 round 3, Texel-tunable

/** @brief Tempo bonus — small advantage for the side to move (initiative). */
EVAL_PARAM int TEMPO_BONUS = 21;

// ============================================================================
// OPEN FILE BONUSES - VICE Part 81 implementation
// ============================================================================

/** @brief Rook bonus for controlling an open file (no pawns of either color) */
EVAL_PARAM int ROOK_OPEN_FILE_BONUS = 17;      

/** @brief Rook bonus for controlling a semi-open file (no own pawns, enemy pawns present) */
EVAL_PARAM int ROOK_SEMI_OPEN_FILE_BONUS = 17;  

/** @brief Queen bonus for controlling an open file */
EVAL_PARAM int QUEEN_OPEN_FILE_BONUS = 2;
EVAL_PARAM int QUEEN_SEMI_OPEN_FILE_BONUS = 6; // VICE value: QueenSemiOpenFile = 3

EVAL_PARAM int ISOLATED_PAWN_PENALTY = 11;  // #9 round 3, Texel-tunable
EVAL_PARAM int DOUBLED_PAWN_PENALTY = 10;

// Connected pawns (#9 round 4): bonus per pawn that is phalanx (own pawn on an
// adjacent file, same rank) or supported (defended by an own pawn), indexed by
// relative rank (White POV; Black mirrors). Ranks 1/8 can't hold pawns, so
// those entries stay pinned at 0. Tapered MG/EG, Texel-tunable.
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_MG = { 0, 2, 6, 12, 17, 29, 43, 0 };
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_EG = { 0, -3, 2, 3, 10, 30, 31, 0 };

// Backward pawn (#9 round 4): no own pawn on an adjacent file at the same rank
// or behind (so it can never be supported by one), and its stop square is
// controlled by an enemy pawn. Isolated pawns are excluded — they already pay
// ISOLATED_PAWN_PENALTY. Tapered MG/EG, Texel-tunable.
EVAL_PARAM int BACKWARD_PAWN_PENALTY_MG = 16;
EVAL_PARAM int BACKWARD_PAWN_PENALTY_EG = 11;

// Rook on the relative 7th rank (#9 round 5): bonus per rook on the enemy's
// 2nd rank, gated on a target — the enemy king confined to its back rank OR
// enemy pawns stuck on that rank. The classic "rook on the 7th" pattern;
// gating avoids rewarding a pointless rook-on-7th in a bare endgame. Tapered
// MG/EG (stronger in the endgame), colour-symmetric, Texel-tunable.
EVAL_PARAM int ROOK_ON_7TH_MG = 20;
EVAL_PARAM int ROOK_ON_7TH_EG = 24;

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
     -13,   2,  -9,  -3,  -1,  28,  37,  -8,
      -6,  -9,   6,   0,  10,  12,  27,  -2,
     -13, -12,   5,  18,  22,  10,   0, -19,
      -9,  15,   7,  29,  29,  18,  17, -18,
      -6, -22,   6, -12,  43,  77,  24, -17,
      40,  96,  23,  71,  62,  77, -49,-110,
       0,   0,   0,   0,   0,   0,   0,   0,};;

EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
     -99,  -7, -34, -14,   9,  -7,  -6, -14,
     -17, -32,   6,  20,  26,  27,   2,   1,
     -20,   3,  16,  25,  32,  31,  34,  -9,
      -5,  12,  23,  16,  33,  23,  30,   0,
      -6,  22,  14,  54,  29,  72,  19,  29,
     -47,  52,  28,  53,  90, 115,  81,  37,
     -90, -44,  76,  25,  13,  66,   0, -20,
    -190, -70, -39, -30,  16,-109, -41,-112,};;

EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
     -20,  11,   4,   5,   8,  -1, -24, -18,
      20,  32,  28,  15,  24,  23,  47,  13,
       9,  28,  26,  13,  15,  39,  23,  14,
       3,  15,   8,  33,  32,   2,  11,  14,
      -5,   3,  11,  48,  33,  28,   7,   2,
     -30,  24,  32,  23,  29,  41,  24, -14,
     -37,  11, -27, -24,  21,  55,  23, -58,
     -31, -14,-111, -79, -17, -32, -12,   1,};;

EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
       3,   6,  21,  25,  28,  23, -14,   5,
     -25,   0,   4,   9,  15,  13,   7, -51,
     -32,  -9,  -3,  -4,   5,   6,   2, -15,
     -29, -15,  -6,   2,   8,   2,  13, -18,
     -13, -11,   0,  12,   8,  21,   0,  -8,
       1,  16,  15,  18,   2,  24,  46,  10,
     -17,  -4,  12,  11,  47,  42,  -2,   6,
      31,  20,  26,  29,  30,  11,  23,  20,};;

EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
      11,  15,  27,  41,  11,  -1,  -7, -39,
     -16,   1,  23,  25,  32,  29,   5,  19,
     -11,   8,  -5,  -3,  -4,   4,  16,  12,
      -8, -41, -18, -28, -15, -13, -12,  -5,
     -38, -43, -40, -43, -22, -10, -12, -13,
     -22, -23,  -8, -32,  23,  45,  33,  44,
     -31, -53, -13,   8, -34,  31,  24,  44,
     -24,  -1,   4,   9,  37,  34,  27,  37,};;

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
EVAL_PARAM std::array<int, 64> KING_TABLE = {
     -35,  27,   8, -91, -15, -55,  26,   6,
     -17,  -6, -32, -94, -74, -50, -11,   0,
     -31, -27, -37, -62, -73, -57, -38, -54,
     -82, -21, -59, -86, -83, -60, -62,-104,
     -54,  -4, -26, -63, -65, -30,  20, -76,
      29,  33,  26, -13,   0,  72, 121,  -8,
      43,  38,  15,  67,   9,  21, -10, -53,
     -33,  55,  56,   6, -30, -14,  10,  -3,};;

// KingE[64] - Endgame king table (encourages centralization)
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
     -60, -38, -16,  14, -15,   6, -35, -61,
     -25,  -2,  24,  39,  38,  26,   7, -21,
     -13,  12,  30,  43,  48,  37,  23,   1,
      -5,  13,  44,  52,  54,  45,  29,   7,
       4,  34,  43,  51,  49,  51,  35,  18,
       8,  29,  32,  27,  27,  50,  38,  15,
     -13,  18,  21,  15,  26,  47,  34,  20,
     -90, -40, -24, -17,  -4,  20,   9, -21,};;

// Endgame PSTs for the non-king pieces (#9 round 2 — tapered PSTs). Initialized
// as copies of the MG tables, so the eval is byte-identical to baseline-t11
// until the Texel tuner differentiates them. The eval uses these for the eg
// accumulator (search.cpp); the king already has MG + EG tables above.
EVAL_PARAM std::array<int, 64> PAWN_TABLE_EG   = {
       0,   0,   0,   0,   0,   0,   0,   0,
      20,   8,  18,  10,  17,  10,  -2,  -5,
       8,  10,   1,   9,   6,   2,  -6,  -4,
      20,  14,   1,  -6,  -4,  -4,   1,   4,
      35,  19,  10, -11,  -7,  -1,   9,  19,
      74,  72,  45,  13,  -8,   9,  39,  57,
     150, 126, 110,  76,  84,  79, 132, 177,
       0,   0,   0,   0,   0,   0,   0,   0,};;
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE_EG = {
       9, -27,   4,  14,   3,   5, -20, -44,
     -11,   4,  14,  14,  16,   5,   5, -23,
       3,  20,  17,  36,  30,  12,   3,   6,
      10,  20,  37,  50,  38,  42,  32,   7,
      12,  28,  48,  44,  47,  29,  34,   8,
       1,   3,  32,  29,   8,   7,  -6, -24,
       5,  22, -10,  22,  12, -13,  -3, -27,
     -32, -19,   9, -12,   0,  -9, -42, -79,};;
EVAL_PARAM std::array<int, 64> BISHOP_TABLE_EG = {
      -3,  12,   2,  10,  10,   6,  14,   6,
       3,  -1,   6,  14,  13,  15,   1, -14,
       6,  12,  24,  26,  31,  11,  14,   5,
      11,  16,  28,  28,  18,  25,  10,   9,
      15,  25,  26,  23,  25,  22,  15,  15,
      22,  12,  15,  13,  11,  15,  17,  22,
      15,  12,  24,   6,  11,   5,   7,   7,
      -2,  -4,  15,  14,  10,   3,   4, -10,};;
EVAL_PARAM std::array<int, 64> ROOK_TABLE_EG   = {
       7,  10,   8,   7,   1,   7,  10, -13,
       9,   5,   5,  10,   1,   7,  -2,  16,
      14,  12,   3,   5,   3,   3,   5,   0,
      22,  20,  18,  10,   6,   5,   5,   9,
      21,  17,  26,  11,  12,  16,  12,  21,
      22,  18,  16,  15,  15,  11,  10,  13,
      17,  13,   7,   9,  -7,   3,  18,  15,
      21,  21,  20,  20,  21,  23,  19,  21,};;
EVAL_PARAM std::array<int, 64> QUEEN_TABLE_EG  = {
     -36, -56, -54, -66, -15, -36, -33, -47,
     -22, -24, -42, -41, -41, -32, -40, -25,
     -13, -44,  -4,  -6,  -2,   6,   9,   5,
     -17,  27,   2,  45,  14,  24,  37,  23,
       5,  16,  19,  39,  47,  35,  56,  44,
     -22, -10, -10,  54,  34,  34,  26,  18,
     -20,   7,  18,  24,  56,  26,  22,  13,
     -23,  14,  16,  14,  23,  20,   8,  25,};;

EVAL_PARAM std::array<int, 8> PASSED_PAWN_BONUS = { 0, 3, 5, 22, 44, 88, 105, 200 };

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
