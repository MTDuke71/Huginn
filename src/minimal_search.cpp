#include "minimal_search.hpp"
#include "evaluation.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include "input_checking.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>  // For std::setw

namespace Huginn {

// VICE Tutorial: Mirror arrays for evaluation symmetry testing
// mirror64: maps 64-square indices to their vertical mirror (rank 1 <-> rank 8)
static const int mirror64[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,  // rank 1 -> rank 8
    48, 49, 50, 51, 52, 53, 54, 55,  // rank 2 -> rank 7
    40, 41, 42, 43, 44, 45, 46, 47,  // rank 3 -> rank 6
    32, 33, 34, 35, 36, 37, 38, 39,  // rank 4 -> rank 5
    24, 25, 26, 27, 28, 29, 30, 31,  // rank 5 -> rank 4
    16, 17, 18, 19, 20, 21, 22, 23,  // rank 6 -> rank 3
     8,  9, 10, 11, 12, 13, 14, 15,  // rank 7 -> rank 2
     0,  1,  2,  3,  4,  5,  6,  7   // rank 8 -> rank 1
};

// Function to swap piece colors using the bit-packed Piece enum
Piece swapPieceColor(Piece piece) {
    if (piece == Piece::None || piece == Piece::Offboard) return piece;
    
    // Extract color and piece type
    Color color = color_of(piece);
    PieceType type = type_of(piece);
    
    // Flip the color
    Color new_color = (color == Color::White) ? Color::Black : Color::White;
    
    // Create the new piece with flipped color
    return make_piece(new_color, type);
}

int MinimalEngine::evaluate(const Position& pos) {
    // VICE Part 82: Check for material draw first (2:03)
    if (pos.get_white_pawns() == 0 && pos.get_black_pawns() == 0 && MaterialDraw(pos)) {
        return 0; // Return draw score for insufficient material
    }
    
    // VICE Part 56: Basic Evaluation with piece-square tables
    int score = 0;
    
    // VICE Part 82: Use pre-existing material tracking for endgame detection
    // This is much more efficient than manually counting material
    int total_material = pos.get_total_material();
    bool is_endgame = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD);
    
