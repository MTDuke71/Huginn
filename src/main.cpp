#include "uci.hpp"
#include "bitboard_benchmark.hpp"
#include "position.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        std::cout << "=== Huginn 2.0 Bitboard Benchmark Mode ===" << std::endl;
        Position pos;
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        BitboardBenchmark::run_comprehensive_benchmark(pos);
        return 0;
    }

    UCIInterface uci;
    uci.run();
    return 0;
}
