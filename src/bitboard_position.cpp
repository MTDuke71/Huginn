/**
 * @file bitboard_position.cpp
 * @brief Implementation of pure 64-square bitboard position
 * 
 * Native bitboard implementation that eliminates all 120-square conversion overhead.
 * Provides maximum performance for bitboard-based move generation and evaluation.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"  // For BitboardMoveList
#include "bitboard.hpp"  // For bitboard utilities like pop_lsb, setBit, clearBit
#include "bitboard_attacks.hpp"  // For attack table initialization
#include <sstream>
#include <algorithm>

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::string square_to_algebraic(int square_64) {
    if (square_64 < 0 || square_64 >= 64) return "-";
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

int algebraic_to_square(const std::string& algebraic) {
    if (algebraic == "-" || algebraic.length() != 2) return -1;
    int file = algebraic[0] - 'a';
    int rank = algebraic[1] - '1';
    if (file < 0 || file >= 8 || rank < 0 || rank >= 8) return -1;
    return rank * 8 + file;
}

// ============================================================================
// CONSTRUCTION AND INITIALIZATION
// ============================================================================

BitboardPosition::BitboardPosition() {
    reset();
    // Ensure attack tables are initialized globally
    ensure_bitboard_attacks_initialized();
}

void BitboardPosition::reset() {
    // Clear all bitboards
    for (int color = 0; color < 2; ++color) {
        for (int piece_type = 0; piece_type < 7; ++piece_type) {
            piece_bitboards[color][piece_type] = 0ULL;
        }
        color_bitboards[color] = 0ULL;
        material_score[color] = 0;
        king_square_64[color] = -1;
    }
    
    occupied_bitboard = 0ULL;
    
    // Reset game state
    side_to_move = Color::White;
    ep_square_64 = -1;
    castling_rights = 0;
    halfmove_clock = 0;
    fullmove_number = 1;
    zobrist_key = 0ULL;
    ply = 0;
}

// ============================================================================
// SQUARE ACCESS (64-SQUARE NATIVE)
// ============================================================================

Piece BitboardPosition::piece_at(int square_64) const {
    if (square_64 < 0 || square_64 >= 64) return Piece::None;
    
    uint64_t square_bb = 1ULL << square_64;
    
    // Check which color occupies this square
    Color piece_color = Color::None;
    if (color_bitboards[0] & square_bb) {
        piece_color = Color::White;
    } else if (color_bitboards[1] & square_bb) {
        piece_color = Color::Black;
    } else {
        return Piece::None;  // Empty square
    }
    
    // Find piece type
    int color_idx = static_cast<int>(piece_color);
    for (int piece_type = 1; piece_type < 7; ++piece_type) {  // Skip None (0)
        if (piece_bitboards[color_idx][piece_type] & square_bb) {
            // Reconstruct piece using the encoded format
            return static_cast<Piece>((static_cast<uint8_t>(piece_color) << 3) | piece_type);
        }
    }
    
    return Piece::None;  // Should not reach here
}

Color BitboardPosition::color_at(int square_64) const {
    if (square_64 < 0 || square_64 >= 64) return Color::None;
    
    uint64_t square_bb = 1ULL << square_64;
    if (color_bitboards[0] & square_bb) return Color::White;
    if (color_bitboards[1] & square_bb) return Color::Black;
    return Color::None;
}

PieceType BitboardPosition::piece_type_at(int square_64) const {
    if (square_64 < 0 || square_64 >= 64) return PieceType::None;
    
    uint64_t square_bb = 1ULL << square_64;
    
    // Check which piece type occupies this square
    for (int color = 0; color < 2; ++color) {
        for (int piece_type = 1; piece_type < 7; ++piece_type) {  // Skip None (0)
            if (piece_bitboards[color][piece_type] & square_bb) {
                return static_cast<PieceType>(piece_type);
            }
        }
    }
    
    return PieceType::None;
}

// ============================================================================
// PIECE MANIPULATION
// ============================================================================

void BitboardPosition::place_piece(int square_64, Color color, PieceType piece_type) {
    if (square_64 < 0 || square_64 >= 64 || color == Color::None || piece_type == PieceType::None) {
        return;
    }
    
    uint64_t square_bb = 1ULL << square_64;
    int color_idx = static_cast<int>(color);
    int piece_idx = static_cast<int>(piece_type);
    
    // Set piece in appropriate bitboards
    piece_bitboards[color_idx][piece_idx] |= square_bb;
    color_bitboards[color_idx] |= square_bb;
    occupied_bitboard |= square_bb;
    
    // Update king position if placing king
    if (piece_type == PieceType::King) {
        king_square_64[color_idx] = square_64;
    }
    
    // Update material score (basic piece values)
    constexpr int piece_values[] = {0, 100, 320, 330, 500, 900, 20000, 0};  // None, P, N, B, R, Q, K
    material_score[color_idx] += piece_values[piece_idx];
}

void BitboardPosition::remove_piece(int square_64) {
    if (square_64 < 0 || square_64 >= 64) return;
    
    uint64_t square_bb = 1ULL << square_64;
    uint64_t clear_mask = ~square_bb;
    
    // Find and remove piece from appropriate bitboards
    for (int color = 0; color < 2; ++color) {
        if (color_bitboards[color] & square_bb) {
            color_bitboards[color] &= clear_mask;
            
            for (int piece_type = 1; piece_type < 7; ++piece_type) {
                if (piece_bitboards[color][piece_type] & square_bb) {
                    piece_bitboards[color][piece_type] &= clear_mask;
                    
                    // Update material score
                    constexpr int piece_values[] = {0, 100, 320, 330, 500, 900, 20000, 0};
                    material_score[color] -= piece_values[piece_type];
                    
                    // Clear king position if removing king
                    if (piece_type == static_cast<int>(PieceType::King)) {
                        king_square_64[color] = -1;
                    }
                    break;
                }
            }
            break;
        }
    }
    
    occupied_bitboard &= clear_mask;
}

void BitboardPosition::move_piece(int from_64, int to_64) {
    if (from_64 < 0 || from_64 >= 64 || to_64 < 0 || to_64 >= 64) return;
    
    // Get piece info before moving
    Color piece_color = color_at(from_64);
    PieceType piece_type = piece_type_at(from_64);
    
    if (piece_color == Color::None || piece_type == PieceType::None) return;
    
    // Remove any piece at destination
    remove_piece(to_64);
    
    // Remove piece from source
    remove_piece(from_64);
    
    // Place piece at destination
    place_piece(to_64, piece_color, piece_type);
}

// ============================================================================
// BITBOARD ACCESSORS
// ============================================================================

uint64_t BitboardPosition::get_pieces(Color color, PieceType piece_type) const {
    if (color == Color::None || piece_type == PieceType::None) return 0ULL;
    return piece_bitboards[static_cast<int>(color)][static_cast<int>(piece_type)];
}

uint64_t BitboardPosition::get_all_pieces(Color color) const {
    if (color == Color::None) return 0ULL;
    return color_bitboards[static_cast<int>(color)];
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool BitboardPosition::is_square_occupied(int square_64) const {
    if (square_64 < 0 || square_64 >= 64) return false;
    return (occupied_bitboard & (1ULL << square_64)) != 0;
}

bool BitboardPosition::is_enemy_piece(int square_64, Color our_color) const {
    if (!is_square_occupied(square_64) || our_color == Color::None) return false;
    Color piece_color = color_at(square_64);
    return piece_color != Color::None && piece_color != our_color;
}

bool BitboardPosition::is_our_piece(int square_64, Color our_color) const {
    if (!is_square_occupied(square_64) || our_color == Color::None) return false;
    return color_at(square_64) == our_color;
}

// ============================================================================
// INTERNAL BITBOARD MANAGEMENT
// ============================================================================

void BitboardPosition::update_color_bitboards() {
    color_bitboards[0] = 0ULL;
    color_bitboards[1] = 0ULL;
    
    for (int color = 0; color < 2; ++color) {
        for (int piece_type = 1; piece_type < 7; ++piece_type) {
            color_bitboards[color] |= piece_bitboards[color][piece_type];
        }
    }
}

void BitboardPosition::update_occupied_bitboard() {
    occupied_bitboard = color_bitboards[0] | color_bitboards[1];
}

void BitboardPosition::update_derived_bitboards() {
    update_color_bitboards();
    update_occupied_bitboard();
}

// ============================================================================
// FEN SUPPORT (Basic Implementation)
// ============================================================================

bool BitboardPosition::set_from_fen(const std::string& fen) {
    reset();
    
    std::istringstream iss(fen);
    std::string board_part, metadata_part;
    std::getline(iss, board_part, ' ');
    std::getline(iss, metadata_part);
    
    return parse_fen_board(board_part) && parse_fen_metadata(metadata_part);
}

bool BitboardPosition::parse_fen_board(const std::string& board_part) {
    int square_64 = 56;  // Start at A8 (top-left)
    
    for (char c : board_part) {
        if (c == '/') {
            square_64 -= 16;  // Move to next rank down (subtract 16, add 8 for next rank)
            continue;
        }
        
        if (std::isdigit(c)) {
            square_64 += (c - '0');  // Skip empty squares
        } else {
            // Place piece
            Color color = std::isupper(c) ? Color::White : Color::Black;
            PieceType piece_type = PieceType::None;
            
            switch (std::tolower(c)) {
                case 'p': piece_type = PieceType::Pawn; break;
                case 'n': piece_type = PieceType::Knight; break;
                case 'b': piece_type = PieceType::Bishop; break;
                case 'r': piece_type = PieceType::Rook; break;
                case 'q': piece_type = PieceType::Queen; break;
                case 'k': piece_type = PieceType::King; break;
                default: return false;
            }
            
            place_piece(square_64, color, piece_type);
            square_64++;
        }
        
        if (square_64 < 0) break;
    }
    
    update_derived_bitboards();
    return true;
}

bool BitboardPosition::parse_fen_metadata(const std::string& metadata_part) {
    std::istringstream iss(metadata_part);
    std::string side, castling, ep, halfmove, fullmove;
    
    iss >> side >> castling >> ep >> halfmove >> fullmove;
    
    // Parse side to move
    side_to_move = (side == "w") ? Color::White : Color::Black;
    
    // Parse castling rights (simplified)
    castling_rights = 0;
    if (castling.find('K') != std::string::npos) castling_rights |= 1;
    if (castling.find('Q') != std::string::npos) castling_rights |= 2;
    if (castling.find('k') != std::string::npos) castling_rights |= 4;
    if (castling.find('q') != std::string::npos) castling_rights |= 8;
    
    // Parse en passant
    ep_square_64 = algebraic_to_square(ep);
    
    // Parse move clocks
    halfmove_clock = !halfmove.empty() ? std::stoi(halfmove) : 0;
    fullmove_number = !fullmove.empty() ? std::stoi(fullmove) : 1;
    
    return true;
}

std::string BitboardPosition::to_fen() const {
    // Basic FEN generation - placeholder implementation
    std::ostringstream fen;
    
    // Board representation
    for (int rank = 7; rank >= 0; --rank) {
        int empty_count = 0;
        for (int file = 0; file < 8; ++file) {
            int square_64 = rank * 8 + file;
            Piece piece = piece_at(square_64);
            
            if (piece == Piece::None) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen << empty_count;
                    empty_count = 0;
                }
                fen << to_char(piece);
            }
        }
        if (empty_count > 0) fen << empty_count;
        if (rank > 0) fen << '/';
    }
    
    // Metadata
    fen << ' ' << (side_to_move == Color::White ? 'w' : 'b');
    fen << ' ';
    
    // Castling (simplified)
    bool has_castling = false;
    if (castling_rights & 1) { fen << 'K'; has_castling = true; }
    if (castling_rights & 2) { fen << 'Q'; has_castling = true; }
    if (castling_rights & 4) { fen << 'k'; has_castling = true; }
    if (castling_rights & 8) { fen << 'q'; has_castling = true; }
    if (!has_castling) fen << '-';
    
    fen << ' ' << (ep_square_64 == -1 ? "-" : square_to_algebraic(ep_square_64));
    fen << ' ' << halfmove_clock << ' ' << fullmove_number;
    
    return fen.str();
}

// ============================================================================
// MOVE OPERATIONS
// ============================================================================

bool BitboardPosition::make_move(const SimpleBitboardMove& move) {
    // Store current state for potential undo
    UndoInfo undo_info;
    undo_info.captured_piece = this->piece_at(move.to_64);
    undo_info.ep_square_64 = this->ep_square_64;
    undo_info.castling_rights = this->castling_rights;
    undo_info.halfmove_clock = this->halfmove_clock;
    undo_info.zobrist_key = this->zobrist_key;
    undo_info.was_en_passant_capture = move.is_ep_capture;
    undo_info.en_passant_captured_square = -1;
    
    // Get the moving piece
    Piece moving_piece = this->piece_at(move.from_64);
    if (moving_piece == Piece::None) {
        return false; // No piece to move
    }
    
    Color moving_color = this->color_at(move.from_64);
    PieceType moving_piece_type = this->piece_type_at(move.from_64);
    
    // Update halfmove clock
    if (moving_piece_type == PieceType::Pawn || move.is_capture) {
        this->halfmove_clock = 0;
    } else {
        this->halfmove_clock++;
    }
    
    // Handle special moves first
    if (move.is_ep_capture) {
        // En passant capture
        int captured_pawn_square = move.to_64 + (moving_color == Color::White ? -8 : 8);
        undo_info.en_passant_captured_square = captured_pawn_square;
        this->remove_piece(captured_pawn_square);
    } else if (move.is_castling) {
        // Castling - move both king and rook
        int rook_from, rook_to;
        if (move.to_64 > move.from_64) {
            // Kingside castling
            rook_from = move.from_64 + 3;
            rook_to = move.from_64 + 1;
        } else {
            // Queenside castling
            rook_from = move.from_64 - 4;
            rook_to = move.from_64 - 1;
        }
        
        // Move the rook
        Piece rook = this->piece_at(rook_from);
        this->remove_piece(rook_from);
        this->place_piece(rook_to, moving_color, PieceType::Rook);
    }
    
    // Handle regular captures
    if (move.is_capture && !move.is_ep_capture) {
        // Remove captured piece
        this->remove_piece(move.to_64);
    }
    
    // Move the piece
    this->remove_piece(move.from_64);
    
    if (move.is_promotion) {
        this->place_piece(move.to_64, moving_color, move.promotion_type);
    } else {
        this->place_piece(move.to_64, moving_color, moving_piece_type);
    }
    
    // Update king square tracking
    if (moving_piece_type == PieceType::King) {
        this->king_square_64[static_cast<int>(moving_color)] = move.to_64;
    }
    
    // Update en passant square
    this->ep_square_64 = -1;
    if (moving_piece_type == PieceType::Pawn && abs(move.to_64 - move.from_64) == 16) {
        // Double pawn push
        this->ep_square_64 = (move.from_64 + move.to_64) / 2;
    }
    
    // Update castling rights
    // If king moves, lose all castling rights for that side
    if (moving_piece_type == PieceType::King) {
        if (moving_color == Color::White) {
            this->castling_rights &= ~(1 | 2);  // Clear white castling
        } else {
            this->castling_rights &= ~(4 | 8);  // Clear black castling
        }
    }
    
    // If rook moves from corner, lose castling rights for that side
    if (moving_piece_type == PieceType::Rook) {
        if (move.from_64 == 0) this->castling_rights &= ~2;  // White queenside
        if (move.from_64 == 7) this->castling_rights &= ~1;  // White kingside
        if (move.from_64 == 56) this->castling_rights &= ~8; // Black queenside
        if (move.from_64 == 63) this->castling_rights &= ~4; // Black kingside
    }
    
    // Switch sides
    this->side_to_move = (this->side_to_move == Color::White) ? Color::Black : Color::White;
    this->ply++;
    
    if (this->side_to_move == Color::White) {
        this->fullmove_number++;
    }
    
    // Update derived bitboards
    this->update_derived_bitboards();
    
    // Simple hash update (placeholder for now)
    this->zobrist_key ^= 0x123456789ABCDEF0ULL;  // Very basic hash update
    
    return true;
}

BitboardPosition::UndoInfo BitboardPosition::make_move_with_undo(const SimpleBitboardMove& move) {
    // Store current state for undo
    UndoInfo undo_info;
    undo_info.captured_piece = this->piece_at(move.to_64);
    undo_info.ep_square_64 = this->ep_square_64;
    undo_info.castling_rights = this->castling_rights;
    undo_info.halfmove_clock = this->halfmove_clock;
    undo_info.zobrist_key = this->zobrist_key;
    undo_info.was_en_passant_capture = move.is_ep_capture;
    undo_info.en_passant_captured_square = -1;
    undo_info.ply = this->ply;                          // Save ply
    undo_info.fullmove_number = this->fullmove_number;  // Save fullmove_number
    undo_info.material_score = this->material_score;    // Save material scores
    undo_info.side_to_move = this->side_to_move;        // Save side to move
    
    // Get the moving piece info
    Piece moving_piece = this->piece_at(move.from_64);
    if (moving_piece == Piece::None) {
        return undo_info; // Invalid move
    }
    
    Color moving_color = this->color_at(move.from_64);
    PieceType moving_piece_type = this->piece_type_at(move.from_64);
    
    // Update halfmove clock
    if (moving_piece_type == PieceType::Pawn || move.is_capture) {
        this->halfmove_clock = 0;
    } else {
        this->halfmove_clock++;
    }
    
    // Handle special moves first
    if (move.is_ep_capture) {
        // En passant capture
        int captured_pawn_square = move.to_64 + (moving_color == Color::White ? -8 : 8);
        undo_info.en_passant_captured_square = captured_pawn_square;
        this->remove_piece(captured_pawn_square);
    } else if (move.is_castling) {
        // Castling - move both king and rook
        int rook_from, rook_to;
        if (move.to_64 > move.from_64) {
            // Kingside castling
            rook_from = move.from_64 + 3;
            rook_to = move.from_64 + 1;
        } else {
            // Queenside castling
            rook_from = move.from_64 - 4;
            rook_to = move.from_64 - 1;
        }
        
        // Move the rook
        Piece rook = this->piece_at(rook_from);
        this->remove_piece(rook_from);
        this->place_piece(rook_to, moving_color, PieceType::Rook);
    }
    
    // Handle regular captures
    if (move.is_capture && !move.is_ep_capture) {
        // Remove captured piece
        this->remove_piece(move.to_64);
    }
    
    // Move the piece
    this->remove_piece(move.from_64);
    
    if (move.is_promotion) {
        this->place_piece(move.to_64, moving_color, move.promotion_type);
    } else {
        this->place_piece(move.to_64, moving_color, moving_piece_type);
    }
    
    // Update king square tracking
    if (moving_piece_type == PieceType::King) {
        this->king_square_64[static_cast<int>(moving_color)] = move.to_64;
    }
    
    // Update en passant square
    this->ep_square_64 = -1;
    if (moving_piece_type == PieceType::Pawn && abs(move.to_64 - move.from_64) == 16) {
        // Double pawn push
        this->ep_square_64 = (move.from_64 + move.to_64) / 2;
    }
    
    // Update castling rights
    // If king moves, lose all castling rights for that side
    if (moving_piece_type == PieceType::King) {
        if (moving_color == Color::White) {
            this->castling_rights &= ~(1 | 2);  // Clear white castling
        } else {
            this->castling_rights &= ~(4 | 8);  // Clear black castling
        }
    }
    
    // If rook moves from corner, lose castling rights for that side
    if (moving_piece_type == PieceType::Rook) {
        if (move.from_64 == 0) this->castling_rights &= ~2;  // White queenside
        if (move.from_64 == 7) this->castling_rights &= ~1;  // White kingside
        if (move.from_64 == 56) this->castling_rights &= ~8; // Black queenside
        if (move.from_64 == 63) this->castling_rights &= ~4; // Black kingside
    }
    
    // Switch sides
    this->side_to_move = (this->side_to_move == Color::White) ? Color::Black : Color::White;
    this->ply++;
    
    if (this->side_to_move == Color::White) {
        this->fullmove_number++;
    }
    
    // Update derived bitboards
    this->update_derived_bitboards();
    
    // Simple hash update (placeholder for now)
    this->zobrist_key ^= 0x123456789ABCDEF0ULL;  // Very basic hash update
    
    return undo_info;
}

void BitboardPosition::unmake_move(const SimpleBitboardMove& move, const UndoInfo& undo_info) {
    // Restore game state
    this->ep_square_64 = undo_info.ep_square_64;
    this->castling_rights = undo_info.castling_rights;
    this->halfmove_clock = undo_info.halfmove_clock;
    this->zobrist_key = undo_info.zobrist_key;
    this->ply = undo_info.ply;                          // Restore ply
    this->fullmove_number = undo_info.fullmove_number;  // Restore fullmove_number
    this->material_score = undo_info.material_score;    // Restore material scores
    this->side_to_move = undo_info.side_to_move;        // Restore side to move directly
    
    // The moving color is the side that was originally to move (saved in UndoInfo)
    Color moving_color = undo_info.side_to_move;
    PieceType moving_piece_type;
    
    if (move.is_promotion) {
        moving_piece_type = PieceType::Pawn;  // Original piece was a pawn
        this->remove_piece(move.to_64);  // Remove promoted piece
    } else {
        moving_piece_type = this->piece_type_at(move.to_64);
        this->remove_piece(move.to_64);  // Remove piece from destination
    }
    
    // Restore the original piece
    this->place_piece(move.from_64, moving_color, moving_piece_type);
    
    // Update king square tracking
    if (moving_piece_type == PieceType::King) {
        this->king_square_64[static_cast<int>(moving_color)] = move.from_64;
    }
    
    // Handle special moves
    if (move.is_ep_capture) {
        // Restore en passant captured pawn
        PieceType captured_pawn_type = PieceType::Pawn;
        Color captured_color = (moving_color == Color::White) ? Color::Black : Color::White;
        this->place_piece(undo_info.en_passant_captured_square, captured_color, captured_pawn_type);
    } else if (move.is_castling) {
        // Undo castling - move rook back
        int rook_from, rook_to;
        if (move.to_64 > move.from_64) {
            // Kingside castling
            rook_from = move.from_64 + 3;
            rook_to = move.from_64 + 1;
        } else {
            // Queenside castling
            rook_from = move.from_64 - 4;
            rook_to = move.from_64 - 1;
        }
        
        // Move rook back
        this->remove_piece(rook_to);
        this->place_piece(rook_from, moving_color, PieceType::Rook);
    }
    
    // Restore captured piece
    if (move.is_capture && !move.is_ep_capture && undo_info.captured_piece != Piece::None) {
        Color captured_color = color_of(undo_info.captured_piece);
        PieceType captured_piece_type = type_of(undo_info.captured_piece);
        this->place_piece(move.to_64, captured_color, captured_piece_type);
    }
    
    // Update derived bitboards
    this->update_derived_bitboards();
}

bool BitboardPosition::is_legal_move(const SimpleBitboardMove& move) {
    // Store the original side to move before making the move
    Color original_color = this->side_to_move;
    
    // Make the move temporarily and get undo info
    UndoInfo undo_info = this->make_move_with_undo(move);
    
    // Check if our king is in check after the move
    // Our color is the one that just moved (original color)
    // Enemy color is the current side to move (switched after make_move)
    Color our_color = original_color;
    Color enemy_color = this->side_to_move;
    int our_king_square = this->king_square_64[static_cast<int>(our_color)];
    
    bool is_legal = true;
    if (our_king_square != -1) {
        // Check if our king is attacked by enemy pieces
        is_legal = !this->is_square_attacked(our_king_square, enemy_color);
    }
    
    // Unmake the move
    this->unmake_move(move, undo_info);
    
    return is_legal;
}

bool BitboardPosition::is_square_attacked(int square_64, Color attacking_color) const {
    if (square_64 < 0 || square_64 >= 64) return false;
    
    uint64_t target_bb = 1ULL << square_64;
    int attacking_color_idx = static_cast<int>(attacking_color);
    
    // Check pawn attacks
    uint64_t enemy_pawns = piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::Pawn)];
    if (attacking_color == Color::White) {
        // White pawns attack diagonally up
        uint64_t pawn_attacks = ((enemy_pawns & ~FILE_A_BB) << 7) | ((enemy_pawns & ~FILE_H_BB) << 9);
        if (pawn_attacks & target_bb) return true;
    } else {
        // Black pawns attack diagonally down  
        uint64_t pawn_attacks = ((enemy_pawns & ~FILE_H_BB) >> 7) | ((enemy_pawns & ~FILE_A_BB) >> 9);
        if (pawn_attacks & target_bb) return true;
    }
    
    // Check knight attacks
    uint64_t enemy_knights = piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::Knight)];
    while (enemy_knights) {
        int knight_sq = get_lsb(enemy_knights);
        if (knight_attacks[knight_sq] & target_bb) return true;
        enemy_knights &= enemy_knights - 1;
    }
    
    // Check king attacks
    uint64_t enemy_king = piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::King)];
    if (enemy_king) {
        int king_sq = get_lsb(enemy_king);
        if (king_attacks[king_sq] & target_bb) return true;
    }
    
    // Check bishop/queen diagonal attacks
    uint64_t enemy_bishops_queens = piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::Bishop)] |
                                   piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::Queen)];
    while (enemy_bishops_queens) {
        int piece_sq = get_lsb(enemy_bishops_queens);
        uint64_t attacks = bishop_attacks(piece_sq, occupied_bitboard);
        if (attacks & target_bb) return true;
        enemy_bishops_queens &= enemy_bishops_queens - 1;
    }
    
    // Check rook/queen rank/file attacks
    uint64_t enemy_rooks_queens = piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::Rook)] |
                                 piece_bitboards[attacking_color_idx][static_cast<int>(PieceType::Queen)];
    while (enemy_rooks_queens) {
        int piece_sq = get_lsb(enemy_rooks_queens);
        uint64_t attacks = rook_attacks(piece_sq, occupied_bitboard);
        if (attacks & target_bb) return true;
        enemy_rooks_queens &= enemy_rooks_queens - 1;
    }
    
    return false;
}