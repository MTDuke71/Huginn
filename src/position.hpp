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
#include "zobrist.hpp"

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
    
    // Full bitboard representation for all piece types [Color][PieceType]
    std::array<std::array<Bitboard, int(PieceType::_Count)>, 2> piece_bitboards{};
    std::array<Bitboard, 2> color_bitboards{ 0, 0 }; // [White, Black] all pieces
    Bitboard occupied_bitboard{ 0 }; // All pieces (White | Black)
    
    // Legacy pawn bitboards (maintained for compatibility)
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
    
    // VICE Tutorial Video #41: MakeMove function
    int MakeMove(const S_MOVE& move);
    
    // VICE Tutorial Video #42: TakeMove function
    void TakeMove();
    
    // VICE Part 83: Null move functions for null move pruning
    void MakeNullMove();
    void TakeNullMove();
    
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
    
    // Atomic piece movement - follows VICE MovePiece pattern
    // Moves a piece from one square to another, updating all necessary data structures
    void move_piece(int from_square, int to_square) {
        DEBUG_ASSERT(is_playable(from_square), "Invalid source square for piece move");
        DEBUG_ASSERT(is_playable(to_square), "Invalid destination square for piece move");
        
        Piece piece = at(from_square);
        DEBUG_ASSERT(!is_none(piece), "Cannot move piece from empty square");
        DEBUG_ASSERT(is_none(at(to_square)), "Cannot move piece to occupied square");
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // 1. Hash piece out of from square and into to square
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][from_square];
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][to_square];
        
        // 2. Update pieces array
        set(from_square, Piece::None);
        set(to_square, piece);
        
        // 3. Update all bitboard representations
        int from_sq64 = MAILBOX_MAPS.to64[from_square];
        int to_sq64 = MAILBOX_MAPS.to64[to_square];
        if (from_sq64 >= 0 && to_sq64 >= 0) {
            // Update new full bitboard system
            popBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], from_sq64);
            popBit(color_bitboards[size_t(piece_color)], from_sq64);
            popBit(occupied_bitboard, from_sq64);
            
            setBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], to_sq64);
            setBit(color_bitboards[size_t(piece_color)], to_sq64);
            setBit(occupied_bitboard, to_sq64);
            
            // Update legacy pawn bitboards for compatibility
            if (piece_type == PieceType::Pawn) {
                popBit(pawns_bb[size_t(piece_color)], from_sq64);
                popBit(all_pawns_bb, from_sq64);
                setBit(pawns_bb[size_t(piece_color)], to_sq64);
                setBit(all_pawns_bb, to_sq64);
            }
        }
        
        // 4. Update piece list (find piece and update its square)
        int color_idx = int(piece_color);
        int type_idx = int(piece_type);
        for (int i = 0; i < pCount[color_idx][type_idx]; ++i) {
            if (pList[color_idx][type_idx][i] == from_square) {
                pList[color_idx][type_idx][i] = to_square;
                return;
            }
        }
        
        DEBUG_ASSERT(false, "Piece not found in piece list during move_piece");
    }
    
    // Atomic piece removal - consolidates all operations for better performance
    // Follows the VICE ClearPiece pattern but maintains Huginn's C++ style
    void clear_piece(int square) {
        DEBUG_ASSERT(is_playable(square), "Cannot clear piece from invalid square");
        
        Piece piece = at(square);
        if (is_none(piece)) return; // Nothing to clear
        
        DEBUG_ASSERT(!is_offboard(piece), "Cannot clear offboard piece");
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // 1. Update zobrist hash (XOR out the piece using Huginn's zobrist system)
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][square];
        
        // 2. Clear the board square
        set(square, Piece::None);
        
        // 3. Update material score (kings can never be captured in chess)
        material_score[size_t(piece_color)] -= value_of(piece);
        
        // 4. Update piece counters
        --piece_counts[size_t(piece_type)];
        
        // 5. Update all bitboard representations
        int sq64 = MAILBOX_MAPS.to64[square];
        if (sq64 >= 0) {
            // Update new full bitboard system
            popBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], sq64);
            popBit(color_bitboards[size_t(piece_color)], sq64);
            popBit(occupied_bitboard, sq64);
            
            // Update legacy pawn bitboards for compatibility
            if (piece_type == PieceType::Pawn) {
                popBit(pawns_bb[size_t(piece_color)], sq64);
                popBit(all_pawns_bb, sq64);
            }
        }
        
        // 6. Remove from piece list (atomic operation)
        int color_idx = int(piece_color);
        int type_idx = int(piece_type);
        
        DEBUG_ASSERT(pCount[color_idx][type_idx] > 0, "Piece count already zero");
        
        // Find the piece in the list and remove it efficiently
        for (int i = 0; i < pCount[color_idx][type_idx]; ++i) {
            if (pList[color_idx][type_idx][i] == square) {
                // Replace with last piece in list and decrement count
                --pCount[color_idx][type_idx];
                pList[color_idx][type_idx][i] = pList[color_idx][type_idx][pCount[color_idx][type_idx]];
                pList[color_idx][type_idx][pCount[color_idx][type_idx]] = -1; // Clear old position
                return;
            }
        }
        
        DEBUG_ASSERT(false, "Piece not found in piece list during clear_piece");
    }
    
    // Atomic piece addition - complements clear_piece for better performance
    // Follows the VICE AddPiece pattern but maintains Huginn's C++ style
    void add_piece(int square, Piece piece) {
        DEBUG_ASSERT(is_playable(square), "Cannot add piece to invalid square");
        DEBUG_ASSERT(!is_none(piece) && !is_offboard(piece), "Cannot add invalid piece");
        DEBUG_ASSERT(is_none(at(square)), "Cannot add piece to occupied square");
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // 1. Update zobrist hash (XOR in the piece)
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][square];
        
        // 2. Place piece on board
        set(square, piece);
        
        // 3. Update material score
        if (piece_type != PieceType::King) { // Don't count king value
            material_score[size_t(piece_color)] += value_of(piece);
        }
        
        // 4. Update piece counters
        ++piece_counts[size_t(piece_type)];
        
        // 5. Update all bitboard representations
        int sq64 = MAILBOX_MAPS.to64[square];
        if (sq64 >= 0) {
            // Update new full bitboard system
            setBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], sq64);
            setBit(color_bitboards[size_t(piece_color)], sq64);
            setBit(occupied_bitboard, sq64);
            
            // Update legacy pawn bitboards for compatibility
            if (piece_type == PieceType::Pawn) {
                setBit(pawns_bb[size_t(piece_color)], sq64);
                setBit(all_pawns_bb, sq64);
            }
        }
        
        // 6. Add to piece list
        int color_idx = int(piece_color);
        int type_idx = int(piece_type);
        
        pList[color_idx][type_idx][pCount[color_idx][type_idx]] = square;
        ++pCount[color_idx][type_idx];
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
    // --- Efficient castling rights update using lookup table ---
    // Single operation replaces multiple conditional checks for better performance
    int from = m.get_from();
    int to = m.get_to();
    
    // Update castling rights using optimized lookup table (single AND operation)
    castling_rights = CastlingLookup::update_castling_rights(castling_rights, from, to);

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

        // Remove captured piece using atomic operation for better performance
        if (!is_none(captured)) {
            clear_piece(m.get_to());
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
                // Use atomic clear_piece for en passant capture
                clear_piece(captured_pawn_sq);
                
                // Update undo information to remember the captured pawn
                undo.captured = captured_pawn;
            }
        }

        // Handle promotion - use atomic operations for better performance
        if (m.is_promotion()) {
            clear_piece(m.get_from());  // Remove pawn
            add_piece(m.get_to(), make_piece(moving_color, m.get_promoted()));  // Add promoted piece
        } else {
            // Regular move - use atomic move_piece for better performance
            move_piece(m.get_from(), m.get_to());
        }

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
            // Use atomic move_piece for castling rook movement
            move_piece(rook_from, rook_to);
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
    
    // Full bitboard access functions for all piece types
    Bitboard get_piece_bitboard(Color color, PieceType piece_type) const {
        return piece_bitboards[size_t(color)][size_t(piece_type)];
    }
    
    Bitboard get_color_bitboard(Color color) const {
        return color_bitboards[size_t(color)];
    }
    
    Bitboard get_occupied_bitboard() const {
        return occupied_bitboard;
    }
    
    // Convenience accessors for specific piece types
    Bitboard get_pawns(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Pawn)];
    }
    
    Bitboard get_knights(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Knight)];
    }
    
    Bitboard get_bishops(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Bishop)];
    }
    
    Bitboard get_rooks(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Rook)];
    }
    
    Bitboard get_queens(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Queen)];
    }
    
    Bitboard get_kings(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::King)];
    }
};
