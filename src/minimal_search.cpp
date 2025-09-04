#include "minimal_search.hpp"
#include "evaluation.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>  // For std::setw

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
    
    // Simple development bonus to discourage excessive pawn moves
    // Count developed pieces (not on starting squares)
    int white_development = 0;
    int black_development = 0;
    
    // Using direct 120-square board indices (safer than sq() function calls)
    // b1=22, g1=27, b8=92, g8=97 for knights
    // c1=23, f1=26, c8=93, f8=96 for bishops  
    // e1=25, e8=95 for kings
    
    // Check if knights are developed (not on starting squares)
    if (pos.board[22] != Piece::WhiteKnight) white_development += 50; // b1
    if (pos.board[27] != Piece::WhiteKnight) white_development += 50; // g1
    if (pos.board[92] != Piece::BlackKnight) black_development += 50; // b8
    if (pos.board[97] != Piece::BlackKnight) black_development += 50; // g8
    
    // Check if bishops are developed (not on starting squares)
    if (pos.board[23] != Piece::WhiteBishop) white_development += 30; // c1
    if (pos.board[26] != Piece::WhiteBishop) white_development += 30; // f1
    if (pos.board[93] != Piece::BlackBishop) black_development += 30; // c8
    if (pos.board[96] != Piece::BlackBishop) black_development += 30; // f8
    
    // Bonus for castling (king not on starting square)
    if (pos.board[25] != Piece::WhiteKing) white_development += 40; // e1
    if (pos.board[95] != Piece::BlackKing) black_development += 40; // e8
    
    score += white_development - black_development;
    
    // Center control bonus - encourage e4/d4 instead of e3/d3
    // Give bonus for pawns controlling center squares
    int center_bonus = 0;
    
    // Direct indices: e4=54, d4=53, e5=64, d5=65, e3=44, d3=43, e6=74, d6=75
    // MASSIVE bonuses for 4th rank center pawns
    if (pos.board[54] == Piece::WhitePawn) center_bonus += 100; // e4
    if (pos.board[53] == Piece::WhitePawn) center_bonus += 100; // d4
    if (pos.board[64] == Piece::BlackPawn) center_bonus -= 100; // e5  
    if (pos.board[65] == Piece::BlackPawn) center_bonus -= 100; // d5
    
    // Small bonus for supporting center from 3rd rank
    if (pos.board[44] == Piece::WhitePawn) center_bonus += 20; // e3
    if (pos.board[43] == Piece::WhitePawn) center_bonus += 20; // d3
    if (pos.board[74] == Piece::BlackPawn) center_bonus -= 20; // e6
    if (pos.board[75] == Piece::BlackPawn) center_bonus -= 20; // d6
    
    score += center_bonus;
    
    // Opening principles: penalty for moving pawns off starting squares early
    // This discourages excessive pawn pushes like h3, a3, h4, etc.
    int pawn_penalty = 0;
    
    // Count pawns that have moved from starting positions
    int white_pawn_moves = 0;
    int black_pawn_moves = 0;
    
    // White starting pawns on rank 2 (a2=31, b2=32, ..., h2=38)
    for (int sq = 31; sq <= 38; ++sq) {
        if (pos.board[sq] != Piece::WhitePawn) {
            white_pawn_moves++;
        }
    }
    
    // Black starting pawns on rank 7 (a7=81, b7=82, ..., h7=88)
    for (int sq = 81; sq <= 88; ++sq) {
        if (pos.board[sq] != Piece::BlackPawn) {
            black_pawn_moves++;
        }
    }
    
    // Penalty increases with number of pawn moves (discourage pawn-heavy openings)
    // MASSIVE penalties to prevent 16-move pawn marathons!
    if (white_pawn_moves > 2) {
        int excess = white_pawn_moves - 2;
        pawn_penalty += excess * 50 + (excess * excess * 25); // Exponential penalty!
    }
    if (black_pawn_moves > 2) {
        int excess = black_pawn_moves - 2;
        pawn_penalty -= excess * 50 + (excess * excess * 25); // Exponential penalty!
    }
    
    // Additional tempo penalty: heavily discourage undeveloped pieces in mid-game
    // If we have many pawn moves but no piece development, apply severe penalty
    int tempo_penalty = 0;
    if (white_pawn_moves >= 4 && white_development < 50) {
        tempo_penalty += 200; // Huge penalty for pawn-heavy, piece-light positions
    }
    if (black_pawn_moves >= 4 && black_development < 50) {
        tempo_penalty -= 200; // Huge penalty for pawn-heavy, piece-light positions
    }
    
    score -= tempo_penalty;
    
    score -= pawn_penalty;
    
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

