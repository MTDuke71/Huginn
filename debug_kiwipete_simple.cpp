#include "src/position.hpp"
#include "src/movegen.hpp"
#include <iostream>

std::string square_to_string(int square) {
    if (square < 0 || square >= 120) return "??";
    int file = square % 10;
    int rank = square / 10;
    if (file < 1 || file > 8 || rank < 2 || rank > 9) return "??";
    char file_char = 'a' + (file - 1);
    char rank_char = '1' + (rank - 2);
    return std::string(1, file_char) + std::string(1, rank_char);
}

std::string move_to_string(const S_MOVE& move) {
    std::string result = square_to_string(move.get_from()) + square_to_string(move.get_to());
    if (move.is_promotion()) {
        char promo_char = '?';
        switch (move.get_promoted()) {
            case PieceType::Queen: promo_char = 'q'; break;
            case PieceType::Rook: promo_char = 'r'; break;
            case PieceType::Bishop: promo_char = 'b'; break;
            case PieceType::Knight: promo_char = 'n'; break;
            default: break;
        }
        result += promo_char;
    }
    return result;
}

int main() {
    Position pos;
    std::string kiwipete_fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    
    // Manual FEN parsing for debug - just test a simpler position first
    pos.reset();
    pos.set_startpos();
    
    std::cout << "Testing with start position first:" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    std::cout << "\nGenerated " << list.v.size() << " legal moves:" << std::endl;
    for (size_t i = 0; i < list.v.size() && i < 10; ++i) {
        const auto& move = list.v[i];
        std::cout << i+1 << ". " << move_to_string(move) << std::endl;
    }
    
    return 0;
}
