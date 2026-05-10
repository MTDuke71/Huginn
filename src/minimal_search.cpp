#include "minimal_search.hpp"
#include "evaluation.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include "attack_tables.hpp"
#include "bitboard.hpp"
#include "board120.hpp"
#include "input_checking.hpp"
#include "msvc_optimizations.hpp"
#include "see.hpp"
#include <cassert>
#include <cmath>     // for std::log used by the LMR-table initializer
#include <array>
#include <iostream>
#include <algorithm>
#include <iomanip>  // For std::setw

// Backlog #13 bisection result (2026-05-06): ply tracking + TT-mate is
// shipped; counter-move is gated off pending a separate re-attempt.
//
// Bisection results vs huginn_t2 (100g, tc=10+0.1):
//   2a (both off, ply-tracking only):    +10 ± 61 Elo, LOS 63%   (neutral)
//   2b (counter-move @ 15K only):        +96 ± 66 Elo, LOS 99.9%
//   2c (TT-mate only, this build):      +104 ± 62 Elo, LOS 99.98%
//   2d (both on):                        +31 ± 52 Elo, LOS 88%   (anti-compound)
//   attempt 2 (both on, c-m at 700K):   -114 ± 64 Elo, LOS 0.01% (700K was the bug)
//
//   ENABLE_PLY_TRACKED_COUNTERMOVE: counter-move read in ordering + write on
//     beta cutoff. Score 15K (below promotions, above history). Gated off
//     because the counter-move + TT-mate combination anti-compounds — see
//     the followup BACKLOG entry to revisit with a different score slot.
//   ENABLE_PLY_TRACKED_TT_MATE: TT-store / TT-probe mate-distance adjustment,
//     mate-leaf encoding via info.ply (consistent under check extensions),
//     and a cap clamp so store_score stays inside (-MATE, MATE) after the
//     ply-add. The cap clamp is what rescued attempt 2's TT pollution case.
#ifndef ENABLE_PLY_TRACKED_COUNTERMOVE
#define ENABLE_PLY_TRACKED_COUNTERMOVE 0
#endif
#ifndef ENABLE_PLY_TRACKED_TT_MATE
#define ENABLE_PLY_TRACKED_TT_MATE 1
#endif

