#include "evaluation.hpp"
#include "movegen_enhanced.hpp"
#include "attack_detection.hpp"
#include "bitboard.hpp"
#include <algorithm>
#include <cmath>

namespace Evaluation {

    namespace PieceSquareTables {
        
        // Pawn piece-square table - encourages central advancement and promotion
        const int PAWN_PST[64] = {
             0,   0,   0,   0,   0,   0,   0,   0,   // Rank 1
             5,  10,  10, -20, -20,  10,  10,   5,   // Rank 2
             5,  -5, -10,   0,   0, -10,  -5,   5,   // Rank 3
             0,   0,   0,  20,  20,   0,   0,   0,   // Rank 4
             5,   5,  10,  25,  25,  10,   5,   5,   // Rank 5
            10,  10,  20,  30,  30,  20,  10,  10,   // Rank 6
            50,  50,  50,  50,  50,  50,  50,  50,   // Rank 7
             0,   0,   0,   0,   0,   0,   0,   0    // Rank 8
        };
        
        // Knight piece-square table - encourages central placement
        const int KNIGHT_PST[64] = {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20,   0,   5,   5,   0, -20, -40,
            -30,   5,  10,  15,  15,  10,   5, -30,
            -30,   0,  15,  20,  20,  15,   0, -30,
            -30,   5,  15,  20,  20,  15,   5, -30,
            -30,   0,  10,  15,  15,  10,   0, -30,
            -40, -20,   0,   0,   0,   0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        };
        
        // Bishop piece-square table - encourages long diagonals
        const int BISHOP_PST[64] = {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10,   5,   0,   0,   0,   0,   5, -10,
            -10,  10,  10,  10,  10,  10,  10, -10,
            -10,   0,  10,  10,  10,  10,   0, -10,
            -10,   5,   5,  10,  10,   5,   5, -10,
            -10,   0,   5,  10,  10,   5,   0, -10,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
        };
        
        // Rook piece-square table - encourages open files and back rank
        const int ROOK_PST[64] = {
             0,   0,   0,   5,   5,   0,   0,   0,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
            -5,   0,   0,   0,   0,   0,   0,  -5,
             5,  10,  10,  10,  10,  10,  10,   5,
             0,   0,   0,   0,   0,   0,   0,   0
        };
        
        // Queen piece-square table - encourages central development
        const int QUEEN_PST[64] = {
            -20, -10, -10,  -5,  -5, -10, -10, -20,
            -10,   0,   5,   0,   0,   0,   0, -10,
            -10,   5,   5,   5,   5,   5,   0, -10,
              0,   0,   5,   5,   5,   5,   0,  -5,
             -5,   0,   5,   5,   5,   5,   0,  -5,
            -10,   0,   5,   5,   5,   5,   0, -10,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -20, -10, -10,  -5,  -5, -10, -10, -20
        };
        
        // King middlegame piece-square table - encourages safety
        const int KING_MG_PST[64] = {
             20,  30,  10,   0,   0,  10,  30,  20,
             20,  20,   0,   0,   0,   0,  20,  20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30
        };
        
        // King endgame piece-square table - encourages activity
        const int KING_EG_PST[64] = {
            -50, -30, -30, -30, -30, -30, -30, -50,
            -30, -30,   0,   0,   0,   0, -30, -30,
            -30, -10,  20,  30,  30,  20, -10, -30,
            -30, -10,  30,  40,  40,  30, -10, -30,
            -30, -10,  30,  40,  40,  30, -10, -30,
            -30, -10,  20,  30,  30,  20, -10, -30,
            -30, -20, -10,   0,   0, -10, -20, -30,
            -50, -40, -30, -20, -20, -30, -40, -50
        };
    }

    int evaluate_material(const Position& pos) {
        int score = 0;
        
        // Use efficient piece count access
        for (int color = 0; color < 2; ++color) {
            int side_score = 0;
            side_score += pos.piece_counts[int(PieceType::Pawn)]   * PAWN_VALUE;
            side_score += pos.piece_counts[int(PieceType::Knight)] * KNIGHT_VALUE;
            side_score += pos.piece_counts[int(PieceType::Bishop)] * BISHOP_VALUE;
            side_score += pos.piece_counts[int(PieceType::Rook)]   * ROOK_VALUE;
            side_score += pos.piece_counts[int(PieceType::Queen)]  * QUEEN_VALUE;
            
            if (color == int(pos.side_to_move)) {
                score += side_score;
            } else {
                score -= side_score;
            }
        }
        
        return score;
    }

