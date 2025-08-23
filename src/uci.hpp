#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// UCI (Universal Chess Interface) implementation for Huginn chess engine
class UCIInterface {
private:
    Position position;
    std::atomic<bool> is_searching{false};
    std::atomic<bool> should_stop{false};
    bool debug_mode = false;
    std::mt19937 rng{std::random_device{}()};
    
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
    
    // Search for best move (currently returns random valid move)
    void search_best_move();
    
    // Send info during search
    void send_search_info(int depth, int nodes, int time_ms, const S_MOVE& best_move);

public:
    UCIInterface();
    
    // Main UCI loop - processes commands from stdin
    void run();
    
    // Send identification info
    void send_id();
    
    // Send available options
    void send_options();
};
