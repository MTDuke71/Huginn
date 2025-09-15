#include "src/syzygy_tablebase.hpp"
#include "src/position.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    // Initialize position system
    init_all();
    
    // Create tablebase
    SyzygyTablebase tb;
    if (!tb.initialize("d:\\TB\\")) {
        std::cout << "Failed to initialize tablebase" << std::endl;
        return 1;
    }
    
    std::cout << "Tablebase info: " << tb.get_info() << std::endl;
    
    // Test position: KQ vs K
    Position pos;
    if (!pos.ParseFen("8/8/8/8/8/3k4/8/4KQ2 w - - 0 1")) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    
    std::cout << "Position parsed successfully" << std::endl;
    std::cout << "Can probe: " << (tb.can_probe(pos) ? "YES" : "NO") << std::endl;
    
    if (tb.can_probe(pos)) {
        int wdl_result = tb.probe_wdl(pos);
        std::cout << "WDL result: " << wdl_result << std::endl;
        
        if (wdl_result != INT32_MAX) {
            std::cout << "Tablebase probe successful!" << std::endl;
        } else {
            std::cout << "Tablebase probe failed" << std::endl;
        }
    }
    
    return 0;
}