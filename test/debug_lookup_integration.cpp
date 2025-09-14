#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "knight_optimizations.hpp"
#include "knight_lookup_tables.hpp"
#include "king_optimizations.hpp"
#include "king_lookup_tables.hpp"
#include "pawn_optimizations.hpp"
#include "sliding_piece_optimizations.hpp"
#include "board120.hpp"  // For MAILBOX_MAPS
#include "init.hpp"

TEST(DebugLookupVsTemplate, ComparePerftCallPattern) {
    Huginn::init(); // Initialize engine components including lookup tables
    
    Position pos;
    pos.set_startpos();
    
    // Test exactly what perft does: generate_legal_moves_enhanced
    S_MOVELIST legal_moves_with_lookup;
    generate_legal_moves_enhanced(pos, legal_moves_with_lookup);
    
    // Now test individual piece types to isolate the issue
    S_MOVELIST knight_template, knight_lookup, king_template, king_lookup, pawn_moves, sliding_moves;
    
    // Check knight piece counts and positions BEFORE calling functions
    int white_knight_count = pos.pCount[int(Color::White)][int(PieceType::Knight)];
    std::cout << "White knight count: " << white_knight_count << std::endl;
    
    // Print some lookup table values to debug initialization
    std::cout << "=== Lookup Table Debug Info ===" << std::endl;
    for (int sq = 0; sq < 8; ++sq) {
        std::cout << "Square " << sq << " lookup move count: " << KnightLookupTables::KNIGHT_MOVE_COUNT[sq] << std::endl;
    }
    
    if (white_knight_count > 0) {
        for (int i = 0; i < white_knight_count; ++i) {
            int knight_pos = pos.pList[int(Color::White)][int(PieceType::Knight)][i];
            std::cout << "Knight " << i << " at position " << knight_pos << std::endl;
            
            // Check 64-square conversion
            int knight_64 = MAILBOX_MAPS.to64[knight_pos];
            std::cout << "  -> 64-square index: " << knight_64 << std::endl;
            
            if (knight_64 >= 0 && knight_64 < 64) {
                std::cout << "  -> Lookup table move count: " << KnightLookupTables::KNIGHT_MOVE_COUNT[knight_64] << std::endl;
                // Print actual move destinations
                for (int j = 0; j < KnightLookupTables::KNIGHT_MOVE_COUNT[knight_64]; ++j) {
                    std::cout << "    Move " << j << ": " << KnightLookupTables::KNIGHT_MOVES[knight_64][j] << std::endl;
                }
            }
        }
    }
    
    // Knight moves - template vs lookup
    KnightOptimizations::generate_knight_moves_template(pos, knight_template, pos.side_to_move);
    KnightLookupTables::generate_knight_moves_lookup(pos, knight_lookup, pos.side_to_move);
    
    // King moves - optimized vs lookup  
    KingOptimizations::generate_king_moves_optimized(pos, king_template, pos.side_to_move);
    KingLookupTables::generate_king_moves_lookup(pos, king_lookup, pos.side_to_move);
    
    // Other pieces
    PawnOptimizations::generate_pawn_moves_optimized(pos, pawn_moves, pos.side_to_move);
    SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, sliding_moves, pos.side_to_move);
    
    // For lookup table, call through generate_all_moves (which should use lookup tables)
    S_MOVELIST all_pseudo_moves;
    generate_all_moves(pos, all_pseudo_moves);
    
    std::cout << "=== Starting Position Move Generation Debug ===" << std::endl;
    std::cout << "Legal moves (via perft path): " << legal_moves_with_lookup.count << std::endl;
    std::cout << "All pseudo-moves (via generate_all_moves): " << all_pseudo_moves.count << std::endl;
    std::cout << "Knight template moves: " << knight_template.count << std::endl;
    std::cout << "Knight lookup moves: " << knight_lookup.count << std::endl;
    std::cout << "King template moves: " << king_template.count << std::endl; 
    std::cout << "King lookup moves: " << king_lookup.count << std::endl;
    std::cout << "Pawn moves: " << pawn_moves.count << std::endl;
    std::cout << "Sliding piece moves: " << sliding_moves.count << std::endl;
    
    int expected_total = knight_template.count + king_template.count + pawn_moves.count + sliding_moves.count;
    std::cout << "Expected total (template-based): " << expected_total << std::endl;
    
    // The legal moves should be 20, and generate_all_moves should generate more (16 pawns + 4 knights = 20)
    EXPECT_EQ(legal_moves_with_lookup.count, 20);
}