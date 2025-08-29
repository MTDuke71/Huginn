// position.cpp - Implementation file for Position class methods
#include "position.hpp"
#include "zobrist.hpp"
#include "movegen_enhanced.hpp"  // For in_check function
#include "attack_detection.hpp"  // For SqAttacked function

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

// VICE Tutorial Video #41: MakeMove function
// Returns 1 if move is legal, 0 if illegal (leaves king in check)
int Position::MakeMove(const S_MOVE& move) {
    // Debug assertions for move validity  
    DEBUG_ASSERT(is_playable(move.get_from()), "Move source square must be playable");
    DEBUG_ASSERT(is_playable(move.get_to()), "Move destination square must be playable");
    
    // Store history information for potential undo
    int from = move.get_from();
    int to = move.get_to();
    
    // Hash out the current position state before making move
    zobrist_key ^= Zobrist::Side;  // Hash out current side
    
    // Hash out en passant square if set
    if (ep_square != -1) {
        zobrist_key ^= Zobrist::EpFile[int(file_of(ep_square))];
    }
    
    // Hash out current castling rights
    zobrist_key ^= Zobrist::Castle[castling_rights];
    
    // Store history before making the move
    if (ply >= static_cast<int>(move_history.size())) {
        move_history.resize(ply + 1);
    }
    
    S_UNDO& undo = move_history[ply];
    undo.move = move;
    undo.castling_rights = castling_rights;
    undo.ep_square = ep_square; 
    undo.halfmove_clock = halfmove_clock;
    undo.zobrist_key = zobrist_key;
    
    // Set captured piece - handle en passant special case
    if (move.is_en_passant()) {
        Color moving_color = color_of(at(from));
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            captured_pawn_sq = to + SOUTH;  // Black pawn south of target
        } else {
            captured_pawn_sq = to + NORTH;  // White pawn north of target  
        }
        undo.captured = at(captured_pawn_sq);  // Capture the pawn that's actually being removed
    } else {
        undo.captured = at(to);  // Normal capture
    }
    
    // Save additional state needed for undo
    save_derived_state(undo);
    
    // Update castling rights based on move (from/to squares)
    castling_rights = CastlingLookup::update_castling_rights(castling_rights, from, to);
    
    // Reset en passant square 
    ep_square = -1;
    
    // Handle en passant captures
    if (move.is_en_passant()) {
        Color moving_color = color_of(at(from));
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            captured_pawn_sq = to + SOUTH;  // Black pawn south of target
        } else {
            captured_pawn_sq = to + NORTH;  // White pawn north of target  
        }
        
        // Clear the captured pawn using atomic operation
        clear_piece(captured_pawn_sq);
    }
    
    // Handle 50-move rule: reset on pawn move or capture
    Piece moving_piece = at(from);
    if (type_of(moving_piece) == PieceType::Pawn || !is_none(at(to))) {
        halfmove_clock = 0;
    } else {
        ++halfmove_clock;
    }
    
    // Increment ply counter
    ++ply;
    
    // Set en passant square for pawn double moves
    if (type_of(moving_piece) == PieceType::Pawn) {
        int from_rank = int(rank_of(from));
        int to_rank = int(rank_of(to));
        
        if (abs(to_rank - from_rank) == 2) {  // Pawn moved two squares
            int ep_rank = (from_rank + to_rank) / 2;
            ep_square = sq(file_of(to), Rank(ep_rank));
            
            // Hash in new en passant square
            zobrist_key ^= Zobrist::EpFile[int(file_of(ep_square))];
        }
    }
    
    // Make the actual move using atomic operations
    if (move.is_promotion()) {
        // Handle capture before promotion
        if (!is_none(at(to))) {
            clear_piece(to);  // Remove captured piece
        }
        clear_piece(from);  // Remove pawn
        add_piece(to, make_piece(color_of(moving_piece), move.get_promoted()));  // Add promoted piece
    } else {
        // Handle normal capture
        if (!is_none(at(to))) {
            clear_piece(to);  // Remove captured piece first
        }
        move_piece(from, to);  // Then move the piece
    }
    
    // Handle castling - move the rook
    if (move.is_castle()) {
        Color king_color = color_of(moving_piece);
        int rook_from, rook_to;
        
        if (king_color == Color::White) {
            if (to == sq(File::G, Rank::R1)) {  // White kingside
                rook_from = sq(File::H, Rank::R1);
                rook_to = sq(File::F, Rank::R1);
            } else {  // White queenside
                rook_from = sq(File::A, Rank::R1);
                rook_to = sq(File::D, Rank::R1);
            }
        } else {
            if (to == sq(File::G, Rank::R8)) {  // Black kingside
                rook_from = sq(File::H, Rank::R8);
                rook_to = sq(File::F, Rank::R8);
            } else {  // Black queenside
                rook_from = sq(File::A, Rank::R8);
                rook_to = sq(File::D, Rank::R8);
            }
        }
        
        // Move the rook using atomic operation
        move_piece(rook_from, rook_to);
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
    
    // Hash in new position state
    zobrist_key ^= Zobrist::Side;  // Hash in new side
    zobrist_key ^= Zobrist::Castle[castling_rights];  // Hash in new castling rights
    
    // Update derived state incrementally  
    update_derived_state_for_move(move, moving_piece, undo.captured);
    
    // Check if move left current player's king in check
    // Note: side_to_move has already been flipped, so we check the previous side's king
    Color previous_side = !side_to_move;  
    int king_square = king_sq[int(previous_side)];
    
    // Use SqAttacked directly to check if king is attacked by current side
    if (SqAttacked(king_square, *this, side_to_move)) {
        // Move is illegal - undo it
        TakeMove();
        return 0;  // Illegal move
    }
    
    return 1;  // Legal move
}