namespace Huginn {

// Contempt — penalty applied to draw scores from the side-to-move's
// perspective. Biases the search away from drawing lines: when the
// engine has a non-draw alternative, it'll prefer that alternative
// over a draw if the alternative is within `CONTEMPT` cp of equal.
// Filed as BACKLOG #16, motivated by 2026-05-08 game where Huginn
// (losing) accepted a fragile 0.00 repetition that black escaped
// from. Single static value is the standard simple implementation;
// can be made asymmetric (only at root) or material-dependent
// later if needed.
constexpr int CONTEMPT = 25;  // cp — tunable; gauntlet to validate

// LMR reduction table indexed by (depth, move-index). Reduction grows
// with both depth and move number. Formula: R = log(d) * log(m) / 2,
// truncated. Matches the MTLChess src/search.zig:63 table. Computed
// once at static initialization via lambda IIFE so it lives in .data
// (no per-call cost).
static const std::array<std::array<int, 64>, 64> LMR_TABLE = []() {
    std::array<std::array<int, 64>, 64> t{};
    for (int d = 1; d < 64; ++d) {
        for (int m = 1; m < 64; ++m) {
            t[d][m] = static_cast<int>(std::log(double(d)) * std::log(double(m)) / 2.0);
        }
    }
    return t;
}();

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
        return -CONTEMPT; // Insufficient material draw — contempt-biased (BACKLOG #16)
    }
    
    // VICE Part 56: Basic Evaluation with piece-square tables
    int score = 0;
    
    // VICE Part 82: Use pre-existing material tracking for endgame detection
    // This is much more efficient than manually counting material
    int total_material = pos.get_total_material();
    bool is_endgame = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD);
    
    // Evaluate all pieces using bitboard iteration
    for (int color = 0; color <= 1; ++color) {
        Color piece_color = static_cast<Color>(color);
        for (int piece_type = int(PieceType::Pawn); piece_type <= int(PieceType::King); ++piece_type) {
            PieceType pt = static_cast<PieceType>(piece_type);
            uint64_t bb = pos.piece_bitboards[color][piece_type];
            int material_value = PIECE_VALUES_MG[piece_type];
            while (bb) {
                int sq64 = pop_lsb(bb);
                int table_index = (piece_color == Color::Black) ? mirror_square_64(sq64) : sq64;
                int pst_value = 0;
                if (pt == PieceType::King) {
                    pst_value = is_endgame ? EvalParams::KING_TABLE_ENDGAME[table_index]
                                           : EvalParams::KING_TABLE[table_index];
                } else {
                    switch (pt) {
                        case PieceType::Pawn:   pst_value = EvalParams::PAWN_TABLE[table_index]; break;
                        case PieceType::Knight: pst_value = EvalParams::KNIGHT_TABLE[table_index]; break;
                        case PieceType::Bishop: pst_value = EvalParams::BISHOP_TABLE[table_index]; break;
                        case PieceType::Rook:   pst_value = EvalParams::ROOK_TABLE[table_index]; break;
                        case PieceType::Queen:  pst_value = EvalParams::QUEEN_TABLE[table_index]; break;
                        default: break;
                    }
                }
                int piece_value = material_value + pst_value;
                if (piece_color == Color::White) score += piece_value;
                else score -= piece_value;
            }
        }
    }
    
    // VICE Part 80: Enhanced pawn structure evaluation with pre-computed masks
    // Evaluate isolated pawns (2:13, 3:07) and passed pawns (2:21, 4:25)
    int pawn_structure_score = 0;
    
    // Get bitboards for efficient pawn structure analysis
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    
    // Iterate white pawns directly via bitboard
    uint64_t bb = white_pawns;
    while (bb) {
        int sq64 = pop_lsb(bb);
        int file_idx = sq64 & 7;
        int rank_idx = sq64 >> 3;

        if ((white_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx]) == 0) {
            pawn_structure_score -= EvalParams::ISOLATED_PAWN_PENALTY;
        }
        if ((black_pawns & EvalParams::WHITE_PASSED_PAWN_MASKS[sq64]) == 0) {
            pawn_structure_score += EvalParams::PASSED_PAWN_BONUS[rank_idx];
        }
    }

    // Iterate black pawns directly via bitboard
    bb = black_pawns;
    while (bb) {
        int sq64 = pop_lsb(bb);
        int file_idx = sq64 & 7;
        int rank_idx = sq64 >> 3;

        if ((black_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx]) == 0) {
            pawn_structure_score += EvalParams::ISOLATED_PAWN_PENALTY;
        }
        if ((white_pawns & EvalParams::BLACK_PASSED_PAWN_MASKS[sq64]) == 0) {
            int mirror_rank = 7 - rank_idx;
            pawn_structure_score -= EvalParams::PASSED_PAWN_BONUS[mirror_rank];
        }
    }

    // Doubled pawn penalty: each extra own pawn on the same file is a liability.
    // Two pawns on a file → −P; three pawns → −2P; etc.
    for (int f = 0; f < 8; ++f) {
        uint64_t file_mask = EvalParams::FILE_MASKS[f];
        int wpc = popcount(white_pawns & file_mask);
        int bpc = popcount(black_pawns & file_mask);
        if (wpc > 1) pawn_structure_score -= (wpc - 1) * EvalParams::DOUBLED_PAWN_PENALTY;
        if (bpc > 1) pawn_structure_score += (bpc - 1) * EvalParams::DOUBLED_PAWN_PENALTY;
    }

    score += pawn_structure_score;
    
    // VICE Part 81: Open and semi-open file bonuses for rooks and queens
    // Evaluate rooks and queens on open files (no pawns) or semi-open files (no own pawns)
    int file_bonus_score = 0;
    
    uint64_t all_pawns = white_pawns | black_pawns;
    
    // White rooks: open / semi-open file bonuses
    uint64_t rooks_bb = pos.piece_bitboards[int(Color::White)][int(PieceType::Rook)];
    while (rooks_bb) {
        int sq64 = pop_lsb(rooks_bb);
        int file_idx = sq64 & 7;
        uint64_t file_mask = EvalParams::FILE_MASKS[file_idx];
        if ((all_pawns & file_mask) == 0) {
            file_bonus_score += EvalParams::ROOK_OPEN_FILE_BONUS;
        } else if ((white_pawns & file_mask) == 0) {
            file_bonus_score += EvalParams::ROOK_SEMI_OPEN_FILE_BONUS;
        }
    }

    // Black rooks
    rooks_bb = pos.piece_bitboards[int(Color::Black)][int(PieceType::Rook)];
    while (rooks_bb) {
        int sq64 = pop_lsb(rooks_bb);
        int file_idx = sq64 & 7;
        uint64_t file_mask = EvalParams::FILE_MASKS[file_idx];
        if ((all_pawns & file_mask) == 0) {
            file_bonus_score -= EvalParams::ROOK_OPEN_FILE_BONUS;
        } else if ((black_pawns & file_mask) == 0) {
            file_bonus_score -= EvalParams::ROOK_SEMI_OPEN_FILE_BONUS;
        }
    }

    // White queens
    uint64_t queens_bb = pos.piece_bitboards[int(Color::White)][int(PieceType::Queen)];
    while (queens_bb) {
        int sq64 = pop_lsb(queens_bb);
        int file_idx = sq64 & 7;
        uint64_t file_mask = EvalParams::FILE_MASKS[file_idx];
        if ((all_pawns & file_mask) == 0) {
            file_bonus_score += EvalParams::QUEEN_OPEN_FILE_BONUS;
        } else if ((white_pawns & file_mask) == 0) {
            file_bonus_score += EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS;
        }
    }

    // Black queens
    queens_bb = pos.piece_bitboards[int(Color::Black)][int(PieceType::Queen)];
    while (queens_bb) {
        int sq64 = pop_lsb(queens_bb);
        int file_idx = sq64 & 7;
        uint64_t file_mask = EvalParams::FILE_MASKS[file_idx];
        if ((all_pawns & file_mask) == 0) {
            file_bonus_score -= EvalParams::QUEEN_OPEN_FILE_BONUS;
        } else if ((black_pawns & file_mask) == 0) {
            file_bonus_score -= EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS;
        }
    }
    
    score += file_bonus_score;
    
    // VICE Part 83: Bishop pair bonus
    int white_bishops = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Bishop)]);
    int black_bishops = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Bishop)]);

    if (white_bishops >= 2) {
        score += EvalParams::BISHOP_PAIR_BONUS;
    }
    if (black_bishops >= 2) {
        score -= EvalParams::BISHOP_PAIR_BONUS;
    }

    // -----------------------------------------------------------------
    // Mobility: count squares each non-pawn, non-king piece can move to
    // (excluding squares occupied by own pieces). Weighted per phase.
    // Pawns are excluded (their move semantics aren't "attack squares")
    // and kings are excluded (their squares are evaluated elsewhere).
    // -----------------------------------------------------------------
    {
        const int mob_weight = is_endgame ? EvalParams::MOBILITY_WEIGHT_ENDGAME
                                          : EvalParams::MOBILITY_WEIGHT_DEFAULT;
        int mobility_score = 0;
        const uint64_t occ = pos.occupied_bitboard;

        for (int color = 0; color <= 1; ++color) {
            const uint64_t own = pos.color_bitboards[color];
            int count = 0;

            // Knights
            uint64_t bb = pos.piece_bitboards[color][int(PieceType::Knight)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(knight_attacks[sq] & ~own);
            }
            // Bishops
            bb = pos.piece_bitboards[color][int(PieceType::Bishop)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(bishop_attacks(sq, occ) & ~own);
            }
            // Rooks
            bb = pos.piece_bitboards[color][int(PieceType::Rook)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(rook_attacks(sq, occ) & ~own);
            }
            // Queens
            bb = pos.piece_bitboards[color][int(PieceType::Queen)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(queen_attacks(sq, occ) & ~own);
            }

            if (color == int(Color::White)) mobility_score += count * mob_weight;
            else                            mobility_score -= count * mob_weight;
        }
        score += mobility_score;
    }

    // Return from current side's perspective (negate if black to move),
    // then add a tempo bonus (initiative goes to whoever moves next).
    int sided_score = (pos.side_to_move == Color::White) ? score : -score;
    return sided_score + EvalParams::TEMPO_BONUS;
}

