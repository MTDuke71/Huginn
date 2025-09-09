// Quick verification of the mate score issue

#include <iostream>
#include <cstdint>

int main() {
    std::cout << "=== Score Range Analysis ===" << std::endl;
    
    // Test the original encoding
    int16_t mate_score = 29000;
    std::cout << "MATE score: " << mate_score << std::endl;
    
    // Original encoding with +32768 offset
    uint16_t adjusted = static_cast<uint16_t>(mate_score + 32768);
    std::cout << "Adjusted (mate + 32768): " << adjusted << std::endl;
    
    // Decode back
    int16_t decoded = static_cast<int16_t>(adjusted - 32768);
    std::cout << "Decoded: " << decoded << std::endl;
    
    // Check if it matches
    std::cout << "Match: " << (decoded == mate_score ? "YES" : "NO") << std::endl;
    
    // Check ranges
    std::cout << "\nRange analysis:" << std::endl;
    std::cout << "int16_t min: " << INT16_MIN << std::endl;
    std::cout << "int16_t max: " << INT16_MAX << std::endl;
    std::cout << "uint16_t max: " << UINT16_MAX << std::endl;
    
    // Test edge cases
    std::cout << "\nEdge case tests:" << std::endl;
    
    int16_t test_scores[] = {-32768, -30000, -29000, 0, 29000, 30000, 32767};
    
    for (int16_t score : test_scores) {
        uint16_t adj = static_cast<uint16_t>(score + 32768);
        int16_t dec = static_cast<int16_t>(adj - 32768);
        std::cout << "Score: " << score << " -> Adjusted: " << adj << " -> Decoded: " << dec << " (Match: " << (dec == score ? "YES" : "NO") << ")" << std::endl;
    }
    
    return 0;
}
