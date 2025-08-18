#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include "bitboard.hpp"
#include "board120.hpp"
#include "chess_types.hpp"
#include "move.hpp"

// Maximum search depth / game length
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
    int move;                 // encoded move (from/to/promo packed)
    uint8_t castling_rights;  // previous castling permissions (castlePerm)
    int ep_square;            // previous en passant square (enPas)
    uint16_t halfmove_clock;  // previous fifty move counter (fiftyMove)
    uint64_t zobrist_key;     // previous position key (posKey)
    Piece captured;           // captured piece (if any)
    
    // Helper to encode/decode move
    static int encode_move(int from, int to, PieceType promo = PieceType::None) {
        return (from & 0x7F) | ((to & 0x7F) << 7) | ((int(promo) & 0x7) << 14);
    }
    
    static void decode_move(int encoded, int& from, int& to, PieceType& promo) {
        from = encoded & 0x7F;
        to = (encoded >> 7) & 0x7F;
        promo = PieceType((encoded >> 14) & 0x7);
    }
};

struct Position {
    std::array<Piece, 120> board{};  // Piece::None for empty, Piece::Offboard for offboard
    Color side_to_move{Color::White};
    int ep_square{-1};               // mailbox-120 index or -1
    uint8_t castling_rights{0};      // bitmask: CASTLE_WK|CASTLE_WQ|CASTLE_BK|CASTLE_BQ
    uint16_t halfmove_clock{0};
    uint16_t fullmove_number{1};
    std::array<int, 2> king_sq{ -1, -1 }; // [White, Black] king locations (120)
    std::array<uint64_t, 2> pawns_bb{ 0, 0 }; // [White, Black] pawn bitboards (64)
    std::array<int, 7> piece_counts{}; // count by PieceType (None, Pawn, ..., King)
    uint64_t zobrist_key{0};
    
    // Piece lists: pList[color][piece_type][index] = square
    // Tracks locations of all pieces for fast iteration
    std::array<PieceList, 2> pList; // [White, Black]
    std::array<std::array<int, int(PieceType::_Count)>, 2> pCount; // Number of pieces [color][type]
    
    // Move history for undo functionality - fixed array for performance
    std::array<S_UNDO, MAXPLY> move_history{};
    int ply{0};                      // current search/game ply

    void reset() {
        // Set all squares to offboard first
        for (auto& square : board) {
            square = Piece::Offboard;
        }
        
        // Set all real (playable) squares to empty
        for (int rank = 0; rank < 8; ++rank) {
            for (int file = 0; file < 8; ++file) {
                int square = sq(static_cast<File>(file), static_cast<Rank>(rank));
                board[square] = Piece::None;
            }
        }
        
        // Clear piece counters - bigPce, majPce, minPce equivalents
        piece_counts.fill(0);  // Clear all piece type counts
        
        // Clear pawn bitboards to 0ULL
        pawns_bb[0] = 0ULL;  // White pawns
        pawns_bb[1] = 0ULL;  // Black pawns
        
        // Clear all pceNum (piece counts per type per color)
        for (int color = 0; color < 2; ++color) {
            for (int type = 0; type < int(PieceType::_Count); ++type) {
                pCount[color][type] = 0;
                
                // Clear piece lists
                for (int i = 0; i < MAX_PIECES_PER_TYPE; ++i) {
                    pList[color][type][i] = -1;  // -1 indicates no piece
                }
            }
        }
        
        // Clear kingSq for both colors
        king_sq[0] = -1;  // White king
        king_sq[1] = -1;  // Black king
        
        // Clear side to Both (represented as None since there's no Both enum value)
        side_to_move = Color::None;  // Neither side to move
        
        // Clear enPas (en passant square)
        ep_square = -1;
        
        // Clear fiftyMove (halfmove clock)
        halfmove_clock = 0;
        
        // Clear ply and hiPly equivalent
        ply = 0;
        
        // Reset fullmove number to 1
        fullmove_number = 1;
        
        // Clear castlePerm (castling rights)
        castling_rights = 0;  // No castling rights
        
        // Clear poskey (Zobrist position key)
        zobrist_key = 0ULL;
        
        // Clear all history variables
        for (int i = 0; i < MAXPLY; ++i) {
            move_history[i].move = 0;  // No move (0 is safe for encoded move)
            move_history[i].castling_rights = 0;
            move_history[i].ep_square = -1;
            move_history[i].halfmove_clock = 0;
            move_history[i].zobrist_key = 0ULL;
            move_history[i].captured = Piece::None;
        }
    }