// VICE Part 82/83: Material draw detection - Fixed to be more conservative
// Checks if the position is a theoretical draw based on insufficient material
bool MinimalEngine::MaterialDraw(const Position& pos) {
    int white_rooks   = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Rook)]);
    int black_rooks   = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Rook)]);
    int white_queens  = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Queen)]);
    int black_queens  = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Queen)]);
    int white_bishops = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Bishop)]);
    int black_bishops = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Bishop)]);
    int white_knights = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)]);
    int black_knights = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Knight)]);
    
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
    mirrored_pos.reset();

    // Mirror all pieces on the board (write through the bitboard-aware set())
    for (int sq120 = 21; sq120 <= 98; ++sq120) {
        Piece original_piece = pos.at(sq120);
        if (original_piece == Piece::Offboard || is_none(original_piece)) continue;

        int sq64 = MAILBOX_MAPS.to64[sq120];
        if (sq64 < 0) continue;

        int mirrored_sq64 = mirror64[sq64];
        int mirrored_sq120 = MAILBOX_MAPS.to120[mirrored_sq64];
        mirrored_pos.set(mirrored_sq120, swapPieceColor(original_piece));
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
    // Age search history array instead of clearing completely
    // Aging preserves recent learning while gradually fading old patterns
    for (int piece = 0; piece < 13; ++piece) {
        for (int sq = 0; sq < 120; ++sq) {
            // Reduce history scores by 75% (age by factor of 4)
            // This preserves 25% of existing knowledge while making room for new learning
            search_history[piece][sq] = search_history[piece][sq] / 4;
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
    
    Piece piece = pos.at(from);
    int piece_index = static_cast<int>(piece) % 13;  // Ensure valid index
    
    // Increase history score for this piece-to-square combination
    search_history[piece_index][to] += depth * depth;  // Deeper moves get higher bonus
}

// Penalize history for moves that fail to improve alpha (negative history scoring)
void MinimalEngine::penalize_search_history(const Position& pos, const S_MOVE& move, int depth) {
    if (move.move == 0) return;
    
    // Get piece and destination square
    int from = move.get_from();
    int to = move.get_to();
    
    if (from < 0 || from >= 120 || to < 0 || to >= 120) return;
    
    Piece piece = pos.at(from);
    int piece_index = static_cast<int>(piece) % 13;  // Ensure valid index
    
    // Decrease history score for this piece-to-square combination
    search_history[piece_index][to] -= depth * depth;  // Penalize with same magnitude as bonus
}

// Apply periodic aging to prevent history scores from becoming too large
void MinimalEngine::age_search_history() {
    for (int piece = 0; piece < 13; ++piece) {
        for (int sq = 0; sq < 120; ++sq) {
            // Reduce all history scores by 12.5% to maintain discrimination
            search_history[piece][sq] = (search_history[piece][sq] * 7) / 8;
        }
    }
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
    // Use the same piece values as evaluation for consistency
    // Note: King value set to 0 for MVV-LVA since king captures are illegal
    int piece_values[7] = {
        0,    // None
        100,  // Pawn
        320,  // Knight (matches PIECE_VALUES_MG)
        330,  // Bishop (matches PIECE_VALUES_MG)
        500,  // Rook
        900,  // Queen (matches PIECE_VALUES_MG)
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
                // Example: Pawn(100) takes Queen(900) = (900 * 100) + (600 - 100) = 90,500
                // Example: Queen(900) takes Pawn(100) = (100 * 100) + (600 - 900) = 9,700
                mvv_lva_scores[victim][attacker] = (piece_values[victim] * 100) + (600 - piece_values[attacker]);
            }
        }
    }
}

