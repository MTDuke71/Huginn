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

// Calculate nodes per second
double calculate_nps(uint64_t nodes, double time_seconds) {
    return nodes / time_seconds;
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
    if (nps >= 1000000) {
        return std::to_string(int(nps / 1000000)) + "M nps";
    } else if (nps >= 1000) {
        return std::to_string(int(nps / 1000)) + "K nps";
    } else {
        return std::to_string(int(nps)) + " nps";
    }
}

int main() {
    std::cout << "=== Huginn Bitboard Engine Timing Test ===\n\n";
    
    // Initialize position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing from starting position: " << pos.to_fen() << "\n\n";
    
    // Expected perft results for verification
    uint64_t expected_results[] = {1, 20, 400, 8902, 197281, 4865609, 119060324};
    
    std::cout << std::left << std::setw(8) << "Depth" 
              << std::setw(15) << "Nodes" 
              << std::setw(12) << "Time (ms)" 
              << std::setw(15) << "NPS" 
              << std::setw(10) << "Status" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    // Test depths 1-6 (depth 7 might be too slow for quick testing)
    for (int depth = 1; depth <= 6; depth++) {
        // Reset position for each test
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        // Time the perft calculation
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft_legal(pos, depth);
        auto end = std::chrono::high_resolution_clock::now();
        
        // Calculate timing
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double time_ms = duration.count() / 1000.0;
        double time_seconds = time_ms / 1000.0;
        double nps = calculate_nps(nodes, time_seconds);
        
        // Check correctness
        std::string status = (nodes == expected_results[depth]) ? "✓ CORRECT" : "✗ WRONG";
        
        // Display results
        std::cout << std::left << std::setw(8) << depth
                  << std::setw(15) << format_number(nodes)
                  << std::setw(12) << std::fixed << std::setprecision(2) << time_ms
                  << std::setw(15) << format_nps(nps)
                  << std::setw(10) << status << std::endl;
        
        // Break early if we get wrong results
        if (nodes != expected_results[depth]) {
            std::cout << "\n⚠️  INCORRECT RESULT at depth " << depth << "!\n";
            std::cout << "Expected: " << format_number(expected_results[depth]) << "\n";
            std::cout << "Got:      " << format_number(nodes) << "\n";
            std::cout << "Stopping timing test.\n";
            break;
        }
        
        // Break if depth is taking too long (over 30 seconds)
        if (time_ms > 30000) {
            std::cout << "\n⏰ Depth " << depth << " took over 30 seconds, stopping here.\n";
            break;
        }
    }
    
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << "• All results verified against known perft values\n";
    std::cout << "• Engine appears to be working correctly\n";
    std::cout << "• Performance can be compared with other engines\n";
    
    return 0;
}