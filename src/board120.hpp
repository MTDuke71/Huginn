// board120.hpp
#pragma once
#include <cstdint>
#include <array>
#include <string_view>
#include <cassert>

// -------------- Files & Ranks (0..7) --------------
enum class File : uint8_t { A=0, B, C, D, E, F, G, H, Count=8, None=255 };
enum class Rank : uint8_t { R1=0, R2, R3, R4, R5, R6, R7, R8, Count=8, None=255 };

// -------------- Square (mailbox 120) --------------
// Layout is 10 columns x 12 rows, indices 0..119.
// Playable squares are [21..98] where (sq % 10) in [1..8] and (sq / 10) in [2..9].
// A1 = 21, B1 = 22, ..., H1 = 28
// A2 = 31, ..., H8 = 98
enum class Square : int16_t {
    Offboard = -1
};

// Offsets in mailbox-120
constexpr inline int NORTH = +10;
constexpr inline int SOUTH = -10;
constexpr inline int EAST  = +1;
constexpr inline int WEST  = -1;

constexpr inline int NE = NORTH + EAST;   // +11
constexpr inline int NW = NORTH + WEST;   // +9
constexpr inline int SE = SOUTH + EAST;   // -9
constexpr inline int SW = SOUTH + WEST;   // -11

// Knight deltas for mailbox-120
constexpr inline int KNIGHT_DELTAS[8] = { +21, +19, +12, +8, -8, -12, -19, -21 };

// King steps
constexpr inline  std::array<int, 8> KING_DELTAS = {
    NORTH, SOUTH, EAST, WEST,
    NE, NW, SE, SW
};

// Piece deltas
constexpr inline int PIECE_DELTAS[7][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, // None
    {0, 0, 0, 0, 0, 0, 0, 0}, // Pawn
    {+21, +19, +12, +8, -8, -12, -19, -21}, // Knight
    {+11, +9, -9, -11, 0, 0, 0, 0}, // Bishop
    {+10, -10, +1, -1, 0, 0, 0, 0}, // Rook
    {+10, -10, +1, -1, +11, +9, -9, -11}, // Queen
    {+10, -10, +1, -1, +11, +9, -9, -11}  // King
};

// For convenience
constexpr inline bool in_range(int v, int lo, int hi) { return v >= lo && v <= hi; }

// Convert (File, Rank) -> square120
constexpr inline int sq(File f, Rank r) {
    const int ff = static_cast<int>(f);
    const int rr = static_cast<int>(r);
    // Base playable square index: 21 + file + 10*rank
    return 21 + ff + rr * 10;
}

// -------------- Lookup tables for file/rank extraction --------------
struct FileRankLookups {
    std::array<File, 120> files{};
    std::array<Rank, 120> ranks{};
    std::array<bool, 120> playable{};

    constexpr FileRankLookups() : files{}, ranks{}, playable{} {
        // Initialize all squares to None/false (offboard)
        for (int i = 0; i < 120; ++i) {
            files[i] = File::None;
            ranks[i] = Rank::None;
            playable[i] = false;
        }
        // Fill playable squares
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 8; ++f) {
                const int sq120 = 21 + f + r * 10;
                files[size_t(sq120)] = File(f);
                ranks[size_t(sq120)] = Rank(r);
                playable[size_t(sq120)] = true;
            }
        }
    }
};
constexpr inline FileRankLookups FILE_RANK_LOOKUPS{};

// Split square120 -> (File, Rank) using lookup tables
// Returns File::None/Rank::None for offboard squares
constexpr inline File file_of(int sq120) {
    if (sq120 == int(Square::Offboard) || sq120 < 0 || sq120 >= 120) return File::None;
    return FILE_RANK_LOOKUPS.files[sq120];
}
constexpr inline Rank rank_of(int sq120) {
    if (sq120 == int(Square::Offboard) || sq120 < 0 || sq120 >= 120) return Rank::None;
    return FILE_RANK_LOOKUPS.ranks[sq120];
}

// Checks using lookup table - no calculations needed
constexpr inline bool is_playable(int sq120) {
    if (sq120 == int(Square::Offboard) || sq120 < 0 || sq120 >= 120) return false;
    return FILE_RANK_LOOKUPS.playable[sq120];
}
constexpr inline bool is_offboard(int sq120) { return !is_playable(sq120); }

// Algebraic <-> square (e.g., "e4")
constexpr inline int from_algebraic(std::string_view s) {
    // Accept "e4" style, lowercase or uppercase files
    if (s.size() != 2) return int(Square::Offboard);
    char fch = s[0];
    char rch = s[1];
    if (fch >= 'A' && fch <= 'H') fch = char(fch - 'A' + 'a');

    if (fch < 'a' || fch > 'h') return int(Square::Offboard);
    if (rch < '1' || rch > '8') return int(Square::Offboard);

    File f = File(uint8_t(fch - 'a'));     // 0..7
    Rank r = Rank(uint8_t(rch - '1'));     // 0..7
    return sq(f, r);                        // 21..98 playable
}

constexpr inline const char* to_algebraic(int sq120, char out[3]) {
    if (!is_playable(sq120)) { out[0] = '?'; out[1] = '?'; out[2] = '\0'; return out; }
    const int f = int(file_of(sq120)); // 0..7
    const int r = int(rank_of(sq120)); // 0..7
    out[0] = char('a' + f);
    out[1] = char('1' + r);
    out[2] = '\0';
    return out;
}

// Pawn push/attack directions in mailbox-120
// White moves "north" (+10), Black moves "south" (-10)
constexpr inline int pawn_push_dir_white() { return NORTH; }
constexpr inline int pawn_push_dir_black() { return SOUTH; }
constexpr inline int pawn_capt_left_white() { return NW; } // from White's perspective
constexpr inline int pawn_capt_right_white(){ return NE; }
constexpr inline int pawn_capt_left_black() { return SE; } // from Black's perspective
constexpr inline int pawn_capt_right_black(){ return SW; }

// -------------- Optional 64 <-> 120 maps --------------
// If you also keep a 0..63 representation, these help translate.
// idx64: file + 8*rank (A1=0, H1=7, A8=56, H8=63)
struct MailboxMaps {
    std::array<int, 64> to120{};
    std::array<int, 120> to64{};

    constexpr MailboxMaps() : to120{}, to64{} {
        // Initialize to64 with offboard = -1
        for (int i = 0; i < 120; ++i) to64[i] = -1;
        // Fill playable squares
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 8; ++f) {
                const int sq120 = 21 + f + r * 10;
                const int sq64  = f + r * 8;
                to120[size_t(sq64)] = sq120;
                to64[size_t(sq120)] = sq64;
            }
        }
    }
};
constexpr inline MailboxMaps MAILBOX_MAPS{};