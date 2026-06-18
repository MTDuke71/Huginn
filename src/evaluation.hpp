/**
 * @file evaluation.hpp
 * @brief Chess position evaluation system with game phase awareness and tactical pattern recognition
 * 
 * Implements a sophisticated evaluation function that assesses chess positions through multiple
 * game phases (opening, middlegame, endgame) using material balance, piece-square tables,
 * tactical bonuses, and positional factors. The evaluation provides the foundation for the
 * engine's search algorithm to distinguish between chess positions.
 * 
 * @note This header holds only the **parameters** — piece-square tables, the
 *       tunable `EVAL_PARAM` bonuses/penalties, and the pawn-structure masks.
 *       The evaluation itself (`evaluate()`) lives in search.cpp.
 *
 * ## Architecture Overview
 *
 * **Game phase — tapered.** The production eval blends midgame and endgame
 * scores smoothly via `game_phase_256()` (256 = opening … 0 = bare kings), so
 * most terms below carry separate `_MG` / `_EG` values. The coarse 3-bucket
 * @ref Huginn::GamePhase enum and its `GAME_PHASE_*_THRESHOLD` constants are
 * legacy from the pre-tapered design and are largely superseded.
 *
 * **Evaluation components (parameterised here):**
 * - **Material + Piece-Square Tables** — separate MG/EG tables per piece;
 *   king has its own opening and endgame tables.
 * - **Pawn structure** — isolated / doubled / connected / backward / passed.
 * - **Piece activity** — mobility, bishop pair, rook/queen on open files,
 *   rook on the 7th, outposts.
 * - **Threats** — bonus per enemy piece attacked by a cheaper attacker.
 * - **King safety** — king-ring attacker pressure + open-file shelter (MG-only).
 *
 * **Tuning:** values marked `EVAL_PARAM` are exposed to the Texel tuner
 * (`tools/texel/`) and fitted on the 725k Zurichess quiet-labeled corpus;
 * `inline constexpr` values are fixed structure, not tuned.
 *
 * @author MTDuke71
 * @version 1.2
 * @see evaluate() in search.cpp for the evaluation entry point.
 * @see docs/EVALUATION_DESIGN.md for detailed algorithm documentation.
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

/**
 * @brief Outpost bonuses (#9 round 8): a knight/bishop on an advanced square,
 *        supported by an own pawn, that enemy pawns on adjacent files can no
 *        longer challenge by advancing. Tapered MG/EG, colour-symmetric,
 *        Texel-tunable. Kept on a sign-split (Intel +9 / AMD −6.25).
 */
EVAL_PARAM int KNIGHT_OUTPOST_BONUS_MG = 36;  ///< Knight outpost, midgame.
EVAL_PARAM int KNIGHT_OUTPOST_BONUS_EG = 15;  ///< Knight outpost, endgame.
EVAL_PARAM int BISHOP_OUTPOST_BONUS_MG = 38;  ///< Bishop outpost, midgame.
EVAL_PARAM int BISHOP_OUTPOST_BONUS_EG = 3;   ///< Bishop outpost, endgame.

/** @brief Bonus for having both bishops (bishop pair advantage) */
EVAL_PARAM int BISHOP_PAIR_BONUS = 46;   // #9 round 3, Texel-tunable

/** @brief Tempo bonus — small advantage for the side to move (initiative). */
EVAL_PARAM int TEMPO_BONUS = 26;

// ============================================================================
// OPEN FILE BONUSES - VICE Part 81 implementation
// ============================================================================

/** @brief Rook bonus for controlling an open file (no pawns of either color) */
EVAL_PARAM int ROOK_OPEN_FILE_BONUS = 18;      

/** @brief Rook bonus for controlling a semi-open file (no own pawns, enemy pawns present) */
EVAL_PARAM int ROOK_SEMI_OPEN_FILE_BONUS = 17;  

