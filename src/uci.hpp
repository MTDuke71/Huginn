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
#include "../Engine3_src/simple_search.hpp"

// UCI (Universal Chess Interface) implementation for Huginn chess engine
class UCIInterface {
private:
    Position position;
    std::unique_ptr<Engine3::SimpleEngine> search_engine;
    std::atomic<bool> is_searching{false};
    std::atomic<bool> should_stop{false};
    bool debug_mode = false;
    
    // Parse a UCI move string (e.g., "e2e4", "e7e8q") to our internal move format
    S_MOVE parse_uci_move(const std::string& uci_move);
    
    // Convert our internal move to UCI string format
    std::string move_to_uci(const S_MOVE& move);
    
    // Split string by whitespace
    std::vector<std::string> split_string(const std::string& str);
    
    // Handle position command
    void handle_position(const std::vector<std::string>& tokens);
    
    // Handle go command
    void handle_go(const std::vector<std::string>& tokens);
    
    // Handle setoption command
    void handle_setoption(const std::vector<std::string>& tokens);
    
    // Search for best move using the search engine
    void search_best_move(const Engine3::SearchLimits& limits);

public:
    UCIInterface();
    
    // Main UCI loop - processes commands from stdin
    void run();
    
    // Send identification info
    void send_id();
    
    // Send available options
    void send_options();
};
