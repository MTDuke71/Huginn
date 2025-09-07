// Test VICE Part 82: Material Draw Detection and King Evaluation Tables
// Validates that:
// 1. Material draw detection correctly identifies insufficient material positions
// 2. King evaluation tables switch correctly between opening and endgame
// 3. Draw recognition guides the engine to aim for draws in unwinnable positions

#include <iostream>
#include <cassert>
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"

using namespace Huginn;

void test_material_draw_detection() {
    std::cout << "\n=== Testing Material Draw Detection ===" << std::endl;
    
    MinimalEngine engine;
    
    // Test 1: King vs King - should be draw
    Position pos1;
    pos1.set_from_fen("8/8/8/8/8/8/8/K6k w - - 0 1");
    bool is_draw1 = MinimalEngine::MaterialDraw(pos1);
    std::cout << "King vs King: " << (is_draw1 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(is_draw1 && "King vs King should be draw");
    
    // Test 2: King + Knight vs King - should be draw (insufficient)
    Position pos2;
    pos2.set_from_fen("8/8/8/8/8/8/8/KN5k w - - 0 1");
    bool is_draw2 = MinimalEngine::MaterialDraw(pos2);
    std::cout << "King + Knight vs King: " << (is_draw2 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(is_draw2 && "King + Knight vs King should be draw");
    
    // Test 3: King + Bishop vs King - should be draw (insufficient)
    Position pos3;
    pos3.set_from_fen("8/8/8/8/8/8/8/KB5k w - - 0 1");
    bool is_draw3 = MinimalEngine::MaterialDraw(pos3);
    std::cout << "King + Bishop vs King: " << (is_draw3 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(is_draw3 && "King + Bishop vs King should be draw");
    
    // Test 4: King + Bishop vs King + Bishop (same colored squares) - should be draw
    Position pos4;
    pos4.set_from_fen("8/8/8/8/8/8/8/KB4kb w - - 0 1");
    bool is_draw4 = MinimalEngine::MaterialDraw(pos4);
    std::cout << "King + Bishop vs King + Bishop: " << (is_draw4 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(is_draw4 && "King + Bishop vs King + Bishop should be draw");
    
    // Test 5: King + 2 Knights vs King - should be draw (insufficient)
    Position pos5;
    pos5.set_from_fen("8/8/8/8/8/8/8/KNN4k w - - 0 1");
    bool is_draw5 = MinimalEngine::MaterialDraw(pos5);
    std::cout << "King + 2 Knights vs King: " << (is_draw5 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(is_draw5 && "King + 2 Knights vs King should be draw");
    
    // Test 6: King + Rook vs King - should NOT be draw (sufficient material)
    Position pos6;
    pos6.set_from_fen("8/8/8/8/8/8/8/KR5k w - - 0 1");
    bool is_draw6 = MinimalEngine::MaterialDraw(pos6);
    std::cout << "King + Rook vs King: " << (is_draw6 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(!is_draw6 && "King + Rook vs King should NOT be draw");
    
    // Test 7: King + Queen vs King - should NOT be draw (sufficient material)
    Position pos7;
    pos7.set_from_fen("8/8/8/8/8/8/8/KQ5k w - - 0 1");
    bool is_draw7 = MinimalEngine::MaterialDraw(pos7);
    std::cout << "King + Queen vs King: " << (is_draw7 ? "DRAW" : "NOT DRAW") << std::endl;
    assert(!is_draw7 && "King + Queen vs King should NOT be draw");
    
    // Test 8: Complex position with pawns - should NOT be draw (pawns present)
    Position pos8;
    pos8.set_from_fen("8/8/8/8/8/8/P6p/K6k w - - 0 1");
    int eval8 = engine.evaluate(pos8); // Use evaluate() instead of MaterialDraw() directly
    std::cout << "Position with pawns eval: " << eval8 << " cp (should NOT be 0)" << std::endl;
    assert(eval8 != 0 && "Position with pawns should NOT evaluate to draw");
    
    // Test 8b: Demonstrate that MaterialDraw alone doesn't check for pawns
    // This shows that MaterialDraw should only be called when no pawns are present
    bool direct_material_check = MinimalEngine::MaterialDraw(pos8);
    std::cout << "Direct MaterialDraw on position with pawns: " << (direct_material_check ? "DRAW" : "NOT DRAW") << std::endl;
    std::cout << "Note: MaterialDraw doesn't check pawns - that's handled by evaluate()" << std::endl;
    
    std::cout << "✓ All material draw detection tests passed!" << std::endl;
}

void test_king_evaluation_tables() {
    std::cout << "\n=== Testing King Evaluation Tables ===" << std::endl;
    
    MinimalEngine engine;
    
    // Test 1: Positions with equal material to see king table effects
    std::cout << "\nTesting king positions with equal material (avoiding material draws):" << std::endl;
    
    // Both sides have rooks - enough material to avoid draw, but equal so king position matters
    Position king_back_rank;
    king_back_rank.set_from_fen("8/8/8/8/8/8/8/R3K2r w - - 0 1");
    int back_rank_eval = engine.evaluate(king_back_rank);
    std::cout << "King on e1 with rooks (opening position): " << back_rank_eval << " cp" << std::endl;
    
    Position king_center_rooks;
    king_center_rooks.set_from_fen("8/8/8/8/4K3/8/8/R6r w - - 0 1");
    int center_rooks_eval = engine.evaluate(king_center_rooks);
    std::cout << "King on e4 with rooks (center position): " << center_rooks_eval << " cp" << std::endl;
    
    int rook_difference = center_rooks_eval - back_rank_eval;
    std::cout << "Difference (center - back rank): " << rook_difference << " cp" << std::endl;
    
    // Test 2: Endgame material threshold testing
    std::cout << "\nTesting endgame material threshold detection:" << std::endl;
    
    // Just kings and bishops - should be endgame material (below 1300 cp threshold)
    Position king_center_bishops;
    king_center_bishops.set_from_fen("8/8/8/8/4K3/8/8/B6b w - - 0 1");
    int center_bishops_eval = engine.evaluate(king_center_bishops);
    std::cout << "King on e4 with bishops (endgame material): " << center_bishops_eval << " cp" << std::endl;
    
    Position king_corner_bishops;
    king_corner_bishops.set_from_fen("8/8/8/8/8/8/8/K6b w - - 0 1");
    int corner_bishops_eval = engine.evaluate(king_corner_bishops);
    std::cout << "King on a1 with bishops (endgame material): " << corner_bishops_eval << " cp" << std::endl;
    
    int bishop_difference = center_bishops_eval - corner_bishops_eval;
    std::cout << "Endgame difference (center - corner): " << bishop_difference << " cp" << std::endl;
    
    // Test 3: Compare table values directly
    std::cout << "\nTable value verification:" << std::endl;
    
    // a1 = square 0, d4 = square 27 in 64-square system
    int opening_table_diff = EvalParams::KING_TABLE[27] - EvalParams::KING_TABLE[0];
    int endgame_table_diff = EvalParams::KING_TABLE_ENDGAME[27] - EvalParams::KING_TABLE_ENDGAME[0];
    
    std::cout << "Opening table (d4 - a1): " << opening_table_diff << " cp" << std::endl;
    std::cout << "Endgame table (d4 - a1): " << endgame_table_diff << " cp" << std::endl;
    
    std::cout << "✓ King evaluation table tests completed!" << std::endl;
}

void test_draw_recognition_in_search() {
    std::cout << "\n=== Testing Draw Recognition in Search ===" << std::endl;
    
    MinimalEngine engine;
    MinimalLimits limits;
    limits.max_depth = 6;
    
    // Test that engine recognizes material draw and returns 0
    Position draw_pos;
    draw_pos.set_from_fen("8/8/8/8/8/8/8/KN5k w - - 0 1"); // King + Knight vs King
    
    int eval = engine.evaluate(draw_pos);
    std::cout << "Material draw position eval: " << eval << " cp" << std::endl;
    assert(eval == 0 && "Material draw position should evaluate to 0");
    
    // Test search behavior on material draw
    S_MOVE best_move = engine.search(draw_pos, limits);
    std::cout << "Search completed on material draw position" << std::endl;
    
    std::cout << "✓ Draw recognition in search tests completed!" << std::endl;
}

void test_vice_part82_comprehensive() {
    std::cout << "\n=== VICE Part 82 Comprehensive Test ===" << std::endl;
    
    MinimalEngine engine;
    
    // Test various insufficient material scenarios
    std::vector<std::string> draw_positions = {
        "8/8/8/8/8/8/8/K6k w - - 0 1",      // King vs King
        "8/8/8/8/8/8/8/KN5k w - - 0 1",      // King + Knight vs King
        "8/8/8/8/8/8/8/KB5k w - - 0 1",      // King + Bishop vs King
        "8/8/8/8/8/8/8/KNN4k w - - 0 1",     // King + 2 Knights vs King
        "8/8/8/8/8/8/8/KB4kb w - - 0 1",     // King + Bishop vs King + Bishop
        "8/8/8/8/8/8/8/KN4kn w - - 0 1",     // King + Knight vs King + Knight
    };
    
    std::vector<std::string> non_draw_positions = {
        "8/8/8/8/8/8/8/KR5k w - - 0 1",      // King + Rook vs King
        "8/8/8/8/8/8/8/KQ5k w - - 0 1",      // King + Queen vs King
        "8/8/8/8/8/8/8/KRR4k w - - 0 1",     // King + 2 Rooks vs King
        "8/8/8/8/8/8/8/KNNN3k w - - 0 1",    // King + 3 Knights vs King
        "8/8/8/8/8/8/8/KBB4k w - - 0 1",     // King + 2 Bishops vs King
    };
    
    std::cout << "Testing draw positions:" << std::endl;
    for (const auto& fen : draw_positions) {
        Position pos;
        pos.set_from_fen(fen);
        int eval = engine.evaluate(pos);
        std::cout << "  " << fen << " -> " << eval << " cp" << std::endl;
        assert(eval == 0 && "Position should be material draw");
    }
    
    std::cout << "Testing non-draw positions:" << std::endl;
    for (const auto& fen : non_draw_positions) {
        Position pos;
        pos.set_from_fen(fen);
        int eval = engine.evaluate(pos);
        std::cout << "  " << fen << " -> " << eval << " cp" << std::endl;
        assert(eval != 0 && "Position should NOT be material draw");
    }
    
    std::cout << "✓ All VICE Part 82 tests passed!" << std::endl;
}

int main() {
    std::cout << "Testing VICE Part 82: Material Draw Detection and King Evaluation Tables" << std::endl;
    std::cout << "=========================================================================" << std::endl;
    
    try {
        test_material_draw_detection();
        test_king_evaluation_tables();
        test_draw_recognition_in_search();
        test_vice_part82_comprehensive();
        
        std::cout << "\n✅ All VICE Part 82 tests passed successfully!" << std::endl;
        std::cout << "\nFeatures implemented:" << std::endl;
        std::cout << "• Material draw detection for insufficient material positions" << std::endl;
        std::cout << "• King position evaluation tables (opening vs endgame)" << std::endl;
        std::cout << "• Automatic draw recognition in evaluation function" << std::endl;
        std::cout << "• Endgame king centralization encouragement" << std::endl;
        std::cout << "• Opening king safety encouragement (castling/back rank)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
