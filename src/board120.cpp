#include "board120.hpp"

// Algebraic notation parsing - not performance critical, moved from header
int from_algebraic(std::string_view s) {
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

// Algebraic notation formatting - not performance critical, moved from header
const char* to_algebraic(int sq120, char out[3]) {
    if (!is_playable(sq120)) { out[0] = '?'; out[1] = '?'; out[2] = '\0'; return out; }
    const int f = int(file_of(sq120)); // 0..7
    const int r = int(rank_of(sq120)); // 0..7
    out[0] = char('a' + f);
    out[1] = char('1' + r);
    out[2] = '\0';
    return out;
}
