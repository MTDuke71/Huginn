#include "move_do.hpp"
#include <cassert>

// ---- local hashing helpers (cpp-only) ----
static inline void hash_side(S_BOARD& b) { b.posKey ^= Zobrist::Side; }

static inline void hash_castle(S_BOARD& b) {
    b.posKey ^= Zobrist::Castle[b.castlePerm & 0xF];
}

static inline void hash_ep(S_BOARD& b) {
    if (b.enPas != NO_SQ) {
        const int ff = static_cast<int>(file_of(b.enPas)); // 0..7
        if (ff >= 0 && ff < 8) b.posKey ^= Zobrist::EpFile[ff];
    }
}

static inline void hash_piece_on(S_BOARD& b, int pc, int s120) {
    const int s64 = MAILBOX_MAPS.to64[s120];
    if (s64 >= 0) b.posKey ^= Zobrist::Piece[pc][s64];
}

// Clear rights when king/rook moves, or a rook is captured on its start square.
int updated_castle_perm_after(const S_BOARD& b, int from, int to, int capturedPc) {
    int m = b.castlePerm;

    // King moves lose both rights for that side
    if (from == sq(File::E, Rank::R1)) m &= ~(WKCA | WQCA);
    if (from == sq(File::E, Rank::R8)) m &= ~(BKCA | BQCA);

    // Rook moves lose the rook's side right
    if (from == sq(File::A, Rank::R1)) m &= ~WQCA;
    if (from == sq(File::H, Rank::R1)) m &= ~WKCA;
    if (from == sq(File::A, Rank::R8)) m &= ~BQCA;
    if (from == sq(File::H, Rank::R8)) m &= ~BKCA;

    // Capturing a rook on its original square also clears that right
    if (capturedPc != EMPTY) {
        if (to == sq(File::A, Rank::R1)) m &= ~WQCA;
        if (to == sq(File::H, Rank::R1)) m &= ~WKCA;
        if (to == sq(File::A, Rank::R8)) m &= ~BQCA;
        if (to == sq(File::H, Rank::R8)) m &= ~BKCA;
    }
    return m;
}

bool make_move(
    S_BOARD& b,
    int from, int to,
    int promoPc,
    bool isCastle,
    bool isEnPassant,
    bool isDoublePush,
    State& st
) {
    // Save previous state
    st.enPasPrev  = b.enPas;
    st.castlePrev = b.castlePerm;
    st.fiftyPrev  = b.fiftyMove;
    st.sidePrev   = b.side;
    st.posKeyPrev = b.posKey;
    st.capturedPc = EMPTY;

    // Remove old EP & castle from hash (they'll change)
    hash_ep(b);
    hash_castle(b);

    // Clear EP by default (set later if double push)
    b.enPas = NO_SQ;

    const int movingPc   = b.pieces[from];
    int       capturedPc = b.pieces[to];

    // EP capture: captured pawn is *behind* 'to'
    if (isEnPassant) {
        const int capSq = to + (b.side == WHITE ? SOUTH : NORTH);
        capturedPc = b.pieces[capSq];
        st.capturedPc = capturedPc;
        hash_piece_on(b, capturedPc, capSq);
        b.pieces[capSq] = EMPTY;
    } else if (capturedPc != EMPTY) {
        st.capturedPc = capturedPc;
        hash_piece_on(b, capturedPc, to);
    }

    // Hash out moving piece from 'from'
    hash_piece_on(b, movingPc, from);
    b.pieces[from] = EMPTY;

    // Promotion?
    int placedPc = movingPc;
    if (promoPc != 0 && promoPc != EMPTY) {
        placedPc = promoPc;
    }

    // Handle the rook move for castling
    if (isCastle) {
        if (b.side == WHITE) {
            if (to == sq(File::G, Rank::R1)) {          // O-O: h1 -> f1
                const int rf = sq(File::H, Rank::R1), rt = sq(File::F, Rank::R1);
                const int rook = b.pieces[rf];
                hash_piece_on(b, rook, rf);
                b.pieces[rf] = EMPTY;
                b.pieces[rt] = rook;
                hash_piece_on(b, rook, rt);
            } else if (to == sq(File::C, Rank::R1)) {   // O-O-O: a1 -> d1
                const int rf = sq(File::A, Rank::R1), rt = sq(File::D, Rank::R1);
                const int rook = b.pieces[rf];
                hash_piece_on(b, rook, rf);
                b.pieces[rf] = EMPTY;
                b.pieces[rt] = rook;
                hash_piece_on(b, rook, rt);
            }
        } else { // BLACK
            if (to == sq(File::G, Rank::R8)) {          // O-O: h8 -> f8
                const int rf = sq(File::H, Rank::R8), rt = sq(File::F, Rank::R8);
                const int rook = b.pieces[rf];
                hash_piece_on(b, rook, rf);
                b.pieces[rf] = EMPTY;
                b.pieces[rt] = rook;
                hash_piece_on(b, rook, rt);
            } else if (to == sq(File::C, Rank::R8)) {   // O-O-O: a8 -> d8
                const int rf = sq(File::A, Rank::R8), rt = sq(File::D, Rank::R8);
                const int rook = b.pieces[rf];
                hash_piece_on(b, rook, rf);
                b.pieces[rf] = EMPTY;
                b.pieces[rt] = rook;
                hash_piece_on(b, rook, rt);
            }
        }
    }

    // Place moving/promo piece on 'to'
    b.pieces[to] = placedPc;
    hash_piece_on(b, placedPc, to);

    // Update clocks
    const bool movedPawn = (movingPc == wP || movingPc == bP);
    if (movedPawn || st.capturedPc != EMPTY) b.fiftyMove = 0;
    else                                     ++b.fiftyMove;

    // Update castle rights
    b.castlePerm = updated_castle_perm_after(b, from, to, st.capturedPc);

    // Double pawn push sets EP square to the jumped-over square
    if (isDoublePush) {
        b.enPas = from + (b.side == WHITE ? NORTH : SOUTH);
    }

    // Hash IN new EP & castle
    hash_ep(b);
    hash_castle(b);

    // Toggle side and hash it
    b.side = (b.side == WHITE ? BLACK : WHITE);
    hash_side(b);

#ifndef NDEBUG
    if (Zobrist::Initialized) {
        const U64 chk = Zobrist::compute(b);
        assert(chk == b.posKey && "posKey mismatch after make_move()");
    }
#endif

    // (You can update counters/bitboards incrementally here if you keep them in sync.)
    return true;
}

