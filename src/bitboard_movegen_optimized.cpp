#include "bitboard_movegen_optimized.hpp"
#include "bitboard.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include "knight_lookup_tables.hpp"
#include "king_lookup_tables.hpp"
#include "pawn_lookup_tables.hpp"
#include "move.hpp"
#include <intrin.h>

// MSVC equivalent for __builtin_ctzll
#ifdef _MSC_VER
inline int ctz(U64 x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
}
#else
#define ctz(x) __builtin_ctzll(x)
#endif

namespace OptimizedBitboardMoveGen {

void generate_all_moves_optimized(const Position& pos, S_MOVELIST& move_list) {
    const Color side = pos.side;
    
    // Generate pawn moves with bulk processing
    generate_pawn_moves_bulk(pos, move_list, side);
    
    // Generate piece moves with optimized algorithms
    if (side == WHITE) {
        // White pieces
        generate_sliding_moves_bulk(pos, move_list, pos.bitboards[wR], ROOK, WHITE);
        generate_sliding_moves_bulk(pos, move_list, pos.bitboards[wB], BISHOP, WHITE);
        generate_sliding_moves_bulk(pos, move_list, pos.bitboards[wQ], QUEEN, WHITE);
    } else {
        // Black pieces  
        generate_sliding_moves_bulk(pos, move_list, pos.bitboards[bR], ROOK, BLACK);
        generate_sliding_moves_bulk(pos, move_list, pos.bitboards[bB], BISHOP, BLACK);
        generate_sliding_moves_bulk(pos, move_list, pos.bitboards[bQ], QUEEN, BLACK);
    }
    
    // Generate knight moves
    generate_knight_moves_optimized(pos, move_list, side);
    
    // Generate king moves  
    generate_king_moves_optimized(pos, move_list, side);
}

void generate_sliding_moves_bulk(const Position& pos, S_MOVELIST& move_list, 
                                U64 pieces, int piece_type, Color color) {
    const U64 occupied = pos.bitboards[ALL_PIECES];
    const U64 friendly = (color == WHITE) ? pos.bitboards[WHITE_PIECES] : pos.bitboards[BLACK_PIECES];
    
    while (pieces) {
        const int from = ctz(pieces);
        pieces ^= (1ULL << from);
        
        U64 attacks;
        if (piece_type == ROOK) {
            attacks = GetRookAttacks(from, occupied);
        } else if (piece_type == BISHOP) {
            attacks = GetBishopAttacks(from, occupied);
        } else { // QUEEN
            attacks = GetRookAttacks(from, occupied) | GetBishopAttacks(from, occupied);
        }
        
        attacks &= ~friendly; // Remove friendly pieces
        
        // Bulk process all destination squares
        while (attacks) {
            const int to = ctz(attacks);
            attacks ^= (1ULL << to);
            
            const int captured = pos.board[to];
            if (captured != EMPTY) {
                AddCaptureMove(pos, from, to, captured, 0, &move_list);
            } else {
                AddQuietMove(pos, from, to, 0, &move_list);
            }
        }
    }
}

void generate_knight_moves_optimized(const Position& pos, S_MOVELIST& move_list, Color color) {
    U64 knights = (color == WHITE) ? pos.bitboards[wN] : pos.bitboards[bN];
    const U64 friendly = (color == WHITE) ? pos.bitboards[WHITE_PIECES] : pos.bitboards[BLACK_PIECES];
    
    while (knights) {
        const int from = ctz(knights);
        knights ^= (1ULL << from);
        
        U64 attacks = knight_attacks[from] & ~friendly;
        
        while (attacks) {
            const int to = ctz(attacks);
            attacks ^= (1ULL << to);
            
            const int captured = pos.board[to];
            if (captured != EMPTY) {
                AddCaptureMove(pos, from, to, captured, 0, &move_list);
            } else {
                AddQuietMove(pos, from, to, 0, &move_list);
            }
        }
    }
}

void generate_king_moves_optimized(const Position& pos, S_MOVELIST& move_list, Color color) {
    const int king_sq = (color == WHITE) ? pos.king_square[WHITE] : pos.king_square[BLACK];
    const U64 friendly = (color == WHITE) ? pos.bitboards[WHITE_PIECES] : pos.bitboards[BLACK_PIECES];
    
    U64 attacks = king_attacks[king_sq] & ~friendly;
    
    while (attacks) {
        const int to = ctz(attacks);
        attacks ^= (1ULL << to);
        
        const int captured = pos.board[to];
        if (captured != EMPTY) {
            AddCaptureMove(pos, king_sq, to, captured, 0, &move_list);
        } else {
            AddQuietMove(pos, king_sq, to, 0, &move_list);
        }
    }
    
    // Castling moves
    if (color == WHITE) {
        if (pos.castle_permission & WKCA) {
            if (pos.board[F1] == EMPTY && pos.board[G1] == EMPTY) {
                if (!is_square_attacked(E1, BLACK, pos) && 
                    !is_square_attacked(F1, BLACK, pos)) {
                    AddQuietMove(pos, E1, G1, MOVE_FLAG_CASTLE, &move_list);
                }
            }
        }
        if (pos.castle_permission & WQCA) {
            if (pos.board[D1] == EMPTY && pos.board[C1] == EMPTY && pos.board[B1] == EMPTY) {
                if (!is_square_attacked(E1, BLACK, pos) && 
                    !is_square_attacked(D1, BLACK, pos)) {
                    AddQuietMove(pos, E1, C1, MOVE_FLAG_CASTLE, &move_list);
                }
            }
        }
    } else {
        if (pos.castle_permission & BKCA) {
            if (pos.board[F8] == EMPTY && pos.board[G8] == EMPTY) {
                if (!is_square_attacked(E8, WHITE, pos) && 
                    !is_square_attacked(F8, WHITE, pos)) {
                    AddQuietMove(pos, E8, G8, MOVE_FLAG_CASTLE, &move_list);
                }
            }
        }
        if (pos.castle_permission & BQCA) {
            if (pos.board[D8] == EMPTY && pos.board[C8] == EMPTY && pos.board[B8] == EMPTY) {
                if (!is_square_attacked(E8, WHITE, pos) && 
                    !is_square_attacked(D8, WHITE, pos)) {
                    AddQuietMove(pos, E8, C8, MOVE_FLAG_CASTLE, &move_list);
                }
            }
        }
    }
}

void generate_pawn_moves_bulk(const Position& pos, S_MOVELIST& move_list, Color color) {
    U64 pawns = (color == WHITE) ? pos.bitboards[wP] : pos.bitboards[bP];
    const U64 occupied = pos.bitboards[ALL_PIECES];
    const U64 enemy = (color == WHITE) ? pos.bitboards[BLACK_PIECES] : pos.bitboards[WHITE_PIECES];
    
    if (color == WHITE) {
        // White pawn pushes
        U64 single_pushes = (pawns << 8) & ~occupied;
        U64 double_pushes = ((single_pushes & RANK_BB[2]) << 8) & ~occupied;
        
        // Process single pushes
        U64 promotions = single_pushes & RANK_BB[7];
        single_pushes &= ~RANK_BB[7];
        
        while (single_pushes) {
            const int to = ctz(single_pushes);
            single_pushes ^= (1ULL << to);
            AddQuietMove(pos, to - 8, to, 0, &move_list);
        }
        
        // Process double pushes
        while (double_pushes) {
            const int to = ctz(double_pushes);
            double_pushes ^= (1ULL << to);
            AddQuietMove(pos, to - 16, to, MOVE_FLAG_PAWN_START, &move_list);
        }
        
        // Process promotions
        while (promotions) {
            const int to = ctz(promotions);
            promotions ^= (1ULL << to);
            const int from = to - 8;
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_QUEEN, &move_list);
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_ROOK, &move_list);
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_BISHOP, &move_list);
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_KNIGHT, &move_list);
        }
        
        // White pawn captures
        U64 left_captures = ((pawns & ~FILE_BB[0]) << 7) & enemy;
        U64 right_captures = ((pawns & ~FILE_BB[7]) << 9) & enemy;
        
        // Process left captures
        U64 left_promotions = left_captures & RANK_BB[7];
        left_captures &= ~RANK_BB[7];
        
        while (left_captures) {
            const int to = ctz(left_captures);
            left_captures ^= (1ULL << to);
            const int captured = pos.board[to];
            AddCaptureMove(pos, to - 7, to, captured, 0, &move_list);
        }
        
        while (left_promotions) {
            const int to = ctz(left_promotions);
            left_promotions ^= (1ULL << to);
            const int from = to - 7;
            const int captured = pos.board[to];
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_QUEEN, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_ROOK, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_BISHOP, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_KNIGHT, &move_list);
        }
        
        // Process right captures
        U64 right_promotions = right_captures & RANK_BB[7];
        right_captures &= ~RANK_BB[7];
        
        while (right_captures) {
            const int to = ctz(right_captures);
            right_captures ^= (1ULL << to);
            const int captured = pos.board[to];
            AddCaptureMove(pos, to - 9, to, captured, 0, &move_list);
        }
        
        while (right_promotions) {
            const int to = ctz(right_promotions);
            right_promotions ^= (1ULL << to);
            const int from = to - 9;
            const int captured = pos.board[to];
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_QUEEN, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_ROOK, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_BISHOP, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_KNIGHT, &move_list);
        }
        
        // En passant
        if (pos.en_passant != NO_SQ) {
            U64 ep_pawns = pawns & pawn_attacks[BLACK][pos.en_passant];
            while (ep_pawns) {
                const int from = ctz(ep_pawns);
                ep_pawns ^= (1ULL << from);
                AddEnPassantMove(pos, from, pos.en_passant, &move_list);
            }
        }
    } else {
        // Black pawn moves (mirror of white)
        U64 single_pushes = (pawns >> 8) & ~occupied;
        U64 double_pushes = ((single_pushes & RANK_BB[5]) >> 8) & ~occupied;
        
        // Process single pushes
        U64 promotions = single_pushes & RANK_BB[0];
        single_pushes &= ~RANK_BB[0];
        
        while (single_pushes) {
            const int to = ctz(single_pushes);
            single_pushes ^= (1ULL << to);
            AddQuietMove(pos, to + 8, to, 0, &move_list);
        }
        
        // Process double pushes
        while (double_pushes) {
            const int to = ctz(double_pushes);
            double_pushes ^= (1ULL << to);
            AddQuietMove(pos, to + 16, to, MOVE_FLAG_PAWN_START, &move_list);
        }
        
        // Process promotions
        while (promotions) {
            const int to = ctz(promotions);
            promotions ^= (1ULL << to);
            const int from = to + 8;
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_QUEEN, &move_list);
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_ROOK, &move_list);
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_BISHOP, &move_list);
            AddQuietMove(pos, from, to, MOVE_FLAG_PROMOTION_KNIGHT, &move_list);
        }
        
        // Black pawn captures
        U64 left_captures = ((pawns & ~FILE_BB[0]) >> 9) & enemy;
        U64 right_captures = ((pawns & ~FILE_BB[7]) >> 7) & enemy;
        
        // Process left captures
        U64 left_promotions = left_captures & RANK_BB[0];
        left_captures &= ~RANK_BB[0];
        
        while (left_captures) {
            const int to = ctz(left_captures);
            left_captures ^= (1ULL << to);
            const int captured = pos.board[to];
            AddCaptureMove(pos, to + 9, to, captured, 0, &move_list);
        }
        
        while (left_promotions) {
            const int to = ctz(left_promotions);
            left_promotions ^= (1ULL << to);
            const int from = to + 9;
            const int captured = pos.board[to];
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_QUEEN, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_ROOK, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_BISHOP, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_KNIGHT, &move_list);
        }
        
        // Process right captures
        U64 right_promotions = right_captures & RANK_BB[0];
        right_captures &= ~RANK_BB[0];
        
        while (right_captures) {
            const int to = ctz(right_captures);
            right_captures ^= (1ULL << to);
            const int captured = pos.board[to];
            AddCaptureMove(pos, to + 7, to, captured, 0, &move_list);
        }
        
        while (right_promotions) {
            const int to = ctz(right_promotions);
            right_promotions ^= (1ULL << to);
            const int from = to + 7;
            const int captured = pos.board[to];
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_QUEEN, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_ROOK, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_BISHOP, &move_list);
            AddCaptureMove(pos, from, to, captured, MOVE_FLAG_PROMOTION_KNIGHT, &move_list);
        }
        
        // En passant
        if (pos.en_passant != NO_SQ) {
            U64 ep_pawns = pawns & pawn_attacks[WHITE][pos.en_passant];
            while (ep_pawns) {
                const int from = ctz(ep_pawns);
                ep_pawns ^= (1ULL << from);
                AddEnPassantMove(pos, from, pos.en_passant, &move_list);
            }
        }
    }
}

} // namespace OptimizedBitboardMoveGen