    // SECOND PASS: Evaluate all pieces using pre-calculated endgame status
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard) continue;
        
        Piece piece = pos.board[sq];
        if (piece == Piece::None) continue;
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values (VICE-compatible)
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   material_value = 100; break;
            case PieceType::Knight: material_value = 320; break;
            case PieceType::Bishop: material_value = 330; break;
            case PieceType::Rook:   material_value = 500; break;
            case PieceType::Queen:  material_value = 900; break;
            case PieceType::King:   material_value = 20000; break;
            default: material_value = 0; break;
        }
        
        // Convert square120 to square64 for piece-square tables
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue; // Invalid square
        
        // Get piece-square table value
        int pst_value = 0;
        int table_index = (piece_color == Color::Black) ? mirror_square_64(sq64) : sq64;
        
        // VICE Part 82: Use different king tables based on pre-calculated material
        if (piece_type == PieceType::King) {
            if (is_endgame) {
                pst_value = EvalParams::KING_TABLE_ENDGAME[table_index];
            } else {
                pst_value = EvalParams::KING_TABLE[table_index];
            }
        } else {
            switch (piece_type) {
                case PieceType::Pawn:   pst_value = EvalParams::PAWN_TABLE[table_index]; break;
                case PieceType::Knight: pst_value = EvalParams::KNIGHT_TABLE[table_index]; break;
                case PieceType::Bishop: pst_value = EvalParams::BISHOP_TABLE[table_index]; break;
                case PieceType::Rook:   pst_value = EvalParams::ROOK_TABLE[table_index]; break;
                case PieceType::Queen:  pst_value = EvalParams::QUEEN_TABLE[table_index]; break;
                default: pst_value = 0; break;
            }
        }
        
        // Add material + piece-square value for this piece
        int piece_value = material_value + pst_value;
        
        if (piece_color == Color::White) {
            score += piece_value;
        } else {
            score -= piece_value;
        }
    }
    
    // VICE Part 80: Enhanced pawn structure evaluation with pre-computed masks
    // Evaluate isolated pawns (2:13, 3:07) and passed pawns (2:21, 4:25)
    int pawn_structure_score = 0;
    
    // Get bitboards for efficient pawn structure analysis
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    
    // Check all pawns for isolated and passed pawn characteristics
    for (int sq64 = 0; sq64 < 64; ++sq64) {
        int sq120 = (sq64 / 8 + 2) * 10 + (sq64 % 8 + 1);
        Piece piece = pos.board[sq120];
        
        if (piece == Piece::WhitePawn) {
            int file = sq64 % 8;
            int rank = sq64 / 8;
            
            // Check for isolated pawn using pre-computed masks (3:07)
            if ((white_pawns & EvalParams::ISOLATED_PAWN_MASKS[file]) == 0) {
                pawn_structure_score -= EvalParams::ISOLATED_PAWN_PENALTY;
            }
            
            // Check for passed pawn using pre-computed masks (4:25)
            if ((black_pawns & EvalParams::WHITE_PASSED_PAWN_MASKS[sq64]) == 0) {
                pawn_structure_score += EvalParams::PASSED_PAWN_BONUS[rank];
            }
            
        } else if (piece == Piece::BlackPawn) {
            int file = sq64 % 8;
            int rank = sq64 / 8;
            
            // Check for isolated pawn using pre-computed masks  
            if ((black_pawns & EvalParams::ISOLATED_PAWN_MASKS[file]) == 0) {
                pawn_structure_score += EvalParams::ISOLATED_PAWN_PENALTY; // Penalty for black
            }
            
            // Check for passed pawn using pre-computed masks
            if ((white_pawns & EvalParams::BLACK_PASSED_PAWN_MASKS[sq64]) == 0) {
                int mirror_rank = 7 - rank; // Mirror rank for black (rank increases as black advances)
                pawn_structure_score -= EvalParams::PASSED_PAWN_BONUS[mirror_rank]; // Bonus for black
            }
        }
    }
    
    score += pawn_structure_score;
    
    // VICE Part 81: Open and semi-open file bonuses for rooks and queens
    // Evaluate rooks and queens on open files (no pawns) or semi-open files (no own pawns)
    int file_bonus_score = 0;
    
    uint64_t all_pawns = white_pawns | black_pawns;
    
    // Evaluate all squares for rooks and queens
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue; // Invalid square
        
        int file = sq64 % 8; // File A=0, B=1, ..., H=7
        uint64_t file_mask = EvalParams::FILE_MASKS[file];
        
        if (piece == Piece::WhiteRook) {
            // Check for open file (no pawns from either side)
            if ((all_pawns & file_mask) == 0) {
                file_bonus_score += EvalParams::ROOK_OPEN_FILE_BONUS;
            }
            // Check for semi-open file (no white pawns on this file)
            else if ((white_pawns & file_mask) == 0) {
                file_bonus_score += EvalParams::ROOK_SEMI_OPEN_FILE_BONUS;
            }
        } else if (piece == Piece::BlackRook) {
            // Check for open file (no pawns from either side)
            if ((all_pawns & file_mask) == 0) {
                file_bonus_score -= EvalParams::ROOK_OPEN_FILE_BONUS;
            }
            // Check for semi-open file (no black pawns on this file)
            else if ((black_pawns & file_mask) == 0) {
                file_bonus_score -= EvalParams::ROOK_SEMI_OPEN_FILE_BONUS;
            }
        } else if (piece == Piece::WhiteQueen) {
            // Check for open file (no pawns from either side)
            if ((all_pawns & file_mask) == 0) {
                file_bonus_score += EvalParams::QUEEN_OPEN_FILE_BONUS;
            }
            // Check for semi-open file (no white pawns on this file)
            else if ((white_pawns & file_mask) == 0) {
                file_bonus_score += EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS;
            }
        } else if (piece == Piece::BlackQueen) {
            // Check for open file (no pawns from either side)
            if ((all_pawns & file_mask) == 0) {
                file_bonus_score -= EvalParams::QUEEN_OPEN_FILE_BONUS;
            }
            // Check for semi-open file (no black pawns on this file)
            else if ((black_pawns & file_mask) == 0) {
                file_bonus_score -= EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS;
            }
        }
    }
    
    score += file_bonus_score;
    
    // VICE Part 83: Bishop pair bonus
    int white_bishops = pos.pCount[int(Color::White)][int(PieceType::Bishop)];
    int black_bishops = pos.pCount[int(Color::Black)][int(PieceType::Bishop)];
    
    if (white_bishops >= 2) {
        score += EvalParams::BISHOP_PAIR_BONUS;
    }
    if (black_bishops >= 2) {
        score -= EvalParams::BISHOP_PAIR_BONUS;
    }
    
    // Return from current side's perspective (negate if black to move)
    return (pos.side_to_move == Color::White) ? score : -score;
}

