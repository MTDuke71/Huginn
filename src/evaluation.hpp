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
EVAL_PARAM int BISHOP_PAIR_BONUS = 44;   // #9 round 3, Texel-tunable

/** @brief Tempo bonus — small advantage for the side to move (initiative). */
EVAL_PARAM int TEMPO_BONUS = 26;

// ============================================================================
// OPEN FILE BONUSES - VICE Part 81 implementation
// ============================================================================

/** @brief Rook bonus for controlling an open file (no pawns of either color) */
EVAL_PARAM int ROOK_OPEN_FILE_BONUS = 17;      

/** @brief Rook bonus for controlling a semi-open file (no own pawns, enemy pawns present) */
EVAL_PARAM int ROOK_SEMI_OPEN_FILE_BONUS = 17;  

/** @brief Queen bonus for controlling an open file */
EVAL_PARAM int QUEEN_OPEN_FILE_BONUS = 4;
EVAL_PARAM int QUEEN_SEMI_OPEN_FILE_BONUS = 6; // VICE value: QueenSemiOpenFile = 3

EVAL_PARAM int ISOLATED_PAWN_PENALTY = 11;  // #9 round 3, Texel-tunable
EVAL_PARAM int DOUBLED_PAWN_PENALTY = 11;

// Connected pawns (#9 round 4): bonus per pawn that is phalanx (own pawn on an
// adjacent file, same rank) or supported (defended by an own pawn), indexed by
// relative rank (White POV; Black mirrors). Ranks 1/8 can't hold pawns, so
// those entries stay pinned at 0. Tapered MG/EG, Texel-tunable.
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_MG = { 0, 2, 6, 12, 17, 29, 50, 0 };
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_EG = { 0, -3, 2, 3, 10, 31, 29, 0 };

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

// Threats (#9 round 6): bonus per enemy piece attacked by a cheaper / more
// dangerous attacker — the side to move usually wins material or forces a
// concession. Indexed by attacker→target class (pawn→minor/rook/queen,
// minor→rook/queen, rook→queen). Tapered MG/EG, colour-symmetric, Texel-
// tunable. Less constrained by quiet data than pawn structure (threats fire
// less often in quiet positions) — watch the fitted magnitudes for sanity.
EVAL_PARAM int THREAT_PAWN_ON_MINOR_MG  = 67;
EVAL_PARAM int THREAT_PAWN_ON_MINOR_EG  = 41;
EVAL_PARAM int THREAT_PAWN_ON_ROOK_MG   = 79;
EVAL_PARAM int THREAT_PAWN_ON_ROOK_EG   = 16;
EVAL_PARAM int THREAT_PAWN_ON_QUEEN_MG  = 67;
EVAL_PARAM int THREAT_PAWN_ON_QUEEN_EG  = 33;
EVAL_PARAM int THREAT_MINOR_ON_ROOK_MG  = 48;
EVAL_PARAM int THREAT_MINOR_ON_ROOK_EG  = 30;
EVAL_PARAM int THREAT_MINOR_ON_QUEEN_MG = 50;
EVAL_PARAM int THREAT_MINOR_ON_QUEEN_EG = 26;
EVAL_PARAM int THREAT_ROOK_ON_QUEEN_MG  = 76;
EVAL_PARAM int THREAT_ROOK_ON_QUEEN_EG  = 30;

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
     -13,   2,  -7,  -1,  -1,  28,  37,  -8,
      -6,  -9,   6,   0,  10,  12,  27,  -2,
     -13, -12,   7,  18,  21,  10,   0, -19,
      -9,  11,   7,  23,  25,  18,  13, -18,
      -7, -23,   4, -14,  43,  77,  24, -17,
      32,  64,  11,  66,  56,  77, -66,-112,
       0,   0,   0,   0,   0,   0,   0,   0,};;

EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
    -107,  -9, -38, -14,   7,  -6,  -7, -14,
     -17, -31,   6,  19,  23,  26,   1,   1,
     -21,   2,  16,  25,  32,  31,  33,  -9,
      -5,  13,  23,  18,  35,  24,  34,   2,
      -7,  26,  13,  53,  30,  74,  26,  30,
     -48,  48,  19,  53,  90, 116,  81,  40,
     -91, -58,  54,  23,  13,  47,   1, -24,
    -197, -77, -49, -35,  20,-115, -44,-112,};;

EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
     -20,  11,   4,   6,   9,  -2, -24, -16,
      20,  30,  26,  15,  24,  25,  47,  13,
       7,  27,  24,  13,  15,  42,  22,  14,
       3,  15,  14,  33,  38,   3,  14,  15,
      -4,  13,  12,  50,  32,  32,  12,   9,
      -6,  23,  32,  24,  33,  45,  43,  18,
     -39,  -7, -28, -38,   8,  55,   4, -57,
     -32, -16,-115, -85, -31, -40, -16,  -1,};;

EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
       1,   6,  21,  25,  28,  23, -14,   5,
     -25,   0,   2,   9,  15,  13,   8, -50,
     -30, -10,  -3,   0,   5,   7,   7, -15,
     -29, -14,  -6,   1,  14,   2,  19, -20,
     -13, -13,   0,  17,  10,  30,   0, -12,
       1,  17,  15,  19,   4,  27,  54,  10,
     -17,  -4,  18,  11,  47,  44,   0,   6,
      31,  22,  26,  32,  33,  11,  22,  20,};;

EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
      10,  15,  25,  36,   9,  -2,  -7, -39,
     -16,   1,  20,  22,  29,  28,   5,  21,
     -10,   7,  -5,  -3,  -5,   4,  13,  10,
      -8, -37, -13, -26,  -9, -10,  -4,  -5,
     -38, -36, -38, -38, -20,  -2,  -7,  -5,
     -15, -22,  -4, -32,  25,  49,  50,  51,
     -31, -53, -17,   6, -35,  31,  23,  49,
     -24,  -1,   4,   9,  55,  42,  29,  39,};;

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
EVAL_PARAM std::array<int, 64> KING_TABLE = {
     -34,  30,   8, -91, -15, -55,  26,   6,
     -17,  -6, -32, -94, -74, -50, -11,   0,
     -31, -27, -38, -62, -73, -57, -37, -54,
     -86, -20, -58, -86, -86, -61, -65,-106,
     -52,  -5, -24, -63, -67, -30,  17, -77,
      29,  32,  28,  -5,   1,  73, 121,  -9,
      48,  39,  15,  69,  10,  25, -11, -61,
     -38,  65,  76,   9, -32, -13,  13,   0,};;

// KingE[64] - Endgame king table (encourages centralization)
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
     -60, -38, -16,  12, -15,   6, -35, -61,
     -24,  -2,  24,  39,  38,  27,   7, -21,
     -13,  12,  30,  43,  48,  37,  23,   1,
      -5,  13,  43,  52,  54,  44,  29,   7,
       2,  34,  42,  51,  49,  51,  36,  17,
       8,  27,  31,  26,  28,  50,  38,  17,
     -18,  19,  20,  15,  26,  47,  34,  20,
     -90, -43, -27, -18,  -3,  19,   9, -21,};;

// Endgame PSTs for the non-king pieces (#9 round 2 — tapered PSTs). Initialized
// as copies of the MG tables, so the eval is byte-identical to baseline-t11
// until the Texel tuner differentiates them. The eval uses these for the eg
// accumulator (search.cpp); the king already has MG + EG tables above.
EVAL_PARAM std::array<int, 64> PAWN_TABLE_EG   = {
       0,   0,   0,   0,   0,   0,   0,   0,
      20,   8,  18,   9,  17,  10,  -2,  -5,
       8,  10,   1,   9,   6,   2,  -6,  -4,
      20,  14,   1,  -6,  -4,  -4,   1,   4,
      35,  19,   7, -11,  -7,  -1,   8,  16,
      72,  69,  43,  12,  -8,   3,  38,  53,
     156, 135, 115,  76,  85,  81, 137, 179,
       0,   0,   0,   0,   0,   0,   0,   0,};;
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE_EG = {
      12, -27,   4,  14,   3,   7, -20, -43,
     -10,   3,  14,  13,  16,   3,   4, -29,
       4,  21,  17,  36,  30,  12,   3,   2,
      11,  20,  40,  50,  38,  42,  32,   8,
      11,  28,  49,  44,  47,  29,  33,   8,
       2,   4,  33,  25,   8,   6,  -6, -23,
       3,  23,  -8,  21,  11, -11,  -4, -28,
     -25, -20,  10, -11,  -5,  -7, -43, -79,};;
EVAL_PARAM std::array<int, 64> BISHOP_TABLE_EG = {
      -2,  13,   2,  12,  10,   6,  15,   6,
       2,  -1,   6,  14,  13,  13,   1, -16,
       8,  12,  24,  26,  31,  11,  14,   4,
      10,  16,  28,  30,  18,  25,  13,   9,
      16,  24,  26,  22,  25,  23,  16,  18,
      18,  12,  14,  12,  10,  16,  18,  15,
      15,  14,  23,   8,  11,   2,  11,   7,
      -2,  -5,  15,  15,  13,   6,   3, -12,};;
EVAL_PARAM std::array<int, 64> ROOK_TABLE_EG   = {
       7,  10,   8,   7,   1,   7,  10, -13,
      11,   5,   7,  10,   1,   7,  -2,  17,
      15,  13,   5,   6,   3,   3,   5,   0,
      25,  23,  21,  14,   7,  11,   5,  12,
      23,  21,  27,  13,  14,  18,  14,  25,
      22,  20,  16,  15,  15,  13,  10,  13,
      18,  14,   9,   9, -12,   5,  16,  16,
      21,  20,  20,  18,  20,  23,  19,  20,};;
EVAL_PARAM std::array<int, 64> QUEEN_TABLE_EG  = {
     -41, -61, -55, -68, -24, -42, -34, -48,
     -24, -31, -42, -41, -41, -34, -45, -28,
     -13, -44,  -4,  -8,  -2,   6,   9,   6,
     -21,  27,   3,  46,  14,  24,  36,  25,
       9,  18,  25,  41,  53,  38,  62,  44,
     -22,  -5, -10,  60,  38,  33,  23,  17,
     -20,  16,  22,  25,  56,  26,  24,  12,
     -23,  14,  18,  15,  17,  20,   8,  26,};;

EVAL_PARAM std::array<int, 8> PASSED_PAWN_BONUS = { 0, 3, 5, 21, 44, 89, 104, 200 };

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
