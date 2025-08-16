
#include "move_do.hpp"
#include <cassert>

// ...existing code...

bool make_move(
    Position& b,
    int from, int to,
    Piece promo,
    bool isCastle,
    bool isEnPassant,
    bool isDoublePush,
    State& st
) {
    st.ep_square       = b.ep_square;
    st.castling_rights = b.castling_rights;
    st.halfmove_clock  = b.halfmove_clock;
    st.captured        = Piece::None;
    // ...rest of function logic...
    return true;
}