// VICE Part 82/83: Material draw detection - Fixed to be more conservative
// Checks if the position is a theoretical draw based on insufficient material
bool MinimalEngine::MaterialDraw(const Position& pos) {
    // Use efficient piece count arrays instead of looping through all squares
    int white_rooks = pos.pCount[int(Color::White)][int(PieceType::Rook)];
    int black_rooks = pos.pCount[int(Color::Black)][int(PieceType::Rook)];
    int white_queens = pos.pCount[int(Color::White)][int(PieceType::Queen)];
    int black_queens = pos.pCount[int(Color::Black)][int(PieceType::Queen)];
    int white_bishops = pos.pCount[int(Color::White)][int(PieceType::Bishop)];
    int black_bishops = pos.pCount[int(Color::Black)][int(PieceType::Bishop)];
    int white_knights = pos.pCount[int(Color::White)][int(PieceType::Knight)];
    int black_knights = pos.pCount[int(Color::Black)][int(PieceType::Knight)];
    
    // If either side has rooks or queens, not a draw
    if (white_rooks > 0 || black_rooks > 0 || white_queens > 0 || black_queens > 0) {
        return false;
    }
    
    // Only minor pieces remain (bishops and knights)
    int white_pieces = white_bishops + white_knights;
    int black_pieces = black_bishops + black_knights;
    
    // Classic insufficient material cases:
    // 1. K vs K
    if (white_pieces == 0 && black_pieces == 0) {
        return true;
    }
    
    // 2. K+N vs K or K+B vs K
    if ((white_pieces <= 1 && black_pieces == 0) || (black_pieces <= 1 && white_pieces == 0)) {
        return true;
    }
    
    // 3. K+B vs K+B with bishops on same color squares (more complex, skip for now)
    // 4. K+N vs K+N is generally drawn but can have winning positions
    
    // Be conservative - only claim draw for the most obvious cases
    return false;
}

// Helper functions for evaluation (Part 56)
// Mirror square for black pieces (flip vertically)
int MinimalEngine::mirror_square_64(int sq64) {
    if (sq64 < 0 || sq64 > 63) return sq64;
    return ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
}

