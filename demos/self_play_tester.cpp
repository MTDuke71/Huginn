#include "../Engine3_src/hybrid_evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <chrono>
#include <string>

class SelfPlayTester {
private:
    struct GameResult {
        std::string pgn;
        std::string result; // "1-0", "0-1", "1/2-1/2"
        int move_count;
        bool had_king_walking;
        bool had_f6_move;
        std::string termination_reason;
    };
    
    std::vector<GameResult> game_results;
    
public:
    void run_self_play_games(int num_games = 10, int time_per_move_ms = 1000) {
        std::cout << "=== SELF-PLAY TESTING ===\n";
        std::cout << "Running " << num_games << " games...\n\n";
        
        for (int game = 1; game <= num_games; ++game) {
            std::cout << "Game " << game << "/" << num_games << "... ";
            
            GameResult result = play_single_game(time_per_move_ms);
            game_results.push_back(result);
            
            std::cout << result.result << " (" << result.move_count << " moves)";
            if (result.had_f6_move) std::cout << " [F6 DETECTED!]";
            if (result.had_king_walking) std::cout << " [KING WALKING!]";
            std::cout << "\n";
        }
        
        analyze_results();
    }

private:
    GameResult play_single_game(int time_per_move_ms) {
        // Note: time_per_move_ms is not currently used in this simplified implementation
        (void)time_per_move_ms; // Suppress unused parameter warning
        
        GameResult game;
        Position pos;
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        std::vector<std::string> moves;
        game.had_king_walking = false;
        game.had_f6_move = false;
        
        // Play game
        for (int move_num = 1; move_num <= 100; ++move_num) { // Max 100 moves
            S_MOVELIST legal_moves;
            generate_legal_moves_enhanced(pos, legal_moves);
            
            // Check for game end conditions
            if (legal_moves.count == 0) {
                if (in_check(pos)) {
                    game.result = pos.side_to_move == Color::White ? "0-1" : "1-0";
                    game.termination_reason = "checkmate";
                } else {
                    game.result = "1/2-1/2";
                    game.termination_reason = "stalemate";
                }
                break;
            }
            
            // Generate best move (simplified - just use evaluation)
            S_MOVE best_move = get_best_move_simple(pos);
            
            // Check for problematic moves
            std::string move_notation = move_to_notation(best_move);
            if (move_notation.find("f7f6") != std::string::npos || 
                move_notation.find("f2f3") != std::string::npos) {
                game.had_f6_move = true;
            }
            
            // Detect king walking (king moves in opening) - more comprehensive
            if (move_num <= 15) {
                int from = best_move.get_from();
                int to = best_move.get_to();
                
                // Check for specific king walking patterns
                if (pos.side_to_move == Color::Black) {
                    // Black king walking moves
                    int e8 = sq(File::E, Rank::R8);
                    int e7 = sq(File::E, Rank::R7);
                    int e6 = sq(File::E, Rank::R6);
                    int f7 = sq(File::F, Rank::R7);
                    int g6 = sq(File::G, Rank::R6);
                    
                    if ((from == e8 && to == e7) ||   // Ke7
                        (from == e7 && to == e6) ||   // Ke6  
                        (from == e8 && to == f7) ||   // Kf7
                        (from == f7 && to == g6)) {   // Kg6
                        game.had_king_walking = true;
                    }
                } else {
                    // White king walking moves
                    int e1 = sq(File::E, Rank::R1);
                    int e2 = sq(File::E, Rank::R2);
                    
                    if (from == e1 && to == e2) {     // Ke2
                        game.had_king_walking = true;
                    }
                }
            }
            
            moves.push_back(move_notation);
            pos.make_move_with_undo(best_move);
        }
        
        if (game.result.empty()) {
            game.result = "1/2-1/2";
            game.termination_reason = "move_limit";
        }
        
        game.move_count = moves.size();
        game.pgn = create_pgn(moves, game.result);
        
        return game;
    }
    
    S_MOVE get_best_move_simple(const Position& pos) {
        S_MOVELIST legal_moves;
        generate_legal_moves_enhanced(const_cast<Position&>(pos), legal_moves);
        
        if (legal_moves.count == 0) {
            return S_MOVE(); // No legal moves
        }
        
        S_MOVE best_move = legal_moves.moves[0];
        int best_eval = -999999;
        
        for (int i = 0; i < legal_moves.count; ++i) {
            Position temp_pos = pos;
            temp_pos.make_move_with_undo(legal_moves.moves[i]);
            
            int eval = -Engine3::HybridEvaluator::evaluate(temp_pos);
            
            if (eval > best_eval) {
                best_eval = eval;
                best_move = legal_moves.moves[i];
            }
        }
        
        return best_move;
    }
    
    std::string move_to_notation(const S_MOVE& move) {
        return square_to_notation(move.get_from()) + square_to_notation(move.get_to());
    }
    
    std::string square_to_notation(int sq) {
        int file = sq % 8;
        int rank = sq / 8;
        
        char file_char = 'a' + file;
        char rank_char = '1' + rank;
        return std::string(1, file_char) + std::string(1, rank_char);
    }
    
    std::string create_pgn(const std::vector<std::string>& moves, const std::string& result) {
        std::string pgn;
        for (size_t i = 0; i < moves.size(); i += 2) {
            pgn += std::to_string((i / 2) + 1) + ". ";
            pgn += moves[i] + " ";
            if (i + 1 < moves.size()) {
                pgn += moves[i + 1] + " ";
            }
        }
        pgn += result;
        return pgn;
    }
    
    void analyze_results() {
        std::cout << "\n=== GAME ANALYSIS ===\n";
        
        int total_games = game_results.size();
        int games_with_f6 = 0;
        int games_with_king_walking = 0;
        int checkmates = 0;
        int stalemates = 0;
        
        double avg_moves = 0;
        
        for (const auto& game : game_results) {
            if (game.had_f6_move) games_with_f6++;
            if (game.had_king_walking) games_with_king_walking++;
            if (game.termination_reason == "checkmate") checkmates++;
            if (game.termination_reason == "stalemate") stalemates++;
            avg_moves += game.move_count;
        }
        
        avg_moves /= total_games;
        
        std::cout << "Total games: " << total_games << "\n";
        std::cout << "Average game length: " << std::fixed << std::setprecision(1) << avg_moves << " moves\n";
        std::cout << "Checkmates: " << checkmates << " (" << (100.0 * checkmates / total_games) << "%)\n";
        std::cout << "Stalemates: " << stalemates << " (" << (100.0 * stalemates / total_games) << "%)\n";
        std::cout << "\n=== PROBLEMATIC MOVE DETECTION ===\n";
        std::cout << "Games with f6/f3 moves: " << games_with_f6 << " (" << (100.0 * games_with_f6 / total_games) << "%)\n";
        std::cout << "Games with king walking: " << games_with_king_walking << " (" << (100.0 * games_with_king_walking / total_games) << "%)\n";
        
        if (games_with_f6 == 0 && games_with_king_walking == 0) {
            std::cout << "✅ SUCCESS: No problematic moves detected!\n";
        } else {
            std::cout << "⚠️  WARNING: Problematic moves still occurring!\n";
        }
    }
};

int main() {
    std::cout << "Huginn Chess Engine - Self-Play Testing\n";
    std::cout << "=======================================\n\n";
    std::cout << "Testing for king walking and f6 move prevention in actual gameplay...\n\n";
    
    SelfPlayTester tester;
    tester.run_self_play_games(10, 500); // 10 games, 500ms per move
    
    return 0;
}