/** @brief Queen bonus for controlling an open file */
EVAL_PARAM int QUEEN_OPEN_FILE_BONUS = 7;
EVAL_PARAM int QUEEN_SEMI_OPEN_FILE_BONUS = 8; // VICE value: QueenSemiOpenFile = 3

EVAL_PARAM int ISOLATED_PAWN_PENALTY = 10;  ///< Per pawn with no own pawn on an adjacent file (#9 r3).
EVAL_PARAM int DOUBLED_PAWN_PENALTY = 9;   ///< Per extra own pawn on the same file.

/**
 * @brief Connected-pawn bonus (#9 round 4): per pawn that is phalanx (own pawn
 *        on an adjacent file, same rank) or supported (defended by an own
 *        pawn), indexed by relative rank (White POV; Black mirrors). Ranks 1/8
 *        can't hold pawns so those entries stay 0. Tapered, Texel-tunable.
 */
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_MG = { 0, 3, 9, 16, 20, 44, 66, 0 }; ///< Midgame, by rank.
EVAL_PARAM std::array<int, 8> CONNECTED_PAWN_BONUS_EG = { 0, -6, 2, 3, 11, 32, 24, 0 }; ///< Endgame, by rank.

/**
 * @brief Backward-pawn penalty (#9 round 4): a pawn with no own pawn on an
 *        adjacent file at or behind its rank (so it can never be supported)
 *        whose stop square an enemy pawn controls. Isolated pawns are excluded
 *        (they already pay ISOLATED_PAWN_PENALTY). Tapered, Texel-tunable.
 */
EVAL_PARAM int BACKWARD_PAWN_PENALTY_MG = 12;  ///< Backward pawn, midgame.
EVAL_PARAM int BACKWARD_PAWN_PENALTY_EG = 10;  ///< Backward pawn, endgame.

/**
 * @brief Rook-on-the-relative-7th bonus (#9 round 5): per rook on the enemy's
 *        2nd rank, gated on a target (enemy king confined to its back rank OR
 *        enemy pawns stuck on that rank) so a pointless rook-on-7th in a bare
 *        endgame isn't rewarded. Tapered (stronger EG), symmetric, tunable.
 */
EVAL_PARAM int ROOK_ON_7TH_MG = 20;  ///< Rook on relative 7th, midgame.
EVAL_PARAM int ROOK_ON_7TH_EG = 19;  ///< Rook on relative 7th, endgame.

/**
 * @brief Threat bonuses (#9 round 6): per enemy piece attacked by a cheaper /
 *        more dangerous attacker (the side to move usually wins material or
 *        forces a concession). Named `THREAT_<attacker>_ON_<target>_<phase>`,
 *        tapered MG/EG, colour-symmetric, Texel-tunable. Threats fire less
 *        often in quiet positions, so they are less constrained by the
 *        quiet-labeled corpus — watch the fitted magnitudes for sanity.
 */
EVAL_PARAM int THREAT_PAWN_ON_MINOR_MG  = 71; ///< Pawn attacks minor, MG.
EVAL_PARAM int THREAT_PAWN_ON_MINOR_EG  = 39; ///< Pawn attacks minor, EG.
EVAL_PARAM int THREAT_PAWN_ON_ROOK_MG   = 97; ///< Pawn attacks rook, MG.
EVAL_PARAM int THREAT_PAWN_ON_ROOK_EG   = 9; ///< Pawn attacks rook, EG.
EVAL_PARAM int THREAT_PAWN_ON_QUEEN_MG  = 69; ///< Pawn attacks queen, MG.
EVAL_PARAM int THREAT_PAWN_ON_QUEEN_EG  = 31; ///< Pawn attacks queen, EG.
EVAL_PARAM int THREAT_MINOR_ON_ROOK_MG  = 54; ///< Minor attacks rook, MG.
EVAL_PARAM int THREAT_MINOR_ON_ROOK_EG  = 27; ///< Minor attacks rook, EG.
EVAL_PARAM int THREAT_MINOR_ON_QUEEN_MG = 50; ///< Minor attacks queen, MG.
EVAL_PARAM int THREAT_MINOR_ON_QUEEN_EG = 13; ///< Minor attacks queen, EG.
EVAL_PARAM int THREAT_ROOK_ON_QUEEN_MG  = 80; ///< Rook attacks queen, MG.
EVAL_PARAM int THREAT_ROOK_ON_QUEEN_EG  = 21; ///< Rook attacks queen, EG.

