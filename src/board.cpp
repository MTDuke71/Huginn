// board.cpp
#include "board.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include <cstring>

// Reset the board to an empty, clean state
void reset_board(Position& pos) {
    pos.reset();
}
