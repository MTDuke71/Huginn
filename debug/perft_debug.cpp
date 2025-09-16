#include <iostream>
#include <chrono>
#include <iomanip>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "chess_types.hpp"

// Debug version of perft to check compilation flags
int main() {
    std::cout << "Huginn2 Bitboard Perft Debug\n";
    std::cout << "=============================\n";
    
    // Check if BITBOARD_ENGINE is defined
    #ifdef BITBOARD_ENGINE
        std::cout << "✅ BITBOARD_ENGINE is DEFINED\n";
    #else
        std::cout << "❌ BITBOARD_ENGINE is NOT DEFINED\n";
    #endif
    
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "\nDivide analysis for depth 1:\n";
    std::cout << "Move        Nodes\n";
    std::cout << "----        -----\n";
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        
        // Simple move to string conversion
        int from = move.get_from();
        int to = move.get_to();
        
        std::string from_str = "??";
        std::string to_str = "??";
        
        // Convert 120-square to algebraic (simplified)
        if (from >= 21 && from <= 98) {
            int file = (from % 10) - 1;
            int rank = (from / 10) - 2;
            if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
                from_str = std::string(1, 'a' + file) + std::to_string(rank + 1);
            }
        }
        
        if (to >= 21 && to <= 98) {
            int file = (to % 10) - 1;
            int rank = (to / 10) - 2;
            if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
                to_str = std::string(1, 'a' + file) + std::to_string(rank + 1);
            }
        }
        
        std::string move_str = from_str + to_str;
        std::cout << std::left << std::setw(12) << move_str 
                  << std::right << std::setw(8) << "1" << "\n";
    }
    
    std::cout << "\nTotal: " << move_list.count << " nodes\n";
    
    return 0;
}