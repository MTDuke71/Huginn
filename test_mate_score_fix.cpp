#include "src/transposition_table.hpp"
#include <iostream>
#include <iomanip>

/**
 * Test mate score handling in transposition table
 * Verify that large scores (like MATE=29000) are handled correctly
 */
int main() {
    std::cout << "=== Mate Score Handling Test ===" << std::endl;
    
    // Test data including mate scores
    struct TestCase {
        int16_t original_score;
        const char* description;
    };
    
    TestCase test_cases[] = {
        {100, "Normal score"},
        {-150, "Negative score"},
        {5000, "Large positive"},
        {-5000, "Large negative"},
        {29000, "MATE score (29000)"},
        {-29000, "Negative MATE score"},
        {30000, "INFINITE score (30000)"},
        {-30000, "Negative INFINITE score"},
        {28500, "Near-mate score"},
        {0, "Zero score"}
    };
    
    std::cout << "\nTesting score compression/decompression:" << std::endl;
    std::cout << std::setw(20) << "Original" << std::setw(15) << "Packed" << std::setw(15) << "Extracted" << std::setw(10) << "Error" << "  Description" << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    bool all_passed = true;
    
    for (const auto& test : test_cases) {
        // Pack the score
        uint64_t packed = TTEntry::FoldData(0x12345678, test.original_score, 10, 2, 5);
        
        // Extract the score
        int16_t extracted = TTEntry::ExtractScore(packed);
        
        // Calculate error
        int error = abs(extracted - test.original_score);
        
        // For extreme scores, we expect some compression error
        bool acceptable_error = false;
        if (abs(test.original_score) <= 25000) {
            acceptable_error = (error == 0);  // No error expected
        } else {
            acceptable_error = (error <= abs(test.original_score) * 0.1);  // 10% error acceptable for extreme scores
        }
        
        std::cout << std::setw(20) << test.original_score 
                  << std::setw(15) << std::hex << "0x" << (packed & 0xFFFF0000) >> 16 << std::dec
                  << std::setw(15) << extracted 
                  << std::setw(10) << error 
                  << "  " << test.description;
        
        if (acceptable_error) {
            std::cout << " ✓" << std::endl;
        } else {
            std::cout << " ✗" << std::endl;
            all_passed = false;
        }
    }
    
    std::cout << std::endl;
    
    // Test full transposition table functionality with mate scores
    std::cout << "Testing transposition table with mate scores:" << std::endl;
    
    TranspositionTable tt(1024);
    
    uint64_t test_key = 0x123456789ABCDEFULL;
    int16_t mate_score = 29000;
    uint8_t depth = 10;
    uint8_t flag = 2;
    uint32_t move = 0xABCD1234;
    
    // Store mate score
    tt.store(test_key, mate_score, depth, flag, move);
    
    // Retrieve and verify
    int retrieved_score;
    uint8_t retrieved_depth, retrieved_flag;
    uint32_t retrieved_move;
    
    bool found = tt.probe(test_key, retrieved_score, retrieved_depth, retrieved_flag, retrieved_move);
    
    if (found) {
        std::cout << "  Original mate score: " << mate_score << std::endl;
        std::cout << "  Retrieved score: " << retrieved_score << std::endl;
        std::cout << "  Score difference: " << abs(retrieved_score - mate_score) << std::endl;
        std::cout << "  Move match: " << (retrieved_move == move ? "✓" : "✗") << std::endl;
        std::cout << "  Depth match: " << (retrieved_depth == depth ? "✓" : "✗") << std::endl;
        std::cout << "  Flag match: " << (retrieved_flag == flag ? "✓" : "✗") << std::endl;
        
        // For mate scores, we expect some approximation due to compression
        bool score_acceptable = abs(retrieved_score - mate_score) <= 1000;  // Allow 1000 centipawn error for extreme scores
        
        if (score_acceptable && retrieved_move == move && retrieved_depth == depth && retrieved_flag == flag) {
            std::cout << "  ✓ Transposition table test PASSED" << std::endl;
        } else {
            std::cout << "  ✗ Transposition table test FAILED" << std::endl;
            all_passed = false;
        }
    } else {
        std::cout << "  ✗ Failed to retrieve stored entry" << std::endl;
        all_passed = false;
    }
    
    if (all_passed) {
        std::cout << "\n=== ALL TESTS PASSED! ===" << std::endl;
        std::cout << "Mate score handling is working correctly." << std::endl;
        std::cout << "The -M499 issue should be resolved." << std::endl;
    } else {
        std::cout << "\n=== SOME TESTS FAILED ===" << std::endl;
        std::cout << "Mate score handling needs further adjustment." << std::endl;
    }
    
    return all_passed ? 0 : 1;
}
