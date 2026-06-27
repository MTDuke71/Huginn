/**
 * @file syzygy_tablebase.hpp
 * @brief Syzygy endgame-tablebase interface (wraps the Fathom library).
 *
 * Gives the search perfect WDL/DTZ results for low-piece-count endgames. The
 * whole feature is compiled out unless `ENABLE_FATHOM` (→ `FATHOM_AVAILABLE`)
 * is set, so non-TB builds pay nothing. Probe results are mapped to Huginn's
 * own score scale by the search (see search.cpp); they are deliberately NOT
 * stored in the TT (rule50 isn't in the Zobrist key).
 */
#pragma once

#include <string>
#include <cstdint>
#include "chess_types.hpp"
#include "position.hpp"
#include "move.hpp"

// Conditional compilation for Fathom library
#ifndef FATHOM_AVAILABLE
    #ifdef ENABLE_FATHOM
        #define FATHOM_AVAILABLE 1
    #else
        #define FATHOM_AVAILABLE 0
    #endif
#endif

#if FATHOM_AVAILABLE
extern "C" {
    #include "tbprobe.h"
}
#endif

namespace Huginn {

/**
 * @brief Syzygy Tablebase Interface
 * 
 * Provides access to Syzygy endgame tablebases for perfect endgame play.
 * Uses the Fathom library for tablebase access.
 */
class SyzygyTablebase {
private:
    std::string tablebase_path;  ///< Directory the .rtbw/.rtbz files were loaded from.
    bool is_initialized;         ///< True between a successful initialize() and shutdown().
    int max_pieces;              ///< Largest piece count the loaded tablebases cover.

public:
    SyzygyTablebase();
    ~SyzygyTablebase();
    
    /**
     * @brief Initialize tablebase with given path
     * @param path Directory containing .rtbw and .rtbz files
     * @return true if initialization successful
     */
    bool initialize(const std::string& path);
    
    /**
     * @brief Clear and shutdown tablebase
     */
    void shutdown();
    
    /**
     * @brief Check if tablebases are loaded and ready
     * @return true if tablebases are available
     */
    bool is_available() const;

    /**
     * @brief Check if position can be probed
     * @param pos Position to check
     * @return true if position has <= max_pieces and no castling/en passant
     */
    bool can_probe(const Position& pos) const;
    
    /**
     * @brief Probe WDL (Win/Draw/Loss) result
     * @param pos Position to probe
     * @return WDL result: >0 = win, 0 = draw, <0 = loss, INT32_MAX = probe failed
     */
    int probe_wdl(const Position& pos) const;
    
    /**
     * @brief Get best move from tablebase
     * @param pos Position to probe
     * @return best move, or null move if probe failed
     */
    S_MOVE probe_root(const Position& pos) const;

    /**
     * @brief Get tablebase statistics
     * @return string with tablebase info
     */
    std::string get_info() const;
};

} // namespace Huginn