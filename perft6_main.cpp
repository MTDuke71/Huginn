// Minimal perft6 driver for Huginn EngineX
#include "board.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    // FEN for starting position
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board board;
    board.set_from_fen(fen);
    
    // Run perft to depth 6
    uint64_t nodes = perft(board, 6);
    std::cout << "Perft(6) nodes: " << nodes << std::endl;
    return 0;
}
