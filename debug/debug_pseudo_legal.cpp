#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// Forward declarations for accessing internal functions
void generate_pseudo_legal_moves(const Position& pos, MoveList& out);

int main() {
    Huginn::init();
    
    std::cout << "=== Pseudo-legal vs Legal Move Comparison ===" << std::endl;
    
    // Set up position after a2a4
    Position pos;
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1";
    
    if (!pos.set_from_fen(fen)) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << fen << std::endl;
    
    // Generate pseudo-legal moves
    MoveList pseudo_legal;
    generate_pseudo_legal_moves(pos, pseudo_legal);
    
    // Generate legal moves
    MoveList legal;
    generate_legal_moves(pos, legal);
    
    std::cout << "Pseudo-legal moves: " << pseudo_legal.v.size() << std::endl;
    std::cout << "Legal moves: " << legal.v.size() << std::endl;
    
    // Find b4 moves in pseudo-legal
    int b4_square = 52;
    std::cout << "\nPseudo-legal moves from b4:" << std::endl;
    int pseudo_count = 0;
    for (const auto& move : pseudo_legal.v) {
        if (move.get_from() == b4_square) {
            pseudo_count++;
            int to = move.get_to();
            int to_file = (to % 10) - 1;
            int to_rank = (to / 10) - 2;
            char to_file_char = 'a' + to_file;
            char to_rank_char = '1' + to_rank;
            
            std::cout << pseudo_count << ". b4" << to_file_char << to_rank_char;
            if (move.is_en_passant()) std::cout << " (en passant)";
            if (move.is_capture()) std::cout << " (capture)";
            std::cout << std::endl;
        }
    }
    
    // Find b4 moves in legal
    std::cout << "\nLegal moves from b4:" << std::endl;
    int legal_count = 0;
    for (const auto& move : legal.v) {
        if (move.get_from() == b4_square) {
            legal_count++;
            int to = move.get_to();
            int to_file = (to % 10) - 1;
            int to_rank = (to / 10) - 2;
            char to_file_char = 'a' + to_file;
            char to_rank_char = '1' + to_rank;
            
            std::cout << legal_count << ". b4" << to_file_char << to_rank_char;
            if (move.is_en_passant()) std::cout << " (en passant)";
            if (move.is_capture()) std::cout << " (capture)";
            std::cout << std::endl;
        }
    }
    
    std::cout << "\nFiltered out: " << (pseudo_count - legal_count) << " moves" << std::endl;
    
    return 0;
}
