#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
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

// Perft function
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

// Test specific positions that might trigger en passant issues
void test_en_passant_positions() {
    std::cout << "=== Testing En Passant Related Positions ===\n\n";
    
    // Position after e2-e4, likely to trigger en passant scenarios
    BitboardPosition pos1;
    pos1.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "Position after e2-e4 (en passant square e3):\n";
    std::cout << "FEN: " << pos1.to_fen() << "\n";
    uint64_t result1 = perft_legal(pos1, 4);
    std::cout << "Perft 4: " << result1 << "\n\n";
    
    // Position after d2-d4, e7-e5 (another en passant setup)
    BitboardPosition pos2;
    pos2.set_from_fen("rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 2");
    std::cout << "Position after d2-d4, e7-e5 (en passant square e6):\n";
    std::cout << "FEN: " << pos2.to_fen() << "\n";
    uint64_t result2 = perft_legal(pos2, 4);
    std::cout << "Perft 4: " << result2 << "\n\n";
}

// Test castling scenarios
void test_castling_positions() {
    std::cout << "=== Testing Castling Related Positions ===\n\n";
    
    // Position where castling is still possible
    BitboardPosition pos3;
    pos3.set_from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    std::cout << "Position with castling rights intact:\n";
    std::cout << "FEN: " << pos3.to_fen() << "\n";
    uint64_t result3 = perft_legal(pos3, 3);
    std::cout << "Perft 3: " << result3 << "\n\n";
}

// Test position state integrity after multiple moves
void test_deep_position_integrity() {
    std::cout << "=== Testing Deep Position State Integrity ===\n\n";
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Manually make a sequence of moves that should trigger any state corruption
    std::vector<std::pair<int, int>> move_sequence = {
        {12, 28},  // e2-e4 (creates en passant square)
        {52, 36},  // e7-e5  
        {6, 21},   // g1-f3
        {57, 42}   // b8-c6
    };
    
    std::vector<BitboardPosition::UndoInfo> undo_stack;
    
    std::cout << "Starting position: " << pos.to_fen() << "\n";
    
    // Make moves
    for (size_t i = 0; i < move_sequence.size(); i++) {
        SimpleBitboardMove move;
        move.from_64 = move_sequence[i].first;
        move.to_64 = move_sequence[i].second;
        move.is_capture = false;
        move.is_ep_capture = false;
        move.is_castling = false;
        move.is_promotion = false;
        move.promotion_type = PieceType::None;
        
        auto undo = pos.make_move_with_undo(move);
        undo_stack.push_back(undo);
        
        char from_file = 'a' + (move.from_64 % 8);
        char from_rank = '1' + (move.from_64 / 8);
        char to_file = 'a' + (move.to_64 % 8);  
        char to_rank = '1' + (move.to_64 / 8);
        
        std::cout << "After " << from_file << from_rank << to_file << to_rank 
                  << ": " << pos.to_fen() << "\n";
    }
    
    // Test perft from this deep position
    std::cout << "\nPerft 1 from deep position: " << perft_legal(pos, 1) << "\n";
    
    // Unmake all moves and verify we get back to start
    for (int i = move_sequence.size() - 1; i >= 0; i--) {
        SimpleBitboardMove move;
        move.from_64 = move_sequence[i].first;
        move.to_64 = move_sequence[i].second;
        move.is_capture = false;
        move.is_ep_capture = false;
        move.is_castling = false;
        move.is_promotion = false;
        move.promotion_type = PieceType::None;
        
        pos.unmake_move(move, undo_stack[i]);
    }
    
    std::cout << "After unmaking all moves: " << pos.to_fen() << "\n";
    std::string expected = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    bool restored = (pos.to_fen() == expected);
    std::cout << "Position restored correctly: " << (restored ? "✓ YES" : "✗ NO") << "\n";
}

int main() {
    std::cout << "=== Deep Position Analysis for Perft Bugs ===\n\n";
    
    test_en_passant_positions();
    test_castling_positions(); 
    test_deep_position_integrity();
    
    return 0;
}