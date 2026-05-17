#include "syzygy_tablebase.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include "search.hpp"  // For MATE constant
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
    : tablebase_path("c:\\TB\\"), is_initialized(false), max_pieces(0) {
}

SyzygyTablebase::~SyzygyTablebase() {
    shutdown();
}

bool SyzygyTablebase::initialize(const std::string& path) {
    // Shutdown any existing tablebase first
    shutdown();
    
    // Use hardcoded path if provided path is empty, otherwise use provided path
    std::string target_path = path.empty() ? "c:\\TB\\" : path;
    
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
    // Position is bitboard-primary; popcount of the occupancy is the
    // total piece count in one instruction. The prior implementation
    // iterated all 120 squares calling pos.at() at every depth-<=-1
    // leaf node — measured -57 Elo / 200g vs t3 from this overhead
    // alone (the probe-success path was tiny by comparison).
    return popcount(pos.occupied_bitboard) <= static_cast<int>(max_pieces);
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
    // Pull the bitboards directly from Position. Both Position and Fathom
    // use the same sq64 layout (a1=bit 0 ... h8=bit 63), so no per-square
    // translation is needed. Replaces a 120-square iteration that called
    // pos.at() / color_of / type_of / 1ULL<<sq for every set piece.
    constexpr int W = int(Color::White);
    constexpr int B = int(Color::Black);
    uint64_t white   = pos.color_bitboards[W];
    uint64_t black   = pos.color_bitboards[B];
    uint64_t kings   = pos.piece_bitboards[W][int(PieceType::King)]   | pos.piece_bitboards[B][int(PieceType::King)];
    uint64_t queens  = pos.piece_bitboards[W][int(PieceType::Queen)]  | pos.piece_bitboards[B][int(PieceType::Queen)];
    uint64_t rooks   = pos.piece_bitboards[W][int(PieceType::Rook)]   | pos.piece_bitboards[B][int(PieceType::Rook)];
    uint64_t bishops = pos.piece_bitboards[W][int(PieceType::Bishop)] | pos.piece_bitboards[B][int(PieceType::Bishop)];
    uint64_t knights = pos.piece_bitboards[W][int(PieceType::Knight)] | pos.piece_bitboards[B][int(PieceType::Knight)];
    uint64_t pawns   = pos.piece_bitboards[W][int(PieceType::Pawn)]   | pos.piece_bitboards[B][int(PieceType::Pawn)];
    
    // Convert en passant square
    unsigned ep = 64; // Invalid square indicator for Fathom
    if (pos.ep_square != -1) {
        ep = static_cast<unsigned>(pos.ep_square);  // ep_square is sq64 == Fathom square
    }

    // Convert castling rights
    unsigned castling = 0;
    if (pos.castling_rights & CASTLE_WK) castling |= TB_CASTLING_K;
    if (pos.castling_rights & CASTLE_WQ) castling |= TB_CASTLING_Q;
    if (pos.castling_rights & CASTLE_BK) castling |= TB_CASTLING_k;
    if (pos.castling_rights & CASTLE_BQ) castling |= TB_CASTLING_q;

    // Use the safe wrapper `tb_probe_wdl` instead of `tb_probe_wdl_impl`.
    // Fathom's TB encoding assumes castling == 0 and rule50 == 0; the
    // wrapper returns TB_RESULT_FAILED otherwise. Calling the impl
    // directly silently fed positions-with-castling and positions-with-
    // a-non-zero-halfmove-counter to the probe, which returned WDL
    // values from a no-castling, rule50=0 reading. Those wrong values
    // got TT-cached at depth=127 EXACT and corrupted subsequent
    // searches — measured at -63 Elo over 100g vs t3 before this fix.
    unsigned result = tb_probe_wdl(white, black, kings, queens, rooks,
                                   bishops, knights, pawns,
                                   pos.halfmove_clock, castling, ep,
                                   pos.side_to_move == Color::White);
    
    if (result == TB_RESULT_FAILED) {
        return INT32_MAX;
    }
    
    // Convert Fathom result to engine format. CURSED_WIN and BLESSED_LOSS
    // are wins/losses that exceed the 50-move counter and so adjudicate as
    // draws under standard rules (which fastchess enforces). Return small
    // non-zero values so the engine still has a tie-breaker preference for
    // a cursed-win path over a forced-real-draw path, but does not commit
    // search resources or play decisions assuming it has a real mate.
    unsigned wdl = result & TB_RESULT_WDL_MASK;
    switch (wdl) {
        case TB_LOSS:         return -MATE + 1000;
        case TB_BLESSED_LOSS: return -1;   // effectively a draw
        case TB_DRAW:         return 0;
        case TB_CURSED_WIN:   return 1;    // effectively a draw
        case TB_WIN:          return MATE - 1000;
        default:              return INT32_MAX;
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
