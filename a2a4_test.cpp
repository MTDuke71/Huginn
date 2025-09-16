#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <map>

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

// Move to string function
std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result;
    result += char('a' + file_of_64(move.from_64));
    result += char('1' + rank_of_64(move.from_64));
    result += char('a' + file_of_64(move.to_64));
    result += char('1' + rank_of_64(move.to_64));
    return result;
}

// Perft function 
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    pos.generate_all_moves(moves);
    
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
    std::cout << "=== Perft Analysis After a2a4 ===\n\n";
    
    // Reference data from user after a2a4
    std::map<std::string, uint64_t> expected_after_a2a4 = {
        {"a7a5", 9062}, {"a7a6", 9312}, {"b7b5", 11606}, {"b7b6", 10348},
        {"b8a6", 9827}, {"b8c6", 10746}, {"c7c5", 10737}, {"c7c6", 10217},
        {"d7d5", 13725}, {"d7d6", 13203}, {"e7e5", 14560}, {"e7e6", 14534},
        {"f7f5", 9847}, {"f7f6", 9328}, {"g7g5", 10293}, {"g7g6", 10310},
        {"g8f6", 10758}, {"g8h6", 9798}, {"h7h5", 10293}, {"h7h6", 9328}
    };
    
    // Initialize
    init_knight_lookup_tables();
    init_king_lookup_tables();
    init_pawn_lookup_tables();
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Make a2a4 move
    SimpleBitboardMove a2a4;
    a2a4.from_64 = 8;  // a2 square
    a2a4.to_64 = 24;   // a4 square
    a2a4.is_capture = false;
    a2a4.is_ep_capture = false;
    a2a4.is_castling = false;
    a2a4.is_promotion = false;
    a2a4.promotion_type = PIECE_NONE;
    
    std::cout << "Making move a2a4...\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(a2a4);
    
    std::cout << "Position after a2a4. Testing perft divide depth 4:\n\n";
    
    BitboardMoveList moves;
    pos.generate_all_moves(moves);
    
    uint64_t total_nodes = 0;
    uint64_t expected_total = 217832;
    bool found_errors = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            
            BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
            uint64_t count = perft_legal(pos, 3);  // Depth 4 - 1 after first move
            pos.unmake_move(simple_move, move_undo);
            
            auto it = expected_after_a2a4.find(move_str);
            if (it != expected_after_a2a4.end()) {
                uint64_t expected_count = it->second;
                int64_t diff = (int64_t)count - (int64_t)expected_count;
                
                std::cout << move_str << "    " << std::setw(8) << count << "    " << std::setw(8) << expected_count;
                std::cout << "    " << std::setw(8) << diff;
                
                if (diff == 0) {
                    std::cout << "       ✅\n";
                } else {
                    std::cout << "       ❌\n";
                    found_errors = true;
                }
                
                total_nodes += count;
            } else {
                std::cout << move_str << "    " << std::setw(8) << count << "    UNKNOWN     UNKNOWN        ?\n";
                total_nodes += count;
            }
        }
    }
    
    std::cout << "------------------------------------------------------------\n";
    int64_t total_diff = (int64_t)total_nodes - (int64_t)expected_total;
    std::cout << "TOTAL   " << std::setw(8) << total_nodes << "    " << std::setw(8) << expected_total;
    std::cout << "    " << std::setw(8) << total_diff;
    
    if (total_diff == 0) {
        std::cout << "       ✅\n";
    } else {
        std::cout << "       ❌\n";
        found_errors = true;
    }
    
    std::cout << "\n=== Summary ===\n";
    if (found_errors) {
        std::cout << "❌ Found the bug! It manifests in the position after a2a4.\n";
    } else {
        std::cout << "✅ Perfect match after a2a4. The bug is elsewhere.\n";
    }
    
    return 0;
}