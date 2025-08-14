#pragma once
#include <cstdint>
#include "board120.hpp"
#include "chess_types.hpp"

// Simple move: from → to (+ optional promotion)
struct Move {
    int from;          // mailbox-120 index
    int to;            // mailbox-120 index
    PieceType promo;   // PieceType::None if not a promotion
};

inline constexpr Move make_move(int from, int to, PieceType promo = PieceType::None) {
    return Move{from, to, promo};
}

inline constexpr bool operator==(const Move& a, const Move& b) {
    return a.from == b.from && a.to == b.to && a.promo == b.promo;
}