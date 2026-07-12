/**
 * @file position.cpp
 * @brief Implementation of the Position class methods
 * 
 * Contains the implementation of core Position class functionality including
 * move making/unmaking, position setup, state management, and board operations.
 * The Position class is the central data structure of the Huginn engine,
 * optimized for fast incremental updates during search.
 * 
 * ## Key Operations
 * - **Move Making/Unmaking**: Fast incremental position updates for search
 * - **Position Setup**: FEN parsing and initial position configuration
 * - **State Management**: Hash keys, piece lists, castling rights, en passant
 * - **Board Operations**: Piece placement, capture handling, promotion logic
 * 
 * ## Performance Features
 * - **Incremental Hash Updates**: Zobrist key maintenance for TT lookup
 * - **Copy-Make Optimization**: Efficient position copying for search
 * - **Fast Validation**: Quick legality checks and position consistency
 * - **Memory Efficiency**: Compact state representation for deep search
 * 
 * @author MTDuke71
 * @version 1.2
 * @see position.hpp for class declaration and interface
 */
#include "position.hpp"
#include "zobrist.hpp"
#include "movegen.hpp"  // For in_check function
#include "attack_detection.hpp"  // For Huginn::SqAttacked function
#include "attack_tables.hpp"     // #59: pawn_attacks for EP-right normalization

/// @brief Incrementally fold the side-to-move, castling-rights, and en-passant
///        changes of a move into the Zobrist key. Piece add/remove XORs are done
///        by the make/unmake primitives; this handles only the state bits.
void Position::update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, uint8_t old_castling_rights, int old_ep_square) {
    // XOR side to move flag to match full computation logic
    // Full computation: if (side_to_move == Color::Black) key ^= Side;
    // Since we always flip sides on a move, we always need to XOR the Side bit
    zobrist_key ^= Zobrist::Side;

    // XOR out old castling rights and XOR in new castling rights
    zobrist_key ^= Zobrist::Castle[old_castling_rights & 0xF];
    zobrist_key ^= Zobrist::Castle[castling_rights & 0xF];

    // XOR out old en passant file (if any)
    if (old_ep_square != -1) {
        int old_ep_file = old_ep_square & 7;  // ep_square is sq64
        if (old_ep_file >= 0 && old_ep_file < 8) {
            zobrist_key ^= Zobrist::EpFile[old_ep_file];
        }
    }
    // XOR in new en passant file (if any)  
    if (ep_square != -1) {
        int new_ep_file = ep_square & 7;  // ep_square is sq64
        if (new_ep_file >= 0 && new_ep_file < 8) {
            zobrist_key ^= Zobrist::EpFile[new_ep_file];
        }
    }
}

/// @brief Recompute the Zobrist key from scratch over the whole position
///        (full rebuild — use after non-incremental edits like FEN setup).
void Position::update_zobrist_key() {
    zobrist_key = Zobrist::compute(*this);
}

