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

const char* piece_to_char(Piece piece) {
    switch (piece) {
        case Piece::None: return "None";
        case Piece::WhitePawn: return "WP";
        case Piece::WhiteKnight: return "WN";
        case Piece::WhiteBishop: return "WB";
        case Piece::WhiteRook: return "WR";
        case Piece::WhiteQueen: return "WQ";
        case Piece::WhiteKing: return "WK";
        case Piece::BlackPawn: return "BP";
        case Piece::BlackKnight: return "BN";
        case Piece::BlackBishop: return "BB";
        case Piece::BlackRook: return "BR";
        case Piece::BlackQueen: return "BQ";
        case Piece::BlackKing: return "BK";
        default: return "Unknown";
    }
}

const char* piece_type_to_char(PieceType pt) {
    switch (pt) {
        case PieceType::None: return "None";
        case PieceType::Pawn: return "Pawn";
        case PieceType::Knight: return "Knight";
        case PieceType::Bishop: return "Bishop";
        case PieceType::Rook: return "Rook";
        case PieceType::Queen: return "Queen";
        case PieceType::King: return "King";
        default: return "Unknown";
    }
}

// Simplified perft for debugging
uint64_t debug_perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        total_nodes += debug_perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== Detailed Unmake Move Debug ===" << std::endl;

    // Start with Kiwipete position
    BitboardPosition pos;
    pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    std::cout << "Initial position: " << pos.to_fen() << std::endl;

    // Find the e5f7 move
    BitboardMoveList moves;
    generate_all_moves(pos, moves);

    SimpleBitboardMove e5f7_move;
    bool found = false;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (move_to_string(simple_move) == "e5f7") {
            e5f7_move = simple_move;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "ERROR: e5f7 move not found!" << std::endl;
        return 1;
    }

    // Debug the move in detail
    std::cout << "\n=== Making e5f7 move ===" << std::endl;
    std::cout << "Move: from=" << e5f7_move.from_64 << " to=" << e5f7_move.to_64 << std::endl;
    std::cout << "Piece at from (e5): " << piece_to_char(pos.piece_at(e5f7_move.from_64)) << std::endl;
    std::cout << "Piece at to (f7): " << piece_to_char(pos.piece_at(e5f7_move.to_64)) << std::endl;
    std::cout << "Move flags: capture=" << e5f7_move.is_capture
              << " ep=" << e5f7_move.is_ep_capture
              << " castle=" << e5f7_move.is_castling
              << " promo=" << e5f7_move.is_promotion << std::endl;

    // Make the move and capture undo info
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(e5f7_move);

    std::cout << "\n=== After making move ===" << std::endl;
    std::cout << "Position: " << pos.to_fen() << std::endl;
    std::cout << "Captured piece in undo_info: " << piece_to_char(undo_info.captured_piece) << std::endl;

    // Do some perft to trigger the bug
    std::cout << "\n=== Running perft (this might cause corruption) ===" << std::endl;
    uint64_t nodes = debug_perft(pos, 3);
    std::cout << "Perft depth 3: " << nodes << " nodes" << std::endl;

    std::cout << "\n=== Before unmaking move ===" << std::endl;
    std::cout << "Position before unmake: " << pos.to_fen() << std::endl;
    std::cout << "Piece at to (f7): " << piece_to_char(pos.piece_at(e5f7_move.to_64)) << std::endl;
    std::cout << "Expected to restore to from (e5): Knight" << std::endl;

    // Unmake the move
    std::cout << "\n=== Unmaking move ===" << std::endl;
    pos.unmake_move(e5f7_move, undo_info);

    std::cout << "\n=== After unmaking move ===" << std::endl;
    std::cout << "Position: " << pos.to_fen() << std::endl;
    std::cout << "Piece at from (e5): " << piece_to_char(pos.piece_at(e5f7_move.from_64)) << std::endl;
    std::cout << "Piece type at from (e5): " << piece_type_to_char(pos.piece_type_at(e5f7_move.from_64)) << std::endl;

    // Check if it's correct
    BitboardPosition expected_pos;
    expected_pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    std::cout << "\n=== Verification ===" << std::endl;
    std::cout << "Expected: " << expected_pos.to_fen() << std::endl;
    std::cout << "Actual:   " << pos.to_fen() << std::endl;
    std::cout << "Match: " << (pos.to_fen() == expected_pos.to_fen() ? "YES" : "NO") << std::endl;

    return 0;
}