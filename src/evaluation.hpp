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

// Outposts (#9 round 8 candidate): knight/bishop on an advanced square,
// supported by an own pawn, that enemy pawns on adjacent files cannot
// challenge by advancing. Tapered MG/EG and Texel-tunable.
EVAL_PARAM int KNIGHT_OUTPOST_BONUS_MG = 33;
EVAL_PARAM int KNIGHT_OUTPOST_BONUS_EG = 11;
EVAL_PARAM int BISHOP_OUTPOST_BONUS_MG = 28;
EVAL_PARAM int BISHOP_OUTPOST_BONUS_EG = 7;

/** @brief Bonus for having both bishops (bishop pair advantage) */
EVAL_PARAM int BISHOP_PAIR_BONUS = 45;   // #9 round 3, Texel-tunable

/** @brief Tempo bonus — small advantage for the side to move (initiative). */
EVAL_PARAM int TEMPO_BONUS = 26;

// ============================================================================
// OPEN FILE BONUSES - VICE Part 81 implementation
// ============================================================================

/** @brief Rook bonus for controlling an open file (no pawns of either color) */
EVAL_PARAM int ROOK_OPEN_FILE_BONUS = 16;      

/** @brief Rook bonus for controlling a semi-open file (no own pawns, enemy pawns present) */
EVAL_PARAM int ROOK_SEMI_OPEN_FILE_BONUS = 16;  

/** @brief Queen bonus for controlling an open file */
EVAL_PARAM int QUEEN_OPEN_FILE_BONUS = 3;
EVAL_PARAM int QUEEN_SEMI_OPEN_FILE_BONUS = 5; // VICE value: QueenSemiOpenFile = 3

EVAL_PARAM int ISOLATED_PAWN_PENALTY = 10;  // #9 round 3, Texel-tunable
EVAL_PARAM int DOUBLED_PAWN_PENALTY = 10;

// Connected pawns (#9 round 4): bonus per pawn that is phalanx (own pawn on an
// adjacent file, same rank) or supported (defended by an own pawn), indexed by
// relative rank (White POV; Black mirrors). Ranks 1/8 can't hold pawns, so
// those entries stay pinned at 0. Tapered MG/EG, Texel-tunable.
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_MG = { 0, 2, 8, 14, 19, 37, 56, 0 };
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_EG = { 0, -5, 2, 3, 10, 32, 28, 0 };

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
EVAL_PARAM int ROOK_ON_7TH_EG = 22;

// Threats (#9 round 6): bonus per enemy piece attacked by a cheaper / more
// dangerous attacker — the side to move usually wins material or forces a
// concession. Indexed by attacker→target class (pawn→minor/rook/queen,
// minor→rook/queen, rook→queen). Tapered MG/EG, colour-symmetric, Texel-
// tunable. Less constrained by quiet data than pawn structure (threats fire
// less often in quiet positions) — watch the fitted magnitudes for sanity.
EVAL_PARAM int THREAT_PAWN_ON_MINOR_MG  = 67;
EVAL_PARAM int THREAT_PAWN_ON_MINOR_EG  = 44;
EVAL_PARAM int THREAT_PAWN_ON_ROOK_MG   = 89;
EVAL_PARAM int THREAT_PAWN_ON_ROOK_EG   = 12;
EVAL_PARAM int THREAT_PAWN_ON_QUEEN_MG  = 66;
EVAL_PARAM int THREAT_PAWN_ON_QUEEN_EG  = 30;
EVAL_PARAM int THREAT_MINOR_ON_ROOK_MG  = 50;
EVAL_PARAM int THREAT_MINOR_ON_ROOK_EG  = 30;
EVAL_PARAM int THREAT_MINOR_ON_QUEEN_MG = 52;
EVAL_PARAM int THREAT_MINOR_ON_QUEEN_EG = 24;
EVAL_PARAM int THREAT_ROOK_ON_QUEEN_MG  = 83;
EVAL_PARAM int THREAT_ROOK_ON_QUEEN_EG  = 27;

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
// #9 round 7: reformulated to be Texel-tunable. The previous design gated on
// >= 2 distinct attackers, which made the term ZERO on most quiet positions —
// so the tuner couldn't constrain it and hand-tuning stalled at ~0 Elo (#35
// Exp 3). The gate is now removed (danger fires on >= 1 attacker, like the
// MTLChess recipe that scored +116 there), and these weights + the shelter
// penalty are EVAL_PARAM so the harness fits them. Seeded at the MTLChess
// values (N/B=2, R=3, Q=5).
EVAL_PARAM std::array<int, size_t(PieceType::_Count)> KS_ATTACK_WEIGHT = { 0, 0, 3, 4, 3, 5, 0 };

