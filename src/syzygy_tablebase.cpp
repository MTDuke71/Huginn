#include "syzygy_tablebase.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include "minimal_search.hpp"  // For MATE constant
#include <iostream>
#include <filesystem>

#if FATHOM_AVAILABLE
// Fathom mapping functions

// Fathom doesn't use individual piece constants - it uses bitboards by type
// This function isn't needed in the same way, but we'll keep it for consistency
unsigned fathom_piece_from_huginn(Piece piece) {
    // This function is no longer needed since Fathom uses piece type bitboards
    // But we'll return 1 for valid pieces, 0 for empty/offboard
    return (is_none(piece) || is_offboard(piece)) ? 0 : 1;
}

// Map Huginn square (120-square) to Fathom square (64-square)
unsigned fathom_square_from_huginn(int huginn_square) {
    // Convert from 120-square to 64-square format
    if (!is_playable(huginn_square)) {
        return 64; // Fathom uses 64 as invalid square (not TB_NOSQUARE constant)
    }
    
    int rank = huginn_square / 10 - 2;
    int file = huginn_square % 10 - 1;
    
    if (rank < 0 || rank > 7 || file < 0 || file > 7) {
        return 64; // Invalid square
    }
    
    return rank * 8 + file;
}

// Map Fathom square back to Huginn square
int huginn_square_from_fathom(unsigned fathom_square) {
    if (fathom_square >= 64) return int(Square::Offboard);
    
    int rank = fathom_square / 8;
    int file = fathom_square % 8;
    
    return (rank + 2) * 10 + (file + 1);
}

#endif // FATHOM_AVAILABLE

