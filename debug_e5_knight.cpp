#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>

std::string square_to_algebraic_local(int square_64) {
    if (square_64 < 0 || square_64 >= 64) return "-";
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

int algebraic_to_square_local(const std::string& algebraic) {
    if (algebraic == "-" || algebraic.length() != 2) return -1;
    int file = algebraic[0] - 'a';
    int rank = algebraic[1] - '1';
    if (file < 0 || file >= 8 || rank < 0 || rank >= 8) return -1;
    return rank * 8 + file;
}

void print_bitboard(uint64_t bb, const std::string& title) {
    std::cout << title << ":\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            if (bb & (1ULL << square)) {
                std::cout << "X ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

int main() {
    std::cout << "=== Debugging E5 Knight Moves ===\n";

    // Initialize attack tables
    ensure_bitboard_attacks_initialized();

    int e5_square = algebraic_to_square_local("e5");
    std::cout << "E5 square index: " << e5_square << std::endl;

    // Get knight attacks for e5
    uint64_t e5_attacks = get_knight_attacks(e5_square);
    print_bitboard(e5_attacks, "E5 Knight Attacks");

    // Check specific squares
    std::vector<std::string> expected_moves = {"d3", "f3", "c4", "g4", "c6", "g6", "d7", "f7"};

    std::cout << "Checking individual knight moves from e5:\n";
    for (const auto& move : expected_moves) {
        int target_square = algebraic_to_square_local(move);
        bool is_in_attacks = (e5_attacks & (1ULL << target_square)) != 0;
        std::cout << "e5" << move << ": square=" << target_square
                  << " present=" << (is_in_attacks ? "YES" : "NO") << std::endl;
    }

    // Manual calculation of knight moves from e5
    std::cout << "\nManual knight move calculation from e5 (square " << e5_square << "):\n";
    int file = e5_square % 8;  // e = 4
    int rank = e5_square / 8;  // 5 = 4
    std::cout << "E5 file=" << file << " rank=" << rank << std::endl;

    // All possible knight moves (2,1) and (1,2) patterns
    int knight_moves[][2] = {
        {-2, -1}, {-2, +1}, {-1, -2}, {-1, +2},
        {+1, -2}, {+1, +2}, {+2, -1}, {+2, +1}
    };

    std::cout << "Manual knight moves:\n";
    for (int i = 0; i < 8; ++i) {
        int new_file = file + knight_moves[i][0];
        int new_rank = rank + knight_moves[i][1];

        if (new_file >= 0 && new_file < 8 && new_rank >= 0 && new_rank < 8) {
            int target_square = new_rank * 8 + new_file;
            std::string target_algebraic = square_to_algebraic_local(target_square);
            std::cout << "  Move " << i << ": e5 -> " << target_algebraic
                      << " (square " << target_square << ")\n";
        } else {
            std::cout << "  Move " << i << ": e5 -> INVALID (" << new_file << "," << new_rank << ")\n";
        }
    }

    return 0;
}