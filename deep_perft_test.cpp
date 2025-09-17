#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert BitboardMoveList::BitboardMove to SimpleBitboardMove
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.promotion_type != PieceType::None;
    return simple_move;
}

// VICE perft function
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        if (pos.MakeMove(m) == 1) {
            nodes += perft_vice(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

// BitboardPosition perft function
static uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        nodes += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return nodes;
}

// Timed perft function
template<typename Func>
uint64_t timed_perft(Func&& perft_func, int depth, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t result = perft_func(depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double seconds = duration.count() / 1000.0;
    
    std::cout << name << " depth " << depth << ": " << result << " nodes";
    std::cout << " (" << std::fixed << std::setprecision(3) << seconds << "s";
    if (seconds > 0) {
        std::cout << ", " << std::fixed << std::setprecision(0) << (result / seconds / 1000.0) << "k nodes/s";
    }
    std::cout << ")" << std::endl;
    
    return result;
}

int main() {
    Huginn::init();

    std::cout << "=== VICE vs BitboardPosition Deep Perft Test (Release Build) ===\n";
    
    // Test the original problematic position
    std::string problem_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R b KQkq - 0 1";
    std::cout << "Problem position: " << problem_fen << "\n\n";

    Position vice_pos;
    vice_pos.set_from_fen(problem_fen);

    BitboardPosition bb_pos;
    bb_pos.set_from_fen(problem_fen);

    std::cout << "=== Testing Depths 1-5 ===\n";
    
    for (int depth = 1; depth <= 5; depth++) {
        std::cout << "\n--- Depth " << depth << " ---\n";
        
        // Test VICE
        Position vice_copy = vice_pos;
        uint64_t vice_result = timed_perft([&](int d) { return perft_vice(vice_copy, d); }, depth, "VICE");
        
        // Test BitboardPosition
        BitboardPosition bb_copy = bb_pos;
        uint64_t bb_result = timed_perft([&](int d) { return perft_bitboard(bb_copy, d); }, depth, "BitboardPosition");
        
        // Compare
        int64_t diff = (int64_t)bb_result - (int64_t)vice_result;
        std::cout << "Difference: " << diff;
        if (diff == 0) {
            std::cout << " ✅ PERFECT MATCH\n";
        } else {
            std::cout << " ❌ MISMATCH!\n";
            break; // Stop on first mismatch
        }
    }
    
    std::cout << "\n=== Standard Kiwipete Position Test ===\n";
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Kiwipete: " << kiwipete_fen << "\n\n";
    
    Position vice_kiwi;
    vice_kiwi.set_from_fen(kiwipete_fen);
    
    BitboardPosition bb_kiwi;
    bb_kiwi.set_from_fen(kiwipete_fen);
    
    for (int depth = 1; depth <= 4; depth++) {
        std::cout << "\n--- Kiwipete Depth " << depth << " ---\n";
        
        Position vice_kiwi_copy = vice_kiwi;
        uint64_t vice_kiwi_result = timed_perft([&](int d) { return perft_vice(vice_kiwi_copy, d); }, depth, "VICE");
        
        BitboardPosition bb_kiwi_copy = bb_kiwi;
        uint64_t bb_kiwi_result = timed_perft([&](int d) { return perft_bitboard(bb_kiwi_copy, d); }, depth, "BitboardPosition");
        
        int64_t kiwi_diff = (int64_t)bb_kiwi_result - (int64_t)vice_kiwi_result;
        std::cout << "Difference: " << kiwi_diff;
        if (kiwi_diff == 0) {
            std::cout << " ✅ PERFECT MATCH\n";
        } else {
            std::cout << " ❌ MISMATCH!\n";
            break;
        }
    }

    return 0;
}