/**
 * @file pawn_movegen_bb.cpp
 * @brief Phase 2: Bitboard-based pawn move generation implementation
 * 
 * This file implements the pawn move generation migration from Phase 2 using
 * the exact pattern specified in the bitboard migration plan. It replaces
 * piece-list iteration with efficient bitboard shift operations.
 * 
 * ## Performance Optimization Strategy
 * 
 * **Bulk operations**: Process multiple pawns simultaneously with bitboard shifts
 * **Rank masks**: Use pre-computed rank bitboards for boundary conditions
 * **File masks**: Prevent wrap-around with file edge masks  
 * **Specialized functions**: Separate white/black for optimal shift directions
 * 
 * ## Migration Plan Implementation
 * 
 * This follows the exact code pattern from the migration plan document:
 * ```cpp
 * uint64_t our_pawns = pos.piece_bitboards[size_t(us)][size_t(PieceType::Pawn)];
 * uint64_t empty = ~pos.occupied_bitboard;
 * uint64_t enemies = pos.color_bitboards[1 - size_t(us)];
 * 
 * // Single pawn pushes
 * uint64_t single_pushes = (us == Color::White) ? 
 *     (our_pawns << 8) & empty : 
 *     (our_pawns >> 8) & empty;
 * ```
 * 
 * @author MTDuke71
 * @version 1.0
 * @see pawn_movegen_bb.hpp for interface documentation
 */

#include "pawn_movegen_bb.hpp"
#include "bitboard.hpp"
#include "board120.hpp"
#include "movegen_enhanced.hpp"  // For S_MOVELIST methods
#include "move.hpp"
#include "pawn_lookup_tables.hpp"  // For INVALID_SQUARE
#include <chrono>

// ============================================================================
// PHASE 2 MAIN IMPLEMENTATION (Migration Plan Pattern)
// ============================================================================

void generate_pawn_moves_bb(const Position& pos, S_MOVELIST& list) {
    Color us = pos.side_to_move;
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(us)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t empty = ~pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[1 - static_cast<size_t>(us)];
    
    if (our_pawns == 0) {
        return; // No pawns to move
    }
    
    if (us == Color::White) {
        generate_white_pawn_moves_bb(pos, list);
    } else {
        generate_black_pawn_moves_bb(pos, list);
    }
    
    // Generate en passant captures for both colors
    generate_en_passant_bb(pos, list);
}

// ============================================================================
// WHITE PAWN MOVE GENERATION
// ============================================================================

void generate_white_pawn_moves_bb(const Position& pos, S_MOVELIST& list) {
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t empty = ~pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[static_cast<size_t>(Color::Black)];
    
    // ---- Single pawn pushes (following migration plan pattern) ----
    uint64_t single_pushes = (our_pawns << 8) & empty;
    
    // ---- Double pawn pushes ----
    uint64_t double_pushes = ((single_pushes & RANK_3) << 8) & empty;
    
    // ---- Captures ----
    uint64_t left_captures = (our_pawns << 7) & enemies & ~FILE_H;  // Prevent H->A wrap
    uint64_t right_captures = (our_pawns << 9) & enemies & ~FILE_A; // Prevent A->H wrap
    
    // ---- Separate promotions from regular moves ----
    uint64_t promotion_pushes = single_pushes & RANK_8;
    uint64_t promotion_left_captures = left_captures & RANK_8;
    uint64_t promotion_right_captures = right_captures & RANK_8;
    
    // Remove promotions from regular moves
    single_pushes &= ~RANK_8;
    left_captures &= ~RANK_8;
    right_captures &= ~RANK_8;
    
    // ---- Process regular single pushes ----
    while (single_pushes != 0) {
        int to = pop_lsb(single_pushes);
        int from = to - 8;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            list.add_quiet_move(make_move(from_120, to_120));
        }
    }
    
    // ---- Process double pushes ----
    while (double_pushes != 0) {
        int to = pop_lsb(double_pushes);
        int from = to - 16;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            list.add_quiet_move(make_pawn_start(from_120, to_120));
        }
    }
    
    // ---- Process left captures ----
    while (left_captures != 0) {
        int to = pop_lsb(left_captures);
        int from = to - 7;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = type_of(pos.at(to_120));
            list.add_capture_move(make_capture(from_120, to_120, captured), pos);
        }
    }
    
    // ---- Process right captures ----
    while (right_captures != 0) {
        int to = pop_lsb(right_captures);
        int from = to - 9;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = type_of(pos.at(to_120));
            list.add_capture_move(make_capture(from_120, to_120, captured), pos);
        }
    }
    
    // ---- Process promotion pushes ----
    while (promotion_pushes != 0) {
        int to = pop_lsb(promotion_pushes);
        int from = to - 8;
        add_promotion_moves(from, to, list, false, pos);
    }
    
    // ---- Process promotion captures ----
    while (promotion_left_captures != 0) {
        int to = pop_lsb(promotion_left_captures);
        int from = to - 7;
        add_promotion_moves(from, to, list, true, pos);
    }
    
    while (promotion_right_captures != 0) {
        int to = pop_lsb(promotion_right_captures);
        int from = to - 9;
        add_promotion_moves(from, to, list, true, pos);
    }
}

