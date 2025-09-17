#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>

using namespace BitboardMoveGen;

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

struct PerftResult {
    uint64_t nodes;
    std::string first_difference_path;
    std::string first_difference_fen;
    bool found_difference;
};

// Known reference values for Kiwipete depth 4 divide (depth 3 from after first move)
std::map<std::string, uint64_t> get_reference_depth4() {
    return {
        {"a2a3", 94405}, {"a2a4", 89498}, {"b2b3", 76840}, {"d2c1", 77410},
        {"d2e3", 89966}, {"d2f4", 80446}, {"d2g5", 89188}, {"d2h6", 80975},
        {"d5d6", 78293}, {"d5e6", 96446}, {"c3b1", 81613}, {"c3d1", 81569},
        {"c3a4", 94513}, {"c3b5", 88102}, {"e5d3", 67158}, {"e5c4", 71359},
        {"e5g4", 69808}, {"e5c6", 83396}, {"e5g6", 80657}, {"e5d7", 89953},
        {"e5f7", 85013}, {"a1b1", 78173}, {"a1c1", 77890}, {"a1d1", 72890},
        {"f3f6", 81181}, {"f3d3", 80658}, {"f3e3", 91406}, {"f3g3", 95376},
        {"f3f4", 88318}, {"f3g4", 92217}, {"f3f5", 107604}, {"f3h3", 103500},
        {"f3h5", 96789}, {"g2g3", 70850}, {"g2g4", 68150}, {"g2h3", 77923},
        {"h1f1", 75262}, {"h1g1", 81414}, {"e1d1", 72673}, {"e1f1", 69000},
        {"e1g1", 84090}, {"e1c1", 72510}
    };
}

PerftResult perft_with_early_exit(BitboardPosition& pos, int depth, 
                                  const std::string& move_path = "",
                                  const std::map<std::string, uint64_t>* reference = nullptr) {
    PerftResult result = {0, "", "", false};
    
    if (depth == 0) {
        result.nodes = 1;
        return result;
    }

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        std::string current_path = move_path.empty() ? move_str : move_path + "-" + move_str;
        
        // Store FEN before move
        std::string fen_before = pos.to_fen();
        
        // Make move
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        
        // Store FEN after move
        std::string fen_after = pos.to_fen();
        
        // Recurse
        PerftResult subtree = perft_with_early_exit(pos, depth - 1, current_path, reference);
        
        // Unmake move
        pos.unmake_move(simple_move, undo);
        
        // Check if we found a difference in subtree
        if (subtree.found_difference) {
            result = subtree;
            return result;
        }
        
        // If we're at depth 4 from root, run depth 3 and check differences
        if (depth == 4 && move_path.empty()) {
            // This is depth 3 after the first move, which should give us insight
            std::cout << "Move: " << move_str << " gives " << subtree.nodes << " nodes at depth 3\n";
            
            // For now, just report the first move to see what we get
            if (move_str == "a2a3") {
                std::cout << "❌ ANALYZING FIRST MOVE a2a3\n";
                std::cout << "Depth 3 nodes after a2a3: " << subtree.nodes << "\n";
                std::cout << "FEN before: " << fen_before << "\n";
                std::cout << "FEN after:  " << fen_after << "\n";
                
                result.found_difference = true;
                result.first_difference_path = current_path;
                result.first_difference_fen = fen_after;
                result.nodes = total_nodes + subtree.nodes;
                return result;
            }
        }
        
        total_nodes += subtree.nodes;
    }
    
    result.nodes = total_nodes;
    return result;
}

int main() {
    std::cout << "=== Depth 4 Early Exit Analysis ===\n";
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    std::cout << "FEN: " << fen << "\n\n";
    
    // Get reference values
    auto reference = get_reference_depth4();
    std::cout << "Loaded " << reference.size() << " reference values for depth 4\n\n";
    
    std::cout << "Running perft depth 4 with early exit on first difference...\n\n";
    
    // Run the analysis
    PerftResult result = perft_with_early_exit(pos, 4, "", &reference);
    
    if (result.found_difference) {
        std::cout << "\n=== DIFFERENCE ANALYSIS ===\n";
        std::cout << "First difference at path: " << result.first_difference_path << "\n";
        std::cout << "Position when difference occurred: " << result.first_difference_fen << "\n";
        
        // Let's analyze this specific position further
        std::cout << "\n=== Analyzing Problem Position ===\n";
        BitboardPosition problem_pos;
        if (problem_pos.set_from_fen(result.first_difference_fen)) {
            BitboardMoveList moves;
            generate_legal_moves(problem_pos, moves);
            std::cout << "Legal moves in problem position: " << moves.moves.size() << "\n";
            
            // Show first few moves
            std::cout << "First few moves from problem position:\n";
            for (size_t i = 0; i < std::min(size_t(5), moves.moves.size()); i++) {
                SimpleBitboardMove simple_move = convert_move(moves.moves[i]);
                std::cout << "  " << (i+1) << ". " << move_to_string(simple_move) << "\n";
            }
        }
    } else {
        std::cout << "\n✓ No differences found in depth 4 analysis!\n";
        std::cout << "Total nodes: " << result.nodes << "\n";
        std::cout << "Expected:    " << 4085603 << "\n";
        std::cout << "Difference:  " << (int64_t)result.nodes - 4085603 << "\n";
    }
    
    return 0;
}