    int evaluate_material_quick(const Position& pos) {
        // Fast material evaluation using cached material scores
        int white_material = pos.material_score[int(Color::White)];
        int black_material = pos.material_score[int(Color::Black)];
        
        return (pos.side_to_move == Color::White) ? 
               (white_material - black_material) : 
               (black_material - white_material);
    }

    int evaluate_positional(const Position& pos) {
        int score = 0;
        
        // Evaluate each piece type using piece-square tables
        for (int color = 0; color < 2; ++color) {
            Color c = static_cast<Color>(color);
            int color_score = 0;
            
            // Pawns
            for (int i = 0; i < pos.pCount[color][int(PieceType::Pawn)]; ++i) {
                int sq120 = pos.pList[color][int(PieceType::Pawn)][i];
                int sq64 = MAILBOX_MAPS.to64[sq120];
                if (sq64 >= 0) {
                    int table_index = (c == Color::White) ? sq64 : (63 - sq64);
                    color_score += PieceSquareTables::PAWN_PST[table_index];
                }
            }
            
            // Knights
            for (int i = 0; i < pos.pCount[color][int(PieceType::Knight)]; ++i) {
                int sq120 = pos.pList[color][int(PieceType::Knight)][i];
                int sq64 = MAILBOX_MAPS.to64[sq120];
                if (sq64 >= 0) {
                    int table_index = (c == Color::White) ? sq64 : (63 - sq64);
                    color_score += PieceSquareTables::KNIGHT_PST[table_index];
                }
            }
            
            // Bishops
            for (int i = 0; i < pos.pCount[color][int(PieceType::Bishop)]; ++i) {
                int sq120 = pos.pList[color][int(PieceType::Bishop)][i];
                int sq64 = MAILBOX_MAPS.to64[sq120];
                if (sq64 >= 0) {
                    int table_index = (c == Color::White) ? sq64 : (63 - sq64);
                    color_score += PieceSquareTables::BISHOP_PST[table_index];
                }
            }
            
            // Rooks
            for (int i = 0; i < pos.pCount[color][int(PieceType::Rook)]; ++i) {
                int sq120 = pos.pList[color][int(PieceType::Rook)][i];
                int sq64 = MAILBOX_MAPS.to64[sq120];
                if (sq64 >= 0) {
                    int table_index = (c == Color::White) ? sq64 : (63 - sq64);
                    color_score += PieceSquareTables::ROOK_PST[table_index];
                }
            }
            
            // Queens
            for (int i = 0; i < pos.pCount[color][int(PieceType::Queen)]; ++i) {
                int sq120 = pos.pList[color][int(PieceType::Queen)][i];
                int sq64 = MAILBOX_MAPS.to64[sq120];
                if (sq64 >= 0) {
                    int table_index = (c == Color::White) ? sq64 : (63 - sq64);
                    color_score += PieceSquareTables::QUEEN_PST[table_index];
                }
            }
            
            // King (choose table based on game phase)
            if (pos.king_sq[color] >= 0) {
                int sq64 = MAILBOX_MAPS.to64[pos.king_sq[color]];
                if (sq64 >= 0) {
                    int table_index = (c == Color::White) ? sq64 : (63 - sq64);
                    bool endgame = is_endgame(pos);
                    color_score += endgame ? 
                        PieceSquareTables::KING_EG_PST[table_index] :
                        PieceSquareTables::KING_MG_PST[table_index];
                }
            }
            
            if (c == pos.side_to_move) {
                score += color_score;
            } else {
                score -= color_score;
            }
        }
        
        return score;
    }

    int evaluate_king_safety(const Position& pos, Color color) {
        int safety_score = 0;
        int king_square = pos.king_sq[int(color)];
        
        if (king_square < 0) return -1000; // King missing!
        
        Color enemy = !color;
        
        // Penalty for king in center during middlegame
        if (!is_endgame(pos)) {
            File king_file = file_of(king_square);
            Rank king_rank = rank_of(king_square);
            
            // Penalty for king on central files/ranks
            if (king_file >= File::D && king_file <= File::E) safety_score -= 30;
            if (color == Color::White && king_rank >= Rank::R4) safety_score -= 40;
            if (color == Color::Black && king_rank <= Rank::R5) safety_score -= 40;
        }
        
        // Count enemy attackers around king
        int attackers = 0;
        for (int delta : KING_DELTAS) {
            int adjacent_sq = king_square + delta;
            if (is_playable(adjacent_sq)) {
                if (SqAttacked(adjacent_sq, pos, enemy)) {
                    attackers++;
                }
            }
        }
        
        // Penalty for multiple attackers
        safety_score -= attackers * 15;
        
        // Bonus for castling rights (if still available)
        if (color == Color::White) {
            if (pos.castling_rights & (CASTLE_WK | CASTLE_WQ)) safety_score += 20;
        } else {
            if (pos.castling_rights & (CASTLE_BK | CASTLE_BQ)) safety_score += 20;
        }
        
        return safety_score;
    }

