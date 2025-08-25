#include "movegen_profiler.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <algorithm>
#include <map>

// Static member initialization
std::vector<MoveGenProfiler::PositionProfile> MoveGenProfiler::results_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_pawn_profile_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_knight_profile_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_bishop_profile_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_rook_profile_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_queen_profile_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_king_profile_;
MoveGenProfiler::FunctionProfile MoveGenProfiler::current_legal_filter_profile_;

void MoveGenProfiler::profile_comprehensive_movegen() {
    std::cout << "=== COMPREHENSIVE MOVE GENERATION PROFILING ===" << std::endl;
    std::cout << "Analyzing individual function performance..." << std::endl << std::endl;
    
    results_.clear();
    
    // Test positions with varying characteristics
    std::vector<std::pair<std::string, std::string>> test_positions = {
        {"Starting Position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"Kiwipete", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
        {"Complex Middlegame", "r1bq1rk1/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQ - 0 1"},
        {"Endgame", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
        {"Tactical Position", "r2qkb1r/pp2nppp/3p4/2pNN1B1/2BnP3/3P4/PPP2PPP/R2QK2R w KQkq - 0 1"},
        {"Open Position", "rnbqkb1r/pp1p1ppp/5n2/2p1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 1"},
        {"Queen Endgame", "8/2k5/8/3K4/8/8/8/7Q w - - 0 1"},
        {"Pawn Promotion", "8/P7/8/8/8/8/7k/7K w - - 0 1"}
    };
    
    for (const auto& [name, fen] : test_positions) {
        profile_position(name, fen);
    }
    
    print_detailed_analysis();
    print_optimization_recommendations();
}

void MoveGenProfiler::profile_position(const std::string& name, const std::string& fen) {
    std::cout << "Profiling: " << name << std::endl;
    
    Position pos;
    pos.set_from_fen(fen);
    
    PositionProfile profile;
    profile.position_name = name;
    profile.fen = fen;
    
    // Reset function profiles
    reset_current_profiles();
    
    // Profile pseudo-legal move generation (multiple iterations for accuracy)
    const int iterations = 1000;
    S_MOVELIST moves;
    
    auto start_total = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        profile_generate_all_moves(pos, moves);
    }
    
    auto end_total = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_total - start_total);
    
    profile.total_time_ns = total_duration.count() / static_cast<double>(iterations);
    profile.total_moves = moves.count;
    
    // Calculate averages and store function profiles
    auto normalize_profile = [&](FunctionProfile& prof) {
        if (prof.call_count > 0) {
            prof.avg_time_per_call = prof.total_time_ns / prof.call_count;
            prof.avg_time_per_move = prof.moves_generated > 0 ? prof.total_time_ns / prof.moves_generated : 0.0;
        }
    };
    
    normalize_profile(current_pawn_profile_);
    normalize_profile(current_knight_profile_);
    normalize_profile(current_bishop_profile_);
    normalize_profile(current_rook_profile_);
    normalize_profile(current_queen_profile_);
    normalize_profile(current_king_profile_);
    
    profile.function_profiles = {
        current_pawn_profile_,
        current_knight_profile_,
        current_bishop_profile_,
        current_rook_profile_,
        current_queen_profile_,
        current_king_profile_
    };
    
    calculate_percentages(profile);
    
    // Also profile legal move generation
    Position pos_copy = pos;  // Make copy since legal generation modifies position
    S_MOVELIST legal_moves;
    
    auto start_legal = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        pos_copy = pos;  // Reset position
        profile_generate_legal_moves(pos_copy, legal_moves);
    }
    auto end_legal = std::chrono::high_resolution_clock::now();
    auto legal_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_legal - start_legal);
    
    current_legal_filter_profile_.total_time_ns = legal_duration.count() / static_cast<double>(iterations);
    current_legal_filter_profile_.call_count = iterations;
    current_legal_filter_profile_.moves_generated = legal_moves.count;
    normalize_profile(current_legal_filter_profile_);
    
    profile.function_profiles.push_back(current_legal_filter_profile_);
    
    results_.push_back(profile);
    
    print_function_breakdown(profile);
    std::cout << std::endl;
}

