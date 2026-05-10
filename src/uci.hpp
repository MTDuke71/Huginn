/**
 * @file uci.hpp
 * @brief Universal Chess Interface (UCI) protocol implementation
 * 
 * Implements the UCI protocol for communication between the Huginn chess engine
 * and UCI-compatible chess GUIs. The UCI protocol is the standard interface used
 * by most modern chess programs and provides a clean separation between the
 * engine's chess logic and the user interface.
 * 
 * ## UCI Protocol Features
 * - **Standard Commands**: uci, isready, position, go, stop, quit
 * - **Engine Options**: Hash table size, threads, opening book settings
 * - **Search Control**: Time management, depth limits, node limits
 * - **Move Communication**: Algebraic notation move parsing and output
 * 
 * ## Implementation Details
 * - **Asynchronous Search**: Multi-threaded search with stop capability
 * - **Thread Safety**: Atomic operations for search control variables
 * - **Command Parsing**: Robust input parsing with error handling
 * - **Debug Mode**: Enhanced logging for debugging and analysis
 * 
 * ## Search Integration
 * - Engine integration for search operations
 * - SearchInfo structure for search state management
 * - Time management and iteration control
 * - Principal variation and score reporting
 * 
 * @author MTDuke71
 * @version 1.2
 * @see search.hpp for search engine interface
 * @see https://www.shredderchess.com/chess-info/features/uci-universal-chess-interface.html
 */

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include "position.hpp"
#include "movegen.hpp"
#include "search.hpp"  // Changed from search.hpp
#include "syzygy_tablebase.hpp"

// UCI constants following VICE Part 67 recommendations
constexpr int UCI_INPUT_BUFFER_SIZE = 400 * 6; // Large buffer for GUI commands

// UCI (Universal Chess Interface) implementation for Huginn chess engine
class UCIInterface {
private:
    Position position;
    std::unique_ptr<Huginn::Engine> search_engine;  // Changed from SimpleEngine
    std::unique_ptr<Huginn::SyzygyTablebase> tablebase;
    std::atomic<bool> is_searching{false};
    std::atomic<bool> should_stop{false};
    // Pointer to running SearchInfo so stop() can update it safely
    std::atomic<Huginn::SearchInfo*> running_info{nullptr};
    bool debug_mode = false;
    int threads = 1; // Default to 1 thread to test for threading issues
    
    // Opening book settings
    bool own_book = true;  // Enable opening book by default
    std::string book_file = "src/performance.bin";  // Default book file path
    
    // Parse a UCI move string (e.g., "e2e4", "e7e8q") to our internal move format
public:
    
    // Split string by whitespace
    std::vector<std::string> split_string(const std::string& str);
    
    // Handle position command
    void handle_position(const std::vector<std::string>& tokens);
    
    // Handle go command
    void handle_go(const std::vector<std::string>& tokens);
    
    // Handle setoption command
    void handle_setoption(const std::vector<std::string>& tokens);
    
    // Search for best move using the search engine
    void search_best_move(const Huginn::MinimalLimits& limits);  // Changed from SearchLimits
    
    // Load opening book
    void load_opening_book();

public:
    UCIInterface();
    
    // Main UCI loop - processes commands from stdin
    void run();

    // Test helper: signal stop to the running search (mirrors UCI 'stop' command)
    void signal_stop();
    
    // Send identification info
    void send_id();
    
    // Send available options
    void send_options();
};
