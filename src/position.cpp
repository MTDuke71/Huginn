// position.cpp - Implementation file for Position class methods
#include "position.hpp"
#include "zobrist.hpp"

// Update Zobrist key incrementally for a move using XOR (much faster than recomputation)
void Position::update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, uint8_t old_castling_rights, int old_ep_square) {
    Color moving_color = color_of(moving);
    PieceType moving_type = type_of(moving);
    
    // XOR out the moving piece from its original square
    int piece_index = int(moving_type) + (moving_color == Color::Black ? 6 : 0);
    zobrist_key ^= Zobrist::Piece[piece_index][m.get_from()];
    
    // XOR out captured piece (if any) from destination square
    if (!is_none(captured)) {
        Color captured_color = color_of(captured);
        PieceType captured_type = type_of(captured);
        int captured_index = int(captured_type) + (captured_color == Color::Black ? 6 : 0);
        zobrist_key ^= Zobrist::Piece[captured_index][m.get_to()];
    }
    
    // XOR in the piece at its new square
    if (m.is_promotion()) {
        // For promotion, the piece type changes
        int promoted_index = int(m.get_promoted()) + (moving_color == Color::Black ? 6 : 0);
        zobrist_key ^= Zobrist::Piece[promoted_index][m.get_to()];
    } else {
        // Regular move - same piece type
        zobrist_key ^= Zobrist::Piece[piece_index][m.get_to()];
    }
    
    // XOR out old castling rights
    zobrist_key ^= Zobrist::Castle[old_castling_rights & 0xF];
    // XOR in new castling rights
    zobrist_key ^= Zobrist::Castle[castling_rights & 0xF];

    // XOR out old en passant file (if any)
    if (old_ep_square != -1) {
        int old_ep_file = static_cast<int>(file_of(old_ep_square));
        if (old_ep_file >= 0 && old_ep_file < 8)
            zobrist_key ^= Zobrist::EpFile[old_ep_file];
    }
    // XOR in new en passant file (if any)
    if (ep_square != -1) {
        int new_ep_file = static_cast<int>(file_of(ep_square));
        if (new_ep_file >= 0 && new_ep_file < 8)
            zobrist_key ^= Zobrist::EpFile[new_ep_file];
    }

    // XOR the side to move (since it always flips)
    zobrist_key ^= Zobrist::Side;
}

// Compute and set the Zobrist key from current position
void Position::update_zobrist_key() {
    zobrist_key = Zobrist::compute(*this);
}