void MoveGenProfiler::profile_generate_all_moves(const Position& pos, S_MOVELIST& list) {
    list.count = 0;
    Color us = pos.side_to_move;
    
    // Profile each piece type individually
    profile_pawn_moves(pos, list, us);
    profile_knight_moves(pos, list, us);
    profile_bishop_moves(pos, list, us);
    profile_rook_moves(pos, list, us);
    profile_queen_moves(pos, list, us);
    profile_king_moves(pos, list, us);
}

void MoveGenProfiler::profile_generate_legal_moves(Position& pos, S_MOVELIST& list) {
    auto start = std::chrono::high_resolution_clock::now();
    
    generate_legal_moves_enhanced(pos, list);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    current_legal_filter_profile_.total_time_ns += duration.count();
    current_legal_filter_profile_.call_count++;
    current_legal_filter_profile_.moves_generated = list.count;
    current_legal_filter_profile_.function_name = "Legal Filter";
}

void MoveGenProfiler::profile_pawn_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int moves_before = list.count;
    
    auto start = std::chrono::high_resolution_clock::now();
    generate_pawn_moves(pos, list, us);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    current_pawn_profile_.total_time_ns += duration.count();
    current_pawn_profile_.call_count++;
    current_pawn_profile_.moves_generated += (list.count - moves_before);
    current_pawn_profile_.function_name = "Pawn Moves";
}

void MoveGenProfiler::profile_knight_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int moves_before = list.count;
    
    auto start = std::chrono::high_resolution_clock::now();
    generate_knight_moves(pos, list, us);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    current_knight_profile_.total_time_ns += duration.count();
    current_knight_profile_.call_count++;
    current_knight_profile_.moves_generated += (list.count - moves_before);
    current_knight_profile_.function_name = "Knight Moves";
}

void MoveGenProfiler::profile_bishop_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int moves_before = list.count;
    
    auto start = std::chrono::high_resolution_clock::now();
    generate_bishop_moves(pos, list, us);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    current_bishop_profile_.total_time_ns += duration.count();
    current_bishop_profile_.call_count++;
    current_bishop_profile_.moves_generated += (list.count - moves_before);
    current_bishop_profile_.function_name = "Bishop Moves";
}

void MoveGenProfiler::profile_rook_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int moves_before = list.count;
    
    auto start = std::chrono::high_resolution_clock::now();
    generate_rook_moves(pos, list, us);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    current_rook_profile_.total_time_ns += duration.count();
    current_rook_profile_.call_count++;
    current_rook_profile_.moves_generated += (list.count - moves_before);
    current_rook_profile_.function_name = "Rook Moves";
}

void MoveGenProfiler::profile_queen_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int moves_before = list.count;
    
    auto start = std::chrono::high_resolution_clock::now();
    generate_queen_moves(pos, list, us);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    current_queen_profile_.total_time_ns += duration.count();
    current_queen_profile_.call_count++;
    current_queen_profile_.moves_generated += (list.count - moves_before);
    current_queen_profile_.function_name = "Queen Moves";
}

void MoveGenProfiler::profile_king_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int moves_before = list.count;
    
    auto start = std::chrono::high_resolution_clock::now();
    generate_king_moves(pos, list, us);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    current_king_profile_.total_time_ns += duration.count();
    current_king_profile_.call_count++;
    current_king_profile_.moves_generated += (list.count - moves_before);
    current_king_profile_.function_name = "King Moves";
}

