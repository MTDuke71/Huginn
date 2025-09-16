/**
 * @file pawn_movegen_native_bb.cpp
 * @brief Native bitboard pawn move generation implementation
 * 
 * This file implements the conversion-overhead-free approach to bitboard
 * pawn move generation. Instead of converting each square individually,
 * we process entire bitboards and defer conversion to the integration point.
 */

#include "pawn_movegen_native_bb.hpp"
#include "pawn_movegen_bb.hpp"  // For generate_pawn_moves_bb
#include "bitboard.hpp"
#include "board120.hpp"
#include "movegen_enhanced.hpp"
#include "move.hpp"
#include "pawn_lookup_tables.hpp"
#include <chrono>

// ============================================================================
// MAIN NATIVE BITBOARD IMPLEMENTATION  
// ============================================================================

void generate_pawn_moves_native_bb(const Position& pos, S_MOVELIST& list) {
    Color us = pos.side_to_move;
    
    // Early exit for no pawns
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(us)][static_cast<size_t>(PieceType::Pawn)];
    if (our_pawns == 0) {
        return;
    }
    
    // Process all pawn moves in bulk without conversion overhead
    BulkPawnMoves bulk_moves;
    if (us == Color::White) {
        bulk_moves = process_white_pawn_bitboards_bulk(pos);
    } else {
        bulk_moves = process_black_pawn_bitboards_bulk(pos);
    }
    
    // Integrate bulk moves with minimal conversion
    integrate_bulk_pawn_moves(bulk_moves, list, us, pos);
    
    // Handle en passant separately (lower frequency)
    generate_en_passant_native_bb(pos, list);
}

// ============================================================================
// BULK BITBOARD PROCESSING (NO CONVERSION OVERHEAD)
// ============================================================================

BulkPawnMoves process_white_pawn_bitboards_bulk(const Position& pos) {
    BulkPawnMoves moves;
    
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t empty = ~pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[static_cast<size_t>(Color::Black)];
    
    // ---- Single pawn pushes (pure bitboard operation) ----
    uint64_t single_pushes = (our_pawns << 8) & empty;
    
    // ---- Double pawn pushes (chained bitboard operation) ----
    uint64_t double_pushes = ((single_pushes & RANK_3) << 8) & empty;
    
    // ---- Captures (bitboard with file mask) ----
    uint64_t left_captures = (our_pawns << 7) & enemies & ~FILE_H;
    uint64_t right_captures = (our_pawns << 9) & enemies & ~FILE_A;
    
    // ---- Separate promotions from regular moves (single bitboard operation) ----
    moves.promotion_pushes = single_pushes & RANK_8;
    moves.promotion_left_captures = left_captures & RANK_8;
    moves.promotion_right_captures = right_captures & RANK_8;
    
    // ---- Regular moves (bitboard subtraction) ----
    moves.single_pushes = single_pushes & ~RANK_8;
    moves.double_pushes = double_pushes;
    moves.left_captures = left_captures & ~RANK_8;
    moves.right_captures = right_captures & ~RANK_8;
    
    return moves;
}

BulkPawnMoves process_black_pawn_bitboards_bulk(const Position& pos) {
    BulkPawnMoves moves;
    
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(Color::Black)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t empty = ~pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[static_cast<size_t>(Color::White)];
    
    // ---- Single pawn pushes (pure bitboard operation) ----
    uint64_t single_pushes = (our_pawns >> 8) & empty;
    
    // ---- Double pawn pushes (chained bitboard operation) ----
    uint64_t double_pushes = ((single_pushes & RANK_6) >> 8) & empty;
    
    // ---- Captures (bitboard with file mask) ----
    uint64_t left_captures = (our_pawns >> 9) & enemies & ~FILE_H;
    uint64_t right_captures = (our_pawns >> 7) & enemies & ~FILE_A;
    
    // ---- Separate promotions from regular moves (single bitboard operation) ----
    moves.promotion_pushes = single_pushes & RANK_1;
    moves.promotion_left_captures = left_captures & RANK_1;
    moves.promotion_right_captures = right_captures & RANK_1;
    
    // ---- Regular moves (bitboard subtraction) ----
    moves.single_pushes = single_pushes & ~RANK_1;
    moves.double_pushes = double_pushes;
    moves.left_captures = left_captures & ~RANK_1;
    moves.right_captures = right_captures & ~RANK_1;
    
    // Note: Black uses different offsets (negative values)
    return moves;
}

// ============================================================================
// OPTIMIZED BULK INTEGRATION (MINIMAL CONVERSION)
// ============================================================================