// ============================================================================
// BLACK PAWN MOVE GENERATION
// ============================================================================

void generate_black_pawn_moves_bb(const Position& pos, S_MOVELIST& list) {
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(Color::Black)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t empty = ~pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[static_cast<size_t>(Color::White)];
    
    // ---- Single pawn pushes (black moves down, so right shift) ----
    uint64_t single_pushes = (our_pawns >> 8) & empty;
    
    // ---- Double pawn pushes ----
    uint64_t double_pushes = ((single_pushes & RANK_6) >> 8) & empty;
    
    // ---- Captures ----
    uint64_t left_captures = (our_pawns >> 9) & enemies & ~FILE_H;  // Prevent H->A wrap
    uint64_t right_captures = (our_pawns >> 7) & enemies & ~FILE_A; // Prevent A->H wrap
    
    // ---- Separate promotions from regular moves ----
    uint64_t promotion_pushes = single_pushes & RANK_1;
    uint64_t promotion_left_captures = left_captures & RANK_1;
    uint64_t promotion_right_captures = right_captures & RANK_1;
    
    // Remove promotions from regular moves
    single_pushes &= ~RANK_1;
    left_captures &= ~RANK_1;
    right_captures &= ~RANK_1;
    
    // ---- Process regular single pushes ----
    while (single_pushes != 0) {
        int to = pop_lsb(single_pushes);
        int from = to + 8;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            list.add_quiet_move(make_move(from_120, to_120));
        }
    }
    
    // ---- Process double pushes ----
    while (double_pushes != 0) {
        int to = pop_lsb(double_pushes);
        int from = to + 16;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            list.add_quiet_move(make_pawn_start(from_120, to_120));
        }
    }
    
    // ---- Process left captures ----
    while (left_captures != 0) {
        int to = pop_lsb(left_captures);
        int from = to + 9;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = type_of(pos.at(to_120));
            list.add_capture_move(make_capture(from_120, to_120, captured), pos);
        }
    }
    
    // ---- Process right captures ----
    while (right_captures != 0) {
        int to = pop_lsb(right_captures);
        int from = to + 7;
        int from_120 = MAILBOX_MAPS.to120[from];
        int to_120 = MAILBOX_MAPS.to120[to];
        
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = type_of(pos.at(to_120));
            list.add_capture_move(make_capture(from_120, to_120, captured), pos);
        }
    }
    
    // ---- Process promotion pushes ----
    while (promotion_pushes != 0) {
        int to = pop_lsb(promotion_pushes);
        int from = to + 8;
        add_promotion_moves(from, to, list, false, pos);
    }
    
    // ---- Process promotion captures ----
    while (promotion_left_captures != 0) {
        int to = pop_lsb(promotion_left_captures);
        int from = to + 9;
        add_promotion_moves(from, to, list, true, pos);
    }
    
    while (promotion_right_captures != 0) {
        int to = pop_lsb(promotion_right_captures);
        int from = to + 7;
        add_promotion_moves(from, to, list, true, pos);
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void add_promotion_moves(int from, int to, S_MOVELIST& list, bool capture_flag, const Position& pos) {
    int from_120 = MAILBOX_MAPS.to120[from];
    int to_120 = MAILBOX_MAPS.to120[to];
    
    if (from_120 == -1 || to_120 == -1) {
        return; // Invalid square conversion
    }
    
    PieceType captured = PieceType::None;
    if (capture_flag) {
        captured = type_of(pos.at(to_120));
    }
    
    // Add all 4 promotion types
    list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Queen, captured));
    list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Rook, captured));
    list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Bishop, captured));
    list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Knight, captured));
}