namespace Huginn {

SyzygyTablebase::SyzygyTablebase() 
    : tablebase_path("d:\\TB\\"), is_initialized(false), max_pieces(0) {
}

SyzygyTablebase::~SyzygyTablebase() {
    shutdown();
}

bool SyzygyTablebase::initialize(const std::string& path) {
    // Shutdown any existing tablebase first
    shutdown();
    
    // Use hardcoded path if provided path is empty, otherwise use provided path
    std::string target_path = path.empty() ? "d:\\TB\\" : path;
    
    #if FATHOM_AVAILABLE
    // Real Fathom implementation
    if (!tb_init(target_path.c_str())) {
        std::cerr << "Failed to initialize Fathom tablebases at: " << target_path << std::endl;
        return false;
    }
    
    unsigned max_tb_pieces = TB_LARGEST;
    if (max_tb_pieces == 0) {
        std::cerr << "No tablebase files found at: " << target_path << std::endl;
        tb_free();
        return false;
    }
    
    tablebase_path = target_path;
    is_initialized = true;
    max_pieces = max_tb_pieces;
    
    std::cout << "Fathom tablebases initialized: " << target_path 
              << " (max " << max_pieces << " pieces)" << std::endl;
    return true;
    
    #else
    // Stub implementation - just check if path exists
    if (!std::filesystem::exists(target_path)) {
        std::cerr << "Warning: Tablebase path does not exist: " << target_path << std::endl;
        return false;
    }
    
    tablebase_path = target_path;
    is_initialized = true;
    max_pieces = 5; // Assume 5-piece tables available
    
    std::cout << "Tablebase initialized (stub implementation): " << target_path << std::endl;
    return true;
    #endif
}

void SyzygyTablebase::shutdown() {
    if (is_initialized) {
        #if FATHOM_AVAILABLE
        tb_free();
        #endif
        
        is_initialized = false;
        max_pieces = 0;
        tablebase_path.clear();
    }
}

const std::string& SyzygyTablebase::get_path() const {
    return tablebase_path;
}

std::string SyzygyTablebase::get_info() const {
    if (!is_initialized) {
        return "Syzygy tablebases: Not initialized";
    }
    
    #if FATHOM_AVAILABLE
    return "Syzygy tablebases: Fathom (max " + std::to_string(max_pieces) + " pieces)";
    #else
    return "Syzygy tablebases: Stub implementation (max " + std::to_string(max_pieces) + " pieces)";
    #endif
}

bool SyzygyTablebase::is_available() const {
    return is_initialized;
}

bool SyzygyTablebase::can_probe(const Position& pos) const {
    if (!is_initialized) {
        return false;
    }
    
    #if FATHOM_AVAILABLE
    // Count total pieces on the board
    int piece_count = 0;
    for (int square = 0; square < 120; ++square) {
        Piece piece = pos.at(square);
        if (!is_none(piece) && !is_offboard(piece)) {
            piece_count++;
        }
    }
    
    // Can probe if piece count is within tablebase range
    return piece_count <= static_cast<int>(max_pieces);
    #else
    // Stub implementation - never probe for now
    return false;
    #endif
}

int SyzygyTablebase::probe_wdl(const Position& pos) const {
    if (!can_probe(pos)) {
        return INT32_MAX; // Indicates probe failed
    }
    
    #if FATHOM_AVAILABLE
    // Convert position to Fathom format
    unsigned white = 0, black = 0;
    unsigned kings = 0, queens = 0, rooks = 0, bishops = 0, knights = 0, pawns = 0;
    
    for (int square = 0; square < 120; ++square) {
        Piece piece = pos.at(square);
        if (is_none(piece) || is_offboard(piece)) continue;
        
        unsigned fathom_square = fathom_square_from_huginn(square);
        if (fathom_square >= 64) continue; // Invalid square
        
        uint64_t piece_bb = 1ULL << fathom_square;
        
        // Add to color bitboards
        if (color_of(piece) == Color::White) {
            white |= piece_bb;
        } else {
            black |= piece_bb;
        }
        
        // Add to piece type bitboards
        switch (type_of(piece)) {
            case PieceType::Pawn:   pawns |= piece_bb; break;
            case PieceType::Knight: knights |= piece_bb; break;
            case PieceType::Bishop: bishops |= piece_bb; break;
            case PieceType::Rook:   rooks |= piece_bb; break;
            case PieceType::Queen:  queens |= piece_bb; break;
            case PieceType::King:   kings |= piece_bb; break;
            default: break;
        }
    }
    
    // Convert en passant square
    unsigned ep = 64; // Invalid square indicator for Fathom
    if (pos.ep_square != -1) {
        ep = fathom_square_from_huginn(pos.ep_square);
    }
    
    // Convert castling rights
    unsigned castling = 0;
    if (pos.castling_rights & CASTLE_WK) castling |= TB_CASTLING_K;
    if (pos.castling_rights & CASTLE_WQ) castling |= TB_CASTLING_Q;
    if (pos.castling_rights & CASTLE_BK) castling |= TB_CASTLING_k;
    if (pos.castling_rights & CASTLE_BQ) castling |= TB_CASTLING_q;
    
    // Probe WDL using tb_probe_wdl_impl (the working function)
    unsigned result = tb_probe_wdl_impl(white, black, kings, queens, rooks, bishops, knights, pawns,
                                        ep, pos.side_to_move == Color::White);
    
    if (result == TB_RESULT_FAILED) {
        return INT32_MAX;
    }
    
    // Convert Fathom result to engine format
    unsigned wdl = result & TB_RESULT_WDL_MASK;
    switch (wdl) {
        case TB_LOSS: return -MATE + 1000;  // Convert to mate score
        case TB_BLESSED_LOSS: return -MATE + 1000;
        case TB_DRAW: return 0;
        case TB_CURSED_WIN: return MATE - 1000;
        case TB_WIN: return MATE - 1000;   // Convert to mate score
        default: return INT32_MAX;
    }
    #else
    // Stub implementation - never probe
    return INT32_MAX;
    #endif
}

int SyzygyTablebase::probe_dtz(const Position& pos) const {
    // TODO: Implement DTZ probing when needed
    #if FATHOM_AVAILABLE
    // DTZ probing is more complex and not needed for basic tablebase functionality
    return INT32_MAX; // Not implemented yet
    #else
    return INT32_MAX;
    #endif
}

S_MOVE SyzygyTablebase::probe_root(const Position& pos) const {
    // TODO: Implement root probing when needed
    #if FATHOM_AVAILABLE
    // Root probing is more complex and not needed for basic tablebase functionality
    return S_MOVE(); // Not implemented yet
    #else
    return S_MOVE();
    #endif
}

} // namespace Huginn