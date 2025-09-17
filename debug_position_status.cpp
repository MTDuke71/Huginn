#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

void check_position_status() {
    cout << "Position Status Analysis" << endl;
    cout << "========================" << endl;
    
    BitboardPosition pos;
    const char* cmdline_kiwipete = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    
    if (!pos.set_from_fen(cmdline_kiwipete)) {
        cout << "Failed to parse position" << endl;
        return;
    }
    
    cout << "Position: " << cmdline_kiwipete << endl;
    cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << endl;
    
    // Get king position
    uint64_t king_bb = pos.get_pieces(pos.side_to_move, PieceType::King);
    if (king_bb == 0) {
        cout << "No king found!" << endl;
        return;
    }
    
    int king_square = pop_lsb_copy(king_bb);  // Use the same function as optimized code
    char king_file = 'a' + (king_square % 8);
    char king_rank = '1' + (king_square / 8);
    cout << "King position: " << king_file << king_rank << " (square " << king_square << ")" << endl;
    
    // Check if in check using the original method
    BitboardMoveList legal_moves;
    BitboardMoveGen::generate_legal_moves(pos, legal_moves);
    cout << "Original legal moves: " << legal_moves.moves.size() << endl;
    
    // Let's also try to check manually if king is in check
    cout << endl << "=== MANUAL DEBUGGING ===" << endl;
    
    // Print the castling rights 
    cout << "Castling rights: " << pos.get_castling_rights() << endl;
    
    // Print en passant square
    cout << "En passant: " << pos.get_ep_square() << endl;
}

int main() {
    cout << "Pawn lookup tables initialized successfully." << endl;
    cout << "Initializing bitboard attack tables..." << endl;
    
    check_position_status();
    
    return 0;
}