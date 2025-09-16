#include <iostream>
#include <vector>
#include <set>
#include <string>

int main() {
    // Working output (48 moves)
    std::vector<std::string> working_moves = {
        "a1b1", "a1c1", "a1d1", "a2a3", "a2a4", "b2b3", "c3a4", "c3b1", "c3b5", "c3d1",
        "d2c1", "d2e3", "d2f4", "d2g5", "d2h6", "d5d6", "d5e6", "e1c1", "e1d1", "e1f1",
        "e1g1", "e2a6", "e2b5", "e2c4", "e2d1", "e2d3", "e2f1", "e5c4", "e5c6", "e5d3",
        "e5d7", "e5f7", "e5g4", "e5g6", "f3d3", "f3e3", "f3f4", "f3f5", "f3f6", "f3g3",
        "f3g4", "f3h3", "f3h5", "g2g3", "g2g4", "g2h3", "h1f1", "h1g1"
    };
    
    // My debug output (46 moves)
    std::vector<std::string> my_moves = {
        "a1b1", "a1c1", "a1d1", "a2a3", "a2a4", "b2b3", "c3a4", "c3b1", "c3b5", "c3d1",
        "d2c1", "d2e3", "d2f4", "d2g5", "d2h6", "d5d6", "d5e6", "e1d1", "e1f1", "e2a6",
        "e2b5", "e2c4", "e2d1", "e2d3", "e2f1", "e5c4", "e5c6", "e5d3", "e5d7", "e5f7",
        "e5g4", "e5g6", "f3d3", "f3e3", "f3f4", "f3f5", "f3f6", "f3g3", "f3g4", "f3h3",
        "f3h5", "g2g3", "g2g4", "g2h3", "h1f1", "h1g1"
    };
    
    std::set<std::string> working_set(working_moves.begin(), working_moves.end());
    std::set<std::string> my_set(my_moves.begin(), my_moves.end());
    
    std::cout << "=== Move Comparison Analysis ===\n\n";
    std::cout << "Working moves: " << working_moves.size() << "\n";
    std::cout << "My moves: " << my_moves.size() << "\n\n";
    
    std::cout << "Moves in WORKING but NOT in MY output:\n";
    for (const auto& move : working_set) {
        if (my_set.find(move) == my_set.end()) {
            std::cout << "  MISSING: " << move << "\n";
        }
    }
    
    std::cout << "\nMoves in MY output but NOT in WORKING:\n";
    for (const auto& move : my_set) {
        if (working_set.find(move) == working_set.end()) {
            std::cout << "  EXTRA: " << move << "\n";
        }
    }
    
    return 0;
}