void Position::make_move_with_undo(const S_MOVE& m, S_UNDO& undo) {
    // std::cout << "[DEBUG] make_move_with_undo called. m.is_castle(): " << m.is_castle() << std::endl;
    
    // Debug assertions for move validity
    DEBUG_ASSERT(is_playable(m.get_from()), "Move source square must be playable");
    DEBUG_ASSERT(is_playable(m.get_to()), "Move destination square must be playable");
    DEBUG_ASSERT(!is_none(at(m.get_from())), "Cannot move from empty square");

    // Ensure move_history has enough capacity
    if (ply >= static_cast<int>(move_history.size())) {
        move_history.resize(ply + 1);
    }

    undo.move = m; // Store the complete S_MOVE
    undo.castling_rights = castling_rights;
    undo.ep_square = ep_square;
    undo.halfmove_clock = halfmove_clock;
    undo.zobrist_key = zobrist_key;
    undo.captured = at(m.get_to());

    // Save derived state for efficient undo (performance optimization)
    undo.king_sq_backup = king_sq;
    undo.pawns_bb_backup = pawns_bb;
    undo.all_pawns_bb_backup = all_pawns_bb;
    undo.piece_counts_backup = piece_counts;
    undo.material_score_backup = material_score;
    undo.pList_backup = pList;
    undo.pCount_backup = pCount;

    // Handle castling moves explicitly and return early
    if (m.is_castle()) {
        // std::cout << "[DEBUG] CASTLING MOVE DETECTED: " << m.get_from() << "->" << m.get_to() << std::endl;
        
        // Update castling rights - clear for moving color
        if (color_of(at(m.get_from())) == Color::White) {
            castling_rights &= ~(CASTLE_WK | CASTLE_WQ);
        } else {
            castling_rights &= ~(CASTLE_BK | CASTLE_BQ);
        }
        
        // Update board for castling
        if (color_of(at(m.get_from())) == Color::White) {
            if (m.get_to() == sq(File::G, Rank::R1)) { // White kingside e1g1
                board[sq(File::E, Rank::R1)] = Piece::None;
                board[sq(File::H, Rank::R1)] = Piece::None;
                board[sq(File::G, Rank::R1)] = Piece::WhiteKing;
                board[sq(File::F, Rank::R1)] = Piece::WhiteRook;
            } else if (m.get_to() == sq(File::C, Rank::R1)) { // White queenside e1c1
                board[sq(File::E, Rank::R1)] = Piece::None;
                board[sq(File::A, Rank::R1)] = Piece::None;
                board[sq(File::C, Rank::R1)] = Piece::WhiteKing;
                board[sq(File::D, Rank::R1)] = Piece::WhiteRook;
            }
        } else {
            if (m.get_to() == sq(File::G, Rank::R8)) { // Black kingside e8g8
                board[sq(File::E, Rank::R8)] = Piece::None;
                board[sq(File::H, Rank::R8)] = Piece::None;
                board[sq(File::G, Rank::R8)] = Piece::BlackKing;
                board[sq(File::F, Rank::R8)] = Piece::BlackRook;
            } else if (m.get_to() == sq(File::C, Rank::R8)) { // Black queenside e8c8
                board[sq(File::E, Rank::R8)] = Piece::None;
                board[sq(File::A, Rank::R8)] = Piece::None;
                board[sq(File::C, Rank::R8)] = Piece::BlackKing;
                board[sq(File::D, Rank::R8)] = Piece::BlackRook;
            }
        }
        
        // Update game state
        ++ply;
        ++halfmove_clock; // Castling is not a pawn move or capture
        side_to_move = !side_to_move;
        if (side_to_move == Color::White) ++fullmove_number;
        ep_square = -1; // Clear en passant
        
        return;
    }

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
    // --- Castling rights update logic (robust) ---
    // Clear castling rights if king or rook moves, rook is captured, or castling is performed
    // King moves (including castling)
    if (type_of(at(m.get_from())) == PieceType::King) {
        if (color_of(at(m.get_from())) == Color::White) {
#ifdef DEBUG_CASTLING
            std::cout << "[DEBUG] White king moves. Clearing WK/WQ rights." << std::endl;
#endif
            castling_rights &= ~(CASTLE_WK | CASTLE_WQ);
        } else {
#ifdef DEBUG_CASTLING
            std::cout << "[DEBUG] Black king moves. Clearing BK/BQ rights." << std::endl;
#endif
            castling_rights &= ~(CASTLE_BK | CASTLE_BQ);
        }
    }
    // Rook moves
    if (type_of(at(m.get_from())) == PieceType::Rook) {
        if (color_of(at(m.get_from())) == Color::White) {
            if (m.get_from() == sq(File::A, Rank::R1)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] White rook moves from a1. Clearing WQ right." << std::endl;
#endif
                castling_rights &= ~CASTLE_WQ;
            }
            if (m.get_from() == sq(File::H, Rank::R1)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] White rook moves from h1. Clearing WK right." << std::endl;
#endif
                castling_rights &= ~CASTLE_WK;
            }
        } else {
            if (m.get_from() == sq(File::A, Rank::R8)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] Black rook moves from a8. Clearing BQ right." << std::endl;
#endif
                castling_rights &= ~CASTLE_BQ;
            }
            if (m.get_from() == sq(File::H, Rank::R8)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] Black rook moves from h8. Clearing BK right." << std::endl;
