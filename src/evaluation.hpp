#pragma once

#include "../src/position.hpp"
#include "../src/chess_types.hpp"
#include <array>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

// Castling rights constants
#define CASTLE_WK 1   // White kingside
#define CASTLE_WQ 2   // White queenside  
#define CASTLE_BK 4   // Black kingside
#define CASTLE_BQ 8   // Black queenside

namespace Huginn {

namespace EvalParams {

inline constexpr int GAME_PHASE_OPENING_THRESHOLD = 28;  // Stay in opening longer
inline constexpr int GAME_PHASE_MIDDLEGAME_THRESHOLD = 16;  // More conservative transition

inline constexpr int PAWN_VALUE = 100;
inline constexpr int KNIGHT_VALUE = 325;
inline constexpr int BISHOP_VALUE = 325;
inline constexpr int ROOK_VALUE = 550;
inline constexpr int QUEEN_VALUE = 1000;
inline constexpr int KING_VALUE = 50000;

inline constexpr int WHITE_KNIGHT_OUTPOST_MIN_RANK = 3;
inline constexpr int BLACK_KNIGHT_OUTPOST_MAX_RANK = 4;
inline constexpr int KNIGHT_OUTPOST_BONUS = 25;

inline constexpr int BISHOP_PAIR_BONUS = 50;
// VICE Part 81: Open and semi-open file bonuses for rooks and queens  
inline constexpr int ROOK_OPEN_FILE_BONUS = 10;      // VICE value: RookOpenFile = 10
inline constexpr int ROOK_SEMI_OPEN_FILE_BONUS = 5;  // VICE value: RookSemiOpenFile = 5
inline constexpr int QUEEN_OPEN_FILE_BONUS = 5;      // VICE value: QueenOpenFile = 5
inline constexpr int QUEEN_SEMI_OPEN_FILE_BONUS = 3; // VICE value: QueenSemiOpenFile = 3

inline constexpr int ISOLATED_PAWN_PENALTY = 10;  // VICE Part 80: Matches original VICE value
inline constexpr int DOUBLED_PAWN_PENALTY = 20;    // Increased penalty

inline constexpr int MOBILITY_WEIGHT_DEFAULT = 5;
inline constexpr int MOBILITY_WEIGHT_ENDGAME = 2;

inline constexpr int DEVELOP_BONUS_DEFAULT = 40;     // Much higher penalty for undevelopment
inline constexpr int DEVELOP_BONUS_OPENING = 60;     // Strong opening development bonus
inline constexpr int DEVELOP_BONUS_ENDGAME = 0;

// VICE Part 82: Material draw detection and king evaluation 
// Endgame threshold: approximately equivalent to Rook + 2 Knights + 2 Pawns (about 1300)
inline constexpr int ENDGAME_MATERIAL_THRESHOLD = 1300;

inline constexpr int KING_SHIELD_MULTIPLIER = 10;
inline constexpr int KING_ATTACK_PENALTY = 15;

inline constexpr int CASTLE_BONUS = 40;
inline constexpr int STUCK_PENALTY = 20;

inline constexpr std::array<int, 64> PAWN_TABLE = {
    0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 0,  -10, -10, 0,  10, 10,
    5,  0,  0,  5,  5,  0,  0,  5,
    0,  0,  10, 20, 20, 10, 0,  0,
    5,  5,  5,  10, 10, 5,  5,  5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
    0,  0,  0,  0,  0,  0,  0,  0};

inline constexpr std::array<int, 64> KNIGHT_TABLE = {
    0,  -10, 0,  0,  0,  0,  -10, 0,
    0,  0,   0,  5,  5,  0,  0,   0,
    0,  0,   10, 10, 10, 10, 0,   0,
    0,  0,   10, 20, 20, 10, 5,   0,
    5,  10,  15, 20, 20, 15, 10,  5,
    5,  10,  10, 20, 20, 10, 10,  5,
    0,  0,   5,  10, 10, 5,  0,   0,
    0,  0,   0,  0,  0,  0,  0,   0};

inline constexpr std::array<int, 64> BISHOP_TABLE = {
    0,  0,  -10, 0,  0,  -10, 0,  0,
    0,  0,  0,   10, 10, 0,   0,  0,
    0,  0,  10,  15, 15, 10,  0,  0,
    0,  10, 15,  20, 20, 15,  10, 0,
    0,  10, 15,  20, 20, 15,  10, 0,
    0,  0,  10,  15, 15, 10,  0,  0,
    0,  0,  0,   10, 10, 0,   0,  0,
    0,  0,  0,   0,  0,  0,   0,  0};

inline constexpr std::array<int, 64> ROOK_TABLE = {
    0,  0,  5,  10, 10, 5,  0,  0,
    0,  0,  5,  10, 10, 5,  0,  0,
    0,  0,  5,  10, 10, 5,  0,  0,
    0,  0,  5,  10, 10, 5,  0,  0,
    0,  0,  5,  10, 10, 5,  0,  0,
    0,  0,  5,  10, 10, 5,  0,  0,
    25, 25, 25, 25, 25, 25, 25, 25,
    0,  0,  5,  10, 10, 5,  0,  0};

inline constexpr std::array<int, 64> QUEEN_TABLE = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};

// VICE Part 82: King position evaluation tables
// KingO[64] - Opening/middlegame king table (encourages castling and back rank safety)
inline constexpr std::array<int, 64> KING_TABLE = {
  -5,  -10, -10, -10, -10, -10, -10, -5,
    -10, -10, -10, -10, -10, -10, -10, -10,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -10, -10, -10, -10, -10, -10, -10, -10,
  -5,  -10, -10, -10, -10, -10, -10, -5};

// KingE[64] - Endgame king table (encourages centralization)
inline constexpr std::array<int, 64> KING_TABLE_ENDGAME = {
    -50, 0,   0,   0,   0,   0,   0,   -50,
    0,   20,  20,  20,  20,  20,  20,  0,
    0,   20,  40,  40,  40,  40,  20,  0,
    0,   20,  40,  50,  50,  40,  20,  0,
    0,   20,  40,  50,  50,  40,  20,  0,
    0,   20,  40,  40,  40,  40,  20,  0,
    0, 20,   20,  20,  20,  20,  20,   0,
    -50, 0, 0,   0,   0,   0,   0, -50};

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
// HybridEvaluator removed: keep EvalParams and GamePhase for MinimalEngine

} // namespace Huginn