// VICE Tutorial: Mirror Board function for evaluation testing
// Creates a mirrored copy of the position for symmetry testing
Position MinimalEngine::mirrorBoard(const Position& pos) {
    Position mirrored_pos;
    
    // Clear the board first
    for (int sq = 0; sq < 120; ++sq) {
        mirrored_pos.board[sq] = pos.board[sq] == Piece::Offboard ? Piece::Offboard : Piece::None;
    }
    
    // Mirror all pieces on the board
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        // Convert 120-square to 64-square, mirror it, then back to 120-square
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue; // Invalid square
        
        int mirrored_sq64 = mirror64[sq64];
        int mirrored_sq120 = MAILBOX_MAPS.to120[mirrored_sq64];
        
        // Swap the piece color using the proper function
        Piece original_piece = pos.board[sq];
        mirrored_pos.board[mirrored_sq120] = swapPieceColor(original_piece);
    }
    
    // Flip the side to move
    mirrored_pos.side_to_move = (pos.side_to_move == Color::White) ? Color::Black : Color::White;
    
    // Mirror castling permissions
    mirrored_pos.castling_rights = 0;
    if (pos.castling_rights & CASTLE_WK) mirrored_pos.castling_rights |= CASTLE_BK;
    if (pos.castling_rights & CASTLE_WQ) mirrored_pos.castling_rights |= CASTLE_BQ;
    if (pos.castling_rights & CASTLE_BK) mirrored_pos.castling_rights |= CASTLE_WK;
    if (pos.castling_rights & CASTLE_BQ) mirrored_pos.castling_rights |= CASTLE_WQ;
    
    // Mirror en passant square
    if (pos.ep_square >= 0) {
        int ep_sq64 = MAILBOX_MAPS.to64[pos.ep_square];
        if (ep_sq64 >= 0) {
            int mirrored_ep_sq64 = mirror64[ep_sq64];
            mirrored_pos.ep_square = MAILBOX_MAPS.to120[mirrored_ep_sq64];
        } else {
            mirrored_pos.ep_square = -1;
        }
    } else {
        mirrored_pos.ep_square = -1;
    }
    
    // Copy other fields
    mirrored_pos.halfmove_clock = pos.halfmove_clock;
    mirrored_pos.fullmove_number = pos.fullmove_number;
    
    // Rebuild derived state for the mirrored position
    mirrored_pos.rebuild_counts();
    mirrored_pos.update_zobrist_key();
    
    return mirrored_pos;
}

