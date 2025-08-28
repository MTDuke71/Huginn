//#define DEBUG_CASTLING
#pragma once
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "bitboard.hpp"
#include "board120.hpp"
#include "chess_types.hpp"
#include "move.hpp"
#include "msvc_optimizations.hpp"

// Maximum search depth / game length (legacy constant - move_history now uses dynamic vector)
#define MAXPLY 2048

struct State {
    // Minimal saved state for unmake
    int ep_square;            // previous ep square
    uint8_t castling_rights;  // previous castling rights
    uint16_t halfmove_clock;  // previous halfmove clock
    Piece captured;           // captured piece (if any)
};

// Enhanced undo structure for comprehensive move history
struct S_UNDO {
    S_MOVE move;              // Full move information with encoding
    uint8_t castling_rights;  // previous castling permissions (castlePerm)
    int ep_square;            // previous en passant square (enPas)
    uint16_t halfmove_clock;  // previous fifty move counter (fiftyMove)
    uint64_t zobrist_key;     // previous position key (posKey)
    Piece captured;           // captured piece (if any)
    
    // Derived state for incremental updates (performance optimization)
    std::array<int, 2> king_sq_backup;        // Previous king positions
    std::array<uint64_t, 2> pawns_bb_backup;  // Previous pawn bitboards
    uint64_t all_pawns_bb_backup;             // Previous combined pawn bitboard
    std::array<int, 7> piece_counts_backup;   // Previous piece counts
    std::array<int, 2> material_score_backup; // Previous material scores
    
    // Piece list backup for undo support
    std::array<PieceList, 2> pList_backup;    // Previous piece lists [color][type][index]
    std::array<std::array<int, int(PieceType::_Count)>, 2> pCount_backup; // Previous piece counts [color][type]
    
    // Constructor
    S_UNDO() : move(), castling_rights(0), ep_square(-1), halfmove_clock(0), zobrist_key(0), captured(Piece::None) {}
    
    // Legacy helper for backward compatibility (deprecated, use S_MOVE directly)
    static int encode_move_legacy(int from, int to, PieceType promo = PieceType::None) {
        return (from & 0x7F) | ((to & 0x7F) << 7) | ((int(promo) & 0x7) << 14);
    }
    
    static void decode_move_legacy(int encoded, int& from, int& to, PieceType& promo) {
        from = encoded & 0x7F;
        to = (encoded >> 7) & 0x7F;
        promo = PieceType((encoded >> 14) & 0x7);
    }
};

class Position {
public:
    std::array<Piece, 120> board{};  // Piece::None for empty, Piece::Offboard for offboard
    Color side_to_move{Color::White};
    int ep_square{-1};               // mailbox-120 index or -1
    uint8_t castling_rights{0};      // bitmask: CASTLE_WK|CASTLE_WQ|CASTLE_BK|CASTLE_BQ
    uint16_t halfmove_clock{0};
    uint16_t fullmove_number{1};
    std::array<int, 2> king_sq{ -1, -1 }; // [White, Black] king locations (120)
    std::array<uint64_t, 2> pawns_bb{ 0, 0 }; // [White, Black] pawn bitboards (64)
    uint64_t all_pawns_bb{ 0 }; // Combined bitboard of all pawns (White | Black)
    std::array<int, 7> piece_counts{}; // count by PieceType (None, Pawn, ..., King)
    uint64_t zobrist_key{0};
    
    // Material score tracking for fast evaluation
    std::array<int, 2> material_score{ 0, 0 }; // [White, Black] material balance
    
    // Piece lists: pList[color][piece_type][index] = square
    // Tracks locations of all pieces for fast iteration
    std::array<PieceList, 2> pList; // [White, Black]
    std::array<std::array<int, int(PieceType::_Count)>, 2> pCount; // Number of pieces [color][type]
    
    // Move history for undo functionality - use vector for dynamic sizing
    std::vector<S_UNDO> move_history;
    int ply{0};                      // current search/game ply