// Get MVV-LVA score for a capture move
int MinimalEngine::get_mvv_lva_score(PieceType victim, PieceType attacker) const {
    int victim_index = static_cast<int>(victim);
    int attacker_index = static_cast<int>(attacker);

    assert(victim_index >= 0 && victim_index < 7);
    assert(attacker_index >= 0 && attacker_index < 7);
    __assume(victim_index >= 0 && victim_index < 7);
    __assume(attacker_index >= 0 && attacker_index < 7);

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
            Piece attacking_piece = pos.at(from_sq);
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
                case PieceType::Bishop: score = 33000; break;
                case PieceType::Knight: score = 32000; break;
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
            Piece attacking_piece = pos.at(from_sq);
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
                case PieceType::Bishop: score = 33000; break;
                case PieceType::Knight: score = 32000; break;
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
int MinimalEngine::pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos, const SearchInfo& info, int depth, const S_MOVE& iid_move) const {
    // For the first call (move_num == 0), score all moves using VICE Part 64 ordering
    if (move_num == 0) {
        // VICE Part 84: Check for transposition table move (highest priority)
        int tt_score;
        uint8_t tt_depth, tt_node_type; 
        uint32_t tt_best_move;
        bool has_tt_move = tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
        
        // Validate TT move to ensure it's reasonable for this position
        bool tt_move_valid = false;
        if (has_tt_move && tt_best_move != 0) {
            // Basic sanity checks on the TT move
            S_MOVE tt_move;
            tt_move.move = static_cast<int>(tt_best_move);
            
            int from = tt_move.get_from();
            int to = tt_move.get_to();
            
            // Validate square bounds and that there's a piece to move
            if (from >= 0 && from < 120 && to >= 0 && to < 120 && 
                pos.at(from) != Piece::None && 
                color_of(pos.at(from)) == pos.side_to_move) {
                tt_move_valid = true;
            }
        }
        
        // Get PV move for this position (if any)
        S_MOVE pv_move;
        bool has_pv_move = pv_table.probe_move(pos.zobrist_key, pv_move);
        
        // Score all moves for ordering
        for (int i = 0; i < move_list.count; i++) {
            S_MOVE& move = move_list.moves[i];
            int score = 0;
            
            // VICE Part 84: TT move gets absolute highest priority (3,000,000) - but only if validated
            if (tt_move_valid && move.move == static_cast<int>(tt_best_move)) {
                score = 3000000;
                
            // VICE Part 64: PV move gets second highest priority (2,000,000)
            } else if (has_pv_move && move.move == pv_move.move) {
                score = 2000000;
                
            // IID move gets third highest priority (1,500,000) - between PV and captures
            } else if (iid_move.move != 0 && move.move == iid_move.move) {
                score = 1500000;
                
            } else if (move.is_capture()) {
                // VICE Part 64: Captures get 1,000,000 + MVV-LVA score
                PieceType victim = move.get_captured();
                
                // Get the attacking piece type from the position
                int from_sq = move.get_from();
                Piece attacking_piece = pos.at(from_sq);
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
#if ENABLE_PLY_TRACKED_COUNTERMOVE
                if (!is_killer && info.ply > 0 && info.ply < 64) {
                    // Get the previous move from search stack
                    S_MOVE previous_move = info.search_stack[info.ply - 1];
                    if (previous_move.move != 0) {
                        S_MOVE counter_move = get_counter_move(previous_move);
                        if (counter_move.move == move.move) {
                            // Slot above history (~1K) and below promotions (25K-90K).
                            // Original 700K placed counter-moves above queen promotion
                            // which is almost certainly wrong — see BACKLOG #13.
                            score = 15000;
                            is_counter_move = true;
                        }
                    }
                }
#endif
                
                if (!is_killer && !is_counter_move) {
                    if (move.is_promotion()) {
                        // Promotions: High priority, queen promotion highest
                        PieceType promoted = move.get_promoted();
                        switch (promoted) {
                            case PieceType::Queen:  score = 90000; break;
                            case PieceType::Rook:   score = 50000; break;
                            case PieceType::Bishop: score = 33000; break;
                            case PieceType::Knight: score = 32000; break;
                            default: score = 25000; break;
                        }
                    } else {
                        // VICE Part 64: History heuristic for remaining quiet moves
                        int from = move.get_from();
                        int to = move.get_to();
                        
                        if (from >= 0 && from < 120 && to >= 0 && to < 120) {
                            Piece piece = pos.at(from);
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
        generate_all_moves(pos, move_list);

        if (move_list.count == 0) break;

        int best_score = -30000;
        S_MOVE depth_best_move;
        depth_best_move.move = 0;
        int legal_count = 0;

        for (int i = 0; i < move_list.count; ++i) {
            // Check time more frequently during move loop
            if (time_up()) break;

            if (pos.MakeMove(move_list.moves[i]) != 1) continue;
            ++legal_count;

            // Create temporary SearchInfo for this search
            SearchInfo temp_info;
            temp_info.ply = 0;
            temp_info.stopped = false;

            // Track move in search stack for counter-move heuristic.
            // temp_info.ply is 0 here; this writes search_stack[0].
            temp_info.search_stack[0] = move_list.moves[i];

            // Use consistent VICE-style AlphaBeta search (not old alpha_beta)
            ++temp_info.ply;
            int score = -AlphaBeta(pos, -INFINITE, INFINITE, depth - 1, temp_info, true, false);
            --temp_info.ply;
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

        // No legal moves at this depth: mate/stalemate. Stop iterative deepening.
        if (legal_count == 0 && !time_up()) break;
        
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
#if ENABLE_PLY_TRACKED_TT_MATE
        // Adjust mate scores to current ply (VICE Part 84: 5:13)
        if (tt_score > MATE - 1000) {
            tt_score -= info.ply;
        } else if (tt_score < -MATE + 1000) {
            tt_score += info.ply;
        }
#endif

        // Use transposition table score if it provides exact bounds (6:01)
        if (tt_node_type == TTEntry::EXACT) {
            return tt_score;  // Exact score
        } else if (tt_node_type == TTEntry::LOWER_BOUND && tt_score >= beta) {
            return beta;  // Beta cutoff
        } else if (tt_node_type == TTEntry::UPPER_BOUND && tt_score <= alpha) {
            return alpha;  // Alpha cutoff  
        }
    }
    
    // Syzygy Tablebase Probe (BACKLOG #10 closure).
    // Only probe at leaf nodes (depth <= 1). Do NOT store the result in TT:
    // halfmove_clock is not part of the zobrist key, so the same key can be
    // reached with different rule50 values, but TB scores are only valid
    // for the rule50=0 case the safe wrapper requires. Caching a TB score
    // pollutes the TT for any future visit to the same piece-placement at
    // a different rule50 — measurable Elo loss in tournament play.
    if (depth <= 1 && tablebase && tablebase->is_available()) {
        int wdl_score;
        if (probe_tablebase_wdl(pos, wdl_score)) {
            return wdl_score;
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

    // Reverse futility / static null-move pruning.
    // At low depth, if our static eval is so far above beta that even giving
    // up `margin` centipawns per ply would still beat beta, we can return
    // immediately. Cheaper than null-move pruning (no make/unmake/recurse).
    const int REVERSE_FUTILITY_MAX_DEPTH = 6;
    const int REVERSE_FUTILITY_MARGIN = 80;  // cp per ply
    if (!in_check && !isRoot && depth > 0 && depth <= REVERSE_FUTILITY_MAX_DEPTH
            && beta < MATE - 1000 && beta > -(MATE - 1000)) {
        int eval = evalPosition(pos);
        int margin = REVERSE_FUTILITY_MARGIN * depth;
        if (eval - margin >= beta) {
            return eval - margin;
        }
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

        // Record null marker so the child's counter-move guard
        // (previous_move.move != 0) skips lookup over a null parent move.
        if (info.ply >= 0 && info.ply < 64) {
            info.search_stack[info.ply] = S_MOVE();
        }

        // Search with reduced depth and narrow window around beta
        ++info.ply;
        int null_score = -AlphaBeta(pos, -beta, -beta + 1, depth - 1 - NULL_MOVE_REDUCTION, info, false, false);
        --info.ply;

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

    // Futility Pruning (Forward Pruning at Pre-Frontier Nodes)
    // Skip move search if position evaluation + safety margin is still <= alpha
    // This is safe because even the best possible move won't improve alpha enough
    const int FUTILITY_MARGIN_BASE = 100;      // Base margin in centipawns
    const int FUTILITY_MARGIN_PER_PLY = 50;    // Additional margin per remaining ply
    const int MAX_FUTILITY_DEPTH = 3;          // Maximum depth to apply futility pruning
    
    bool futility_prune = false;
    int futility_margin = 0;
    
    if (depth <= MAX_FUTILITY_DEPTH && !in_check && !isRoot) {
        // Calculate position evaluation if we haven't already
        int eval = evalPosition(pos);
        
        // Calculate safety margin based on remaining depth
        futility_margin = FUTILITY_MARGIN_BASE + (FUTILITY_MARGIN_PER_PLY * depth);
        
        // If even with the safety margin we can't reach alpha, prune this node
        if (eval + futility_margin <= alpha) {
            futility_prune = true;
            
            // For debugging: track futility pruning statistics
            info.futility_cuts++;
            
            // Return alpha (or slightly better) since no move can improve it significantly
            return alpha;
        }
    }

    // Razoring: at low depth, if static eval + a generous margin is still
    // below alpha, the position is hopeless — reduce search depth by 1
    // rather than full search. Soft pruning (depth reduction, not return).
    {
        const int RAZORING_MARGIN = 400;        // 4 pawns of slack
        const int MAX_RAZORING_DEPTH = 4;
        if (depth >= 2 && depth <= MAX_RAZORING_DEPTH && !in_check && !isRoot) {
            int eval = evalPosition(pos);
            if (eval + RAZORING_MARGIN < alpha) {
                depth--;
                info.razoring_cuts++;
            }
        }
    }

    // Generate pseudo-legal moves; legality is checked per-move via MakeMove
    // below, and mate/stalemate is detected after the loop via legal_count.
    S_MOVELIST move_list;
    generate_all_moves(pos, move_list);

    // Check for repetition - but only after we ensure we have moves to try
    // This prevents returning draw score at root before storing any PV move (VICE fix)
    if (!isRoot && isRepetition(pos)) {
        return -CONTEMPT; // Repetition draw — contempt-biased (BACKLOG #16)
    }
    
    // Internal Iterative Deepening for PV nodes without hash move
    S_MOVE iid_move;
    iid_move.move = 0;
    
    // Check if we should perform IID (PV node without hash move)
    if (!isRoot && !tt_hit && depth >= 4) {
        // Likely PV node with full alpha-beta window and no hash move
        bool likely_pv_node = (beta - alpha > 1);
        if (likely_pv_node) {
            iid_move = internal_iterative_deepening(pos, alpha, beta, depth, info);
        }
    }
    
    int best_score = -30000;
    S_MOVE best_move;  // Track best move for transposition table storage
    best_move.move = 0;
    int legal_count = 0;  // For mate/stalemate detection after the loop

#ifdef USE_MULTI_CUT
    // Multi-Cut pruning: track beta cutoffs to enable early termination
    // If multiple moves cause beta-cutoffs, remaining moves are likely also good
    int beta_cutoff_count = 0;
    const int MULTI_CUT_THRESHOLD = 3;  // Number of cutoffs before pruning
    const int MIN_MULTI_CUT_DEPTH = 6;  // Minimum depth to apply multi-cut
    const int MULTI_CUT_MOVES_TO_TRY = 6;  // Try this many moves before applying multi-cut
#endif

    // Try each move
    for (int i = 0; i < move_list.count; ++i) {
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, info, depth, iid_move);

#ifdef USE_MULTI_CUT
        // Multi-Cut pruning: if we've had enough beta cutoffs, prune remaining moves
        if (depth >= MIN_MULTI_CUT_DEPTH && i >= MULTI_CUT_MOVES_TO_TRY &&
            beta_cutoff_count >= MULTI_CUT_THRESHOLD && !in_check) {

            // Skip remaining moves - position is too strong, remaining moves likely also good
            info.multi_cut_prunes += (move_list.count - i);
            break;
        }
#endif

        if (pos.MakeMove(move_list.moves[i]) != 1) continue; // Skip illegal moves
        ++legal_count;

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

            // Tuned reduction: log(d)*log(m)/2 lookup, clamped to leave at
            // least one ply of search after reduction. Replaces the prior
            // R=1 / R=2 step function — provides finer granularity at high
            // depth and high move number.
            int d_idx = std::min(depth, 63);
            int m_idx = std::min(i, 63);
            int reduction = LMR_TABLE[d_idx][m_idx];
            reduction = std::max(reduction, 1);
            reduction = std::min(reduction, depth - 2);

            // Try reduced search first
            int reduced_depth = depth - 1 - reduction;
            if (reduced_depth >= 1) {
                info.lmr_attempts++;  // Track LMR attempt
                ++info.ply;
                score = -AlphaBeta(pos, -alpha - 1, -alpha, reduced_depth, info, true, false);
                --info.ply;

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
                ++info.ply;
                score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
                --info.ply;
            } else {
                // PVS: Try null window first, then re-search if it fails high
                ++info.ply;
                score = -AlphaBeta(pos, -alpha - 1, -alpha, depth - 1, info, true, false);
                --info.ply;
                if (score > alpha && score < beta) {
                    // Null window search failed high, re-search with full window
                    ++info.ply;
                    score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
                    --info.ply;
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
                    
#ifdef USE_MULTI_CUT
                    // Track beta cutoffs for multi-cut pruning
                    beta_cutoff_count++;
#endif
                    
                    // Beta cutoff - update killer moves and history
                    update_killer_moves(move_list.moves[i], depth);
                    
#if ENABLE_PLY_TRACKED_COUNTERMOVE
                    // Update counter-move table if we have a previous move
                    if (info.ply > 0 && info.ply < 64) {
                        S_MOVE previous_move = info.search_stack[info.ply - 1];
                        if (previous_move.move != 0) {
                            update_counter_move(previous_move, move_list.moves[i]);
                        }
                    }
#endif
                    
                    break;
                }
            }
        } else {
            // Move didn't improve alpha - apply negative history scoring for quiet moves
            if (!move_list.moves[i].is_capture() && depth > 0) {
                penalize_search_history(pos, move_list.moves[i], depth);
            }
        }
    }

    // No legal moves found: checkmate or stalemate. Detect after the loop
    // since pseudo-legal generation can yield moves that all turn out to be
    // illegal (king-into-check, pinned piece, EP self-check).
    if (legal_count == 0 && !info.stopped && !info.quit) {
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttacked(king_sq, pos, !pos.side_to_move)) {
#if ENABLE_PLY_TRACKED_TT_MATE
            // Use info.ply so the leaf encoding matches the TT store/probe
            // adjustment that uses info.ply. The two diverge under check
            // extensions (which bump depth) — using info.max_depth-depth
            // would corrupt the TT for check-extended mate paths.
            return -MATE + info.ply;
#else
            return -MATE + (info.max_depth - depth); // Checkmate (loss with distance)
#endif
        }
        return -CONTEMPT; // Stalemate — contempt-biased (BACKLOG #16)
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
#if ENABLE_PLY_TRACKED_TT_MATE
    if (store_score > MATE - 1000) {
        store_score += info.ply;
        // Clamp to keep mate scores inside (-MATE, MATE) — see BACKLOG #13
        // step 4. Prevents `>= MATE` corruption if check extensions cause
        // encoding drift somewhere we didn't audit.
        if (store_score >= MATE) store_score = MATE - 1;
    } else if (store_score < -MATE + 1000) {
        store_score -= info.ply;
        if (store_score <= -MATE) store_score = -(MATE - 1);
    }
#endif

    tt_table.store(pos.zobrist_key, store_score, depth, node_type, best_move.move);

    return best_score;
}

// Internal Iterative Deepening for PV nodes without hash move
// Performs a shallow search to find a good move for ordering when no hash move is available
S_MOVE MinimalEngine::internal_iterative_deepening(Position& pos, int alpha, int beta, int depth, SearchInfo& info) {
    S_MOVE iid_move;
    iid_move.move = 0;  // Initialize to null move
    
    // Only perform IID if conditions are met:
    // 1. Sufficient depth to justify the overhead
    // 2. Likely PV node (full alpha-beta window)
    const int MIN_IID_DEPTH = 4;      // Minimum depth to perform IID
    const int IID_REDUCTION = 2;      // Reduce depth by 2 for IID search
    
    if (depth < MIN_IID_DEPTH) {
        return iid_move;  // Not worth the overhead at shallow depths
    }
    
    // Check if this looks like a PV node (not a null window search)
    bool likely_pv_node = (beta - alpha > 1);
    if (!likely_pv_node) {
        return iid_move;  // Only do IID for PV nodes
    }
    
    // Perform shallow search to find best move
    int iid_depth = depth - IID_REDUCTION;
    if (iid_depth >= 1) {
        // Generate pseudo-legal moves; the inner MakeMove guard handles legality.
        // No explicit mate detection here — IID is just an ordering hint, and
        // returning iid_move == 0 (its initial value) is fine if all moves are
        // illegal, since the parent AlphaBeta proceeds with its own search.
        S_MOVELIST iid_move_list;
        generate_all_moves(pos, iid_move_list);

        if (iid_move_list.count == 0) {
            return iid_move;  // No pseudo-legal moves at all
        }
        
        // Use simple move ordering for IID (no TT move dependency)
        order_moves(iid_move_list, pos);
        
        int best_score = -30000;
        
        // Try moves in IID search
        for (int i = 0; i < iid_move_list.count; ++i) {
            if (pos.MakeMove(iid_move_list.moves[i]) != 1) continue;

            // Track move in search stack for counter-move heuristic
            if (info.ply >= 0 && info.ply < 64) {
                info.search_stack[info.ply] = iid_move_list.moves[i];
            }

            ++info.ply;
            int score = -AlphaBeta(pos, -beta, -alpha, iid_depth, info, true, false);
            --info.ply;

            pos.TakeMove();
            
            // Check for early termination
            if (info.stopped || info.quit) {
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                iid_move = iid_move_list.moves[i];
                
                // Alpha-beta pruning in IID
                if (score >= beta) {
                    break;
                }
                if (score > alpha) {
                    alpha = score;
                }
            }
        }
    }
    
    return iid_move;
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
    
    // VICE Part 65: Generate only capture moves for quiescence search.
    // Pseudo-legal: the per-move `MakeMove() != 1` guard below filters illegals.
    // Saves the per-capture Make/Unmake legality filter that the legal version did.
    S_MOVELIST move_list;
    generate_all_caps_pseudo(pos, move_list);
    
    // Search all capture moves
    for (int i = 0; i < move_list.count; ++i) {
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, info, -1);  // No depth in quiescence

        S_MOVE move = move_list.moves[i];

        // SEE pruning: skip captures that lose material on the recapture
        // sequence. Promotions are searched anyway because the value gain
        // from promotion can flip a "bad" capture into a sound one. King
        // captures are never SEE-pruned (king is the most valuable, so
        // SEE wouldn't classify them as losing anyway, but be explicit).
        if (!move.is_promotion() && Huginn::see(pos, move) < 0) {
            continue;
        }

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
            // CRITICAL: Validate that the book move is actually legal in current position
            S_MOVELIST legal_moves;
            generate_legal_moves(pos, legal_moves);
            
            // Check if book move is in the legal move list
            bool book_move_is_legal = false;
            for (int i = 0; i < legal_moves.count; ++i) {
                if (legal_moves.moves[i].move == book_move.move) {
                    book_move_is_legal = true;
                    break;
                }
            }
            
            if (book_move_is_legal) {
                std::cout << "info string Found book move: " << move_to_uci(book_move) << std::endl;
                return book_move;
            } else {
                std::cout << "info string Book move " << move_to_uci(book_move) << " is illegal, ignoring" << std::endl;
            }
        }
    }
    
    // Syzygy Tablebase Root Probe - Check for perfect endgame move
    if (tablebase && tablebase->is_available()) {
        S_MOVE tablebase_move = probe_tablebase_root(pos);
        if (tablebase_move.move != 0) {
            std::cout << "info string Found tablebase move: " << move_to_uci(tablebase_move) << std::endl;
            return tablebase_move;
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

        // Iteration-start time gate: if the next iteration likely won't fit in
        // the remaining budget, return the previous depth's best move instead
        // of wasting time on a partial iteration that we'll discard anyway.
        // Heuristic: assume next iteration is ~3x the elapsed time so far;
        // bail if elapsed > budget/4 (i.e. 4*elapsed > budget).
        if (current_depth > 1 && !info.infinite && !info.depth_only
                && info.stop_time != std::chrono::steady_clock::time_point{}) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time).count();
            auto budget_ms = std::chrono::duration_cast<std::chrono::milliseconds>(info.stop_time - info.start_time).count();
            if (budget_ms > 0 && elapsed_ms * 4 > budget_ms) {
                break;
            }
        }

        info.depth = current_depth;
        
        // Apply periodic aging to prevent history scores from becoming too large
        // Age every 3 depths to maintain move discrimination
        if (current_depth > 1 && (current_depth % 3) == 0) {
            age_search_history();
        }
        
        // Store best move from previous iteration for move ordering
        S_MOVE prev_best = best_move;
        
        // Root search: try all moves at root to find the best one. Pseudo-legal
        // generation; legality is checked per-move via MakeMove inside the loop.
        S_MOVELIST move_list;
        generate_all_moves(pos, move_list);

        if (move_list.count == 0) break; // No pseudo-legal moves at all

        int best_score = -30000;
        S_MOVE depth_best_move;
        depth_best_move.move = 0;
        int legal_count = 0;

        // Order moves to try previous iteration's best move first for better alpha-beta cutoffs
        if (prev_best.move != 0) {
            for (int i = 0; i < move_list.count; ++i) {
                if (move_list.moves[i].move == prev_best.move) {
                    // Swap the previous best move to position 0
                    S_MOVE temp = move_list.moves[0];
                    move_list.moves[0] = move_list.moves[i];
                    move_list.moves[i] = temp;
                    break;
                }
            }
        }

        // Try each move at the root with PVS-style alpha tightening:
        // pass local_alpha (the best score found so far at root) as the
        // recursion's alpha, so subsequent subtrees can produce alpha-beta
        // cutoffs against it instead of searching with the full window.
        // The fail-high break is dormant here (beta = 30000 only fires on
        // found-mate) but is in place ready for aspiration windows.
        const int root_alpha_init = -30000;
        const int root_beta = 30000;
        int local_alpha = root_alpha_init;

        for (int i = 0; i < move_list.count; ++i) {
            if (info.stopped || info.quit) break;

            if (pos.MakeMove(move_list.moves[i]) != 1) continue; // Skip illegal moves
            ++legal_count;

            // Track move in search stack for counter-move heuristic.
            // info.ply is 0 at root; this writes search_stack[0].
            if (info.ply >= 0 && info.ply < 64) {
                info.search_stack[info.ply] = move_list.moves[i];
            }

            ++info.ply;
            int score = -AlphaBeta(pos, -root_beta, -local_alpha, current_depth - 1, info, true, false);
            --info.ply;
            pos.TakeMove();

            if (info.stopped || info.quit) break;

            if (score > best_score) {
                best_score = score;
                depth_best_move = move_list.moves[i];
                if (best_score > local_alpha) local_alpha = best_score;
                if (best_score >= root_beta) break; // fail-high (no aspiration yet → only mate)
            }
        }

        // If search was interrupted, return previous best move (time management benefit)
        if (info.stopped || info.quit) {
            break;
        }

        // No legal moves at root: mate or stalemate. Stop iterative deepening
        // since deeper iterations would just repeat the same empty result.
        if (legal_count == 0) break;
        
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
        
        // Print futility pruning statistics for this depth
        if (info.futility_cuts > 0) {
            std::cout << "info string Depth " << current_depth << " - Futility cuts: " 
                      << info.futility_cuts << " (" << std::fixed << std::setprecision(1) 
                      << (double(info.futility_cuts) / info.nodes * 100.0) << "%)" << std::endl;
        }
        
        // Print razoring statistics for this depth
        if (info.razoring_cuts > 0) {
            std::cout << "info string Depth " << current_depth << " - Razoring cuts: "
                      << info.razoring_cuts << " (" << std::fixed << std::setprecision(1)
                      << (double(info.razoring_cuts) / info.nodes * 100.0) << "%)" << std::endl;
        }
        
#ifdef USE_MULTI_CUT
        // Print multi-cut pruning statistics for this depth
        if (info.multi_cut_prunes > 0) {
            std::cout << "info string Depth " << current_depth << " - Multi-cut prunes: " 
                      << info.multi_cut_prunes << " (" << std::fixed << std::setprecision(1) 
                      << (double(info.multi_cut_prunes) / info.nodes * 100.0) << "%)" << std::endl;
        }
#endif

        // Iteration-start time gating happens at the top of the loop; no
        // post-iteration time check needed here.
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

// Syzygy Tablebase functions
bool MinimalEngine::probe_tablebase_wdl(const Position& pos, int& wdl_score) const {
    if (!tablebase || !tablebase->is_available()) {
        return false;  // No tablebase available
    }

    if (!tablebase->can_probe(pos)) {
        return false;  // Position cannot be probed
    }

    int result = tablebase->probe_wdl(pos);
    if (result == INT32_MAX) {
        return false;  // Probe failed
    }

    wdl_score = result;
    return true;
}

S_MOVE MinimalEngine::probe_tablebase_root(const Position& pos) const {
    S_MOVE null_move;
    null_move.move = 0;
    
    if (!tablebase || !tablebase->is_available()) {
        return null_move;  // No tablebase available
    }
    
    if (!tablebase->can_probe(pos)) {
        return null_move;  // Position cannot be probed
    }
    
    return tablebase->probe_root(pos);
}

} // namespace Huginn
