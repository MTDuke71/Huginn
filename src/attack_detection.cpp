/**
 * @brief Determines if a given square is attacked by any piece of the specified color.
 *
 * This function checks whether the square `sq` is attacked by any piece of `attacking_color`
 * in the given `pos` (Position). It uses optimized piece lists if available for performance,
 * but falls back to a full board scan if the piece lists are not maintained (e.g., after
 * direct board setup).
 *
 * The function checks for attacks from all piece types:
 *   - Pawns: Checks pawn attack patterns based on color.
 *   - Knights: Checks all possible knight jumps.
 *   - Kings: Checks adjacent squares for king attacks.
 *   - Rooks and Queens: Checks rank and file sliding attacks.
 *   - Bishops and Queens: Checks diagonal sliding attacks.
 *
 * The function first determines if piece lists are available and valid. If not, it performs
 * a direct scan of the board for attackers. Otherwise, it uses the piece lists for each type
 * for efficient attack detection.
 *
 * @param sq The square to check for attacks (typically 0..119 in mailbox representation).
 * @param pos The current board position.
 * @param attacking_color The color of the potential attackers (Color::White or Color::Black).
 * @return true if the square is attacked by any piece of the given color, false otherwise.
 */
#include "attack_detection.hpp"

// Main attack detection function moved from header for better compilation times
bool SqAttacked(int sq, const Position& pos, Color attacking_color) {
    if (!is_playable(sq)) return false;
    
    int color_idx = int(attacking_color);
    
    // Quick check: if we have any pieces in the lists for this color, assume lists are maintained
    // This is much faster than scanning the entire board for consistency
    bool has_pieces_in_lists = false;
    for (int type = 0; type < int(PieceType::_Count); ++type) {
        if (pos.pCount[color_idx][type] > 0) {
            has_pieces_in_lists = true;
            break;
        }
    }
    
    // If no pieces in lists but we should have pieces (like when using pos.set() directly),
    // fall back to board scanning
    if (!has_pieces_in_lists) {
        // Quick scan to see if there are actually pieces of this color on the board
        bool has_pieces_on_board = false;
        for (int i = 0; i < 120 && !has_pieces_on_board; ++i) {
            if (is_playable(i)) {
                Piece p = pos.at(i);
                if (!is_none(p) && color_of(p) == attacking_color) {
                    has_pieces_on_board = true;
                }
            }
        }
        
        // If we have pieces on board but not in lists, use fallback
        if (has_pieces_on_board) {
            // Fallback: Original board-scanning implementation
            // Check pawn attacks
            if (attacking_color == Color::White) {
                // White pawns attack diagonally "upward" (from lower ranks)
                // So check squares below the target square for white pawns
                int pawn_sq1 = sq + pawn_capt_left_black();  // SE from target = where white pawn would be
                int pawn_sq2 = sq + pawn_capt_right_black(); // SW from target = where white pawn would be
                
                if (is_playable(pawn_sq1)) {
                    Piece p1 = pos.at(pawn_sq1);
                    if (p1 == Piece::WhitePawn) return true;
                }
                if (is_playable(pawn_sq2)) {
                    Piece p2 = pos.at(pawn_sq2);
                    if (p2 == Piece::WhitePawn) return true;
                }
            } else {
                // Black pawns attack diagonally "downward" (from higher ranks)
                // So check squares above the target square for black pawns
                int pawn_sq1 = sq + pawn_capt_left_white();  // NW from target = where black pawn would be
                int pawn_sq2 = sq + pawn_capt_right_white(); // NE from target = where black pawn would be
                
                if (is_playable(pawn_sq1)) {
                    Piece p1 = pos.at(pawn_sq1);
                    if (p1 == Piece::BlackPawn) return true;
                }
                if (is_playable(pawn_sq2)) {
                    Piece p2 = pos.at(pawn_sq2);
                    if (p2 == Piece::BlackPawn) return true;
                }
            }
            
            // Check knight attacks
            for (int delta : KNIGHT_DELTAS) {
                int knight_sq = sq + delta;
                if (is_playable(knight_sq)) {
                    Piece p = pos.at(knight_sq);
                    if (type_of(p) == PieceType::Knight && color_of(p) == attacking_color) {
                        return true;
                    }
                }
            }
            
            // Check king attacks
            for (int delta : KING_DELTAS) {
                int king_sq = sq + delta;
                if (is_playable(king_sq)) {
                    Piece p = pos.at(king_sq);
                    if (type_of(p) == PieceType::King && color_of(p) == attacking_color) {
                        return true;
                    }
                }
            }
            
            // Check ranks and files for rook/queen attacks
            constexpr int rank_file_dirs[4] = { NORTH, SOUTH, EAST, WEST };
            for (int dir : rank_file_dirs) {
                for (int i = 1; i < 8; ++i) { // Max 7 squares in any direction
                    int target_sq = sq + i * dir;
                    if (!is_playable(target_sq)) break; // Hit edge of board
                    
                    Piece p = pos.at(target_sq);
                    if (!is_none(p)) {
                        // Found a piece - check if it's an attacking rook or queen
                        if (color_of(p) == attacking_color) {
                            PieceType pt = type_of(p);
                            if (pt == PieceType::Rook || pt == PieceType::Queen) {
                                return true;
                            }
                        }
                        break; // Stop looking in this direction (piece blocks further attacks)
                    }
                }
            }
            
            // Check diagonals for bishop/queen attacks
            constexpr int diagonal_dirs[4] = { NE, NW, SE, SW };
            for (int dir : diagonal_dirs) {
                for (int i = 1; i < 8; ++i) { // Max 7 squares in any direction
                    int target_sq = sq + i * dir;
                    if (!is_playable(target_sq)) break; // Hit edge of board
                    
                    Piece p = pos.at(target_sq);
                    if (!is_none(p)) {
                        // Found a piece - check if it's an attacking bishop or queen
                        if (color_of(p) == attacking_color) {
                            PieceType pt = type_of(p);
                            if (pt == PieceType::Bishop || pt == PieceType::Queen) {
                                return true;
                            }
                        }
                        break; // Stop looking in this direction (piece blocks further attacks)
                    }
                }
            }
            
            return false; // No attacks found in fallback mode
        }
        
        // No pieces of this color, so no attacks
        return false;
    }
    
    // Optimized path: piece lists are maintained, use them for better performance
    
    // 1. Check pawns using piece list (most common attackers)
    int pawn_count = pos.pCount[color_idx][int(PieceType::Pawn)];
    // 1. Check pawns using piece list (most common attackers; this order is intentional for performance and matches typical chess attack patterns)
        int pawn_sq = pos.pList[color_idx][int(PieceType::Pawn)][i];
        if (pawn_attacks_square(pawn_sq, sq, attacking_color)) {
            return true;
        }
    }
    
    // 2. Check knights using piece list
    int knight_count = pos.pCount[color_idx][int(PieceType::Knight)];
    for (int i = 0; i < knight_count; ++i) {
        int knight_sq = pos.pList[color_idx][int(PieceType::Knight)][i];
        if (knight_attacks_square(knight_sq, sq)) {
            return true;
        }
    }
    
    // 3. Check king using piece list (always exactly 1)
    if (pos.king_sq[color_idx] >= 0) {
        if (king_attacks_square(pos.king_sq[color_idx], sq)) {
            return true;
        }
    }
    
    // 4. Check rooks using piece list for rank/file attacks
    int rook_count = pos.pCount[color_idx][int(PieceType::Rook)];
    for (int i = 0; i < rook_count; ++i) {
        int rook_sq = pos.pList[color_idx][int(PieceType::Rook)][i];
        if (sliding_attacks_rank_file(rook_sq, sq, pos)) {
            return true;
        }
    }
    
    // 5. Check bishops using piece list for diagonal attacks
    int bishop_count = pos.pCount[color_idx][int(PieceType::Bishop)];
    for (int i = 0; i < bishop_count; ++i) {
        int bishop_sq = pos.pList[color_idx][int(PieceType::Bishop)][i];
        if (sliding_attacks_diagonal(bishop_sq, sq, pos)) {
            return true;
        }
    }
    
    // 6. Check queens using piece list for both rank/file and diagonal attacks
    int queen_count = pos.pCount[color_idx][int(PieceType::Queen)];
    for (int i = 0; i < queen_count; ++i) {
        int queen_sq = pos.pList[color_idx][int(PieceType::Queen)][i];
        if (sliding_attacks_rank_file(queen_sq, sq, pos) || 
            sliding_attacks_diagonal(queen_sq, sq, pos)) {
            return true;
        }
    }
    // No attacks found in optimized mode
    return false;
}
}