/// @brief Self-check that the cached state agrees with the bitboards
///        (occupancy, king squares, side, castling/ep validity, Zobrist).
/// @param[out] reason If non-null, set to a description on the first failure.
/// @return true if the position is internally consistent. Debug/test aid.
bool Position::is_consistent(std::string* reason) const {
    auto fail = [&](const std::string& why) {
        if (reason) *reason = why;
        return false;
    };

    if (castling_rights & ~CASTLE_ALL) return fail("castling rights contain invalid bits");
    if (ep_square != -1 && (ep_square < 0 || ep_square >= 64)) return fail("ep square out of range");

    std::array<Bitboard, 2> expected_color{0ULL, 0ULL};
    Bitboard expected_occupied = 0ULL;
    std::array<int, 2> expected_material{0, 0};
    std::array<int, 2> expected_king_sq{-1, -1};

    if (piece_bitboards[0][int(PieceType::None)] != 0ULL ||
        piece_bitboards[1][int(PieceType::None)] != 0ULL) {
        return fail("None piece bitboard is non-empty");
    }

    for (int color = 0; color < 2; ++color) {
        for (int type = int(PieceType::Pawn); type <= int(PieceType::King); ++type) {
            const Bitboard bb = piece_bitboards[color][type];
            if (bb & expected_occupied) return fail("piece bitboards overlap");

            expected_color[color] |= bb;
            expected_occupied |= bb;

            if (type == int(PieceType::King)) {
                const int king_count = popcount(bb);
                if (king_count > 1) return fail("multiple kings for one side");
                if (king_count == 1) expected_king_sq[color] = get_lsb(bb);
            } else {
                expected_material[color] +=
                    popcount(bb) * value_of(make_piece(Color(color), PieceType(type)));
            }
        }
    }

    if (expected_color[0] != color_bitboards[0]) return fail("white color bitboard cache mismatch");
    if (expected_color[1] != color_bitboards[1]) return fail("black color bitboard cache mismatch");
    if ((color_bitboards[0] & color_bitboards[1]) != 0ULL) return fail("color bitboards overlap");
    if (expected_occupied != occupied_bitboard) return fail("occupied bitboard cache mismatch");
    if (expected_king_sq != king_sq) return fail("king square cache mismatch");
    if (expected_material != material_score) return fail("material cache mismatch");

    auto piece_from_piece_bitboards = [&](int sq) {
        const Bitboard bit = 1ULL << sq;
        for (int color = 0; color < 2; ++color) {
            for (int type = int(PieceType::Pawn); type <= int(PieceType::King); ++type) {
                if (piece_bitboards[color][type] & bit) {
                    return make_piece(Color(color), PieceType(type));
                }
            }
        }
        return Piece::None;
    };

    for (int sq = 0; sq < 64; ++sq) {
        if (at_sq64(sq) != piece_from_piece_bitboards(sq)) {
            return fail("at_sq64 disagrees with per-piece bitboards");
        }
    }

    uint64_t expected_key = 0ULL;
    for (int color = 0; color < 2; ++color) {
        for (int type = int(PieceType::Pawn); type <= int(PieceType::King); ++type) {
            Bitboard bb = piece_bitboards[color][type];
            const int zpiece = type + (color == int(Color::Black) ? 6 : 0);
            while (bb) {
                const int sq = pop_lsb(bb);
                expected_key ^= Zobrist::Piece[zpiece][sq];
            }
        }
    }
    if (side_to_move == Color::Black) expected_key ^= Zobrist::Side;
    expected_key ^= Zobrist::Castle[castling_rights & 0xF];
    if (ep_square != -1) expected_key ^= Zobrist::EpFile[ep_square & 7];
    if (expected_key != zobrist_key) return fail("zobrist key mismatch");

    if (reason) reason->clear();
    return true;
}

/// @brief Clear the position to empty: no pieces, no rights, side None, ply 0.
void Position::reset() {
    material_score[0] = 0;
    material_score[1] = 0;

    for (int color = 0; color < 2; ++color) {
        color_bitboards[color] = 0ULL;
        for (int type = 0; type < int(PieceType::_Count); ++type) {
            piece_bitboards[color][type] = 0ULL;
        }
    }
    occupied_bitboard = 0ULL;
    king_sq[0] = -1;
    king_sq[1] = -1;
    side_to_move = Color::None;
    ep_square = -1;
    halfmove_clock = 0;
    ply = 0;
    fullmove_number = 1;
    castling_rights = 0;
    zobrist_key = 0ULL;
    move_history.clear();
}
namespace {
/// Strict, fully-consuming, non-negative integer parse for FEN clock fields
/// (BACKLOG #54): digits only — rejects signs, spaces, and suffix junk that
/// std::stoi silently accepted ("-1" became 65535, "12junk" became 12) — and
/// an explicit range check instead of silent narrowing into uint16_t.
bool parse_fen_counter(const std::string& token, long min_value, long max_value,
                       uint16_t& out) {
    if (token.empty() || token.size() > 5) return false;
    long value = 0;
    for (char ch : token) {
        if (ch < '0' || ch > '9') return false;
        value = value * 10 + (ch - '0');
    }
    if (value < min_value || value > max_value) return false;
    out = static_cast<uint16_t>(value);
    return true;
}
} // namespace

