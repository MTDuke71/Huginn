#include "evaluation.hpp"
#include "movegen_enhanced.hpp"
#include "attack_detection.hpp"
#include "bitboard.hpp"
#include "search.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>

namespace Evaluation {

    namespace PieceSquareTables {
        
        // Pawn piece-square table - encourages central advancement and promotion
        const int PAWN_PST[64] = {
             0,   0,   0,   0,   0,   0,   0,   0,   // Rank 1
             5,  10,  20,  30,  30,  20,  10,   5,   // Rank 2: Very strong central pawn encouragement
             5,  -5, -10,   0,   0, -10,  -5,   5,   // Rank 3
             0,   0,   0,  45,  45,   0,   0,   0,   // Rank 4: Massive bonus for central control
             5,   5,  10,  50,  50,  10,   5,   5,   // Rank 5
            10,  10,  20,  55,  55,  20,  10,  10,   // Rank 6
            50,  50,  50,  50,  50,  50,  50,  50,   // Rank 7
             0,   0,   0,   0,   0,   0,   0,   0    // Rank 8
        };
        
        // Knight piece-square table - heavily penalizes rim squares ("Knights on the rim are dim")
        const int KNIGHT_PST[64] = {
            -80, -60, -40, -30, -30, -40, -60, -80,  // Rank 1: Harsh rim penalties
            -60, -20,   0,   5,   5,   0, -20, -60,  // Rank 2: Rim still bad
            -40,   5,  10,  15,  15,  10,   5, -40,  // Rank 3: Rim penalties
            -30,   0,  15,  20,  20,  15,   0, -30,  // Rank 4: Slight rim penalty
            -30,   5,  15,  20,  20,  15,   5, -30,  // Rank 5: Slight rim penalty
            -40,   0,  10,  15,  15,  10,   0, -40,  // Rank 6: Rim penalties
            -60, -20,   0,   0,   0,   0, -20, -60,  // Rank 7: Rim still bad
            -80, -60, -40, -30, -30, -40, -60, -80   // Rank 8: Harsh rim penalties
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
        File king_file = file_of(king_square);
        Rank king_rank = rank_of(king_square);
        
        // MASSIVE PENALTY for king in center during opening/middlegame
        // But in endgames, king activity is desirable, so skip these penalties
        bool early_game = pos.fullmove_number <= 15; // Extended to cover more of opening
        bool endgame = is_endgame(pos);
        
        if (early_game && !endgame) {
            // Kings should NOT be in the center files (d, e) or advanced ranks
            if (color == Color::White) {
                if (king_rank >= Rank::R3 && king_rank <= Rank::R6) {
                    safety_score -= 800; // Massive penalty for king in center ranks
                }
                if (king_file >= File::D && king_file <= File::E) {
                    safety_score -= 600; // Huge penalty for king in center files
                }
                // Extra penalty for king far from back rank
                if (king_rank >= Rank::R4) {
                    safety_score -= 1000; // Nearly mate-level penalty
                }
            } else { // Black
                if (king_rank >= Rank::R3 && king_rank <= Rank::R6) {
                    safety_score -= 800; // Massive penalty for king in center ranks  
                }
                if (king_file >= File::D && king_file <= File::E) {
                    safety_score -= 600; // Huge penalty for king in center files
                }
                // Extra penalty for king far from back rank
                if (king_rank <= Rank::R5) {
                    safety_score -= 1000; // Nearly mate-level penalty for forward kings
                }
            }
        }
        
        // CRITICAL: Check for broken pawn shelter (g5/g4/f6 pattern) - THE MAIN FIX
        // But only apply these penalties in opening/middlegame, not endgame
        if (!endgame) {
            if (color == Color::Black) {
                // Check for the catastrophic g5, g4, f6 pattern
                Piece g7_pawn = pos.at(sq(File::G, Rank::R7));
                Piece f7_pawn = pos.at(sq(File::F, Rank::R7));
                
                // Severe penalty if g-pawn moved from g7
                if (is_none(g7_pawn) || color_of(g7_pawn) != color) {
                    safety_score -= 300; // Massive penalty for missing g7 pawn
                    
                    // Even worse if g-pawn is on g5 or g4 
                    Piece g5_piece = pos.at(sq(File::G, Rank::R5));
                    Piece g4_piece = pos.at(sq(File::G, Rank::R4));
                    if (!is_none(g5_piece) && color_of(g5_piece) == color && type_of(g5_piece) == PieceType::Pawn) {
                        safety_score -= 200; // g5 is terrible
                    }
                    if (!is_none(g4_piece) && color_of(g4_piece) == color && type_of(g4_piece) == PieceType::Pawn) {
                        safety_score -= 400; // g4 is catastrophic
                    }
                }
                
                // Severe penalty if f-pawn moved from f7 (especially f6)
                if (is_none(f7_pawn) || color_of(f7_pawn) != color) {
                    safety_score -= 400; // Major penalty for missing f7 pawn (increased)
                    
                    Piece f6_piece = pos.at(sq(File::F, Rank::R6));
                    if (!is_none(f6_piece) && color_of(f6_piece) == color && type_of(f6_piece) == PieceType::Pawn) {
                        // f6 is absolutely catastrophic - massive penalty
                        if (early_game) {
                            safety_score -= 1200; // HUGE penalty in opening - nearly losing 
                        } else {
                            safety_score -= 800; // Still terrible in middlegame
                        }
                    }
                }
            }
            
            // Similar checks for White
            if (color == Color::White) {
                Piece g2_pawn = pos.at(sq(File::G, Rank::R2));
                Piece f2_pawn = pos.at(sq(File::F, Rank::R2));
                
                if (is_none(g2_pawn) || color_of(g2_pawn) != color) {
                    safety_score -= 300;
                    
                    Piece g4_piece = pos.at(sq(File::G, Rank::R4));
                    Piece g5_piece = pos.at(sq(File::G, Rank::R5));
                    if (!is_none(g4_piece) && color_of(g4_piece) == color && type_of(g4_piece) == PieceType::Pawn) {
                        safety_score -= 200;
                    }
                    if (!is_none(g5_piece) && color_of(g5_piece) == color && type_of(g5_piece) == PieceType::Pawn) {
                        safety_score -= 400;
                    }
                }
                
                if (is_none(f2_pawn) || color_of(f2_pawn) != color) {
                    safety_score -= 250;
                    
                    Piece f3_piece = pos.at(sq(File::F, Rank::R3));
                    if (!is_none(f3_piece) && color_of(f3_piece) == color && type_of(f3_piece) == PieceType::Pawn) {
                        safety_score -= 500;
                    }
                }
            }
        }
        
        // Enhanced penalty for king in center during middlegame
        if (!is_endgame(pos)) {
            if (king_file >= File::D && king_file <= File::E) safety_score -= 100; // Increased from 30
            if (color == Color::White && king_rank >= Rank::R4) safety_score -= 150; // Increased from 40
            if (color == Color::Black && king_rank <= Rank::R5) safety_score -= 150; // Increased from 40
        }
        
        // Count enemy attackers around king with heavier penalties
        int attackers = 0;
        for (int delta : KING_DELTAS) {
            int adjacent_sq = king_square + delta;
            if (is_playable(adjacent_sq)) {
                if (SqAttacked(adjacent_sq, pos, enemy)) {
                    attackers++;
                }
            }
        }
        
        // Much stronger penalty for multiple attackers
        safety_score -= attackers * 50; // Increased from 15
        
        // CRITICAL: Check for immediate mate threats (queen attacks)
        if (SqAttacked(king_square, pos, enemy)) {
            safety_score -= 200; // King is in check - very dangerous
        }
        
        // Bonus for castling rights (if still available)
        if (color == Color::White) {
            if (pos.castling_rights & (CASTLE_WK | CASTLE_WQ)) safety_score += 50; // Increased from 20
        } else {
            if (pos.castling_rights & (CASTLE_BK | CASTLE_BQ)) safety_score += 50; // Increased from 20
        }
        
        return safety_score;
    }