#endif
                castling_rights &= ~CASTLE_BK;
            }
        }
    }
    // Rook captured
    if (type_of(undo.captured) == PieceType::Rook) {
        if (color_of(undo.captured) == Color::White) {
            if (m.get_to() == sq(File::A, Rank::R1)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] White rook captured on a1. Clearing WQ right." << std::endl;
#endif
                castling_rights &= ~CASTLE_WQ;
            }
            if (m.get_to() == sq(File::H, Rank::R1)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] White rook captured on h1. Clearing WK right." << std::endl;
#endif
                castling_rights &= ~CASTLE_WK;
            }
        } else {
            if (m.get_to() == sq(File::A, Rank::R8)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] Black rook captured on a8. Clearing BQ right." << std::endl;
#endif
                castling_rights &= ~CASTLE_BQ;
            }
            if (m.get_to() == sq(File::H, Rank::R8)) {
#ifdef DEBUG_CASTLING
                std::cout << "[DEBUG] Black rook captured on h8. Clearing BK right." << std::endl;
#endif
                castling_rights &= ~CASTLE_BK;
            }
        }
    }
    // Explicit castling move: clear both king and rook rights for the moving color
    if (m.is_castle()) {
#ifdef DEBUG_CASTLING
        std::cout << "[DEBUG] Castling move performed. Clearing rights for " << (color_of(at(m.get_from())) == Color::White ? "White" : "Black") << std::endl;
#endif
        if (color_of(at(m.get_from())) == Color::White) {
            castling_rights &= ~(CASTLE_WK | CASTLE_WQ);
            // Move rook in piece list for White
            if (m.get_to() == sq(File::G, Rank::R1)) { // White kingside castle e1g1
                move_piece_in_list(Color::White, PieceType::Rook, sq(File::H, Rank::R1), sq(File::F, Rank::R1));
                board[sq(File::H, Rank::R1)] = Piece::None;
                board[sq(File::F, Rank::R1)] = Piece::WhiteRook;
            } else if (m.get_to() == sq(File::C, Rank::R1)) { // White queenside castle e1c1
                move_piece_in_list(Color::White, PieceType::Rook, sq(File::A, Rank::R1), sq(File::C, Rank::R1));
                board[sq(File::A, Rank::R1)] = Piece::None;
                board[sq(File::C, Rank::R1)] = Piece::WhiteRook;
            }
        } else {
            castling_rights &= ~(CASTLE_BK | CASTLE_BQ);
            // Move rook in piece list for Black
            if (m.get_to() == sq(File::G, Rank::R8)) { // Black kingside castle e8g8
                move_piece_in_list(Color::Black, PieceType::Rook, sq(File::H, Rank::R8), sq(File::F, Rank::R8));
                board[sq(File::H, Rank::R8)] = Piece::None;
                board[sq(File::F, Rank::R8)] = Piece::BlackRook;
            } else if (m.get_to() == sq(File::C, Rank::R8)) { // Black queenside castle e8c8
                move_piece_in_list(Color::Black, PieceType::Rook, sq(File::A, Rank::R8), sq(File::D, Rank::R8));
                board[sq(File::A, Rank::R8)] = Piece::None;
                board[sq(File::D, Rank::R8)] = Piece::BlackRook;
            }
        }
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
    // Increment ply after saving undo info
    ++ply;
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
}

