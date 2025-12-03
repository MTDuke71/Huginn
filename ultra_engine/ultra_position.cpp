/**
 * @file ultra_position.cpp
 * @brief Implementation of ultra-optimized chess position system
 */

#include "ultra_position.hpp"
#include <sstream>
#include <cctype>

namespace UltraEngine {

// ============================================================================
// FEN PARSING AND GENERATION
// ============================================================================

bool UltraPosition::set_fen(const std::string& fen) {
    clear();
    
    std::istringstream iss(fen);
    std::string board, color, castling, en_passant, halfmove, fullmove;
    
    if (!(iss >> board >> color >> castling >> en_passant >> halfmove >> fullmove)) {
        return false; // Invalid FEN format
    }
    
    // Parse board position
    int rank = 7, file = 0;
    for (char c : board) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (std::isdigit(c)) {
            file += c - '0'; // Skip empty squares
        } else {
            int piece_color = std::isupper(c) ? White : Black;
            int piece_type = char_to_piece(std::tolower(c));
            if (piece_type >= 0) {
                set_piece(rank * 8 + file, piece_color, piece_type);
            }
            file++;
        }
        
        if (file > 8 || rank < 0) return false; // Invalid board
    }
    
    // Parse side to move
    to_move_ = (color == "w") ? White : Black;
    
    // Parse castling rights
    castling_rights_ = 0;
    for (char c : castling) {
        switch (c) {
            case 'K': castling_rights_ |= 1; break; // White kingside
            case 'Q': castling_rights_ |= 2; break; // White queenside
            case 'k': castling_rights_ |= 4; break; // Black kingside
            case 'q': castling_rights_ |= 8; break; // Black queenside
        }
    }
    
    // Parse en passant square
    if (en_passant != "-") {
        if (en_passant.length() == 2) {
            int file_ep = en_passant[0] - 'a';
            int rank_ep = en_passant[1] - '1';
            en_passant_square_ = rank_ep * 8 + file_ep;
        }
    }
    
    // Parse move counters
    halfmove_clock_ = std::stoi(halfmove);
    fullmove_number_ = std::stoi(fullmove);
    
    // Rebuild derived bitboards
    rebuild_derived_bitboards();
    
    return true;
}

std::string UltraPosition::get_fen() const {
    std::ostringstream oss;
    
    // Board position
    for (int rank = 7; rank >= 0; rank--) {
        int empty_count = 0;
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int piece_info = get_piece_at(square);
            
            if (piece_info < 0) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    oss << empty_count;
                    empty_count = 0;
                }
                int color = piece_info >> 3;
                int piece = piece_info & 7;
                oss << piece_to_char(color, piece);
            }
        }
        
        if (empty_count > 0) {
            oss << empty_count;
        }
        
        if (rank > 0) oss << '/';
    }
    
    // Side to move
    oss << ' ' << (to_move_ == White ? 'w' : 'b');
    
    // Castling rights
    oss << ' ';
    bool has_castling = false;
    if (castling_rights_ & 1) { oss << 'K'; has_castling = true; }
    if (castling_rights_ & 2) { oss << 'Q'; has_castling = true; }
    if (castling_rights_ & 4) { oss << 'k'; has_castling = true; }
    if (castling_rights_ & 8) { oss << 'q'; has_castling = true; }
    if (!has_castling) oss << '-';
    
    // En passant square
    oss << ' ';
    if (en_passant_square_ >= 0) {
        int file = en_passant_square_ % 8;
        int rank = en_passant_square_ / 8;
        oss << char('a' + file) << char('1' + rank);
    } else {
        oss << '-';
    }
    
    // Move counters
    oss << ' ' << halfmove_clock_ << ' ' << fullmove_number_;
    
    return oss.str();
}

// ============================================================================
// KING MOVE GENERATION HELPERS
// ============================================================================

int UltraPosition::generate_king_captures(UltraMoveList& moves, int color) const {
    const uint64_t king_bb = piece_boards_[color][King];
    if (!king_bb) return 0;
    
    const int king_square = get_lsb(king_bb);
    const uint64_t attacks = UltraAttacks::king(king_square);
    const uint64_t enemy_pieces = color_boards_[1 - color];
    
    int count = 0;
    uint64_t capture_targets = attacks & enemy_pieces;
    
    while (capture_targets) {
        const int to_square = pop_lsb(capture_targets);
        moves.add_capture(king_square, to_square, 1);
        count++;
    }
    
    return count;
}