/// @brief Parse a FEN string into this position (board, side, castling, ep,
///        clocks), rebuild the derived bitboards/counts and the Zobrist key.
/// @param fen A FEN record.
/// @return true on success; false on malformed input, in which case *this is
///         left EXACTLY as it was (BACKLOG #54: the parse is transactional —
///         it runs on a scratch position and commits only on full success).
bool Position::set_from_fen(const std::string& fen) {
    Position parsed;
    parsed.reset();

    std::vector<std::string> tokens;
    std::istringstream iss(fen);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    if (tokens.size() != 6) {
        return false;
    }

    // Piece placement: exactly eight ranks of exactly eight files. Digits are
    // restricted to '1'..'8' (isdigit also let '0' and '9' through) and the
    // file is bounds-checked BEFORE set_sq64 so no `1ULL << square` can run
    // off the board.
    const std::string& placement = tokens[0];
    int rank = 7;
    int file = 0;
    for (char ch : placement) {
        if (ch == '/') {
            if (file != 8 || rank <= 0) return false;
            rank--;
            file = 0;
        } else if (ch >= '1' && ch <= '8') {
            file += ch - '0';
            if (file > 8) return false;
        } else {
            Piece piece = from_char(ch);
            if (piece == Piece::None || file >= 8) return false;
            parsed.set_sq64(sq64(static_cast<File>(file), static_cast<Rank>(rank)), piece);
            file++;
        }
    }
    if (rank != 0 || file != 8) return false;

    // Side to move: must be "w" or "b"
    if (tokens[1] == "w") {
        parsed.side_to_move = Color::White;
    } else if (tokens[1] == "b") {
        parsed.side_to_move = Color::Black;
    } else {
        return false;
    }

    // Castling rights: "-" or a non-empty subset of "KQkq" without duplicates
    if (tokens[2] != "-") {
        if (tokens[2].empty() || tokens[2].size() > 4) return false;
        for (char c : tokens[2]) {
            uint8_t right;
            switch (c) {
                case 'K': right = CASTLE_WK; break;
                case 'Q': right = CASTLE_WQ; break;
                case 'k': right = CASTLE_BK; break;
                case 'q': right = CASTLE_BQ; break;
                default: return false;
            }
            if (parsed.castling_rights & right) return false;  // duplicate
            parsed.castling_rights |= right;
        }
    }

    // En passant: "-" or a rank-3/6 square coherent with the side to move
    // (only a black double push can leave a rank-6 target behind it, and only
    // a white double push a rank-3 one).
    if (tokens[3] == "-") {
        parsed.ep_square = -1;
    } else if (tokens[3].size() == 2 &&
               tokens[3][0] >= 'a' && tokens[3][0] <= 'h' &&
               ((tokens[3][1] == '6' && parsed.side_to_move == Color::White) ||
                (tokens[3][1] == '3' && parsed.side_to_move == Color::Black))) {
        File ep_file = File(tokens[3][0] - 'a');
        Rank ep_rank = Rank(tokens[3][1] - '1');
        int ep_sq = sq64(ep_file, ep_rank);
        // #59: normalize on input exactly like MakeMove — keep the EP right
        // only if a side-to-move pawn could pseudo-capture onto it, so a FEN
        // with a decorative EP square hashes identically to the same
        // placement without one.
        const Color pusher = !parsed.side_to_move;
        parsed.ep_square =
            (pawn_attacks[int(pusher)][ep_sq] &
             parsed.piece_bitboards[int(parsed.side_to_move)][int(PieceType::Pawn)])
                ? ep_sq : -1;
    } else {
        return false;
    }

    // Move counters: strict digits-only parse with explicit storage ranges
    if (!parse_fen_counter(tokens[4], 0, 65535, parsed.halfmove_clock)) return false;
    if (!parse_fen_counter(tokens[5], 1, 65535, parsed.fullmove_number)) return false;

    parsed.rebuild_counts();
    parsed.update_zobrist_key();

    // Full command parsed clean — commit.
    *this = std::move(parsed);
    return true;
}

