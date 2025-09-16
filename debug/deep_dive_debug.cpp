#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <string>

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

// Simple perft for counting nodes
uint64_t perft_simple(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    S_MOVELIST move_list;
    
    generate_legal_moves_enhanced(pos, move_list);
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        
        if (pos.MakeMove(move) == 1) {
            nodes += perft_simple(pos, depth - 1);
            pos.TakeMove();
        }
    }
    
    return nodes;
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
    std::cout << "Deep Dive Perft Debug: After b2b3" << std::endl;
    std::cout << "==================================" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    // Generate initial moves and find b2b3
    S_MOVELIST initial_moves;
    generate_legal_moves_enhanced(pos, initial_moves);
    
    S_MOVE b2b3_move = find_move(initial_moves, "b2b3");
    if (b2b3_move.move == 0) {
        std::cout << "ERROR: Could not find b2b3 move!" << std::endl;
        return 1;
    }
    
    std::cout << "Making move: b2b3" << std::endl;
    
    // Make the move
    if (pos.MakeMove(b2b3_move) != 1) {
        std::cout << "ERROR: b2b3 move was illegal!" << std::endl;
        return 1;
    }
    
    std::cout << "Position after b2b3:" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "En passant square: " << pos.ep_square << std::endl << std::endl;
    
    // Now do perft divide at depth 2 from this position
    std::cout << "Perft divide depth 2 from position after b2b3:" << std::endl;
    std::cout << "===============================================" << std::endl;
    
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    uint64_t total_nodes = 0;
    
    std::cout << "Move        Nodes" << std::endl;
    std::cout << "----        -----" << std::endl;
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        std::string move_str = move_to_string(move);
        
        if (pos.MakeMove(move) == 1) {
            uint64_t nodes = perft_simple(pos, 1); // depth 2 total = depth 1 from each move
            total_nodes += nodes;
            
            std::cout << std::left << std::setw(12) << move_str 
                      << std::right << std::setw(8) << nodes << std::endl;
            
            pos.TakeMove();
        }
    }
    
    std::cout << std::endl;
    std::cout << "Total nodes: " << total_nodes << std::endl;
    std::cout << "From our divide analysis, b2b3 should give 380 nodes" << std::endl;
    std::cout << "Expected (known good): 420 nodes" << std::endl;
    std::cout << "Difference: " << (long long)total_nodes - 420LL << std::endl;
    
    if (total_nodes != 420) {
        std::cout << std::endl << "ANALYSIS: The position after b2b3 is missing " 
                  << (420 - total_nodes) << " nodes." << std::endl;
        std::cout << "This suggests specific moves are not being generated correctly" << std::endl;
        std::cout << "from the position after b2b3." << std::endl;
        
        std::cout << std::endl << "Let's investigate deeper..." << std::endl;
        std::cout << "All Black moves give 19 nodes, but some should give more." << std::endl;
        std::cout << "Let's see what White moves are missing after a7a6:" << std::endl << std::endl;
        
        // Make a7a6 and see what White moves we get
        S_MOVE a7a6_move = find_move(move_list, "a7a6");
        if (a7a6_move.move != 0 && pos.MakeMove(a7a6_move) == 1) {
            S_MOVELIST white_moves;
            generate_legal_moves_enhanced(pos, white_moves);
            
            std::cout << "White moves after b2b3 a7a6:" << std::endl;
            std::cout << "Total: " << white_moves.count << " (should be 21)" << std::endl;
            
            for (int i = 0; i < white_moves.count; i++) {
                std::cout << "  " << move_to_string(white_moves.moves[i]) << std::endl;
            }
            
            pos.TakeMove(); // Undo a7a6
        }
    }
    
    return 0;
}