    // Parse FEN string and set position accordingly
    bool set_from_fen(const std::string& fen) {
        reset(); // Start with clean slate
        
        // Split FEN into components
        std::vector<std::string> tokens;
        std::istringstream iss(fen);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.size() != 6) {
            return false; // Invalid FEN format
        }
        
        // 1. Parse piece placement (board)
        const std::string& placement = tokens[0];
        int rank = 7; // Start from rank 8 (index 7)
        int file = 0;
        
        for (char ch : placement) {
            if (ch == '/') {
                rank--;
                file = 0;
                if (rank < 0) return false; // Too many ranks
            } else if (ch >= '1' && ch <= '8') {
                // Empty squares
                int empty_count = ch - '0';
                file += empty_count;
                if (file > 8) return false; // Too many files
            } else {
                // Piece character
                Piece piece = from_char(ch);
                if (is_none(piece) && ch != '.' && ch != '#') {
                    return false; // Invalid piece character
                }
                if (file >= 8) return false; // Too many files
                
                int square = sq(static_cast<File>(file), static_cast<Rank>(rank));
                board[square] = piece;
                file++;
            }
        }
        
        if (rank != 0 || file != 8) {
            return false; // Incomplete board
        }
        
        // 2. Parse side to move
        const std::string& side = tokens[1];
        if (side == "w") {
            side_to_move = Color::White;
        } else if (side == "b") {
            side_to_move = Color::Black;
        } else {
            return false; // Invalid side to move
        }
        
        // 3. Parse castling rights
        const std::string& castling = tokens[2];
        castling_rights = CASTLE_NONE;
        if (castling != "-") {
            for (char ch : castling) {
                switch (ch) {
                    case 'K': castling_rights |= CASTLE_WK; break;
                    case 'Q': castling_rights |= CASTLE_WQ; break;
                    case 'k': castling_rights |= CASTLE_BK; break;
                    case 'q': castling_rights |= CASTLE_BQ; break;
                    default: return false; // Invalid castling character
                }
            }
        }
        
        // 4. Parse en passant square
        const std::string& ep = tokens[3];
        if (ep == "-") {
            ep_square = -1;
        } else {
            if (ep.length() != 2) return false;
            char file_char = ep[0];
            char rank_char = ep[1];
            if (file_char < 'a' || file_char > 'h' || rank_char < '1' || rank_char > '8') {
                return false;
            }
            File ep_file = static_cast<File>(file_char - 'a');
            Rank ep_rank = static_cast<Rank>(rank_char - '1');
            ep_square = sq(ep_file, ep_rank);
        }
        
        // 5. Parse halfmove clock
        try {
            halfmove_clock = static_cast<uint16_t>(std::stoi(tokens[4]));
        } catch (...) {
            return false; // Invalid halfmove clock
        }
        
        // 6. Parse fullmove number
        try {
            fullmove_number = static_cast<uint16_t>(std::stoi(tokens[5]));
            if (fullmove_number == 0) fullmove_number = 1; // Ensure it's at least 1
        } catch (...) {
            return false; // Invalid fullmove number
        }
        
