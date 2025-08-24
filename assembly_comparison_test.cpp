#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "move.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

// Test version with old clear() method for assembly comparison
void generate_all_moves_old_clear(const Position& pos, S_MOVELIST& list) {
    list.clear();  // OLD: Function call version
    
    Color us = pos.side_to_move;
    
    generate_pawn_moves(pos, list, us);
    generate_knight_moves(pos, list, us);
    generate_bishop_moves(pos, list, us);
    generate_rook_moves(pos, list, us);
    generate_queen_moves(pos, list, us);
    generate_king_moves(pos, list, us);
}

// Test version with new direct assignment for assembly comparison  
void generate_all_moves_new_direct(const Position& pos, S_MOVELIST& list) {
    list.count = 0;  // NEW: Direct assignment version
    
    Color us = pos.side_to_move;
    
    generate_pawn_moves(pos, list, us);
    generate_knight_moves(pos, list, us);
    generate_bishop_moves(pos, list, us);
    generate_rook_moves(pos, list, us);
    generate_queen_moves(pos, list, us);
    generate_king_moves(pos, list, us);
}
