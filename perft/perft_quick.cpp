#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "bitboard_position.hpp"
#include "bitboard_perft_optimized.hpp"
#include "init.hpp"
#include "epd_parser.hpp"

// Function to run a single perft test on a given position
void run_perft_test(const TestPosition& pos_info) {
    std::cout << "\n[Position] FEN: " << pos_info.fen << std::endl;

    BitboardPosition pos;
    pos.set_from_fen(pos_info.fen);

    bool position_passed = true;
    for (const auto& perft_pair : pos_info.perft_results) {
        int depth = perft_pair.first;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t result = BitboardPerftOptimized::perft_fast(pos, depth);
        auto end_time = std::chrono::high_resolution_clock::now();
        long long duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        uint64_t expected_nodes = perft_pair.second;

        std::cout << "  Depth " << depth << ": " << result << " nodes (" << duration_ms << "ms) ";
        if (result == expected_nodes) {
            std::cout << "-> PASS" << std::endl;
        } else {
            std::cout << "-> FAIL (Expected: " << expected_nodes << ")" << std::endl;
            position_passed = false;
        }
    }
}

int main(int argc, char* argv[]) {
    Huginn::init(); 

    std::string epd_file = "test/perftsuite.epd";
    std::vector<TestPosition> test_positions = parse_epd_file(epd_file);

    if (test_positions.empty()) {
        std::cerr << "No test positions found in " << epd_file << ". Exiting." << std::endl;
        return 1;
    }

    std::cout << "--- Running Fast Perft Test Suite ---" << std::endl;

    for (const auto& test_pos : test_positions) {
        run_perft_test(test_pos);
    }

    std::cout << "\n--- Test Suite Finished ---" << std::endl;

    return 0;
}