// Non-linear danger conversion: danger = min(units*units / DIVISOR, CAP),
// applied whenever ANY enemy piece attacks the king ring (no min-attacker gate
// — that gate is what made the term untunable). DIVISOR/CAP are fixed structure
// (only the ratio weight^2/DIVISOR matters, and the weights are tunable, so the
// tuner has full freedom without a tunable divisor). DIVISOR=4 matches MTLChess.
inline constexpr int KS_ATTACK_DIVISOR = 4;
inline constexpr int KS_ATTACK_CAP     = 500;

// Shelter: penalty per open file on or adjacent to the king's file (no own pawn
// anywhere on that file). Fires very often → aids tunability. Texel-tunable.
EVAL_PARAM int KS_OPEN_FILE_PENALTY = 21;

// Texel-tuned on Zurichess quiet-labeled (725k pos), #9. rank1 top -> rank8 bottom.
EVAL_PARAM std::array<int, 64> PAWN_TABLE = {
       0,   0,   0,   0,   0,   0,   0,   0,
     -15,   1,  -5,  -1,   1,  33,  39,  -8,
      -9,  -9,   6,  -1,  10,  14,  27,  -4,
     -13, -11,   8,  18,  22,  12,   0, -20,
      -9,  12,   8,  23,  23,  20,  15, -17,
      -7, -23,   5, -13,  42,  77,  25, -16,
      30,  50,   9,  61,  50,  78, -76,-110,
       0,   0,   0,   0,   0,   0,   0,   0,};;

EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
    -107, -10, -37, -14,   8,  -5,  -8, -13,
     -20, -29,   5,  19,  23,  27,  -2,  -1,
     -21,   3,  17,  26,  34,  33,  34,  -9,
      -7,  11,  23,  16,  37,  24,  31,   2,
      -8,  26,   4,  48,  24,  68,  22,  30,
     -49,  39,  19,  46,  79, 118,  72,  40,
     -94, -61,  51,  15,  12,  39,  -4, -25,
    -194, -83, -52, -35,  23,-119, -46,-112,};;

EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
     -19,  11,   3,   7,  10,  -3, -18, -12,
      16,  30,  26,  15,  25,  26,  47,  15,
       3,  27,  26,  13,  16,  43,  23,  15,
       3,  13,  13,  36,  39,   5,  13,  16,
      -3,  12,  11,  43,  28,  30,  12,   8,
      -3,  27,  28,  22,  32,  53,  35,  16,
     -37, -13, -37, -45,   4,  55,  -8, -58,
     -36, -23,-115, -90, -38, -49, -18,   1,};;

EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
       1,   6,  21,  25,  28,  22, -14,   5,
     -27,   1,   2,   9,  16,  16,   7, -50,
     -29, -11,  -1,   1,   5,   9,   8, -16,
     -29, -15,  -5,   2,  14,  -1,  18, -21,
     -15, -14,   1,  17,  10,  30,  -2, -11,
       1,  18,  18,  21,   4,  29,  54,  10,
     -13,  -1,  19,  12,  49,  44,  -2,   3,
      28,  20,  25,  33,  39,   9,  21,  17,};;

EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
      14,  17,  27,  38,  13,  -1,  -2, -35,
     -15,   4,  22,  24,  32,  31,   8,  26,
     -10,  11,  -4,  -1,  -2,   5,  14,   8,
      -4, -38, -11, -25, -10, -13,  -7,  -7,
     -34, -31, -39, -47, -27, -14, -15, -17,
     -11, -20,  -3, -44,   6,  18,  15,  21,
     -29, -52, -21,   4, -54,  18,   5,  37,
     -27,  -7,   0, -11,  47,  40,  29,  38,};;

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
EVAL_PARAM std::array<int, 64> KING_TABLE = {
     -34,  30,   8, -88, -15, -55,  26,   6,
     -18,  -6, -32, -93, -68, -50,  -5,   0,
     -34, -22, -35, -60, -73, -56, -37, -54,
     -88,  -7, -58, -85, -86, -61, -65,-109,
     -47,  -6, -23, -62, -67, -30,  17, -81,
      32,  30,  31,  -2,   4,  75, 122, -15,
      51,  45,  17,  74,   8,  23, -12, -68,
     -51,  71,  85,  19, -35, -16,  11,   3,};;

