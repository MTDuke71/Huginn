#include <iostream>
#include <string>
#include "src/position.h"
#include "src/movegen.h"
#include "src/types.h"
#include "src/uci.h"

void test_d7d5_move() {
    std::cout << "=== Testing d7d5 Move Issue ===" << std::endl;
    
    // Create position
    Position pos;
    
    // Set to starting position
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos.set(fen);
    
    std::cout << "Starting position FEN: " << pos.fen() << std::endl;
    
    // Apply e2e4 first
    Move e2e4 = UCI::to_move(pos, "e2e4");
    if (e2e4 == MOVE_NONE) {
        std::cout << "ERROR: Failed to parse e2e4" << std::endl;
        return;
    }
    
    std::cout << "Parsed e2e4 move: " << UCI::move(e2e4, false) << std::endl;
    
    // Check if e2e4 is legal
    MoveList<LEGAL> legal_moves;
    legal_moves.generate(pos);
    bool e2e4_legal = false;
    for (const auto& m : legal_moves) {
        if (m.move == e2e4) {
            e2e4_legal = true;
            break;
        }
    }
    
    std::cout << "e2e4 is legal: " << (e2e4_legal ? "YES" : "NO") << std::endl;
    
    if (e2e4_legal) {
        // Apply e2e4
        StateInfo st1;
        pos.do_move(e2e4, st1);
        std::cout << "After e2e4: " << pos.fen() << std::endl;
        
        // Now try d7d5
        Move d7d5 = UCI::to_move(pos, "d7d5");
        if (d7d5 == MOVE_NONE) {
            std::cout << "ERROR: Failed to parse d7d5" << std::endl;
            return;
        }
        
        std::cout << "Parsed d7d5 move: " << UCI::move(d7d5, false) << std::endl;
        
        // Check if d7d5 is legal
        MoveList<LEGAL> legal_moves2;
        legal_moves2.generate(pos);
        bool d7d5_legal = false;
        
        std::cout << "Legal moves in current position:" << std::endl;
        for (const auto& m : legal_moves2) {
            std::string move_str = UCI::move(m.move, false);
            std::cout << "  " << move_str;
            if (m.move == d7d5) {
                d7d5_legal = true;
                std::cout << " <-- THIS IS d7d5!";
            }
            std::cout << std::endl;
        }
        
        std::cout << "d7d5 is legal: " << (d7d5_legal ? "YES" : "NO") << std::endl;
        
        if (d7d5_legal) {
            // Apply d7d5
            StateInfo st2;
            std::cout << "Applying d7d5..." << std::endl;
            pos.do_move(d7d5, st2);
            std::cout << "After d7d5: " << pos.fen() << std::endl;
        } else {
            std::cout << "d7d5 is not legal - this is the bug!" << std::endl;
            
            // Let's see what move parsing gives us for d7d5
            std::cout << "Move details for d7d5:" << std::endl;
            std::cout << "  From square: " << square_name(from_sq(d7d5)) << std::endl;
            std::cout << "  To square: " << square_name(to_sq(d7d5)) << std::endl;
            std::cout << "  Move type: " << type_of(d7d5) << std::endl;
            
            // Check what piece is on d7
            Square d7 = SQ_D7;
            Piece piece_on_d7 = pos.piece_on(d7);
            std::cout << "  Piece on d7: " << piece_on_d7 << " (should be black pawn = " << make_piece(BLACK, PAWN) << ")" << std::endl;
            
            // Check what's on d5
            Square d5 = SQ_D5;
            Piece piece_on_d5 = pos.piece_on(d5);
            std::cout << "  Piece on d5: " << piece_on_d5 << " (should be empty = " << NO_PIECE << ")" << std::endl;
        }
    }
}

int main() {
    UCI::init();
    test_d7d5_move();
    return 0;
}