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
    
    // Check time more frequently - every 512 nodes instead of 1000
    if (nodes_searched % 512 == 0 && time_up()) {
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

// Simple repetition detection - VICE tutorial style (made static as per Part 55)
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

// Clear search tables - reset history and killers
void MinimalEngine::clear_search_tables() {
    // Clear search history array (3:55)
    for (int piece = 0; piece < 13; ++piece) {
        for (int sq = 0; sq < 120; ++sq) {
            search_history[piece][sq] = 0;
        }
    }
    
    // Clear search killers array (4:37)
    for (int depth = 0; depth < MAX_DEPTH; ++depth) {
        search_killers[depth][0] = S_MOVE();  // Clear first killer
        search_killers[depth][1] = S_MOVE();  // Clear second killer
    }
}

// PV table helper functions
void MinimalEngine::store_pv_move(uint64_t position_key, const S_MOVE& move) {
    pv_table.store_move(position_key, move);
}

bool MinimalEngine::probe_pv_move(uint64_t position_key, S_MOVE& move) const {
    return pv_table.probe_move(position_key, move);
}

// Get PV line for display (Part 53)
int MinimalEngine::get_pv_line(Position& pos, int depth, S_MOVE pv_array[MAX_DEPTH]) {
    return pv_table.get_pv_line(pos, depth, pv_array);
}

// Update search history when move improves alpha (3:55)
void MinimalEngine::update_search_history(const Position& pos, const S_MOVE& move, int depth) {
    if (move.move == 0) return;
    
    // Get piece and destination square
    int from = move.get_from();
    int to = move.get_to();
    
    if (from < 0 || from >= 120 || to < 0 || to >= 120) return;
    
    Piece piece = pos.board[from];
    int piece_index = static_cast<int>(piece) % 13;  // Ensure valid index
    
    // Increase history score for this piece-to-square combination
    search_history[piece_index][to] += depth * depth;  // Deeper moves get higher bonus
}

// Update killer moves when move causes beta cutoff (4:37)  
void MinimalEngine::update_killer_moves(const S_MOVE& move, int depth) {
    if (move.move == 0 || depth < 0 || depth >= MAX_DEPTH) return;
    
    // Only store non-capture moves as killers
    if (!move.is_capture()) {
        // If this move isn't already first killer, shift and add
        if (search_killers[depth][0].move != move.move) {
            search_killers[depth][1] = search_killers[depth][0];  // Second = old first
            search_killers[depth][0] = move;                      // First = new move
        }
    }
}

S_MOVE MinimalEngine::search(Position pos, const MinimalLimits& limits) {
    current_limits = limits;
    start_time = std::chrono::steady_clock::now();
    nodes_searched = 0;
    should_stop = false;
    
    // Clear search tables for new search (VICE tutorial approach)
    clear_search_tables();
    
    S_MOVE best_move;
    best_move.move = 0;
    
    // Iterative deepening with time budget estimation
    for (int depth = 1; depth <= limits.max_depth; ++depth) {
        // Check time before starting new depth
        if (time_up()) break;
        
        // Time budget estimation: if we've used more than half our time,
        // and this isn't the first depth, be cautious about starting deeper search
        if (depth > 1 && !limits.infinite) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
            auto remaining = limits.max_time_ms - elapsed.count();
            
            // If we have less than 25% of original time left, don't start new depth
            if (remaining < limits.max_time_ms * 0.25) {
                break;
            }
            
            // If we have very little time left (less than 100ms), definitely stop
            if (remaining < 100) {
                break;
            }
        }
        
        S_MOVELIST move_list;
        generate_legal_moves_enhanced(pos, move_list);
        
        if (move_list.count == 0) break;
        
        int best_score = -30000;
        S_MOVE depth_best_move;
        depth_best_move.move = 0;
        
        for (int i = 0; i < move_list.count; ++i) {
            // Check time more frequently during move loop
            if (time_up()) break;
            
            if (pos.MakeMove(move_list.moves[i]) != 1) continue;
            
            int score = -alpha_beta(pos, depth - 1, -30000, 30000);
            pos.TakeMove();
            
            // Check time immediately after each move search
            if (time_up()) break;
            
            if (score > best_score) {
                best_score = score;
                depth_best_move = move_list.moves[i];
            }
        }
        
        if (depth_best_move.move != 0) {
            best_move = depth_best_move;
            
            // Store best move in PV table (VICE tutorial approach)
            store_pv_move(pos.zobrist_key, depth_best_move);
            
            // UCI output with full PV line (Part 53)
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time).count();
            
            // Get PV line from table
            S_MOVE pv_array[MAX_DEPTH];
            int pv_moves = get_pv_line(pos, depth, pv_array);
            
            std::cout << "info depth " << depth 
                     << " score cp " << best_score 
                     << " nodes " << nodes_searched 
                     << " time " << elapsed
                     << " pv ";
            
            // Print full PV line
            for (int i = 0; i < pv_moves; ++i) {
                std::cout << move_to_uci(pv_array[i]);
                if (i < pv_moves - 1) std::cout << " ";
            }
            std::cout << std::endl;
        }
        
        if (time_up()) break;
    }
    
    return best_move;
}

