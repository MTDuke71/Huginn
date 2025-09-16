#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

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

// Convert 64-square to algebraic notation
string square64_to_algebraic(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

// Convert move to string
string move_to_string_detailed(const BitboardMoveList::BitboardMove& move) {
    string result = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
    if (move.is_promotion) {
        switch (move.promotion_type) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    return result;
}

int main() {
    cout << "=== En Passant Capture Test: After a2a3 a7a5 c2c3 ===" << endl;
    
    // Start from position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5 (sets en passant target to a6)
    BitboardMoveList moves1;
    generate_all_moves(pos, moves1);
    
    for (const auto& move : moves1.moves) {
        if (move_to_string_detailed(move) == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo1 = pos.make_move_with_undo(simple_move);
                
                cout << "After a2a3 a7a5:" << endl;
                cout << "En passant target: ";
                if (pos.ep_square_64 == -1) {
                    cout << "None";
                } else {
                    int file = pos.ep_square_64 % 8;
                    int rank = pos.ep_square_64 / 8;
                    cout << char('a' + file) << char('1' + rank);
                }
                cout << endl;
                
                // Now make c2c3
                BitboardMoveList moves2;
                generate_all_moves(pos, moves2);
                
                for (const auto& move2 : moves2.moves) {
                    if (move_to_string_detailed(move2) == "c2c3") {
                        SimpleBitboardMove simple_move2 = convert_move(move2);
                        if (pos.is_legal_move(simple_move2)) {
                            BitboardPosition::UndoInfo undo2 = pos.make_move_with_undo(simple_move2);
                            
                            cout << endl << "After a2a3 a7a5 c2c3:" << endl;
                            cout << "Now it's Black's turn. Looking for b4xa5 en passant..." << endl;
                            
                            // Check if black can play b7b5 to put pawn on b5, then if white can capture en passant
                            BitboardMoveList black_moves;
                            generate_all_moves(pos, black_moves);
                            
                            cout << "Black moves available: " << black_moves.moves.size() << endl;
                            
                            bool found_b7b5 = false;
                            for (const auto& black_move : black_moves.moves) {
                                if (move_to_string_detailed(black_move) == "b7b5") {
                                    SimpleBitboardMove black_simple = convert_move(black_move);
                                    if (pos.is_legal_move(black_simple)) {
                                        BitboardPosition::UndoInfo black_undo = pos.make_move_with_undo(black_simple);
                                        found_b7b5 = true;
                                        
                                        cout << "After a2a3 a7a5 c2c3 b7b5:" << endl;
                                        cout << "En passant target: ";
                                        if (pos.ep_square_64 == -1) {
                                            cout << "None";
                                        } else {
                                            int file = pos.ep_square_64 % 8;
                                            int rank = pos.ep_square_64 / 8;
                                            cout << char('a' + file) << char('1' + rank);
                                        }
                                        cout << endl;
                                        
                                        // Now check for white en passant captures
                                        BitboardMoveList white_moves;
                                        generate_all_moves(pos, white_moves);
                                        
                                        cout << "White moves available: " << white_moves.moves.size() << endl;
                                        cout << "Looking for en passant captures:" << endl;
                                        
                                        bool found_ep = false;
                                        for (const auto& white_move : white_moves.moves) {
                                            SimpleBitboardMove white_simple = convert_move(white_move);
                                            if (white_simple.is_ep_capture) {
                                                cout << "  Found en passant: " << move_to_string_detailed(white_move) << endl;
                                                found_ep = true;
                                            }
                                        }
                                        
                                        if (!found_ep) {
                                            cout << "  No en passant captures found!" << endl;
                                        }
                                        
                                        pos.unmake_move(black_simple, black_undo);
                                        break;
                                    }
                                }
                            }
                            
                            if (!found_b7b5) {
                                cout << "Could not find b7b5 move" << endl;
                            }
                            
                            pos.unmake_move(simple_move2, undo2);
                            goto done;
                        }
                    }
                }
                
                pos.unmake_move(simple_move, undo1);
                break;
            }
        }
    }
    
    done:
    return 0;
}