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
