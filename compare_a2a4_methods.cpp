#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert function (exactly like perft_divide)
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

// Move to string function (exactly like perft_divide)
std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result;
    result += char('a' + file_of_64(move.from_64));
    result += char('1' + rank_of_64(move.from_64));
    result += char('a' + file_of_64(move.to_64));
    result += char('1' + rank_of_64(move.to_64));
    return result;
}

// Perft WITH legal checking (exactly like perft_divide)
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "Comparing individual a2a4 test vs divide function logic" << std::endl;
    std::cout << "======================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Method 1: Direct a2a4 test" << std::endl;
    std::cout << "---------------------------" << std::endl;
    
    // Create a2a4 move manually
    SimpleBitboardMove a2a4_manual;
    a2a4_manual.from_64 = 8;  // a2
    a2a4_manual.to_64 = 24;   // a4
    a2a4_manual.is_capture = false;
    a2a4_manual.is_ep_capture = false;
    a2a4_manual.is_castling = false;
    a2a4_manual.is_promotion = false;
    a2a4_manual.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(a2a4_manual)) {
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(a2a4_manual);
        uint64_t nodes_manual = perft_legal(pos, 2);
        pos.unmake_move(a2a4_manual, undo_info);
        std::cout << "Manual a2a4 perft(2): " << nodes_manual << std::endl;
    }
    
    std::cout << "\nMethod 2: Divide function logic" << std::endl;
    std::cout << "-------------------------------" << std::endl;
    
    // Generate all moves and find a2a4
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int depth = 3;
    uint64_t a2a4_divide_result = 0;
    bool found_a2a4 = false;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        if (move_str == "a2a4") {
            found_a2a4 = true;
            std::cout << "Found a2a4 in move list" << std::endl;
            
            if (pos.is_legal_move(simple_move)) {
                std::cout << "a2a4 is legal" << std::endl;
                BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
                
                // This is exactly the divide logic: (depth <= 1) ? 1 : perft_legal(pos, depth - 1)
                uint64_t nodes = (depth <= 1) ? 1 : perft_legal(pos, depth - 1);
                
                pos.unmake_move(simple_move, undo_info);
                
                a2a4_divide_result = nodes;
                std::cout << "Divide logic a2a4 result: " << nodes << std::endl;
                std::cout << "Calculation: depth=" << depth << ", so (depth <= 1) is false, nodes = perft_legal(pos, " << (depth-1) << ")" << std::endl;
            } else {
                std::cout << "a2a4 is NOT legal!" << std::endl;
            }
            break;
        }
    }
    
    if (!found_a2a4) {
        std::cout << "ERROR: a2a4 not found in generated moves!" << std::endl;
    }
    
    std::cout << "\nComparison:" << std::endl;
    std::cout << "Manual test: " << (pos.is_legal_move(a2a4_manual) ? 
        [&]() {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(a2a4_manual);
            uint64_t result = perft_legal(pos, 2);
            pos.unmake_move(a2a4_manual, undo);
            return result;
        }() : 0) << std::endl;
    std::cout << "Divide logic: " << a2a4_divide_result << std::endl;
    std::cout << "Expected: 420" << std::endl;
    
    return 0;
}