void generate_en_passant_bb(const Position& pos, S_MOVELIST& list) {
    // Check if en passant is available
    if (pos.ep_square == PawnLookupTables::INVALID_SQUARE) {
        return;
    }
    
    // Convert en passant square to 64-square index
    int ep_sq_64 = MAILBOX_MAPS.to64[pos.ep_square];
    if (ep_sq_64 == -1) {
        return; // Invalid en passant square
    }
    
    Color us = pos.side_to_move;
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(us)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t ep_target = 1ULL << ep_sq_64;
    
    if (us == Color::White) {
        // White en passant: look for pawns that can capture diagonally up
        uint64_t left_attackers = (ep_target >> 7) & our_pawns & ~FILE_A;
        uint64_t right_attackers = (ep_target >> 9) & our_pawns & ~FILE_H;
        
        while (left_attackers != 0) {
            int from = pop_lsb(left_attackers);
            int from_120 = MAILBOX_MAPS.to120[from];
            if (from_120 != -1) {
                list.add_en_passant_move(make_en_passant(from_120, pos.ep_square));
            }
        }
        
        while (right_attackers != 0) {
            int from = pop_lsb(right_attackers);
            int from_120 = MAILBOX_MAPS.to120[from];
            if (from_120 != -1) {
                list.add_en_passant_move(make_en_passant(from_120, pos.ep_square));
            }
        }
    } else {
        // Black en passant: look for pawns that can capture diagonally down
        uint64_t left_attackers = (ep_target << 9) & our_pawns & ~FILE_A;
        uint64_t right_attackers = (ep_target << 7) & our_pawns & ~FILE_H;
        
        while (left_attackers != 0) {
            int from = pop_lsb(left_attackers);
            int from_120 = MAILBOX_MAPS.to120[from];
            if (from_120 != -1) {
                list.add_en_passant_move(make_en_passant(from_120, pos.ep_square));
            }
        }
        
        while (right_attackers != 0) {
            int from = pop_lsb(right_attackers);
            int from_120 = MAILBOX_MAPS.to120[from];
            if (from_120 != -1) {
                list.add_en_passant_move(make_en_passant(from_120, pos.ep_square));
            }
        }
    }
}

// ============================================================================
// PERFORMANCE COMPARISON
// ============================================================================

double benchmark_pawn_move_performance(const Position& pos, int iterations) {
    using namespace std::chrono;
    
    // Benchmark new bitboard method
    auto start_new = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        S_MOVELIST list_new;
        list_new.count = 0;
        generate_pawn_moves_bb(pos, list_new);
        
        // Prevent optimization from eliminating the work
        volatile int dummy = list_new.count;
        (void)dummy;
    }
    auto end_new = high_resolution_clock::now();
    double new_time = duration_cast<microseconds>(end_new - start_new).count() / 1000.0;
    
    // For comparison with old method, we would need to call the original
    // piece-list based pawn generation here. This is left as a placeholder
    // for the benchmarking framework.
    
    return new_time; // Return timing for now
}