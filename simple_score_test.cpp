#include "src/transposition_table.hpp"
#include <iostream>

int main() {
    std::cout << "=== Simple Score Test ===" << std::endl;
    
    // Test the exact problematic case
    int original_score = 29000;  // MATE score
    std::cout << "Original score (int): " << original_score << std::endl;
    
    // Test casting to int16_t
    int16_t score16 = static_cast<int16_t>(original_score);
    std::cout << "Casted to int16_t: " << score16 << std::endl;
    
    // Test the FoldData function directly
    uint64_t folded = TTEntry::FoldData(0x12345678, score16, 10, 2, 5);
    std::cout << "Folded data: 0x" << std::hex << folded << std::dec << std::endl;
    
    // Test ExtractScore
    int16_t extracted16 = TTEntry::ExtractScore(folded);
    std::cout << "Extracted (int16_t): " << extracted16 << std::endl;
    
    // Test assignment to int
    int extracted_int = extracted16;
    std::cout << "Assigned to int: " << extracted_int << std::endl;
    
    // Test the full transposition table
    std::cout << "\nTesting full transposition table:" << std::endl;
    
    TranspositionTable tt(1024);
    uint64_t key = 0x123456789ABCDEFULL;
    
    // Store with convenience function
    tt.store(key, original_score, 10, 2, 0x12345678);
    
    // Probe with convenience function
    int retrieved_score;
    uint8_t depth, flag;
    uint32_t move;
    
    bool found = tt.probe(key, retrieved_score, depth, flag, move);
    
    if (found) {
        std::cout << "Retrieved score: " << retrieved_score << std::endl;
        std::cout << "Score difference: " << (retrieved_score - original_score) << std::endl;
        
        if (retrieved_score == original_score) {
            std::cout << "✓ Perfect match!" << std::endl;
        } else {
            std::cout << "✗ Mismatch detected!" << std::endl;
        }
    } else {
        std::cout << "✗ Failed to retrieve entry" << std::endl;
    }
    
    // Test negative mate score
    std::cout << "\nTesting negative mate score:" << std::endl;
    
    int neg_mate = -28003;  // Should give about -M499
    std::cout << "Negative mate score: " << neg_mate << std::endl;
    
    uint64_t key2 = 0xABCDEF123456789ULL;
    tt.store(key2, neg_mate, 12, 1, 0x87654321);
    
    int retrieved_neg;
    uint8_t d2, f2;
    uint32_t m2;
    
    if (tt.probe(key2, retrieved_neg, d2, f2, m2)) {
        std::cout << "Retrieved negative: " << retrieved_neg << std::endl;
        std::cout << "Difference: " << (retrieved_neg - neg_mate) << std::endl;
        
        // Calculate what UCI output would be
        const int MATE = 29000;
        if (retrieved_neg < -MATE + 100) {
            int mate_in_plies = MATE + retrieved_neg;
            int mate_in_moves = (mate_in_plies + 1) / 2;
            std::cout << "UCI would show: mate -" << mate_in_moves << std::endl;
        }
    }
    
    return 0;
}
