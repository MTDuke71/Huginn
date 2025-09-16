#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/pawn_optimizations.hpp"
#include "../src/knight_optimizations.hpp"
#include "../src/sliding_piece_optimizations.hpp"
#include "../src/king_optimizations.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <set>

// Helper function to convert move to string
std::string move_to_string(const S_MOVE& move) {
    if (move.move == 0) return "null";
    
    int from = move.get_from();
    int to = move.get_to();
    
    // Convert from 120-square to file/rank
    int from_file = (from % 10) - 1;
    int from_rank = (from / 10) - 2;
    int to_file = (to % 10) - 1;
    int to_rank = (to / 10) - 2;
    
    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
        to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) {
        return "??"; // Invalid square
    }
    
    std::string result = "";
    result += ('a' + from_file);
    result += ('1' + from_rank);
    result += ('a' + to_file);
    result += ('1' + to_rank);
    
    // Add promotion piece if applicable
    PieceType promoted = move.get_promoted();
    if (promoted != PieceType::None) {
        switch (promoted) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    
    return result;
}

// Generate moves using piece list method (force traditional path)
void generate_moves_piecelist(const Position& pos, S_MOVELIST& list) {
    list.count = 0;
    Color us = pos.side_to_move;
    
    // Use traditional piece list generation (same as generate_all_moves without BITBOARD_ENGINE)
    PawnOptimizations::generate_pawn_moves_template(pos, list, us);
    KnightOptimizations::generate_knight_moves_template(pos, list, us);
    
    // Generate sliding piece moves using traditional method
    SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, list, us);
    
    // King moves - use optimized version
    KingOptimizations::generate_king_moves_optimized(pos, list, us);
}

// Generate moves using bitboard method (force bitboard path)
void generate_moves_bitboard(const Position& pos, S_MOVELIST& list) {
    BitboardMoveGen::generate_all_moves_bitboard(pos, list);
}

// Apply legal move filtering
void filter_legal_moves(Position& pos, S_MOVELIST& pseudo_moves, S_MOVELIST& legal_moves) {
    legal_moves.count = 0;
    
    for (int i = 0; i < pseudo_moves.count; ++i) {
        if (pos.MakeMove(pseudo_moves.moves[i]) == 1) {
            legal_moves.add_quiet_move(pseudo_moves.moves[i]);
            pos.TakeMove();
        }
    }
}

// Find a specific move in move list
S_MOVE find_move(const S_MOVELIST& move_list, const std::string& move_str) {
    for (int i = 0; i < move_list.count; i++) {
        if (move_to_string(move_list.moves[i]) == move_str) {
            return move_list.moves[i];
        }
    }
    return S_MOVE{}; // Return null move if not found
}

int main() {
    std::cout << "Hybrid Method Comparison: Bitboard vs Piece List" << std::endl;
    std::cout << "===============================================" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    // Make the sequence: b2b3 a7a6 to get to the problematic position
    S_MOVELIST initial_moves;
    generate_legal_moves_enhanced(pos, initial_moves);
    
    S_MOVE b2b3_move = find_move(initial_moves, "b2b3");
    if (b2b3_move.move == 0 || pos.MakeMove(b2b3_move) != 1) {
        std::cout << "ERROR: Could not make b2b3 move!" << std::endl;
        return 1;
    }
    
    S_MOVELIST black_moves;
    generate_legal_moves_enhanced(pos, black_moves);
    
    S_MOVE a7a6_move = find_move(black_moves, "a7a6");
    if (a7a6_move.move == 0 || pos.MakeMove(a7a6_move) != 1) {
        std::cout << "ERROR: Could not make a7a6 move!" << std::endl;
        return 1;
    }
    
    std::cout << "Position after: b2b3 a7a6" << std::endl;
    std::cout << "Side to move: White" << std::endl << std::endl;
    
    // Generate moves using both methods
    std::cout << "PIECE LIST METHOD:" << std::endl;
    std::cout << "==================" << std::endl;
    
    S_MOVELIST piecelist_pseudo, piecelist_legal;
    generate_moves_piecelist(pos, piecelist_pseudo);
    filter_legal_moves(pos, piecelist_pseudo, piecelist_legal);
    
    std::set<std::string> piecelist_moves;
    std::cout << "Legal moves (" << piecelist_legal.count << " total):" << std::endl;
    for (int i = 0; i < piecelist_legal.count; i++) {
        std::string move_str = move_to_string(piecelist_legal.moves[i]);
        piecelist_moves.insert(move_str);
        std::cout << "  " << move_str << std::endl;
    }
    
    std::cout << std::endl << "BITBOARD METHOD:" << std::endl;
    std::cout << "================" << std::endl;
    
    S_MOVELIST bitboard_pseudo, bitboard_legal;
    generate_moves_bitboard(pos, bitboard_pseudo);
    filter_legal_moves(pos, bitboard_pseudo, bitboard_legal);
    
    std::set<std::string> bitboard_moves;
    std::cout << "Legal moves (" << bitboard_legal.count << " total):" << std::endl;
    for (int i = 0; i < bitboard_legal.count; i++) {
        std::string move_str = move_to_string(bitboard_legal.moves[i]);
        bitboard_moves.insert(move_str);
        std::cout << "  " << move_str << std::endl;
    }
    
    std::cout << std::endl << "COMPARISON:" << std::endl;
    std::cout << "===========" << std::endl;
    
    // Find moves in piece list but not in bitboard
    std::cout << "Moves in PIECE LIST but NOT in BITBOARD:" << std::endl;
    for (const std::string& move : piecelist_moves) {
        if (bitboard_moves.find(move) == bitboard_moves.end()) {
            std::cout << "  " << move << " *** MISSING FROM BITBOARD ***" << std::endl;
        }
    }
    
    // Find moves in bitboard but not in piece list
    std::cout << std::endl << "Moves in BITBOARD but NOT in PIECE LIST:" << std::endl;
    for (const std::string& move : bitboard_moves) {
        if (piecelist_moves.find(move) == piecelist_moves.end()) {
            std::cout << "  " << move << " *** EXTRA IN BITBOARD ***" << std::endl;
        }
    }
    
    std::cout << std::endl << "SUMMARY:" << std::endl;
    std::cout << "Piece list method: " << piecelist_legal.count << " moves" << std::endl;
    std::cout << "Bitboard method:   " << bitboard_legal.count << " moves" << std::endl;
    std::cout << "Difference:        " << (int)piecelist_legal.count - (int)bitboard_legal.count << std::endl;
    
    return 0;
}