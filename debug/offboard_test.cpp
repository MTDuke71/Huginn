#include "board120.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Square::Offboard integration:\n\n";
    
    // Test Square::Offboard constant
    std::cout << "Square::Offboard value: " << int(Square::Offboard) << "\n\n";
    
    // Test functions with Square::Offboard
    int offboard_sq = int(Square::Offboard);
    
    std::cout << "Testing functions with Square::Offboard (-1):\n";
    std::cout << "file_of(-1): " << (file_of(offboard_sq) == File::None ? "File::None" : "Valid") << "\n";
    std::cout << "rank_of(-1): " << (rank_of(offboard_sq) == Rank::None ? "Rank::None" : "Valid") << "\n";
    std::cout << "is_playable(-1): " << (is_playable(offboard_sq) ? "true" : "false") << "\n";
    std::cout << "is_offboard(-1): " << (is_offboard(offboard_sq) ? "true" : "false") << "\n\n";
    
    // Test with other invalid squares
    std::cout << "Testing with other invalid squares:\n";
    int test_squares[] = {-5, 0, 10, 20, 29, 119, 150};
    
    for (int sq : test_squares) {
        std::cout << "Square " << sq << ": ";
        std::cout << (is_playable(sq) ? "playable" : "offboard") << "\n";
    }
    
    std::cout << "\nTesting from_algebraic with invalid input:\n";
    std::cout << "from_algebraic(\"z9\"): " << from_algebraic("z9") << " (should be " << int(Square::Offboard) << ")\n";
    std::cout << "from_algebraic(\"\"): " << from_algebraic("") << " (should be " << int(Square::Offboard) << ")\n";
    
    return 0;
}
