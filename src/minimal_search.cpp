#include "minimal_search.hpp"
#include "evaluation.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include <iostream>
#include <algorithm>

namespace Huginn {

int MinimalEngine::evaluate(const Position& pos) {
    // VICE Part 56: Basic Evaluation with piece-square tables
    int score = 0;
    
    // Count material and add piece-square table values for both sides
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard) continue;
        
        Piece piece = pos.board[sq];
        if (piece == Piece::None) continue;
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values (same as before)
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   material_value = 100; break;
            case PieceType::Knight: material_value = 300; break;
            case PieceType::Bishop: material_value = 300; break;
            case PieceType::Rook:   material_value = 500; break;
            case PieceType::Queen:  material_value = 900; break;
            case PieceType::King:   material_value = 0; break; // King has no material value
            default: material_value = 0; break;
        }
        
        // Convert square120 to square64 for piece-square tables
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue; // Invalid square
        
        // Get piece-square table value
        int pst_value = 0;
        if (piece_color == Color::Black) {
            sq64 = mirror_square_64(sq64); // Mirror for black pieces
        }
        
        switch (piece_type) {
            case PieceType::Pawn:   pst_value = EvalParams::PAWN_TABLE[sq64]; break;
            case PieceType::Knight: pst_value = EvalParams::KNIGHT_TABLE[sq64]; break;
            case PieceType::Bishop: pst_value = EvalParams::BISHOP_TABLE[sq64]; break;
            case PieceType::Rook:   pst_value = EvalParams::ROOK_TABLE[sq64]; break;
            case PieceType::Queen:  pst_value = EvalParams::QUEEN_TABLE[sq64]; break;
            case PieceType::King:   pst_value = EvalParams::KING_TABLE[sq64]; break;
            default: pst_value = 0; break;
        }
        
        // Add material + piece-square value for this piece
        int piece_value = material_value + pst_value;
        
        if (piece_color == Color::White) {
            score += piece_value;
        } else {
            score -= piece_value;
        }
    }
    
    // Return from current side's perspective (negate if black to move)
    return (pos.side_to_move == Color::White) ? score : -score;
}

// Helper functions for evaluation (Part 56)
// Mirror square for black pieces (flip vertically)
int MinimalEngine::mirror_square_64(int sq64) {
    if (sq64 < 0 || sq64 > 63) return sq64;
    return ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
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
    
    // Use existing FILE_RANK_LOOKUPS instead of custom conversion
    File from_file = file_of(from);
    Rank from_rank = rank_of(from);
    File to_file = file_of(to);
    Rank to_rank = rank_of(to);
    
    // Check for invalid squares
    if (from_file == File::None || to_file == File::None || 
        from_rank == Rank::None || to_rank == Rank::None) {
        return "0000";
    }
    
    result += char('a' + int(from_file));
    result += char('1' + int(from_rank));
    result += char('a' + int(to_file));
    result += char('1' + int(to_rank));
    
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
    // Two main benefits (VICE tutorial):
    // 1. Time Management: Return best move found so far if time runs out (0:49)
    // 2. Move Ordering Efficiency: PV and heuristics improve alpha-beta efficiency (1:49)
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
    
    // Note: Node counting is done in AlphaBeta and quiescence functions
}

// Clear search tables and PV before new search (2:25)
// VICE Part 57 - Clear To Search: Prepare engine for clean search
void MinimalEngine::clearForSearch(MinimalEngine& engine, SearchInfo& info) {
    // Clear the history and killers arrays (0:57)
    engine.clear_search_tables();
    
    // Clear the principal variation (PV) table (2:17)
    engine.pv_table.clear();
    
    // Reset the ply counter to zero (2:21)
    info.ply = 0;
    
    // Initialize start_time, set stop to zero, and reset nodes count (2:58)
    info.start_time = std::chrono::steady_clock::now();
    info.stopped = false;    // Set stop to zero (false)
    info.quit = false;       // Reset quit flag as well
    info.nodes = 0;          // Reset nodes count
    
    // Reset engine state for new search
    engine.should_stop = false;     // Reset stop flag
    engine.nodes_searched = 0;      // Reset nodes count
}

