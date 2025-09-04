#include "gtest/gtest.h"
#include "../src/uci.hpp"
#include "../src/init.hpp"
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>

using GoCmd = std::vector<std::string>;

class UCIParamTest : public ::testing::TestWithParam<GoCmd> {
};

TEST_P(UCIParamTest, StopInterruptsQuicklyAcrossVariants) {
    Huginn::init();
    UCIInterface uci;

    // Set startpos
    uci.handle_position({"position", "startpos"});

    GoCmd go_cmd = GetParam();

    // Capture stdout
    std::ostringstream oss;
    auto old_buf = std::cout.rdbuf(oss.rdbuf());

    auto t0 = std::chrono::high_resolution_clock::now();

    std::thread search_thread([&uci, &go_cmd]() {
        uci.handle_go(go_cmd);
    });

    // Stop after a short delay
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    uci.signal_stop();

    search_thread.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout.rdbuf(old_buf);

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    EXPECT_LT(elapsed_ms, 1000) << "Search did not stop quickly enough for command: " << (go_cmd.size() ? go_cmd[0] : "go") << " (" << elapsed_ms << "ms)";

    std::string output = oss.str();
    EXPECT_NE(output.find("bestmove "), std::string::npos) << "No bestmove produced for command output:\n" << output;
}

INSTANTIATE_TEST_SUITE_P(
    UciVariants,
    UCIParamTest,
    ::testing::Values(
        // movetime
        GoCmd({"go", "movetime", "10000"}),
        // wtime + winc
        GoCmd({"go", "wtime", "60000", "winc", "2000"}),
        // btime + binc + movestogo (position startpos is white to move; still parse checks)
        GoCmd({"go", "btime", "60000", "binc", "1500", "movestogo", "30"}),
        // infinite
        GoCmd({"go", "infinite"}),
        // depth (depth forces depth_only/infinite semantics)
        GoCmd({"go", "depth", "5"})
    )
);

// No main - test runner supplies it
