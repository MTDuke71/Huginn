#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "src/position.hpp"
#include "src/movegen.hpp"
#include "src/board120.hpp"
#include "src/init.hpp"
#include "src/move.hpp"

// Convert algebraic notation to from/to squares
std::pair<int, int> parse_algebraic(const std::string& move_str) {
    if (move_str.length() != 4) return {-1, -1};
    
    int from_file = move_str[0] - 'a';
    int from_rank = move_str[1] - '1';
    int to_file = move_str[2] - 'a';
    int to_rank = move_str[3] - '1';
    
    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
        to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) {
        return {-1, -1};
    }
    
    int from_sq = sq(static_cast<File>(from_file), static_cast<Rank>(from_rank));
    int to_sq = sq(static_cast<File>(to_file), static_cast<Rank>(to_rank));
    
    return {from_sq, to_sq};
}

// Convert square to algebraic notation  
std::string sq_to_algebraic(int square) {
    File f = file_of(square);
    Rank r = rank_of(square);
    std::string result;
    result += char('a' + int(f));
    result += char('1' + int(r));
    return result;
}

int main() {
    Huginn::init();
    
    std::cout << "=== Debugging position after e1g1 (White kingside castle) in Kiwipete ===" << std::endl;
    
    // Set up Kiwipete position
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cout << "Failed to parse Kiwipete FEN" << std::endl;
        return 1;
    }
    
    // Make the e1g1 (White kingside castle) move
    auto e1g1_coords = parse_algebraic("e1g1");
    if (e1g1_coords.first == -1) {
        std::cout << "Failed to parse e1g1 move" << std::endl;
        return 1;
    }

    // Find the e1g1 move in the legal moves
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);

    S_MOVE e1g1_move;
    bool found_move = false;
    for (const auto& move : legal_moves.v) {
        if (move.get_from() == e1g1_coords.first && move.get_to() == e1g1_coords.second && move.is_castle()) {
            e1g1_move = move;
            found_move = true;
            break;
        }
    }

    if (!found_move) {
        std::cout << "Could not find e1g1 (castle) move in legal moves!" << std::endl;
        return 1;
    }
    pos.make_move_with_undo(e1g1_move);
    std::cout << "Applied e1g1 (castle) move. New position FEN: " << pos.to_fen() << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;

    // Generate moves in the new position
    MoveList moves_after_castle;
    generate_legal_moves(pos, moves_after_castle);

    std::cout << "Our engine found " << moves_after_castle.v.size() << " moves after castling" << std::endl;

    // Print all moves after castling
    std::cout << "\nMoves after castling:" << std::endl;
    for (size_t i = 0; i < moves_after_castle.v.size(); ++i) {
        const auto& move = moves_after_castle.v[i];
        std::string move_str = sq_to_algebraic(move.get_from()) + sq_to_algebraic(move.get_to());
        std::cout << (i+1) << ". " << move_str;
        if (move.is_castle()) std::cout << " (castle)";
        if (move.is_promotion()) std::cout << " (promotion)";
        if (move.is_en_passant()) std::cout << " (en passant)";
        std::cout << std::endl;
    }

    pos.undo_move();
    
    return 0;
}