void MinimalEngine::check_up(SearchInfo& info) {
    // VICE style time checking function
    if (time_up()) {
        info.stopped = true;
        return;
    }

    // Also respect the per-search stop_time provided via SearchInfo
    auto now = std::chrono::steady_clock::now();
    if (!info.infinite) {
        auto stop_time = info.stop_time;
        if (stop_time != std::chrono::steady_clock::time_point{}) {
            if (now >= stop_time) info.stopped = true;
        }
    }

    // If the engine's external should_stop flag has been set (via UCI stop), honor it
    if (should_stop) {
        info.stopped = true;
    }
}

std::string MinimalEngine::format_uci_score(int score) const {
    // Convert engine score to proper UCI format
    // MATE = 29000, so scores close to +/-MATE are mate scores
    
    if (score > MATE - 100) {
        // Positive mate score: we are mating opponent
        int mate_in = (MATE - score + 1) / 2;
        return "mate " + std::to_string(mate_in);
    } else if (score < -MATE + 100) {
        // Negative mate score: we are being mated
        int mate_in = -((-MATE - score + 1) / 2);
        return "mate " + std::to_string(mate_in);
    } else {
        // Regular centipawn score
        return "cp " + std::to_string(score);
    }
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
    // Conservative repetition detection to avoid false positives in mate searches
    // Only check for repetition in actual game positions, not during deep search
    
    // Don't check for repetition if move history is too short
    if (pos.move_history.size() < 6) {
        return false; // Need at least 6 plies for meaningful repetition check
    }
    
    // Be very conservative - only detect clear 3-fold repetitions
    uint64_t current_key = pos.zobrist_key;
    int repetition_count = 1; // Count current position
    
    // Only check the last 12 moves to avoid false positives
    int start_check = std::max(0, static_cast<int>(pos.move_history.size()) - 12);
    
    for (int index = start_check; index < static_cast<int>(pos.move_history.size()) - 1; ++index) {
        if (current_key == pos.move_history[index].zobrist_key) {
            repetition_count++;
        }
    }
    
    // Only return true for definite 3-fold repetition to be safe
    return repetition_count >= 3;
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
    for (int depth = 0; depth < 64; ++depth) {
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
int MinimalEngine::get_pv_line(Position& pos, int depth, S_MOVE pv_array[64]) {
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
    if (move.move == 0 || depth < 0 || depth >= 64) return;
    
    // Only store non-capture moves as killers
    if (!move.is_capture()) {
        // If this move isn't already first killer, shift and add
        if (search_killers[depth][0].move != move.move) {
            search_killers[depth][1] = search_killers[depth][0];  // Second = old first
            search_killers[depth][0] = move;                      // First = new move
        }
    }
}

// Initialize MVV-LVA (Most Valuable Victim, Least Valuable Attacker) scoring table
void MinimalEngine::init_mvv_lva() {
    // Piece values for MVV-LVA (using standard values)
    int piece_values[7] = {
        0,    // None
        100,  // Pawn
        300,  // Knight  
        350,  // Bishop
        500,  // Rook
        1000, // Queen
        0     // King (should never be captured)
    };
    
    // Initialize MVV-LVA scores
    // Higher scores = better captures to search first
    // Formula: (victim_value * 100) + (600 - attacker_value)
    // This prioritizes: valuable victims + cheap attackers
    for (int victim = 0; victim < 7; victim++) {
        for (int attacker = 0; attacker < 7; attacker++) {
            if (victim == 0) {
                // No victim = not a capture
                mvv_lva_scores[victim][attacker] = 0;
            } else {
                // Valuable victim + cheap attacker = high score
                // Example: Pawn(100) takes Queen(1000) = (1000 * 100) + (600 - 100) = 100,500
                // Example: Queen(1000) takes Pawn(100) = (100 * 100) + (600 - 1000) = 9,600
                mvv_lva_scores[victim][attacker] = (piece_values[victim] * 100) + (600 - piece_values[attacker]);
            }
        }
    }
}

// Get MVV-LVA score for a capture move
int MinimalEngine::get_mvv_lva_score(PieceType victim, PieceType attacker) const {
    int victim_index = static_cast<int>(victim);
    int attacker_index = static_cast<int>(attacker);
    
    // Bounds checking
    if (victim_index < 0 || victim_index >= 7 || attacker_index < 0 || attacker_index >= 7) {
        return 0;
    }
    
    return mvv_lva_scores[victim_index][attacker_index];
}

// Order moves using MVV-LVA and other heuristics
void MinimalEngine::order_moves(std::vector<S_MOVE>& moves, const Position& pos) const {
    // Assign scores to each move for ordering
    for (auto& move : moves) {
        int score = 0;
        
        if (move.is_capture()) {
            // Captures: Use MVV-LVA scoring
            PieceType victim = move.get_captured();
            
            // Get the attacking piece type from the position
            int from_sq = move.get_from();
            Piece attacking_piece = pos.board[from_sq];
            PieceType attacker = type_of(attacking_piece);
            
            score = get_mvv_lva_score(victim, attacker);
            
            // Bonus for en passant captures (always pawn takes pawn)
            if (move.is_en_passant()) {
                score += 10000;  // High priority for en passant
            }
            
        } else if (move.is_promotion()) {
            // Promotions: High priority, queen promotion highest
            PieceType promoted = move.get_promoted();
            switch (promoted) {
                case PieceType::Queen:  score = 90000; break;
                case PieceType::Rook:   score = 50000; break;
                case PieceType::Bishop: score = 35000; break;
                case PieceType::Knight: score = 30000; break;
                default: score = 25000; break;
            }
            
        } else {
            // Quiet moves: Lower priority
            // Could add killer moves, history heuristic here later
            score = 1000;  // Base score for quiet moves
        }
        
        move.score = score;
    }
    
    // Sort moves by score (highest first)
    std::sort(moves.begin(), moves.end(), [](const S_MOVE& a, const S_MOVE& b) {
        return a.score > b.score;
    });
}

// Order moves in S_MOVELIST using MVV-LVA and other heuristics
void MinimalEngine::order_moves(S_MOVELIST& move_list, const Position& pos) const {
    // Assign scores to each move for ordering
    for (int i = 0; i < move_list.count; i++) {
        S_MOVE& move = move_list.moves[i];
        int score = 0;
        
        if (move.is_capture()) {
            // Captures: Use MVV-LVA scoring
            PieceType victim = move.get_captured();
            
            // Get the attacking piece type from the position
            int from_sq = move.get_from();
            Piece attacking_piece = pos.board[from_sq];
            PieceType attacker = type_of(attacking_piece);
            
            score = get_mvv_lva_score(victim, attacker);
            
            // Bonus for en passant captures (always pawn takes pawn)
            if (move.is_en_passant()) {
                score += 10000;  // High priority for en passant
            }
            
        } else if (move.is_promotion()) {
            // Promotions: High priority, queen promotion highest
            PieceType promoted = move.get_promoted();
            switch (promoted) {
                case PieceType::Queen:  score = 90000; break;
                case PieceType::Rook:   score = 50000; break;
                case PieceType::Bishop: score = 35000; break;
                case PieceType::Knight: score = 30000; break;
                default: score = 25000; break;
            }
            
        } else {
            // Quiet moves: Lower priority
            // Could add killer moves, history heuristic here later
            score = 1000;  // Base score for quiet moves
        }
        
        move.score = score;
    }
    
    // Sort moves by score (highest first) using C-style array sort
    std::sort(&move_list.moves[0], &move_list.moves[move_list.count], 
              [](const S_MOVE& a, const S_MOVE& b) {
                  return a.score > b.score;
              });
}

// VICE Part 62: Pick Next Move - Select best move from remaining moves
// This is more efficient than sorting all moves upfront
int MinimalEngine::pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos, int depth) const {
    // For the first call (move_num == 0), score all moves using VICE Part 64 ordering
    if (move_num == 0) {
        // Get PV move for this position (if any)
        S_MOVE pv_move;
        bool has_pv_move = pv_table.probe_move(pos.zobrist_key, pv_move);
        
        // Score all moves for ordering
        for (int i = 0; i < move_list.count; i++) {
            S_MOVE& move = move_list.moves[i];
            int score = 0;
            
            // VICE Part 64: PV move gets highest priority (2,000,000)
            if (has_pv_move && move.move == pv_move.move) {
                score = 2000000;
                
            } else if (move.is_capture()) {
                // VICE Part 64: Captures get 1,000,000 + MVV-LVA score
                PieceType victim = move.get_captured();
                
                // Get the attacking piece type from the position
                int from_sq = move.get_from();
                Piece attacking_piece = pos.board[from_sq];
                PieceType attacker = type_of(attacking_piece);
                
                score = 1000000 + get_mvv_lva_score(victim, attacker);
                
                // Bonus for en passant captures (always pawn takes pawn)
                if (move.is_en_passant()) {
                    score += 10000;  // High priority for en passant
                }
                
            } else {
                // Check for killer moves (non-captures only)
                bool is_killer = false;
                if (depth >= 0 && depth < 64) {
                    // VICE Part 64: First killer = 900,000, Second killer = 800,000
                    if (search_killers[depth][0].move == move.move) {
                        score = 900000;
                        is_killer = true;
                    } else if (search_killers[depth][1].move == move.move) {
                        score = 800000;
                        is_killer = true;
                    }
                }
                
                if (!is_killer) {
                    if (move.is_promotion()) {
                        // Promotions: High priority, queen promotion highest
                        PieceType promoted = move.get_promoted();
                        switch (promoted) {
                            case PieceType::Queen:  score = 90000; break;
                            case PieceType::Rook:   score = 50000; break;
                            case PieceType::Bishop: score = 35000; break;
                            case PieceType::Knight: score = 30000; break;
                            default: score = 25000; break;
                        }
                    } else {
                        // VICE Part 64: History heuristic for remaining quiet moves
                        int from = move.get_from();
                        int to = move.get_to();
                        
                        if (from >= 0 && from < 120 && to >= 0 && to < 120) {
                            Piece piece = pos.board[from];
                            int piece_index = static_cast<int>(piece) % 13;
                            score = search_history[piece_index][to];  // History score
                        } else {
                            score = 1000;  // Base score for quiet moves
                        }
                    }
                }
            }
            
            move.score = score;
        }
    }
    
    // Find the best move from move_num onwards
    int best_score = -1;
    int best_index = move_num;
    
    for (int i = move_num; i < move_list.count; i++) {
        if (move_list.moves[i].score > best_score) {
            best_score = move_list.moves[i].score;
            best_index = i;
        }
    }
    
    // Swap the best move to the current position
    if (best_index != move_num) {
        S_MOVE temp = move_list.moves[move_num];
        move_list.moves[move_num] = move_list.moves[best_index];
        move_list.moves[best_index] = temp;
    }
    
    return best_score;
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
    
    // Use SearchInfo for consistent node counting
    uint64_t total_nodes = 0;
    
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
            
            // Create temporary SearchInfo for this search
            SearchInfo temp_info;
            temp_info.ply = 0;
            temp_info.stopped = false;
            
            // Use consistent VICE-style AlphaBeta search (not old alpha_beta)
            int score = -AlphaBeta(pos, -INFINITE, INFINITE, depth - 1, temp_info, true, false);
            pos.TakeMove();
            
            // Accumulate nodes from this search
            total_nodes += temp_info.nodes;
            
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
            S_MOVE pv_array[64];
            int pv_moves = get_pv_line(pos, depth, pv_array);
            
            std::cout << "info depth " << depth 
                     << " score " << format_uci_score(best_score)
                     << " nodes " << total_nodes 
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
    
    // Update nodes_searched for backward compatibility
    nodes_searched = static_cast<int>(total_nodes);
    
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
    
    // Skip time management if this is a depth-only search (UCI go depth command)
    if (info.depth_only) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time);
    
    // Check time limit (if not infinite search)
    if (!info.infinite) {
        // Use SearchInfo stop_time if available, otherwise fall back to default
        auto stop_time = info.stop_time;
        if (stop_time == std::chrono::steady_clock::time_point{}) {
            // No stop_time set, use default 5 seconds
            stop_time = info.start_time + std::chrono::milliseconds(5000);
        }
        
        if (now >= stop_time) {
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
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, depth);
        
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
                
                // VICE Part 64: Update history heuristic for non-capture moves that improve alpha
                if (!move_list.moves[i].is_capture()) {
                    update_search_history(pos, move_list.moves[i], depth);
                }
                
                if (alpha >= beta) {
                    // VICE Part 60: Track fail high statistics (0:13)
                    info.fh++; // Increment fail high count
                    if (i == 0) {
                        info.fhf++; // Fail high first (first move caused beta cutoff)
                    }
                    
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
    
    // VICE Part 65: Generate only capture moves for quiescence search
    S_MOVELIST move_list;
    generate_all_caps(pos, move_list);  // Only captures - more efficient than filtering
    
    // Search all capture moves
    for (int i = 0; i < move_list.count; ++i) {
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, -1);  // No depth in quiescence
        
        S_MOVE move = move_list.moves[i];
        
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
        S_MOVE pv_array[64];
        int pv_moves = get_pv_line(pos, current_depth, pv_array);
        
        // Print results after each completed depth (3:03, 5:32)
        std::cout << "info depth " << current_depth 
                  << " score " << format_uci_score(best_score)
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
        // Skip this if depth_only is set (UCI go depth command)
        if (!info.infinite && !info.depth_only && elapsed.count() > 3000) {  // If we've used 3+ seconds
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
