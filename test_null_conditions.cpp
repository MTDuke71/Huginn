#include "src/init.hpp"
#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include <iostream>

using namespace Huginn;

// Custom search that reports null move attempts
int TestAlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull, bool isRoot) {
    static int call_count = 0;
    call_count++;
    
    if (call_count <= 10) {  // Only show first 10 calls to avoid spam
        std::cout << "Call " << call_count << ": depth=" << depth << " doNull=" << doNull << " isRoot=" << isRoot << std::endl;
    }
    
    // Check conditions for null move
    bool in_check = SqAttacked(pos.king_sq[int(pos.side_to_move)], pos, !pos.side_to_move);
    const int MIN_NULL_MOVE_DEPTH = 4;
    
    if (doNull && !in_check && !isRoot && depth >= MIN_NULL_MOVE_DEPTH && 
        pos.has_non_pawn_material(pos.side_to_move)) {
        
        std::cout << "*** NULL MOVE ATTEMPTED at depth " << depth << " ***" << std::endl;
        info.null_cut++; // Just increment for this test
        return beta; // Fake cutoff for testing
    }
    
    // Just return a fake score for testing
    return alpha + 10;
}

int main() {
    init();
    
    std::cout << "=== Null Move Testing ===" << std::endl;
    
    std::string fen = "r1bq1rk1/pp2nppp/2n1p3/3pP3/2PP4/2N1BN2/PP3PPP/R2QK2R w KQ - 0 10";
    Position pos;
    pos.set_from_fen(fen);
    
    SearchInfo info;
    
    // Test with different parameters
    std::cout << "\nTest 1: Root call (should NOT trigger null move)" << std::endl;
    TestAlphaBeta(pos, -1000, 1000, 6, info, true, true);
    
    std::cout << "\nTest 2: Non-root, depth 6 (SHOULD trigger null move)" << std::endl;
    TestAlphaBeta(pos, -1000, 1000, 6, info, true, false);
    
    std::cout << "\nTest 3: Non-root, depth 3 (should NOT trigger - too shallow)" << std::endl;
    TestAlphaBeta(pos, -1000, 1000, 3, info, true, false);
    
    std::cout << "\nTest 4: doNull=false (should NOT trigger)" << std::endl;
    TestAlphaBeta(pos, -1000, 1000, 6, info, false, false);
    
    std::cout << "\nNull cuts recorded: " << info.null_cut << std::endl;
    
    return 0;
}