void unmake_move(
    S_BOARD& b,
    int from, int to,
    int promoPc,
    bool isCastle,
    bool isEnPassant,
    bool /*isDoublePush*/,
    const State& st
) {
    // Restore side & clocks first
    b.side        = st.sidePrev;
    b.enPas       = st.enPasPrev;
    b.castlePerm  = st.castlePrev;
    b.fiftyMove   = st.fiftyPrev;

    // Undo placement (board only; we restore posKey from snapshot)
    int moved = b.pieces[to];
    if (promoPc != 0 && promoPc != EMPTY) {
        moved = (b.side == WHITE ? wP : bP);
    }

    b.pieces[from] = moved;
    b.pieces[to]   = (st.capturedPc == EMPTY ? EMPTY : st.capturedPc);

    if (isEnPassant && st.capturedPc != EMPTY) {
        const int capSq = to + (b.side == WHITE ? SOUTH : NORTH);
        b.pieces[capSq] = st.capturedPc;
        b.pieces[to]    = EMPTY;
    }

    if (isCastle) {
        if (b.side == WHITE) {
            if (to == sq(File::G, Rank::R1)) { // rook f1 -> h1
                b.pieces[sq(File::H, Rank::R1)] = wR;
                b.pieces[sq(File::F, Rank::R1)] = EMPTY;
            } else if (to == sq(File::C, Rank::R1)) { // rook d1 -> a1
                b.pieces[sq(File::A, Rank::R1)] = wR;
                b.pieces[sq(File::D, Rank::R1)] = EMPTY;
            }
        } else {
            if (to == sq(File::G, Rank::R8)) { // rook f8 -> h8
                b.pieces[sq(File::H, Rank::R8)] = bR;
                b.pieces[sq(File::F, Rank::R8)] = EMPTY;
            } else if (to == sq(File::C, Rank::R8)) { // rook d8 -> a8
                b.pieces[sq(File::A, Rank::R8)] = bR;
                b.pieces[sq(File::D, Rank::R8)] = EMPTY;
            }
        }
    }

    // Restore the exact Zobrist key snapshot (fast & robust)
    b.posKey = st.posKeyPrev;

#ifndef NDEBUG
    if (Zobrist::Initialized) {
        const U64 chk = Zobrist::compute(b);
        assert(chk == b.posKey && "posKey mismatch after unmake_move()");
    }
#endif
}