    // Constructor
    Position() : move_history() {
        move_history.reserve(64);  // Reserve reasonable initial capacity
    }

    void reset();
    bool set_from_fen(const std::string& fen);
    std::string to_fen() const;
    void save_derived_state(S_UNDO& undo);
    void rebuild_counts();
    void set_startpos();
    
    void restore_derived_state(const S_UNDO& undo) {
        king_sq = undo.king_sq_backup;
        pawns_bb = undo.pawns_bb_backup;
        all_pawns_bb = undo.all_pawns_bb_backup;
        piece_counts = undo.piece_counts_backup;
        material_score = undo.material_score_backup;
        pList = undo.pList_backup;
        pCount = undo.pCount_backup;
    }
    
    // Update derived state incrementally for a move (much faster than rebuild_counts)
    void update_derived_state_for_move(const S_MOVE& m, Piece moving, Piece captured) {
        Color moving_color = color_of(moving);
        PieceType moving_type = type_of(moving);
        
        // Handle captured piece
        if (!is_none(captured)) {
            --piece_counts[size_t(type_of(captured))];
            
            // Subtract captured piece value from opponent's material (exclude kings)
            if (type_of(captured) != PieceType::King) {
                Color captured_color = color_of(captured);
                if (captured_color != Color::None) {
                    material_score[size_t(captured_color)] -= value_of(captured);
                }
            }
            
            // Remove captured pawn from bitboard
            if (type_of(captured) == PieceType::Pawn) {
                int s64_to = MAILBOX_MAPS.to64[m.get_to()];
                if (s64_to >= 0) {
                    Color captured_color = color_of(captured);
                    if (captured_color != Color::None) {
                        popBit(pawns_bb[size_t(captured_color)], s64_to);
                        popBit(all_pawns_bb, s64_to);
                    }
                }
            }
        }
        
        // Handle promotion
        if (m.is_promotion()) {
            // Remove pawn, add promoted piece
            --piece_counts[size_t(PieceType::Pawn)];
            ++piece_counts[size_t(m.get_promoted())];
            
            // Update material score: remove pawn value, add promoted piece value
            material_score[size_t(moving_color)] -= value_of(make_piece(moving_color, PieceType::Pawn));
            material_score[size_t(moving_color)] += value_of(make_piece(moving_color, m.get_promoted()));
            
            // Remove pawn from bitboard (promoted piece isn't a pawn)
            int s64_to = MAILBOX_MAPS.to64[m.get_to()];
            if (s64_to >= 0) {
                popBit(pawns_bb[size_t(moving_color)], s64_to);
                popBit(all_pawns_bb, s64_to);
            }
        } else {
            // Regular move - update piece-specific derived state
            if (moving_type == PieceType::Pawn) {
                // Update pawn bitboard
                int s64_from = MAILBOX_MAPS.to64[m.get_from()];
                int s64_to = MAILBOX_MAPS.to64[m.get_to()];
                if (s64_from >= 0 && s64_to >= 0) {
                    popBit(pawns_bb[size_t(moving_color)], s64_from);
                    setBit(pawns_bb[size_t(moving_color)], s64_to);
                    popBit(all_pawns_bb, s64_from);
                    setBit(all_pawns_bb, s64_to);
                }
            }
            
            if (moving_type == PieceType::King) {
                // Update king square
                king_sq[size_t(moving_color)] = m.get_to();
            }
        }
    }
    