/// @brief Serialise the current position to a FEN string (inverse of set_from_fen).
std::string Position::to_fen() const {
    std::string fen;
    for (int rank = 7; rank >= 0; --rank) {
        int empty_count = 0;
        for (int file = 0; file < 8; ++file) {
            int sq_idx = sq64(static_cast<File>(file), static_cast<Rank>(rank));
            Piece piece = at_sq64(sq_idx);
            if (is_none(piece)) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen += std::to_string(empty_count);
                    empty_count = 0;
                }
                fen += to_char(piece);
            }
        }
        if (empty_count > 0) {
            fen += std::to_string(empty_count);
        }
        if (rank > 0) {
            fen += '/';
        }
    }
    fen += ' ';
    fen += (side_to_move == Color::White) ? 'w' : 'b';
    fen += ' ';
    std::string castling;
    if (castling_rights & CASTLE_WK) castling += 'K';
    if (castling_rights & CASTLE_WQ) castling += 'Q';
    if (castling_rights & CASTLE_BK) castling += 'k';
    if (castling_rights & CASTLE_BQ) castling += 'q';
    fen += castling.empty() ? std::string(1, '-') : castling;
    fen += ' ';
    if (ep_square == -1) {
        fen += '-';
    } else {
        File file = File(ep_square & 7);   // ep_square is sq64
        Rank rank = Rank(ep_square >> 3);
        fen += char('a' + int(file));
        fen += char('1' + int(rank));
    }
    fen += ' ';
    fen += std::to_string(halfmove_clock);
    fen += ' ';
    fen += std::to_string(fullmove_number);
    return fen;
}

/// @brief Recompute derived state (occupancy, colour bitboards, king squares,
///        material) from the per-piece bitboards after a non-incremental edit.
void Position::rebuild_counts() {
    // Recompute color_bitboards / occupied_bitboard from piece_bitboards
    // (the per-piece-type bitboards are the source of truth — set() and
    // set_from_fen maintain them directly).
    for (int color = 0; color < 2; ++color) {
        uint64_t total = 0;
        for (int type = int(PieceType::Pawn); type <= int(PieceType::King); ++type) {
            total |= piece_bitboards[color][type];
        }
        color_bitboards[color] = total;
    }
    occupied_bitboard = color_bitboards[0] | color_bitboards[1];

    // Derive king_sq[] and material_score[] from the bitboards
    material_score[0] = 0;
    material_score[1] = 0;
    king_sq[0] = -1;
    king_sq[1] = -1;
    for (int color = 0; color < 2; ++color) {
        Color c = static_cast<Color>(color);
        uint64_t kings = piece_bitboards[color][int(PieceType::King)];
        if (kings != 0) {
            int king_sq64 = get_lsb(kings);
            king_sq[color] = king_sq64;  // king_sq is now sq64
        }
        for (int type = int(PieceType::Pawn); type < int(PieceType::King); ++type) {
            int count = popcount(piece_bitboards[color][type]);
            material_score[color] += count * value_of(make_piece(c, static_cast<PieceType>(type)));
        }
    }

}

// Set up the standard chess starting position using FEN
/// @brief Set the standard chess starting position (via the start FEN).
void Position::set_startpos() {
    const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    set_from_fen(start_fen);
}

