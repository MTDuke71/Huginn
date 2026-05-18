/**
 * @file square.hpp
 * @brief 64-square board coordinate helpers.
 */
#pragma once

#include <cstdint>

// Files and ranks are zero-based: A/R1 == 0, H/R8 == 7.
enum class File : uint8_t { A = 0, B, C, D, E, F, G, H, Count = 8, None = 255 };
enum class Rank : uint8_t { R1 = 0, R2, R3, R4, R5, R6, R7, R8, Count = 8, None = 255 };

// Convert (File, Rank) -> square64 (a1=0, h1=7, a8=56, h8=63).
constexpr inline int sq64(File f, Rank r) {
    return static_cast<int>(f) + static_cast<int>(r) * 8;
}

constexpr inline File file_of_sq64(int sq) {
    return (sq >= 0 && sq < 64) ? File(sq & 7) : File::None;
}

constexpr inline Rank rank_of_sq64(int sq) {
    return (sq >= 0 && sq < 64) ? Rank(sq >> 3) : Rank::None;
}
