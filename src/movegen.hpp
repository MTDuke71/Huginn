#pragma once
/**
 * @file movegen.hpp
 * @brief Legacy move-list facade over the production bitboard move generator.
 *
 * The engine's actual pseudo-legal generation lives in BitboardMoveGen
 * (movegen_bb.hpp). This header keeps the VICE-style S_MOVELIST interface used
 * by search, tests, and UCI helpers, adding move-ordering scores and optional
 * legality filtering around the bitboard core.
 */

#include "position.hpp"
#include "move.hpp"
#include "chess_types.hpp"
#include "square.hpp"
#include "attack_detection.hpp"  // For Huginn::SqAttacked function
#include "msvc_optimizations.hpp"
#include <algorithm>
#include <vector>

/// Maximum number of pseudo-legal moves Huginn stores for one position.
#define MAX_POSITION_MOVES 256

/**
 * @brief Fixed-capacity move list with search-ordering scores.
 *
 * S_MOVELIST is intentionally stack-allocated and cache-friendly. The bitboard
 * generator appends pseudo-legal moves into it, and search later sorts or filters
 * the same entries. The add_* helpers assign the coarse ordering bands used by
 * alpha-beta: promotions, captures, castles, then quiet moves.
 */
struct S_MOVELIST {
    S_MOVE moves[MAX_POSITION_MOVES];  ///< Fixed-size storage for generated moves.
    int count;                         ///< Number of valid entries in moves[].
    
    /// Constructs an empty move list.
    S_MOVELIST() : count(0) {}
    
    /// Clears the list without touching existing storage.
    void clear() { count = 0; }

    /**
     * @brief Capacity gate shared by every append helper (BACKLOG #55).
     *
     * Legal chess tops out around 218 pseudo-legal moves, but the generator
     * also runs on composed/adversarial positions the FEN boundary accepts
     * (a board of 47 queens generates 279). The old
     * `__assume(count < MAX_POSITION_MOVES)` was a promise to the optimizer,
     * not a check — overflow wrote past the array (stack corruption; explicit
     * UB on GCC). Fail closed instead: assert diagnostically in debug builds,
     * drop the move and keep the list valid in release builds.
     * @return true if the list is full and the append must be dropped.
     */
    FORCE_INLINE bool full() const {
        if (count < MAX_POSITION_MOVES) [[likely]] {
            return false;
        }
        DEBUG_ASSERT(false, "S_MOVELIST overflow: dropping move (BACKLOG #55)");
        return true;
    }

    /**
     * @brief Appends a quiet move with the base quiet ordering score.
     * @param move Non-capture, non-promotion move to append.
     */
    FORCE_INLINE void add_quiet_move(const S_MOVE& move) {
        if (full()) return;
        moves[count] = move;
        moves[count].score = 0;  // Quiet moves get base score
        count++;
    }

    /**
     * @brief Appends a capture and assigns MVV-LVA ordering score.
     * @param move Capture move to append.
     * @param pos Pre-move position, used to identify the attacking piece.
     */
    FORCE_INLINE void add_capture_move(const S_MOVE& move, const Position& pos) {
        if (full()) return;
        moves[count] = move;
        // MVV-LVA scoring: Most Valuable Victim - Least Valuable Attacker
        Piece victim_piece = make_piece(!pos.side_to_move, move.get_captured());
        Piece attacker_piece = pos.at_sq64(move.get_from());
        moves[count].score = 1000000 + (10 * value_of(victim_piece)) - value_of(attacker_piece);
        count++;
    }

    /**
     * @brief Appends an en-passant capture in the capture ordering band.
     * @param move En-passant move to append.
     */
    FORCE_INLINE void add_en_passant_move(const S_MOVE& move) {
        if (full()) return;
        moves[count] = move;
        moves[count].score = 1000105;  // En passant gets high priority (captures pawn)
        count++;
    }

    /**
     * @brief Appends an already-scored move, preserving its ordering score.
     * @param move Move whose score was assigned by a previous generator pass.
     *
     * Used by the filter passes (captures-only, legality) that copy from
     * another S_MOVELIST and must keep the source scores intact.
     */
    FORCE_INLINE void add_scored_move(const S_MOVE& move) {
        if (full()) return;
        moves[count] = move;
        count++;
    }

    /**
     * @brief Appends a promotion with a promotion-piece-based ordering score.
     * @param move Promotion move to append; captures receive an extra bonus.
     */
    FORCE_INLINE void add_promotion_move(const S_MOVE& move) {
        if (full()) return;
        moves[count] = move;
        // Promotion scoring based on promoted piece value
        Piece promo_piece = make_piece(Color::White, move.get_promoted());  // Color doesn't matter for value
        int promo_bonus = value_of(promo_piece) * 100;
        int capture_bonus = 0;
        if (move.is_capture()) [[likely]] {
            Piece captured_piece = make_piece(Color::White, move.get_captured());  // Color doesn't matter for value
            capture_bonus = value_of(captured_piece) * 10;
        }
        moves[count].score = 2000000 + promo_bonus + capture_bonus;
        count++;
    }
    
    /**
     * @brief Appends a castling move in the quiet-but-useful ordering band.
     * @param move Castling move to append.
     */
    FORCE_INLINE void add_castle_move(const S_MOVE& move) {
        if (full()) return;
        moves[count] = move;
        moves[count].score = 50000;  // Castling gets moderate priority
        count++;
    }
    
