#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_promotion = move.is_promotion;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    return simple_move;
}

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

std::string square_to_string(int square_64) {
    char file = 'a' + (square_64 % 8);
    char rank = '1' + (square_64 / 8);
    return std::string(1, file) + rank;
}

void analyze_castling_obstacles(const BitboardPosition& pos) {
    std::cout << "=== Castling Obstacle Analysis ===" << std::endl;

    // Black king position
    int black_king_pos = -1;
    for (int sq = 0; sq < 64; sq++) {
        if (pos.piece_type_at(sq) == PieceType::King && pos.color_at(sq) == Color::Black) {
            black_king_pos = sq;
            break;
        }
    }

    std::cout << "Black king at: " << square_to_string(black_king_pos) << std::endl;

    // Check squares between king and rooks for kingside castling (e8-g8)
    std::cout << "\nKingside castling (e8-g8) path:" << std::endl;
    for (int sq = 61; sq <= 62; sq++) { // f8, g8
        PieceType piece = pos.piece_type_at(sq);
        if (piece != PieceType::None) {
            Color color = pos.color_at(sq);
            char piece_char = (piece == PieceType::Pawn) ? 'P' :
                             (piece == PieceType::Knight) ? 'N' :
                             (piece == PieceType::Bishop) ? 'B' :
                             (piece == PieceType::Rook) ? 'R' :
                             (piece == PieceType::Queen) ? 'Q' :
                             (piece == PieceType::King) ? 'K' : '?';
            if (color == Color::Black) piece_char = tolower(piece_char);
            std::cout << square_to_string(sq) << ": " << piece_char << std::endl;
        } else {
            std::cout << square_to_string(sq) << ": empty" << std::endl;
        }
    }

    // Check squares between king and rooks for queenside castling (e8-c8)
    std::cout << "\nQueenside castling (e8-c8) path:" << std::endl;
    for (int sq = 57; sq <= 59; sq++) { // b8, c8, d8
        PieceType piece = pos.piece_type_at(sq);
        if (piece != PieceType::None) {
            Color color = pos.color_at(sq);
            char piece_char = (piece == PieceType::Pawn) ? 'P' :
                             (piece == PieceType::Knight) ? 'N' :
                             (piece == PieceType::Bishop) ? 'B' :
                             (piece == PieceType::Rook) ? 'R' :
                             (piece == PieceType::Queen) ? 'Q' :
                             (piece == PieceType::King) ? 'K' : '?';
            if (color == Color::Black) piece_char = tolower(piece_char);
            std::cout << square_to_string(sq) << ": " << piece_char << std::endl;
        } else {
            std::cout << square_to_string(sq) << ": empty" << std::endl;
        }
    }

    // Check if squares are under attack
    std::cout << "\nSquare attack analysis:" << std::endl;

    // For kingside castling, check f8 and g8
    std::cout << "f8 under attack by white: " << (pos.is_square_attacked(61, Color::White) ? "YES" : "NO") << std::endl;
    std::cout << "g8 under attack by white: " << (pos.is_square_attacked(62, Color::White) ? "YES" : "NO") << std::endl;

    // For queenside castling, check c8, d8
    std::cout << "c8 under attack by white: " << (pos.is_square_attacked(58, Color::White) ? "YES" : "NO") << std::endl;
    std::cout << "d8 under attack by white: " << (pos.is_square_attacked(59, Color::White) ? "YES" : "NO") << std::endl;

    std::cout << "e8 (king) under attack by white: " << (pos.is_square_attacked(60, Color::White) ? "YES" : "NO") << std::endl;
}

void compare_positions_after_knight_moves() {
    std::cout << "=== Comparing Positions After Knight Moves ===" << std::endl;

    BitboardPosition pos1, pos2;
    pos1.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    pos2.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    // Make e5f7 move
    SimpleBitboardMove move1;
    move1.from_64 = 36; // e5
    move1.to_64 = 45;   // f7
    move1.is_capture = true;
    move1.is_promotion = false;
    move1.is_ep_capture = false;
    move1.is_castling = false;
    move1.promotion_type = PieceType::None;

    BitboardPosition::UndoInfo undo1 = pos1.make_move_with_undo(move1);

    // Make e5g6 move
    SimpleBitboardMove move2;
    move2.from_64 = 36; // e5
    move2.to_64 = 46;   // g6
    move2.is_capture = true;
    move2.is_promotion = false;
    move2.is_ep_capture = false;
    move2.is_castling = false;
    move2.promotion_type = PieceType::None;

    BitboardPosition::UndoInfo undo2 = pos2.make_move_with_undo(move2);

    std::cout << "\n=== After e5f7 ===" << std::endl;
    std::cout << "FEN: " << pos1.to_fen() << std::endl;
    analyze_castling_obstacles(pos1);

    std::cout << "\n=== After e5g6 ===" << std::endl;
    std::cout << "FEN: " << pos2.to_fen() << std::endl;
    analyze_castling_obstacles(pos2);

    // Compare the key squares
    std::cout << "\n=== Key Differences ===" << std::endl;

    // Check what pieces are on the critical squares
    std::cout << "f7 after e5f7: ";
    PieceType f7_piece = pos1.piece_type_at(45);
    if (f7_piece != PieceType::None) {
        char piece_char = (f7_piece == PieceType::Knight) ? 'N' : '?';
        if (pos1.color_at(45) == Color::Black) piece_char = tolower(piece_char);
        std::cout << piece_char << std::endl;
    } else {
        std::cout << "empty" << std::endl;
    }

    std::cout << "g6 after e5g6: ";
    PieceType g6_piece = pos2.piece_type_at(46);
    if (g6_piece != PieceType::None) {
        char piece_char = (g6_piece == PieceType::Knight) ? 'N' : '?';
        if (pos2.color_at(46) == Color::Black) piece_char = tolower(piece_char);
        std::cout << piece_char << std::endl;
    } else {
        std::cout << "empty" << std::endl;
    }

    // Check pieces that might be attacking castling squares differently
    std::cout << "\nPieces affecting castling paths:" << std::endl;
    std::cout << "Bishop on g7 in both positions" << std::endl;
    std::cout << "Queen on e7 in both positions" << std::endl;
    std::cout << "Knight now on f7 vs g6 - different attack patterns" << std::endl;
}

int main() {
    compare_positions_after_knight_moves();
    return 0;
}