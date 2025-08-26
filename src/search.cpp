#include "search.hpp"
#include "movegen_enhanced.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include "zobrist.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace Search {

    // =================================================================
    // Utility Functions
    // =================================================================

    std::string move_to_uci(const S_MOVE& move) {
        std::string result;
        
        int from = move.get_from();
        int to = move.get_to();
        
        // Convert to algebraic notation
        File from_file = file_of(from);
        Rank from_rank = rank_of(from);
        File to_file = file_of(to);
        Rank to_rank = rank_of(to);
        
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

    S_MOVE uci_to_move(const std::string& uci_move, const Position& pos) {
        if (uci_move.length() < 4) return S_MOVE();
        
        // Parse from square
        File from_file = File(uci_move[0] - 'a');
        Rank from_rank = Rank(uci_move[1] - '1');
        int from = sq(from_file, from_rank);
        
        // Parse to square
        File to_file = File(uci_move[2] - 'a');
        Rank to_rank = Rank(uci_move[3] - '1');
        int to = sq(to_file, to_rank);
        
        // Check for promotion
        PieceType promotion = PieceType::None;
        if (uci_move.length() == 5) {
            switch (uci_move[4]) {
                case 'q': promotion = PieceType::Queen; break;
                case 'r': promotion = PieceType::Rook; break;
                case 'b': promotion = PieceType::Bishop; break;
                case 'n': promotion = PieceType::Knight; break;
            }
        }
        
        // Generate legal moves to find the exact move
        Position temp_pos = pos;
        S_MOVELIST moves;
        generate_legal_moves_enhanced(temp_pos, moves);
        
        for (int i = 0; i < moves.count; ++i) {
            const S_MOVE& move = moves.moves[i];
            if (move.get_from() == from && move.get_to() == to) {
                if (promotion == PieceType::None || move.get_promoted() == promotion) {
                    return move;
                }
            }
        }
        
        return S_MOVE(); // Invalid move
    }

    std::string score_to_uci(int score) {
        if (score > MATE_IN_MAX_PLY) {
            int mate_in = (MATE_SCORE - score + 1) / 2;
            return "mate " + std::to_string(mate_in);
        } else if (score < -MATE_IN_MAX_PLY) {
            int mate_in = -(MATE_SCORE + score) / 2;
            return "mate " + std::to_string(mate_in);
        } else {
            return "cp " + std::to_string(score);
        }
    }

    std::chrono::milliseconds get_time_since(std::chrono::steady_clock::time_point start) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
    }

    std::string PVLine::to_string() const {
        std::ostringstream oss;
        for (int i = 0; i < length; ++i) {
            if (i > 0) oss << " ";
            oss << move_to_uci(moves[i]);
        }
        return oss.str();
    }

    // =================================================================
    // Move Ordering
    // =================================================================

    void MoveOrderer::clear() {
        // Clear killer moves
        for (int ply = 0; ply < MAX_PLY; ++ply) {
            killer_moves[ply][0] = S_MOVE();
            killer_moves[ply][1] = S_MOVE();
        }
        
        // Clear history table
        for (int color = 0; color < 2; ++color) {
            for (int from = 0; from < 120; ++from) {
                for (int to = 0; to < 120; ++to) {
                    history_table[color][from][to] = 0;
                }
            }
        }
    }

    void MoveOrderer::add_killer_move(int ply, const S_MOVE& move) {
        if (ply < MAX_PLY && !move.is_capture()) {
            // Shift killer moves
            if (!(killer_moves[ply][0] == move)) {
                killer_moves[ply][1] = killer_moves[ply][0];
                killer_moves[ply][0] = move;
            }
        }
    }

    void MoveOrderer::update_history(Color color, const S_MOVE& move, int depth) {
        if (!move.is_capture() && depth > 0) {
            int from = move.get_from();
            int to = move.get_to();
            if (from >= 0 && from < 120 && to >= 0 && to < 120) {
                history_table[int(color)][from][to] += depth * depth;
            }
        }
    }

    int MoveOrderer::get_move_score(const S_MOVE& move, Color color, int ply) const {
        // Move ordering scores (higher = better)
        
        // Hash move gets highest priority (handled externally)
        
        // Captures use MVV-LVA (already scored in move generation)
        if (move.is_capture()) {
            return move.score;
        }
        
        // Promotions
        if (move.is_promotion()) {
            return move.score;
        }
        
        // Killer moves
        if (ply < MAX_PLY) {
            if (killer_moves[ply][0] == move) return 9000;
            if (killer_moves[ply][1] == move) return 8000;
        }
        
        // History heuristic
        int from = move.get_from();
        int to = move.get_to();
        if (from >= 0 && from < 120 && to >= 0 && to < 120) {
            return history_table[int(color)][from][to];
        }
        
        return 0;
    }

    void MoveOrderer::order_moves(S_MOVELIST& moves, Color color, int ply, const S_MOVE& pv_move) const {
        // Score all moves
        for (int i = 0; i < moves.count; ++i) {
            S_MOVE& move = moves.moves[i];
            
            // PV move gets highest score
            if (pv_move.move != 0 && move == pv_move) {
                move.score = 1000000;
            } else {
                // Use existing score (for captures) plus heuristic score
                int base_score = move.score;
                int heuristic_score = get_move_score(move, color, ply);
                move.score = base_score + heuristic_score;
            }
        }
        
        // Sort moves by score (highest first)
        moves.sort_by_score();
    }

    // =================================================================
    // Transposition Table
    // =================================================================

    TranspositionTable::TranspositionTable(size_t size_mb) {
        resize(size_mb);
    }

    void TranspositionTable::resize(size_t size_mb) {
        size_t num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);
        
        // Round down to nearest power of 2
        size_t actual_entries = 1;
        while (actual_entries * 2 <= num_entries) {
            actual_entries *= 2;
        }
        
        table.resize(actual_entries);
        size_mask = actual_entries - 1;
        clear();
    }

    void TranspositionTable::clear() {
        std::fill(table.begin(), table.end(), TTEntry{});
        current_age = 0;
    }

    bool TranspositionTable::probe(uint64_t key, TTEntry& entry) const {
        size_t index = key & size_mask;
        const TTEntry& stored = table[index];
        
        if (stored.is_valid(key)) {
            entry = stored;
            return true;
        }
        
        return false;
    }

    void TranspositionTable::store(uint64_t key, const S_MOVE& best_move, 
                                  int score, int eval, int depth, TTFlag flag) {
        size_t index = key & size_mask;
        TTEntry& entry = table[index];
        
        // Replace if:
        // 1. Empty slot
        // 2. Same position (different depth/age)
        // 3. Deeper search
        // 4. Older entry
        bool should_replace = (entry.key == 0) || 
                             (entry.key == key) ||
                             (depth >= entry.depth) ||
                             (entry.age != current_age);
        
        if (should_replace) {
            entry.key = key;
            entry.best_move = best_move;
            entry.score = static_cast<int16_t>(score);
            entry.eval = static_cast<int16_t>(eval);
            entry.depth = static_cast<uint8_t>(depth);
            entry.flag = flag;
            entry.age = current_age;
        }
    }

    size_t TranspositionTable::get_hashfull() const {
        size_t filled = 0;
        size_t sample_size = std::min(size_t(1000), table.size());
        
        for (size_t i = 0; i < sample_size; ++i) {
            if (table[i].key != 0) filled++;
        }
        
        return (filled * 1000) / sample_size;
    }

    // =================================================================
    // Search Engine
    // =================================================================

    Engine::Engine(size_t tt_size_mb) : tt(tt_size_mb) {
        clear_hash();
    }

    S_MOVE Engine::search(const Position& pos, const SearchLimits& search_limits) {
        // Initialize search
        root_position = pos;
        limits = search_limits;
        stats.reset();
        stop_search = false;
        search_start = std::chrono::steady_clock::now();
        tt.new_search();
        
        S_MOVE best_move;
        root_pv.clear();
        
        // Generate root moves first to ensure we have legal moves
        S_MOVELIST root_moves;
        Position temp_pos = root_position;
        generate_legal_moves_enhanced(temp_pos, root_moves);
        
        // If no legal moves, return immediately
        if (root_moves.count == 0) {
            return S_MOVE(); // No legal moves available
        }
        
        // Set a fallback best move (first legal move)
        best_move = root_moves.moves[0];
        
        // Iterative deepening
        int previous_score = 0;
        bool eval_dropped_significantly = false;
        
        for (int depth = 1; depth <= limits.max_depth && !should_stop(); ++depth) {
            root_depth = depth;
            PVLine current_pv;
            
            Position search_pos = root_position;
            int score = alpha_beta(search_pos, -MATE_SCORE, MATE_SCORE, depth, 0, current_pv);
            
            if (should_stop()) break;
            
            // Check for dramatic evaluation drop (potential tactical crisis)
            if (depth >= 5 && previous_score > 200 && score < 50) {
                eval_dropped_significantly = true;
                // Extend search time when evaluation drops dramatically
                if (!limits.infinite && limits.max_time.count() < 30000) {
                    limits.max_time = std::chrono::milliseconds(limits.max_time.count() * 2);
                }
            }
            previous_score = score;
            
            // Update statistics
            stats.depth_reached = depth;
            stats.time_elapsed = get_time_since(search_start);
            stats.calculate_nps();
            
            // Update best move and PV
            if (current_pv.length > 0) {
                best_move = current_pv.moves[0];
                root_pv = current_pv;
            }
            
            // Send search info
            if (info_callback) {
                SearchInfo search_info;
                search_info.depth = depth;
                search_info.score = score;
                search_info.nodes = stats.nodes_searched;
                search_info.time_ms = static_cast<int>(stats.time_elapsed.count());
                
                // Convert PV to vector
                search_info.pv.clear();
                for (int i = 0; i < current_pv.length; ++i) {
                    search_info.pv.push_back(current_pv.moves[i]);
                }
                
                info_callback(search_info);
            }
            
            // Check for mate
            if (is_mate_score(score)) {
                break;
            }
            
            // Check time limits for single-depth searches
            if (depth == 1 && time_up()) {
                break;
            }
        }
        
        return best_move;
    }

    int Engine::alpha_beta(Position& pos, int alpha, int beta, int depth, int ply, PVLine& pv) {
        pv.clear();
        
        if (should_stop()) return alpha;
        
        stats.nodes_searched++;
        
        // Check for terminal positions
        if (ply >= MAX_PLY) {
            return Evaluation::evaluate_position(pos);
        }
        
        // Mate distance pruning
        int mate_alpha = std::max(alpha, -MATE_SCORE + ply);
        int mate_beta = std::min(beta, MATE_SCORE - ply - 1);
        if (mate_alpha >= mate_beta) {
            return mate_alpha;
        }
        alpha = mate_alpha;
        beta = mate_beta;
        
        // Transposition table lookup
        TTEntry tt_entry;
        S_MOVE hash_move;
        bool tt_hit = tt.probe(pos.zobrist_key, tt_entry);
        
        if (tt_hit) {
            hash_move = tt_entry.best_move;
            
            if (tt_entry.depth >= depth) {
                int tt_score = tt_entry.score;
                
                switch (tt_entry.flag) {
                    case TTFlag::EXACT:
                        return tt_score;
                    case TTFlag::ALPHA:
                        if (tt_score <= alpha) return alpha;
                        beta = std::min(beta, tt_score);
                        break;
                    case TTFlag::BETA:
                        if (tt_score >= beta) return beta;
                        alpha = std::max(alpha, tt_score);
                        break;
                }
                
                if (alpha >= beta) return tt_score;
            }
        }
        
        // Quiescence search at leaf nodes
        if (depth <= 0) {
            return quiescence_search(pos, alpha, beta, ply);
        }
        
        // Generate moves
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
        
        // Check for checkmate/stalemate
        if (moves.count == 0) {
            int king_square = pos.king_sq[int(pos.side_to_move)];
            if (king_square >= 0 && SqAttacked(king_square, pos, !pos.side_to_move)) {
                return -MATE_SCORE + ply; // Checkmate
            } else {
                return 0; // Stalemate
            }
        }
        
        // Check if in check
        int king_square = pos.king_sq[int(pos.side_to_move)];
        bool in_check = (king_square >= 0 && SqAttacked(king_square, pos, !pos.side_to_move));
        
        // Mate threat extension: if we have very few moves or opponent can capture our king next move
        bool extend_for_mate_threat = false;
        if (moves.count <= 3) { // Very limited mobility suggests mate threats
            extend_for_mate_threat = true;
        }
        
        // Order moves
        move_orderer.order_moves(moves, pos.side_to_move, ply, hash_move);
        
        // Null move pruning
        bool do_null_move = depth >= 3 && !in_check && ply > 0 && 
                           !is_mate_score(beta) && pos.has_non_pawn_material(pos.side_to_move);
        
        if (do_null_move) {
            // Make null move (pass turn to opponent) 
            // Save current state
            Color original_side = pos.side_to_move;
            int original_ply = pos.ply;
            
            pos.side_to_move = !pos.side_to_move;
            pos.ply++;
            pos.zobrist_key ^= Zobrist::Side; // Update zobrist for side change
            
            // Search with reduced depth
            PVLine null_pv;
            int null_score = -alpha_beta(pos, -beta, -beta + 1, depth - 3, ply + 1, null_pv);
            
            // Restore state
            pos.side_to_move = original_side;
            pos.ply = original_ply;
            pos.zobrist_key ^= Zobrist::Side; // Restore zobrist
            
            // Null move cutoff
            if (null_score >= beta) {
                return beta; // Null move pruning cutoff
            }
        }
        
        // Search variables
        int best_score = -MATE_SCORE;
        S_MOVE best_move;
        TTFlag tt_flag = TTFlag::ALPHA;
        int original_alpha = alpha;
        
        // Search all moves
        for (int i = 0; i < moves.count; ++i) {
            const S_MOVE& move = moves.moves[i];
            
            // Make move
            pos.make_move_with_undo(move);
            
            // Check extensions: extend search by 1 ply if move gives check
            int extension = 0;
            int king_square = pos.king_sq[int(pos.side_to_move)];
            if (king_square >= 0 && SqAttacked(king_square, pos, !pos.side_to_move)) {
                extension = 1; // Extend search when in check
            }
            // Also extend for mate threat situations
            else if (extend_for_mate_threat && depth <= 2) {
                extension = 1; // Extend when in potential mate threat with low depth
            }
            
            PVLine child_pv;
            int score;
            
            // Principal Variation Search with Late Move Reductions
            if (i == 0) {
                // Search first move with full window
                score = -alpha_beta(pos, -beta, -alpha, depth - 1 + extension, ply + 1, child_pv);
            } else {
                // Late Move Reduction (LMR)
                int reduction = 0;
                if (i >= 4 && depth >= 3 && !in_check && !extension && 
                    !move.is_capture() && !move.is_promotion()) {
                    reduction = 1; // Reduce depth by 1 for late quiet moves
                }
                
                // Search with null window and possible reduction
                score = -alpha_beta(pos, -alpha - 1, -alpha, depth - 1 + extension - reduction, ply + 1, child_pv);
                
                // Re-search if necessary (either score improved or reduction was applied)
                if ((score > alpha && score < beta) || reduction > 0) {
                    score = -alpha_beta(pos, -beta, -alpha, depth - 1 + extension, ply + 1, child_pv);
                }
            }
            
            // Undo move
            pos.undo_move();
            
            if (should_stop()) return alpha;
            
            // Update best score
            if (score > best_score) {
                best_score = score;
                best_move = move;
                
                // Update PV
                pv.clear();
                pv.add_move(move);
                for (int j = 0; j < child_pv.length; ++j) {
                    pv.add_move(child_pv.moves[j]);
                }
                
                // Alpha-beta cutoff
                if (score >= beta) {
                    // Beta cutoff - update heuristics
                    move_orderer.add_killer_move(ply, move);
                    move_orderer.update_history(pos.side_to_move, move, depth);
                    
                    // Store in transposition table
                    tt.store(pos.zobrist_key, best_move, best_score, 
                            Evaluation::evaluate_position(pos), depth, TTFlag::BETA);
                    
                    return beta;
                }
                
                if (score > alpha) {
                    alpha = score;
                    tt_flag = TTFlag::EXACT;
                }
            }
        }
        
        // Store in transposition table
        tt.store(pos.zobrist_key, best_move, best_score, 
                Evaluation::evaluate_position(pos), depth, tt_flag);
        
        return best_score;
    }

    int Engine::quiescence_search(Position& pos, int alpha, int beta, int ply) {
        if (should_stop()) return alpha;
        
        stats.qnodes_searched++;
        
        if (ply >= MAX_PLY) {
            return Evaluation::evaluate_position(pos);
        }
        
        // Stand pat
        int stand_pat = Evaluation::evaluate_position(pos);
        
        if (stand_pat >= beta) {
            return beta;
        }
        
        if (stand_pat > alpha) {
            alpha = stand_pat;
        }
        
        // Generate all moves for quiescence (captures and checks)
        S_MOVELIST moves;
        generate_all_moves(pos, moves);
        
        // Filter to captures and checks, and order
        S_MOVELIST tactical_moves;
        for (int i = 0; i < moves.count; ++i) {
            const S_MOVE& move = moves.moves[i];
            
            // Include captures
            if (move.is_capture()) {
                tactical_moves.moves[tactical_moves.count++] = move;
                continue;
            }
            
            // Include checks (test by making the move temporarily)
            pos.make_move_with_undo(move);
            bool gives_check = in_check(pos);
            pos.undo_move();
            
            if (gives_check) {
                tactical_moves.moves[tactical_moves.count++] = move;
            }
        }
        
        // Order tactical moves by MVV-LVA (already scored)
        tactical_moves.sort_by_score();
        
        // Search tactical moves (captures and checks)
        for (int i = 0; i < tactical_moves.count; ++i) {
            const S_MOVE& tactical_move = tactical_moves.moves[i];
            
            // Make move
            pos.make_move_with_undo(tactical_move);
            int score = -quiescence_search(pos, -beta, -alpha, ply + 1);
            pos.undo_move();
            
            if (should_stop()) return alpha;
            
            if (score >= beta) {
                return beta;
            }
            
            if (score > alpha) {
                alpha = score;
            }
        }
        
        return alpha;
    }

    bool Engine::should_stop() const {
        if (stop_search.load()) return true;
        
        // Check time limits
        if (time_up()) return true;
        
        // Check node limits
        if (stats.nodes_searched >= limits.max_nodes) return true;
        
        return false;
    }

    bool Engine::time_up() const {
        if (limits.infinite) return false;
        
        auto elapsed = get_time_since(search_start);
        return elapsed >= limits.max_time;
    }

    bool Engine::is_mate_score(int score) const {
        return std::abs(score) > MATE_IN_MAX_PLY;
    }

    int Engine::mate_distance(int score) const {
        if (score > MATE_IN_MAX_PLY) {
            return (MATE_SCORE - score + 1) / 2;
        } else if (score < -MATE_IN_MAX_PLY) {
            return -(MATE_SCORE + score) / 2;
        }
        return 0;
    }

    std::chrono::milliseconds Engine::calculate_search_time() const {
        if (limits.infinite || limits.max_time.count() == INFINITE_TIME) {
            return std::chrono::milliseconds(INFINITE_TIME);
        }
        
        // Simple time management
        if (limits.remaining_time.count() > 0) {
            int time_for_move = limits.remaining_time.count() / 30; // Aim for 30 moves
            time_for_move += limits.increment.count() / 2; // Use half of increment
            return std::chrono::milliseconds(std::max(100, time_for_move));
        }
        
        return limits.max_time;
    }

    void Engine::send_search_info(int depth, int score, const PVLine& pv) const {
        auto elapsed = get_time_since(search_start);
        
        std::cout << "info"
                  << " depth " << depth
                  << " score " << score_to_uci(score)
                  << " nodes " << stats.nodes_searched
                  << " time " << elapsed.count()
                  << " nps " << static_cast<uint64_t>(stats.nodes_per_second);
        
        if (pv.length > 0) {
            std::cout << " pv " << pv.to_string();
        }
        
        std::cout << std::endl;
    }

    void Engine::send_bestmove(const S_MOVE& move) const {
        std::cout << "bestmove " << move_to_uci(move) << std::endl;
    }

    void Engine::send_info(const std::string& info) const {
        std::cout << "info string " << info << std::endl;
    }

} // namespace Search
