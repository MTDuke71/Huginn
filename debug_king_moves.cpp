#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/board120.hpp"
#include "src/init.hpp"
#include "src/king_lookup_tables.hpp"
#include "src/king_optimizations.hpp"
#include <iostream>

int main() {
    // Initialize the engine
    AllInit();
    
    // Set up a simple position with just a king
    Position pos;
    pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    // Debug: Check piece counts
    std::cout << "King count: " << pos.pCount[int(Color::White)][int(PieceType::King)] << std::endl;
    std::cout << "King position: " << pos.pList[int(Color::White)][int(PieceType::King)][0] << std::endl;
    std::cout << "King square stored: " << pos.king_sq[int(Color::White)] << std::endl;
    std::cout << "Square D4: " << sq(File::D, Rank::R4) << std::endl;
    
    // Test different move generation methods
    S_MOVELIST moves1, moves2, moves3;
    
    std::cout << "\n=== Testing generate_all_moves ===" << std::endl;
    generate_all_moves(pos, moves1);
    std::cout << "Total moves from generate_all_moves: " << moves1.size() << std::endl;
    
    std::cout << "\n=== Testing KingLookupTables ===" << std::endl;
    KingLookupTables::generate_king_moves_lookup(pos, moves2, Color::White);
    std::cout << "Total moves from lookup tables: " << moves2.size() << std::endl;
    
    std::cout << "\n=== Testing KingOptimizations ===" << std::endl;
    KingOptimizations::generate_king_moves_optimized(pos, moves3, Color::White);
    std::cout << "Total moves from optimizations: " << moves3.size() << std::endl;
    
    // Print all moves
    std::cout << "\n=== All moves from generate_all_moves ===" << std::endl;
    for (int i = 0; i < moves1.size(); ++i) {
        std::cout << "Move " << i << ": from=" << moves1[i].get_from() 
                  << " to=" << moves1[i].get_to() 
                  << " piece=" << int(pos.at(moves1[i].get_from())) << std::endl;
    }
    
    return 0;
}