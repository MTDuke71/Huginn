#include "../src/hybrid_evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>

class PerftMoveAnalyzer {
public:
    struct MoveStats {
        int count = 0;
        int total_eval = 0;
        int min_eval = INT_MAX;
        int max_eval = INT_MIN;
        std::vector<std::string> positions;
        
        double average_eval() const {
            return count > 0 ? static_cast<double>(total_eval) / count : 0.0;
        }
    };
    
    void analyze_move_patterns(int depth = 2) {
        std::cout << "=== PERFT-STYLE MOVE PATTERN ANALYSIS ===\n\n";
        
        Position pos;
        pos.set_startpos();
        
        std::map<std::string, MoveStats> move_patterns;
        
        analyze_position_recursive(pos, depth, move_patterns, "");
        
        // Display results
        std::cout << "Move Pattern Analysis (depth " << depth << "):\n";
        std::cout << std::string(70, '=') << "\n";
        std::cout << std::setw(15) << "Move Pattern" 
                  << std::setw(8) << "Count"
                  << std::setw(12) << "Avg Eval"
                  << std::setw(12) << "Min Eval" 
                  << std::setw(12) << "Max Eval" << "\n";
        std::cout << std::string(70, '-') << "\n";
        
        // Sort by average evaluation
        std::vector<std::pair<std::string, MoveStats>> sorted_moves;
        for (const auto& [pattern, stats] : move_patterns) {
            sorted_moves.emplace_back(pattern, stats);
        }
        
        std::sort(sorted_moves.begin(), sorted_moves.end(),
                  [](const auto& a, const auto& b) {
                      return a.second.average_eval() > b.second.average_eval();
                  });
        
        for (const auto& [pattern, stats] : sorted_moves) {
            std::cout << std::setw(15) << pattern
                      << std::setw(8) << stats.count
                      << std::setw(12) << std::fixed << std::setprecision(1) << stats.average_eval()
                      << std::setw(12) << stats.min_eval
                      << std::setw(12) << stats.max_eval << "\n";
        }
        
        // Highlight concerning patterns
        std::cout << "\n=== CONCERNING PATTERNS ===\n";
        for (const auto& [pattern, stats] : sorted_moves) {
            if (stats.average_eval() < -500 || pattern.find("f6") != std::string::npos) {
                std::cout << "⚠️  " << pattern << ": avg " << std::fixed << std::setprecision(1) 
                          << stats.average_eval() << "cp (VERY BAD)\n";
            }
        }
    }
    
private:
    void analyze_position_recursive(Position& pos, int depth, 
                                   std::map<std::string, MoveStats>& move_patterns,
                                   const std::string& move_sequence) {
        if (depth <= 0) return;
        
        S_MOVELIST legal_moves;
        generate_legal_moves_enhanced(pos, legal_moves);
        
        for (int i = 0; i < legal_moves.count; ++i) {
            S_MOVE move = legal_moves.moves[i];
            
            // Make the move
            Position temp_pos = pos;
            temp_pos.make_move_with_undo(move);
            
            // Evaluate position
            int eval = -Engine3::HybridEvaluator::evaluate(temp_pos);
            
            // Create move notation
            std::string move_notation = square_to_notation(move.get_from()) + 
                                       square_to_notation(move.get_to());
            
            // Update statistics
            MoveStats& stats = move_patterns[move_notation];
            stats.count++;
            stats.total_eval += eval;
            stats.min_eval = std::min(stats.min_eval, eval);
            stats.max_eval = std::max(stats.max_eval, eval);
            
            // Recurse if depth allows
            if (depth > 1) {
                analyze_position_recursive(temp_pos, depth - 1, move_patterns, 
                                         move_sequence + move_notation + " ");
            }
        }
    }
    
    std::string square_to_notation(int sq) {
        if (sq < 21 || sq > 98) return "invalid";
        File f = file_of(sq);
        Rank r = rank_of(sq);
        if (f == File::None || r == Rank::None) return "invalid";
        
        char file_char = 'a' + static_cast<int>(f);
        char rank_char = '1' + static_cast<int>(r);
        return std::string(1, file_char) + std::string(1, rank_char);
    }
};

int main() {
    PerftMoveAnalyzer analyzer;
    analyzer.analyze_move_patterns(2); // Analyze 2 moves deep
    
    return 0;
}
