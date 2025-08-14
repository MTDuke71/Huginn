#pragma once
#include <vector>
#include "position.hpp"
#include "move.hpp"

// Pseudo-legal move list; start small and grow coverage with tests.
struct MoveList {
    std::vector<Move> v;
    void clear(){ v.clear(); }
    void add(const Move& m){ v.push_back(m); }
    size_t size() const { return v.size(); }
    const Move& operator[](size_t i) const { return v[i]; }
};

inline void generate_pseudo_legal_moves(const Position& pos, MoveList& out) {
    out.clear();
    // --- STARTER: knights only (example, white only) ---
    // Expand piece by piece using TDD.
    for (int r=0; r<8; ++r) {
        for (int f=0; f<8; ++f) {
            int s = sq(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = pos.at(s);
            if (is_none(p)) continue;
            if (color_of(p) != pos.side_to_move) continue;

            PieceType t = type_of(p);
            if (t == PieceType::Knight) {
                for (int d : KNIGHT_DELTAS) {
                    int to = s + d;
                    if (!is_playable(to)) continue;
                    Piece q = pos.at(to);
                    if (!is_none(q) && color_of(q) == pos.side_to_move) continue; // own piece
                    out.add(make_move(s, to));
                }
            }
            // TODO: add other pieces via tests
        }
    }
}

// For now, treat pseudo-legal==legal until check logic is added.
inline void generate_legal_moves(const Position& pos, MoveList& out) {
    generate_pseudo_legal_moves(pos, out);
}