// Core AlphaBeta search function (2:58)
int MinimalEngine::AlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull, bool isRoot) {
    // Increment node count for every position visited (except root calls)
    if (!isRoot) {
        info.nodes++;
    }
    
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
        
        int score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);  // Not a root call
        pos.TakeMove();
        
        if (info.stopped || info.quit) {
            return 0;
        }
        
        if (score > best_score) {
            best_score = score;
            if (score > alpha) {
                alpha = score;
                
                // Store best move in PV table (VICE tutorial style)
                store_pv_move(pos.zobrist_key, move_list.moves[i]);
                
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
    // Increment node count for every position visited
    info.nodes++;
    
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

// VICE-style iterative deepening search function (Part 58)
// Implements the two main benefits of iterative deepening:
// 1. Time Management: Return best move if time runs out (0:49)
// 2. Move Ordering Efficiency: Use PV and heuristics from shallower searches (1:49)
S_MOVE MinimalEngine::searchPosition(Position& pos, SearchInfo& info) {
    S_MOVE best_move;
    best_move.move = 0;
    
    // VICE Part 57: Clear everything before starting search
    clearForSearch(*this, info);
    
    // Set up search parameters
    info.start_time = std::chrono::steady_clock::now();
    
    // Iterative deepening loop (0:22) - search depth 1, then 2, then 3, etc.
    for (int current_depth = 1; current_depth <= info.max_depth; ++current_depth) {
        // Check if we should stop before starting new depth (time management)
        if (info.stopped || info.quit) {
            break;
        }
        
        info.depth = current_depth;
        
        // Store best move from previous iteration for move ordering
        S_MOVE prev_best = best_move;
        
        // Root search: try all moves at root to find the best one
        S_MOVELIST move_list;
        generate_legal_moves_enhanced(pos, move_list);
        
        if (move_list.count == 0) break; // No legal moves
        
        int best_score = -30000;
        S_MOVE depth_best_move;
        depth_best_move.move = 0;
        
        // Try each move at the root
        for (int i = 0; i < move_list.count; ++i) {
            if (info.stopped || info.quit) break;
            
            if (pos.MakeMove(move_list.moves[i]) != 1) continue; // Skip illegal moves
            
            // Search this move
            int score = -AlphaBeta(pos, -30000, 30000, current_depth - 1, info, true, true);  // isRoot = true
            pos.TakeMove();
            
            if (info.stopped || info.quit) break;
            
            if (score > best_score) {
                best_score = score;
                depth_best_move = move_list.moves[i];
            }
        }
        
        // If search was interrupted, return previous best move (time management benefit)
        if (info.stopped || info.quit) {
            break;
        }
        
        // Update best move for this iteration
        if (depth_best_move.move != 0) {
            best_move = depth_best_move;
            // Store in PV table for next iteration's move ordering
            store_pv_move(pos.zobrist_key, depth_best_move);
        }
        
        // Calculate elapsed time for output
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time);
        
        // Get Principal Variation line from this depth (5:32)
        S_MOVE pv_array[MAX_DEPTH];
        int pv_moves = get_pv_line(pos, current_depth, pv_array);
        
        // Print results after each completed depth (3:03, 5:32)
        std::cout << "info depth " << current_depth 
                  << " score cp " << best_score 
                  << " nodes " << info.nodes 
                  << " time " << elapsed.count()
                  << " pv ";
        
        // Print full Principal Variation
        for (int i = 0; i < pv_moves; ++i) {
            std::cout << move_to_uci(pv_array[i]);
            if (i < pv_moves - 1) std::cout << " ";
        }
        std::cout << std::endl;
        
        // Time management: if we're getting close to time limit, consider stopping
        if (!info.infinite && elapsed.count() > 3000) {  // If we've used 3+ seconds
            // Only continue to next depth if we have reasonable time left
            auto time_for_next_depth = elapsed.count() * 3;  // Estimate next depth takes 3x longer
            if (time_for_next_depth > 5000) {  // Would exceed 5 second limit
                break;
            }
        }
    }
    
    return best_move;
}

} // namespace Huginn
