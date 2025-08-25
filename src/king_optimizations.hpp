#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

/**
 * Huginn Chess Engine - King Move Generation Optimizations
 * =========================================================
 * 
 * This module implements optimized king move generation by separating
 * ordinary king moves from castling logic, based on profiling results
 * showing king moves consume 19.1% of move generation time.
 * 
 * Optimization Strategy:
 * 1. Pre-check castling viability to avoid expensive SqAttacked calls
 * 2. Separate ordinary king moves from castling moves
 * 3. Use early returns for impossible castling scenarios
 * 4. Cache-friendly data structures for quick castling validation
 */

namespace KingOptimizations {

    // Pre-computed castling squares for quick validation
    struct CastlingSquares {
        // White castling squares
        static constexpr int WHITE_KING_START = sq(File::E, Rank::R1);
        static constexpr int WHITE_KINGSIDE_ROOK = sq(File::H, Rank::R1);
        static constexpr int WHITE_QUEENSIDE_ROOK = sq(File::A, Rank::R1);
        static constexpr int WHITE_KINGSIDE_KING_TO = sq(File::G, Rank::R1);
        static constexpr int WHITE_QUEENSIDE_KING_TO = sq(File::C, Rank::R1);
        
        // Black castling squares
        static constexpr int BLACK_KING_START = sq(File::E, Rank::R8);
        static constexpr int BLACK_KINGSIDE_ROOK = sq(File::H, Rank::R8);
        static constexpr int BLACK_QUEENSIDE_ROOK = sq(File::A, Rank::R8);
        static constexpr int BLACK_KINGSIDE_KING_TO = sq(File::G, Rank::R8);
        static constexpr int BLACK_QUEENSIDE_KING_TO = sq(File::C, Rank::R8);
        
        // Path squares for quick empty check
        static constexpr std::array<int, 2> WHITE_KINGSIDE_PATH = {
            sq(File::F, Rank::R1), sq(File::G, Rank::R1)
        };
        static constexpr std::array<int, 3> WHITE_QUEENSIDE_PATH = {
            sq(File::D, Rank::R1), sq(File::C, Rank::R1), sq(File::B, Rank::R1)
        };
        static constexpr std::array<int, 2> BLACK_KINGSIDE_PATH = {
            sq(File::F, Rank::R8), sq(File::G, Rank::R8)
        };
        static constexpr std::array<int, 3> BLACK_QUEENSIDE_PATH = {
            sq(File::D, Rank::R8), sq(File::C, Rank::R8), sq(File::B, Rank::R8)
        };
        
        // Attack checking squares (king must not be attacked during castling)
        static constexpr std::array<int, 3> WHITE_KINGSIDE_ATTACK_CHECK = {
            sq(File::E, Rank::R1), sq(File::F, Rank::R1), sq(File::G, Rank::R1)
        };
        static constexpr std::array<int, 3> WHITE_QUEENSIDE_ATTACK_CHECK = {
            sq(File::E, Rank::R1), sq(File::D, Rank::R1), sq(File::C, Rank::R1)
        };
        static constexpr std::array<int, 3> BLACK_KINGSIDE_ATTACK_CHECK = {
            sq(File::E, Rank::R8), sq(File::F, Rank::R8), sq(File::G, Rank::R8)
        };
        static constexpr std::array<int, 3> BLACK_QUEENSIDE_ATTACK_CHECK = {
            sq(File::E, Rank::R8), sq(File::D, Rank::R8), sq(File::C, Rank::R8)
        };
    };

    /**
     * Fast check if castling is theoretically possible
     * Returns false immediately if basic conditions aren't met
     */
    inline bool can_castle_theoretically(const Position& pos, Color us, bool kingside) {
        if (us == Color::White) {
            if (pos.king_sq[int(us)] != CastlingSquares::WHITE_KING_START) return false;
            
            if (kingside) {
                return (pos.castling_rights & CASTLE_WK) && 
                       pos.at(CastlingSquares::WHITE_KINGSIDE_ROOK) == Piece::WhiteRook;
            } else {
                return (pos.castling_rights & CASTLE_WQ) && 
                       pos.at(CastlingSquares::WHITE_QUEENSIDE_ROOK) == Piece::WhiteRook;
            }
        } else {
            if (pos.king_sq[int(us)] != CastlingSquares::BLACK_KING_START) return false;
            
            if (kingside) {
                return (pos.castling_rights & CASTLE_BK) && 
                       pos.at(CastlingSquares::BLACK_KINGSIDE_ROOK) == Piece::BlackRook;
            } else {
                return (pos.castling_rights & CASTLE_BQ) && 
                       pos.at(CastlingSquares::BLACK_QUEENSIDE_ROOK) == Piece::BlackRook;
            }
        }
    }

