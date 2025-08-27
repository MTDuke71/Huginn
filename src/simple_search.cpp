#include "simple_search.hpp"
#include "attack_detection.hpp"
#include <algorithm>
#include <iostream>

namespace Engine3 {

// Convert move to UCI notation
std::string SimpleEngine::move_to_uci(const S_MOVE& move) {
    if (move.move == 0) return "0000";
    
    std::string result;
    
    int from = move.get_from();
    int to = move.get_to();
    
    // Convert square120 to file/rank
    auto square120_to_file_rank = [](int sq120) -> std::pair<int, int> {
        if (sq120 < 21 || sq120 > 98) return {-1, -1}; // Invalid
        
        int file = (sq120 % 10) - 1;
        int rank = (sq120 / 10) - 2;
        
        if (file < 0 || file > 7 || rank < 0 || rank > 7) return {-1, -1};
        
        return {file, rank};
    };
    
    auto [from_file, from_rank] = square120_to_file_rank(from);
    auto [to_file, to_rank] = square120_to_file_rank(to);
    
    if (from_file < 0 || to_file < 0) return "0000";
    
    result += char('a' + from_file);
    result += char('1' + from_rank);
    result += char('a' + to_file);
    result += char('1' + to_rank);
    
    // Add promotion piece if applicable
    if (move.is_promotion()) {
        PieceType promo = move.get_promoted();
        switch (promo) {
            case PieceType::Queen:  result += 'q'; break;
            case PieceType::Rook:   result += 'r'; break;
            case PieceType::Bishop: result += 'b'; break;
            case PieceType::Knight: result += 'n'; break;
            default: break;
        }
    }
    
    return result;
}

// Convert PV to string
std::string SimpleEngine::pv_to_string(const PVLine& pv) {
    std::string result;
    for (int i = 0; i < pv.length; ++i) {
        if (i > 0) result += " ";
        result += move_to_uci(pv.moves[i]);
    }
    return result;
}

// Calculate mate distance
int SimpleEngine::mate_distance(int score) {
    if (score > MATE_IN_MAX_PLY) {
        return (MATE_SCORE - score + 1) / 2;
    } else if (score < -MATE_IN_MAX_PLY) {
        return -(MATE_SCORE + score) / 2;
    }
    return 0;
}

// Convert score to UCI format
std::string SimpleEngine::score_to_uci(int score) {
    if (is_mate_score(score)) {
        int mate_dist = mate_distance(score);
        return "mate " + std::to_string(mate_dist);
    } else {
        return "cp " + std::to_string(score);
    }
}

// Check if time is up
bool SimpleEngine::time_up() const {
    if (current_limits.infinite) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    
    return elapsed.count() >= static_cast<long long>(current_limits.max_time_ms) || 
           stats.nodes_searched >= current_limits.max_nodes ||
           should_stop;
}

// Update search statistics
void SimpleEngine::update_stats() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    stats.time_ms = elapsed.count();
}

// Score a move for ordering (simple version)
int SimpleEngine::score_move(const Position& pos, const S_MOVE& move) {
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
    
    // Check if move gives check (simple version)
    Position temp_pos = pos;
    temp_pos.make_move_with_undo(move);
    int opp_king_sq = temp_pos.king_sq[int(temp_pos.side_to_move)];
    if (opp_king_sq >= 0 && SqAttacked(opp_king_sq, temp_pos, !temp_pos.side_to_move)) {
        score += 50;
    }
    
    return score;
}

