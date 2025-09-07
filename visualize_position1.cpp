#include <iostream>
#include <string>

void print_fen_board(const std::string& fen) {
    std::string board_part = fen.substr(0, fen.find(' '));
    
    std::cout << "\n  a b c d e f g h" << std::endl;
    
    int rank = 8;
    int file = 0;
    
    for (char c : board_part) {
        if (c == '/') {
            std::cout << " " << rank << std::endl;
            rank--;
            file = 0;
            if (rank > 0) std::cout << rank << " ";
        } else if (isdigit(c)) {
            int empty_squares = c - '0';
            for (int i = 0; i < empty_squares; i++) {
                std::cout << ". ";
                file++;
            }
        } else {
            if (file == 0) std::cout << rank << " ";
            std::cout << c << " ";
            file++;
        }
    }
    std::cout << " " << rank << std::endl;
    std::cout << "  a b c d e f g h" << std::endl;
}

int main() {
    std::cout << "=== Position 1 Analysis ===\n" << std::endl;
    
    std::string fen1 = "1qr3k1/p2nbppp/bp2p3/3p4/3P4/1P2PNP1/P2Q1PBP/1N2R1K1 b - - 0 1";
    std::string fen2 = "1n2r1k1/p2q1pbp/1p2pnp1/3p4/3P4/BP2P3/P2NBPPP/1QR3K1 w - - 0 1";
    
    std::cout << "Original Position (Black to move):" << std::endl;
    std::cout << "FEN: " << fen1 << std::endl;
    print_fen_board(fen1);
    
    std::cout << "\nMirrored Position (White to move):" << std::endl;
    std::cout << "FEN: " << fen2 << std::endl;
    print_fen_board(fen2);
    
    std::cout << "\n=== Key Differences to Look For ===" << std::endl;
    std::cout << "1. Pawn structure asymmetries" << std::endl;
    std::cout << "2. Piece placement bonuses/penalties" << std::endl;
    std::cout << "3. File evaluation for rooks/queens" << std::endl;
    std::cout << "4. King safety differences" << std::endl;
    
    return 0;
}