EVAL_PARAM int MOBILITY_WEIGHT_DEFAULT = 5;   ///< Flat mobility, MG (#9 r2). Used when ENABLE_SAFE_MOBILITY is OFF.
EVAL_PARAM int MOBILITY_WEIGHT_ENDGAME = 2;   ///< Flat mobility, EG. Used when ENABLE_SAFE_MOBILITY is OFF.

/**
 * @brief Safe-mobility per-piece weights (#9 round 9; ENABLE_SAFE_MOBILITY).
 *        Per square the piece can move to within its "safe area" — squares not
 *        occupied by own pieces and not attacked by an enemy pawn; the queen
 *        additionally excludes squares attacked by an enemy minor (#41
 *        Queen-error cluster). Signed (White − Black), tapered MG/EG,
 *        Texel-tunable. Replaces the flat MOBILITY_WEIGHT_* when the flag is on.
 */
EVAL_PARAM int KNIGHT_MOBILITY_MG = 6;  ///< Knight safe-mobility, MG.
EVAL_PARAM int KNIGHT_MOBILITY_EG = 2;  ///< Knight safe-mobility, EG.
EVAL_PARAM int BISHOP_MOBILITY_MG = 6;  ///< Bishop safe-mobility, MG.
EVAL_PARAM int BISHOP_MOBILITY_EG = 3;  ///< Bishop safe-mobility, EG.
EVAL_PARAM int ROOK_MOBILITY_MG   = 6;  ///< Rook safe-mobility, MG.
EVAL_PARAM int ROOK_MOBILITY_EG   = 1;  ///< Rook safe-mobility, EG.
EVAL_PARAM int QUEEN_MOBILITY_MG  = 3;  ///< Queen safe-mobility, MG.
EVAL_PARAM int QUEEN_MOBILITY_EG  = 2;  ///< Queen safe-mobility, EG.

inline constexpr int DEVELOP_BONUS_DEFAULT = 40;     ///< Undevelopment penalty (non-opening).
inline constexpr int DEVELOP_BONUS_OPENING = 60;     ///< Opening development bonus.
inline constexpr int DEVELOP_BONUS_ENDGAME = 0;      ///< No development term in the endgame.

/// @brief Material below which the king-activity / material-draw endgame logic
///        kicks in (~R+N+B = 1150; lowered from 1300 so the engine doesn't
///        sacrifice material to force draws). VICE Part 82/83.
inline constexpr int ENDGAME_MATERIAL_THRESHOLD = 1150;

// ============================================================================
// KING SAFETY (#35 Experiment 3)
// MG-only: the tapered eval blends this out toward the endgame, where the king
// should be active (an untapered KS term sank the #2 attempt at -126 Elo).
// Magnitudes are a conservative first cut — Texel-tune later (#9). Replaces the
// four dead constants (KING_SHIELD_MULTIPLIER / KING_ATTACK_PENALTY /
// CASTLE_BONUS / STUCK_PENALTY) that were defined but never referenced.
// ============================================================================

/**
 * @brief King-ring attacker weight per attacking PieceType (None, Pawn,
 *        Knight, Bishop, Rook, Queen, King) — "attack units" summed per
 *        attacked ring square; heavier pieces weigh more.
 *
 * #9 round 7 reformulation: the prior design gated on ≥2 distinct attackers,
 * which made the term zero on most quiet positions so the tuner couldn't
 * constrain it (hand-tuning stalled at ~0 Elo, #35 Exp 3). The gate was removed
 * (danger fires on ≥1 attacker, like the MTLChess recipe that scored +116), and
 * these weights + the shelter penalty are `EVAL_PARAM` so the harness fits them.
 * Seeded at the MTLChess values (N/B=2, R=3, Q=5).
 */
