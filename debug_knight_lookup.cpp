#include "position.hpp"
#include "knight_lookup_tables.hpp"
#include "knight_optimizations.hpp"
#include "move.hpp"
#include "init.hpp"
#include "board120.hpp"  // For MAILBOX_MAPS
#include <iostream>

int main() {
    // Initialize the engine
    Huginn::init();
    
    // Set up starting position
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Starting Position Knight Move Comparison ===\n";
    
    // Test with template function (known to work)
    S_MOVELIST template_moves;
    KnightOptimizations::generate_knight_moves_template(pos, template_moves, Color::White);
    
    // Test with lookup table function (broken)
    S_MOVELIST lookup_moves;
    KnightLookupTables::generate_knight_moves_lookup(pos, lookup_moves, Color::White);
    
    std::cout << "Template function moves: " << template_moves.count << std::endl;
    std::cout << "Lookup table moves: " << lookup_moves.count << std::endl;
    
    // Print knights positions for debugging
    std::cout << "\nWhite knight positions:\n";
    int piece_count = pos.pCount[int(Color::White)][int(PieceType::Knight)];
    for (int i = 0; i < piece_count; ++i) {
        int knight_pos = pos.pList[int(Color::White)][int(PieceType::Knight)][i];
        std::cout << "Knight " << i << " at square " << knight_pos << std::endl;
        
        // Check 64-square conversion
        int knight_64 = MAILBOX_MAPS.to64[knight_pos];
        std::cout << "  -> 64-square index: " << knight_64 << std::endl;
        
        if (knight_64 != -1) {
            std::cout << "  -> Lookup table move count: " << KnightLookupTables::KNIGHT_MOVE_COUNT[knight_64] << std::endl;
        }
    }
    
    return 0;
}