/// @brief Apply a pseudo-legal move with full incremental update (bitboards,
///        Zobrist, castling/ep, clocks) and push an undo record, then test
///        legality by checking whether the mover's king is left in check.
/// @param move A pseudo-legal move from the generator.
/// @return 1 if the move is legal (and stays applied); 0 if it left the king in
///         check (the move is fully unmade before returning). VICE Part 41.
int Position::MakeMove(const S_MOVE& move) {
    // Debug assertions for move validity (from/to are 64-square indices)
    DEBUG_ASSERT(move.get_from() >= 0 && move.get_from() < 64, "Move source square must be a valid sq64");
    DEBUG_ASSERT(move.get_to() >= 0 && move.get_to() < 64, "Move destination square must be a valid sq64");

    // Store history information for potential undo
    int from = move.get_from();
    int to = move.get_to();
    
    // Store history before making the move (BEFORE any modifications)
    if (ply >= static_cast<int>(move_history.size())) {
        move_history.resize(ply + 1);
    }
    
    S_UNDO& undo = move_history[ply];
    undo.move = move;
    undo.castling_rights = castling_rights;
    undo.ep_square = ep_square; 
    undo.halfmove_clock = halfmove_clock;
    undo.zobrist_key = zobrist_key;  // Save ORIGINAL zobrist key (BEFORE modifications)
    
    // Update castling rights based on move (from/to squares, sq64)
    uint8_t new_castling_rights = CastlingLookup::update_castling_rights_sq64(undo.castling_rights, from, to);
    castling_rights = new_castling_rights;

    // Set captured piece - handle en passant special case
    if (move.is_en_passant()) {
        Color moving_color = color_of(at_sq64(from));
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            captured_pawn_sq = to - 8;  // Black pawn one rank south of target (sq64)
        } else {
            captured_pawn_sq = to + 8;  // White pawn one rank north of target (sq64)
        }

        Piece piece_at_captured_sq = at_sq64(captured_pawn_sq);
        if (is_none(piece_at_captured_sq)) {
            // This is an invalid en passant move - there's no pawn to capture!
            return 0;  // Return illegal move
        }
        
        undo.captured = piece_at_captured_sq;  // Capture the pawn that's actually being removed
    } else {
        undo.captured = at_sq64(to);  // Normal capture (to is sq64)
    }
    
    // Handle en passant captures BEFORE updating ep_square
    if (move.is_en_passant()) {
        Color moving_color = color_of(at_sq64(from));
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            captured_pawn_sq = to - 8;  // Black pawn one rank south of target (sq64)
        } else {
            captured_pawn_sq = to + 8;  // White pawn one rank north of target (sq64)
        }

        // Clear the captured pawn using atomic operation
        clear_piece_sq64(captured_pawn_sq);
    }

    // Handle 50-move rule: reset on pawn move or capture
    Piece moving_piece = at_sq64(from);
    if (type_of(moving_piece) == PieceType::Pawn || !is_none(at_sq64(to))) {
        halfmove_clock = 0;
    } else {
        ++halfmove_clock;
    }
    
    // Increment ply counter
    ++ply;
    
    // Reset en passant square 
    ep_square = -1;
    
    // Set en passant square for pawn double moves.
    // #59: the EP right is stored ONLY when an enemy pawn could pseudo-
    // capture onto it (Polyglot / X-FEN convention). An uncapturable EP
    // square used to be stored and hashed, making positions with identical
    // legal move sets hash differently — missed threefolds and TT splits.
    // pawn_attacks[pusher][ep_sq] = the two squares a capturer must occupy.
    if (type_of(moving_piece) == PieceType::Pawn && !move.is_capture()) {
        int from_rank = from >> 3;  // sq64 rank 0..7
        int to_rank = to >> 3;

        if (abs(to_rank - from_rank) == 2) {  // Pawn moved two squares
            int ep_rank = (from_rank + to_rank) / 2;
            int ep_sq = sq64(File(to & 7), Rank(ep_rank));
            const Color pusher = color_of(moving_piece);
            if (pawn_attacks[int(pusher)][ep_sq] &
                piece_bitboards[int(!pusher)][int(PieceType::Pawn)]) {
                ep_square = ep_sq;
            }
        }
    }

    // Make the actual move using atomic operations
    if (move.is_promotion()) {
        // Handle capture before promotion
        if (!is_none(at_sq64(to))) {
            clear_piece_sq64(to);  // Remove captured piece
        }
        clear_piece_sq64(from);  // Remove pawn
        add_piece_sq64(to, make_piece(color_of(moving_piece), move.get_promoted()));  // Add promoted piece
    } else {
        // Handle normal capture
        if (!is_none(at_sq64(to))) {
            clear_piece_sq64(to);  // Remove captured piece first
        }
        move_piece_sq64(from, to);  // Then move the piece
    }
    
    // Handle castling - move the rook
    if (move.is_castle()) {
        Color king_color = color_of(moving_piece);
        int rook_from, rook_to;
        
        if (king_color == Color::White) {
            if (to == sq64(File::G, Rank::R1)) {  // White kingside
                rook_from = sq64(File::H, Rank::R1);
                rook_to = sq64(File::F, Rank::R1);
            } else {  // White queenside
                rook_from = sq64(File::A, Rank::R1);
                rook_to = sq64(File::D, Rank::R1);
            }
        } else {
            if (to == sq64(File::G, Rank::R8)) {  // Black kingside
                rook_from = sq64(File::H, Rank::R8);
                rook_to = sq64(File::F, Rank::R8);
            } else {  // Black queenside
                rook_from = sq64(File::A, Rank::R8);
                rook_to = sq64(File::D, Rank::R8);
            }
        }

        // Move the rook using atomic operation
        move_piece_sq64(rook_from, rook_to);
    }
    
    // Update king square if king moved
    if (type_of(moving_piece) == PieceType::King) {
        king_sq[int(color_of(moving_piece))] = to;
    }
    
    // Change side to move
    side_to_move = !side_to_move;
    if (side_to_move == Color::White) {
        ++fullmove_number;
    }
    
    // Update zobrist_key incrementally using XOR (much faster than recomputing)
    update_zobrist_for_move(move, moving_piece, undo.captured, undo.castling_rights, undo.ep_square);
    
    // Check if move left current player's king in check
    // Note: side_to_move has already been flipped, so we check the previous side's king
    Color previous_side = !side_to_move;
    int king_square = king_sq[int(previous_side)];

    // king_square == -1 only happens in partial test positions with no king of that color;
    // there is nothing to be in check, so treat the move as legal.
    if (king_square >= 0 && Huginn::SqAttackedBB(king_square, *this, side_to_move)) {
        // Move is illegal - undo it
        TakeMove();
        return 0;  // Illegal move
    }
    
    return 1;  // Legal move
}