// Order moves for better alpha-beta pruning
void SimpleEngine::order_moves(const Position& pos, S_MOVELIST& moves) {
    // Score all moves
    std::vector<std::pair<int, int>> move_scores;
    for (int i = 0; i < moves.count; ++i) {
        int score = score_move(pos, moves.moves[i]);
        move_scores.push_back({score, i});
    }
    
    // Sort by score (highest first)
    std::sort(move_scores.begin(), move_scores.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Reorder moves array
    S_MOVELIST temp_moves = moves;
    for (int i = 0; i < moves.count; ++i) {
        moves.moves[i] = temp_moves.moves[move_scores[i].second];
    }
}

// Quiescence search to avoid horizon effect
int SimpleEngine::quiescence_search(Position& pos, int alpha, int beta) {
    stats.nodes_searched++;
    
    if (time_up()) return 0;
    
    // Stand pat evaluation
    int stand_pat = HybridEvaluator::evaluate(pos);
    
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;
    
    // Generate capture moves only
    S_MOVELIST captures;
    generate_legal_moves_enhanced(pos, captures);
    
    // Filter for captures only
    S_MOVELIST capture_moves;
    capture_moves.count = 0;
    for (int i = 0; i < captures.count; ++i) {
        if (captures.moves[i].get_captured() != PieceType::None) {
            capture_moves.moves[capture_moves.count++] = captures.moves[i];
        }
    }
    
    order_moves(pos, capture_moves);
    
    for (int i = 0; i < capture_moves.count; ++i) {
        if (time_up()) break;
        
        pos.make_move_with_undo(capture_moves.moves[i]);
        int score = -quiescence_search(pos, -beta, -alpha);
        pos.undo_move();
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

// Main alpha-beta search
int SimpleEngine::alpha_beta(Position& pos, int depth, int alpha, int beta, PVLine& pv) {
    pv.clear();
    stats.nodes_searched++;
    
    if (time_up()) return 0;
    
    // Check for mate/stalemate
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    if (legal_moves.count == 0) {
        // Check if king is in check
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttacked(king_sq, pos, !pos.side_to_move)) {
            return -MATE_SCORE + (stats.max_depth_reached - depth); // Mate, prefer quicker mates
        } else {
            return 0; // Stalemate
        }
    }
    
    // Quiescence search at leaf nodes
    if (depth <= 0) {
        return quiescence_search(pos, alpha, beta);
    }
    
    order_moves(pos, legal_moves);
    
    PVLine best_pv;
    // bool pv_found = false; // TODO: Use for PV handling logic
    
    for (int i = 0; i < legal_moves.count; ++i) {
        if (time_up()) break;
        
        pos.make_move_with_undo(legal_moves.moves[i]);
        
        PVLine child_pv;
        int score = -alpha_beta(pos, depth - 1, -beta, -alpha, child_pv);
        
        pos.undo_move();
        
        if (score >= beta) {
            return beta; // Beta cutoff
        }
        
        if (score > alpha) {
            alpha = score;
            
            // Update PV
            pv.clear();
            pv.add_move(legal_moves.moves[i]);
            for (int j = 0; j < child_pv.length; ++j) {
                pv.add_move(child_pv.moves[j]);
            }
            // pv_found = true; // TODO: Use for PV handling logic
        }
    }
    
    return alpha;
}

// Main search function
S_MOVE SimpleEngine::search(Position pos, const SearchLimits& limits) {
    reset();
    current_limits = limits;
    start_time = std::chrono::steady_clock::now();
    
    S_MOVE best_move;
    best_move.move = 0;
    best_move.score = 0;
    // int best_score = -MATE_SCORE; // TODO: Use for move scoring logic
    
    // Iterative deepening
    for (int depth = 1; depth <= limits.max_depth; ++depth) {
        if (time_up()) break;
        
        stats.max_depth_reached = depth;
        
        PVLine current_pv;
        int score = alpha_beta(pos, depth, -MATE_SCORE, MATE_SCORE, current_pv);
        
        if (time_up()) break;
        
        // best_score = score; // TODO: Use for move scoring logic
        main_pv = current_pv;
        
        if (main_pv.length > 0) {
            best_move = main_pv.moves[0];
        }
        
        update_stats();
        
        // Print search info (UCI format)
        std::cout << "info depth " << depth 
                  << " score " << score_to_uci(score)
                  << " nodes " << stats.nodes_searched
                  << " time " << stats.time_ms;
        
        if (stats.time_ms > 0) {
            std::cout << " nps " << (stats.nodes_searched * 1000) / stats.time_ms;
        }
        
        std::cout << " pv " << pv_to_string(main_pv) << std::endl;
        
        // Check for mate
        if (abs(score) > 30000) {
            break; // Found mate, no need to search deeper
        }
    }
    
    update_stats();
    return best_move;
}

} // namespace Engine3
