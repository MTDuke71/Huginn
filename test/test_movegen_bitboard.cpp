#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"
#include <iostream>

TEST(MovegenBitboard, Startpos_d1_is_20) {
    std::cout << "Running MovegenBitboard.Startpos_d1_is_20" << std::endl;
    Position pos;
    pos.set_startpos();
    MoveList list;
    generate_legal_moves(pos, list);
    for (const auto& move : list.v) {
        if (move.move == 2950) {
            int from, to;
            PieceType promo;
            S_MOVE::decode_move(move.move, from, to, promo);
            std::cout << "Extra move: from=" << from << ", to=" << to << ", promo=" << static_cast<int>(promo) << std::endl;
        }
    }
    EXPECT_EQ(list.size(), 20u);
}