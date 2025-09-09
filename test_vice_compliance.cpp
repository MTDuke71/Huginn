// Quick test to verify VICE data packing compliance
#include <iostream>
#include <iomanip>
#include <cstdint>

using Move = uint32_t;

// VICE-style data packing functions exactly as implemented in TTEntry
uint64_t pack_data(uint32_t move, int16_t score, uint8_t depth, uint8_t flag, uint8_t age) {
    uint16_t adjusted_score = static_cast<uint16_t>(score + 32768);
    return (static_cast<uint64_t>(move) << 32) |
           (static_cast<uint64_t>(adjusted_score) << 16) |
           (static_cast<uint64_t>(depth) << 8) |
           (static_cast<uint64_t>(flag) << 4) |
           static_cast<uint64_t>(age);
}

// VICE-style macros
uint64_t FoldData(uint32_t move, int16_t score, uint8_t depth, uint8_t flag, uint8_t age) {
    return pack_data(move, score, depth, flag, age);
}

uint32_t ExtractMove(uint64_t data) { return static_cast<uint32_t>(data >> 32); }
int16_t ExtractScore(uint64_t data) { 
    uint16_t adjusted = static_cast<uint16_t>((data >> 16) & 0xFFFF);
    return static_cast<int16_t>(adjusted - 32768);
}
uint8_t ExtractDepth(uint64_t data) { return static_cast<uint8_t>((data >> 8) & 0xFF); }
uint8_t ExtractFlag(uint64_t data) { return static_cast<uint8_t>((data >> 4) & 0xF); }
uint8_t ExtractAge(uint64_t data) { return static_cast<uint8_t>(data & 0xF); }

int main() {
    std::cout << "\n=== VICE Data Packing Compliance Test ===" << std::endl;
    
    // Test various data combinations
    Move test_move = 0x12345678;  // 32-bit move
    int16_t test_score = -1234;   // Negative score
    uint8_t test_depth = 15;      // 8-bit depth
    uint8_t test_flag = 3;        // 4-bit flag (EXACT)
    uint8_t test_age = 7;         // 4-bit age
    
    // Pack the data
    uint64_t packed = pack_data(test_move, test_score, test_depth, test_flag, test_age);
    
    std::cout << "Original Data:" << std::endl;
    std::cout << "  Move: 0x" << std::hex << test_move << std::dec << std::endl;
    std::cout << "  Score: " << test_score << std::endl;
    std::cout << "  Depth: " << static_cast<int>(test_depth) << std::endl;
    std::cout << "  Flag: " << static_cast<int>(test_flag) << std::endl;
    std::cout << "  Age: " << static_cast<int>(test_age) << std::endl;
    
    std::cout << "\nPacked Data: 0x" << std::hex << packed << std::dec << std::endl;
    
    // Test VICE macros
    std::cout << "\nVICE Macro Extraction:" << std::endl;
    std::cout << "  ExtractMove: 0x" << std::hex << ExtractMove(packed) << std::dec << std::endl;
    std::cout << "  ExtractScore: " << ExtractScore(packed) << std::endl;
    std::cout << "  ExtractDepth: " << static_cast<int>(ExtractDepth(packed)) << std::endl;
    std::cout << "  ExtractFlag: " << static_cast<int>(ExtractFlag(packed)) << std::endl;
    std::cout << "  ExtractAge: " << static_cast<int>(ExtractAge(packed)) << std::endl;
    
    // Verify data integrity
    bool all_correct = true;
    if (ExtractMove(packed) != test_move) {
        std::cout << "ERROR: Move mismatch!" << std::endl;
        all_correct = false;
    }
    if (ExtractScore(packed) != test_score) {
        std::cout << "ERROR: Score mismatch!" << std::endl;
        all_correct = false;
    }
    if (ExtractDepth(packed) != test_depth) {
        std::cout << "ERROR: Depth mismatch!" << std::endl;
        all_correct = false;
    }
    if (ExtractFlag(packed) != test_flag) {
        std::cout << "ERROR: Flag mismatch!" << std::endl;
        all_correct = false;
    }
    if (ExtractAge(packed) != test_age) {
        std::cout << "ERROR: Age mismatch!" << std::endl;
        all_correct = false;
    }
    
    if (all_correct) {
        std::cout << "\n✓ All data packing/unpacking tests PASSED!" << std::endl;
    } else {
        std::cout << "\n✗ Data packing/unpacking tests FAILED!" << std::endl;
    }
    
    // Test FoldData macro
    std::cout << "\nTesting FoldData macro:" << std::endl;
    uint64_t folded = FoldData(test_move, test_score, test_depth, test_flag, test_age);
    std::cout << "  FoldData result: 0x" << std::hex << folded << std::dec << std::endl;
    std::cout << "  pack_data result: 0x" << std::hex << packed << std::dec << std::endl;
    
    if (folded == packed) {
        std::cout << "✓ FoldData macro matches pack_data function!" << std::endl;
    } else {
        std::cout << "✗ FoldData macro differs from pack_data function!" << std::endl;
    }
    
    std::cout << "=== VICE Compliance Test Complete ===\n" << std::endl;
    
    return 0;
}
