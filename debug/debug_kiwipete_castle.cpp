#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "../src/position.hpp"
#include "movegen_enhanced.hpp"
#include "../src/init.hpp"
#include "../src/move.hpp"

// Helper: Convert square to algebraic notation
std::string sq_to_algebraic(int square) {
    int file = (square % 10) - 1;
    int rank = (square / 10) - 2;
    std::string result;
    result += char('a' + file);
    result += char('1' + rank);
    return result;
}

// Perft function
uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList moves;
    generate_legal_moves(pos, moves);
    uint64_t nodes = 0;
    for (const auto& move : moves.v) {
        std::string move_alg = sq_to_algebraic(move.get_from()) + sq_to_algebraic(move.get_to());
        if (move_alg == "h1d1" || move_alg == "c1d1") {
            #ifdef DEBUG_CASTLING
            std::cout << "[PERFT] Depth " << depth << " before move " << move_alg
                      << " rights: " << int(pos.castling_rights) << (move.is_castle() ? " (castling)" : "") << std::endl;
            std::cout << "[PERFT] White King list: ";
            for (int i = 0; i < pos.pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)]; ++i)
                std::cout << pos.pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)][i] << " ";
            std::cout << " | White Rook list: ";
            for (int i = 0; i < pos.pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)]; ++i)
                std::cout << pos.pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)][i] << " ";
            std::cout << std::endl;
            #endif
        }
        pos.make_move_with_undo(move);
        if (move_alg == "h1d1" || move_alg == "c1d1") {
            #ifdef DEBUG_CASTLING
            std::cout << "[PERFT] Depth " << depth << " after move " << move_alg
                      << " rights: " << int(pos.castling_rights) << std::endl;
            std::cout << "[PERFT] White King list: ";
            for (int i = 0; i < pos.pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)]; ++i)
                std::cout << pos.pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)][i] << " ";
            std::cout << " | White Rook list: ";
            for (int i = 0; i < pos.pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)]; ++i)
                std::cout << pos.pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)][i] << " ";
            std::cout << std::endl;
            #endif
        }
        nodes += perft(pos, depth - 1);
        pos.undo_move();
        if (move_alg == "h1d1" || move_alg == "c1d1") {
            #ifdef DEBUG_CASTLING
            std::cout << "[PERFT] Depth " << depth << " after undo " << move_alg
                      << " rights: " << int(pos.castling_rights) << std::endl;
            std::cout << "[PERFT] White King list: ";
            for (int i = 0; i < pos.pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)]; ++i)
                std::cout << pos.pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)][i] << " ";
            std::cout << " | White Rook list: ";
            for (int i = 0; i < pos.pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)]; ++i)
                std::cout << pos.pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)][i] << " ";
            std::cout << std::endl;
            #endif
        }
    }
    return nodes;
}

std::string move_to_algebraic(const S_MOVE& move) {
    File from_file = file_of(move.get_from());
    Rank from_rank = rank_of(move.get_from());
    File to_file = file_of(move.get_to());
    Rank to_rank = rank_of(move.get_to());
    std::string result;
    result += char('a' + int(from_file));
    result += char('1' + int(from_rank));
    result += char('a' + int(to_file));
    result += char('1' + int(to_rank));
    return result;
}

int main() {
    Huginn::init();
    std::cout << "=== Kiwipete before White castles kingside: Perft breakdown at depth 3 ===\n";
    // Set up Kiwipete position (pre-castle)
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cout << "Failed to parse Kiwipete FEN" << std::endl;
        return 1;
    }
    std::cout << "Set position to: " << kiwipete_fen << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;

    // Generate all legal moves in the position
    MoveList moves;
    generate_legal_moves(pos, moves);

    std::map<std::string, uint64_t> move_counts;
    uint64_t total_nodes = 0;
    for (const auto& move : moves.v) {
        std::string move_alg = sq_to_algebraic(move.get_from()) + sq_to_algebraic(move.get_to());
        pos.make_move_with_undo(move);
        uint64_t count = perft(pos, 2); // depth 3: root move + 2 more
        pos.undo_move();
        move_counts[move_alg] = count;
        total_nodes += count;
    }
    std::cout << "\nPerft breakdown at depth 3:" << std::endl;
    for (const auto& entry : move_counts) {
        std::cout << entry.first << " - " << entry.second << std::endl;
    }
    std::cout << "\nTotal: " << total_nodes << std::endl;

    // --- Now do breakdown after e1c1 (White castles queenside) ---
    std::cout << "\n=== After e1c1 (White castles queenside): Black to move, castling rights ===" << std::endl;
    Position after_e1c1;
    after_e1c1.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/2KR3R b kq - 1 1");
    std::cout << "FEN: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/2KR3R b kq - 1 1" << std::endl;
    std::cout << "Castling rights: " << ((after_e1c1.castling_rights & CASTLE_BK) ? "k" : "") << ((after_e1c1.castling_rights & CASTLE_BQ) ? "q" : "") << std::endl;

    MoveList black_moves;
    generate_legal_moves(after_e1c1, black_moves);
    std::cout << "\nDepth 2 breakdown for Black after e1c1:" << std::endl;
    int total_black_nodes = 0;
    for (const auto& move : black_moves.v) {
        // Print castling rights before move
        std::cout << "\nMove: " << sq_to_algebraic(move.get_from()) << sq_to_algebraic(move.get_to());
        std::cout << " | Before rights: " << ((after_e1c1.castling_rights & CASTLE_BK) ? "k" : "") << ((after_e1c1.castling_rights & CASTLE_BQ) ? "q" : "");

        // Save current rights
        uint8_t rights_before = after_e1c1.castling_rights;
        PieceType moving_type = type_of(after_e1c1.at(move.get_from()));
        PieceType captured_type = type_of(after_e1c1.at(move.get_to()));

        after_e1c1.make_move_with_undo(move);

        // Print debug info for king/rook moves/captures
        if (moving_type == PieceType::King) {
            std::cout << " | King move";
        }
        if (moving_type == PieceType::Rook) {
            std::cout << " | Rook move";
        }
        if (captured_type == PieceType::Rook) {
            std::cout << " | Rook captured";
        }
        if (move.is_castle()) {
            std::cout << " | Castling move";
        }

        // Print castling rights after move
        std::cout << " | After rights: " << ((after_e1c1.castling_rights & CASTLE_BK) ? "k" : "") << ((after_e1c1.castling_rights & CASTLE_BQ) ? "q" : "");

        // Perft at depth 1 (number of replies)
        MoveList reply_moves;
        generate_legal_moves(after_e1c1, reply_moves);
        std::cout << " | Replies: " << reply_moves.v.size() << std::endl;
        total_black_nodes += reply_moves.v.size();

        after_e1c1.undo_move();
    }
    std::cout << "\nTotal replies (depth 2 nodes): " << total_black_nodes << std::endl;
    return 0;
}
