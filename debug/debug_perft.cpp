#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp" 
#include "init.hpp"

static uint64_t perft_debug(Position& pos, int depth, int level = 0) {
    std::string indent(level * 2, ' ');
    std::cout << indent << "perft_debug(depth=" << depth << ")" << std::endl;
    
    if (depth == 0) return 1;
    
    MoveList list; 
    std::cout << indent << "Generating legal moves..." << std::endl;
    generate_legal_moves(pos, list);
    std::cout << indent << "Generated " << list.v.size() << " legal moves" << std::endl;
    
    uint64_t nodes = 0;
    for (size_t i = 0; i < list.v.size(); ++i) {
        const auto& m = list.v[i];
        std::cout << indent << "Making move " << (i+1) << "/" << list.v.size() << std::endl;
        
        try {
            pos.make_move_with_undo(m);
            nodes += perft_debug(pos, depth-1, level + 1);
            pos.undo_move();
        } catch (const std::exception& e) {
            std::cout << indent << "ERROR on move " << (i+1) << ": " << e.what() << std::endl;
            throw;
        }
    }
    return nodes;
}

int main() {
    Huginn::init(); // Initialize engine
    
    Position pos; 
    pos.set_startpos();
    
    std::cout << "Testing perft depth 3..." << std::endl;
    try {
        uint64_t result = perft_debug(pos, 3);
        std::cout << "Perft depth 3 result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