    /// Sorts moves in descending ordering-score order.
    void sort_by_score() {
        std::sort(moves, moves + count, [](const S_MOVE& a, const S_MOVE& b) {
            return a.score > b.score;
        });
    }
    
    /// Mutable indexed access to a generated move.
    S_MOVE& operator[](int index) { return moves[index]; }
    /// Const indexed access to a generated move.
    const S_MOVE& operator[](int index) const { return moves[index]; }
    
    /// Returns the number of valid generated moves.
    int size() const { return count; }
    
    /// Iterator to the first generated move.
    S_MOVE* begin() { return moves; }
    /// Iterator one past the last generated move.
    S_MOVE* end() { return moves + count; }
    /// Const iterator to the first generated move.
    const S_MOVE* begin() const { return moves; }
    /// Const iterator one past the last generated move.
    const S_MOVE* end() const { return moves + count; }
};

/**
 * @brief Generates all pseudo-legal moves for the side to move.
 * @param pos Position to generate from; not modified.
 * @param[out] list Destination move list, overwritten with generated moves.
 *
 * Delegates to BitboardMoveGen. The result can include moves that leave the own
 * king in check; callers that need legal moves should use generate_legal_moves()
 * or validate via Position::MakeMove().
 */
void generate_all_moves(const Position& pos, S_MOVELIST& list);

/**
 * @brief Generates legal moves by filtering pseudo-legal moves with MakeMove.
 * @param pos Position to generate from; temporarily mutated and restored.
 * @param[out] list Destination move list, overwritten with legal moves.
 *
 * Capture/quiet classification is preserved so MVV-LVA scores from the bitboard
 * generator survive the legality pass.
 */
void generate_legal_moves(Position& pos, S_MOVELIST& list);

/**
 * @brief Generates pseudo-legal captures only for quiescence search.
 * @param pos Position to generate from; not modified.
 * @param[out] list Destination list containing only capture moves.
 *
 * This is a lazy filter: it does not MakeMove/TakeMove. Quiescence must still
 * reject illegal captures with Position::MakeMove() before recursing. Captures
 * keep their pre-move MVV-LVA score, which is more accurate than re-scoring on
 * the post-move board where the attacker has already left its source square.
 */
void generate_all_caps_pseudo(const Position& pos, S_MOVELIST& list);

/**
 * @brief Generates the pseudo-legal tactical frontier for quiescence search:
 *        captures plus quiet (non-capturing) promotions. (BACKLOG #52)
 * @param pos Position to generate from; not modified.
 * @param[out] list Destination list of captures and promotions.
 *
 * Same lazy-legality contract as generate_all_caps_pseudo(): no
 * MakeMove/TakeMove here, the caller must reject illegal moves.
 */
void generate_tactical_pseudo(const Position& pos, S_MOVELIST& list);

/**
 * @brief Tests whether the side to move is currently in check.
 * @param pos Position to inspect.
 * @return true if the side-to-move king exists and is attacked by the opponent.
 */
inline bool in_check(const Position& pos) {
    Color current_color = pos.side_to_move;
    int king_sq = pos.king_sq[int(current_color)];
    if (king_sq < 0) return false; // No king (shouldn't happen in valid position)
    
    // Check if the king is attacked by the opponent
    Color opponent_color = (current_color == Color::White) ? Color::Black : Color::White;
    return Huginn::SqAttackedBB(king_sq, pos, opponent_color);
}

/**
 * @brief Tests one pseudo-legal move for king safety.
 * @param pos Position to test from; temporarily mutated and restored.
 * @param move Pseudo-legal move to validate.
 * @return true if the move is legal and leaves the moving side's king safe.
 *
 * Castling is checked explicitly for "from", transit, and destination attacks.
 * Other moves are delegated to Position::MakeMove()/TakeMove().
 */
inline bool is_legal_move(Position& pos, const S_MOVE& move) {
    // Special handling for castling
    if (move.is_castle()) {
        int from = move.get_from();
        int to = move.get_to();
        Color current_side = pos.side_to_move;
        Color opponent_side = (current_side == Color::White) ? Color::Black : Color::White;
        
        // King cannot be in check before castling
        if (Huginn::SqAttackedBB(from, pos, opponent_side)) {
            return false;
        }

        // Check that king doesn't pass through attacked squares during castling.
        // from/to are sq64; castling is same-rank so ±1 stepping stays contiguous
        // (rank-1 = sq64 0..7, rank-8 = 56..63), matching the old 120 behaviour.
        int step = (to > from) ? 1 : -1;
        for (int sq = from + step; sq != to + step; sq += step) {
            if (Huginn::SqAttackedBB(sq, pos, opponent_side)) {
                return false;
            }
        }
        return true;
    }
    
    // For all other moves, use the proper move/undo system
    Color current_side = pos.side_to_move;
    
    // Apply the move using VICE method
    if (pos.MakeMove(move) != 1) {
        return false;  // Move was illegal
    }
    
    // Get the king position after the move (for the side that just moved)
    int king_sq = pos.king_sq[int(current_side)];
    
    // Check if our king would be in check after the move
    // Note: after the move, it's the opponent's turn, so we check if opponent attacks our king
    bool legal = !Huginn::SqAttackedBB(king_sq, pos, !current_side);
    
    // Undo the move to restore the original position
    pos.TakeMove();
    
    return legal;
}
