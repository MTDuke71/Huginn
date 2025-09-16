#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>

using namespace BitboardMoveGen;

// Helper function to convert move for compatibility
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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

// Create f3f5 move
SimpleBitboardMove create_f3f5_move() {
    SimpleBitboardMove move;
    move.from_64 = 21; // f3 = file 5, rank 3 = 5 + 2*8 = 21
    move.to_64 = 37;   // f5 = file 5, rank 5 = 5 + 4*8 = 37
    move.is_capture = false;
    move.is_ep_capture = false;
    move.is_castling = false;
    move.is_promotion = false;
    move.promotion_type = static_cast<PieceType>(0);
    return move;
}

// Simple perft function
uint64_t perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== F3F5 FOCUSED ANALYSIS ===" << std::endl;
    
    // Initialize the Kiwipete position
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "Starting FEN: " << kiwipete_fen << std::endl;
    
    // Create and make the f3f5 move
    SimpleBitboardMove f3f5_move = create_f3f5_move();
    std::cout << "\nMaking move f3f5..." << std::endl;
    
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5_move);
    
    std::cout << "Position after f3f5 created successfully" << std::endl;
    
    // Generate legal moves from the f3f5 position
    BitboardMoveList moves_after_f3f5;
    generate_legal_moves(pos, moves_after_f3f5);
    
    std::cout << "\nMoves generated after f3f5: " << moves_after_f3f5.moves.size() << std::endl;
    std::cout << "Expected: 45 moves" << std::endl;
    
    // Reference values for each move after f3f5 at depth 3
    std::map<std::string, uint64_t> f3f5_depth3_reference = {
        {"a6b5", 2448}, {"a6b7", 2398}, {"a6c4", 2404}, {"a6c8", 2076}, {"a6d3", 2398}, {"a6e2", 2086},
        {"a8b8", 2442}, {"a8c8", 2281}, {"a8d8", 2284}, {"b4b3", 2533}, {"b4c3", 2436}, {"b6a4", 2334},
        {"b6c4", 2347}, {"b6c8", 2064}, {"b6d5", 2274}, {"c7c5", 2322}, {"c7c6", 2429}, {"d7d6", 2353},
        {"e6d5", 2398}, {"e6f5", 2050}, {"e7c5", 2766}, {"e7d6", 2432}, {"e7d8", 2189}, {"e7f8", 2184},
        {"e8c8", 2304}, {"e8d8", 2254}, {"e8f8", 2208}, {"e8g8", 2230}, {"f6d5", 2505}, {"f6e4", 2921},
        {"f6g4", 2555}, {"f6g8", 2349}, {"f6h5", 2453}, {"f6h7", 2349}, {"g6f5", 2007}, {"g6g5", 2295},
        {"g7f8", 2173}, {"g7h6", 2420}, {"h3g2", 2697}, {"h8f8", 2015}, {"h8g8", 2120}, {"h8h4", 2426},
        {"h8h5", 2331}, {"h8h6", 2226}, {"h8h7", 2226}
    };
    
    // Analyze each move after f3f5
    std::cout << "\n=== DEPTH 3 PERFT ANALYSIS FROM F3F5 POSITION ===" << std::endl;
    
    uint64_t total_nodes = 0;
    int mismatches = 0;
    uint64_t total_deficit = 0;
    
    for (size_t i = 0; i < moves_after_f3f5.moves.size(); i++) {
        const auto& move = moves_after_f3f5.moves[i];
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        // Calculate perft depth 2 for this move
        BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft(pos, 2);
        pos.unmake_move(simple_move, move_undo);
        
        total_nodes += nodes;
        
        // Check against reference
        if (f3f5_depth3_reference.count(move_str)) {
            uint64_t expected = f3f5_depth3_reference[move_str];
            int64_t diff = (int64_t)nodes - (int64_t)expected;
            
            if (diff != 0) {
                std::cout << std::setw(2) << (i+1) << ". " << std::setw(6) << move_str 
                          << " -> " << std::setw(4) << nodes << " (exp: " << std::setw(4) << expected 
                          << ", diff: " << std::setw(4) << diff << ") ❌" << std::endl;
                mismatches++;
                if (diff < 0) total_deficit += -diff;
            } else {
                std::cout << std::setw(2) << (i+1) << ". " << std::setw(6) << move_str 
                          << " -> " << std::setw(4) << nodes << " ✓" << std::endl;
            }
        } else {
            std::cout << std::setw(2) << (i+1) << ". " << std::setw(6) << move_str 
                      << " -> " << std::setw(4) << nodes << " (no ref)" << std::endl;
        }
    }
    
    std::cout << "\nSummary after f3f5:" << std::endl;
    std::cout << "Total nodes: " << total_nodes << std::endl;
    std::cout << "Expected: 104992" << std::endl;
    std::cout << "Difference: " << (int64_t)total_nodes - 104992 << std::endl;
    std::cout << "Mismatches: " << mismatches << " out of " << moves_after_f3f5.moves.size() << " moves" << std::endl;
    std::cout << "Total deficit: " << total_deficit << std::endl;
    
    // Unmake the f3f5 move to restore original position
    pos.unmake_move(f3f5_move, undo);
    
    std::cout << "\nf3f5 move unmade successfully" << std::endl;
    
    return 0;
}