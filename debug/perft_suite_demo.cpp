#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "../src/position.hpp"
#include "../src/movegen.hpp"

// Perft function - counts all legal move paths to a given depth
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; 
    generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        pos.make_move_with_undo(m);
        nodes += perft(pos, depth - 1);
        pos.undo_move();
    }
    return nodes;
}

// Structure to hold a perft test case
struct PerftTestCase {
    std::string fen;
    std::vector<std::pair<int, uint64_t>> expected_results; // depth, node_count pairs
};

// Parse a line from the EPD file
PerftTestCase parse_epd_line(const std::string& line) {
    PerftTestCase test_case;
    
    // Find the first semicolon to separate FEN from perft data
    size_t first_semicolon = line.find(';');
    if (first_semicolon == std::string::npos) {
        return test_case; // Invalid line
    }
    
    test_case.fen = line.substr(0, first_semicolon);
    
    // Parse perft results
    std::string perft_data = line.substr(first_semicolon + 1);
    std::istringstream ss(perft_data);
    std::string token;
    
    while (std::getline(ss, token, ';')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        
        if (token.empty()) continue;
        
        // Parse "D<depth> <count>"
        if (token[0] == 'D') {
            size_t space_pos = token.find(' ');
            if (space_pos != std::string::npos) {
                int depth = std::stoi(token.substr(1, space_pos - 1));
                uint64_t count = std::stoull(token.substr(space_pos + 1));
                test_case.expected_results.push_back({depth, count});
            }
        }
    }
    
    return test_case;
}

// Load all test cases from the EPD file
std::vector<PerftTestCase> load_perft_suite(const std::string& filename) {
    std::vector<PerftTestCase> test_cases;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return test_cases;
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        
        PerftTestCase test_case = parse_epd_line(line);
        if (!test_case.fen.empty() && !test_case.expected_results.empty()) {
            test_cases.push_back(test_case);
        } else {
            std::cerr << "Warning: Failed to parse line " << line_number << ": " << line << std::endl;
        }
    }
    
    file.close();
    return test_cases;
}

// Test a single position up to the specified max depth
bool test_position(const PerftTestCase& test_case, int max_depth, int& total_tests, int& failed_tests) {
    Position pos;
    
    if (!pos.set_from_fen(test_case.fen)) {
        std::cout << "FAILED to parse FEN: " << test_case.fen << std::endl;
        failed_tests++;
        total_tests++;
        return true; // Return true to indicate failure (stop testing)
    }
    
    // Collect all depths for this position
    std::vector<int> depths_to_test;
    for (const auto& expected : test_case.expected_results) {
        if (expected.first <= max_depth) {
            depths_to_test.push_back(expected.first);
        }
    }
    
    if (depths_to_test.empty()) {
        return false; // No tests to run for this position
    }
    
    // Test each depth
    for (const auto& expected : test_case.expected_results) {
        int depth = expected.first;
        uint64_t expected_nodes = expected.second;
        
        if (depth > max_depth) continue; // Skip depths beyond our limit
        
        total_tests++;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t actual_nodes = perft(pos, depth);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        if (actual_nodes == expected_nodes) {
            // For passing tests, show a brief single line
            if (depth == *depths_to_test.rbegin()) { // Only show for the last depth tested
                std::cout << "PASS - depths 1-" << depth << " (" << duration.count() << "ms)" << std::endl;
            }
        } else {
            // First failure detected - show detailed information and stop
            std::cout << "\n=== FIRST FAILURE DETECTED ===" << std::endl;
            std::cout << "FAIL: Depth " << depth << std::endl;
            std::cout << "  FEN: " << test_case.fen << std::endl;
            std::cout << "  Expected: " << expected_nodes << std::endl;
            std::cout << "  Actual:   " << actual_nodes << std::endl;
            std::cout << "  Time: " << duration.count() << "ms" << std::endl;
            std::cout << "\n=== STOPPING AT FIRST FAILURE FOR DEBUGGING ===" << std::endl;
            failed_tests++;
            return true; // Return true to indicate we should stop testing
        }
    }
    
    return false; // Return false to continue testing
}

int main(int argc, char* argv[]) {
    std::cout << "=== Perft Suite Demo ===" << std::endl;
    std::cout << "Testing chess engine against comprehensive perft suite" << std::endl;
    std::cout << std::endl;
    
    // Default parameters
    int max_depth = 6;
    std::string epd_file = "test/perftsuite.epd";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--depth" && i + 1 < argc) {
            max_depth = std::stoi(argv[++i]);
        } else if (arg == "--file" && i + 1 < argc) {
            epd_file = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --depth <n>     Maximum depth to test (default: 6)" << std::endl;
            std::cout << "  --file <path>   Path to EPD file (default: test/perftsuite.epd)" << std::endl;
            std::cout << "  --help, -h      Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "Configuration:" << std::endl;
    std::cout << "  EPD file: " << epd_file << std::endl;
    std::cout << "  Max depth: " << max_depth << std::endl;
    std::cout << std::endl;
    
    // Load test cases
    std::vector<PerftTestCase> test_cases = load_perft_suite(epd_file);
    
    if (test_cases.empty()) {
        std::cerr << "Error: No test cases loaded from " << epd_file << std::endl;
        return 1;
    }
    
    std::cout << "Loaded " << test_cases.size() << " test positions" << std::endl;
    std::cout << std::endl;
    
    // Count total expected tests
    int total_expected_tests = 0;
    for (const auto& test_case : test_cases) {
        for (const auto& expected : test_case.expected_results) {
            if (expected.first <= max_depth) {
                total_expected_tests++;
            }
        }
    }
    
    std::cout << "Will run approximately " << total_expected_tests << " tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    auto overall_start_time = std::chrono::high_resolution_clock::now();
    
    int total_tests = 0;
    int failed_tests = 0;
    
    // Test each position
    for (size_t i = 0; i < test_cases.size(); i++) {
        std::cout << "[" << (i + 1) << "/" << test_cases.size() << "] ";
        bool should_stop = test_position(test_cases[i], max_depth, total_tests, failed_tests);
        if (should_stop) {
            std::cout << "\nStopped testing at position " << (i + 1) << " due to failure." << std::endl;
            break;
        }
    }
    
    auto overall_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(overall_end_time - overall_start_time);
    
    // Summary
    std::cout << "========================================" << std::endl;
    std::cout << "=== FINAL RESULTS ===" << std::endl;
    std::cout << "Total tests run: " << total_tests << std::endl;
    std::cout << "Tests passed: " << (total_tests - failed_tests) << std::endl;
    std::cout << "Tests failed: " << failed_tests << std::endl;
    std::cout << "Success rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * (total_tests - failed_tests) / total_tests) << "%" << std::endl;
    std::cout << "Total time: " << total_duration.count() << "ms" << std::endl;
    
    if (failed_tests == 0) {
        std::cout << std::endl << "🎉 ALL TESTS PASSED! Chess engine is working correctly! 🎉" << std::endl;
    } else {
        std::cout << std::endl << "❌ Some tests failed. Please check the output above for details." << std::endl;
    }
    
    return failed_tests > 0 ? 1 : 0;
}
