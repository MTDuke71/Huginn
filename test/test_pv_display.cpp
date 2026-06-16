#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/movegen.hpp"
#include "../src/search.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> split_ws(const std::string& text) {
    std::istringstream iss(text);
    std::vector<std::string> out;
    std::string token;
    while (iss >> token) out.push_back(token);
    return out;
}

int pv_move_count_from_info_line(const std::string& line) {
    const auto tokens = split_ws(line);
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "pv") return static_cast<int>(tokens.size() - i - 1);
    }
    return -1;
}

}  // namespace

TEST(PVDisplay, TruncatesTTHorizonAtFiftyMoveRule) {
    Huginn::init();

    Position root;
    ASSERT_TRUE(root.set_from_fen("2Q4k/8/5K2/8/8/8/8/8 b - - 99 1"));

    S_MOVELIST legal_root_moves;
    generate_legal_moves(root, legal_root_moves);
    ASSERT_EQ(legal_root_moves.count, 1);
    EXPECT_EQ(Huginn::Engine::move_to_uci(legal_root_moves.moves[0]), "h8h7");

    Position child = root;
    ASSERT_EQ(child.MakeMove(legal_root_moves.moves[0]), 1);
    ASSERT_EQ(child.halfmove_clock, 100);

    S_MOVELIST child_moves;
    generate_legal_moves(child, child_moves);
    ASSERT_GT(child_moves.count, 0);
    const std::string stale_tt_move = Huginn::Engine::move_to_uci(child_moves.moves[0]);

    Huginn::Engine engine;
    engine.tt_table.store(child.zobrist_key, 0, 1, TTEntry::EXACT,
                          static_cast<uint32_t>(child_moves.moves[0].move));

    Huginn::SearchInfo info;
    info.max_depth = 1;
    info.infinite = true;

    std::ostringstream captured;
    auto* old_buf = std::cout.rdbuf(captured.rdbuf());
    const S_MOVE best = engine.searchPosition(root, info);
    std::cout.rdbuf(old_buf);

    EXPECT_EQ(Huginn::Engine::move_to_uci(best), "h8h7");

    bool saw_info = false;
    std::istringstream lines(captured.str());
    std::string line;
    while (std::getline(lines, line)) {
        if (line.find(" pv ") == std::string::npos) continue;
        saw_info = true;
        EXPECT_EQ(pv_move_count_from_info_line(line), 1)
            << "PV should stop after the draw-creating move, not continue with "
            << stale_tt_move << ":\n" << line;
        EXPECT_EQ(line.find(stale_tt_move), std::string::npos)
            << "PV continued past the fifty-move-rule horizon:\n" << line;
    }
    EXPECT_TRUE(saw_info) << "search did not emit an info pv line:\n" << captured.str();
}