// KingE[64] - Endgame king table (encourages centralization)
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
     -62, -41, -16,  11, -15,   6, -35, -61,
     -26,  -2,  24,  39,  38,  28,   4, -21,
     -16,  11,  31,  44,  49,  38,  23,  -2,
      -6,  10,  45,  53,  56,  45,  29,   7,
      -1,  35,  42,  52,  51,  51,  36,  16,
       4,  28,  32,  27,  28,  50,  39,  16,
     -23,  18,  21,  16,  27,  48,  37,  20,
     -90, -48, -31, -20,  -3,  17,   5, -25,};;

// Endgame PSTs for the non-king pieces (#9 round 2 — tapered PSTs). Initialized
// as copies of the MG tables, so the eval is byte-identical to baseline-t11
// until the Texel tuner differentiates them. The eval uses these for the eg
// accumulator (search.cpp); the king already has MG + EG tables above.
EVAL_PARAM std::array<int, 64> PAWN_TABLE_EG   = {
       0,   0,   0,   0,   0,   0,   0,   0,
      20,  12,  18,   9,  17,   6,  -2,  -1,
      10,  10,   0,   6,   6,   0,  -9,  -4,
      20,  14,   1,  -6,  -5,  -4,  -1,   4,
      34,  17,   7, -11,  -7,  -2,   8,  16,
      73,  70,  42,  10, -11,   2,  37,  54,
     158, 136, 115,  80,  87,  78, 142, 179,
       0,   0,   0,   0,   0,   0,   0,   0,};;
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE_EG = {
      13, -25,   5,  15,   2,   4, -19, -43,
     -10,   3,  14,  13,  16,   4,   5, -26,
       4,  21,  19,  36,  30,  12,   3,   4,
       6,  18,  36,  47,  38,  38,  32,   7,
      11,  23,  44,  40,  44,  29,  31,   5,
       1,   0,  32,  22,   7,   0,  -6, -25,
       3,  22,  -8,  20,  11, -13,  -4, -30,
     -25, -19,  11, -11,  -6,  -7, -45, -77,};;
EVAL_PARAM std::array<int, 64> BISHOP_TABLE_EG = {
       0,  12,   2,  13,  12,   6,  17,   5,
       4,  -2,   6,  14,  13,  12,   1, -15,
       8,  13,  24,  26,  31,  11,  14,   2,
      10,  15,  28,  24,  18,  25,  13,   8,
      17,  20,  23,  21,  22,  20,  15,  18,
      16,  11,  13,   7,   6,  10,  15,  13,
      16,  17,  26,   9,  12,   3,  12,   8,
       1,  -1,  13,  16,  16,   7,   2, -11,};;
EVAL_PARAM std::array<int, 64> ROOK_TABLE_EG   = {
       7,  10,   8,   7,   1,   3,  10, -13,
      13,   5,   7,  10,   1,   3,  -3,  17,
      15,  16,   5,   6,   3,   3,   5,   0,
      24,  23,  22,  14,   7,  10,   4,  15,
      24,  21,  29,  11,  13,  18,  15,  24,
      22,  20,  16,  15,  15,  10,   8,  13,
      18,  14,   9,   9, -14,   4,  17,  16,
      20,  19,  18,  18,  17,  22,  18,  20,};;
EVAL_PARAM std::array<int, 64> QUEEN_TABLE_EG  = {
     -40, -61, -51, -68, -24, -41, -31, -45,
     -23, -31, -41, -41, -41, -33, -45, -27,
     -13, -44,  -4,  -9,   2,   6,  10,   7,
     -19,  38,   5,  46,  17,  24,  34,  23,
      15,  20,  26,  47,  53,  37,  60,  44,
     -22,  -1, -10,  60,  39,  33,  22,  12,
     -19,  16,  22,  22,  56,  25,  23,  -5,
     -23,  14,  16,  13,  -1,   1,  -8,  11,};;

EVAL_PARAM std::array<int, 8> PASSED_PAWN_BONUS = { 0, 2, 4, 21, 44, 89, 104, 200 };

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
