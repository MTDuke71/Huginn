#include "board120.hpp"
#include <iostream>

int main() {
    std::cout << "Analyzing Rank values and potential conflicts:\n\n";
    
    // Show all rank values
    std::cout << "Rank enum values:\n";
    std::cout << "Rank::R1 = " << static_cast<int>(Rank::R1) << "\n";
    std::cout << "Rank::R2 = " << static_cast<int>(Rank::R2) << "\n";
    std::cout << "Rank::R3 = " << static_cast<int>(Rank::R3) << "\n";
    std::cout << "Rank::R4 = " << static_cast<int>(Rank::R4) << "\n";
    std::cout << "Rank::R5 = " << static_cast<int>(Rank::R5) << "\n";
    std::cout << "Rank::R6 = " << static_cast<int>(Rank::R6) << "\n";
    std::cout << "Rank::R7 = " << static_cast<int>(Rank::R7) << "\n";
    std::cout << "Rank::R8 = " << static_cast<int>(Rank::R8) << "\n";
    std::cout << "Rank::Count = " << static_cast<int>(Rank::Count) << "\n";
    std::cout << "Rank::None = " << static_cast<int>(Rank::None) << "\n\n";
    
    // Test default initialization
    Rank default_rank{};  // This will be 0, which is Rank::R1
    std::cout << "Default-initialized Rank value: " << static_cast<int>(default_rank) << "\n";
    std::cout << "This equals Rank::R1: " << (default_rank == Rank::R1 ? "true" : "false") << "\n";
    std::cout << "This equals Rank::None: " << (default_rank == Rank::None ? "true" : "false") << "\n\n";
    
    // Test comparisons
    std::cout << "Comparison tests:\n";
    std::cout << "Rank::R1 == 0: " << (static_cast<int>(Rank::R1) == 0 ? "true" : "false") << "\n";
    std::cout << "Rank::None == 255: " << (static_cast<int>(Rank::None) == 255 ? "true" : "false") << "\n\n";
    
    // Test potential confusion scenarios
    std::cout << "Potential confusion scenarios:\n";
    
    // Scenario 1: Treating 0 as "invalid"
    auto check_rank_validity = [](Rank r) -> bool {
        // WRONG way - treating 0 as invalid
        // return static_cast<int>(r) != 0;
        
        // CORRECT way - checking against None
        return r != Rank::None;
    };
    
    std::cout << "Is Rank::R1 valid? " << (check_rank_validity(Rank::R1) ? "true" : "false") << "\n";
    std::cout << "Is Rank::None valid? " << (check_rank_validity(Rank::None) ? "true" : "false") << "\n";
    
    return 0;
}
