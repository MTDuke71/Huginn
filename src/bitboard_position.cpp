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
#include "bitboard.hpp"  // For bitboard utilities like pop_lsb, setBit, clearBit
#include "bitboard_attacks.hpp"  // For attack table initialization
#include <sstream>
#include <algorithm>

// ============================================================================
// CONSTRUCTION AND INITIALIZATION
// ============================================================================

BitboardPosition::BitboardPosition() {
    reset();
    
    // Initialize attack tables on first construction
    static bool tables_initialized = false;
    if (!tables_initialized) {
        init_bitboard_attacks();
        tables_initialized = true;
    }
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
    
    // Parse en passant (simplified - would need full conversion)
    ep_square_64 = (ep == "-") ? -1 : 0;  // Placeholder
    
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
    
    fen << ' ' << (ep_square_64 == -1 ? "-" : "a1");  // Placeholder
    fen << ' ' << halfmove_clock << ' ' << fullmove_number;
    
    return fen.str();
}