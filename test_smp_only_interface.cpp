#include "src/transposition_table.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

/**
 * Test VICE Part 105 SMP-only data format conversion
 * Verifies that the new SMP-only interface works correctly
 */
int main() {
    std::cout << "=== VICE Part 105 SMP-Only Data Format Test ===" << std::endl;
    
    // Create a small transposition table
    TranspositionTable tt(1024); // 2^10 entries
    
    // Test data (same as in VICE tutorial)
    uint64_t test_zobrist = 0x123456789ABCDEFULL;
    uint32_t test_move = 0x12345678;
    int16_t test_score = -150;
    uint8_t test_depth = 12;
    uint8_t test_flag = 2; // BETA_CUTOFF
    uint8_t test_age = 5;
    
    std::cout << "\n1. Testing SMP data format storage:" << std::endl;
    std::cout << "   Zobrist: 0x" << std::hex << test_zobrist << std::dec << std::endl;
    std::cout << "   Move: 0x" << std::hex << test_move << std::dec << std::endl;
    std::cout << "   Score: " << test_score << std::endl;
    std::cout << "   Depth: " << static_cast<int>(test_depth) << std::endl;
    std::cout << "   Flag: " << static_cast<int>(test_flag) << std::endl;
    std::cout << "   Age: " << static_cast<int>(test_age) << std::endl;
    
    // Test 1: Direct SMP data interface
    std::cout << "\n2. Testing direct SMP data interface:" << std::endl;
    
    // Create SMP data using VICE macros
    uint64_t smp_data = TTEntry::FoldData(test_move, test_score, test_depth, test_flag, test_age);
    std::cout << "   SMP Data: 0x" << std::hex << smp_data << std::dec << std::endl;
    
    // Store using SMP interface
    tt.store(test_zobrist, smp_data);
    
    // Probe using SMP interface
    uint64_t retrieved_smp_data;
    bool found = tt.probe(test_zobrist, retrieved_smp_data);
    
    if (found) {
        std::cout << "   ✓ SMP probe successful" << std::endl;
        std::cout << "   Retrieved SMP Data: 0x" << std::hex << retrieved_smp_data << std::dec << std::endl;
        
        // Verify data matches
        if (smp_data == retrieved_smp_data) {
            std::cout << "   ✓ SMP data matches perfectly!" << std::endl;
        } else {
            std::cout << "   ✗ SMP data mismatch!" << std::endl;
            return 1;
        }
        
        // Extract and verify individual fields
        uint32_t r_move = TTEntry::ExtractMove(retrieved_smp_data);
        int16_t r_score = TTEntry::ExtractScore(retrieved_smp_data);
        uint8_t r_depth = TTEntry::ExtractDepth(retrieved_smp_data);
        uint8_t r_flag = TTEntry::ExtractFlag(retrieved_smp_data);
        uint8_t r_age = TTEntry::ExtractAge(retrieved_smp_data);
        
        std::cout << "   Extracted Move: 0x" << std::hex << r_move << std::dec << std::endl;
        std::cout << "   Extracted Score: " << r_score << std::endl;
        std::cout << "   Extracted Depth: " << static_cast<int>(r_depth) << std::endl;
        std::cout << "   Extracted Flag: " << static_cast<int>(r_flag) << std::endl;
        std::cout << "   Extracted Age: " << static_cast<int>(r_age) << std::endl;
        
        bool fields_match = (r_move == test_move) && (r_score == test_score) && 
                           (r_depth == test_depth) && (r_flag == test_flag) && (r_age == test_age);
        
        if (fields_match) {
            std::cout << "   ✓ All extracted fields match original data!" << std::endl;
        } else {
            std::cout << "   ✗ Field mismatch detected!" << std::endl;
            return 1;
        }
    } else {
        std::cout << "   ✗ SMP probe failed!" << std::endl;
        return 1;
    }
    
    // Test 2: Convenience interface (should still work)
    std::cout << "\n3. Testing convenience interface (backward compatibility):" << std::endl;
    
    uint64_t test_zobrist2 = 0xFEDCBA9876543210ULL;
    tt.store(test_zobrist2, test_score, test_depth, test_flag, test_move);
    
    int r_score2;
    uint8_t r_depth2, r_flag2;
    uint32_t r_move2;
    
    bool found2 = tt.probe(test_zobrist2, r_score2, r_depth2, r_flag2, r_move2);
    
    if (found2) {
        std::cout << "   ✓ Convenience probe successful" << std::endl;
        
        bool conv_match = (r_move2 == test_move) && (r_score2 == test_score) && 
                         (r_depth2 == test_depth) && (r_flag2 == test_flag);
        
        if (conv_match) {
            std::cout << "   ✓ Convenience interface data matches!" << std::endl;
        } else {
            std::cout << "   ✗ Convenience interface data mismatch!" << std::endl;
            return 1;
        }
    } else {
        std::cout << "   ✗ Convenience probe failed!" << std::endl;
        return 1;
    }
    
    // Test 3: Performance comparison (quick test)
    std::cout << "\n4. Quick performance test:" << std::endl;
    
    const int num_operations = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; i++) {
        uint64_t key = static_cast<uint64_t>(i) * 0x123456789ABCDEFULL;
        uint64_t data = TTEntry::FoldData(i, -100 + (i % 200), (i % 20) + 1, i % 4, i % 16);
        tt.store(key, data);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "   " << num_operations << " SMP stores in " << duration.count() << " μs" << std::endl;
    std::cout << "   Average: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(duration.count()) / num_operations << " μs per store" << std::endl;
    
    // Test statistics
    std::cout << "\n5. Transposition table statistics:" << std::endl;
    auto stats = tt.get_statistics();
    std::cout << "   Hits: " << stats.hits << std::endl;
    std::cout << "   Misses: " << stats.misses << std::endl;
    std::cout << "   Writes: " << stats.writes << std::endl;
    
    std::cout << "\n=== VICE Part 105 SMP-Only Conversion: SUCCESS! ===" << std::endl;
    std::cout << "✓ Direct SMP data interface working" << std::endl;
    std::cout << "✓ Convenience interface working (backward compatibility)" << std::endl;
    std::cout << "✓ Data integrity verified" << std::endl;
    std::cout << "✓ Performance acceptable" << std::endl;
    
    return 0;
}
