#include <iostream>
#include "../src/position.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/init.hpp"
#include "../src/knight_lookup_tables.hpp"

int main() {
    std::cout << "Knight Move Generation Debug\n";
    std::cout << "============================\n";
    
    // Initialize engine
    Huginn::init();
    
    // Set up starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "\n1. Checking knight bitboards:\n";
    uint64_t white_knights = pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)];
    std::cout << "White knight bitboard: 0x" << std::hex << white_knights << std::dec << std::endl;
    
    // Should be 0x42 (bits 1 and 6 set for b1 and g1)
    std::cout << "Expected: 0x42 (for squares b1=1, g1=6)\n";
    
    std::cout << "\n2. Checking KNIGHT_ATTACKS table:\n";
    std::cout << "KNIGHT_ATTACKS[1] (b1): 0x" << std::hex << KnightLookupTables::KNIGHT_ATTACKS[1] << std::dec << std::endl;
    std::cout << "KNIGHT_ATTACKS[6] (g1): 0x" << std::hex << KnightLookupTables::KNIGHT_ATTACKS[6] << std::dec << std::endl;
    
    std::cout << "\n3. Testing knight move generation:\n";
    S_MOVELIST moves;
    moves.count = 0;
    
    BitboardMoveGen::generate_knight_moves_bitboard(pos, moves, Color::White);
    
    std::cout << "Generated knight moves (direct): " << moves.count << std::endl;
    
    std::cout << "\n4. Testing full bitboard move generation:\n";
    S_MOVELIST all_moves;
    all_moves.count = 0;
    
    BitboardMoveGen::generate_all_moves_bitboard(pos, all_moves);
    
    std::cout << "Generated total moves (bitboard): " << all_moves.count << std::endl;
    
    // Count knight moves in the full list
    int knight_count = 0;
    for (int i = 0; i < all_moves.count; ++i) {
        int from = all_moves.moves[i].get_from();
        // Check if from square has a knight (squares 22=b1, 27=g1 in 120-square)
        if (from == 22 || from == 27) {
            knight_count++;
            std::cout << "  Knight move: from=" << from << " to=" << all_moves.moves[i].get_to() << std::endl;
        }
    }
    std::cout << "Knight moves in full generation: " << knight_count << std::endl;
    
    if (moves.count == 0) {
        std::cout << "❌ NO KNIGHT MOVES GENERATED!\n";
        
        // Let's debug step by step
        std::cout << "\nDEBUG ANALYSIS:\n";
        
        // Check if we have knights
        if (white_knights == 0) {
            std::cout << "❌ No white knights found in position!\n";
        } else {
            std::cout << "✅ White knights found: 0x" << std::hex << white_knights << std::dec << std::endl;
        }
        
        // Check color bitboards
        uint64_t own_pieces = pos.color_bitboards[int(Color::White)];
        std::cout << "Own pieces bitboard: 0x" << std::hex << own_pieces << std::dec << std::endl;
        
    } else {
        std::cout << "✅ Knight moves generated successfully!\n";
        for (int i = 0; i < moves.count; ++i) {
            int from = moves.moves[i].get_from();
            int to = moves.moves[i].get_to();
            std::cout << "  Move " << i << ": from=" << from << " to=" << to << std::endl;
        }
    }
    
    return 0;
}