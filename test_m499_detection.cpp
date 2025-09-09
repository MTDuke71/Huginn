#include "src/transposition_table.hpp"
#include <iostream>

// Quick test to see what score produces "mate --499"
int main() {
    const int MATE = 29000;
    
    // Calculate what score would produce mate -499
    int mate_in_moves = 499;
    int mate_in_plies = mate_in_moves * 2 - 1;  // 997
    int problematic_score = -(MATE - mate_in_plies);  // -(29000 - 997) = -28003
    
    std::cout << "Score that produces mate -499: " << problematic_score << std::endl;
    
    // Test the new safety checks
    const int MAX_NORMAL_SCORE = 5000;
    
    bool would_be_caught1 = (problematic_score < -MATE + 2000);
    bool would_be_caught2 = (problematic_score < -MAX_NORMAL_SCORE && problematic_score > -MATE + 2000);
    
    std::cout << "Would be caught by check 1 (< -27000): " << would_be_caught1 << std::endl;
    std::cout << "Would be caught by check 2 (suspicious range): " << would_be_caught2 << std::endl;
    
    if (would_be_caught1 || would_be_caught2) {
        std::cout << "✓ Safety checks should prevent this corruption" << std::endl;
    } else {
        std::cout << "✗ Safety checks would NOT catch this!" << std::endl;
        std::cout << "Need to adjust the bounds" << std::endl;
    }
    
    // Test the transposition table directly
    TranspositionTable tt(1024);
    uint64_t test_key = 0x123456789ABCDEFULL;
    
    // Try to store the problematic score
    tt.store(test_key, problematic_score, 10, 2, 0x12345678);
    
    int retrieved_score;
    uint8_t depth, flag;
    uint32_t move;
    
    bool found = tt.probe(test_key, retrieved_score, depth, flag, move);
    
    if (found) {
        std::cout << "\nTransposition table test:" << std::endl;
        std::cout << "Stored: " << problematic_score << std::endl;
        std::cout << "Retrieved: " << retrieved_score << std::endl;
        std::cout << "This should have been blocked by safety checks!" << std::endl;
    } else {
        std::cout << "\n✓ Transposition table correctly blocked the corrupted score" << std::endl;
    }
    
    return 0;
}
