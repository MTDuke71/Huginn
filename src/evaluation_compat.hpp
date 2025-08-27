#pragma once

// Compatibility layer to migrate from legacy Evaluation namespace to Huginn
// This allows demo executables to continue working while we transition

#include "evaluation.hpp"
#include "search.hpp"
#include "position.hpp"
#include <chrono>
#include <iostream>

namespace Evaluation {
    // Main evaluation function - forwards to huginn_engine
    inline int evaluate_position(const Position& pos) {
        return Huginn::HybridEvaluator::evaluate(pos);
    }
    
    // Component evaluation functions - simplified to use main evaluate
    // These were separate in the legacy system but huginn_engine has a unified evaluator
    inline int evaluate_material(const Position& pos) {
        // huginn_engine doesn't expose material evaluation separately
        // Return a portion of the total evaluation as an approximation
        return Huginn::HybridEvaluator::evaluate(pos) / 4; // Rough estimate
    }
    
    inline int evaluate_positional(const Position& pos) {
        // huginn_engine doesn't expose positional evaluation separately
        return Huginn::HybridEvaluator::evaluate(pos) / 4; // Rough estimate
    }
    
    inline int evaluate_king_safety(const Position& pos) {
        // huginn_engine doesn't expose king safety evaluation separately
        return Huginn::HybridEvaluator::evaluate(pos) / 4; // Rough estimate
    }
    
    inline int evaluate_pawn_structure(const Position& pos) {
        // huginn_engine doesn't expose pawn structure evaluation separately
        return Huginn::HybridEvaluator::evaluate(pos) / 4; // Rough estimate
    }
    
    inline int evaluate_development(const Position& pos) {
        // huginn_engine doesn't expose development evaluation separately
        return Huginn::HybridEvaluator::evaluate(pos) / 4; // Rough estimate
    }
    
    // For opening analyzer demo
    inline void analyze_opening_moves(int depth) {
        // Placeholder implementation - could be expanded with huginn_engine
        std::cout << "Opening analysis not yet implemented in huginn_engine" << std::endl;
    }
}

// Search compatibility for demos that use Search::Engine
namespace Search {
    // Simple search limits compatibility
    struct SearchLimits {
        int max_depth = 6;
        std::chrono::milliseconds max_time{5000};
        bool infinite = false;
    };
    
    // Convert to huginn_engine format
    inline Huginn::SearchLimits to_huginn_engine_limits(const SearchLimits& limits) {
        Huginn::SearchLimits huginn_limits;
        huginn_limits.max_depth = limits.max_depth;
        huginn_limits.max_time_ms = limits.max_time.count();
        huginn_limits.infinite = limits.infinite;
        return huginn_limits;
    }
    
    // Simple search engine wrapper
    class Engine {
    private:
        Huginn::SimpleEngine huginn_engine;
        
    public:
        Engine() = default;
        
        S_MOVE search(Position pos, const SearchLimits& limits) {
            return huginn_engine.search(pos, to_huginn_engine_limits(limits));
        }
        
        void reset() { huginn_engine.reset(); }
        void stop() { huginn_engine.stop(); }
        
        // Dummy methods for compatibility
        void set_hash_size(int mb) { /* huginn_engine doesn't use hash tables yet */ }
        void set_threads(int threads) { /* huginn_engine is single-threaded */ }
        void clear_hash() { /* huginn_engine doesn't use hash tables yet */ }
        uint64_t get_hashfull() const { return 0; }
        
        bool is_mate_score(int score) const { return Huginn::SimpleEngine::is_mate_score(score); }
        int mate_distance(int score) const { return Huginn::SimpleEngine::mate_distance(score); }
        
        // For UCI callback compatibility
        template<typename Callback>
        void set_info_callback(Callback&& cb) { /* huginn_engine doesn't use callbacks yet */ }
    };
    
    // Move conversion
    inline std::string move_to_uci(const S_MOVE& move) {
        return Huginn::SimpleEngine::move_to_uci(move);
    }
}
