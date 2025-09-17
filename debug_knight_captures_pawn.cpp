#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>

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

void analyze_knight_captures_pawn(const std::string& fen, const std::string& move_name, uint64_t expected_nodes) {
    std::cout << "\n=== Analyzing " << move_name << " ===" << std::endl;

    BitboardPosition pos;
    pos.set_from_fen(fen);

    std::cout << "Position: " << fen << std::endl;

    // Find the specific move
    BitboardMoveList moves;
    generate_all_moves(pos, moves);

    SimpleBitboardMove target_move;
    bool found = false;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (move_to_string(simple_move) == move_name) {
            target_move = simple_move;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "ERROR: Move " << move_name << " not found!" << std::endl;
        return;
    }

    // Analyze the move in detail
    std::cout << "Move details: from=" << target_move.from_64 << " to=" << target_move.to_64 << std::endl;
    std::cout << "Piece at from: " << piece_to_char(pos.piece_at(target_move.from_64)) << std::endl;
    std::cout << "Piece at to: " << piece_to_char(pos.piece_at(target_move.to_64)) << std::endl;
    std::cout << "Move flags: capture=" << target_move.is_capture
              << " ep=" << target_move.is_ep_capture
              << " castle=" << target_move.is_castling
              << " promo=" << target_move.is_promotion << std::endl;

    // Check if it's really a knight captures pawn
    if (pos.piece_type_at(target_move.from_64) != PieceType::Knight) {
        std::cout << "WARNING: Not a knight move!" << std::endl;
    }
    if (pos.piece_type_at(target_move.to_64) != PieceType::Pawn) {
        std::cout << "WARNING: Not capturing a pawn!" << std::endl;
    }

    // Make the move
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(target_move);
    std::cout << "Position after move: " << pos.to_fen() << std::endl;

    // Test perft at different depths
    std::cout << "\nPerft analysis:" << std::endl;
    for (int depth = 1; depth <= 3; ++depth) {
        uint64_t nodes = debug_perft(pos, depth);
        std::cout << "Depth " << depth << ": " << nodes << " nodes" << std::endl;

        if (depth == 3 && expected_nodes > 0) {
            int64_t diff = (int64_t)nodes - (int64_t)expected_nodes;
            std::cout << "Expected: " << expected_nodes << ", Difference: " << diff << std::endl;
        }
    }

    // Analyze the position for special features
    std::cout << "\nPosition analysis after " << move_name << ":" << std::endl;
    std::cout << "En passant square: " << pos.ep_square_64 << std::endl;
    std::cout << "Castling rights: " << pos.castling_rights << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;

    // Check for any pawns that could be involved in en passant
    if (pos.ep_square_64 != -1) {
        std::cout << "WARNING: En passant square is set after knight captures pawn!" << std::endl;
    }

    // Count pieces to see if material is correct
    uint64_t white_pawn_bb = pos.get_pieces(Color::White, PieceType::Pawn);
    uint64_t black_pawn_bb = pos.get_pieces(Color::Black, PieceType::Pawn);
    int white_pawns = 0;
    int black_pawns = 0;

    // Count bits manually
    while (white_pawn_bb) { white_pawns++; white_pawn_bb &= white_pawn_bb - 1; }
    while (black_pawn_bb) { black_pawns++; black_pawn_bb &= black_pawn_bb - 1; }
    std::cout << "Pawns remaining: White=" << white_pawns << " Black=" << black_pawns << std::endl;

    // Generate moves from this position and look for any unusual patterns
    BitboardMoveList next_moves;
    generate_legal_moves(pos, next_moves);
    std::cout << "Legal moves from this position: " << next_moves.size() << std::endl;

    // Look for any en passant moves (there shouldn't be any after knight captures pawn)
    int ep_moves = 0;
    for (const auto& move : next_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        if (sm.is_ep_capture) {
            ep_moves++;
            std::cout << "WARNING: Found en passant move: " << move_to_string(sm) << std::endl;
        }
    }
    if (ep_moves > 0) {
        std::cout << "WARNING: " << ep_moves << " en passant moves found after knight captures pawn!" << std::endl;
    }

    // Unmake the move and verify
    pos.unmake_move(target_move, undo_info);

    BitboardPosition expected_pos;
    expected_pos.set_from_fen(fen);

    if (pos.to_fen() != expected_pos.to_fen()) {
        std::cout << "ERROR: Position not restored correctly!" << std::endl;
        std::cout << "Expected: " << expected_pos.to_fen() << std::endl;
        std::cout << "Actual:   " << pos.to_fen() << std::endl;
    } else {
        std::cout << "✓ Position restored correctly" << std::endl;
    }
}

int main() {
    std::cout << "=== Knight Captures Pawn Debug Analysis ===" << std::endl;

    // Test the two problematic knight captures pawn moves from Kiwipete
    analyze_knight_captures_pawn(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "e5f7",
        88799);

    analyze_knight_captures_pawn(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "e5g6",
        83866);

    // Test some other knight captures pawn scenarios for comparison
    std::cout << "\n=== Testing other knight captures scenarios ===" << std::endl;

    // Simple position with knight captures pawn
    analyze_knight_captures_pawn(
        "rnbqkbnr/pppp1ppp/8/4p3/3P4/5N2/PPP1PPPP/RNBQKB1R b KQkq - 1 2",
        "e5d4",  // Black pawn captures white pawn (not knight, but for comparison)
        0);

    return 0;
}