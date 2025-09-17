#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

uint64_t perft_original(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;
    
    if (depth == 1) {
        BitboardMoveList moves;
        BitboardMoveGen::generate_legal_moves(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    uint64_t total = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            total += perft_original(new_pos, depth - 1);
        }
    }
    return total;
}

uint64_t perft_optimized(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;
    
    if (depth == 1) {
        BitboardMoveList moves;
        BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    uint64_t total = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            total += perft_optimized(new_pos, depth - 1);
        }
    }
    return total;
}

void analyze_cmdline_kiwipete() {
    cout << "Command Line Kiwipete Analysis" << endl;
    cout << "==============================" << endl;
    
    BitboardPosition pos;
    const char* cmdline_kiwipete = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    
    if (!pos.set_from_fen(cmdline_kiwipete)) {
        cout << "Failed to parse command line Kiwipete" << endl;
        return;
    }
    
    cout << "Position: " << cmdline_kiwipete << endl;
    
    // Test depths 1-3
    for (int depth = 1; depth <= 3; depth++) {
        uint64_t orig_total = perft_original(pos, depth);
        uint64_t opt_total = perft_optimized(pos, depth);
        
        cout << "Depth " << depth << ":" << endl;
        cout << "  Original:  " << orig_total << endl;
        cout << "  Optimized: " << opt_total << endl;
        cout << "  Difference: " << (int64_t)opt_total - (int64_t)orig_total << endl;
        cout << endl;
        
        if (orig_total != opt_total) {
            cout << "  *** DISCREPANCY FOUND AT DEPTH " << depth << " ***" << endl;
            break;
        }
    }
}

int main() {
    cout << "Pawn lookup tables initialized successfully." << endl;
    cout << "Initializing bitboard attack tables..." << endl;
    
    analyze_cmdline_kiwipete();
    
    return 0;
}