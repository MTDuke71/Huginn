#include "../src/minimal_search.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

struct TestPosition {
    std::string fen;
    std::string description;
    std::vector<std::string> good_moves;
    std::vector<std::string> bad_moves;
    int expected_eval_range_min;
    int expected_eval_range_max;
};

struct MoveResult {
    int from, to;
    int eval;
    bool is_good_move;
    bool is_bad_move;
    std::string move_notation;
};

class MoveEvaluationTester {
private:
    std::vector<TestPosition> test_positions;
    Huginn::MinimalEngine engine;  // Engine for evaluation
    
    std::string square_to_notation(int sq) {
        if (sq < 21 || sq > 98) return "invalid";
        File f = file_of(sq);
        Rank r = rank_of(sq);
        if (f == File::None || r == Rank::None) return "invalid";
        
        char file_char = 'a' + static_cast<int>(f);
        char rank_char = '1' + static_cast<int>(r);
        return std::string(1, file_char) + std::string(1, rank_char);
    }
    
    std::string move_to_notation(int from, int to) {
        return square_to_notation(from) + square_to_notation(to);
    }
    
    bool is_move_in_list(const std::string& move_notation, const std::vector<std::string>& move_list) {
        return std::find(move_list.begin(), move_list.end(), move_notation) != move_list.end();
    }
    
public:
    void add_test_position(const TestPosition& pos) {
        test_positions.push_back(pos);
    }
    
    void run_all_tests() {
        std::cout << "=== SYSTEMATIC MOVE EVALUATION TESTING ===\n\n";
        
        int total_tests = 0;
        int passed_tests = 0;
        
        for (size_t i = 0; i < test_positions.size(); ++i) {
            const auto& test = test_positions[i];
            std::cout << "Test " << (i+1) << ": " << test.description << "\n";
            std::cout << "FEN: " << test.fen << "\n";
            
            bool test_passed = run_single_test(test);
            total_tests++;
            if (test_passed) passed_tests++;
            
            std::cout << "Result: " << (test_passed ? "PASSED" : "FAILED") << "\n";
            std::cout << std::string(60, '-') << "\n\n";
        }
        
        std::cout << "=== TEST SUMMARY ===\n";
        std::cout << "Passed: " << passed_tests << "/" << total_tests << "\n";
        std::cout << "Success Rate: " << std::fixed << std::setprecision(1) 
                  << (100.0 * passed_tests / total_tests) << "%\n";
    }
    
private:
    bool run_single_test(const TestPosition& test) {
        Position pos;
        pos.set_from_fen(test.fen);
        
        // Generate all legal moves
        S_MOVELIST legal_moves;
        generate_legal_moves_enhanced(pos, legal_moves);
        
        std::vector<MoveResult> results;
        
        // Evaluate all moves
        for (int i = 0; i < legal_moves.count; ++i) {
            S_MOVE move = legal_moves.moves[i];
            
            Position temp_pos = pos;
            temp_pos.make_move_with_undo(move);
            
            int eval = -engine.evalPosition(temp_pos);
            
            MoveResult result;
            result.from = move.get_from();
            result.to = move.get_to();
            result.eval = eval;
            result.move_notation = move_to_notation(result.from, result.to);
            result.is_good_move = is_move_in_list(result.move_notation, test.good_moves);
            result.is_bad_move = is_move_in_list(result.move_notation, test.bad_moves);
            
            results.push_back(result);
        }
        
        // Sort by evaluation (best to worst)
        std::sort(results.begin(), results.end(), 
                  [](const MoveResult& a, const MoveResult& b) {
                      return a.eval > b.eval;
                  });
        
        // Display results
        std::cout << std::setw(12) << "Move" << std::setw(10) << "Eval" 
                  << std::setw(8) << "Type" << std::setw(15) << "Expected" << "\n";
        std::cout << std::string(45, '-') << "\n";
        
        bool test_passed = true;
        
        for (const auto& result : results) {
            std::string type = "Normal";
            std::string expected = "";
            bool move_correct = true;
            
            if (result.is_good_move) {
                type = "GOOD";
                expected = "High eval";
                // Good moves should be in top 50% of evaluations
                auto pos_iter = std::find_if(results.begin(), results.end(),
                    [&result](const MoveResult& r) { return r.move_notation == result.move_notation; });
                size_t position = std::distance(results.begin(), pos_iter);
                if (position > results.size() / 2) {
                    move_correct = false;
                    test_passed = false;
                }
            } else if (result.is_bad_move) {
                type = "BAD";
                expected = "Low eval";
                // Bad moves should have evaluation below threshold (e.g., -500cp)
                if (result.eval > -500) {
                    move_correct = false;
                    test_passed = false;
                }
            }
            
            // Only show moves we're specifically testing
            if (result.is_good_move || result.is_bad_move) {
                std::cout << std::setw(12) << result.move_notation 
                          << std::setw(10) << result.eval << "cp"
                          << std::setw(8) << type
                          << std::setw(15) << expected
                          << (move_correct ? " ✓" : " ✗") << "\n";
            }
        }
        
        return test_passed;
    }
};

int main() {
    MoveEvaluationTester tester;
    
    // Test 1: Opening position after 1.e4 - should avoid f6
    tester.add_test_position({
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "Opening after 1.e4 - Black to move",
        {"g8f6", "b8c6", "e7e6", "d7d6", "c7c5"}, // Good moves
        {"f7f6"}, // Bad moves
        -100, 100 // Expected eval range for position
    });
    
    // Test 2: Position after 1.Nf3 - should still avoid f6
    tester.add_test_position({
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
        "Opening after 1.Nf3 - Black to move", 
        {"g8f6", "b8c6", "e7e6", "d7d6"},
        {"f7f6"},
        -100, 100
    });
    
    // Test 3: King safety test - exposed king
    tester.add_test_position({
        "rnbq1bnr/pppp1ppp/4k3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQ - 1 3",
        "Black king exposed on e6",
        {"e1g1", "d2d4", "f1c4"}, // Good moves for White
        {}, // No specific bad moves to test
        200, 1000 // Should be winning for White
    });
    
    // Add more test positions as needed...
    
    tester.run_all_tests();
    
    return 0;
}