/// @brief Undo the most recent MakeMove, restoring the board, Zobrist key, and
///        all state from the top undo record (the exact inverse of MakeMove).
///        Precondition: at least one move has been made (ply > 0). VICE Part 42.
void Position::TakeMove() {
    // Must have at least one move to undo
    DEBUG_ASSERT(ply > 0, "Cannot take move when ply is 0");
    
    // Decrement ply first to get the correct history index
    --ply;
    
    DEBUG_ASSERT(ply >= 0, "Invalid ply after decrement in TakeMove");
    DEBUG_ASSERT(ply < static_cast<int>(move_history.size()), "Ply exceeds move_history size in TakeMove");
    S_UNDO& undo = move_history[ply];
    S_MOVE move = undo.move;
    
    // Extract move information - for TakeMove, from/to are effectively reversed
    int from = move.get_from();
    int to = move.get_to();

    // Restore position state from history (let forced recomputation handle zobrist)
    castling_rights = undo.castling_rights;
    ep_square = undo.ep_square;
    halfmove_clock = undo.halfmove_clock;    // NOTE: Don't restore zobrist_key here - let incremental updates handle it
    // The atomic operations will restore piece positions incrementally
    // We just need to restore side/castling/en passant incrementally
    
    // Handle en passant undo - restore captured pawn
    if (move.is_en_passant()) {
        Color moving_color = color_of(at_sq64(to));
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            captured_pawn_sq = to - 8;  // Black pawn was one rank south of target (sq64)
        } else {
            captured_pawn_sq = to + 8;  // White pawn was one rank north of target (sq64)
        }

        // Restore the captured pawn (stored in undo.captured)
        add_piece_sq64(captured_pawn_sq, undo.captured);
    }
    
    // Handle castling undo - move rook back to original position
    if (move.is_castle()) {
        Color king_color = color_of(at_sq64(to));
        int rook_from, rook_to;

        if (king_color == Color::White) {
            if (to == sq64(File::G, Rank::R1)) {  // White kingside
                rook_from = sq64(File::H, Rank::R1);  // Original rook position
                rook_to = sq64(File::F, Rank::R1);    // Current rook position
            } else {  // White queenside
                rook_from = sq64(File::A, Rank::R1);  // Original rook position
                rook_to = sq64(File::D, Rank::R1);    // Current rook position
            }
        } else {
            if (to == sq64(File::G, Rank::R8)) {  // Black kingside
                rook_from = sq64(File::H, Rank::R8);  // Original rook position
                rook_to = sq64(File::F, Rank::R8);    // Current rook position
            } else {  // Black queenside
                rook_from = sq64(File::A, Rank::R8);  // Original rook position
                rook_to = sq64(File::D, Rank::R8);    // Current rook position
            }
        }

        // Move rook back to original position
        move_piece_sq64(rook_to, rook_from);
    }
    
    // Move piece back to original square BEFORE adding captured piece
    if (move.is_promotion()) {
        // For promotion, remove promoted piece and restore original pawn
        clear_piece_sq64(to);  // Remove promoted piece
        Color moving_color = color_of(undo.captured) == Color::None ?
            (side_to_move == Color::White ? Color::Black : Color::White) :
            (color_of(undo.captured) == Color::White ? Color::Black : Color::White);
        add_piece_sq64(from, make_piece(moving_color, PieceType::Pawn));  // Restore pawn
    } else {
        // Regular move - move piece back
        DEBUG_ASSERT(to >= 0 && to < 64, "Invalid 'to' sq64 in TakeMove");
        DEBUG_ASSERT(from >= 0 && from < 64, "Invalid 'from' sq64 in TakeMove");
        DEBUG_ASSERT(!is_none(at_sq64(to)), "No piece to move back from 'to' square");
        move_piece_sq64(to, from);
    }

    // Add captured piece back to destination square (if any)
    if (!is_none(undo.captured) && !move.is_en_passant()) {
        add_piece_sq64(to, undo.captured);
    }

    // Update king square if king moved
    Piece moved_piece = at_sq64(from);
    if (type_of(moved_piece) == PieceType::King) {
        king_sq[int(color_of(moved_piece))] = from;
    }
    
    // Flip side to move back
    side_to_move = !side_to_move;
    if (side_to_move == Color::Black) {
        --fullmove_number;
    }
    
    // Restore the original zobrist key directly from history
    // The atomic operations automatically handle piece position changes via XOR
    zobrist_key = undo.zobrist_key;
}