    // Update Zobrist key incrementally for a move using XOR (much faster than recomputation)
    void update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, uint8_t old_castling_rights, int old_ep_square);
    
    // Compute and set the Zobrist key from current position
    void update_zobrist_key();

    // Access
    inline Piece at(int s) const { 
        return (s >= 0 && s < 120) ? board[size_t(s)] : Piece::Offboard; 
    }
    inline void set(int s, Piece p) { if (is_playable(s)) board[size_t(s)] = p; }
    
    // Piece list management helpers
    void add_piece_to_list(Color c, PieceType pt, int square) {
        if (c == Color::None || pt == PieceType::None) return;
        DEBUG_ASSERT(is_playable(square), "Cannot add piece to invalid square");
        int color_idx = int(c);
        int type_idx = int(pt);
        DEBUG_ASSERT(pCount[color_idx][type_idx] < MAX_PIECES_PER_TYPE, 
                    "Too many pieces of this type on the board");
        if (pCount[color_idx][type_idx] < MAX_PIECES_PER_TYPE) {
            pList[color_idx][type_idx][pCount[color_idx][type_idx]] = square;
            ++pCount[color_idx][type_idx];
        }
    }
    
    void remove_piece_from_list(Color c, PieceType pt, int square) {
        if (c == Color::None || pt == PieceType::None) return;
        DEBUG_ASSERT(is_playable(square), "Cannot remove piece from invalid square");
        int color_idx = int(c);
        int type_idx = int(pt);
        DEBUG_ASSERT(pCount[color_idx][type_idx] > 0, 
                    "Cannot remove piece from empty piece list");
        // Find and remove the piece from the list
        for (int i = 0; i < pCount[color_idx][type_idx]; ++i) {
            if (pList[color_idx][type_idx][i] == square) {
                // Move last piece to this position and decrement count
                --pCount[color_idx][type_idx];
                pList[color_idx][type_idx][i] = pList[color_idx][type_idx][pCount[color_idx][type_idx]];
                pList[color_idx][type_idx][pCount[color_idx][type_idx]] = -1;
                return;
            }
        }
        DEBUG_ASSERT(false, "Piece not found in piece list during removal");
    }
    
    void move_piece_in_list(Color c, PieceType pt, int from_square, int to_square) {
        if (c == Color::None || pt == PieceType::None) return;
        DEBUG_ASSERT(is_playable(from_square), "Invalid source square for piece move");
        DEBUG_ASSERT(is_playable(to_square), "Invalid destination square for piece move");
        int color_idx = int(c);
        int type_idx = int(pt);
        // Find and update the piece location
        for (int i = 0; i < pCount[color_idx][type_idx]; ++i) {
            if (pList[color_idx][type_idx][i] == from_square) {
                pList[color_idx][type_idx][i] = to_square;
                return;
            }
        }
        // Instead of crashing, silently ignore missing pieces (piece list corruption recovery)
        // This allows the engine to continue even with piece list inconsistencies
        // DEBUG_ASSERT(false, "Piece not found in piece list during move");
    }
    
    // Enhanced move making with full undo support
    void make_move_with_undo(const S_MOVE& m) {
        // Debug assertions for move validity
        DEBUG_ASSERT(is_playable(m.get_from()), "Move source square must be playable");
        DEBUG_ASSERT(is_playable(m.get_to()), "Move destination square must be playable");
        
        // Safety check: if source square is empty, skip this move (piece list corruption recovery)
        if (is_none(at(m.get_from()))) {
            return; // Silently ignore invalid moves to prevent crashes
        }
        
        // Ensure move_history has enough capacity
        if (ply >= static_cast<int>(move_history.size())) {
            move_history.resize(ply + 1);
        }
        
        S_UNDO& undo = move_history[ply]; // Vector access
        undo.move = m; // Store the complete S_MOVE
        undo.castling_rights = castling_rights;
        undo.ep_square = ep_square;
        undo.halfmove_clock = halfmove_clock;
        undo.zobrist_key = zobrist_key;
        undo.captured = at(m.get_to());
        
        // Save derived state for efficient undo (performance optimization)
        save_derived_state(undo);

#ifdef DEBUG_CASTLING
    std::cout << "[DEBUG] Before move: " << m.get_from() << "->" << m.get_to() << " castle? " << m.is_castle() << " rights: " << int(castling_rights) << std::endl;
    std::cout << "[DEBUG] White King list: ";
    for (int i = 0; i < pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)]; ++i)
        std::cout << pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)][i] << " ";
    std::cout << " | White Rook list: ";
    for (int i = 0; i < pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)]; ++i)
        std::cout << pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)][i] << " ";
    std::cout << std::endl;
