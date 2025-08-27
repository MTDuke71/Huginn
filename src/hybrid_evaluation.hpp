#pragma once

#include "../src/position.hpp"
#include "../src/chess_types.hpp"
#include <array>

// Castling rights constants
#define CASTLE_WK 1   // White kingside
#define CASTLE_WQ 2   // White queenside  
#define CASTLE_BK 4   // Black kingside
#define CASTLE_BQ 8   // Black queenside

namespace Engine3 {

namespace EvalParams {

inline constexpr int GAME_PHASE_OPENING_THRESHOLD = 24;
inline constexpr int GAME_PHASE_MIDDLEGAME_THRESHOLD = 12;

inline constexpr int PAWN_VALUE = 100;
inline constexpr int KNIGHT_VALUE = 300;
inline constexpr int BISHOP_VALUE = 350;
inline constexpr int ROOK_VALUE = 500;
inline constexpr int QUEEN_VALUE = 1000;
inline constexpr int KING_VALUE = 10000;

inline constexpr int WHITE_KNIGHT_OUTPOST_MIN_RANK = 3;
inline constexpr int BLACK_KNIGHT_OUTPOST_MAX_RANK = 4;
inline constexpr int KNIGHT_OUTPOST_BONUS = 25;

inline constexpr int BISHOP_PAIR_BONUS = 50;
inline constexpr int ROOK_OPEN_FILE_BONUS = 15;

inline constexpr int ISOLATED_PAWN_PENALTY = 15;
inline constexpr int DOUBLED_PAWN_PENALTY = 10;

inline constexpr int MOBILITY_WEIGHT_DEFAULT = 5;
inline constexpr int MOBILITY_WEIGHT_ENDGAME = 2;

inline constexpr int DEVELOP_BONUS_DEFAULT = 15;
inline constexpr int DEVELOP_BONUS_OPENING = 20;
inline constexpr int DEVELOP_BONUS_ENDGAME = 0;

inline constexpr int KING_SHIELD_MULTIPLIER = 10;
inline constexpr int KING_ATTACK_PENALTY = 15;

inline constexpr int CASTLE_BONUS = 40;
inline constexpr int STUCK_PENALTY = 20;

inline constexpr std::array<int, 64> PAWN_TABLE = {
    0,  0,  0,   0,  0,  0,   0,  0,  5,  10, 10, -20, -20, 10, 10, 5,
    5,  -5, -10, 0,  0,  -10, -5, 5,  0,  0,  0,  20,  20,  0,  0,  0,
    5,  5,  10,  25, 25, 10,  5,  5,  10, 10, 20, 30,  30,  20, 10, 10,
    50, 50, 50,  50, 50, 50,  50, 50, 0,  0,  0,  0,   0,   0,  0,  0};

inline constexpr std::array<int, 64> KNIGHT_TABLE = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};

inline constexpr std::array<int, 64> BISHOP_TABLE = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};

inline constexpr std::array<int, 64> ROOK_TABLE = {
    0,  0,  0,  5,  5,  0,  0,  0,  -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
    5,  10, 10, 10, 10, 10, 10, 5,  0,  0, 0, 0, 0, 0, 0, 0};

inline constexpr std::array<int, 64> QUEEN_TABLE = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};

inline constexpr std::array<int, 64> KING_TABLE = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};

inline constexpr std::array<int, 64> KING_TABLE_ENDGAME = {
    -50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0,   0,
    -10, -20, -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10,
    30,  40,  40,  30,  -10, -30, -30, -10, 30,  40,  40,  30,  -10,
    -30, -30, -10, 20,  30,  30,  20,  -10, -30, -30, -30, 0,   0,
    0,   0,   -30, -30, -50, -30, -30, -30, -30, -30, -30, -50};

inline constexpr std::array<int, 8> PASSED_PAWN_BONUS = {0,  5,  10,  20,
                                                         35, 60, 100, 0};

} // namespace EvalParams

enum class GamePhase {
    Opening,
    Middlegame,
    Endgame
};

class HybridEvaluator {
public:
    // Main evaluation function
    static int evaluate(const Position& pos);
    
    // Helper functions
    static GamePhase get_game_phase(const Position& pos);
    static int mirror_square(int sq64);
    static int square120_to_64(int sq120);
    static int square64_to_120(int sq64);
    
private:
    // Evaluation components
    static int evaluate_material(const Position& pos, GamePhase phase);
    static int evaluate_piece_square_tables(const Position& pos, GamePhase phase);
    static int evaluate_pawn_structure(const Position& pos);
    static int evaluate_piece_activity(const Position& pos, GamePhase phase);
    static int evaluate_king_safety(const Position& pos, GamePhase phase);
    static int evaluate_development(const Position& pos, GamePhase phase);
    static int evaluate_mobility(const Position& pos, GamePhase phase);
    
    // Helper functions for specific evaluations
    static bool is_passed_pawn(const Position& pos, int sq120, Color color);
    static bool is_isolated_pawn(const Position& pos, int sq120, Color color);
    static bool is_doubled_pawn(const Position& pos, int sq120, Color color);
    static bool is_knight_outpost(const Position& pos, int sq120, Color color);
    static bool is_open_file(const Position& pos, int file);
    static int count_king_attackers(const Position& pos, int king_sq, Color attacking_color);
    static int count_developed_pieces(const Position& pos, Color color);
};

} // namespace Engine3