void integrate_bulk_pawn_moves(const BulkPawnMoves& bulk_moves, S_MOVELIST& list, 
                               Color us, const Position& pos) {
    // Determine direction-specific offsets
    int push_offset = (us == Color::White) ? -8 : 8;
    int double_offset = (us == Color::White) ? -16 : 16;
    int left_offset = (us == Color::White) ? -7 : 9;
    int right_offset = (us == Color::White) ? -9 : 7;
    
    // ---- Process regular moves with optimized conversion ----
    
    // Single pushes
    uint64_t single_pushes = bulk_moves.single_pushes;
    while (single_pushes != 0) {
        int to_64 = pop_lsb(single_pushes);
        int from_64 = to_64 + push_offset;
        int from_120 = MAILBOX_MAPS.to120[from_64];
        int to_120 = MAILBOX_MAPS.to120[to_64];
        if (from_120 != -1 && to_120 != -1) {
            list.add_quiet_move(make_move(from_120, to_120));
        }
    }
    
    // Double pushes
    uint64_t double_pushes = bulk_moves.double_pushes;
    while (double_pushes != 0) {
        int to_64 = pop_lsb(double_pushes);
        int from_64 = to_64 + double_offset;
        int from_120 = MAILBOX_MAPS.to120[from_64];
        int to_120 = MAILBOX_MAPS.to120[to_64];
        if (from_120 != -1 && to_120 != -1) {
            list.add_quiet_move(make_pawn_start(from_120, to_120));
        }
    }
    
    // Left captures
    uint64_t left_captures = bulk_moves.left_captures;
    while (left_captures != 0) {
        int to_64 = pop_lsb(left_captures);
        int from_64 = to_64 + left_offset;
        int from_120 = MAILBOX_MAPS.to120[from_64];
        int to_120 = MAILBOX_MAPS.to120[to_64];
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = type_of(pos.at(to_120));
            list.add_capture_move(make_capture(from_120, to_120, captured), pos);
        }
    }
    
    // Right captures
    uint64_t right_captures = bulk_moves.right_captures;
    while (right_captures != 0) {
        int to_64 = pop_lsb(right_captures);
        int from_64 = to_64 + right_offset;
        int from_120 = MAILBOX_MAPS.to120[from_64];
        int to_120 = MAILBOX_MAPS.to120[to_64];
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = type_of(pos.at(to_120));
            list.add_capture_move(make_capture(from_120, to_120, captured), pos);
        }
    }
    
    // ---- Process promotions with bulk conversion ----
    bulk_add_promotion_moves(bulk_moves.promotion_pushes, list, push_offset, false, pos);
    bulk_add_promotion_moves(bulk_moves.promotion_left_captures, list, left_offset, true, pos);
    bulk_add_promotion_moves(bulk_moves.promotion_right_captures, list, right_offset, true, pos);
}

// ============================================================================
// BULK CONVERSION UTILITIES (OPTIMIZED FOR SPEED)
// ============================================================================

void bulk_add_promotion_moves(uint64_t promotion_bitboard, S_MOVELIST& list,
                             int offset, bool is_capture, const Position& pos) {
    while (promotion_bitboard != 0) {
        int to_64 = pop_lsb(promotion_bitboard);
        int from_64 = to_64 + offset;
        
        int from_120 = MAILBOX_MAPS.to120[from_64];
        int to_120 = MAILBOX_MAPS.to120[to_64];
        
        if (from_120 != -1 && to_120 != -1) {
            PieceType captured = is_capture ? type_of(pos.at(to_120)) : PieceType::None;
            
            // Add all 4 promotion variants
            list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Queen, captured));
            list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Rook, captured));
            list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Bishop, captured));
            list.add_promotion_move(make_promotion(from_120, to_120, PieceType::Knight, captured));
        }
    }
}

// ============================================================================
// EN PASSANT (NATIVE BITBOARD IMPLEMENTATION)
// ============================================================================

void generate_en_passant_native_bb(const Position& pos, S_MOVELIST& list) {
    if (pos.ep_square == PawnLookupTables::INVALID_SQUARE) {
        return;
    }
    
    int ep_sq_64 = MAILBOX_MAPS.to64[pos.ep_square];
    if (ep_sq_64 == -1) {
        return;
    }
    
    Color us = pos.side_to_move;
    uint64_t our_pawns = pos.piece_bitboards[static_cast<size_t>(us)][static_cast<size_t>(PieceType::Pawn)];
    uint64_t ep_target = 1ULL << ep_sq_64;
    
    // Find attacking pawns with bitboard operations
    uint64_t attackers = 0;
    if (us == Color::White) {
        attackers = ((ep_target >> 7) & ~FILE_A) | ((ep_target >> 9) & ~FILE_H);
    } else {
        attackers = ((ep_target << 7) & ~FILE_H) | ((ep_target << 9) & ~FILE_A);
    }
    
    attackers &= our_pawns;
    
    // Convert en passant attackers (typically 0-2 moves)
    while (attackers != 0) {
        int from_64 = pop_lsb(attackers);
        int from_120 = MAILBOX_MAPS.to120[from_64];
        if (from_120 != -1) {
            list.add_en_passant_move(make_en_passant(from_120, pos.ep_square));
        }
    }
}

// ============================================================================
// PERFORMANCE BENCHMARKING
// ============================================================================

NativeBitboardBenchmark benchmark_native_vs_conversion(const Position& pos, int iterations) {
    using namespace std::chrono;
    
    NativeBitboardBenchmark result;
    
    // Benchmark native implementation
    auto start_native = high_resolution_clock::now();
    int moves_count = 0;
    for (int i = 0; i < iterations; i++) {
        S_MOVELIST list_native;
        generate_pawn_moves_native_bb(pos, list_native);
        moves_count = list_native.count;
        volatile int dummy = list_native.count;
        (void)dummy;
    }
    auto end_native = high_resolution_clock::now();
    result.native_time_ms = duration_cast<microseconds>(end_native - start_native).count() / 1000.0;
    
    // Benchmark conversion-heavy implementation
    auto start_conversion = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        S_MOVELIST list_conversion;
        generate_pawn_moves_bb(pos, list_conversion);  // Original implementation
        volatile int dummy = list_conversion.count;
        (void)dummy;
    }
    auto end_conversion = high_resolution_clock::now();
    result.conversion_time_ms = duration_cast<microseconds>(end_conversion - start_conversion).count() / 1000.0;
    
    result.improvement_percent = ((result.conversion_time_ms - result.native_time_ms) / result.conversion_time_ms) * 100.0;
    result.moves_generated = moves_count;
    
    return result;
}