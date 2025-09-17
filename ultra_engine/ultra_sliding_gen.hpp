/**
 * @file ultra_sliding_gen.hpp
 * @brief Ultra-optimized sliding piece move generation (bishops, rooks, queens)
 * 
 * This is the performance-critical component - sliding pieces dominate move
 * generation complexity. Uses magic bitboards with optimized attack computation
 * and pure 64-bit coordinates for maximum performance.
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#pragma once
#include "ultra_move.hpp"
#include "ultra_attacks.hpp"

namespace UltraEngine {

// Forward declarations to avoid conflicts
template<typename Position> class UltraSlidingGen;

// ============================================================================
// ULTRA-FAST SLIDING PIECE MOVE GENERATION
// Template-based approach for maximum flexibility and performance
// ============================================================================

template<typename Position>
class UltraSlidingGen {
public:
    // ========================================================================
    // BISHOP MOVE GENERATION
    // ========================================================================
    
    // Generate all legal bishop moves for given color
    static void generate_bishop_moves(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t bishops = pos.get_piece_board(color, Position::Bishop);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t bishop_bb = bishops;
        while (bishop_bb != 0) {
            const int from_sq = pop_lsb(bishop_bb);
            
            // Get all possible bishop attacks from this square
            const uint64_t attacks = UltraAttacks::bishop(from_sq, all_occupied) & ~own_pieces;
            
            // Generate captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece);
            }
            
            // Generate quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Generate only bishop captures for given color
    static void generate_bishop_captures(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t bishops = pos.get_piece_board(color, Position::Bishop);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t bishop_bb = bishops;
        while (bishop_bb != 0) {
            const int from_sq = pop_lsb(bishop_bb);
            
            // Get bishop attacks that hit enemy pieces
            uint64_t captures = UltraAttacks::bishop(from_sq, all_occupied) & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece);
            }
        }
    }
    
    // Generate only quiet bishop moves for given color
    static void generate_bishop_quiet(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t bishops = pos.get_piece_board(color, Position::Bishop);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t bishop_bb = bishops;
        while (bishop_bb != 0) {
            const int from_sq = pop_lsb(bishop_bb);
            
            // Get bishop attacks that don't hit any pieces
            uint64_t quiet_moves = UltraAttacks::bishop(from_sq, all_occupied) & ~own_pieces & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Count bishop moves without generating them (ultra-fast for perft)
    static int count_bishop_moves(const Position& pos, int color) {
        const uint64_t bishops = pos.get_piece_board(color, Position::Bishop);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        int count = 0;
        uint64_t bishop_bb = bishops;
        while (bishop_bb != 0) {
            const int from_sq = pop_lsb(bishop_bb);
            const uint64_t attacks = UltraAttacks::bishop(from_sq, all_occupied) & ~own_pieces;
            count += popcount(attacks);
        }
        
        return count;
    }
    
    // ========================================================================
    // ROOK MOVE GENERATION
    // ========================================================================
    
    // Generate all legal rook moves for given color
    static void generate_rook_moves(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t rooks = pos.get_piece_board(color, Position::Rook);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t rook_bb = rooks;
        while (rook_bb != 0) {
            const int from_sq = pop_lsb(rook_bb);
            
            // Get all possible rook attacks from this square
            const uint64_t attacks = UltraAttacks::rook(from_sq, all_occupied) & ~own_pieces;
            
            // Generate captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece);
            }
            
            // Generate quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Generate only rook captures for given color
    static void generate_rook_captures(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t rooks = pos.get_piece_board(color, Position::Rook);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t rook_bb = rooks;
        while (rook_bb != 0) {
            const int from_sq = pop_lsb(rook_bb);
            
            // Get rook attacks that hit enemy pieces
            uint64_t captures = UltraAttacks::rook(from_sq, all_occupied) & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece);
            }
        }
    }
    
    // Generate only quiet rook moves for given color
    static void generate_rook_quiet(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t rooks = pos.get_piece_board(color, Position::Rook);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t rook_bb = rooks;
        while (rook_bb != 0) {
            const int from_sq = pop_lsb(rook_bb);
            
            // Get rook attacks that don't hit any pieces
            uint64_t quiet_moves = UltraAttacks::rook(from_sq, all_occupied) & ~own_pieces & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Count rook moves without generating them (ultra-fast for perft)
    static int count_rook_moves(const Position& pos, int color) {
        const uint64_t rooks = pos.get_piece_board(color, Position::Rook);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        int count = 0;
        uint64_t rook_bb = rooks;
        while (rook_bb != 0) {
            const int from_sq = pop_lsb(rook_bb);
            const uint64_t attacks = UltraAttacks::rook(from_sq, all_occupied) & ~own_pieces;
            count += popcount(attacks);
        }
        
        return count;
    }
    
    // ========================================================================
    // QUEEN MOVE GENERATION
    // ========================================================================
    
    // Generate all legal queen moves for given color
    static void generate_queen_moves(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t queens = pos.get_piece_board(color, Position::Queen);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t queen_bb = queens;
        while (queen_bb != 0) {
            const int from_sq = pop_lsb(queen_bb);
            
            // Get all possible queen attacks from this square (rook + bishop)
            const uint64_t attacks = UltraAttacks::queen(from_sq, all_occupied) & ~own_pieces;
            
            // Generate captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece);
            }
            
            // Generate quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Generate only queen captures for given color
    static void generate_queen_captures(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t queens = pos.get_piece_board(color, Position::Queen);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t queen_bb = queens;
        while (queen_bb != 0) {
            const int from_sq = pop_lsb(queen_bb);
            
            // Get queen attacks that hit enemy pieces
            uint64_t captures = UltraAttacks::queen(from_sq, all_occupied) & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece);
            }
        }
    }
    
    // Generate only quiet queen moves for given color
    static void generate_queen_quiet(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t queens = pos.get_piece_board(color, Position::Queen);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        uint64_t queen_bb = queens;
        while (queen_bb != 0) {
            const int from_sq = pop_lsb(queen_bb);
            
            // Get queen attacks that don't hit any pieces
            uint64_t quiet_moves = UltraAttacks::queen(from_sq, all_occupied) & ~own_pieces & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Count queen moves without generating them (ultra-fast for perft)
    static int count_queen_moves(const Position& pos, int color) {
        const uint64_t queens = pos.get_piece_board(color, Position::Queen);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        int count = 0;
        uint64_t queen_bb = queens;
        while (queen_bb != 0) {
            const int from_sq = pop_lsb(queen_bb);
            const uint64_t attacks = UltraAttacks::queen(from_sq, all_occupied) & ~own_pieces;
            count += popcount(attacks);
        }
        
        return count;
    }
    
    // ========================================================================
    // COMBINED SLIDING PIECE GENERATION
    // ========================================================================
    
    // Generate all sliding piece moves (bishops, rooks, queens) for given color
    static void generate_all_sliding_moves(const Position& pos, UltraMoveList& moves, int color) {
        generate_bishop_moves(pos, moves, color);
        generate_rook_moves(pos, moves, color);
        generate_queen_moves(pos, moves, color);
    }
    
    // Generate only sliding piece captures for given color
    static void generate_all_sliding_captures(const Position& pos, UltraMoveList& moves, int color) {
        generate_bishop_captures(pos, moves, color);
        generate_rook_captures(pos, moves, color);
        generate_queen_captures(pos, moves, color);
    }
    
    // Generate only quiet sliding piece moves for given color
    static void generate_all_sliding_quiet(const Position& pos, UltraMoveList& moves, int color) {
        generate_bishop_quiet(pos, moves, color);
        generate_rook_quiet(pos, moves, color);
        generate_queen_quiet(pos, moves, color);
    }
    
    // Count all sliding piece moves without generating them (ultra-fast for perft)
    static int count_all_sliding_moves(const Position& pos, int color) {
        return count_bishop_moves(pos, color) + 
               count_rook_moves(pos, color) + 
               count_queen_moves(pos, color);
    }
    
    // ========================================================================
    // OPTIMIZED BULK OPERATIONS
    // For maximum performance in critical paths
    // ========================================================================
    
    // Ultra-optimized combined sliding piece generation
    // Minimizes function call overhead by doing all pieces in one pass
    static void generate_sliding_moves_bulk(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        // Process bishops
        uint64_t bishops = pos.get_piece_board(color, Position::Bishop);
        while (bishops != 0) {
            const int from_sq = pop_lsb(bishops);
            const uint64_t attacks = UltraAttacks::bishop(from_sq, all_occupied) & ~own_pieces;
            
            // Captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                moves.add_capture(from_sq, to_sq, pos.get_piece_type_at(to_sq));
            }
            
            // Quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
        
        // Process rooks
        uint64_t rooks = pos.get_piece_board(color, Position::Rook);
        while (rooks != 0) {
            const int from_sq = pop_lsb(rooks);
            const uint64_t attacks = UltraAttacks::rook(from_sq, all_occupied) & ~own_pieces;
            
            // Captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                moves.add_capture(from_sq, to_sq, pos.get_piece_type_at(to_sq));
            }
            
            // Quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
        
        // Process queens
        uint64_t queens = pos.get_piece_board(color, Position::Queen);
        while (queens != 0) {
            const int from_sq = pop_lsb(queens);
            const uint64_t attacks = UltraAttacks::queen(from_sq, all_occupied) & ~own_pieces;
            
            // Captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                moves.add_capture(from_sq, to_sq, pos.get_piece_type_at(to_sq));
            }
            
            // Quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Ultra-optimized move counting for perft
    static int count_sliding_moves_bulk(const Position& pos, int color) {
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t all_occupied = pos.get_all_occupied();
        
        int count = 0;
        
        // Count bishop moves
        uint64_t bishops = pos.get_piece_board(color, Position::Bishop);
        while (bishops != 0) {
            const int from_sq = pop_lsb(bishops);
            count += popcount(UltraAttacks::bishop(from_sq, all_occupied) & ~own_pieces);
        }
        
        // Count rook moves
        uint64_t rooks = pos.get_piece_board(color, Position::Rook);
        while (rooks != 0) {
            const int from_sq = pop_lsb(rooks);
            count += popcount(UltraAttacks::rook(from_sq, all_occupied) & ~own_pieces);
        }
        
        // Count queen moves
        uint64_t queens = pos.get_piece_board(color, Position::Queen);
        while (queens != 0) {
            const int from_sq = pop_lsb(queens);
            count += popcount(UltraAttacks::queen(from_sq, all_occupied) & ~own_pieces);
        }
        
        return count;
    }
    
    // ========================================================================
    // TEMPLATE INTERFACE METHODS FOR POSITION INTEGRATION
    // These methods provide the standard interface expected by UltraPosition
    // ========================================================================
    
    // Generate all sliding piece moves (template interface)
    static int generate_all(const Position& pos, UltraMoveList& moves, int color) {
        const int initial_count = moves.size();
        generate_all_sliding_moves(pos, moves, color);
        return moves.size() - initial_count;
    }
    
    // Generate only sliding piece captures (template interface)
    static int generate_captures(const Position& pos, UltraMoveList& moves, int color) {
        const int initial_count = moves.size();
        generate_all_sliding_captures(pos, moves, color);
        return moves.size() - initial_count;
    }
    
    // Generate only quiet sliding piece moves (template interface)
    static int generate_quiet(const Position& pos, UltraMoveList& moves, int color) {
        const int initial_count = moves.size();
        generate_all_sliding_quiet(pos, moves, color);
        return moves.size() - initial_count;
    }
    
    // Count sliding piece moves (template interface)
    static int count_moves(const Position& pos, int color) {
        return count_all_sliding_moves(pos, color);
    }
};

} // namespace UltraEngine
