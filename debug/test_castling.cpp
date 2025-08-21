#include <iostream>
#include <string>
#include "src/position.hpp"
#include "src/movegen.hpp"
#include "src/init.hpp"

std::string move_to_str(const S_MOVE& move) {
    File f1 = file_of(move.get_from());
    Rank r1 = rank_of(move.get_from());
    File f2 = file_of(move.get_to());
    Rank r2 = rank_of(move.get_to());
    std::string s;
    s += char('a' + int(f1));
    s += char('1' + int(r1));
    s += char('a' + int(f2));
    s += char('1' + int(r2));
    return s;
}

int main() {
    Huginn::init();
    Position pos;
    std::string fen = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
    if (!pos.set_from_fen(fen)) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    std::cout << "Initial FEN: " << pos.to_fen() << std::endl;
    std::cout << "Initial castling rights: " << int(pos.castling_rights) << std::endl;

    MoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Generated moves:" << std::endl;
    for (const auto& move : moves.v) {
        std::string move_str = move_to_str(move);
        std::cout << move_str;
        if (move.is_castle()) std::cout << " (CASTLE)";
        std::cout << std::endl;
    }
    
    S_MOVE e1c1;
    bool found = false;
    for (const auto& move : moves.v) {
        if (move_to_str(move) == "e1c1" && move.is_castle()) {
            e1c1 = move;
            found = true;
            std::cout << "Found e1c1 castling move. Internal move value: " << std::hex << move.move << std::dec << std::endl;
            std::cout << "is_castle() returns: " << move.is_castle() << std::endl;
            break;
        }
    }
    if (!found) {
        std::cout << "Did not find e1c1 castling move!" << std::endl;
        return 1;
    }
    
    std::cout << "About to call make_move_with_undo. Move value: " << std::hex << e1c1.move << std::dec << std::endl;
    std::cout << "is_castle() before call: " << e1c1.is_castle() << std::endl;
    
    S_UNDO undo;
    pos.make_move_with_undo(e1c1, undo);
    std::cout << "After e1c1 FEN: " << pos.to_fen() << std::endl;
    std::cout << "After e1c1 castling rights: " << int(pos.castling_rights) << std::endl;
    // Check rook position
    std::cout << "Piece on c1: " << to_char(pos.at(sq(File::C, Rank::R1))) << std::endl;
    std::cout << "Piece on a1: " << to_char(pos.at(sq(File::A, Rank::R1))) << std::endl;
    return 0;
}
