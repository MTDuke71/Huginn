#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "En Passant Test" << std::endl;
    std::cout << "===============" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    
    // Set up position with en passant opportunity
    // FEN: rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3
    std::string fen = "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3";
    if (!pos.set_from_fen(fen)) {
        std::cout << "Failed to set FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "Position: " << fen << std::endl;
    std::cout << "En passant square: " << pos.ep_square << std::endl;
    
    if (pos.ep_square != -1) {
        int ep_file = (pos.ep_square % 10) - 1;
        int ep_rank = (pos.ep_square / 10) - 2;
        std::cout << "En passant square: " << (char)('a' + ep_file) << (ep_rank + 1) << std::endl;
    }
    
    // Generate moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "\nTotal legal moves: " << move_list.count << std::endl;
    
    // Look for en passant moves
    int en_passant_count = 0;
    std::cout << "\nEn passant moves found:" << std::endl;
    
    for (int i = 0; i < move_list.count; i++) {
        const S_MOVE& move = move_list.moves[i];
        if (move.is_en_passant()) {
            en_passant_count++;
            
            // Convert to string
            int from = move.get_from();
            int to = move.get_to();
            int from_file = (from % 10) - 1;
            int from_rank = (from / 10) - 2;
            int to_file = (to % 10) - 1;
            int to_rank = (to / 10) - 2;
            
            std::cout << "  " << (char)('a' + from_file) << (from_rank + 1)
                      << (char)('a' + to_file) << (to_rank + 1) << " (en passant)" << std::endl;
        }
    }
    
    std::cout << "\nTotal en passant moves: " << en_passant_count << std::endl;
    std::cout << "Expected: 1 (e5f6)" << std::endl;
    
    return 0;
}