#include <iostream>
#include <vector>
#include <string>
#include <map>

int main() {
    std::cout << "=== Expected Moves Analysis for a2a4 in Kiwipete ===" << std::endl;
    std::cout << "Position after a2a4: rnbqkbnr/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1" << std::endl;
    
    // The expected 44 moves after a2a4
    std::vector<std::string> expected_moves = {
        "a6a5", "a7a5", "a7a6",                                    // 3 moves
        "b4b3", "b6a4", "b6a8", "b6c4", "b6c8", "b6d4", "b6d7", "b6d8",  // 8 moves  
        "c7c5", "c7c6",                                           // 2 moves
        "d7d5", "d7d6",                                           // 2 moves
        "e6e5",                                                   // 1 move
        "e7a3", "e7b4", "e7c5", "e7d6", "e7d8", "e7f6", "e7f8", // 7 moves
        "e8c8", "e8d8", "e8f8", "e8g8",                         // 4 moves  
        "f6d5", "f6e4", "f6g4", "f6g8", "f6h5", "f6h7",         // 6 moves
        "g6g5",                                                   // 1 move
        "g7f8", "g7h6",                                           // 2 moves
        "h3g2",                                                   // 1 move
        "h8f8", "h8g8", "h8h4", "h8h5", "h8h6", "h8h7"          // 6 moves
    };                                                           // Total: 44 moves
    
    std::cout << "\nTotal expected moves: " << expected_moves.size() << std::endl;
    
    // Group moves by piece type based on board position
    std::map<std::string, std::vector<std::string>> groups;
    
    for (const auto& move : expected_moves) {
        std::string from = move.substr(0, 2);
        groups[from].push_back(move.substr(2, 2));
    }
    
    std::cout << "\n=== MOVES BY PIECE ===" << std::endl;
    
    // Analyze each group
    for (const auto& group : groups) {
        std::string square = group.first;
        const auto& destinations = group.second;
        
        std::cout << square << " (" << destinations.size() << " moves): ";
        for (size_t i = 0; i < destinations.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << destinations[i];
        }
        std::cout << std::endl;
        
        // Identify piece type based on move pattern
        if (square == "a6" || square == "a7" || square == "b4" || square == "c7" || square == "d7" || square == "e6" || square == "g6" || square == "h3") {
            std::cout << "  -> PAWN moves" << std::endl;
        } else if (square == "b6" || square == "f6") {
            std::cout << "  -> KNIGHT moves" << std::endl;
        } else if (square == "e7" || square == "g7") {
            std::cout << "  -> BISHOP moves" << std::endl;
        } else if (square == "h8") {
            std::cout << "  -> ROOK moves" << std::endl;
        } else if (square == "e8") {
            std::cout << "  -> KING moves (including castling)" << std::endl;
        }
        std::cout << std::endl;
    }
    
    std::cout << "=== SPECIAL MOVES ===" << std::endl;
    std::cout << "Castling moves: e8c8 (queenside), e8g8 (kingside)" << std::endl;
    std::cout << "En passant capture: b4b3 (captures the a4 pawn)" << std::endl;
    
    std::cout << "\n=== ANALYSIS COMPLETE ===" << std::endl;
    std::cout << "Your engine generates 43 moves instead of 44." << std::endl;
    std::cout << "Compare your engine's output with the groups above to find the missing move." << std::endl;
    
    return 0;
}