    int evaluate_pawn_structure(const Position& pos) {
        int score = 0;
        
        // Get pawn bitboards
        uint64_t white_pawns = pos.get_pawn_bitboard(Color::White);
        uint64_t black_pawns = pos.get_pawn_bitboard(Color::Black);
        uint64_t all_pawns = pos.get_all_pawns_bitboard();
        
        // Pawn structure penalties and bonuses
        constexpr int ISOLATED_PAWN_PENALTY = 20;
        constexpr int DOUBLED_PAWN_PENALTY = 15;
        constexpr int BACKWARD_PAWN_PENALTY = 12;
        constexpr int PASSED_PAWN_BONUS[8] = {0, 10, 15, 25, 40, 70, 120, 0}; // By rank
        constexpr int PAWN_CHAIN_BONUS = 8;
        constexpr int CONNECTED_PAWNS_BONUS = 5;
        constexpr int WEAK_PAWN_PENALTY = 10;
        
        // Analyze each file for pawn structure
        for (int file = 0; file < 8; ++file) {
            uint64_t file_mask = FILE_BB[file];
            uint64_t white_pawns_on_file = white_pawns & file_mask;
            uint64_t black_pawns_on_file = black_pawns & file_mask;
            
            int white_pawns_count = CNT(white_pawns_on_file);
            int black_pawns_count = CNT(black_pawns_on_file);
            
            // DOUBLED PAWNS PENALTY
            if (white_pawns_count > 1) {
                score -= DOUBLED_PAWN_PENALTY * (white_pawns_count - 1);
            }
            if (black_pawns_count > 1) {
                score += DOUBLED_PAWN_PENALTY * (black_pawns_count - 1);
            }
            
            // ISOLATED PAWNS PENALTY
            if (white_pawns_count > 0) {
                // Check if there are white pawns on adjacent files
                bool has_support = false;
                if (file > 0 && (white_pawns & FILE_BB[file - 1])) has_support = true;
                if (file < 7 && (white_pawns & FILE_BB[file + 1])) has_support = true;
                
                if (!has_support) {
                    score -= ISOLATED_PAWN_PENALTY * white_pawns_count;
                }
            }
            
            if (black_pawns_count > 0) {
                // Check if there are black pawns on adjacent files
                bool has_support = false;
                if (file > 0 && (black_pawns & FILE_BB[file - 1])) has_support = true;
                if (file < 7 && (black_pawns & FILE_BB[file + 1])) has_support = true;
                
                if (!has_support) {
                    score += ISOLATED_PAWN_PENALTY * black_pawns_count;
                }
            }
            
            // PASSED PAWNS ANALYSIS
            if (white_pawns_count == 1 && black_pawns_count == 0) {
                // Potential white passed pawn - check if path is clear
                uint64_t temp_pawns = white_pawns_on_file;
                int pawn_sq = POP(temp_pawns);
                
                int rank = pawn_sq / 8;
                bool is_passed = true;
                
                // Check if any black pawns can stop this pawn
                for (int check_file = std::max(0, file - 1); check_file <= std::min(7, file + 1); ++check_file) {
                    uint64_t check_file_mask = FILE_BB[check_file];
                    uint64_t blocking_pawns = black_pawns & check_file_mask;
                    
                    uint64_t temp_blocking = blocking_pawns;
                    while (temp_blocking) {
                        int blocker_sq = POP(temp_blocking);
                        int blocker_rank = blocker_sq / 8;
                        
                        if (blocker_rank > rank) { // Black pawn ahead of white pawn
                            is_passed = false;
                            break;
                        }
                    }
                    if (!is_passed) break;
                }
                
                if (is_passed) {
                    score += PASSED_PAWN_BONUS[rank];
                }
            }
            
            if (black_pawns_count == 1 && white_pawns_count == 0) {
                // Potential black passed pawn - check if path is clear
                uint64_t temp_pawns = black_pawns_on_file;
                int pawn_sq = POP(temp_pawns);
                
                int rank = pawn_sq / 8;
                bool is_passed = true;
                
                // Check if any white pawns can stop this pawn
                for (int check_file = std::max(0, file - 1); check_file <= std::min(7, file + 1); ++check_file) {
                    uint64_t check_file_mask = FILE_BB[check_file];
                    uint64_t blocking_pawns = white_pawns & check_file_mask;
                    
                    uint64_t temp_blocking = blocking_pawns;
                    while (temp_blocking) {
                        int blocker_sq = POP(temp_blocking);
                        int blocker_rank = blocker_sq / 8;
                        
                        if (blocker_rank < rank) { // White pawn ahead of black pawn (from black's perspective)
                            is_passed = false;
                            break;
                        }
                    }
                    if (!is_passed) break;
                }
                
                if (is_passed) {
                    score -= PASSED_PAWN_BONUS[7 - rank]; // Flip rank for black
                }
            }
        }
        
        // PAWN CHAINS AND CONNECTED PAWNS
        // Analyze pawn connections (pawns defending each other)
        uint64_t temp_white_pawns = white_pawns;
        while (temp_white_pawns) {
            int sq = POP(temp_white_pawns);
            int file = sq % 8;
            int rank = sq / 8;
            
            // Check if this pawn is defended by another pawn
            if (rank > 0) {
                if (file > 0 && (white_pawns & BIT_MASK[sq - 9])) { // Pawn on lower-left
                    score += PAWN_CHAIN_BONUS;
                }
                if (file < 7 && (white_pawns & BIT_MASK[sq - 7])) { // Pawn on lower-right
                    score += PAWN_CHAIN_BONUS;
                }
            }
            
            // Check for connected pawns (same rank, adjacent files)
            if (file > 0 && (white_pawns & BIT_MASK[sq - 1])) { // Pawn to the left
                score += CONNECTED_PAWNS_BONUS;
            }
            if (file < 7 && (white_pawns & BIT_MASK[sq + 1])) { // Pawn to the right
                score += CONNECTED_PAWNS_BONUS;
            }
        }
        
        uint64_t temp_black_pawns = black_pawns;
        while (temp_black_pawns) {
            int sq = POP(temp_black_pawns);
            int file = sq % 8;
            int rank = sq / 8;
            
            // Check if this pawn is defended by another pawn
            if (rank < 7) {
                if (file > 0 && (black_pawns & BIT_MASK[sq + 7])) { // Pawn on upper-left
                    score -= PAWN_CHAIN_BONUS;
                }
                if (file < 7 && (black_pawns & BIT_MASK[sq + 9])) { // Pawn on upper-right
                    score -= PAWN_CHAIN_BONUS;
                }
            }
            
            // Check for connected pawns (same rank, adjacent files)
            if (file > 0 && (black_pawns & BIT_MASK[sq - 1])) { // Pawn to the left
                score -= CONNECTED_PAWNS_BONUS;
            }
            if (file < 7 && (black_pawns & BIT_MASK[sq + 1])) { // Pawn to the right
                score -= CONNECTED_PAWNS_BONUS;
            }
        }
        
        // CRITICAL: Massive penalty for f6/f3 pawn moves (king safety disaster)
        // Check if f7 pawn moved to f6 (catastrophic weakening for Black)
        Piece f7_pawn = pos.at(sq(File::F, Rank::R7));
        Piece f6_pawn = pos.at(sq(File::F, Rank::R6));
        
        if ((is_none(f7_pawn) || color_of(f7_pawn) != Color::Black) && 
            (!is_none(f6_pawn) && color_of(f6_pawn) == Color::Black && type_of(f6_pawn) == PieceType::Pawn)) {
            // f6 move detected - this is nearly a blunder level move
            if (pos.fullmove_number <= 10) {
                score += 800; // MASSIVE penalty for Black in opening
            } else {
                score += 400; // Still very bad in middlegame
            }
        }
        
        // Check if f2 pawn moved to f3 (also weakening for White)
        Piece f2_pawn = pos.at(sq(File::F, Rank::R2));
        Piece f3_pawn = pos.at(sq(File::F, Rank::R3));
        
        if ((is_none(f2_pawn) || color_of(f2_pawn) != Color::White) && 
            (!is_none(f3_pawn) && color_of(f3_pawn) == Color::White && type_of(f3_pawn) == PieceType::Pawn)) {
            // f3 move detected - also weakening for White
            if (pos.fullmove_number <= 10) {
                score -= 800; // MASSIVE penalty for White in opening
            } else {
                score -= 400; // Still bad in middlegame
            }
        }
        
        // Return score from the perspective of the side to move
        return pos.side_to_move == Color::White ? score : -score;
    }

