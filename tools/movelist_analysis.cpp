#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/init.hpp"

int main() {
    try {
        std::cout << "Starting movelist analysis...\n";
        std::cout << "Calling Huginn::init()...\n";
        Huginn::init();
        std::cout << "Initialization complete.\n";
    
    // Test positions with varying complexity
    std::vector<std::string> test_positions = {
        // Starting position
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        
        // Middle game positions
        "rnbqkb1r/pppp1ppp/5n2/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3",
        "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 3 3",
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 4 4",
        "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 6 5",
        
        // Tactical positions with many moves
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", // Perft position
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", // Complex endgame
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", // Many promotions
        
        // Endgame positions
        "8/8/8/8/8/8/6K1/7k w - - 0 1", // Minimal pieces
        "8/8/8/3k4/8/3K4/8/8 w - - 0 1", // King vs King
        "8/8/8/3k4/8/3r4/3K4/8 w - - 0 1", // King and rook vs king
        
        // Promotion-heavy positions
        "8/P1P1P1P1/8/8/8/8/p1p1p1p1/8 w - - 0 1", // Multiple promotions
        "7k/8/8/8/8/8/6PP/6KR w - - 0 1", // Pawn race
        "8/1P6/8/8/8/8/1p6/8 w - - 0 1", // Promotion race
        
        // Positions with many pieces
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
        "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 5 4",
        "rnbqk2r/pppp1ppp/4pn2/8/1bPP4/2N2N2/PP2PPPP/R1BQKB1R b KQkq c3 0 4"
    };
    
    std::vector<int> move_counts;
    std::map<int, int> count_distribution;
    int max_moves = 0;
    
    Position pos;
    S_MOVELIST moves;
    
    std::cout << "Position and movelist created successfully.\n";
    std::cout << "Analyzing move counts across various chess positions...\n\n";
    
    for (size_t i = 0; i < test_positions.size(); ++i) {
        std::cout << "Processing position " << (i + 1) << "...\n";
        moves.clear();
        std::cout << "Setting FEN: " << test_positions[i] << "\n";
        pos.set_from_fen(test_positions[i]);
        std::cout << "Generating moves...\n";
        
        generate_legal_moves_enhanced(pos, moves);
        int move_count = moves.count;
        
        move_counts.push_back(move_count);
        count_distribution[move_count]++;
        
        if (move_count > max_moves) {
            max_moves = move_count;
        }
        
        std::cout << "Position " << (i + 1) << ": " << move_count << " legal moves\n";
        if (move_count > 50) {
            std::cout << "  -> HIGH MOVE COUNT: " << test_positions[i] << "\n";
        }
    }
    
    // Statistical analysis
    std::sort(move_counts.begin(), move_counts.end());
    
    int min_moves = move_counts.front();
    int median_moves = move_counts[move_counts.size() / 2];
    double avg_moves = 0.0;
    for (int count : move_counts) {
        avg_moves += count;
    }
    avg_moves /= move_counts.size();
    
    // Calculate percentiles
    int p90 = move_counts[static_cast<size_t>(move_counts.size() * 0.9)];
    int p95 = move_counts[static_cast<size_t>(move_counts.size() * 0.95)];
    int p99 = move_counts[static_cast<size_t>(move_counts.size() * 0.99)];
    
    std::cout << "\n=== STATISTICAL ANALYSIS ===\n";
    std::cout << "Minimum moves: " << min_moves << "\n";
    std::cout << "Maximum moves: " << max_moves << "\n";
    std::cout << "Average moves: " << avg_moves << "\n";
    std::cout << "Median moves: " << median_moves << "\n";
    std::cout << "90th percentile: " << p90 << "\n";
    std::cout << "95th percentile: " << p95 << "\n";
    std::cout << "99th percentile: " << p99 << "\n";
    
    std::cout << "\n=== MEMORY ANALYSIS ===\n";
    std::cout << "Current S_MOVELIST size: " << (256 * sizeof(S_MOVE)) << " bytes (256 moves)\n";
    std::cout << "S_MOVE size: " << sizeof(S_MOVE) << " bytes\n";
    std::cout << "Typical usage: " << avg_moves << " moves (" << (avg_moves / 256.0 * 100) << "% of array)\n";
    std::cout << "Wasted memory per list: " << ((256 - avg_moves) * sizeof(S_MOVE)) << " bytes\n";
    
    std::cout << "\n=== OPTIMIZATION RECOMMENDATIONS ===\n";
    std::cout << "Recommended array size for 99% coverage: " << p99 << " moves\n";
    std::cout << "Memory savings with " << p99 << "-move array: " << ((256 - p99) * sizeof(S_MOVE)) << " bytes per list\n";
    std::cout << "Memory reduction: " << ((256 - p99) / 256.0 * 100) << "%\n";
    
    if (p95 < 64) {
        std::cout << "\nSuggestion: 64-move array would cover 95% of cases\n";
        std::cout << "Memory savings: " << ((256 - 64) * sizeof(S_MOVE)) << " bytes (" << (192/256.0*100) << "% reduction)\n";
    }
    
    std::cout << "\n=== MOVE COUNT DISTRIBUTION ===\n";
    for (const auto& [count, frequency] : count_distribution) {
        std::cout << count << " moves: " << frequency << " positions\n";
    }
    
    return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}