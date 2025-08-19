#pragma once
#include <vector>
#include <algorithm>
#include "position.hpp"
#include "move.hpp"
#include "board120.hpp"

// Helper function to check if a pawn on pawn_sq attacks target_sq
inline bool pawn_attacks_square(int pawn_sq, int target_sq, Color pawn_color) {
    if (pawn_color == Color::White) {
        // White pawn attacks NE and NW
        return (target_sq == pawn_sq + NE || target_sq == pawn_sq + NW);
    } else {
        // Black pawn attacks SE and SW
        return (target_sq == pawn_sq + SE || target_sq == pawn_sq + SW);
    }
}

// Helper function to check if a knight on knight_sq attacks target_sq
inline bool knight_attacks_square(int knight_sq, int target_sq) {
    for (int delta : KNIGHT_DELTAS) {
        if (knight_sq + delta == target_sq) {
            return true;
        }
    }
    return false;
}

// Helper function to check if a king on king_sq attacks target_sq  
inline bool king_attacks_square(int king_sq, int target_sq) {
    for (int delta : KING_DELTAS) {
        if (king_sq + delta == target_sq) {
            return true;
        }
    }
    return false;
}

// Helper function to check if a sliding piece attacks along rank/file
inline bool sliding_attacks_rank_file(int piece_sq, int target_sq, const Position& pos) {
    // Check if on same rank or file
    File piece_file = file_of(piece_sq);
    Rank piece_rank = rank_of(piece_sq);
    File target_file = file_of(target_sq);
    Rank target_rank = rank_of(target_sq);
    
    if (piece_file != target_file && piece_rank != target_rank) {
        return false; // Not on same rank or file
    }
    
    // Determine direction
    int dir = 0;
    if (piece_file == target_file) {
        // Same file - check rank direction
        dir = (target_rank > piece_rank) ? NORTH : SOUTH;
    } else {
        // Same rank - check file direction  
        dir = (target_file > piece_file) ? EAST : WEST;
    }
    
    // Check for blocking pieces
    int current_sq = piece_sq + dir;
    while (current_sq != target_sq && is_playable(current_sq)) {
        if (!is_none(pos.at(current_sq))) {
            return false; // Blocked by piece
        }
        current_sq += dir;
    }
    
    return current_sq == target_sq;
}

// Helper function to check if a sliding piece attacks along diagonal
inline bool sliding_attacks_diagonal(int piece_sq, int target_sq, const Position& pos) {
    // Check if on same diagonal
    File piece_file = file_of(piece_sq);
    Rank piece_rank = rank_of(piece_sq);
    File target_file = file_of(target_sq);
    Rank target_rank = rank_of(target_sq);
    
    int file_diff = int(target_file) - int(piece_file);
    int rank_diff = int(target_rank) - int(piece_rank);
    
    if (abs(file_diff) != abs(rank_diff)) {
        return false; // Not on diagonal
    }
    
    // Determine direction
    int dir = 0;
    if (file_diff > 0 && rank_diff > 0) dir = NE;
    else if (file_diff < 0 && rank_diff > 0) dir = NW;
    else if (file_diff > 0 && rank_diff < 0) dir = SE;
    else if (file_diff < 0 && rank_diff < 0) dir = SW;
    
    // Check for blocking pieces
    int current_sq = piece_sq + dir;
    while (current_sq != target_sq && is_playable(current_sq)) {
        if (!is_none(pos.at(current_sq))) {
            return false; // Blocked by piece
        }
        current_sq += dir;
    }
    
    return current_sq == target_sq;
}

// Optimized SqAttacked using piece lists for much better performance
// sq: square to check (120-square index)
// pos: current position  
// attacking_color: which color's pieces we're checking for attacks
inline bool SqAttacked(int sq, const Position& pos, Color attacking_color) {
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
    for (int i = 0; i < pawn_count; ++i) {
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
    
    return false; // No attacks found
}

// Move list using enhanced S_MOVE structure with scoring
struct MoveList {
    std::vector<S_MOVE> v;
    void clear() { v.clear(); }
    void add(const S_MOVE& m) { v.push_back(m); }
    void add(int from, int to, PieceType captured = PieceType::None, 
             bool en_passant = false, bool pawn_start = false, 
             PieceType promoted = PieceType::None, bool castle = false) {
        v.emplace_back(from, to, captured, en_passant, pawn_start, promoted, castle);
    }
    size_t size() const { return v.size(); }
    S_MOVE& operator[](size_t i) { return v[i]; }
    const S_MOVE& operator[](size_t i) const { return v[i]; }
    
    // Move ordering functions
    void sort_by_score() {
        std::sort(v.begin(), v.end(), [](const S_MOVE& a, const S_MOVE& b) {
            return a.score > b.score; // Higher scores first
        });
    }
};

inline void generate_pseudo_legal_moves(const Position& pos, MoveList& out) {
    out.clear();
    // --- STARTER: knights only (example, white only) ---
    // Expand piece by piece using TDD.
    for (int r=0; r<8; ++r) {
        for (int f=0; f<8; ++f) {
            int s = sq(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = pos.at(s);
            if (is_none(p)) continue;
            if (color_of(p) != pos.side_to_move) continue;

            PieceType t = type_of(p);
            if (t == PieceType::Knight) {
                for (int d : KNIGHT_DELTAS) {
                    int to = s + d;
                    if (!is_playable(to)) continue;
                    Piece q = pos.at(to);
                    if (!is_none(q) && color_of(q) == pos.side_to_move) continue; // own piece
                    
                    // Check if this is a capture
                    PieceType captured = is_none(q) ? PieceType::None : type_of(q);
                    out.add(s, to, captured);
                }
            }
            // TODO: add other pieces via tests
        }
    }
}

// For now, treat pseudo-legal==legal until check logic is added.
inline void generate_legal_moves(const Position& pos, MoveList& out) {
    generate_pseudo_legal_moves(pos, out);
}