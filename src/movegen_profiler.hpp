#ifndef MOVEGEN_PROFILER_HPP
#define MOVEGEN_PROFILER_HPP

#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>

/**
 * Detailed profiling system for move generation functions
 * Provides granular timing analysis of each piece-specific generator
 */
class MoveGenProfiler {
public:
    struct FunctionProfile {
        std::string function_name;
        double total_time_ns;
        int call_count;
        int moves_generated;
        double avg_time_per_call;
        double avg_time_per_move;
        double percentage_of_total;
    };

    struct PositionProfile {
        std::string position_name;
        std::string fen;
        std::vector<FunctionProfile> function_profiles;
        double total_time_ns;
        int total_moves;
    };

private:
    static std::vector<PositionProfile> results_;
    static FunctionProfile current_pawn_profile_;
    static FunctionProfile current_knight_profile_;
    static FunctionProfile current_bishop_profile_;
    static FunctionProfile current_rook_profile_;
    static FunctionProfile current_queen_profile_;
    static FunctionProfile current_king_profile_;
    static FunctionProfile current_legal_filter_profile_;

public:
    // Main profiling functions
    static void profile_comprehensive_movegen();
    static void profile_position(const std::string& name, const std::string& fen);
    static void print_detailed_analysis();
    static void print_optimization_recommendations();

    // Individual function profilers
    static void profile_generate_all_moves(const Position& pos, S_MOVELIST& list);
    static void profile_generate_legal_moves(Position& pos, S_MOVELIST& list);
    
    // Piece-specific profiling wrappers
    static void profile_pawn_moves(const Position& pos, S_MOVELIST& list, Color us);
    static void profile_knight_moves(const Position& pos, S_MOVELIST& list, Color us);
    static void profile_bishop_moves(const Position& pos, S_MOVELIST& list, Color us);
    static void profile_rook_moves(const Position& pos, S_MOVELIST& list, Color us);
    static void profile_queen_moves(const Position& pos, S_MOVELIST& list, Color us);
    static void profile_king_moves(const Position& pos, S_MOVELIST& list, Color us);

private:
    // Utility functions
    static void reset_current_profiles();
    static void calculate_percentages(PositionProfile& profile);
    static void print_function_breakdown(const PositionProfile& profile);
    static void print_summary_table();
    static std::string format_time(double nanoseconds);
};

#endif // MOVEGEN_PROFILER_HPP