// VICE Part 80: Mirror evaluation test for debugging symmetry issues
// Tests if evaluation is symmetric when board is mirrored (0:15, 0:31)
void MinimalEngine::MirrorAvailTest(const Position& pos) {
    std::cout << "\n=== Mirror Evaluation Test ===" << std::endl;
    
    // Evaluate original position
    int eval1 = evalPosition(pos);
    std::cout << "Original position eval: " << eval1 << " cp" << std::endl;
    
    // Create mirrored position and evaluate
    Position mirrored = mirrorBoard(pos);
    int eval2 = evalPosition(mirrored);
    std::cout << "Mirrored position eval: " << eval2 << " cp" << std::endl;
    
    // Since mirrorBoard flips the side to move, we need to negate eval2 
    // to compare from the same perspective
    int eval2_corrected = -eval2;
    
    // The evaluations should be equal for symmetric evaluation function
    if (eval1 == eval2_corrected) {
        std::cout << "✓ PASS: Evaluation is symmetric!" << std::endl;
    } else {
        std::cout << "✗ FAIL: Evaluation asymmetry detected!" << std::endl;
        std::cout << "Difference: " << abs(eval1 - eval2_corrected) << " cp" << std::endl;
        std::cout << "This indicates a bug in the evaluation function." << std::endl;
    }
    std::cout << "=========================" << std::endl;
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

std::string MinimalEngine::format_uci_score(int score, Color side_to_move) const {
    // Convert engine score to proper UCI format
    // UCI specification: 
    // - cp <x>: score from engine's point of view in centipawns
    // - mate <y>: mate in y MOVES (not plies). If engine is getting mated, use negative y
    
    // The search returns scores from the side-to-move's perspective:
    // - Positive score means side-to-move is winning
    // - Negative score means side-to-move is losing
    
    // MATE = 29000, so scores close to +/-MATE are mate scores
    if (score > MATE - 100) {
        // Positive mate score: side_to_move (engine) is mating opponent
        int mate_in_plies = MATE - score;
        int mate_in_moves = (mate_in_plies + 1) / 2;  // Convert plies to moves
        return "mate " + std::to_string(mate_in_moves);
    } else if (score < -MATE + 100) {
        // Negative mate score: side_to_move (engine) is being mated
        int mate_in_plies = MATE + score;  // score is negative, so this is MATE - abs(score)
        int mate_in_moves = (mate_in_plies + 1) / 2;  // Convert plies to moves
        return "mate -" + std::to_string(mate_in_moves);  // Negative because engine is being mated
    } else {
        // Regular centipawn score from engine's perspective
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
    
    // Clear counter-moves table
    for (int from_sq = 0; from_sq < 120; ++from_sq) {
        for (int to_sq = 0; to_sq < 120; ++to_sq) {
            counter_moves[from_sq][to_sq] = S_MOVE();
        }
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

// Update counter-move table when move causes beta cutoff
void MinimalEngine::update_counter_move(const S_MOVE& previous_move, const S_MOVE& counter_move) {
    // Validate move parameters
    if (previous_move.move == 0 || counter_move.move == 0) return;
    if (counter_move.is_capture()) return;  // Only store quiet moves as counter-moves
    
    int from_sq = previous_move.get_from();
    int to_sq = previous_move.get_to();
    
    // Validate square indices (must be 0-119 for 120-square representation)
    if (from_sq < 0 || from_sq >= 120 || to_sq < 0 || to_sq >= 120) return;
    
    // Store the counter-move for this [from][to] combination
    counter_moves[from_sq][to_sq] = counter_move;
}

// Get counter-move for the opponent's last move
S_MOVE MinimalEngine::get_counter_move(const S_MOVE& previous_move) const {
    // Validate move parameter
    if (previous_move.move == 0) return S_MOVE();
    
    int from_sq = previous_move.get_from();
    int to_sq = previous_move.get_to();
    
    // Validate square indices
    if (from_sq < 0 || from_sq >= 120 || to_sq < 0 || to_sq >= 120) {
        return S_MOVE();
    }
    
    // Return stored counter-move, or empty move if none stored
    return counter_moves[from_sq][to_sq];
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
int MinimalEngine::pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos, const SearchInfo& info, int depth) const {
    // For the first call (move_num == 0), score all moves using VICE Part 64 ordering
    if (move_num == 0) {
        // VICE Part 84: Check for transposition table move (highest priority)
        int tt_score;
        uint8_t tt_depth, tt_node_type; 
        uint32_t tt_best_move;
        bool has_tt_move = tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
        
        // Get PV move for this position (if any)
        S_MOVE pv_move;
        bool has_pv_move = pv_table.probe_move(pos.zobrist_key, pv_move);
        
        // Score all moves for ordering
        for (int i = 0; i < move_list.count; i++) {
            S_MOVE& move = move_list.moves[i];
            int score = 0;
            
            // VICE Part 84: TT move gets absolute highest priority (3,000,000)
            if (has_tt_move && move.move == static_cast<int>(tt_best_move)) {
                score = 3000000;
                
            // VICE Part 64: PV move gets second highest priority (2,000,000)
            } else if (has_pv_move && move.move == pv_move.move) {
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
                
                // Check for counter-move (if not a killer move)
                bool is_counter_move = false;
                if (!is_killer && info.ply > 0 && info.ply < 64) {
                    // Get the previous move from search stack
                    S_MOVE previous_move = info.search_stack[info.ply - 1];
                    if (previous_move.move != 0) {
                        S_MOVE counter_move = get_counter_move(previous_move);
                        if (counter_move.move == move.move) {
                            score = 700000;  // Counter-move priority: between killers and promotions
                            is_counter_move = true;
                        }
                    }
                }
                
                if (!is_killer && !is_counter_move) {
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
                     << " score " << format_uci_score(best_score, pos.side_to_move)
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
    
    // VICE Part 70: Check for GUI input during search (3:23)
    if (input_is_waiting()) {
        read_input(info);
    }
    
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
    
    // VICE Part 84: Transposition Table Probe
    // Check if we've already searched this position to sufficient depth
    int tt_score;
    uint8_t tt_depth, tt_node_type;
    uint32_t tt_best_move;
    bool tt_hit = tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
    
    if (tt_hit && tt_depth >= depth && !isRoot) {
        // Adjust mate scores to current ply (VICE Part 84: 5:13)
        if (tt_score > MATE - 1000) {
            tt_score -= info.ply;
        } else if (tt_score < -MATE + 1000) {
            tt_score += info.ply;
        }
        
        // Use transposition table score if it provides exact bounds (6:01)
        if (tt_node_type == TTEntry::EXACT) {
            return tt_score;  // Exact score
        } else if (tt_node_type == TTEntry::LOWER_BOUND && tt_score >= beta) {
            return beta;  // Beta cutoff
        } else if (tt_node_type == TTEntry::UPPER_BOUND && tt_score <= alpha) {
            return alpha;  // Alpha cutoff  
        }
    }
    
    // Check for early exit conditions
    if (depth == 0) {
        return quiescence(pos, alpha, beta, info, 0);  // Enter quiescence search at leaf nodes
    }
    
    // VICE Part 76: In check extension (3:01)
    // If the side to move is in check, extend the search depth by 1
    // This helps prevent the engine from getting checkmated by forcing sequences
    bool in_check = false;
    int king_sq = pos.king_sq[int(pos.side_to_move)];
    if (king_sq >= 0) {
        in_check = SqAttacked(king_sq, pos, !pos.side_to_move);
        if (in_check) {
            depth++; // Extend search depth when in check
        }
    }
    
    // Periodically check time and node limits
    if ((info.nodes & 2047) == 0) {  // Check every 2048 nodes
        checkup(info);
    }
    
    if (info.stopped || info.quit) {
        return 0;
    }
    
    // VICE Part 83: Null Move Pruning
    // Only try null move if:
    // 1. We're allowed to do null move (doNull = true)
    // 2. Not in check (zugzwang safety)
    // 3. Not at root level
    // 4. Depth is sufficient (at least 4 for R=3 reduction)
    // 5. Side to move has non-pawn material (big pieces)
    const int NULL_MOVE_REDUCTION = 4;  // R = 4, more aggressive pruning
    const int MIN_NULL_MOVE_DEPTH = 5;  // Minimum depth to try null move (increased for R=4)
    
    if (doNull && !in_check && !isRoot && depth >= MIN_NULL_MOVE_DEPTH && 
        pos.has_non_pawn_material(pos.side_to_move)) {
        
        // DEBUG: Uncomment to see null move attempts
        // std::cout << "Trying null move at depth " << depth << std::endl;
        
        // Make null move (give opponent a free move)
        pos.MakeNullMove();
        
        // Search with reduced depth and narrow window around beta
        int null_score = -AlphaBeta(pos, -beta, -beta + 1, depth - 1 - NULL_MOVE_REDUCTION, info, false, false);
        
        // Undo null move
        pos.TakeNullMove();
        
        // Check if we should stop
        if (info.stopped || info.quit) {
            return 0;
        }
        
        // If null move search shows position is already too good (>= beta), 
        // then our actual moves should easily beat beta - prune this node
        if (null_score >= beta) {
            // Null move cutoff - this position is too good for the opponent
            info.null_cut++; // Track null move cutoffs for statistics
            return beta;
        }
    }

    // Generate all legal moves first
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    // Check for repetition - but only after we ensure we have moves to try
    // This prevents returning draw score at root before storing any PV move (VICE fix)
    if (!isRoot && isRepetition(pos)) {
        return 0; // Draw score
    }    // No legal moves (checkmate or stalemate)
    if (move_list.count == 0) {
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttacked(king_sq, pos, !pos.side_to_move)) {
            // Checkmate: side_to_move is mated
            // Return negative score with distance information
            return -MATE + (info.max_depth - depth); // Negative = loss, distance = plies to mate
        } else {
            return 0; // Stalemate
        }
    }
    
    int best_score = -30000;
    S_MOVE best_move;  // Track best move for transposition table storage
    best_move.move = 0;
    
    // Try each move
    for (int i = 0; i < move_list.count; ++i) {
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, info, depth);
        
        if (pos.MakeMove(move_list.moves[i]) != 1) continue; // Skip illegal moves
        
        // Track move in search stack for counter-move heuristic
        if (info.ply >= 0 && info.ply < 64) {
            info.search_stack[info.ply] = move_list.moves[i];
        }
        
        int score;
        
        // Late Move Reduction (LMR) implementation
        // Reduce depth for moves that are unlikely to be best
        const int LMR_MIN_DEPTH = 3;           // Minimum depth to apply LMR
        const int LMR_FULL_DEPTH_MOVES = 4;   // First N moves searched at full depth
        
        bool needs_full_search = true;
        
        if (depth >= LMR_MIN_DEPTH && i >= LMR_FULL_DEPTH_MOVES && 
            !in_check && !move_list.moves[i].is_capture() && 
            !move_list.moves[i].is_promotion()) {
            
            // Calculate reduction based on depth and move number
            // More aggressive reduction for later moves and deeper searches
            int reduction = 1;
            if (i >= 8 && depth >= 6) {
                reduction = 2;  // More reduction for very late moves at high depth
            }
            
            // Try reduced search first
            int reduced_depth = depth - 1 - reduction;
            if (reduced_depth >= 1) {
                info.lmr_attempts++;  // Track LMR attempt
                score = -AlphaBeta(pos, -alpha - 1, -alpha, reduced_depth, info, true, false);
                
                // If reduced search fails high, we need full search
                if (score > alpha) {
                    info.lmr_failures++;  // Track LMR failure (needs re-search)
                    needs_full_search = true;
                } else {
                    needs_full_search = false;
                }
            }
        }
        
        // Full depth search (either initial search or re-search after LMR fail-high)
        if (needs_full_search) {
            if (i == 0 || alpha == best_score) {
                // First move or no improvement yet - use full window
                score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
            } else {
                // PVS: Try null window first, then re-search if it fails high
                score = -AlphaBeta(pos, -alpha - 1, -alpha, depth - 1, info, true, false);
                if (score > alpha && score < beta) {
                    // Null window search failed high, re-search with full window
                    score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
                }
            }
        }
        
        pos.TakeMove();
        
        if (info.stopped || info.quit) {
            return 0;
        }
        
        if (score > best_score) {
            best_score = score;
            best_move = move_list.moves[i];  // Track best move for TT storage
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
                    
                    // Update counter-move table if we have a previous move
                    if (info.ply > 0 && info.ply < 64) {
                        S_MOVE previous_move = info.search_stack[info.ply - 1];
                        if (previous_move.move != 0) {
                            update_counter_move(previous_move, move_list.moves[i]);
                        }
                    }
                    
                    break;
                }
            }
        }
    }
    
    // VICE Part 84: Store result in transposition table (6:38)
    uint8_t node_type;
    if (best_score <= alpha) {
        node_type = TTEntry::UPPER_BOUND;  // All moves failed low (upper bound)
    } else if (best_score >= beta) {
        node_type = TTEntry::LOWER_BOUND;  // Beta cutoff (lower bound) 
    } else {
        node_type = TTEntry::EXACT;        // Exact score within alpha-beta window
    }
    
    // Adjust mate scores for storage (VICE Part 84: 5:13)
    int store_score = best_score;
    if (store_score > MATE - 1000) {
        store_score += info.ply;
    } else if (store_score < -MATE + 1000) {
        store_score -= info.ply;
    }
    
    tt_table.store(pos.zobrist_key, store_score, depth, node_type, best_move.move);

    return best_score;
}

// Quiescence search to handle horizon effect (4:40)
int MinimalEngine::quiescence(Position& pos, int alpha, int beta, SearchInfo& info, int q_depth) {
    // Quiescence depth limit to prevent stack overflow and improve performance
    const int MAX_QUIESCENCE_DEPTH = 10;
    
    // If we've reached maximum quiescence depth, return stand pat evaluation
    if (q_depth >= MAX_QUIESCENCE_DEPTH) {
        return evalPosition(pos);
    }
    
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
        pick_next_move(move_list, i, pos, info, -1);  // No depth in quiescence
        
        S_MOVE move = move_list.moves[i];
        
        if (pos.MakeMove(move) != 1) continue; // Skip illegal moves
        
        int score = -quiescence(pos, -beta, -alpha, info, q_depth + 1);
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
    
    // VICE Part 85: Check opening book first
    if (opening_book.is_book_loaded() && opening_book.has_book_moves(pos)) {
        S_MOVE book_move = opening_book.get_book_move(pos);
        if (book_move.move != 0) {
            std::cout << "info string Found book move: " << move_to_uci(book_move) << std::endl;
            return book_move;
        }
    }
    
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
            
            // Search this move - isRoot = false for non-root recursive calls
            int score = -AlphaBeta(pos, -30000, 30000, current_depth - 1, info, true, false);
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
                  << " score " << format_uci_score(best_score, pos.side_to_move)
                  << " nodes " << info.nodes 
                  << " time " << elapsed.count()
                  << " nullcut " << info.null_cut
                  << " lmr " << info.lmr_attempts << "/" << info.lmr_failures
                  << " tthits " << tt_table.get_hits()
                  << " ttwrites " << tt_table.get_writes()
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

// VICE Part 84: Print transposition table statistics
void MinimalEngine::print_tt_stats() const {
    uint64_t hits = tt_table.get_hits();
    uint64_t misses = tt_table.get_misses();
    uint64_t writes = tt_table.get_writes();
    uint64_t total_probes = hits + misses;
    double hit_rate = tt_table.get_hit_rate();
    double utilization = tt_table.get_utilization();
    
    std::cout << std::endl;
    std::cout << "=== Transposition Table Statistics ===" << std::endl;
    std::cout << "Table size: " << tt_table.get_size() << " entries" << std::endl;
    std::cout << "Total probes: " << total_probes << std::endl;
    std::cout << "Hits: " << hits << std::endl;
    std::cout << "Misses: " << misses << std::endl;
    std::cout << "Writes: " << writes << std::endl;
    std::cout << "Hit rate: " << std::fixed << std::setprecision(1) << (hit_rate * 100.0) << "%" << std::endl;
    std::cout << "Table utilization: " << std::fixed << std::setprecision(1) << (utilization * 100.0) << "%" << std::endl;
    std::cout << "=======================================" << std::endl;
}

// VICE Part 85: Opening book functions
bool MinimalEngine::load_opening_book(const std::string& book_path) {
    return opening_book.load_book(book_path);
}

S_MOVE MinimalEngine::get_book_move(const Position& pos) const {
    return opening_book.get_book_move(pos);
}

bool MinimalEngine::is_in_opening_book(const Position& pos) const {
    return opening_book.has_book_moves(pos);
}

void MinimalEngine::print_book_moves(const Position& pos) const {
    auto book_moves = opening_book.get_all_book_moves(pos);
    
    if (book_moves.empty()) {
        std::cout << "No book moves available for this position." << std::endl;
        return;
    }
    
    std::cout << "Opening book moves:" << std::endl;
    uint32_t total_weight = 0;
    for (const auto& [move, weight] : book_moves) {
        total_weight += weight;
    }
    
    for (const auto& [move, weight] : book_moves) {
        double percentage = (double(weight) / total_weight) * 100.0;
        std::cout << "  " << move_to_uci(move) 
                  << " (weight: " << weight 
                  << ", " << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
    }
}

} // namespace Huginn
