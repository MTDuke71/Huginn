#include "evaluation.hpp"
#include "attack_detection.hpp"
#include "movegen_enhanced.hpp"
#include <algorithm>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

namespace Huginn {

// Convert square120 to square64 format
int HybridEvaluator::square120_to_64(int sq120) {
    if (sq120 < 21 || sq120 > 98) return -1; // Off board
    
    int file = (sq120 % 10) - 1;
    int rank = (sq120 / 10) - 2;
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    
    return rank * 8 + file;
}

// Convert square64 to square120 format  
int HybridEvaluator::square64_to_120(int sq64) {
    if (sq64 < 0 || sq64 > 63) return -1;
    
    int file = sq64 % 8;
    int rank = sq64 / 8;
    
    return (rank + 2) * 10 + (file + 1);
}

// Mirror square for black pieces (flip vertically)
int HybridEvaluator::mirror_square(int sq64) {
    if (sq64 < 0 || sq64 > 63) return sq64;
    return ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
}

// Determine game phase based on piece count
GamePhase HybridEvaluator::get_game_phase(const Position& pos) {
    int piece_count = 0;
    
    // Count all pieces except kings
    for (int sq = 0; sq < 120; ++sq) {
        Piece piece = pos.board[sq];
        if (piece != Piece::None && piece != Piece::WhiteKing && piece != Piece::BlackKing) {
            piece_count++;
        }
    }
    
    if (piece_count > EvalParams::GAME_PHASE_OPENING_THRESHOLD)
        return GamePhase::Opening;
    if (piece_count > EvalParams::GAME_PHASE_MIDDLEGAME_THRESHOLD)
        return GamePhase::Middlegame;
    return GamePhase::Endgame;
}

// Check if pawn is passed
bool HybridEvaluator::is_passed_pawn(const Position& pos, int sq120, Color color) {
    int file = (sq120 % 10) - 1;
    int rank = (sq120 / 10) - 2;
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return false;
    
    // Check squares in front of the pawn and adjacent files
    int start_file = std::max(0, file - 1);
    int end_file = std::min(7, file + 1);
    
    if (color == Color::White) {
        // Check ranks ahead for black pawns
        for (int r = rank + 1; r <= 7; ++r) {
            for (int f = start_file; f <= end_file; ++f) {
                int check_sq = (r + 2) * 10 + (f + 1);
                if (pos.board[check_sq] == Piece::BlackPawn) {
                    return false;
                }
            }
        }
    } else {
        // Check ranks behind for white pawns  
        for (int r = rank - 1; r >= 0; --r) {
            for (int f = start_file; f <= end_file; ++f) {
                int check_sq = (r + 2) * 10 + (f + 1);
                if (pos.board[check_sq] == Piece::WhitePawn) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

// Check if pawn is isolated (no friendly pawns on adjacent files)
bool HybridEvaluator::is_isolated_pawn(const Position& pos, int sq120, Color color) {
    int file = (sq120 % 10) - 1;
    
    if (file < 0 || file > 7) return false;
    
    Piece friendly_pawn = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;
    
    // Check left file
    if (file > 0) {
        for (int rank = 0; rank <= 7; ++rank) {
            int check_sq = (rank + 2) * 10 + file; // file-1 + 1
            if (pos.board[check_sq] == friendly_pawn) {
                return false;
            }
        }
    }
    
    // Check right file
    if (file < 7) {
        for (int rank = 0; rank <= 7; ++rank) {
            int check_sq = (rank + 2) * 10 + (file + 2); // file+1 + 1
            if (pos.board[check_sq] == friendly_pawn) {
                return false;
            }
        }
    }
    
    return true;
}

// Check if there are doubled pawns on this file
bool HybridEvaluator::is_doubled_pawn(const Position& pos, int sq120, Color color) {
    int file = (sq120 % 10) - 1;
    int rank = (sq120 / 10) - 2;
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return false;
    
    Piece friendly_pawn = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;
    int count = 0;
    
    // Count pawns on this file
    for (int r = 0; r <= 7; ++r) {
        int check_sq = (r + 2) * 10 + (file + 1);
        if (pos.board[check_sq] == friendly_pawn) {
            count++;
        }
    }
    
    return count > 1;
}

// Check if knight is on an outpost
bool HybridEvaluator::is_knight_outpost(const Position& pos, int sq120, Color color) {
    int file = (sq120 % 10) - 1;
    int rank = (sq120 / 10) - 2;
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return false;
    
    if (color == Color::White) {
        if (rank < EvalParams::WHITE_KNIGHT_OUTPOST_MIN_RANK) return false;
        
        // Check if black pawns can attack this square
        if (file > 0) {
            int attack_sq = (rank + 1 + 2) * 10 + file; // rank+1, file-1 in 120 format
            if (pos.board[attack_sq] == Piece::BlackPawn) return false;
        }
        if (file < 7) {
            int attack_sq = (rank + 1 + 2) * 10 + (file + 2); // rank+1, file+1 in 120 format  
            if (pos.board[attack_sq] == Piece::BlackPawn) return false;
        }
        
        // Check if white pawns support this square
        bool supported = false;
        if (file > 0) {
            int support_sq = (rank - 1 + 2) * 10 + file; // rank-1, file-1 in 120 format
            if (pos.board[support_sq] == Piece::WhitePawn) supported = true;
        }
        if (file < 7) {
            int support_sq = (rank - 1 + 2) * 10 + (file + 2); // rank-1, file+1 in 120 format
            if (pos.board[support_sq] == Piece::WhitePawn) supported = true;
        }
        
        return supported;
    } else {
        if (rank > EvalParams::BLACK_KNIGHT_OUTPOST_MAX_RANK) return false;
        
        // Check if white pawns can attack this square
        if (file > 0) {
            int attack_sq = (rank - 1 + 2) * 10 + file; // rank-1, file-1 in 120 format
            if (pos.board[attack_sq] == Piece::WhitePawn) return false;
        }
        if (file < 7) {
            int attack_sq = (rank - 1 + 2) * 10 + (file + 2); // rank-1, file+1 in 120 format
            if (pos.board[attack_sq] == Piece::WhitePawn) return false;
        }
        
        // Check if black pawns support this square
        bool supported = false;
        if (file > 0) {
            int support_sq = (rank + 1 + 2) * 10 + file; // rank+1, file-1 in 120 format
            if (pos.board[support_sq] == Piece::BlackPawn) supported = true;
        }
        if (file < 7) {
            int support_sq = (rank + 1 + 2) * 10 + (file + 2); // rank+1, file+1 in 120 format
            if (pos.board[support_sq] == Piece::BlackPawn) supported = true;
        }
        
        return supported;
    }
}

// Check if file is open (no pawns)
bool HybridEvaluator::is_open_file(const Position& pos, int file) {
    if (file < 0 || file > 7) return false;
    
    for (int rank = 0; rank <= 7; ++rank) {
        int sq120 = (rank + 2) * 10 + (file + 1);
        if (pos.board[sq120] == Piece::WhitePawn || pos.board[sq120] == Piece::BlackPawn) {
            return false;
        }
    }
    
    return true;
}

// Count developed pieces (not on starting squares)
int HybridEvaluator::count_developed_pieces(const Position& pos, Color color) {
    int developed = 0;
    
    if (color == Color::White) {
        // Knights: b1(22), g1(27)
        if (pos.board[22] != Piece::WhiteKnight) developed++;
        if (pos.board[27] != Piece::WhiteKnight) developed++;
        
        // Bishops: c1(23), f1(26) 
        if (pos.board[23] != Piece::WhiteBishop) developed++;
        if (pos.board[26] != Piece::WhiteBishop) developed++;
    } else {
        // Knights: b8(92), g8(97)
        if (pos.board[92] != Piece::BlackKnight) developed++;
        if (pos.board[97] != Piece::BlackKnight) developed++;
        
        // Bishops: c8(93), f8(96)
        if (pos.board[93] != Piece::BlackBishop) developed++;
        if (pos.board[96] != Piece::BlackBishop) developed++;
    }
    
    return developed;
}

// Evaluate material and piece-square tables
int HybridEvaluator::evaluate_material(const Position& pos, GamePhase phase) {
    int score = 0;
    
    const std::array<int, 64>* king_table = (phase == GamePhase::Endgame) ? 
        &EvalParams::KING_TABLE_ENDGAME : &EvalParams::KING_TABLE;
    
    for (int sq120 = 0; sq120 < 120; ++sq120) {
        Piece piece = pos.board[sq120];
        if (piece == Piece::None) continue;
        
        int sq64 = square120_to_64(sq120);
        if (sq64 < 0) continue;
        
        int piece_value = 0;
        int table_value = 0;
        // bool is_white = static_cast<int>(piece) < 6; // TODO: Use for color-dependent evaluation
        
        switch (piece) {
            case Piece::WhitePawn:
                piece_value = EvalParams::PAWN_VALUE;
                table_value = EvalParams::PAWN_TABLE[sq64];
                break;
            case Piece::BlackPawn:
                piece_value = -EvalParams::PAWN_VALUE;
                table_value = -EvalParams::PAWN_TABLE[mirror_square(sq64)];
                break;
            case Piece::WhiteKnight:
                piece_value = EvalParams::KNIGHT_VALUE;
                table_value = EvalParams::KNIGHT_TABLE[sq64];
                break;
            case Piece::BlackKnight:
                piece_value = -EvalParams::KNIGHT_VALUE;
                table_value = -EvalParams::KNIGHT_TABLE[mirror_square(sq64)];
                break;
            case Piece::WhiteBishop:
                piece_value = EvalParams::BISHOP_VALUE;
                table_value = EvalParams::BISHOP_TABLE[sq64];
                break;
            case Piece::BlackBishop:
                piece_value = -EvalParams::BISHOP_VALUE;
                table_value = -EvalParams::BISHOP_TABLE[mirror_square(sq64)];
                break;
            case Piece::WhiteRook:
                piece_value = EvalParams::ROOK_VALUE;
                table_value = EvalParams::ROOK_TABLE[sq64];
                break;
            case Piece::BlackRook:
                piece_value = -EvalParams::ROOK_VALUE;
                table_value = -EvalParams::ROOK_TABLE[mirror_square(sq64)];
                break;
            case Piece::WhiteQueen:
                piece_value = EvalParams::QUEEN_VALUE;
                table_value = EvalParams::QUEEN_TABLE[sq64];
                break;
            case Piece::BlackQueen:
                piece_value = -EvalParams::QUEEN_VALUE;
                table_value = -EvalParams::QUEEN_TABLE[mirror_square(sq64)];
                break;
            case Piece::WhiteKing:
                piece_value = EvalParams::KING_VALUE;
                table_value = (*king_table)[sq64];
                break;
            case Piece::BlackKing:
                piece_value = -EvalParams::KING_VALUE;
                table_value = -(*king_table)[mirror_square(sq64)];
                break;
            default:
                break;
        }
        
        score += piece_value + table_value;
    }
    
    return score;
}

// Evaluate pawn structure
int HybridEvaluator::evaluate_pawn_structure(const Position& pos) {
    int score = 0;
    int white_file_counts[8] = {0};
    int black_file_counts[8] = {0};
    
    // First pass: count pawns per file and evaluate individual pawns
    for (int sq120 = 0; sq120 < 120; ++sq120) {
        Piece piece = pos.board[sq120];
        if (piece != Piece::WhitePawn && piece != Piece::BlackPawn) continue;
        
        int sq64 = square120_to_64(sq120);
        if (sq64 < 0) continue;
        
        int file = sq64 % 8;
        int rank = sq64 / 8;
        
        if (piece == Piece::WhitePawn) {
            white_file_counts[file]++;
            
            // Passed pawn bonus
            if (is_passed_pawn(pos, sq120, Color::White)) {
                score += EvalParams::PASSED_PAWN_BONUS[rank];
            }
            
            // Penalty for premature flank pawn moves in opening (a6, h6, a5, h5 for black)
            if (get_game_phase(pos) == GamePhase::Opening) {
                if ((file == 0 || file == 7) && rank >= 4) { // Advanced flank pawns
                    score -= 15; // Penalty for premature flank advances
                }
            }
        } else {
            black_file_counts[file]++;
            
            // Passed pawn bonus  
            if (is_passed_pawn(pos, sq120, Color::Black)) {
                score -= EvalParams::PASSED_PAWN_BONUS[7 - rank];
            }
            
            // Penalty for premature flank pawn moves in opening
            if (get_game_phase(pos) == GamePhase::Opening) {
                if ((file == 0 || file == 7) && rank <= 3) { // Advanced flank pawns for black
                    score += 15; // Penalty for premature flank advances
                }
            }
        }
    }
    
    // Second pass: evaluate pawn structure penalties
    for (int file = 0; file < 8; ++file) {
        // Doubled pawns
        if (white_file_counts[file] > 1) {
            score -= EvalParams::DOUBLED_PAWN_PENALTY * (white_file_counts[file] - 1);
        }
        if (black_file_counts[file] > 1) {
            score += EvalParams::DOUBLED_PAWN_PENALTY * (black_file_counts[file] - 1);
        }
        
        // Isolated pawns
        if (white_file_counts[file] > 0) {
            bool has_support = false;
            if (file > 0 && white_file_counts[file - 1] > 0) has_support = true;
            if (file < 7 && white_file_counts[file + 1] > 0) has_support = true;
            
            if (!has_support) {
                score -= EvalParams::ISOLATED_PAWN_PENALTY * white_file_counts[file];
            }
        }
        
        if (black_file_counts[file] > 0) {
            bool has_support = false;
            if (file > 0 && black_file_counts[file - 1] > 0) has_support = true;
            if (file < 7 && black_file_counts[file + 1] > 0) has_support = true;
            
            if (!has_support) {
                score += EvalParams::ISOLATED_PAWN_PENALTY * black_file_counts[file];
            }
        }
    }
    
    return score;
}

// Evaluate piece activity (outposts, open files, etc.)
int HybridEvaluator::evaluate_piece_activity(const Position& pos, GamePhase phase) {
    (void)phase; // TODO: Use phase for piece activity evaluation
    int score = 0;
    int white_bishops = 0, black_bishops = 0;
    
    for (int sq120 = 0; sq120 < 120; ++sq120) {
        Piece piece = pos.board[sq120];
        if (piece == Piece::None) continue;
        
        int sq64 = square120_to_64(sq120);
        if (sq64 < 0) continue;
        
        int file = sq64 % 8;
        
        switch (piece) {
            case Piece::WhiteKnight:
                if (is_knight_outpost(pos, sq120, Color::White)) {
                    score += EvalParams::KNIGHT_OUTPOST_BONUS;
                }
                break;
            case Piece::BlackKnight:
                if (is_knight_outpost(pos, sq120, Color::Black)) {
                    score -= EvalParams::KNIGHT_OUTPOST_BONUS;
                }
                break;
            case Piece::WhiteBishop:
                white_bishops++;
                break;
            case Piece::BlackBishop:
                black_bishops++;
                break;
            case Piece::WhiteRook:
                if (is_open_file(pos, file)) {
                    score += EvalParams::ROOK_OPEN_FILE_BONUS;
                }
                break;
            case Piece::BlackRook:
                if (is_open_file(pos, file)) {
                    score -= EvalParams::ROOK_OPEN_FILE_BONUS;
                }
                break;
            default:
                break;
        }
    }
    
    // Bishop pair bonus
    if (white_bishops >= 2) score += EvalParams::BISHOP_PAIR_BONUS;
    if (black_bishops >= 2) score -= EvalParams::BISHOP_PAIR_BONUS;
    
    return score;
}

// Evaluate mobility (simplified version using legal move count)
int HybridEvaluator::evaluate_mobility(const Position& pos, GamePhase phase) {
    int mobility_weight = (phase == GamePhase::Endgame) ? 
        EvalParams::MOBILITY_WEIGHT_ENDGAME : EvalParams::MOBILITY_WEIGHT_DEFAULT;
    
    // Count legal moves for current side
    Position temp_pos = pos;  // Make a copy
    S_MOVELIST moves;
    generate_legal_moves_enhanced(temp_pos, moves);
    int current_mobility = moves.count;
    
    // Count legal moves for opponent (make a null move)
    temp_pos.side_to_move = !temp_pos.side_to_move;
    generate_legal_moves_enhanced(temp_pos, moves);
    int opponent_mobility = moves.count;
    
    int mobility_diff = current_mobility - opponent_mobility;
    return mobility_weight * (pos.side_to_move == Color::White ? mobility_diff : -mobility_diff);
}

// Evaluate development
int HybridEvaluator::evaluate_development(const Position& pos, GamePhase phase) {
    if (phase == GamePhase::Endgame) return 0; // Development doesn't matter in endgame
    
    int develop_bonus = (phase == GamePhase::Opening) ? 
        EvalParams::DEVELOP_BONUS_OPENING : EvalParams::DEVELOP_BONUS_DEFAULT;
    
    int white_developed = count_developed_pieces(pos, Color::White);
    int black_developed = count_developed_pieces(pos, Color::Black);
    
    return develop_bonus * (white_developed - black_developed);
}

// Evaluate king safety (simplified version)
int HybridEvaluator::evaluate_king_safety(const Position& pos, GamePhase phase) {
    if (phase == GamePhase::Endgame) return 0; // King safety less important in endgame
    
    int score = 0;
    
    // Evaluate white king safety
    int white_king_sq = pos.king_sq[static_cast<int>(Color::White)];
    if (white_king_sq >= 0) {
        // Check for castling
        bool white_castled = (white_king_sq == 27) || (white_king_sq == 23); // g1 or c1 in 120 format
        bool white_home = (white_king_sq == 25); // e1 in 120 format
        
        if (white_castled) {
            score += EvalParams::CASTLE_BONUS;
        } else if (white_home && !(pos.castling_rights & CASTLE_WK) && !(pos.castling_rights & CASTLE_WQ)) {
            score -= EvalParams::STUCK_PENALTY;
        }
    }
    
    // Evaluate black king safety
    int black_king_sq = pos.king_sq[static_cast<int>(Color::Black)];
    if (black_king_sq >= 0) {
        // Check for castling
        bool black_castled = (black_king_sq == 97) || (black_king_sq == 93); // g8 or c8 in 120 format
        bool black_home = (black_king_sq == 95); // e8 in 120 format
        
        if (black_castled) {
            score -= EvalParams::CASTLE_BONUS;
        } else if (black_home && !(pos.castling_rights & CASTLE_BK) && !(pos.castling_rights & CASTLE_BQ)) {
            score += EvalParams::STUCK_PENALTY;
        }
    }
    
    return score;
}

// Main evaluation function
int HybridEvaluator::evaluate(const Position& pos) {
    // Determine game phase
    GamePhase phase = get_game_phase(pos);
    
    int total_score = 0;
    
    // Material evaluation (most important)
    total_score += evaluate_material(pos, phase);
    
    // Pawn structure evaluation
    total_score += evaluate_pawn_structure(pos);
    
    // Piece activity and positioning
    total_score += evaluate_piece_activity(pos, phase);
    
    // King safety evaluation (more important in middlegame)
    if (phase != GamePhase::Endgame) {
        total_score += evaluate_king_safety(pos, phase);
    }
    
    // Development evaluation (important in opening and early middlegame)
    if (phase == GamePhase::Opening || phase == GamePhase::Middlegame) {
        total_score += evaluate_development(pos, phase);
    }
    
    // Mobility evaluation
    total_score += evaluate_mobility(pos, phase);
    
    // Return from side to move perspective
    return pos.side_to_move == Color::White ? total_score : -total_score;
}

} // namespace Huginn