// VICE Part 55 - Search Function Definitions
// This implements the core search infrastructure following the VICE tutorial:
// - evalPosition: Position evaluation function 
// - checkup: Time management and GUI interrupt checking
// - clearForSearch: Initialize search tables before new search
// - AlphaBeta: Core recursive search with alpha-beta pruning
// - quiescence: Search only captures to handle horizon effect

// Position evaluation (0:34) - Returns score from current side's perspective
int MinimalEngine::evalPosition(const Position& pos) {
    // For now, use the existing evaluate function
    return evaluate(pos);
}

// Check time limits and GUI interrupts (1:34)
void MinimalEngine::checkup(SearchInfo& info) {
    // Check if we should stop due to time limit
    if (info.quit || info.stopped) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time);
    
    // Check time limit (if not infinite search)
    if (!info.infinite) {
        // Simple time management - stop if we've used our allocated time
        int time_limit_ms = 5000; // Default 5 seconds for now
        if (elapsed.count() >= time_limit_ms) {
            info.stopped = true;
        }
    }
    
    // Increment node count
    info.nodes++;
}

// Clear search tables and PV before new search (2:25)
void MinimalEngine::clearForSearch(MinimalEngine& engine) {
    // Clear search tables
    engine.clear_search_tables();
    
    // Clear PV table
    engine.pv_table.clear();
}

// Core AlphaBeta search function (2:58)
int MinimalEngine::AlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull) {
    // Check for early exit conditions
    if (depth == 0) {
        return quiescence(pos, alpha, beta, info);  // Enter quiescence search at leaf nodes
    }
    
    // Periodically check time and node limits
    if ((info.nodes & 2047) == 0) {  // Check every 2048 nodes
        checkup(info);
    }
    
    if (info.stopped || info.quit) {
        return 0;
    }
    
    // Check for repetition
    if (isRepetition(pos)) {
        return 0; // Draw score
    }
    
    // Generate all legal moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    // No legal moves (checkmate or stalemate)
    if (move_list.count == 0) {
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttacked(king_sq, pos, !pos.side_to_move)) {
            return -29000 + (info.max_depth - depth); // Checkmate, prefer quicker mates
        } else {
            return 0; // Stalemate
        }
    }
    
    int best_score = -30000;
    
    // Try each move
    for (int i = 0; i < move_list.count; ++i) {
        if (pos.MakeMove(move_list.moves[i]) != 1) continue; // Skip illegal moves
        
        int score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true);
        pos.TakeMove();
        
        if (info.stopped || info.quit) {
            return 0;
        }
        
        if (score > best_score) {
            best_score = score;
            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    // Beta cutoff - update killer moves and history
                    update_killer_moves(move_list.moves[i], depth);
                    break;
                }
            }
        }
    }
    
    return best_score;
}

// Quiescence search to handle horizon effect (4:40)
int MinimalEngine::quiescence(Position& pos, int alpha, int beta, SearchInfo& info) {
    // Periodically check time
    if ((info.nodes & 2047) == 0) {
        checkup(info);
    }
    
    if (info.stopped || info.quit) {
        return 0;
    }
    
    // Stand pat - evaluate current position
    int stand_pat = evalPosition(pos);
    
    // Beta cutoff on stand pat
    if (stand_pat >= beta) {
        return beta;
    }
    
    // Alpha improvement
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    
    // Generate only capture moves for quiescence
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);  // For now, generate all moves
    
    // Filter to only captures (simplified for now)
    for (int i = 0; i < move_list.count; ++i) {
        S_MOVE move = move_list.moves[i];
        
        // Only search captures in quiescence
        if (!move.is_capture()) continue;
        
        if (pos.MakeMove(move) != 1) continue; // Skip illegal moves
        
        int score = -quiescence(pos, -beta, -alpha, info);
        pos.TakeMove();
        
        if (info.stopped || info.quit) {
            return 0;
        }
        
        if (score >= beta) {
            return beta; // Beta cutoff
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

} // namespace Huginn
