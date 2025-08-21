#include <iostream>
#include "position.hpp"
#include "movegen.hpp" 
#include "init.hpp"

static uint64_t perft_debug_simple(Position& pos, int depth, int max_moves = 5) {
    if (depth == 0) return 1;
    
    MoveList list; 
    generate_legal_moves(pos, list);
    
    uint64_t nodes = 0;
    size_t move_count = std::min(size_t(max_moves), list.v.size());
    
    for (size_t i = 0; i < move_count; ++i) {
        const auto& m = list.v[i];
        std::cout << "Move " << (i+1) << "/" << move_count << " at depth " << depth << std::endl;
        
        State st{};
        try {
            make_move(pos, m, st);
            nodes += perft_debug_simple(pos, depth-1, max_moves);
            unmake_move(pos, m, st);
        } catch (const std::exception& e) {
            std::cout << "ERROR on move " << (i+1) << ": " << e.what() << std::endl;
            // Let's debug the position state
            std::cout << "Position state when error occurred:" << std::endl;
            std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
            
            // Check piece counts
            for (int color = 0; color < 2; ++color) {
                std::cout << "Color " << color << " piece counts:" << std::endl;
                for (int type = 0; type < 7; ++type) {
                    int count = pos.pCount[color][type];
                    std::cout << "  Type " << type << ": " << count << std::endl;
                    
                    if (count > 0 && count <= 10) {
                        std::cout << "    Pieces at: ";
                        for (int idx = 0; idx < count; ++idx) {
                            int square = pos.pList[color][type][idx];
                            std::cout << square << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
            
            throw;
        }
    }
    return nodes;
}

int main() {
    Huginn::init(); // Initialize engine
    
    Position pos; 
    pos.set_startpos();
    
    std::cout << "Testing perft depth 3 with limited moves..." << std::endl;
    try {
        uint64_t result = perft_debug_simple(pos, 3, 6); // Only test first 6 moves
        std::cout << "Perft depth 3 result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
