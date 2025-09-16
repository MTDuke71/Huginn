#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>

using namespace BitboardMoveGen;
using namespace std;

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

// Reference data for depth 5 after a2a3
map<string, uint64_t> reference_data = {
    {"a7a5", 199263}, {"a7a6", 165948}, {"b7b5", 198797}, {"b7b6", 197301},
    {"b8a6", 182327}, {"b8c6", 215405}, {"c7c5", 219841}, {"c7c6", 203960},
    {"d7d5", 331473}, {"d7d6", 300892}, {"e7e5", 373372}, {"e7e6", 371069},
    {"f7f5", 181945}, {"f7f6", 163919}, {"g7g5", 196185}, {"g7g6", 199099},
    {"g8f6", 213990}, {"g8h6", 181930}, {"h7h5", 200588}, {"h7h6", 165963}
};

// Perft function with detailed comparison
uint64_t perft_breakdown(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    vector<pair<string, uint64_t>> results;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t count = perft_breakdown(pos, depth - 1);
            
            string from_sq = string(1, 'a' + (move.from_64 % 8)) + string(1, '1' + (move.from_64 / 8));
            string to_sq = string(1, 'a' + (move.to_64 % 8)) + string(1, '1' + (move.to_64 / 8));
            string move_str = from_sq + to_sq;
            
            results.push_back({move_str, count});
            nodes += count;
            pos.unmake_move(simple_move, undo);
        }
    }
    
    // If this is depth 4 (showing moves from a2a3 position), compare with reference
    if (depth == 4) {
        cout << "Move     Our Result   Expected     Difference   Status" << endl;
        cout << "-------------------------------------------------------" << endl;
        
        uint64_t total_expected = 0;
        for (const auto& result : results) {
            uint64_t expected = reference_data[result.first];
            int64_t diff = (int64_t)result.second - (int64_t)expected;
            total_expected += expected;
            
            string status = (diff == 0) ? "✓ CORRECT" : 
                           (diff > 0) ? "✗ OVER" : "✗ UNDER";
            
            cout << left << setw(8) << result.first 
                 << " " << setw(11) << result.second
                 << " " << setw(11) << expected
                 << " " << setw(11) << diff
                 << " " << status << endl;
        }
        
        cout << "-------------------------------------------------------" << endl;
        cout << "TOTAL    " << setw(11) << nodes 
             << " " << setw(11) << total_expected
             << " " << setw(11) << ((int64_t)nodes - (int64_t)total_expected) << endl;
    }
    
    return nodes;
}

int main() {
    cout << "=== Deep Analysis: a2a3 Position (Depth 4 from a2a3) ===" << endl;
    cout << "Expected total: 4463267" << endl << endl;
    
    // Start from position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    cout << "Position after a2a3:" << endl;
    cout << "rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1" << endl << endl;
    
    uint64_t total = perft_breakdown(pos, 4);  // depth 4 from this position = depth 5 total
    
    cout << endl << "=== Summary ===" << endl;
    cout << "Our total:   " << total << endl;
    cout << "Expected:    4463267" << endl;
    cout << "Difference:  " << ((int64_t)total - 4463267) << endl;
    
    return 0;
}