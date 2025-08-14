// squares120.hpp (EXTRAS ONLY)
#pragma once
#include <array>
#include "board120.hpp"  // provides File, Rank, sq(), is_playable, etc.

// Named constants (computed via sq())
namespace Square120 {
    constexpr int A1 = sq(File::A, Rank::R1); constexpr int B1 = sq(File::B, Rank::R1);
    constexpr int C1 = sq(File::C, Rank::R1); constexpr int D1 = sq(File::D, Rank::R1);
    constexpr int E1 = sq(File::E, Rank::R1); constexpr int F1 = sq(File::F, Rank::R1);
    constexpr int G1 = sq(File::G, Rank::R1); constexpr int H1 = sq(File::H, Rank::R1);

    constexpr int A2 = sq(File::A, Rank::R2); constexpr int B2 = sq(File::B, Rank::R2);
    constexpr int C2 = sq(File::C, Rank::R2); constexpr int D2 = sq(File::D, Rank::R2);
    constexpr int E2 = sq(File::E, Rank::R2); constexpr int F2 = sq(File::F, Rank::R2);
    constexpr int G2 = sq(File::G, Rank::R2); constexpr int H2 = sq(File::H, Rank::R2);

    constexpr int A3 = sq(File::A, Rank::R3); constexpr int B3 = sq(File::B, Rank::R3);
    constexpr int C3 = sq(File::C, Rank::R3); constexpr int D3 = sq(File::D, Rank::R3);
    constexpr int E3 = sq(File::E, Rank::R3); constexpr int F3 = sq(File::F, Rank::R3);
    constexpr int G3 = sq(File::G, Rank::R3); constexpr int H3 = sq(File::H, Rank::R3);

    constexpr int A4 = sq(File::A, Rank::R4); constexpr int B4 = sq(File::B, Rank::R4);
    constexpr int C4 = sq(File::C, Rank::R4); constexpr int D4 = sq(File::D, Rank::R4);
    constexpr int E4 = sq(File::E, Rank::R4); constexpr int F4 = sq(File::F, Rank::R4);
    constexpr int G4 = sq(File::G, Rank::R4); constexpr int H4 = sq(File::H, Rank::R4);

    constexpr int A5 = sq(File::A, Rank::R5); constexpr int B5 = sq(File::B, Rank::R5);
    constexpr int C5 = sq(File::C, Rank::R5); constexpr int D5 = sq(File::D, Rank::R5);
    constexpr int E5 = sq(File::E, Rank::R5); constexpr int F5 = sq(File::F, Rank::R5);
    constexpr int G5 = sq(File::G, Rank::R5); constexpr int H5 = sq(File::H, Rank::R5);

    constexpr int A6 = sq(File::A, Rank::R6); constexpr int B6 = sq(File::B, Rank::R6);
    constexpr int C6 = sq(File::C, Rank::R6); constexpr int D6 = sq(File::D, Rank::R6);
    constexpr int E6 = sq(File::E, Rank::R6); constexpr int F6 = sq(File::F, Rank::R6);
    constexpr int G6 = sq(File::G, Rank::R6); constexpr int H6 = sq(File::H, Rank::R6);

    constexpr int A7 = sq(File::A, Rank::R7); constexpr int B7 = sq(File::B, Rank::R7);
    constexpr int C7 = sq(File::C, Rank::R7); constexpr int D7 = sq(File::D, Rank::R7);
    constexpr int E7 = sq(File::E, Rank::R7); constexpr int F7 = sq(File::F, Rank::R7);
    constexpr int G7 = sq(File::G, Rank::R7); constexpr int H7 = sq(File::H, Rank::R7);

    constexpr int A8 = sq(File::A, Rank::R8); constexpr int B8 = sq(File::B, Rank::R8);
    constexpr int C8 = sq(File::C, Rank::R8); constexpr int D8 = sq(File::D, Rank::R8);
    constexpr int E8 = sq(File::E, Rank::R8); constexpr int F8 = sq(File::F, Rank::R8);
    constexpr int G8 = sq(File::G, Rank::R8); constexpr int H8 = sq(File::H, Rank::R8);
}

// All playable squares, as 120 indices (21..98)
constexpr inline std::array<int,64> ALL120 = []{
    std::array<int,64> a{};
    int k=0;
    for (int r=0;r<8;++r)
        for (int f=0;f<8;++f)
            a[size_t(k++)] = sq(static_cast<File>(f), static_cast<Rank>(r));
    return a;
}();

// Optional: iterate all playable 120 squares
struct Playable120 {
    struct It { int i; int operator*() const { return ALL120[size_t(i)]; }
                bool operator!=(const It& o) const { return i!=o.i; }
                void operator++(){ ++i; } };
    It begin() const { return {0}; }
    It end()   const { return {64}; }
};
