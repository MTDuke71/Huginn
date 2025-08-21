#include <iostream>
#include "src/chess_types.hpp"

int main() {
    std::cout << "Piece values:" << std::endl;
    std::cout << "None: " << int(Piece::None) << std::endl;
    std::cout << "WhitePawn: " << int(Piece::WhitePawn) << std::endl;
    std::cout << "WhiteKnight: " << int(Piece::WhiteKnight) << std::endl;
    std::cout << "WhiteBishop: " << int(Piece::WhiteBishop) << std::endl;
    std::cout << "WhiteRook: " << int(Piece::WhiteRook) << std::endl;
    std::cout << "WhiteQueen: " << int(Piece::WhiteQueen) << std::endl;
    std::cout << "WhiteKing: " << int(Piece::WhiteKing) << std::endl;
    std::cout << "BlackPawn: " << int(Piece::BlackPawn) << std::endl;
    std::cout << "BlackKnight: " << int(Piece::BlackKnight) << std::endl;
    std::cout << "BlackBishop: " << int(Piece::BlackBishop) << std::endl;
    std::cout << "BlackRook: " << int(Piece::BlackRook) << std::endl;
    std::cout << "BlackQueen: " << int(Piece::BlackQueen) << std::endl;
    std::cout << "BlackKing: " << int(Piece::BlackKing) << std::endl;
    
    return 0;
}
