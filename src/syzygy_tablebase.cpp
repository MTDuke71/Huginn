#include "syzygy_tablebase.hpp"
#include "position.hpp"
#include "move.hpp"
#include <iostream>
#include <filesystem>

// Stub implementation for Syzygy tablebase support
// This allows the engine to compile and run without actual tablebase functionality
// When ready to add real tablebase support, replace this with Fathom integration

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
}

void SyzygyTablebase::shutdown() {
    if (is_initialized) {
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
    
    return "Syzygy tablebases: Stub implementation (max " + std::to_string(max_pieces) + " pieces)";
}

bool SyzygyTablebase::is_available() const {
    return is_initialized;
}

bool SyzygyTablebase::can_probe(const Position& pos) const {
    // Stub implementation - never probe for now
    // This can be enabled later when real tablebase support is added
    return false;
}

int SyzygyTablebase::probe_wdl(const Position& pos) const {
    // Stub implementation - never probe
    return INT32_MAX; // Indicates probe failed
}

int SyzygyTablebase::probe_dtz(const Position& pos) const {
    // Stub implementation - never probe
    return INT32_MAX; // Indicates probe failed
}

S_MOVE SyzygyTablebase::probe_root(const Position& pos) const {
    // Stub implementation - return null move
    return S_MOVE();
}

} // namespace Huginn