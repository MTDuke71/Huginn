#include "gtest/gtest.h"
#include "../src/uci.hpp"
#include "../src/init.hpp"
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>

TEST(UCIIntegration, StopInterruptsQuicklyAndReturnsBestMove) {
    Huginn::init();
    UCIInterface uci;

    // Prepare position
    uci.handle_position(std::vector<std::string>{"position", "startpos"});

    // Prepare go command: long movetime so search would run if not stopped
    std::vector<std::string> go_cmd = {"go", "movetime", "10000"}; // 10s

    // Capture stdout
    std::ostringstream oss;
    auto old_buf = std::cout.rdbuf(oss.rdbuf());

    // Start time measurement
    auto t0 = std::chrono::high_resolution_clock::now();

    // Run search in background
    std::thread search_thread([&uci, &go_cmd]() {
        uci.handle_go(go_cmd);
    });

    // Wait a short time then stop
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    uci.signal_stop();

    // Join and measure
    search_thread.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    // Restore stdout
    std::cout.rdbuf(old_buf);

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // Ensure the search stopped promptly (under 1 second)
    EXPECT_LT(elapsed_ms, 1000) << "Search did not stop quickly enough: " << elapsed_ms << "ms";

    // Ensure a bestmove line was produced
    std::string output = oss.str();
    bool found_bestmove = (output.find("bestmove ") != std::string::npos);
    EXPECT_TRUE(found_bestmove) << "No bestmove found in UCI output:\n" << output;
}

// Note: test runner main is provided by the test harness; no main() here.
