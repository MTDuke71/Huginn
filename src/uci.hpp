#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "minimal_search.hpp"  // Changed from search.hpp

// UCI (Universal Chess Interface) implementation for Huginn chess engine
class UCIInterface {
private:
    Position position;
    std::unique_ptr<Huginn::MinimalEngine> search_engine;  // Changed from SimpleEngine
    std::atomic<bool> is_searching{false};
    std::atomic<bool> should_stop{false};
    // Pointer to running SearchInfo so stop() can update it safely
    std::atomic<Huginn::SearchInfo*> running_info{nullptr};
    bool debug_mode = false;
    int threads = 1; // Default to 1 thread to test for threading issues
    
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