void MoveGenProfiler::reset_current_profiles() {
    auto reset_profile = [](FunctionProfile& prof, const std::string& name) {
        prof = {};
        prof.function_name = name;
    };
    
    reset_profile(current_pawn_profile_, "Pawn Moves");
    reset_profile(current_knight_profile_, "Knight Moves");
    reset_profile(current_bishop_profile_, "Bishop Moves");
    reset_profile(current_rook_profile_, "Rook Moves");
    reset_profile(current_queen_profile_, "Queen Moves");
    reset_profile(current_king_profile_, "King Moves");
    reset_profile(current_legal_filter_profile_, "Legal Filter");
}

void MoveGenProfiler::calculate_percentages(PositionProfile& profile) {
    double total_time = 0.0;
    for (const auto& func : profile.function_profiles) {
        total_time += func.total_time_ns;
    }
    
    for (auto& func : profile.function_profiles) {
        func.percentage_of_total = total_time > 0 ? (func.total_time_ns / total_time) * 100.0 : 0.0;
    }
}

void MoveGenProfiler::print_function_breakdown(const PositionProfile& profile) {
    std::cout << "  Position: " << profile.position_name << " (" << profile.total_moves << " moves)" << std::endl;
    std::cout << "  " << std::string(65, '-') << std::endl;
    std::cout << "  " << std::left << std::setw(15) << "Function"
              << std::setw(12) << "Time (ns)"
              << std::setw(10) << "% Total"
              << std::setw(8) << "Moves"
              << std::setw(12) << "ns/move"
              << std::endl;
    std::cout << "  " << std::string(65, '-') << std::endl;
    
    for (const auto& func : profile.function_profiles) {
        std::cout << "  " << std::left << std::setw(15) << func.function_name
                  << std::fixed << std::setprecision(1)
                  << std::setw(12) << func.total_time_ns
                  << std::setw(10) << func.percentage_of_total
                  << std::setw(8) << func.moves_generated
                  << std::setw(12) << func.avg_time_per_move
                  << std::endl;
    }
}

void MoveGenProfiler::print_detailed_analysis() {
    std::cout << std::endl << "=== DETAILED PROFILING ANALYSIS ===" << std::endl;
    
    print_summary_table();
    
    // Find most expensive functions across all positions
    std::vector<std::pair<std::string, double>> function_totals;
    std::map<std::string, double> total_times;
    std::map<std::string, int> total_moves;
    
    for (const auto& pos_profile : results_) {
        for (const auto& func : pos_profile.function_profiles) {
            total_times[func.function_name] += func.total_time_ns;
            total_moves[func.function_name] += func.moves_generated;
        }
    }
    
    std::cout << std::endl << "=== FUNCTION PERFORMANCE RANKING ===" << std::endl;
    std::cout << std::left << std::setw(15) << "Function"
              << std::setw(15) << "Total Time (ns)"
              << std::setw(12) << "Total Moves"
              << std::setw(15) << "Avg ns/move"
              << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    std::vector<std::pair<std::string, double>> sorted_functions;
    for (const auto& [name, time] : total_times) {
        sorted_functions.push_back({name, time});
    }
    std::sort(sorted_functions.begin(), sorted_functions.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (const auto& [name, time] : sorted_functions) {
        double avg_per_move = total_moves[name] > 0 ? time / total_moves[name] : 0.0;
        std::cout << std::left << std::setw(15) << name
                  << std::fixed << std::setprecision(1)
                  << std::setw(15) << time
                  << std::setw(12) << total_moves[name]
                  << std::setw(15) << avg_per_move
                  << std::endl;
    }
}

void MoveGenProfiler::print_summary_table() {
    std::cout << std::endl << "=== SUMMARY BY POSITION ===" << std::endl;
    std::cout << std::left << std::setw(20) << "Position"
              << std::setw(8) << "Moves"
              << std::setw(15) << "Total Time (ns)"
              << std::setw(15) << "Time/Move (ns)"
              << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& profile : results_) {
        double time_per_move = profile.total_moves > 0 ? profile.total_time_ns / profile.total_moves : 0.0;
        std::cout << std::left << std::setw(20) << profile.position_name
                  << std::setw(8) << profile.total_moves
                  << std::fixed << std::setprecision(1)
                  << std::setw(15) << profile.total_time_ns
                  << std::setw(15) << time_per_move
                  << std::endl;
    }
}