void Position::reset() {
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
    piece_counts.fill(0);
    material_score[0] = 0;
    material_score[1] = 0;
    pawns_bb[0] = 0ULL;
    pawns_bb[1] = 0ULL;
    all_pawns_bb = 0ULL;
    for (int color = 0; color < 2; ++color) {
        for (int type = 0; type < int(PieceType::_Count); ++type) {
            pCount[color][type] = 0;
            for (int i = 0; i < MAX_PIECES_PER_TYPE; ++i) {
                pList[color][type][i] = -1;
            }
        }
    }
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

bool Position::set_from_fen(const std::string& fen) {
    reset();
    std::vector<std::string> tokens;
    std::istringstream iss(fen);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    if (tokens.size() != 6) {
        return false;
    }
    // Piece placement: basic validation
    const std::string& placement = tokens[0];
    int rank = 7;
    int file = 0;
    for (char ch : placement) {
        if (ch == '/') {
            if (file != 8) return false;
            rank--;
            file = 0;
        } else if (isdigit(ch)) {
            file += ch - '0';
        } else if (from_char(ch) == Piece::None) {
            return false;
        } else {
            int square = sq(static_cast<File>(file), static_cast<Rank>(rank));
            board[square] = from_char(ch);
            file++;
        }
    }
    if (rank != 0 || file != 8) return false;

    // Side to move: must be "w" or "b"
    if (tokens[1] == "w") {
        side_to_move = Color::White;
    } else if (tokens[1] == "b") {
        side_to_move = Color::Black;
    } else {
        return false;
    }

    // Castling rights: must be subset of "KQkq" or "-"
    castling_rights = 0;
    if (tokens[2] == "-") {
        // No rights
    } else {
        for (char c : tokens[2]) {
            if (c == 'K') castling_rights |= CASTLE_WK;
            else if (c == 'Q') castling_rights |= CASTLE_WQ;
            else if (c == 'k') castling_rights |= CASTLE_BK;
            else if (c == 'q') castling_rights |= CASTLE_BQ;
            else return false;
        }
    }

    // En passant: must be "-" or valid square a3/a6/h3/h6 etc.
    if (tokens[3] == "-") {
        ep_square = -1;
    } else if (tokens[3].size() == 2 && tokens[3][0] >= 'a' && tokens[3][0] <= 'h' && (tokens[3][1] == '3' || tokens[3][1] == '6')) {
        File file = File(tokens[3][0] - 'a');
        Rank rank = Rank(tokens[3][1] - '1');
        ep_square = sq(file, rank);
    } else {
        return false;
    }

    // Move counters: must be valid integers
    try {
        halfmove_clock = std::stoi(tokens[4]);
        fullmove_number = std::stoi(tokens[5]);
    } catch (...) {
        return false;
    }

    rebuild_counts();
    update_zobrist_key();
    return true;
}

std::string Position::to_fen() const {
    std::string fen;
    for (int rank = 7; rank >= 0; --rank) {
        int empty_count = 0;
        for (int file = 0; file < 8; ++file) {
            int sq120 = sq(static_cast<File>(file), static_cast<Rank>(rank));
            Piece piece = board[sq120];
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
        File file = file_of(ep_square);
        Rank rank = rank_of(ep_square);
        fen += char('a' + int(file));
        fen += char('1' + int(rank));
    }
    fen += ' ';
    fen += std::to_string(halfmove_clock);
    fen += ' ';
    fen += std::to_string(fullmove_number);
    return fen;
}

void Position::save_derived_state(S_UNDO& undo) {
    undo.king_sq_backup = king_sq;
    undo.pawns_bb_backup = pawns_bb;
    undo.all_pawns_bb_backup = all_pawns_bb;
    undo.piece_counts_backup = piece_counts;
    undo.material_score_backup = material_score;
    undo.pList_backup = pList;
    undo.pCount_backup = pCount;
}

void Position::rebuild_counts() {
    // Clear all piece lists and counts
    for (int color = 0; color < 2; ++color) {
        for (int type = 0; type < int(PieceType::_Count); ++type) {
            pCount[color][type] = 0;
            for (int i = 0; i < MAX_PIECES_PER_TYPE; ++i) {
                pList[color][type][i] = -1;
            }
        }
    }
    piece_counts.fill(0);
    material_score[0] = 0;
    material_score[1] = 0;
    pawns_bb[0] = 0ULL;
    pawns_bb[1] = 0ULL;
    all_pawns_bb = 0ULL;
    king_sq[0] = -1;
    king_sq[1] = -1;

    // Scan board and update all derived state
    for (int sq120 = 0; sq120 < 120; ++sq120) {
        Piece piece = board[sq120];
        if (!is_playable(sq120) || is_none(piece) || piece == Piece::Offboard) continue;
        Color color = color_of(piece);
        PieceType type = type_of(piece);
        int color_idx = int(color);
        int type_idx = int(type);
        int idx = pCount[color_idx][type_idx];
        pList[color_idx][type_idx][idx] = sq120;
        pCount[color_idx][type_idx]++;
        piece_counts[type_idx]++;
        if (type == PieceType::King) {
            king_sq[color_idx] = sq120;
        }
        if (type == PieceType::Pawn) {
            int s64 = MAILBOX_MAPS.to64[sq120];
            if (s64 >= 0) {
                pawns_bb[color_idx] |= (1ULL << s64);
                all_pawns_bb |= (1ULL << s64);
            }
        }
        if (type != PieceType::King) {
            material_score[color_idx] += value_of(piece);
        }
    }
}

// Set up the standard chess starting position using FEN
void Position::set_startpos() {
    const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    set_from_fen(start_fen);
}
