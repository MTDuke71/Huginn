#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert function
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.is_promotion;
    simple_move.promotion_type = move.promotion_type;
    return simple_move;
}

// Perft function with legal checking
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

// Format large numbers with commas
std::string format_number(uint64_t num) {
    std::string str = std::to_string(num);
    int insertPosition = str.length() - 3;
    while (insertPosition > 0) {
        str.insert(insertPosition, ",");
        insertPosition -= 3;
    }
    return str;
}

// Format NPS with appropriate units
std::string format_nps(double nps) {
    if (nps >= 1000000000) {
        return std::to_string(int(nps / 1000000000)) + "B nps";
    } else if (nps >= 1000000) {
        return std::to_string(int(nps / 1000000)) + "M nps";
    } else if (nps >= 1000) {
        return std::to_string(int(nps / 1000)) + "K nps";
    } else {
        return std::to_string(int(nps)) + " nps";
    }
}

int main() {
    std::cout << "=== Huginn Perft 6 Timing Test ===\n\n";
    
    // Initialize position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing from starting position: " << pos.to_fen() << "\n\n";
    
    const uint64_t expected_perft6 = 119060324;
    
    std::cout << "Starting perft 6 calculation...\n";
    std::cout << "Expected result: " << format_number(expected_perft6) << " nodes\n\n";
    
    // Time the perft 6 calculation
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft_legal(pos, 6);
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate timing
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double time_seconds = duration.count() / 1000.0;
    double nps = nodes / time_seconds;
    
    // Check correctness
    bool correct = (nodes == expected_perft6);
    
    std::cout << "=== RESULTS ===\n";
    std::cout << "Nodes:       " << format_number(nodes) << "\n";
    std::cout << "Expected:    " << format_number(expected_perft6) << "\n";
    std::cout << "Time:        " << std::fixed << std::setprecision(2) << time_seconds << " seconds\n";
    std::cout << "Speed:       " << format_nps(nps) << "\n";
    std::cout << "Status:      " << (correct ? "✓ CORRECT" : "✗ WRONG") << "\n";
    
    if (!correct) {
        int64_t difference = (int64_t)nodes - (int64_t)expected_perft6;
        std::cout << "Difference:  " << difference << " nodes\n";
    }
    
    std::cout << "\n=== Performance Analysis ===\n";
    std::cout << "Time taken:  " << duration.count() << " ms\n";
    std::cout << "This gives approximately " << format_nps(nps) << " performance\n";
    
    if (time_seconds < 10) {
        std::cout << "⚡ Very fast! (under 10 seconds)\n";
    } else if (time_seconds < 60) {
        std::cout << "🚀 Good performance (under 1 minute)\n";  
    } else if (time_seconds < 300) {
        std::cout << "⏱️  Acceptable performance (under 5 minutes)\n";
    } else {
        std::cout << "🐌 Slow performance (over 5 minutes)\n";
    }
    
    return 0;
}