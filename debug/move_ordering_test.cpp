#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "init.hpp"

// Replicate the exact score_move function from search.cpp
int score_move_test(const Position& pos, const S_MOVE& move) {
    int score = 0;
    
    // Basic capture scoring
    PieceType captured = move.get_captured();
    if (captured != PieceType::None) {
        score += 1000 + static_cast<int>(captured) * 10;
        
        // Get piece type being moved
        int from_sq = move.get_from();
        if (from_sq >= 0 && from_sq < 120) {
            PieceType moving_piece = type_of(pos.board[from_sq]);
            score -= static_cast<int>(moving_piece); // MVV-LVA
        }
    }
    
    // Promotion
    if (move.get_promoted() != PieceType::None) {
        score += 900;
    }
    
    // Check if move gives check (simple version) - THIS IS THE PROBLEMATIC PART
    std::cout << "    Creating temp position copy..." << std::endl;
    Position temp_pos = pos;  // <-- POTENTIAL ISSUE HERE
    
    std::cout << "    Original pos ply: " << pos.ply << ", move_history size: " << pos.move_history.size() << std::endl;
    std::cout << "    Temp pos ply: " << temp_pos.ply << ", move_history size: " << temp_pos.move_history.size() << std::endl;
    
    if (temp_pos.MakeMove(move) == 1) {
        std::cout << "    Move made on temp pos, ply: " << temp_pos.ply << ", move_history size: " << temp_pos.move_history.size() << std::endl;
        
        int opp_king_sq = temp_pos.king_sq[int(temp_pos.side_to_move)];
        if (opp_king_sq >= 0 && SqAttacked(opp_king_sq, temp_pos, !temp_pos.side_to_move)) {
            score += 50;
        }
        temp_pos.TakeMove();
        std::cout << "    Move taken back, ply: " << temp_pos.ply << ", move_history size: " << temp_pos.move_history.size() << std::endl;
    }
    
    return score;
}

// Replicate the exact order_moves function from search.cpp
void order_moves_test(const Position& pos, S_MOVELIST& moves) {
    std::cout << "  === Starting move ordering ===" << std::endl;
    std::cout << "  Position state - ply: " << pos.ply << ", move_history size: " << pos.move_history.size() << std::endl;
    
    // Score all moves
    std::vector<std::pair<int, int>> move_scores;
    for (int i = 0; i < moves.count; ++i) {
        std::cout << "  Scoring move " << i+1 << "/" << moves.count << " (move=" << std::hex << moves.moves[i].move << std::dec << ")" << std::endl;
        
        int score = score_move_test(pos, moves.moves[i]);
        move_scores.push_back({score, i});
        
        std::cout << "  Move " << i+1 << " scored: " << score << std::endl;
    }
    
    // Sort by score (highest first)
    std::sort(move_scores.begin(), move_scores.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Reorder moves array
    S_MOVELIST temp_moves = moves;
    for (int i = 0; i < moves.count; ++i) {
        moves.moves[i] = temp_moves.moves[move_scores[i].second];
    }
    
    std::cout << "  === Move ordering complete ===" << std::endl;
}

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Move Ordering Crash Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    std::cout << "Initial state - ply: " << pos.ply << ", move_history size: " << pos.move_history.size() << std::endl;
    
    // Generate legal moves like the search engine does
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "Generated " << legal_moves.count << " legal moves" << std::endl;
    
    try {
        std::cout << "Testing move ordering (this is where the crash should happen)..." << std::endl;
        order_moves_test(pos, legal_moves);
        
        std::cout << "✓ SUCCESS: Move ordering completed without crash!" << std::endl;
        std::cout << "Final position state - ply: " << pos.ply << ", move_history size: " << pos.move_history.size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ ERROR: Exception caught during move ordering: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "✗ ERROR: Unknown exception caught during move ordering" << std::endl; 
        return 1;
    }
    
    return 0;
}
