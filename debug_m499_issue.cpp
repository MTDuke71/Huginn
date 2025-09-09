#include "src/transposition_table.hpp"
#include "src/minimal_search.hpp"
#include <iostream>

int main() {
    std::cout << "=== Debugging -M499 Issue ===" << std::endl;
    
    const int MATE = 29000;
    
    // Test what score would produce -M499
    // If mate_in_moves = 499, then mate_in_plies = 499 * 2 - 1 = 997
    // If mate_in_plies = 997, then score = -(MATE - 997) = -(29000 - 997) = -28003
    
    int problematic_score = -28003;  // This should give about -M499
    
    std::cout << "Testing score that should give -M499:" << std::endl;
    std::cout << "Score: " << problematic_score << std::endl;
    
    // Test the UCI formatting directly
    if (problematic_score < -MATE + 100) {
        int mate_in_plies = MATE + problematic_score;
        int mate_in_moves = (mate_in_plies + 1) / 2;
        std::cout << "Mate in plies: " << mate_in_plies << std::endl;
        std::cout << "Mate in moves: " << mate_in_moves << std::endl;
        std::cout << "UCI output: mate -" << mate_in_moves << std::endl;
    }
    
    std::cout << std::endl;
    
    // Now test transposition table storage/retrieval
    std::cout << "Testing transposition table with problematic score:" << std::endl;
    
    TranspositionTable tt(1024);
    uint64_t test_key = 0x123456789ABCDEFULL;
    
    // Store the problematic score
    tt.store(test_key, problematic_score, 10, 2, 0x12345678);
    
    // Retrieve it
    int retrieved_score;
    uint8_t depth, flag;
    uint32_t move;
    
    bool found = tt.probe(test_key, retrieved_score, depth, flag, move);
    
    if (found) {
        std::cout << "Original score: " << problematic_score << std::endl;
        std::cout << "Retrieved score: " << retrieved_score << std::endl;
        std::cout << "Difference: " << (retrieved_score - problematic_score) << std::endl;
        
        if (retrieved_score == problematic_score) {
            std::cout << "✓ Transposition table preserved the score correctly" << std::endl;
        } else {
            std::cout << "✗ Transposition table corrupted the score!" << std::endl;
            std::cout << "This is likely the source of the -M499 issue." << std::endl;
        }
        
        // Test UCI formatting of retrieved score
        if (retrieved_score < -MATE + 100) {
            int mate_in_plies = MATE + retrieved_score;
            int mate_in_moves = (mate_in_plies + 1) / 2;
            std::cout << "Retrieved score UCI: mate -" << mate_in_moves << std::endl;
        }
        
    } else {
        std::cout << "✗ Failed to retrieve score from transposition table" << std::endl;
    }
    
    // Test a range of mate scores
    std::cout << "\nTesting range of mate scores:" << std::endl;
    
    int test_scores[] = {-29000, -28500, -28000, -27000, -26000, -25000};
    
    for (int score : test_scores) {
        uint64_t key = static_cast<uint64_t>(score) + 0x123456789ABCDEFULL;
        tt.store(key, score, 10, 2, 0);
        
        int retrieved;
        uint8_t d, f;
        uint32_t m;
        
        if (tt.probe(key, retrieved, d, f, m)) {
            int diff = retrieved - score;
            std::cout << "Score " << score << " -> " << retrieved << " (diff: " << diff << ")";
            
            if (diff != 0) {
                std::cout << " ✗ CORRUPTED";
            } else {
                std::cout << " ✓ OK";
            }
            std::cout << std::endl;
        }
    }
    
    return 0;
}
