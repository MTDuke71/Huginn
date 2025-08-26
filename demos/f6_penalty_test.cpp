#include "evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

int main() {
    std::cout << "Testing f6 Move Evaluation\n";
    std::cout << "=========================\n\n";
    
    // Test position after 1.Nf3 (Black to move)
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1");
    
    std::cout << "Position after 1.Nf3 - Black to move\n";
    std::cout << "Testing key moves:\n\n";
    
    // Test specific moves
    struct TestMove {
        std::string fen;
        std::string move_name;
    };
    
    std::vector<TestMove> test_moves = {
        {"rnbqkbnr/ppppp1pp/5p2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2", "1...f6"},
        {"rnbqkbnr/pppppppp/5n2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2", "1...Nf6"},
        {"rnbqkbnr/pppp1ppp/4p3/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2", "1...e6"},
        {"rnbqkbnr/ppp1pppp/3p4/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2", "1...d6"},
        {"rnbqkbnr/pp1ppppp/2p5/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2", "1...c6"},
        {"r1bqkbnr/pppppppp/2n5/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2", "1...Nc6"}
    };
    
    std::cout << std::setw(12) << "Move" << std::setw(15) << "Evaluation" << std::setw(12) << "Penalty" << std::endl;
    std::cout << std::string(39, '-') << std::endl;
    
    int baseline_eval = Evaluation::evaluate_position(pos);
    
    for (const auto& test : test_moves) {
        Position test_pos;
        test_pos.set_from_fen(test.fen);
        
        int eval = Evaluation::evaluate_position(test_pos);
        int penalty = eval - baseline_eval;
        
        std::cout << std::setw(12) << test.move_name 
                  << std::setw(15) << eval << "cp"
                  << std::setw(12) << penalty << "cp" << std::endl;
    }
    
    std::cout << "\nBaseline (after 1.Nf3): " << baseline_eval << "cp\n";
    
    return 0;
}
