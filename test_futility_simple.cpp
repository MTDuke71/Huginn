#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include "src/init.hpp"
#include <iostream>
#include <chrono>

int main() {
    AllInit();
    
    S_BOARD pos;
    
    // Test a position where futility pruning should be active
    ParseFen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", &pos);
    
    S_SEARCHINFO info;
    info.depth = 3;
    info.starttime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    info.stoptime = info.starttime + 5000; // 5 seconds
    info.stopped = 0;
    info.quit = 0;
    info.movestogo = 30;
    info.infinite = 0;
    info.nodes = 0;
    info.fh = 0.0f;
    info.fhf = 0.0f;
    info.nullcut = 0;
    info.futility_cuts = 0;
    
    // Search with futility pruning
    int score = AlphaBeta(-INFINITE, INFINITE, info.depth, &pos, &info, 1);
    
    std::cout << "Search completed with score: " << score << std::endl;
    std::cout << "Total nodes: " << info.nodes << std::endl;
    std::cout << "Futility cuts: " << info.futility_cuts << std::endl;
    std::cout << "Null move cuts: " << info.nullcut << std::endl;
    
    if (info.futility_cuts > 0) {
        std::cout << "SUCCESS: Futility pruning is working! (" << info.futility_cuts << " cuts)" << std::endl;
        double futility_ratio = (double)info.futility_cuts / info.nodes * 100.0;
        std::cout << "Futility pruning rate: " << futility_ratio << "%" << std::endl;
    } else {
        std::cout << "WARNING: No futility cuts detected in this position" << std::endl;
    }
    
    return 0;
}