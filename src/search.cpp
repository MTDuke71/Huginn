#include "search.hpp"
#include "attack_detection.hpp"
#include <algorithm>
#include <iostream>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

namespace Huginn {

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

// Score a move for ordering (enhanced version for better performance)
int SimpleEngine::score_move(const Position& pos, const S_MOVE& move) {
    int score = 0;
    
    // 1. Hash move (highest priority - from transposition table)
    // TODO: Add when transposition table is implemented
    
    // 2. Winning captures (MVV-LVA)
    PieceType captured = move.get_captured();
    if (captured != PieceType::None) {
        int from_sq = move.get_from();
        if (from_sq >= 0 && from_sq < 120) {
            PieceType moving_piece = type_of(pos.board[from_sq]);
            
            // Most Valuable Victim - Least Valuable Attacker
            int victim_value = static_cast<int>(captured) * 100;
            int attacker_value = static_cast<int>(moving_piece);
            score += 10000 + victim_value - attacker_value;
        }
    }
    
    // 3. Promotions (very high value)
    if (move.get_promoted() != PieceType::None) {
        score += 9000 + static_cast<int>(move.get_promoted()) * 100;
    }
    
    // 4. Killer moves (moves that caused beta cutoffs at same depth)
    // TODO: Add killer move heuristic
    
    // 5. History heuristic (moves that worked well before)
    // TODO: Add history tables
    
    // 6. Castling (generally good)
    if (move.is_castle()) {
        score += 500;
    }
    
    // 7. Center control moves
    int to_sq = move.get_to();
    int file = to_sq % 10;
    int rank = to_sq / 10;
    if (file >= 3 && file <= 6 && rank >= 3 && rank <= 6) {
        score += 20; // Center squares
    }
    
    return score;
}

// Order moves for better alpha-beta pruning (optimized version)
void SimpleEngine::order_moves(const Position& pos, S_MOVELIST& moves) {
    // Score moves directly in the movelist structure to avoid allocations
    for (int i = 0; i < moves.count; ++i) {
        moves.moves[i].score = score_move(pos, moves.moves[i]);
    }
    
    // Use simple insertion sort for small lists (faster than std::sort for small N)
    if (moves.count <= 16) {
        for (int i = 1; i < moves.count; ++i) {
            S_MOVE key = moves.moves[i];
            int j = i - 1;
            while (j >= 0 && moves.moves[j].score < key.score) {
                moves.moves[j + 1] = moves.moves[j];
                --j;
            }
            moves.moves[j + 1] = key;
        }
    } else {
        // For larger lists, use partial sort to only sort the best moves
        std::partial_sort(moves.moves, moves.moves + std::min(8, moves.count), moves.moves + moves.count,
                         [](const S_MOVE& a, const S_MOVE& b) { return a.score > b.score; });
    }
}

// Quiescence search to avoid horizon effect
int SimpleEngine::quiescence_search(Position& pos, int alpha, int beta) {
    increment_nodes();
    
    if (time_up()) return 0;
    
    // Stand pat evaluation
    int stand_pat = HybridEvaluator::evaluate(pos);
    
    // Beta cutoff from stand pat
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;
    
    // Generate capture moves only
    S_MOVELIST captures;
    generate_legal_moves_enhanced(pos, captures);
    
    // Filter for captures only - preallocate for better performance
    S_MOVELIST capture_moves;
    capture_moves.count = 0;
    
#ifdef _MSC_VER
    // Hint that we expect some captures but not all moves
    __assume(captures.count >= 0 && captures.count <= 256);
#endif
    
    for (int i = 0; i < captures.count; ++i) {
        if (captures.moves[i].get_captured() != PieceType::None) {
            capture_moves.moves[capture_moves.count++] = captures.moves[i];
        }
    }
    
    // BUGFIX: Disable move ordering in quiescence to avoid excessive Position copying
    // order_moves(pos, capture_moves);  // <-- DISABLED
    
    for (int i = 0; i < capture_moves.count; ++i) {
        if (time_up()) break;
        
        if (pos.MakeMove(capture_moves.moves[i]) == 1) {
            int score = -quiescence_search(pos, -beta, -alpha);
            pos.TakeMove();
            
            // Beta cutoff - most common case first
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
    }
    
    return alpha;
}

// Main alpha-beta search
int SimpleEngine::alpha_beta(Position& pos, int depth, int alpha, int beta, PVLine& pv) {
    pv.clear();
    increment_nodes();

    if (time_up()) return 0;

    // --- Repetition detection ---
    // Only track up to MAX_PLY positions
    if (repetition_stack.size() > static_cast<size_t>(depth)) repetition_stack.resize(depth);
    repetition_stack.push_back(pos.zobrist_key);
    int repetition_count = 0;
    for (size_t i = 0; i + 1 < repetition_stack.size(); ++i) {
        if (repetition_stack[i] == pos.zobrist_key) repetition_count++;
    }
    if (repetition_count >= 2) { // Threefold repetition
        repetition_stack.pop_back();
        return 0; // Draw score
    }

    // Probe transposition table
    int tt_score;
    uint8_t tt_depth, tt_node_type;
    uint32_t tt_best_move;
    uint64_t zobrist_key = pos.zobrist_key;

    if (tt.probe(zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move)) {
        if (tt_depth >= depth) {
            // Use cached result if search depth is sufficient
            if (tt_node_type == TTEntry::EXACT) {
                repetition_stack.pop_back();
                return tt_score;
            } else if (tt_node_type == TTEntry::LOWER_BOUND && tt_score >= beta) {
                repetition_stack.pop_back();
                return beta; // Beta cutoff
            } else if (tt_node_type == TTEntry::UPPER_BOUND && tt_score <= alpha) {
                repetition_stack.pop_back();
                return alpha; // Alpha cutoff
            }
        }
    }
    
    // Check for mate/stalemate
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    // Early exit for checkmate/stalemate - this is uncommon
    if (legal_moves.count == 0) {
#ifdef _MSC_VER
        __assume(0); // Tell compiler this path is unlikely
#endif
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
    S_MOVE best_move = S_MOVE(); // Track best move for TT
    int original_alpha = alpha;
    
    for (int i = 0; i < legal_moves.count; ++i) {
        if (time_up()) break;
        
        if (pos.MakeMove(legal_moves.moves[i]) == 1) {
            PVLine child_pv;
            int score = -alpha_beta(pos, depth - 1, -beta, -alpha, child_pv);
            
            pos.TakeMove();
            
            // Beta cutoff - most common case first
            if (score >= beta) {
                // Store as lower bound (beta cutoff)
                tt.store(zobrist_key, beta, depth, TTEntry::LOWER_BOUND, legal_moves.moves[i].move);
                return beta;
            }
            
            if (score > alpha) {
                alpha = score;
                best_move = legal_moves.moves[i];
                
                // Update PV
                pv.clear();
                pv.add_move(legal_moves.moves[i]);
                for (int j = 0; j < child_pv.length; ++j) {
                    pv.add_move(child_pv.moves[j]);
                }
            }
        }
    }
    
    // Store result in transposition table
    if (alpha > original_alpha) {
        // Exact score (PV node)
        tt.store(zobrist_key, alpha, depth, TTEntry::EXACT, best_move.move);
    } else {
        // Upper bound (all moves failed low)
        tt.store(zobrist_key, alpha, depth, TTEntry::UPPER_BOUND);
    }

    repetition_stack.pop_back();
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
    int max_search_depth = (limits.max_depth > 0) ? limits.max_depth : 12; // Use reasonable depth for time-limited searches
    for (int depth = 1; depth <= max_search_depth; ++depth) {
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
        std::cout.flush(); // Ensure immediate output
        
        // Check for mate
        if (abs(score) > 30000) {
            break; // Found mate, no need to search deeper
        }
    }
    
    update_stats();
    return best_move;
}

// ThreadedEngine implementation

// Thread-safe time checking
bool ThreadedEngine::thread_time_up() const {
    if (current_limits.infinite) return false;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

    return elapsed.count() >= static_cast<long long>(current_limits.max_time_ms) ||
           thread_safe_stats.nodes_searched >= current_limits.max_nodes ||
           global_stop;
}

// Thread worker function
S_MOVE ThreadedEngine::thread_search_worker(Position pos, const SearchLimits& limits, int thread_id) {
    S_MOVE best_move;
    best_move.move = 0;
    best_move.score = -MATE_SCORE;

    // Generate root moves once
    S_MOVELIST root_moves;
    generate_legal_moves_enhanced(pos, root_moves);

    if (root_moves.count == 0) return best_move;

    // Each thread works on different moves using round-robin distribution
    PVLine best_pv;

    // Use a hybrid approach for better PV construction:
    // Thread 0 does full iterative deepening search (like single-threaded)
    // Other threads help by searching alternative moves in parallel
    if (thread_id == 0) {
        // Thread 0: Do full iterative deepening like single-threaded search
        int max_search_depth = (limits.max_depth > 0) ? limits.max_depth : 64; // Use 64 as practical maximum for unlimited depth
        for (int depth = 1; depth <= max_search_depth; ++depth) {
            if (thread_time_up()) break;

            PVLine current_pv;
            int score = alpha_beta(pos, depth, -MATE_SCORE, MATE_SCORE, current_pv);

            if (thread_time_up()) break;

            if (current_pv.length > 0) {
                best_move = current_pv.moves[0];
                best_move.score = score;
                best_pv = current_pv;

                // Update global best move
                {
                    std::lock_guard<std::mutex> lock(best_move_mutex);
                    global_best_move = best_move;
                    global_best_pv = best_pv;
                    global_best_score = score;
                }

                // Output UCI info
                std::lock_guard<std::mutex> output_lock(output_mutex);

                thread_safe_stats.max_depth_reached = depth;

                uint64_t total_nodes = thread_safe_stats.nodes_searched.load();
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
                uint64_t time_ms = elapsed.count();

                std::cout << "info depth " << depth
                          << " score " << score_to_uci(score)
                          << " nodes " << total_nodes
                          << " time " << time_ms;

                if (time_ms > 0) {
                    std::cout << " nps " << (total_nodes * 1000) / time_ms;
                }

                std::cout << " pv " << pv_to_string(best_pv) << std::endl;
            }

            // Check for mate
            if (abs(score) > 30000) {
                break;
            }
        }
    } else {
        // Other threads: Help by searching alternative moves at various depths
        // This provides parallel search without interfering with PV construction
        int max_search_depth = (limits.max_depth > 0) ? limits.max_depth : 64; // Use 64 as practical maximum for unlimited depth
        for (int depth = 1; depth <= max_search_depth; ++depth) {
            if (thread_time_up()) break;

            // Each helper thread searches a different subset of moves
            for (int move_idx = thread_id - 1; move_idx < root_moves.count; move_idx += (limits.threads - 1)) {
                if (thread_time_up()) break;

                S_MOVE move = root_moves.moves[move_idx];

                // Make the move
                Position thread_pos = pos;
                if (!thread_pos.MakeMove(move)) continue; // Skip illegal moves

                PVLine child_pv;

                // Search from this position (depth-1 to avoid interfering with main search)
                int score = -alpha_beta(thread_pos, depth - 1, -MATE_SCORE, MATE_SCORE, child_pv);

                // Only update global if significantly better (to avoid constant lock contention)
                if (score > global_best_score.load() + 100) {
                    std::lock_guard<std::mutex> lock(best_move_mutex);
                    if (score > global_best_score.load()) {
                        PVLine full_pv;
                        full_pv.add_move(move);
                        for (int i = 0; i < child_pv.length; ++i) {
                            full_pv.add_move(child_pv.moves[i]);
                        }

                        global_best_move = move;
                        global_best_move.score = score;
                        global_best_pv = full_pv;
                        global_best_score = score;
                    }
                }
            }
        }
    }

    return best_move;
}

// Main threaded search
S_MOVE ThreadedEngine::search(Position pos, const SearchLimits& limits) {
    reset();
    current_limits = limits;
    start_time = std::chrono::steady_clock::now();

    // Initialize global best move
    global_best_move.move = 0;
    global_best_move.score = -MATE_SCORE;
    global_best_pv.clear();
    global_best_score = -MATE_SCORE;

    // Use only 1 thread if threads is 1 or less
    if (limits.threads <= 1) {
        return single_threaded_search(pos, limits);
    }

    // Launch multiple threads
    std::vector<std::future<S_MOVE>> futures;

    for (int i = 0; i < limits.threads; ++i) {
        futures.push_back(
            std::async(std::launch::async,
                      &ThreadedEngine::thread_search_worker,
                      this, pos, limits, i)
        );
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.get();  // Just wait for completion, best move is in global_best_move
    }

    // Return the global best move
    std::lock_guard<std::mutex> lock(best_move_mutex);
    S_MOVE final_best = global_best_move;

    // Update final stats
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    thread_safe_stats.time_ms = elapsed.count();

    return final_best;
}

// Single-threaded search for ThreadedEngine
S_MOVE ThreadedEngine::single_threaded_search(Position pos, const SearchLimits& limits) {
    thread_safe_stats.reset();
    current_limits = limits;
    start_time = std::chrono::steady_clock::now();

    S_MOVE best_move;
    best_move.move = 0;
    best_move.score = 0;

    // Iterative deepening (similar to SimpleEngine but with thread-safe stats)
    int max_search_depth = (limits.max_depth > 0) ? limits.max_depth : 64; // Use 64 as practical maximum for unlimited depth
    for (int depth = 1; depth <= max_search_depth; ++depth) {
        if (thread_time_up()) break;

        thread_safe_stats.max_depth_reached = depth;

        PVLine current_pv;
        int score = alpha_beta(pos, depth, -MATE_SCORE, MATE_SCORE, current_pv);

        if (thread_time_up()) break;

        main_pv = current_pv;

        if (main_pv.length > 0) {
            best_move = main_pv.moves[0];
        }

        // Update final stats
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        thread_safe_stats.time_ms = elapsed.count();

        // Print search info (UCI format)
        std::cout << "info depth " << depth
                  << " score " << score_to_uci(score)
                  << " nodes " << thread_safe_stats.nodes_searched.load()
                  << " time " << thread_safe_stats.time_ms.load();

        if (thread_safe_stats.time_ms.load() > 0) {
            std::cout << " nps " << (thread_safe_stats.nodes_searched.load() * 1000) / thread_safe_stats.time_ms.load();
        }

        std::cout << " pv " << pv_to_string(main_pv) << std::endl;

        // Check for mate
        if (abs(score) > 30000) {
            break; // Found mate, no need to search deeper
        }
    }

    // Update final stats
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    thread_safe_stats.time_ms = elapsed.count();

    return best_move;
}

} // namespace Huginn
