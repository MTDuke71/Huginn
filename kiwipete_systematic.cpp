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

// Perft divide function with systematic analysis
void perft_divide_systematic(BitboardPosition& pos, int depth) {
    std::cout << "\n=== PERFT DIVIDE DEPTH " << depth << " ===" << std::endl;
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Total moves from root: " << moves.moves.size() << std::endl;
    
    // Reference values for depth 4 per move
    std::map<std::string, uint64_t> depth4_reference = {
        {"a1b1", 83348}, {"a1c1", 83263}, {"a1d1", 79695}, {"a2a3", 94405}, {"a2a4", 90978},
        {"b2b3", 81066}, {"c3a4", 91447}, {"c3b1", 84773}, {"c3b5", 81498}, {"c3d1", 84782},
        {"d2c1", 83037}, {"d2e3", 90274}, {"d2f4", 84869}, {"d2g5", 87951}, {"d2h6", 82323},
        {"d5d6", 79551}, {"d5e6", 97464}, {"e1c1", 79803}, {"e1d1", 79989}, {"e1f1", 77887},
        {"e1g1", 86975}, {"e2a6", 69334}, {"e2b5", 79739}, {"e2c4", 84835}, {"e2d1", 74963},
        {"e2d3", 85119}, {"e2f1", 88728}, {"e5c4", 77752}, {"e5c6", 83885}, {"e5d3", 77431},
        {"e5d7", 93913}, {"e5f7", 88799}, {"e5g4", 79912}, {"e5g6", 83866}, {"f3d3", 83727},
        {"f3e3", 92505}, {"f3f4", 90488}, {"f3f5", 104992}, {"f3f6", 77838}, {"f3g3", 94461},
        {"f3g4", 92037}, {"f3h3", 98524}, {"f3h5", 95034}, {"g2g3", 77468}, {"g2g4", 75677},
        {"g2h3", 82759}, {"h1f1", 81563}, {"h1g1", 84876}
    };
    
    uint64_t total_nodes = 0;
    int move_number = 1;
    int mismatches = 0;
    uint64_t total_deficit = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        // Debug f3f5 move specifically
        if (move_str == "f3f5") {
            std::cout << "\n=== DEBUGGING F3F5 MOVE ===" << std::endl;
            std::cout << "f3f5 move details:" << std::endl;
            std::cout << "  from_64: " << simple_move.from_64 << std::endl;
            std::cout << "  to_64: " << simple_move.to_64 << std::endl;
            std::cout << "  is_capture: " << simple_move.is_capture << std::endl;
            std::cout << "  is_ep_capture: " << simple_move.is_ep_capture << std::endl;
            std::cout << "  is_castling: " << simple_move.is_castling << std::endl;
            std::cout << "  is_promotion: " << simple_move.is_promotion << std::endl;
        }

        // Make move and calculate perft
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);

        total_nodes += nodes;
        
        // Check against reference if we're at depth 4
        if (depth == 4 && depth4_reference.count(move_str)) {
            uint64_t expected = depth4_reference[move_str];
            int64_t diff = (int64_t)nodes - (int64_t)expected;
            
            if (diff != 0) {
                std::cout << std::setw(2) << move_number << ". " << std::setw(6) << move_str 
                          << " -> " << std::setw(8) << nodes << " (exp: " << std::setw(8) << expected 
                          << ", diff: " << std::setw(6) << diff << ") ❌" << std::endl;
                mismatches++;
                if (diff < 0) total_deficit += -diff;
            } else {
                std::cout << std::setw(2) << move_number << ". " << std::setw(6) << move_str 
                          << " -> " << std::setw(8) << nodes << " ✓" << std::endl;
            }
        } else {
            std::cout << std::setw(2) << move_number << ". " << std::setw(6) << move_str 
                      << " -> " << std::setw(8) << nodes << " nodes" << std::endl;
        }
        move_number++;
    }

    std::cout << "\nTotal nodes: " << total_nodes << std::endl;
    
    if (depth == 4) {
        std::cout << "Mismatches: " << mismatches << " out of " << moves.moves.size() << " moves" << std::endl;
        std::cout << "Total deficit: " << total_deficit << std::endl;
    }
    
    // Known reference values for f3f5 position
    const std::vector<uint64_t> reference = {1, 45, 2396, 104992};
    
    if (depth < reference.size()) {
        uint64_t expected = reference[depth];
        std::cout << "Expected:    " << expected << std::endl;
        if (total_nodes == expected) {
            std::cout << "✓ MATCH!" << std::endl;
        } else {
            std::cout << "✗ MISMATCH! Difference: " << (int64_t)total_nodes - (int64_t)expected << std::endl;
        }
    }
}

int main() {
    std::cout << "=== KIWIPETE SYSTEMATIC ANALYSIS ===" << std::endl;
    
    // Initialize the f3f5 position - this is the position after f3f5 move from Kiwipete
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PNQ2/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 1 1";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << kiwipete_fen << std::endl;
    
    // Step 1: Verify basic move generation
    std::cout << "\n=== STEP 1: BASIC MOVE GENERATION ===" << std::endl;
    BitboardMoveList root_moves;
    generate_legal_moves(pos, root_moves);
    std::cout << "Root position legal moves: " << root_moves.moves.size() << std::endl;
    std::cout << "Expected: 45 moves" << std::endl;
    
    // Print the actual moves being generated for debugging
    std::cout << "\nActual moves generated:" << std::endl;
    for (size_t i = 0; i < root_moves.moves.size(); i++) {
        SimpleBitboardMove simple_move = convert_move(root_moves.moves[i]);
        std::string move_str = move_to_string(simple_move);
        std::cout << (i+1) << ". " << move_str << std::endl;
    }
    
    if (root_moves.moves.size() == 45) {
        std::cout << "✓ Root move count is correct!" << std::endl;
    } else {
        std::cout << "✗ Root move count mismatch!" << std::endl;
        std::cout << "This is a fundamental issue - exiting for investigation." << std::endl;
        return 1;
    }
    
    // Step 2: Quick perft verification at depth 1
    std::cout << "\n=== STEP 2: DEPTH 1 PERFT ===" << std::endl;
    uint64_t depth1_nodes = perft(pos, 1);
    std::cout << "Depth 1 perft: " << depth1_nodes << std::endl;
    std::cout << "Expected: 45" << std::endl;
    
    if (depth1_nodes == 45) {
        std::cout << "✓ Depth 1 perft matches!" << std::endl;
    } else {
        std::cout << "✗ Depth 1 perft mismatch!" << std::endl;
        return 1;
    }
    
    // Step 3: Systematic perft divide analysis
    for (int depth = 2; depth <= 4; depth++) {
        perft_divide_systematic(pos, depth);
        std::cout << std::string(50, '-') << std::endl;
    }
    
    return 0;
}