    /**
     * Check if castling path is clear (no pieces between king and rook)
     */
    template<size_t N>
    inline bool is_path_clear(const Position& pos, const std::array<int, N>& path) {
        for (int sq : path) {
            if (pos.at(sq) != Piece::None) return false;
        }
        return true;
    }

    /**
     * Check if castling path is safe (no squares under attack)
     */
    template<size_t N>
    inline bool is_path_safe(const Position& pos, const std::array<int, N>& squares, Color enemy) {
        for (int sq : squares) {
            if (SqAttacked(sq, pos, enemy)) return false;
        }
        return true;
    }

    /**
     * Generate ordinary king moves only (no castling)
     * This is much faster when castling isn't possible
     */
    inline void generate_ordinary_king_moves(const Position& pos, S_MOVELIST& list, Color us) {
        int king_sq = pos.king_sq[int(us)];
        if (king_sq == -1) return;  // Safety check
        
        // Regular king moves using pre-computed deltas
        for (int delta : KING_DELTAS) {
            int to = king_sq + delta;
            if (IS_PLAYABLE(to)) {
                Piece target = pos.at(to);
                
                if (target == Piece::None) {
                    list.add_quiet_move(make_move(king_sq, to));
                } else if (color_of(target) == !us) {
                    list.add_capture_move(make_capture(king_sq, to, type_of(target)), pos);
                }
            }
        }
    }

    /**
     * Generate castling moves only (if possible)
     * Separated from ordinary moves for better performance
     */
    inline void generate_castling_moves(const Position& pos, S_MOVELIST& list, Color us) {
        Color enemy = !us;
        
        if (us == Color::White) {
            // White kingside castling
            if (can_castle_theoretically(pos, us, true) &&
                is_path_clear(pos, CastlingSquares::WHITE_KINGSIDE_PATH) &&
                is_path_safe(pos, CastlingSquares::WHITE_KINGSIDE_ATTACK_CHECK, enemy)) {
                list.add_castle_move(make_castle(CastlingSquares::WHITE_KING_START, 
                                               CastlingSquares::WHITE_KINGSIDE_KING_TO));
            }
            
            // White queenside castling
            if (can_castle_theoretically(pos, us, false) &&
                is_path_clear(pos, CastlingSquares::WHITE_QUEENSIDE_PATH) &&
                is_path_safe(pos, CastlingSquares::WHITE_QUEENSIDE_ATTACK_CHECK, enemy)) {
                list.add_castle_move(make_castle(CastlingSquares::WHITE_KING_START, 
                                               CastlingSquares::WHITE_QUEENSIDE_KING_TO));
            }
        } else {
            // Black kingside castling
            if (can_castle_theoretically(pos, us, true) &&
                is_path_clear(pos, CastlingSquares::BLACK_KINGSIDE_PATH) &&
                is_path_safe(pos, CastlingSquares::BLACK_KINGSIDE_ATTACK_CHECK, enemy)) {
                list.add_castle_move(make_castle(CastlingSquares::BLACK_KING_START, 
                                               CastlingSquares::BLACK_KINGSIDE_KING_TO));
            }
            
            // Black queenside castling
            if (can_castle_theoretically(pos, us, false) &&
                is_path_clear(pos, CastlingSquares::BLACK_QUEENSIDE_PATH) &&
                is_path_safe(pos, CastlingSquares::BLACK_QUEENSIDE_ATTACK_CHECK, enemy)) {
                list.add_castle_move(make_castle(CastlingSquares::BLACK_KING_START, 
                                               CastlingSquares::BLACK_QUEENSIDE_KING_TO));
            }
        }
    }

    /**
     * Optimized king move generation combining ordinary and castling moves
     * Uses early returns and conditional logic to minimize overhead
     */
    inline void generate_king_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        int king_sq = pos.king_sq[int(us)];
        if (king_sq == -1) return;  // Safety check
        
        // Generate ordinary king moves first (this is the most common case)
        for (int delta : KING_DELTAS) {
            int to = king_sq + delta;
            if (IS_PLAYABLE(to)) {
                Piece target = pos.at(to);
                
                if (target == Piece::None) {
                    list.add_quiet_move(make_move(king_sq, to));
                } else if (color_of(target) == !us) {
                    list.add_capture_move(make_capture(king_sq, to, type_of(target)), pos);
                }
            }
        }
        
        // Quick castling check - early return if no castling rights at all
        if (!pos.castling_rights) return;
        
        // Early return if king not on starting square (90%+ of positions)
        bool on_starting_square = (us == Color::White) ? 
            (king_sq == CastlingSquares::WHITE_KING_START) : 
            (king_sq == CastlingSquares::BLACK_KING_START);
        if (!on_starting_square) return;
        
        // Only check our castling rights
        uint8_t our_castle_rights = (us == Color::White) ? 
            (CASTLE_WK | CASTLE_WQ) : (CASTLE_BK | CASTLE_BQ);
        if (!(pos.castling_rights & our_castle_rights)) return;
        