/// @brief Make a "null move" — flip the side to move and clear en passant
///        without moving a piece — for null-move pruning. Pushes an undo record
///        and updates the Zobrist key. Caller must ensure the side to move is
///        not in check. VICE Part 83. @see TakeNullMove.
void Position::MakeNullMove() {
    // Create undo entry for null move
    if (ply >= static_cast<int>(move_history.size())) {
        move_history.resize(ply + 1);
    }
    
    S_UNDO& undo = move_history[ply];
    
    // Store current state for undo
    undo.move = S_MOVE();  // Empty move for null move
    undo.castling_rights = castling_rights;
    undo.ep_square = ep_square;
    undo.halfmove_clock = halfmove_clock;
    undo.zobrist_key = zobrist_key;
    
    // Clear en passant if it was set
    if (ep_square != -1) {
        zobrist_key ^= Zobrist::EpFile[ep_square & 7];  // File index (sq64)
        ep_square = -1;
    }
    
    // Switch side to move
    side_to_move = !side_to_move;
    zobrist_key ^= Zobrist::Side;
    
    // Increment ply
    ++ply;
    
    // Increment halfmove clock (null move doesn't reset it)
    ++halfmove_clock;
    
    // Increment fullmove number if black just moved
    if (side_to_move == Color::White) {
        ++fullmove_number;
    }
}

/// @brief Undo the most recent MakeNullMove (restore side, en passant, Zobrist).
void Position::TakeNullMove() {
    // Must have at least one move to undo
    DEBUG_ASSERT(ply > 0, "Cannot take null move when ply is 0");
    
    // Decrement ply first
    --ply;
    
    DEBUG_ASSERT(ply >= 0, "Invalid ply after null move undo");
    DEBUG_ASSERT(ply < static_cast<int>(move_history.size()), "Ply exceeds move_history size in null move undo");
    
    S_UNDO& undo = move_history[ply];
    
    // Restore position state
    castling_rights = undo.castling_rights;
    ep_square = undo.ep_square;
    halfmove_clock = undo.halfmove_clock;
    zobrist_key = undo.zobrist_key;
    
    // Flip side to move back
    side_to_move = !side_to_move;
    if (side_to_move == Color::Black) {
        --fullmove_number;
    }
}

// #61: Position::perft() and the member Position::generate_all_moves() were
// removed here. The member generator was an empty stub (cleared the list and
// nothing else), which made the member perft() return 0 for any depth > 0.
// Neither had a single caller. The production entry points are the free
// functions in movegen.hpp/movegen.cpp.