        // Rebuild derived state
        rebuild_counts();
        return true;
    }

    // Put standard start position on 12x10
    void set_startpos() {
        // Use FEN parsing for the standard starting position
        const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        if (!set_from_fen(start_fen)) {
            // Fallback to manual setup if FEN parsing fails (shouldn't happen)
            reset();
            side_to_move = Color::White;
            // White pieces
            board[sq(File::A, Rank::R1)] = Piece::WhiteRook;
            board[sq(File::B, Rank::R1)] = Piece::WhiteKnight;
            board[sq(File::C, Rank::R1)] = Piece::WhiteBishop;
            board[sq(File::D, Rank::R1)] = Piece::WhiteQueen;
            board[sq(File::E, Rank::R1)] = Piece::WhiteKing;
            board[sq(File::F, Rank::R1)] = Piece::WhiteBishop;
            board[sq(File::G, Rank::R1)] = Piece::WhiteKnight;
            board[sq(File::H, Rank::R1)] = Piece::WhiteRook;
            for (int f = 0; f < 8; ++f)
                board[sq(static_cast<File>(f), Rank::R2)] = Piece::WhitePawn;

            // Black pieces
            board[sq(File::A, Rank::R8)] = Piece::BlackRook;
            board[sq(File::B, Rank::R8)] = Piece::BlackKnight;
            board[sq(File::C, Rank::R8)] = Piece::BlackBishop;
            board[sq(File::D, Rank::R8)] = Piece::BlackQueen;
            board[sq(File::E, Rank::R8)] = Piece::BlackKing;
            board[sq(File::F, Rank::R8)] = Piece::BlackBishop;
            board[sq(File::G, Rank::R8)] = Piece::BlackKnight;
            board[sq(File::H, Rank::R8)] = Piece::BlackRook;
            for (int f = 0; f < 8; ++f)
                board[sq(static_cast<File>(f), Rank::R7)] = Piece::BlackPawn;

            castling_rights = CASTLE_ALL; // KQkq
            ep_square = -1;
            halfmove_clock = 0;
            fullmove_number = 1;
            rebuild_counts();
        }
    }

    // Update piece counts, king squares, pawn bitboards, and piece lists
    void rebuild_counts() {
        king_sq = { -1, -1 };
        pawns_bb = { 0, 0 };
        piece_counts.fill(0);
        
        // Clear piece lists and counts
        for (int color = 0; color < 2; ++color) {
            for (int type = 0; type < int(PieceType::_Count); ++type) {
                pCount[color][type] = 0;
                for (int i = 0; i < MAX_PIECES_PER_TYPE; ++i) {
                    pList[color][type][i] = -1; // -1 indicates no piece
                }
            }
        }
        
        for (int s = 0; s < 120; ++s) {
            Piece p = board[s];
            if (is_none(p)) continue;
            PieceType pt = type_of(p);
            ++piece_counts[size_t(pt)];
            Color c = color_of(p);
            if (c != Color::None) {
                int color_idx = int(c);
                int type_idx = int(pt);
                
                // Add to piece list
                if (pCount[color_idx][type_idx] < MAX_PIECES_PER_TYPE) {
                    pList[color_idx][type_idx][pCount[color_idx][type_idx]] = s;
                    ++pCount[color_idx][type_idx];
                }
                
                if (pt == PieceType::Pawn) {
                    int s64 = MAILBOX_MAPS.to64[s];
                    if (s64 >= 0) setBit(pawns_bb[size_t(c)], s64);
                }
                if (pt == PieceType::King) {
                    king_sq[size_t(c)] = s;
                }
            }
        }
    }

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
        DEBUG_ASSERT(false, "Piece not found in piece list during move");
    }
    
    // Enhanced move making with full undo support
    void make_move_with_undo(const Move& m) {
        // Debug assertions for move validity
        DEBUG_ASSERT(is_playable(m.from), "Move source square must be playable");
        DEBUG_ASSERT(is_playable(m.to), "Move destination square must be playable");
        DEBUG_ASSERT(!is_none(at(m.from)), "Cannot move from empty square");
        
        // Check for ply overflow
        if (ply >= MAXPLY) {
            DEBUG_ASSERT(false, "Move history overflow - too many moves in game/search");
            return; // For now, just return without making the move
        }
        
        S_UNDO& undo = move_history[ply]; // Direct array access
        undo.move = S_UNDO::encode_move(m.from, m.to, m.promo);
        undo.castling_rights = castling_rights;
        undo.ep_square = ep_square;
        undo.halfmove_clock = halfmove_clock;
        undo.zobrist_key = zobrist_key;
        undo.captured = at(m.to);
        
        // Increment ply after saving undo info
        ++ply;
        
        // Make the move
        Piece moving = at(m.from);
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
            remove_piece_from_list(color_of(captured), type_of(captured), m.to);
        }
        
        // Handle promotion - remove pawn and add promoted piece
        if (m.promo != PieceType::None) {
            remove_piece_from_list(moving_color, PieceType::Pawn, m.from);
            add_piece_to_list(moving_color, m.promo, m.to);
            set(m.to, make_piece(moving_color, m.promo));
        } else {
            // Regular move - update piece location in list
            move_piece_in_list(moving_color, moving_type, m.from, m.to);
            set(m.to, moving);
        }
        
        set(m.from, Piece::None);
        
        ep_square = -1; // Reset, update with double pawn push logic later
        side_to_move = !side_to_move;
        if (side_to_move == Color::White) ++fullmove_number;
        
        // Update derived state (piece counts, king squares, etc.)
        // Note: piece lists are already updated above for performance
        rebuild_counts();
        // Update zobrist_key here when Zobrist integration is complete
    }
    
    // Undo the last move
    bool undo_move() {
        if (ply == 0) return false; // No moves to undo
        
        // Decrement ply first to get the correct index
        --ply;
        S_UNDO& undo = move_history[ply]; // Direct array access
        
        // Decode the move
        int from, to;
        PieceType promo;
        S_UNDO::decode_move(undo.move, from, to, promo);
        
        // Undo side to move first
        side_to_move = !side_to_move;
        if (side_to_move == Color::Black) --fullmove_number;
        
        // Get the piece that moved
        Piece moved = at(to);
        
        // If it was a promotion, restore to pawn
        if (promo != PieceType::None) {
            moved = make_piece(color_of(moved), PieceType::Pawn);
        }
        
        // Move piece back
        set(from, moved);
        set(to, undo.captured); // Restore captured piece (or Piece::None)
        
        // Restore position state
        castling_rights = undo.castling_rights;
        ep_square = undo.ep_square;
        halfmove_clock = undo.halfmove_clock;
        zobrist_key = undo.zobrist_key;
        
        // Update derived state
        rebuild_counts();
        
        return true;
    }
};