    int evaluate_pawn_structure(const Position& pos) {
        int score = 0;
        
        // Simple pawn structure evaluation
        for (int color = 0; color < 2; ++color) {
            int color_score = 0;
            Color c = static_cast<Color>(color);
            
            // Count pawns per file
            int pawns_per_file[8] = {0};
            for (int i = 0; i < pos.pCount[color][int(PieceType::Pawn)]; ++i) {
                int sq120 = pos.pList[color][int(PieceType::Pawn)][i];
                File file = file_of(sq120);
                if (file != File::None) {
                    pawns_per_file[int(file)]++;
                }
            }
            
            // Penalty for doubled pawns
            for (int file = 0; file < 8; ++file) {
                if (pawns_per_file[file] > 1) {
                    color_score -= (pawns_per_file[file] - 1) * 20;
                }
            }
            
            // Bonus for pawn center control
            color_score += pawns_per_file[int(File::D)] * 10;
            color_score += pawns_per_file[int(File::E)] * 10;
            
            if (c == pos.side_to_move) {
                score += color_score;
            } else {
                score -= color_score;
            }
        }
        
        return score;
    }

    bool is_endgame(const Position& pos) {
        // Simple endgame detection: few pieces remaining
        int total_pieces = 0;
        for (int type = int(PieceType::Pawn); type <= int(PieceType::Queen); ++type) {
            total_pieces += pos.piece_counts[type];
        }
        
        // Endgame if less than 12 pieces total (excluding kings)
        return total_pieces < 12;
    }

    bool is_checkmate(const Position& pos) {
        // Check if king is in check
        int king_square = pos.king_sq[int(pos.side_to_move)];
        if (king_square < 0) return false;
        
        bool in_check = SqAttacked(king_square, pos, !pos.side_to_move);
        if (!in_check) return false;
        
        // Generate all legal moves - if none, it's checkmate
        Position temp_pos = pos;
        S_MOVELIST moves;
        generate_legal_moves_enhanced(temp_pos, moves);
        
        return moves.count == 0;
    }

    bool is_stalemate(const Position& pos) {
        // Check if king is NOT in check
        int king_square = pos.king_sq[int(pos.side_to_move)];
        if (king_square < 0) return false;
        
        bool in_check = SqAttacked(king_square, pos, !pos.side_to_move);
        if (in_check) return false;
        
        // Generate all legal moves - if none, it's stalemate
        Position temp_pos = pos;
        S_MOVELIST moves;
        generate_legal_moves_enhanced(temp_pos, moves);
        
        return moves.count == 0;
    }

    bool is_insufficient_material(const Position& pos) {
        // Check for basic insufficient material draws
        int white_pieces = 0, black_pieces = 0;
        bool white_has_major = false, black_has_major = false;
        
        for (int type = int(PieceType::Pawn); type <= int(PieceType::Queen); ++type) {
            int white_count = pos.pCount[int(Color::White)][type];
            int black_count = pos.pCount[int(Color::Black)][type];
            
            white_pieces += white_count;
            black_pieces += black_count;
            
            if (type >= int(PieceType::Rook)) { // Rook or Queen
                if (white_count > 0) white_has_major = true;
                if (black_count > 0) black_has_major = true;
            }
        }
        
        // K vs K
        if (white_pieces == 0 && black_pieces == 0) return true;
        
        // K+minor vs K
        if ((white_pieces == 1 && black_pieces == 0 && !white_has_major) ||
            (black_pieces == 1 && white_pieces == 0 && !black_has_major)) return true;
        
        // K+minor vs K+minor
        if (white_pieces == 1 && black_pieces == 1 && !white_has_major && !black_has_major) {
            // Both sides only have one minor piece
            return true;
        }
        
        return false;
    }

    int evaluate_position(const Position& pos) {
        // Check for special positions first
        if (is_checkmate(pos)) {
            return -CHECKMATE_SCORE;
        }
        
        if (is_stalemate(pos) || is_insufficient_material(pos)) {
            return STALEMATE_SCORE;
        }
        
        int score = 0;
        
        // Material evaluation (most important)
        score += evaluate_material_quick(pos);
        
        // Positional evaluation
        score += evaluate_positional(pos);
        
        // King safety
        score += evaluate_king_safety(pos, pos.side_to_move);
        score -= evaluate_king_safety(pos, !pos.side_to_move);
        
        // Pawn structure
        score += evaluate_pawn_structure(pos);
        
        return score;
    }

} // namespace Evaluation
