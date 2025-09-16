#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Move Generation Analysis" << std::endl;
    std::cout << "=======================" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    // Generate all legal moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "Total legal moves: " << move_list.count << std::endl;
    std::cout << "Expected: 20" << std::endl << std::endl;
    
    // Categorize moves
    int pawn_moves = 0;
    int knight_moves = 0;
    
    std::cout << "All generated moves:" << std::endl;
    for (int i = 0; i < move_list.count; i++) {
        const S_MOVE& move = move_list.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        // Convert to algebraic notation
        int from_file = MAILBOX_MAPS.to64[from] % 8;
        int from_rank = MAILBOX_MAPS.to64[from] / 8;
        int to_file = MAILBOX_MAPS.to64[to] % 8;
        int to_rank = MAILBOX_MAPS.to64[to] / 8;
        
        char from_file_char = 'a' + from_file;
        char from_rank_char = '1' + from_rank;
        char to_file_char = 'a' + to_file;
        char to_rank_char = '1' + to_rank;
        
        // Check piece type
        Piece piece = pos.board[from];
        PieceType piece_type = type_of(piece);
        
        std::cout << i+1 << ". " << from_file_char << from_rank_char 
                  << to_file_char << to_rank_char;
                  
        if (piece_type == PieceType::Pawn) {
            std::cout << " (pawn)";
            pawn_moves++;
        } else if (piece_type == PieceType::Knight) {
            std::cout << " (knight)";
            knight_moves++;
        } else {
            std::cout << " (" << to_char(piece) << ")";
        }
        
        std::cout << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Move breakdown:" << std::endl;
    std::cout << "Pawn moves: " << pawn_moves << " (expected: 16)" << std::endl;
    std::cout << "Knight moves: " << knight_moves << " (expected: 4)" << std::endl;
    std::cout << "Other moves: " << (move_list.count - pawn_moves - knight_moves) << " (expected: 0)" << std::endl;
    
    // Check for missing knight moves specifically
    std::cout << std::endl << "Checking expected knight moves:" << std::endl;
    
    // Expected knight moves: b1a3, b1c3, g1f3, g1h3
    std::vector<std::string> expected_knight_moves = {"b1a3", "b1c3", "g1f3", "g1h3"};
    
    for (const std::string& expected : expected_knight_moves) {
        bool found = false;
        for (int j = 0; j < move_list.count; j++) {
            const S_MOVE& move = move_list.moves[j];
            int from = move.get_from();
            int to = move.get_to();
            
            int from_file = MAILBOX_MAPS.to64[from] % 8;
            int from_rank = MAILBOX_MAPS.to64[from] / 8;
            int to_file = MAILBOX_MAPS.to64[to] % 8;
            int to_rank = MAILBOX_MAPS.to64[to] / 8;
            
            std::string move_str = "";
            move_str += ('a' + from_file);
            move_str += ('1' + from_rank);
            move_str += ('a' + to_file);
            move_str += ('1' + to_rank);
            
            if (move_str == expected) {
                found = true;
                break;
            }
        }
        
        std::cout << expected << ": " << (found ? "FOUND" : "MISSING") << std::endl;
    }
    
    return 0;
}