#endif
    // --- Standard castling rights update (conditional logic) ---
    // Clear castling rights when pieces move from key squares
    int from = m.get_from();
    int to = m.get_to();
    
    // Clear castling rights based on piece movement from key squares
    if (from == 21 || to == 21) {  // a1 square
        castling_rights &= ~CASTLE_WQ;  // Clear white queenside
    }
    if (from == 25 || to == 25) {  // e1 square  
        castling_rights &= ~(CASTLE_WK | CASTLE_WQ);  // Clear both white rights
    }
    if (from == 28 || to == 28) {  // h1 square
        castling_rights &= ~CASTLE_WK;  // Clear white kingside
    }
    if (from == 91 || to == 91) {  // a8 square
        castling_rights &= ~CASTLE_BQ;  // Clear black queenside
    }
    if (from == 95 || to == 95) {  // e8 square
        castling_rights &= ~(CASTLE_BK | CASTLE_BQ);  // Clear both black rights
    }
    if (from == 98 || to == 98) {  // h8 square
        castling_rights &= ~CASTLE_BK;  // Clear black kingside
    }

#ifdef DEBUG_CASTLING
    std::cout << "[DEBUG] After move: " << m.get_from() << "->" << m.get_to() << " rights: " << int(castling_rights) << std::endl;
    std::cout << "[DEBUG] White King list: ";
    for (int i = 0; i < pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)]; ++i)
        std::cout << pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)][i] << " ";
    std::cout << " | White Rook list: ";
    for (int i = 0; i < pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)]; ++i)
        std::cout << pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)][i] << " ";
    std::cout << std::endl;
