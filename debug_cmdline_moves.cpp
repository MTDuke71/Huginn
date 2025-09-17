#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

string move_to_string(int from_64, int to_64) {
    string result;
    result += char('a' + (from_64 % 8));
    result += char('1' + (from_64 / 8));
    result += char('a' + (to_64 % 8));
    result += char('1' + (to_64 / 8));
    return result;
}

void analyze_cmdline_kiwipete_moves() {
    cout << "Command Line Kiwipete Move Analysis" << endl;
    cout << "===================================" << endl;
    
    BitboardPosition pos;
    const char* cmdline_kiwipete = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    
    if (!pos.set_from_fen(cmdline_kiwipete)) {
        cout << "Failed to parse command line Kiwipete" << endl;
        return;
    }
    
    cout << "Position: " << cmdline_kiwipete << endl << endl;
    
    // Get moves from both versions
    BitboardMoveList original_moves, optimized_moves;
    
    BitboardPosition pos_orig = pos;
    BitboardMoveGen::generate_legal_moves(pos_orig, original_moves);
    
    BitboardPosition pos_opt = pos;
    BitboardPerftOptimized::generate_legal_moves_fast(pos_opt, optimized_moves);
    
    cout << "Original moves (" << original_moves.moves.size() << "):" << endl;
    for (size_t i = 0; i < original_moves.moves.size(); i++) {
        const auto& move = original_moves.moves[i];
        cout << "  " << (i+1) << ". " << move_to_string(move.from_64, move.to_64) 
             << " (cap:" << move.is_capture << " ep:" << move.is_ep_capture 
             << " castle:" << move.is_castling << ")" << endl;
    }
    
    cout << "\nOptimized moves (" << optimized_moves.moves.size() << "):" << endl;
    for (size_t i = 0; i < optimized_moves.moves.size(); i++) {
        const auto& move = optimized_moves.moves[i];
        cout << "  " << (i+1) << ". " << move_to_string(move.from_64, move.to_64) 
             << " (cap:" << move.is_capture << " ep:" << move.is_ep_capture 
             << " castle:" << move.is_castling << ")" << endl;
    }
}

int main() {
    cout << "Pawn lookup tables initialized successfully." << endl;
    cout << "Initializing bitboard attack tables..." << endl;
    
    analyze_cmdline_kiwipete_moves();
    
    return 0;
}