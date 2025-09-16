#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>
#include <string>

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

// Inline perft function to avoid header issues
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

int main() {
    std::cout << "=== Debug: Divide Function vs Direct Perft ===\n\n";
    
    // Initialize position using set_from_fen
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Starting position loaded successfully\n";
    std::cout << "Initial position FEN: " << pos.to_fen() << "\n\n";
    
    // Test specific move: a2a4 (which shows discrepancy: 502 vs 420)
    SimpleBitboardMove test_move;
    test_move.from_64 = 8;  // a2
    test_move.to_64 = 24;   // a4
    test_move.is_capture = false;
    test_move.is_ep_capture = false;
    test_move.is_castling = false;
    test_move.is_promotion = false;
    test_move.promotion_type = PieceType::None;
    
    std::cout << "=== Testing move a2a4 ===\n";
    
    // Method 1: Direct perft call (like test_a2a4_perft2.cpp)
    std::cout << "\nMethod 1: Direct perft after make_move\n";
    std::cout << "Initial position: " << pos.to_fen() << "\n";
    
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(test_move);
    std::cout << "After a2a4: " << pos.to_fen() << "\n";
    
    uint64_t direct_result = perft_legal(pos, 2);
    std::cout << "Direct perft(2) result: " << direct_result << "\n";
    
    pos.unmake_move(test_move, undo);
    std::cout << "After unmake: " << pos.to_fen() << "\n";
    
    // Method 2: Exactly mirror divide function logic
    std::cout << "\nMethod 2: Mirror divide function logic\n";
    std::cout << "Initial position: " << pos.to_fen() << "\n";
    
    // Generate legal moves (exactly like divide function)
    std::vector<SimpleBitboardMove> legal_moves;
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "After generate_all_moves: " << pos.to_fen() << "\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        std::cout << "Before is_legal_move check: " << pos.to_fen() << "\n";
        
        if (pos.is_legal_move(simple_move)) {
            legal_moves.push_back(simple_move);
        }
        
        std::cout << "After is_legal_move check:  " << pos.to_fen() << "\n";
        
        // If position changed, break and report
        std::string current_fen = pos.to_fen();
        if (current_fen != "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
            std::cout << "⚠️  CORRUPTION DETECTED during legal move generation!\n";
            std::cout << "Expected: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n";
            std::cout << "Actual:   " << current_fen << "\n";
            break;
        }
    }
    
    std::cout << "Generated " << legal_moves.size() << " legal moves\n";
    
    // Find a2a4 in the legal moves
    uint64_t divide_result = 0;
    bool found_a2a4 = false;
    
    for (const SimpleBitboardMove& move : legal_moves) {
        if (move.from_64 == 8 && move.to_64 == 24) {  // a2 to a4
            found_a2a4 = true;
            std::cout << "Found a2a4 in legal moves list\n";
            
            // Mirror divide function exactly
            BitboardPosition::UndoInfo undo_divide = pos.make_move_with_undo(move);
            std::cout << "After make_move in divide logic: " << pos.to_fen() << "\n";
            
            divide_result = perft_legal(pos, 2);
            std::cout << "Divide function perft(2) result: " << divide_result << "\n";
            
            pos.unmake_move(move, undo_divide);
            std::cout << "After unmake in divide logic: " << pos.to_fen() << "\n";
            break;
        }
    }
    
    if (!found_a2a4) {
        std::cout << "ERROR: a2a4 not found in legal moves!\n";
        return 1;
    }
    
    // Compare results
    std::cout << "\n=== COMPARISON ===\n";
    std::cout << "Direct method result:       " << direct_result << "\n";
    std::cout << "Divide function result:     " << divide_result << "\n";
    std::cout << "Difference:                 " << (int64_t)divide_result - (int64_t)direct_result << "\n";
    
    if (direct_result == divide_result) {
        std::cout << "✓ Results MATCH - issue must be elsewhere\n";
    } else {
        std::cout << "✗ Results DIFFER - found the problem!\n";
    }
    
    // Additional test: Check if is_legal_move affects position state
    std::cout << "\n=== Testing is_legal_move state effects ===\n";
    std::cout << "Position before is_legal_move: " << pos.to_fen() << "\n";
    bool is_legal = pos.is_legal_move(test_move);
    std::cout << "is_legal_move result: " << (is_legal ? "true" : "false") << "\n";
    std::cout << "Position after is_legal_move:  " << pos.to_fen() << "\n";
    
    return 0;
}