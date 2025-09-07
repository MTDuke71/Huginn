#include <iostream>
#include "src/position.hpp"
#include "src/minimal_search.hpp"

using namespace Huginn;

int main() {
    MinimalEngine engine;
    
    std::cout << "=== Material Draw Test ===" << std::endl;
    
    // Test various material draw scenarios
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"8/8/8/8/8/8/8/K6k w - - 0 1", "King vs King"},
        {"8/8/8/8/8/8/8/KN5k w - - 0 1", "King+Knight vs King"},
        {"8/8/8/8/8/8/8/KB5k w - - 0 1", "King+Bishop vs King"},
        {"8/8/8/8/8/8/8/KN4nk w - - 0 1", "King+Knight vs King+Knight"},
        {"8/8/8/8/8/8/8/KB4bk w - - 0 1", "King+Bishop vs King+Bishop"},
        {"8/8/8/8/8/8/8/KBN3bk w - - 0 1", "King+Bishop+Knight vs King+Bishop"},
        {"8/8/8/8/8/8/8/KR5k w - - 0 1", "King+Rook vs King (not a draw)"},
        {"8/8/8/8/8/8/8/KQ5k w - - 0 1", "King+Queen vs King (not a draw)"}
    };
    
    for (const auto& test : test_cases) {
        Position pos;
        pos.set_from_fen(test.first);
        
        bool is_draw = engine.MaterialDraw(pos);
        int eval = engine.evaluate(pos);
        
        std::cout << test.second << ": " << (is_draw ? "DRAW" : "NOT DRAW") << " (eval: " << eval << " cp)" << std::endl;
    }
    
    return 0;
}
