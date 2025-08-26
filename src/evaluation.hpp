#pragma once

#include "position.hpp"
#include "chess_types.hpp"

/**
 * Huginn Chess Engine - Position Evaluation System
 * 
 * Provides comprehensive position evaluation for search algorithms.
 * Returns scores from the perspective of the side to move.
 */

namespace Evaluation {

    // Material values in centipawns (standard chess engine values)
    constexpr int PAWN_VALUE   = 100;
    constexpr int KNIGHT_VALUE = 320;
    constexpr int BISHOP_VALUE = 330;
    constexpr int ROOK_VALUE   = 500;
    constexpr int QUEEN_VALUE  = 900;
    constexpr int KING_VALUE   = 20000;  // Large value to prevent king captures

    // Special evaluation scores
    constexpr int CHECKMATE_SCORE = 32000;   // Mate score
    constexpr int STALEMATE_SCORE = 0;       // Draw score
    constexpr int DRAW_SCORE      = 0;       // Draw by repetition/50-move rule

    // Piece-square tables for positional evaluation
    namespace PieceSquareTables {
        
        // Pawn piece-square table (encourages central advancement)
        extern const int PAWN_PST[64];
        
        // Knight piece-square table (encourages central placement)
        extern const int KNIGHT_PST[64];
        
        // Bishop piece-square table (encourages long diagonals)
        extern const int BISHOP_PST[64];
        
        // Rook piece-square table (encourages open files and back rank)
        extern const int ROOK_PST[64];
        
        // Queen piece-square table (encourages central activity)
        extern const int QUEEN_PST[64];
        
        // King piece-square table for middlegame (encourages safety)
        extern const int KING_MG_PST[64];
        
        // King piece-square table for endgame (encourages activity)
        extern const int KING_EG_PST[64];
    }

    /**
     * Basic material evaluation
     * @param pos Position to evaluate
     * @return Material score from perspective of side to move
     */
    int evaluate_material(const Position& pos);

    /**
     * Positional evaluation using piece-square tables
     * @param pos Position to evaluate
     * @return Positional score from perspective of side to move
     */
    int evaluate_positional(const Position& pos);

    /**
     * King safety evaluation
     * @param pos Position to evaluate
     * @param color Color of king to evaluate
     * @return King safety score (positive = safe, negative = unsafe)
     */
    int evaluate_king_safety(const Position& pos, Color color);

    /**
     * Pawn structure evaluation
     * @param pos Position to evaluate
     * @return Pawn structure score from perspective of side to move
     */
    int evaluate_pawn_structure(const Position& pos);

    /**
     * Development and early game evaluation
     * @param pos Position to evaluate
     * @return Development score from perspective of side to move
     */
    int evaluate_development(const Position& pos);

    /**
     * Hanging pieces evaluation - detects undefended pieces under attack
     * @param pos Position to evaluate
     * @return Penalty for hanging pieces from perspective of side to move
     */
    int evaluate_hanging_pieces(const Position& pos);

    /**
     * Comprehensive position evaluation
     * @param pos Position to evaluate
     * @return Total evaluation score from perspective of side to move
     */
    int evaluate_position(const Position& pos);

    /**
     * Quick material-only evaluation for quiescence search
     * @param pos Position to evaluate
     * @return Material balance from perspective of side to move
     */
    int evaluate_material_quick(const Position& pos);

    /**
     * Check if position is checkmate
     * @param pos Position to check
     * @return True if current side to move is checkmated
     */
    bool is_checkmate(const Position& pos);

    /**
     * Check if position is stalemate
     * @param pos Position to check
     * @return True if current side to move is stalemated
     */
    bool is_stalemate(const Position& pos);

    /**
     * Check for insufficient material draw
     * @param pos Position to check
     * @return True if position has insufficient material for checkmate
     */
    bool is_insufficient_material(const Position& pos);

    /**
     * Detect endgame phase based on material
     * @param pos Position to evaluate
     * @return True if position is in endgame phase
     */
    bool is_endgame(const Position& pos);

    /**
     * Comprehensive pawn structure evaluation using bitboards
     * @param pos Position to evaluate
     * @return Pawn structure score from perspective of side to move
     */
    int evaluate_pawn_structure(const Position& pos);

    /**
     * Analyze and print evaluation for all legal moves from starting position
     * @param depth Search depth for analysis (default 8)
     */
    void analyze_opening_moves(int depth = 8);

} // namespace Evaluation