// Minimal make/unmake stubs (quiet moves + captures only for now)
// Flesh these out as you implement move legality, en-passant, castling, promotions.
inline void make_move(Position& pos, const Move& m, State& st) {
    st.ep_square = pos.ep_square;
    st.castling_rights = pos.castling_rights;
    st.halfmove_clock = pos.halfmove_clock;
    st.captured = pos.at(m.to);

    Piece moving = pos.at(m.from);
    // update halfmove clock
    if (type_of(moving) == PieceType::Pawn || !is_none(st.captured)) pos.halfmove_clock = 0;
    else ++pos.halfmove_clock;

    pos.set(m.to, moving);
    pos.set(m.from, Piece::None);
    // promotions (simple)
    if (m.promo != PieceType::None) {
        pos.set(m.to, make_piece(color_of(moving), m.promo));
    }
    pos.ep_square = -1; // set by double push logic once you add it
    pos.side_to_move = !pos.side_to_move;
    if (pos.side_to_move == Color::White) ++pos.fullmove_number; // black just moved
}

inline void unmake_move(Position& pos, const Move& m, const State& st) {
    pos.side_to_move = !pos.side_to_move;
    if (pos.side_to_move == Color::Black) --pos.fullmove_number; // undo the increment
    Piece moved = pos.at(m.to);
    // If promo, restore pawn
    if (m.promo != PieceType::None) {
        moved = make_piece(color_of(moved), PieceType::Pawn);
    }
    pos.set(m.from, moved);
    pos.set(m.to, st.captured);
    pos.ep_square = st.ep_square;
    pos.castling_rights = st.castling_rights;
    pos.halfmove_clock = st.halfmove_clock;
}