EVAL_PARAM std::array<int, size_t(PieceType::_Count)> KS_ATTACK_WEIGHT = { 0, 0, 3, 4, 3, 5, 0 };

/// @brief Non-linear danger divisor: `danger = min(units²/DIVISOR, CAP)`, applied
///        whenever any enemy piece attacks the king ring. Fixed structure (only
///        weight²/DIVISOR matters and the weights are tunable). DIVISOR=4 matches MTLChess.
inline constexpr int KS_ATTACK_DIVISOR = 4;
/// @brief Upper clamp on the per-king danger score (centipawns).
inline constexpr int KS_ATTACK_CAP     = 500;

/// @brief Shelter penalty per open file on or adjacent to the king's file (no own
///        pawn anywhere on it). Fires often → aids tunability. Texel-tunable.
EVAL_PARAM int KS_OPEN_FILE_PENALTY = 23;

// Midgame piece-square tables (White POV; Black mirrors via square flip),
// Texel-tuned on the 725k Zurichess quiet-labeled corpus (#9). Indexed by sq64
// but laid out rank1 (top) -> rank8 (bottom) for readability.
/// @brief Midgame pawn PST.
EVAL_PARAM std::array<int, 64> PAWN_TABLE = {
       0,   0,   0,   0,   0,   0,   0,   0,
     -11,   5,  -2,   1,   2,  38,  44,  -3,
     -11,  -8,   5,  -3,  11,   8,  28,  -4,
     -12, -13,   6,  15,  20,  12,   0, -17,
      -9,  10,   8,  20,  22,  20,  12, -13,
      -5, -22,   3, -12,  41,  81,  25, -15,
      30,  46,   2,  62,  48,  77, -81,-116,
       0,   0,   0,   0,   0,   0,   0,   0,};;

/// @brief Midgame knight PST.
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE = {
    -108,  -9, -39, -16,   8,  -8,  -5, -11,
     -22, -31,  -1,  17,  18,  26,  -2,   1,
     -19,  -1,  19,  21,  34,  28,  34,  -8,
      -6,  12,  20,  21,  38,  32,  37,   5,
      -9,  22,   5,  51,  32,  71,  25,  29,
     -51,  30,  10,  45,  72, 116,  60,  40,
     -93, -64,  47,  11,   9,  38,  -3, -27,
    -200, -99, -55, -43,  25,-124, -42,-116,};;

/// @brief Midgame bishop PST.
EVAL_PARAM std::array<int, 64> BISHOP_TABLE = {
     -16,  15,   5,   7,   8,   0, -23, -12,
      18,  33,  25,  15,  24,  25,  51,  16,
       5,  26,  26,  21,  22,  43,  24,  18,
       3,  11,  18,  35,  42,   6,  13,  21,
      -4,  16,   4,  41,  26,  34,  14,   7,
      -2,  26,  19,  18,  30,  55,  32,  16,
     -36, -22, -41, -49,  -4,  56, -10, -57,
     -37, -23,-120, -90, -50, -56, -19,   0,};;

/// @brief Midgame rook PST.
EVAL_PARAM std::array<int, 64> ROOK_TABLE = {
      -1,   4,  19,  23,  26,  22, -14,   2,
     -28,  -1,  -5,   9,  16,  17,  10, -50,
     -29, -11,   1,   0,   6,  11,  10, -16,
     -32, -14,  -5,   9,  17,   1,  27, -22,
     -20, -10,   9,  31,  23,  40,   7, -12,
      -1,  24,  26,  35,  13,  39,  68,  13,
     -12,   1,  22,  16,  55,  45,   2,   2,
      26,  26,  24,  34,  45,   2,  22,  19,};;

