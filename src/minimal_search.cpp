#include "minimal_search.hpp"
#include "evaluation.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include <iostream>
#include <algorithm>

namespace Huginn {

int MinimalEngine::evaluate(const Position& pos) {
    // Simple material counting only
    int material = 0;
    
    // Count material for both sides using the existing piece values
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard) continue;
        
        Piece piece = pos.board[sq];
        if (piece == Piece::None) continue;
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        int value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   value = 100; break;
            case PieceType::Knight: value = 300; break;
            case PieceType::Bishop: value = 300; break;
            case PieceType::Rook:   value = 500; break;
            case PieceType::Queen:  value = 900; break;
            case PieceType::King:   value = 0; break; // King has no material value
            default: value = 0; break;
        }
        
        if (piece_color == Color::White) {
            material += value;
        } else {
            material -= value;
        }
    }
    
    // Return from current side's perspective
    return (pos.side_to_move == Color::White) ? material : -material;
}

bool MinimalEngine::time_up() const {
    if (should_stop) return true;
    if (current_limits.infinite) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    return elapsed.count() >= current_limits.max_time_ms;
}

int MinimalEngine::alpha_beta(Position& pos, int depth, int alpha, int beta) {
    nodes_searched++;
    
    // Check time every 1000 nodes
    if (nodes_searched % 1000 == 0 && time_up()) {
        return alpha;
    }
    
    // Check for repetition (VICE tutorial style)
    if (isRepetition(pos)) {
        return 0; // Draw score
    }
    
    // Terminal node - evaluate position
    if (depth == 0) {
        return evaluate(pos);
    }
    
    // Generate all legal moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    // No legal moves (checkmate or stalemate)
    if (move_list.count == 0) {
        // Check if king is in check to determine mate vs stalemate
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttacked(king_sq, pos, !pos.side_to_move)) {
            return -29000 + (current_limits.max_depth - depth); // Checkmate, prefer quicker mates
        } else {
            return 0; // Stalemate
        }
    }
    
    int best_score = -30000;
    
    // Try each move
    for (int i = 0; i < move_list.count; ++i) {
        if (pos.MakeMove(move_list.moves[i]) != 1) continue; // Skip illegal moves
        
        int score = -alpha_beta(pos, depth - 1, -beta, -alpha);
        pos.TakeMove();
        
        if (time_up()) return alpha;
        
        if (score > best_score) {
            best_score = score;
            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    break; // Beta cutoff
                }
            }
        }
    }
    
    return best_score;
}

std::string MinimalEngine::move_to_uci(const S_MOVE& move) {
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

// Simple repetition detection - VICE tutorial style
bool MinimalEngine::isRepetition(const Position& pos) {
    // Start from when fifty move rule was last reset (tutorial optimization)
    int start_index = static_cast<int>(pos.move_history.size()) - pos.halfmove_clock;
    if (start_index < 0) start_index = 0;
    
    // Look through history for matching position key
    for (int index = start_index; index < static_cast<int>(pos.move_history.size()); ++index) {
        if (pos.zobrist_key == pos.move_history[index].zobrist_key) {
            return true; // Repetition found
        }
    }
    
    return false; // No repetition
}

S_MOVE MinimalEngine::search(Position pos, const MinimalLimits& limits) {
    current_limits = limits;
    start_time = std::chrono::steady_clock::now();
    nodes_searched = 0;
    should_stop = false;
    
    S_MOVE best_move;
    best_move.move = 0;
    
    // Iterative deepening
    for (int depth = 1; depth <= limits.max_depth; ++depth) {
        if (time_up()) break;
        
        S_MOVELIST move_list;
        generate_legal_moves_enhanced(pos, move_list);
        
        if (move_list.count == 0) break;
        
        int best_score = -30000;
        S_MOVE depth_best_move;
        depth_best_move.move = 0;
        
        for (int i = 0; i < move_list.count; ++i) {
            if (pos.MakeMove(move_list.moves[i]) != 1) continue;
            
            int score = -alpha_beta(pos, depth - 1, -30000, 30000);
            pos.TakeMove();
            
            if (time_up()) break;
            
            if (score > best_score) {
                best_score = score;
                depth_best_move = move_list.moves[i];
            }
        }
        
        if (depth_best_move.move != 0) {
            best_move = depth_best_move;
            
            // UCI output
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time).count();
            
            std::cout << "info depth " << depth 
                     << " score cp " << best_score 
                     << " nodes " << nodes_searched 
                     << " time " << elapsed
                     << " pv " << move_to_uci(best_move) << std::endl;
        }
        
        if (time_up()) break;
    }
    
    return best_move;
}

} // namespace Huginn
