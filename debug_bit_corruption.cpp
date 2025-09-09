#include <iostream>
#include <iomanip>
#include <cstdint>

// Reproduce the pack/unpack logic to debug
uint64_t debug_pack_data(uint32_t best_move, int16_t score, uint8_t depth, uint8_t node_type, uint8_t age) {
    std::cout << "=== PACK DEBUG ===" << std::endl;
    std::cout << "Input - Move: 0x" << std::hex << best_move << std::dec 
              << ", Score: " << score 
              << ", Depth: " << (int)depth 
              << ", Type: " << (int)node_type 
              << ", Age: " << (int)age << std::endl;
    
    uint16_t adjusted_score = static_cast<uint16_t>(score + 32768);
    std::cout << "Adjusted score: " << adjusted_score << " (0x" << std::hex << adjusted_score << std::dec << ")" << std::endl;
    
    uint64_t result = (static_cast<uint64_t>(best_move) << 32) |
                      (static_cast<uint64_t>(adjusted_score) << 16) |
                      (static_cast<uint64_t>(depth) << 8) |
                      (static_cast<uint64_t>(node_type) << 4) |
                      (static_cast<uint64_t>(age));
    
    std::cout << "Packed result: 0x" << std::hex << result << std::dec << std::endl;
    return result;
}

void debug_unpack_data(uint64_t packed) {
    std::cout << "\n=== UNPACK DEBUG ===" << std::endl;
    std::cout << "Packed data: 0x" << std::hex << packed << std::dec << std::endl;
    
    uint32_t best_move = static_cast<uint32_t>(packed >> 32);
    uint16_t adjusted_score = static_cast<uint16_t>((packed >> 16) & 0xFFFF);
    int16_t score = static_cast<int16_t>(adjusted_score - 32768);
    uint8_t depth = static_cast<uint8_t>((packed >> 8) & 0xFF);
    uint8_t node_type = static_cast<uint8_t>((packed >> 4) & 0xF);
    uint8_t age = static_cast<uint8_t>(packed & 0xF);
    
    std::cout << "Extracted - Move: 0x" << std::hex << best_move << std::dec 
              << ", Adjusted: " << adjusted_score 
              << ", Score: " << score 
              << ", Depth: " << (int)depth 
              << ", Type: " << (int)node_type 
              << ", Age: " << (int)age << std::endl;
}

int main() {
    std::cout << "=== Bit Manipulation Debug ===" << std::endl;
    
    // Test with a normal score that might get corrupted
    struct TestCase {
        int16_t score;
        const char* desc;
    };
    
    TestCase tests[] = {
        {150, "Normal positive"},
        {-150, "Normal negative"},
        {0, "Zero"},
        {32767, "Max int16_t"},
        {-32768, "Min int16_t"},
        {29000, "MATE score"},
        {-28000, "Large negative"}
    };
    
    for (const auto& test : tests) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Testing: " << test.desc << " (score = " << test.score << ")" << std::endl;
        
        uint64_t packed = debug_pack_data(0x12345678, test.score, 10, 2, 5);
        debug_unpack_data(packed);
        
        // Verify round-trip
        uint16_t adjusted = static_cast<uint16_t>((packed >> 16) & 0xFFFF);
        int16_t recovered = static_cast<int16_t>(adjusted - 32768);
        
        if (recovered == test.score) {
            std::cout << "✓ Round-trip successful" << std::endl;
        } else {
            std::cout << "✗ Round-trip FAILED! " << test.score << " != " << recovered << std::endl;
        }
    }
    
    // Now test the specific case of score corruption
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Testing potential corruption scenarios:" << std::endl;
    
    // What if there's memory corruption or race condition?
    // Let's see what happens if bits get flipped
    
    int16_t normal_score = 150;  // Normal position score
    uint64_t normal_packed = debug_pack_data(0x12345678, normal_score, 10, 2, 5);
    
    std::cout << "\nTesting bit corruption scenarios:" << std::endl;
    
    // Flip some bits in the score portion
    for (int bit = 16; bit < 32; bit++) {
        uint64_t corrupted = normal_packed ^ (1ULL << bit);
        std::cout << "\nBit " << bit << " flipped:" << std::endl;
        debug_unpack_data(corrupted);
        
        int16_t corrupted_score = static_cast<int16_t>(((corrupted >> 16) & 0xFFFF) - 32768);
        
        // Check if this falls into mate range
        const int MATE = 29000;
        if (corrupted_score < -MATE + 100) {
            std::cout << "*** THIS WOULD SHOW AS MATE! ***" << std::endl;
            int mate_in_plies = MATE + corrupted_score;
            int mate_in_moves = (mate_in_plies + 1) / 2;
            std::cout << "UCI: mate -" << mate_in_moves << std::endl;
        }
    }
    
    return 0;
}