int UltraPosition::generate_king_quiet(UltraMoveList& moves, int color) const {
    const uint64_t king_bb = piece_boards_[color][King];
    if (!king_bb) return 0;
    
    const int king_square = get_lsb(king_bb);
    const uint64_t attacks = UltraAttacks::king(king_square);
    const uint64_t all_pieces = all_occupied_;
    
    int count = 0;
    uint64_t quiet_targets = attacks & ~all_pieces;
    
    while (quiet_targets) {
        const int to_square = pop_lsb(quiet_targets);
        moves.add_quiet(king_square, to_square);
        count++;
    }
    
    return count;
}

// ============================================================================
// CASTLING HELPERS
// ============================================================================

int UltraPosition::generate_castling_moves(UltraMoveList& moves, int color) const {
    int count = 0;
    
    if (color == White) {
        // White kingside castling
        if ((castling_rights_ & 1) && 
            !(all_occupied_ & 0x60ULL) && // f1, g1 empty
            !UltraAttacks::is_square_attacked(4, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // e1 safe
            !UltraAttacks::is_square_attacked(5, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // f1 safe
            !UltraAttacks::is_square_attacked(6, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) { // g1 safe
            moves.add_castle(4, 6); // e1-g1
            count++;
        }
        
        // White queenside castling
        if ((castling_rights_ & 2) && 
            !(all_occupied_ & 0xEULL) && // b1, c1, d1 empty
            !UltraAttacks::is_square_attacked(4, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // e1 safe
            !UltraAttacks::is_square_attacked(3, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // d1 safe
            !UltraAttacks::is_square_attacked(2, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) { // c1 safe
            moves.add_castle(4, 2); // e1-c1
            count++;
        }
    } else {
        // Black kingside castling
        if ((castling_rights_ & 4) && 
            !(all_occupied_ & 0x6000000000000000ULL) && // f8, g8 empty
            !UltraAttacks::is_square_attacked(60, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // e8 safe
            !UltraAttacks::is_square_attacked(61, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // f8 safe
            !UltraAttacks::is_square_attacked(62, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) { // g8 safe
            moves.add_castle(60, 62); // e8-g8
            count++;
        }
        
        // Black queenside castling
        if ((castling_rights_ & 8) && 
            !(all_occupied_ & 0xE00000000000000ULL) && // b8, c8, d8 empty
            !UltraAttacks::is_square_attacked(60, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // e8 safe
            !UltraAttacks::is_square_attacked(59, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) && // d8 safe
            !UltraAttacks::is_square_attacked(58, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) { // c8 safe
            moves.add_castle(60, 58); // e8-c8
            count++;
        }
    }
    
    return count;
}

int UltraPosition::count_castling_moves(int color) const {
    int count = 0;
    
    if (color == White) {
        // White kingside castling
        if ((castling_rights_ & 1) && 
            !(all_occupied_ & 0x60ULL) &&
            !UltraAttacks::is_square_attacked(4, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(5, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(6, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) {
            count++;
        }
        
        // White queenside castling
        if ((castling_rights_ & 2) && 
            !(all_occupied_ & 0xEULL) &&
            !UltraAttacks::is_square_attacked(4, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(3, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(2, Black, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) {
            count++;
        }
    } else {
        // Black kingside castling
        if ((castling_rights_ & 4) && 
            !(all_occupied_ & 0x6000000000000000ULL) &&
            !UltraAttacks::is_square_attacked(60, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(61, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(62, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) {
            count++;
        }
        
        // Black queenside castling
        if ((castling_rights_ & 8) && 
            !(all_occupied_ & 0xE00000000000000ULL) &&
            !UltraAttacks::is_square_attacked(60, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(59, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_) &&
            !UltraAttacks::is_square_attacked(58, White, (const uint64_t(*)[6])piece_boards_.data(), all_occupied_)) {
            count++;
        }
    }
    
    return count;
}

void UltraPosition::handle_castling(int from, int to) {
    // Move the rook during castling
    if (from == 4 && to == 6) {
        // White kingside: move rook from h1 to f1
        remove_piece(7);
        set_piece(5, White, Rook);
    } else if (from == 4 && to == 2) {
        // White queenside: move rook from a1 to d1
        remove_piece(0);
        set_piece(3, White, Rook);
    } else if (from == 60 && to == 62) {
        // Black kingside: move rook from h8 to f8
        remove_piece(63);
        set_piece(61, Black, Rook);
    } else if (from == 60 && to == 58) {
        // Black queenside: move rook from a8 to d8
        remove_piece(56);
        set_piece(59, Black, Rook);
    }
}

void UltraPosition::handle_uncastling(int from, int to) {
    // Restore the rook during uncastling
    if (from == 4 && to == 6) {
        // White kingside: move rook back from f1 to h1
        remove_piece(5);
        set_piece(7, White, Rook);
    } else if (from == 4 && to == 2) {
        // White queenside: move rook back from d1 to a1
        remove_piece(3);
        set_piece(0, White, Rook);
    } else if (from == 60 && to == 62) {
        // Black kingside: move rook back from f8 to h8
        remove_piece(61);
        set_piece(63, Black, Rook);
    } else if (from == 60 && to == 58) {
        // Black queenside: move rook back from d8 to a8
        remove_piece(59);
        set_piece(56, Black, Rook);
    }
}

void UltraPosition::update_castling_rights(int from, int to, int piece_type) {
    // Remove castling rights when king or rook moves
    if (piece_type == King) {
        if (from == 4) castling_rights_ &= ~3; // White king moved
        if (from == 60) castling_rights_ &= ~12; // Black king moved
    } else if (piece_type == Rook) {
        if (from == 0) castling_rights_ &= ~2; // White queenside rook moved
        if (from == 7) castling_rights_ &= ~1; // White kingside rook moved
        if (from == 56) castling_rights_ &= ~8; // Black queenside rook moved
        if (from == 63) castling_rights_ &= ~4; // Black kingside rook moved
    }
    
    // Remove castling rights when rook is captured
    if (to == 0) castling_rights_ &= ~2; // White queenside rook captured
    if (to == 7) castling_rights_ &= ~1; // White kingside rook captured
    if (to == 56) castling_rights_ &= ~8; // Black queenside rook captured
    if (to == 63) castling_rights_ &= ~4; // Black kingside rook captured
}

// ============================================================================
// GAME STATE QUERIES
// ============================================================================

bool UltraPosition::is_legal_move(const UltraMove& move) const {
    // Make a copy and test the move
    UltraPosition temp = *this;
    const int moving_color = temp.to_move_;  // Save the color that is about to move
    temp.make_move(move);
    
    // Check if the moving side's king is in check after the move
    // After make_move, to_move_ has switched, so we check the king of (1 - to_move_)
    const int king_square = UltraEngine::get_lsb(temp.piece_boards_[moving_color][King]);
    return !UltraAttacks::is_square_attacked(king_square, 1 - moving_color, 
                                             (const uint64_t(*)[6])temp.piece_boards_.data(), temp.all_occupied_);
}

bool UltraPosition::is_checkmate() const {
    if (!is_in_check()) return false;
    
    UltraMoveList moves;
    int move_count = generate_all_moves(moves);
    
    // Check if any move is legal
    for (int i = 0; i < move_count; i++) {
        if (is_legal_move(moves[i])) {
            return false;
        }
    }
    
    return true;
}

bool UltraPosition::is_stalemate() const {
    if (is_in_check()) return false;
    
    UltraMoveList moves;
    int move_count = generate_all_moves(moves);
    
    // Check if any move is legal
    for (int i = 0; i < move_count; i++) {
        if (is_legal_move(moves[i])) {
            return false;
        }
    }
    
    return true;
}

bool UltraPosition::is_draw() const {
    // 50-move rule
    if (halfmove_clock_ >= 100) return true;
    
    // Insufficient material (basic check)
    const int white_pieces = popcount(color_boards_[White]);
    const int black_pieces = popcount(color_boards_[Black]);
    
    if (white_pieces <= 2 && black_pieces <= 2) {
        // Only kings, or king + minor piece
        const bool white_has_major = piece_boards_[White][Queen] || piece_boards_[White][Rook];
        const bool black_has_major = piece_boards_[Black][Queen] || piece_boards_[Black][Rook];
        const bool white_has_pawns = piece_boards_[White][Pawn];
        const bool black_has_pawns = piece_boards_[Black][Pawn];
        
        if (!white_has_major && !black_has_major && !white_has_pawns && !black_has_pawns) {
            return true; // Only kings and minor pieces
        }
    }
    
    return is_stalemate();
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

int UltraPosition::char_to_piece(char c) const {
    switch (c) {
        case 'p': return Pawn;
        case 'n': return Knight;
        case 'b': return Bishop;
        case 'r': return Rook;
        case 'q': return Queen;
        case 'k': return King;
        default: return -1;
    }
}

char UltraPosition::piece_to_char(int color, int piece) const {
    const char pieces[] = "pnbrqk";
    char c = pieces[piece];
    return (color == White) ? std::toupper(c) : c;
}

} // namespace UltraEngine