void MoveGenProfiler::print_optimization_recommendations() {
    std::cout << std::endl << "=== OPTIMIZATION RECOMMENDATIONS ===" << std::endl;
    
    // Analyze the profiling data to provide specific recommendations
    std::map<std::string, double> avg_percentages;
    std::map<std::string, double> total_times;
    
    for (const auto& pos_profile : results_) {
        for (const auto& func : pos_profile.function_profiles) {
            avg_percentages[func.function_name] += func.percentage_of_total;
            total_times[func.function_name] += func.total_time_ns;
        }
    }
    
    // Calculate averages
    for (auto& [name, percentage] : avg_percentages) {
        percentage /= results_.size();
    }
    
    // Find the most expensive functions
    std::vector<std::pair<std::string, double>> sorted_by_percentage;
    for (const auto& [name, percentage] : avg_percentages) {
        sorted_by_percentage.push_back({name, percentage});
    }
    std::sort(sorted_by_percentage.begin(), sorted_by_percentage.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::cout << "Based on profiling analysis:" << std::endl << std::endl;
    
    // Provide specific recommendations based on the results
    for (size_t i = 0; i < std::min(size_t(3), sorted_by_percentage.size()); ++i) {
        const auto& [func_name, avg_percentage] = sorted_by_percentage[i];
        std::cout << (i + 1) << ". **" << func_name << "** consumes " 
                  << std::fixed << std::setprecision(1) << avg_percentage 
                  << "% of move generation time on average." << std::endl;
        
        if (func_name == "Pawn Moves") {
            std::cout << "   - Consider optimizing pawn promotion handling" << std::endl;
            std::cout << "   - En passant detection could be streamlined" << std::endl;
            std::cout << "   - Pawn capture direction loops might benefit from unrolling" << std::endl;
        } else if (func_name == "Queen Moves") {
            std::cout << "   - Queens combine rook + bishop moves - consider shared optimization" << std::endl;
            std::cout << "   - Sliding piece direction loops could be optimized" << std::endl;
        } else if (func_name == "Rook Moves" || func_name == "Bishop Moves") {
            std::cout << "   - Sliding piece generation could use bitboard ray attacks" << std::endl;
            std::cout << "   - Consider magic bitboard implementation for better performance" << std::endl;
        } else if (func_name == "Legal Filter") {
            std::cout << "   - Legal move filtering is expensive - consider better early pruning" << std::endl;
            std::cout << "   - Pin-aware move generation could reduce illegal move attempts" << std::endl;
        } else if (func_name == "Knight Moves") {
            std::cout << "   - Knight moves use lookup tables - ensure they're cache-friendly" << std::endl;
        } else if (func_name == "King Moves") {
            std::cout << "   - King moves include castling - consider separating ordinary vs castling moves" << std::endl;
        }
        std::cout << std::endl;
    }
    
    std::cout << "General recommendations:" << std::endl;
    std::cout << "- Focus optimization efforts on the highest percentage functions above" << std::endl;
    std::cout << "- Consider bitboard-based move generation for sliding pieces" << std::endl;
    std::cout << "- Implement pin-aware move generation to reduce legal filtering overhead" << std::endl;
    std::cout << "- Use piece square tables and attack tables for faster lookups" << std::endl;
    std::cout << "- Profile with compiler optimizations (-O3) for production insights" << std::endl;
}

std::string MoveGenProfiler::format_time(double nanoseconds) {
    if (nanoseconds < 1000) {
        return std::to_string(static_cast<int>(nanoseconds)) + " ns";
    } else if (nanoseconds < 1000000) {
        return std::to_string(nanoseconds / 1000.0) + " μs";
    } else {
        return std::to_string(nanoseconds / 1000000.0) + " ms";
    }
}