        // Generate castling moves with optimized checks
        Color enemy = !us;
        
        if (us == Color::White) {
            // White kingside - streamlined checks
            if ((pos.castling_rights & CASTLE_WK) && 
                pos.at(CastlingSquares::WHITE_KINGSIDE_ROOK) == Piece::WhiteRook &&
                pos.at(CastlingSquares::WHITE_KINGSIDE_PATH[0]) == Piece::None &&
                pos.at(CastlingSquares::WHITE_KINGSIDE_PATH[1]) == Piece::None) {
                
                // Only check attacks if path is clear
                if (!SqAttacked(CastlingSquares::WHITE_KINGSIDE_ATTACK_CHECK[0], pos, enemy) &&
                    !SqAttacked(CastlingSquares::WHITE_KINGSIDE_ATTACK_CHECK[1], pos, enemy) &&
                    !SqAttacked(CastlingSquares::WHITE_KINGSIDE_ATTACK_CHECK[2], pos, enemy)) {
                    list.add_castle_move(make_castle(CastlingSquares::WHITE_KING_START, 
                                                   CastlingSquares::WHITE_KINGSIDE_KING_TO));
                }
            }
            
            // White queenside - streamlined checks  
            if ((pos.castling_rights & CASTLE_WQ) &&
                pos.at(CastlingSquares::WHITE_QUEENSIDE_ROOK) == Piece::WhiteRook &&
                pos.at(CastlingSquares::WHITE_QUEENSIDE_PATH[0]) == Piece::None &&
                pos.at(CastlingSquares::WHITE_QUEENSIDE_PATH[1]) == Piece::None &&
                pos.at(CastlingSquares::WHITE_QUEENSIDE_PATH[2]) == Piece::None) {
                
                // Only check attacks if path is clear
                if (!SqAttacked(CastlingSquares::WHITE_QUEENSIDE_ATTACK_CHECK[0], pos, enemy) &&
                    !SqAttacked(CastlingSquares::WHITE_QUEENSIDE_ATTACK_CHECK[1], pos, enemy) &&
                    !SqAttacked(CastlingSquares::WHITE_QUEENSIDE_ATTACK_CHECK[2], pos, enemy)) {
                    list.add_castle_move(make_castle(CastlingSquares::WHITE_KING_START, 
                                                   CastlingSquares::WHITE_QUEENSIDE_KING_TO));
                }
            }
        } else {
            // Black kingside - streamlined checks
            if ((pos.castling_rights & CASTLE_BK) &&
                pos.at(CastlingSquares::BLACK_KINGSIDE_ROOK) == Piece::BlackRook &&
                pos.at(CastlingSquares::BLACK_KINGSIDE_PATH[0]) == Piece::None &&
                pos.at(CastlingSquares::BLACK_KINGSIDE_PATH[1]) == Piece::None) {
                
                // Only check attacks if path is clear
                if (!SqAttacked(CastlingSquares::BLACK_KINGSIDE_ATTACK_CHECK[0], pos, enemy) &&
                    !SqAttacked(CastlingSquares::BLACK_KINGSIDE_ATTACK_CHECK[1], pos, enemy) &&
                    !SqAttacked(CastlingSquares::BLACK_KINGSIDE_ATTACK_CHECK[2], pos, enemy)) {
                    list.add_castle_move(make_castle(CastlingSquares::BLACK_KING_START, 
                                                   CastlingSquares::BLACK_KINGSIDE_KING_TO));
                }
            }
            
            // Black queenside - streamlined checks
            if ((pos.castling_rights & CASTLE_BQ) &&
                pos.at(CastlingSquares::BLACK_QUEENSIDE_ROOK) == Piece::BlackRook &&
                pos.at(CastlingSquares::BLACK_QUEENSIDE_PATH[0]) == Piece::None &&
                pos.at(CastlingSquares::BLACK_QUEENSIDE_PATH[1]) == Piece::None &&
                pos.at(CastlingSquares::BLACK_QUEENSIDE_PATH[2]) == Piece::None) {
                
                // Only check attacks if path is clear
                if (!SqAttacked(CastlingSquares::BLACK_QUEENSIDE_ATTACK_CHECK[0], pos, enemy) &&
                    !SqAttacked(CastlingSquares::BLACK_QUEENSIDE_ATTACK_CHECK[1], pos, enemy) &&
                    !SqAttacked(CastlingSquares::BLACK_QUEENSIDE_ATTACK_CHECK[2], pos, enemy)) {
                    list.add_castle_move(make_castle(CastlingSquares::BLACK_KING_START, 
                                                   CastlingSquares::BLACK_QUEENSIDE_KING_TO));
                }
            }
        }
    }

} // namespace KingOptimizations
