#include "board120.hpp"
#include <iostream>

int main() {
    std::cout << "Testing file_of and rank_of with offboard handling:\n\n";
    
    // Test playable squares
    std::cout << "Playable squares:\n";
    int a1 = sq(File::A, Rank::R1);
    int h8 = sq(File::H, Rank::R8);
    std::cout << "A1 (sq=" << a1 << "): file=" << static_cast<int>(file_of(a1)) 
              << ", rank=" << static_cast<int>(rank_of(a1)) << "\n";
    std::cout << "H8 (sq=" << h8 << "): file=" << static_cast<int>(file_of(h8)) 
              << ", rank=" << static_cast<int>(rank_of(h8)) << "\n";
    
    // Test offboard squares
    std::cout << "\nOffboard squares:\n";
    int offboard_squares[] = {0, 10, 20, 29, 119};
    for (int sq : offboard_squares) {
        File f = file_of(sq);
        Rank r = rank_of(sq);
        std::cout << "Square " << sq << ": file=" 
                  << (f == File::None ? "None" : std::to_string(static_cast<int>(f)))
                  << ", rank=" 
                  << (r == Rank::None ? "None" : std::to_string(static_cast<int>(r)))
                  << "\n";
    }
    
    return 0;
}