// VICE Tutorial Video #42: TakeMove function
// Undoes the last move by retrieving information from the history array
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
    
    // Hash out current position state
    zobrist_key ^= Zobrist::Side;  // Hash out current side
    
    // Hash out current en passant square if set
    if (ep_square != -1) {
        zobrist_key ^= Zobrist::EpFile[int(file_of(ep_square))];
    }
    
    // Hash out current castling rights
    zobrist_key ^= Zobrist::Castle[castling_rights];
    
    // Restore position state from history
    castling_rights = undo.castling_rights;
    ep_square = undo.ep_square;
    halfmove_clock = undo.halfmove_clock;
    zobrist_key = undo.zobrist_key;  // Restore complete zobrist state
    
    // Handle en passant undo - restore captured pawn
    if (move.is_en_passant()) {
        Color moving_color = color_of(at(to));
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            captured_pawn_sq = to + SOUTH;  // Black pawn was south of target
        } else {
            captured_pawn_sq = to + NORTH;  // White pawn was north of target
        }
        
        // Restore the captured pawn (stored in undo.captured)
        add_piece(captured_pawn_sq, undo.captured);
    }
    
    // Handle castling undo - move rook back to original position
    if (move.is_castle()) {
        Color king_color = color_of(at(to));
        int rook_from, rook_to;
        
        if (king_color == Color::White) {
            if (to == sq(File::G, Rank::R1)) {  // White kingside
                rook_from = sq(File::H, Rank::R1);  // Original rook position
                rook_to = sq(File::F, Rank::R1);    // Current rook position
            } else {  // White queenside
                rook_from = sq(File::A, Rank::R1);  // Original rook position
                rook_to = sq(File::D, Rank::R1);    // Current rook position
            }
        } else {
            if (to == sq(File::G, Rank::R8)) {  // Black kingside
                rook_from = sq(File::H, Rank::R8);  // Original rook position
                rook_to = sq(File::F, Rank::R8);    // Current rook position
            } else {  // Black queenside
                rook_from = sq(File::A, Rank::R8);  // Original rook position
                rook_to = sq(File::D, Rank::R8);    // Current rook position
            }
        }
        
        // Move rook back to original position
        move_piece(rook_to, rook_from);
    }
    
    // Move piece back to original square BEFORE adding captured piece
    if (move.is_promotion()) {
        // For promotion, remove promoted piece and restore original pawn
        clear_piece(to);  // Remove promoted piece
        Color moving_color = color_of(undo.captured) == Color::None ? 
            (side_to_move == Color::White ? Color::Black : Color::White) : 
            (color_of(undo.captured) == Color::White ? Color::Black : Color::White);
        add_piece(from, make_piece(moving_color, PieceType::Pawn));  // Restore pawn
    } else {
        // Regular move - move piece back
        DEBUG_ASSERT(is_playable(to), "Invalid 'to' square in TakeMove");
        DEBUG_ASSERT(is_playable(from), "Invalid 'from' square in TakeMove");
        DEBUG_ASSERT(!is_none(at(to)), "No piece to move back from 'to' square");
        move_piece(to, from);
    }
    
    // Add captured piece back to destination square (if any)
    if (!is_none(undo.captured) && !move.is_en_passant()) {
        add_piece(to, undo.captured);
    }
    
    // Update king square if king moved
    Piece moved_piece = at(from);
    if (type_of(moved_piece) == PieceType::King) {
        king_sq[int(color_of(moved_piece))] = from;
    }
    
    // Flip side to move back
    side_to_move = !side_to_move;
    if (side_to_move == Color::Black) {
        --fullmove_number;
    }
    
    // Restore derived state from history (much faster than rebuilding)
    restore_derived_state(undo);
}
