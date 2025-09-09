#include "src/transposition_table.hpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Debugging Non-Mate Position Score Corruption ===" << std::endl;
    
    // Test normal chess position scores (typically -500 to +500 centipawns)
    int normal_scores[] = {
        0,      // Equal position
        50,     // Slight advantage
        -50,    // Slight disadvantage  
        150,    // Small advantage
        -150,   // Small disadvantage
        500,    // Winning advantage
        -500,   // Losing position
        1000,   // Large advantage
        -1000,  // Large disadvantage
        2000,   // Huge advantage
        -2000   // Huge disadvantage
    };
    
    TranspositionTable tt(1024);
    
    std::cout << "\nTesting normal position scores:" << std::endl;
    std::cout << std::setw(12) << "Original" << std::setw(12) << "Retrieved" << std::setw(10) << "Diff" << std::setw(15) << "UCI Format" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    const int MATE = 29000;
    
    for (int i = 0; i < sizeof(normal_scores)/sizeof(normal_scores[0]); i++) {
        int original = normal_scores[i];
        uint64_t key = 0x1000000000000000ULL + i;
        
        // Store the score
        tt.store(key, original, 10, 2, 0x12345678);
        
        // Retrieve the score
        int retrieved;
        uint8_t depth, flag;
        uint32_t move;
        
        bool found = tt.probe(key, retrieved, depth, flag, move);
        
        if (found) {
            int diff = retrieved - original;
            
            // Determine UCI format
            std::string uci_format;
            if (retrieved > MATE - 100) {
                int mate_in_plies = MATE - retrieved;
                int mate_in_moves = (mate_in_plies + 1) / 2;
                uci_format = "mate " + std::to_string(mate_in_moves);
            } else if (retrieved < -MATE + 100) {
                int mate_in_plies = MATE + retrieved;
                int mate_in_moves = (mate_in_plies + 1) / 2;
                uci_format = "mate -" + std::to_string(mate_in_moves);
            } else {
                uci_format = "cp " + std::to_string(retrieved);
            }
            
            std::cout << std::setw(12) << original 
                      << std::setw(12) << retrieved 
                      << std::setw(10) << diff 
                      << std::setw(15) << uci_format;
            
            if (diff != 0) {
                std::cout << " ✗ CORRUPTED!";
            } else {
                std::cout << " ✓";
            }
            std::cout << std::endl;
            
        } else {
            std::cout << std::setw(12) << original << "    NOT FOUND" << std::endl;
        }
    }
    
    // Test the specific range that would cause -M499
    std::cout << "\nTesting scores that would produce -M499:" << std::endl;
    
    // If UCI shows "mate -499", then:
    // mate_in_moves = 499
    // mate_in_plies = 499 * 2 - 1 = 997 (approximately)
    // score = -(MATE - mate_in_plies) = -(29000 - 997) = -28003
    
    int problematic_score = -28003;
    std::cout << "Score that should give -M499: " << problematic_score << std::endl;
    
    uint64_t key_prob = 0x9999999999999999ULL;
    tt.store(key_prob, problematic_score, 15, 1, 0xABCDEF12);
    
    int retrieved_prob;
    uint8_t d, f;
    uint32_t m;
    
    if (tt.probe(key_prob, retrieved_prob, d, f, m)) {
        std::cout << "Retrieved problematic score: " << retrieved_prob << std::endl;
        std::cout << "Difference: " << (retrieved_prob - problematic_score) << std::endl;
        
        if (retrieved_prob < -MATE + 100) {
            int mate_in_plies = MATE + retrieved_prob;
            int mate_in_moves = (mate_in_plies + 1) / 2;
            std::cout << "UCI output: mate -" << mate_in_moves << std::endl;
        }
    }
    
    // Now test if we can identify what normal score gets corrupted to -28003
    std::cout << "\nReverse engineering: what normal score becomes -28003?" << std::endl;
    
    // Test a range of scores to see if any get corrupted to the problematic range
    for (int test_score = -5000; test_score <= 5000; test_score += 100) {
        uint64_t test_key = 0x5000000000000000ULL + test_score + 10000;
        tt.store(test_key, test_score, 8, 3, 0x11111111);
        
        int test_retrieved;
        uint8_t td, tf;
        uint32_t tm;
        
        if (tt.probe(test_key, test_retrieved, td, tf, tm)) {
            // Check if this got corrupted into the mate range
            if (test_retrieved < -MATE + 100 && test_score > -MATE + 100) {
                std::cout << "FOUND CORRUPTION: " << test_score << " -> " << test_retrieved << std::endl;
                std::cout << "This would show as: mate -" << ((MATE + test_retrieved + 1) / 2) << std::endl;
            }
        }
    }
    
    return 0;
}