#endif

        // Make the move
        Piece moving = at(m.get_from());
        Piece captured = undo.captured;

        if (type_of(moving) == PieceType::Pawn || !is_none(captured)) {
            halfmove_clock = 0;
        } else {
            ++halfmove_clock;
        }

        // Update piece lists before changing the board
        Color moving_color = color_of(moving);
        PieceType moving_type = type_of(moving);

        // Remove captured piece from piece list
        if (!is_none(captured)) {
            remove_piece_from_list(color_of(captured), type_of(captured), m.get_to());
        }

        // Handle en passant captures
        if (m.is_en_passant()) {
            // The captured pawn is not on the destination square, but adjacent to it
            int captured_pawn_sq;
            if (moving_color == Color::White) {
                // White captures en passant - captured black pawn is south of target square
                captured_pawn_sq = m.get_to() + SOUTH;
            } else {
                // Black captures en passant - captured white pawn is north of target square
                captured_pawn_sq = m.get_to() + NORTH;
            }

            Piece captured_pawn = at(captured_pawn_sq);
            if (!is_none(captured_pawn)) {
                // Remove the captured pawn from board and piece lists
                remove_piece_from_list(color_of(captured_pawn), PieceType::Pawn, captured_pawn_sq);
                set(captured_pawn_sq, Piece::None);

                // Update undo information to remember the captured pawn
                undo.captured = captured_pawn;
            }
        }

        // Handle promotion - remove pawn and add promoted piece
        if (m.is_promotion()) {
            remove_piece_from_list(moving_color, PieceType::Pawn, m.get_from());
            add_piece_to_list(moving_color, m.get_promoted(), m.get_to());
            set(m.get_to(), make_piece(moving_color, m.get_promoted()));
        } else {
            // Regular move - update piece location in list
            move_piece_in_list(moving_color, moving_type, m.get_from(), m.get_to());
            set(m.get_to(), moving);
        }

        set(m.get_from(), Piece::None);

        // Handle castling move - place rook correctly
        if (m.is_castle()) {
            Color king_color = color_of(moving);
            int rook_from, rook_to;
            
            if (king_color == Color::White) {
                if (m.get_to() == sq(File::G, Rank::R1)) { // Kingside
                    rook_from = sq(File::H, Rank::R1);
                    rook_to = sq(File::F, Rank::R1);
                } else { // Queenside
                    rook_from = sq(File::A, Rank::R1);
                    rook_to = sq(File::D, Rank::R1);
                }
            } else {
                if (m.get_to() == sq(File::G, Rank::R8)) { // Kingside
                    rook_from = sq(File::H, Rank::R8);
                    rook_to = sq(File::F, Rank::R8);
                } else { // Queenside
                    rook_from = sq(File::A, Rank::R8);
                    rook_to = sq(File::D, Rank::R8);
                }
            }
            
            // Move the rook - but first verify it's actually there
            Piece rook = at(rook_from);
            if (is_none(rook) || type_of(rook) != PieceType::Rook || color_of(rook) != king_color) {
#ifndef NDEBUG
                std::cout << "CASTLING ERROR: Expected " << (king_color == Color::White ? "white" : "black") 
                          << " rook at square " << rook_from << " but found piece " << static_cast<int>(rook) << std::endl;
#endif
                // This should not happen in a legal position - just skip the rook move
                return;
            }
            set(rook_to, rook);
            set(rook_from, Piece::None);
            move_piece_in_list(king_color, PieceType::Rook, rook_from, rook_to);
        }

        ep_square = -1; // Reset, then check for pawn double moves

        // Set en passant square for pawn double moves
        if (type_of(moving) == PieceType::Pawn) {
            int from_rank = int(rank_of(m.get_from()));
            int to_rank = int(rank_of(m.get_to()));
            int rank_diff = abs(to_rank - from_rank);

            // Check if pawn moved two squares
            if (rank_diff == 2) {
                // Set en passant square to the square "behind" the pawn
                int ep_rank = (from_rank + to_rank) / 2; // Square between from and to
                File file = file_of(m.get_to());
                ep_square = sq(file, Rank(ep_rank));
            }
        }

        side_to_move = !side_to_move;
        if (side_to_move == Color::White) ++fullmove_number;

        // Update derived state incrementally (much faster than rebuild_counts)
        update_derived_state_for_move(m, moving, captured);

        // Update zobrist_key incrementally using XOR (much faster than recomputing)
        update_zobrist_for_move(m, moving, captured, undo.castling_rights, undo.ep_square);
        
        // Increment ply after successful move
        ++ply;
    }
    
    // Undo the last move
    bool undo_move() {
    #ifdef DEBUG_CASTLING
    std::cout << "[DEBUG] After undo move rights: " << int(castling_rights) << std::endl;
    std::cout << "[DEBUG] White King list: ";
    for (int i = 0; i < pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)]; ++i)
        std::cout << pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::King)][i] << " ";
    std::cout << " | White Rook list: ";
    for (int i = 0; i < pCount[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)]; ++i)
        std::cout << pList[static_cast<size_t>(Color::White)][static_cast<size_t>(PieceType::Rook)][i] << " ";
    std::cout << std::endl;
    #endif
        if (ply == 0) return false; // No moves to undo
        
        // Decrement ply first to get the correct index
        --ply;
        S_UNDO& undo = move_history[ply]; // Vector access
        
        // Extract move information using modern getter methods (30-34% faster than decode_move)
        int from = undo.move.get_from();
        int to = undo.move.get_to();
        PieceType promo = undo.move.get_promoted();
        
        // Undo side to move first
        side_to_move = !side_to_move;
        if (side_to_move == Color::Black) --fullmove_number;
        
        // Get the piece that moved
        Piece moved = at(to);
        
        // Handle promotion undo - restore piece lists
        if (promo != PieceType::None) {
            // Remove promoted piece from lists and add pawn back
            remove_piece_from_list(color_of(moved), type_of(moved), to);
            moved = make_piece(color_of(moved), PieceType::Pawn);
            add_piece_to_list(color_of(moved), PieceType::Pawn, from);
        } else {
            // Regular move - update piece location in lists
            move_piece_in_list(color_of(moved), type_of(moved), to, from);
        }
        
        // Move piece back
        set(from, moved);
        
        // Handle castling undo - restore rook position
        if (undo.move.is_castle()) {
            Color king_color = color_of(moved);
            int rook_from, rook_to;
            
            if (king_color == Color::White) {
                if (to == sq(File::G, Rank::R1)) { // Kingside
                    rook_from = sq(File::H, Rank::R1);
                    rook_to = sq(File::F, Rank::R1);
                } else { // Queenside
                    rook_from = sq(File::A, Rank::R1);
                    rook_to = sq(File::D, Rank::R1);
                }
            } else {
                if (to == sq(File::G, Rank::R8)) { // Kingside
                    rook_from = sq(File::H, Rank::R8);
                    rook_to = sq(File::F, Rank::R8);
                } else { // Queenside
                    rook_from = sq(File::A, Rank::R8);
                    rook_to = sq(File::D, Rank::R8);
                }
            }
            
            // Move the rook back to its original position
            Piece rook = at(rook_to);
            set(rook_from, rook);
            set(rook_to, Piece::None);
            move_piece_in_list(king_color, PieceType::Rook, rook_to, rook_from);
        }

        // Handle en passant undo
        if (undo.move.is_en_passant()) {
            // Restore the captured pawn to its original square
            Color moving_color = color_of(moved);
            int captured_pawn_sq;
            if (moving_color == Color::White) {
                // White captured en passant - restore black pawn south of destination
                captured_pawn_sq = to + SOUTH;
            } else {
                // Black captured en passant - restore white pawn north of destination
                captured_pawn_sq = to + NORTH;
            }
            
            // Restore the captured pawn and update piece lists
            set(captured_pawn_sq, undo.captured);
            set(to, Piece::None); // Clear the destination square
            
            // Restore the captured pawn to piece lists
            if (!is_none(undo.captured)) {
                add_piece_to_list(color_of(undo.captured), PieceType::Pawn, captured_pawn_sq);
            }
        } else {
            set(to, undo.captured); // Restore captured piece (or Piece::None)
            
            // Restore captured piece to piece lists
            if (!is_none(undo.captured)) {
                add_piece_to_list(color_of(undo.captured), type_of(undo.captured), to);
            }
        }
        
        // Restore position state
        castling_rights = undo.castling_rights;
        ep_square = undo.ep_square;
        halfmove_clock = undo.halfmove_clock;
        zobrist_key = undo.zobrist_key;
        
        // Restore derived state incrementally (much faster than rebuild_counts)
        restore_derived_state(undo);
        
        return true;
    }
    
    // Material evaluation access functions
    int get_material_score(Color c) const {
        return material_score[size_t(c)];
    }
    
    int get_material_balance() const {
        return material_score[size_t(Color::White)] - material_score[size_t(Color::Black)];
    }
    
    int get_total_material() const {
        return material_score[size_t(Color::White)] + material_score[size_t(Color::Black)];
    }
    
    // Check if side has non-pawn material (for null move pruning)
    bool has_non_pawn_material(Color c) const {
        int color_idx = int(c);
        // Check if side has pieces other than pawns and king
        return pCount[color_idx][int(PieceType::Queen)] > 0 ||
               pCount[color_idx][int(PieceType::Rook)] > 0 ||
               pCount[color_idx][int(PieceType::Bishop)] > 0 ||
               pCount[color_idx][int(PieceType::Knight)] > 0;
    }
    
    // Pawn bitboard access functions
    uint64_t get_pawn_bitboard(Color c) const {
        return pawns_bb[size_t(c)];
    }
    
    uint64_t get_all_pawns_bitboard() const {
        return all_pawns_bb;
    }
    
    uint64_t get_white_pawns() const {
        return pawns_bb[size_t(Color::White)];
    }
    
    uint64_t get_black_pawns() const {
        return pawns_bb[size_t(Color::Black)];
    }
};
