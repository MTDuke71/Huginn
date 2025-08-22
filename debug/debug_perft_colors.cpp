#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp" 
#include "init.hpp"

static uint64_t perft_with_debug(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    MoveList list; 
    generate_legal_moves(pos, list);
    
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        State st{};
        
        // Check the piece we're moving
        Piece moving_piece = pos.board[m.get_from()];
        Color moving_color = color_of(moving_piece);
        
        if (moving_color != Color::White && moving_color != Color::Black) {
            std::cout << "Invalid moving piece color detected!" << std::endl;
            std::cout << "From square: " << m.get_from() << std::endl;
            std::cout << "Piece value: " << int(moving_piece) << std::endl;
            std::cout << "Color: " << int(moving_color) << std::endl;
            continue;
        }
        
        // Check captured piece if any
        if (m.is_capture()) {
            Piece captured_piece = pos.board[m.get_to()];
            if (captured_piece != Piece::None) {
                Color captured_color = color_of(captured_piece);
                if (captured_color != Color::White && captured_color != Color::Black) {
                    std::cout << "Invalid captured piece color detected!" << std::endl;
                    std::cout << "To square: " << m.get_to() << std::endl;
                    std::cout << "Piece value: " << int(captured_piece) << std::endl;
                    std::cout << "Color: " << int(captured_color) << std::endl;
                    continue;
                }
            }
        }
        
        make_move(pos, m, st);
        nodes += perft_with_debug(pos, depth-1);
        unmake_move(pos, m, st);
    }
    return nodes;
}

int main() {
    Huginn::init(); // Initialize engine
    
    Position pos; 
    pos.set_startpos();
    
    std::cout << "Testing perft depth 3 with debugging..." << std::endl;
    uint64_t result = perft_with_debug(pos, 3);
    std::cout << "Perft depth 3 result: " << result << " (expected: 8902)" << std::endl;
    
    return 0;
}