    int evaluate_development(const Position& pos) {
        int score = 0;
        
        // Check if we're still in opening/early middlegame (< 10 moves)
        bool early_game = pos.fullmove_number <= 10;
        if (!early_game) return 0; // Skip development evaluation in middlegame/endgame
        
        for (int color = 0; color < 2; ++color) {
            Color c = static_cast<Color>(color);
            int dev_score = 0;
            
            // MASSIVE penalty for early rook moves that break castling
            if (c == Color::White) {
                // Check if white rooks moved from original squares
                Piece a1_piece = pos.at(sq(File::A, Rank::R1));
                Piece h1_piece = pos.at(sq(File::H, Rank::R1));
                
                if (is_none(a1_piece) || type_of(a1_piece) != PieceType::Rook || color_of(a1_piece) != Color::White) {
                    // A1 rook moved early - check if it was a castling-breaking move
                    if (!(pos.castling_rights & CASTLE_WQ)) {
                        dev_score -= 200; // Heavy penalty for early queenside rook moves
                    }
                }
                
                if (is_none(h1_piece) || type_of(h1_piece) != PieceType::Rook || color_of(h1_piece) != Color::White) {
                    if (!(pos.castling_rights & CASTLE_WK)) {
                        dev_score -= 200; // Heavy penalty for early kingside rook moves  
                    }
                }
                
                // Extra penalty for very early rook moves (before move 5)
                if (pos.fullmove_number <= 5) {
                    for (int i = 0; i < pos.pCount[color][int(PieceType::Rook)]; ++i) {
                        int rook_sq = pos.pList[color][int(PieceType::Rook)][i];
                        // If rook is not on back rank, it moved early
                        if (rank_of(rook_sq) != Rank::R1) {
                            dev_score -= 300; // Massive penalty for very early rook development
                        }
                    }
                }
            } else {
                // Similar checks for Black
                Piece a8_piece = pos.at(sq(File::A, Rank::R8));
                Piece h8_piece = pos.at(sq(File::H, Rank::R8));
                
                if (is_none(a8_piece) || type_of(a8_piece) != PieceType::Rook || color_of(a8_piece) != Color::Black) {
                    if (!(pos.castling_rights & CASTLE_BQ)) {
                        dev_score -= 200; // Heavy penalty for early queenside rook moves
                    }
                }
                
                if (is_none(h8_piece) || type_of(h8_piece) != PieceType::Rook || color_of(h8_piece) != Color::Black) {
                    if (!(pos.castling_rights & CASTLE_BK)) {
                        dev_score -= 200; // Heavy penalty for early kingside rook moves
                    }
                }
                
                // Extra penalty for very early rook moves (before move 5)
                if (pos.fullmove_number <= 5) {
                    for (int i = 0; i < pos.pCount[color][int(PieceType::Rook)]; ++i) {
                        int rook_sq = pos.pList[color][int(PieceType::Rook)][i];
                        // If rook is not on back rank, it moved early
                        if (rank_of(rook_sq) != Rank::R8) {
                            dev_score -= 300; // Massive penalty for very early rook development
                        }
                    }
                }
            }
            
            // HUGE bonus for maintaining castling rights in early game
            if (c == Color::White) {
                if (pos.castling_rights & CASTLE_WK) dev_score += 75; // Increased from 50
                if (pos.castling_rights & CASTLE_WQ) dev_score += 75; // Increased from 50
            } else {
                if (pos.castling_rights & CASTLE_BK) dev_score += 75; // Increased from 50
                if (pos.castling_rights & CASTLE_BQ) dev_score += 75; // Increased from 50
            }
            
            // Bonus for proper minor piece development with "Knights on the rim are dim" penalty
            int developed_knights = 0;
            int developed_bishops = 0;
            
            for (int i = 0; i < pos.pCount[color][int(PieceType::Knight)]; ++i) {
                int knight_sq = pos.pList[color][int(PieceType::Knight)][i];
                Rank knight_rank = rank_of(knight_sq);
                File knight_file = file_of(knight_sq);
                
                // Knights developed off back rank get bonus
                if ((c == Color::White && knight_rank != Rank::R1) ||
                    (c == Color::Black && knight_rank != Rank::R8)) {
                    
                    // MASSIVE penalty for "Knights on the rim are dim"
                    if (knight_file == File::A || knight_file == File::H) {
                        dev_score -= 100; // Heavy penalty for rim knights (Na3, Nh3, Na6, Nh6, etc.)
                    } else if (knight_rank == Rank::R1 || knight_rank == Rank::R8) {
                        dev_score -= 50; // Penalty for back rank edge squares
                    } else {
                        developed_knights++; // Only count non-rim knights as "developed"
                    }
                } else {
                    // Even on back rank, penalize rim squares
                    if (knight_file == File::A || knight_file == File::H) {
                        dev_score -= 30; // Penalty for rim squares even on back rank
                    }
                }
            }
            
            for (int i = 0; i < pos.pCount[color][int(PieceType::Bishop)]; ++i) {
                int bishop_sq = pos.pList[color][int(PieceType::Bishop)][i];
                Rank bishop_rank = rank_of(bishop_sq);
                
                // Bishops developed off back rank get bonus
                if ((c == Color::White && bishop_rank != Rank::R1) ||
                    (c == Color::Black && bishop_rank != Rank::R8)) {
                    developed_bishops++;
                }
            }
            
            dev_score += developed_knights * 30;  // Bonus for knight development
            dev_score += developed_bishops * 25;  // Bonus for bishop development
            
            // Apply score based on side to move
            if (c == pos.side_to_move) {
                score += dev_score;
            } else {
                score -= dev_score;
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

    int evaluate_hanging_pieces(const Position& pos) {
        int score = 0;
        
        // Check all squares for pieces
        for (int sq = 0; sq < 64; ++sq) {
            Piece piece = pos.at(sq);
            if (piece == Piece::None) continue;
            
            Color piece_color = color_of(piece);
            PieceType piece_type = type_of(piece);
            
            // Skip pawns for now - they're handled differently
            if (piece_type == PieceType::Pawn) continue;
            
            // Check if this piece is attacked by the opponent
            bool is_attacked = SqAttacked(sq, pos, !piece_color);
            
            if (is_attacked) {
                // Check if this piece is defended by our pieces
                bool is_defended = SqAttacked(sq, pos, piece_color);
                
                if (!is_defended) {
                    // This piece is hanging! Apply penalty based on piece value
                    int penalty = 0;
                    switch (piece_type) {
                        case PieceType::Queen:  penalty = 800; break;  // Almost full queen value
                        case PieceType::Rook:   penalty = 400; break;  // Almost full rook value
                        case PieceType::Bishop: penalty = 250; break;  // Almost full bishop value
                        case PieceType::Knight: penalty = 250; break;  // Almost full knight value
                        case PieceType::King:   penalty = 50;  break;  // King hanging is bad but not losing
                        default: penalty = 0; break;
                    }
                    
                    // Apply penalty from the perspective of the piece owner
                    if (piece_color == Color::White) {
                        score -= penalty;  // White hanging piece = penalty for White
                    } else {
                        score += penalty;  // Black hanging piece = bonus for White
                    }
                }
            }
        }
        
        return score;
    }

    int evaluate_position(const Position& pos) {
        // Check for special positions first
        if (is_checkmate(pos)) {
            // If current side to move is in checkmate, they lose
            // Return negative score for the side in checkmate
            // But since evaluation is from current side's perspective, this is correct
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
        
        // Development evaluation (for opening play)
        score += evaluate_development(pos);
        
        // Hanging pieces evaluation (critical for piece safety)
        score += evaluate_hanging_pieces(pos);
        
        return score;
    }

    void analyze_opening_moves(int depth) {
        std::cout << "\n=== Opening Move Analysis (Static Evaluation) ===\n";
        std::cout << std::setw(8) << "Move" << std::setw(10) << "Score" << std::setw(12) << "Evaluation" << std::setw(12) << "Hash%" << std::endl;
        std::cout << std::string(42, '-') << std::endl;
        
        // Initialize starting position
        Position pos;
        pos.set_startpos();
        
        // Create search engine to track hash usage
        Search::Engine engine;
        engine.set_position(pos);
        
        // Generate all legal moves from starting position
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
        
        // Vector to store move evaluations
        struct MoveEval {
            S_MOVE move;
            int score;
            std::string move_str;
        };
        std::vector<MoveEval> move_evals;
        
        // Analyze each move
        for (int i = 0; i < moves.count; ++i) {
            S_MOVE move = moves.moves[i];
            
            // Make the move
            Position temp_pos = pos;
            temp_pos.make_move_with_undo(move);
            
            // Get the static evaluation (from White's perspective)
            int score = -evaluate_position(temp_pos); // Negate because it's Black's turn after White's move
            
            // Create move string
            std::string move_str = "";
            
            int from_sq = move.get_from();
            int to_sq = move.get_to();
            
            // Get piece at from square in original position
            Piece piece_moved = pos.at(from_sq);
            PieceType piece_type = type_of(piece_moved);
            
            // Format move string (basic algebraic notation)
            if (piece_type == PieceType::Pawn) {
                // Pawn moves - just show destination
                char file_char = 'a' + int(file_of(to_sq));
                char rank_char = '1' + int(rank_of(to_sq));
                
                // Check if it's a capture
                if (move.is_capture()) {
                    char from_file = 'a' + int(file_of(from_sq));
                    move_str = std::string(1, from_file) + "x" + std::string(1, file_char) + std::string(1, rank_char);
                } else {
                    move_str = std::string(1, file_char) + std::string(1, rank_char);
                }
                
                // Check for promotion
                if (move.get_promoted() != PieceType::None) {
                    char promo_char = '?';
                    switch (move.get_promoted()) {
                        case PieceType::Queen:  promo_char = 'Q'; break;
                        case PieceType::Rook:   promo_char = 'R'; break;
                        case PieceType::Bishop: promo_char = 'B'; break;
                        case PieceType::Knight: promo_char = 'N'; break;
                        default: break;
                    }
                    move_str += "=" + std::string(1, promo_char);
                }
            } else {
                // Piece moves
                char piece_char = '?';
                switch (piece_type) {
                    case PieceType::Knight: piece_char = 'N'; break;
                    case PieceType::Bishop: piece_char = 'B'; break;
                    case PieceType::Rook:   piece_char = 'R'; break;
                    case PieceType::Queen:  piece_char = 'Q'; break;
                    case PieceType::King:   piece_char = 'K'; break;
                    default: piece_char = '?'; break;
                }
                
                char to_file = 'a' + int(file_of(to_sq));
                char to_rank = '1' + int(rank_of(to_sq));
                
                // Check for castling
                if (move.is_castle()) {
                    if (to_file == 'g') {
                        move_str = "O-O";    // Kingside
                    } else {
                        move_str = "O-O-O";  // Queenside
                    }
                } else {
                    // Regular piece move
                    if (move.is_capture()) {
                        move_str = std::string(1, piece_char) + "x" + std::string(1, to_file) + std::string(1, to_rank);
                    } else {
                        move_str = std::string(1, piece_char) + std::string(1, to_file) + std::string(1, to_rank);
                    }
                }
            }
            
            move_evals.push_back({move, score, move_str});
            
            // Undo the move
            temp_pos.undo_move();
        }
        
        // Sort moves by score (best first)
        std::sort(move_evals.begin(), move_evals.end(), 
                  [](const MoveEval& a, const MoveEval& b) {
                      return a.score > b.score;
                  });
        
        // Print results
        for (size_t i = 0; i < move_evals.size(); ++i) {
            const auto& eval = move_evals[i];
            std::string eval_str;
            if (eval.score > 100) {
                eval_str = "Excellent";
            } else if (eval.score > 50) {
                eval_str = "Good";
            } else if (eval.score > -50) {
                eval_str = "OK";
            } else if (eval.score > -100) {
                eval_str = "Poor";
            } else {
                eval_str = "Bad";
            }
            
            // Get hash usage percentage (placeholder since we're using static eval)
            size_t hash_usage = 0; // Static evaluation doesn't use hash table
            
            std::cout << std::setw(8) << eval.move_str 
                      << std::setw(10) << std::showpos << eval.score << std::noshowpos
                      << std::setw(12) << eval_str 
                      << std::setw(11) << hash_usage << "%" << std::endl;
        }
        
        std::cout << "\nAnalysis complete! Scores are from White's perspective.\n";
        std::cout << "Positive scores favor White after the move.\n";
        std::cout << "Hash% shows transposition table usage.\n";
        std::cout << "This uses static evaluation, not search to depth " << depth << ".\n\n";
    }

} // namespace Evaluation