/// @brief Midgame queen PST.
EVAL_PARAM std::array<int, 64> QUEEN_TABLE = {
      -1,   6,   7,  18,   0, -14, -16, -50,
     -27,  -3,  14,  14,  22,  21,   0,  13,
     -18,   7,  -6,   1,   0,   5,  11,   3,
      -4, -35,  -3, -10,   3,   1,   1,  -6,
     -35, -19, -28, -28, -10,  -2,  -5, -10,
     -10, -18,   9, -30,  11,  25,  23,  21,
     -36, -50, -25,   1, -55,  17,   3,  36,
     -34, -13,  -3, -14,  47,  35,  29,  34,};;

/// @brief Opening/middlegame king PST — rewards castling and back-rank safety
///        (VICE Part 82's KingO). Blended out toward the endgame by the taper.
EVAL_PARAM std::array<int, 64> KING_TABLE = {
     -37,  30,   8, -86, -17, -55,  22,   4,
     -18,  -6, -32, -92, -68, -50,  -5,  -2,
     -37, -18, -34, -60, -73, -57, -36, -58,
     -90,  -2, -58, -86, -88, -66, -66,-111,
     -45, -10, -22, -61, -65, -31,  14, -85,
      35,  31,  38,   4,  10,  77, 120, -19,
      57,  53,  22,  80,  11,  27, -13, -77,
     -54,  76,  92,  21, -38, -20,  13,   5,};;

/// @brief Endgame king PST — rewards centralization (VICE Part 82's KingE).
EVAL_PARAM std::array<int, 64> KING_TABLE_ENDGAME = {
     -64, -41, -16,  11, -15,   6, -35, -63,
     -26,  -2,  24,  40,  38,  28,   4, -23,
     -17,  11,  31,  44,  50,  38,  23,  -2,
      -6,   9,  46,  54,  57,  47,  30,   7,
      -2,  35,  42,  52,  51,  52,  37,  17,
       5,  29,  31,  25,  27,  50,  39,  16,
     -22,  18,  20,  13,  28,  47,  37,  21,
     -90, -48, -32, -23,  -2,  20,   6, -25,};;

/**
 * @brief Endgame piece-square tables for the non-king pieces (#9 round 2 —
 *        tapered PSTs). Originally seeded as copies of the MG tables (so the
 *        eval was byte-identical to baseline-t11 until the tuner differentiated
 *        them); feed the EG accumulator in search.cpp. The king already has its
 *        own MG/EG tables above.
 */
/// @brief Endgame pawn PST.
EVAL_PARAM std::array<int, 64> PAWN_TABLE_EG   = {
       0,   0,   0,   0,   0,   0,   0,   0,
      20,  12,  18,   9,  17,   6,  -2,  -1,
      10,  10,   0,   6,   6,   0,  -9,  -4,
      20,  14,   1,  -6,  -5,  -5,  -1,   4,
      34,  17,   7, -11,  -7,  -4,   8,  16,
      72,  70,  41,   9, -13,   1,  38,  54,
     162, 145, 118,  79,  91,  83, 144, 184,
       0,   0,   0,   0,   0,   0,   0,   0,};;
/// @brief Endgame knight PST.
EVAL_PARAM std::array<int, 64> KNIGHT_TABLE_EG = {
      13, -25,   4,  14,   1,   6, -28, -42,
      -7,   3,  17,  16,  17,   5,   6, -27,
       5,  21,  15,  35,  30,  15,   3,   6,
       6,  18,  35,  42,  35,  36,  30,   6,
      10,  21,  42,  37,  37,  23,  23,   4,
       0,   2,  30,  20,   7,  -2,  -8, -25,
       5,  23,  -6,  20,  11, -11,  -1, -30,
     -24, -16,  11, -11,  -7,  -8, -46, -75,};;
