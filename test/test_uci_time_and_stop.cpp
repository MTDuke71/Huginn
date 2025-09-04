#include "../src/uci.hpp"
#include "../src/init.hpp"
#include <thread>
#include <chrono>
#include <iostream>

// Simple integration test: ensure UCIInterface time allocation and stop work
int main() {
    Huginn::init();
    UCIInterface uci;

    // Set a complex position that may take some time to search
    uci.handle_position(std::vector<std::string>{"position", "startpos"});

    // Prepare limits via 'go' parsing: give a large movetime so search would run long
    std::vector<std::string> go_cmd = {"go", "movetime", "10000"}; // 10s movetime

    // Run search_best_move in a separate thread
    std::thread search_thread([&uci, &go_cmd]() {
        uci.handle_go(go_cmd);
    });

    // Wait briefly and then send stop
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Simulate UCI stop command by calling the public helper
    uci.signal_stop();

    // Join the search thread
    if (search_thread.joinable()) search_thread.join();

    std::cout << "UCI time/stop integration test completed" << std::endl;
    return 0;
}
