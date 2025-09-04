#include "src/position.hpp"
#include "../src/evaluation.hpp"
#include "src/movegen_enhanced.hpp"
#include "../src/minimal_search.hpp"
#include "src/attack_detection.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>

void analyze_position_detailed(Position pos, const std::string& description) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "FEN: " << pos.to_fen() << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Basic evaluation using stable MinimalEngine
    Huginn::MinimalEngine engine;
    int total_eval = engine.evalPosition(pos);
    std::cout << "Total Evaluation: " << total_eval << " cp" << std::endl;
    
    // Check if king is in check
    int king_square = pos.king_sq[int(pos.side_to_move)];
    bool in_check = (king_square >= 0 && SqAttacked(king_square, pos, !pos.side_to_move));
    std::cout << "King in check: " << (in_check ? "YES" : "NO") << std::endl;
    std::cout << "King square: " << king_square << std::endl;
    
    // Generate all legal moves
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    std::cout << "Legal moves count: " << legal_moves.count << std::endl;
    
    if (legal_moves.count == 0) {
        if (in_check) {
            std::cout << "*** CHECKMATE DETECTED! ***" << std::endl;
        } else {
            std::cout << "*** STALEMATE DETECTED! ***" << std::endl;
        }
    }
    
    // Show all legal moves
    std::cout << "All legal moves:" << std::endl;
    for (int i = 0; i < legal_moves.count; ++i) {
        Position temp_pos = pos;
        temp_pos.make_move_with_undo(legal_moves.moves[i]);
        
        // Check if opponent has any legal moves after this move
        S_MOVELIST opponent_moves;
        generate_legal_moves_enhanced(temp_pos, opponent_moves);
        
        // Check if opponent king is in check
        int opp_king_sq = temp_pos.king_sq[int(temp_pos.side_to_move)];
        bool opp_in_check = (opp_king_sq >= 0 && SqAttacked(opp_king_sq, temp_pos, !temp_pos.side_to_move));
        
        bool delivers_mate = (opponent_moves.count == 0 && opp_in_check);
        
        Huginn::MinimalEngine eval_engine;
        int eval_after = -eval_engine.evalPosition(temp_pos);
        
        std::cout << "  " << (i+1) << ". " << engine.move_to_uci(legal_moves.moves[i]) 
                  << " -> eval: " << eval_after << " cp";
        
        if (delivers_mate) {
            std::cout << " *** DELIVERS MATE! ***";
        } else if (opp_in_check) {
            std::cout << " (gives check, opp has " << opponent_moves.count << " moves)";
        } else {
            std::cout << " (opp has " << opponent_moves.count << " moves)";
        }
        std::cout << std::endl;
    }
}

void test_simple_mate_position() {
    std::cout << "\n=== TESTING SIMPLE MATE IN 1 ===" << std::endl;
    
    // Create a simple mate in 1: Queen + King vs King
    // Position: k7/8/1K6/8/8/8/8/7Q w - - 0 1 (White Queen on h1, Black King on a8, White King on b6)
    Position simple_mate;
    simple_mate.set_from_fen("k7/8/1K6/8/8/8/8/7Q w - - 0 1");
    
    analyze_position_detailed(simple_mate, "SIMPLE MATE IN 1 (Qh8#)");
    
    // Test the mating move Qh8#
    S_MOVELIST moves;
    generate_legal_moves_enhanced(simple_mate, moves);
    
    Huginn::MinimalEngine engine;
    
    for (int i = 0; i < moves.count; ++i) {
        std::string move_str = engine.move_to_uci(moves.moves[i]);
        if (move_str == "h1h8") {
            std::cout << "\nFound Qh8 move! Testing it..." << std::endl;
            Position after_mate = simple_mate;
            after_mate.make_move_with_undo(moves.moves[i]);
            
            analyze_position_detailed(after_mate, "AFTER Qh8 (should be mate)");
            break;
        }
    }
}

int main() {
    std::cout << "=== DETAILED MATE ANALYSIS ===" << std::endl;
    
    // Initialize the engine
    Huginn::init();
    
    // Test 1: Original problematic position
    Position pos1;
    pos1.set_startpos();
    
    Huginn::MinimalEngine engine;
    
    std::vector<std::string> moves = {
        "d2d4", "f7f6", "e2e4", "e8f7", "g1f3", "e7e6", "f1d3", "h7h6", 
        "c2c4", "b8c6", "d4d5", "f7e8", "d5c6", "b7c6", "b1c3", "e8e7", 
        "h2h4", "g7g6", "e4e5", "a8b8", "d3g6", "b8a8", "c1f4", "h6h5", "d1d6"
    };
    
    for (const auto& move_str : moves) {
        S_MOVELIST legal_moves;
        generate_legal_moves_enhanced(pos1, legal_moves);
        
        bool move_found = false;
        for (int i = 0; i < legal_moves.count; ++i) {
            std::string uci_move = engine.move_to_uci(legal_moves.moves[i]);
            if (uci_move == move_str) {
                pos1.make_move_with_undo(legal_moves.moves[i]);
                move_found = true;
                break;
            }
        }
        if (!move_found) {
            std::cout << "Error: Could not find move " << move_str << std::endl;
            return 1;
        }
    }
    
    analyze_position_detailed(pos1, "ORIGINAL MATE POSITION");
    
    // Test 2: The famous "mate in 1" position
    Position pos2;
    pos2.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    analyze_position_detailed(pos2, "FAMOUS MATE IN 1 POSITION");
    
    // Test 3: Simple mate position to verify basic functionality
    test_simple_mate_position();
    
    return 0;
}