/// @brief Endgame bishop PST.
EVAL_PARAM std::array<int, 64> BISHOP_TABLE_EG = {
       0,  16,   2,  13,  12,  13,  19,   8,
       5,  -2,  10,  14,  16,  14,  -3, -13,
       9,  15,  24,  24,  31,  11,  14,   4,
      10,  12,  24,  23,  14,  24,  11,   8,
      15,  16,  21,  15,  18,  14,  14,  19,
      16,   6,  11,   6,   2,   3,  12,  12,
      16,  18,  25,  10,  12,  -1,  10,   9,
       0,  -2,  16,  16,  15,   9,   1, -11,};;
/// @brief Endgame rook PST.
EVAL_PARAM std::array<int, 64> ROOK_TABLE_EG   = {
       7,  10,   8,   7,   1,   3,  10, -13,
      13,   5,  11,  10,   1,   3,  -2,  15,
      16,  17,   8,   8,   3,   3,   5,   0,
      28,  26,  25,  14,   9,  14,   5,  17,
      27,  24,  29,  10,  15,  18,  16,  27,
      25,  20,  16,  15,  15,  10,   8,  13,
      19,  14,   9,   9, -12,   6,  19,  17,
      21,  19,  20,  16,  16,  25,  19,  20,};;
/// @brief Endgame queen PST.
EVAL_PARAM std::array<int, 64> QUEEN_TABLE_EG  = {
     -44, -61, -41, -68, -24, -40, -37, -54,
     -24, -36, -43, -40, -38, -31, -49, -34,
     -12, -44,   2,  -9,   2,   8,   8,   7,
     -20,  38,   5,  45,  18,  22,  33,  20,
      15,  17,  29,  49,  50,  36,  59,  36,
     -23,   0, -11,  60,  46,  29,  24,   5,
     -17,  13,  27,  23,  56,  24,  23, -11,
     -26,  13,  16,  12,  -2,   0, -11,   4,};;

/// @brief Passed-pawn bonus indexed by the pawn's relative rank (White POV;
///        Black mirrors). Steeply increasing toward promotion.
EVAL_PARAM std::array<int, 8> PASSED_PAWN_BONUS = { 0, 2, 5, 21, 44, 89, 101, 200 };

// Pawn-structure bitboard masks (VICE Part 77) — used to detect passed,
// isolated, and doubled pawns. All are `constexpr` (compile-time fixed).

/// @brief Per-file mask: every square on file 0 (A) … 7 (H).
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

/// @brief Per-rank mask: every square on rank 1 (index 0) … rank 8 (index 7).
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

/// @brief Per-file mask of the adjacent files — a pawn is isolated when its
///        side has no pawn on these files. Indexed by the pawn's file (0–7).
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

/// @brief Per-square mask of the squares that must be empty of enemy pawns for
///        a White pawn on that square to be passed (own + adjacent files, ahead).
///        Built at startup by init_evaluation_masks().
extern std::array<uint64_t, 64> WHITE_PASSED_PAWN_MASKS;

/// @brief Black counterpart of WHITE_PASSED_PAWN_MASKS (squares ahead toward rank 1).
extern std::array<uint64_t, 64> BLACK_PASSED_PAWN_MASKS;

/**
 * @brief Populate the runtime passed-pawn masks (VICE Part 78).
 *
 * Fills WHITE_/BLACK_PASSED_PAWN_MASKS. Call once at startup before any
 * evaluation; the constexpr file/rank/isolated masks need no initialization.
 */
void init_evaluation_masks();

} // namespace EvalParams

/**
 * @enum Huginn::GamePhase
 * @brief Coarse 3-bucket game phase (legacy — see the @file note).
 *
 * Superseded for scoring by the smooth `game_phase_256()` taper; retained for
 * the few call sites that still want a discrete opening/middlegame/endgame tag.
 */
enum class GamePhase {
    Opening,     ///< High material — development-focused.
    Middlegame,  ///< Medium material — tactical/positional emphasis.
    Endgame      ///< Low material — king activity and promotion focus.
};
// HybridEvaluator removed: keep EvalParams and GamePhase for Engine

} // namespace Huginn
