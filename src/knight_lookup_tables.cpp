#include "knight_lookup_tables.hpp"
#include "board120.hpp"

namespace KnightLookupTables {

    // Pre-computed lookup tables (initialized at startup)
    int KNIGHT_MOVES[64][8];
    int KNIGHT_MOVE_COUNT[64];
    uint64_t KNIGHT_ATTACKS[64];
    
    /**
     * Convert 64-square board coordinates to rank/file
     */
    inline int square_to_rank(int sq64) { return sq64 / 8; }
    inline int square_to_file(int sq64) { return sq64 % 8; }
    inline int rank_file_to_square(int rank, int file) { 
        if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
        return rank * 8 + file; 
    }
    
    /**
     * Initialize knight lookup tables
     * Call this once at engine startup
     */
    void initialize_knight_tables() {
        // Knight move offsets in rank/file coordinates
        const int knight_deltas[8][2] = {
            {-2, -1}, {-2, +1}, {-1, -2}, {-1, +2},
            {+1, -2}, {+1, +2}, {+2, -1}, {+2, +1}
        };
        
        // Initialize all squares
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            int rank = square_to_rank(sq64);
            int file = square_to_file(sq64);
            
            int move_count = 0;
            uint64_t attack_bitboard = 0ULL;
            
            // Check all 8 possible knight moves
            for (int i = 0; i < 8; ++i) {
                int new_rank = rank + knight_deltas[i][0];
                int new_file = file + knight_deltas[i][1];
                
                int target_sq = rank_file_to_square(new_rank, new_file);
                
                if (target_sq != -1) { // Valid square
                    KNIGHT_MOVES[sq64][move_count] = target_sq;
                    attack_bitboard |= (1ULL << target_sq);
                    move_count++;
                }
            }
            
            KNIGHT_MOVE_COUNT[sq64] = move_count;
            KNIGHT_ATTACKS[sq64] = attack_bitboard;
            
            // Fill remaining slots with -1 for safety
            for (int i = move_count; i < 8; ++i) {
                KNIGHT_MOVES[sq64][i] = -1;
            }
        }
    }
    
    /**
     * Debug function to print knight lookup tables
     */
    void print_knight_tables() {
        printf("Knight Lookup Tables:\n");
        printf("=====================\n\n");
        
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            int rank = square_to_rank(sq64);
            int file = square_to_file(sq64);
            char square_name[3];
            sprintf(square_name, "%c%d", 'a' + file, rank + 1);
            
            printf("Square %s (%d): %d moves -> ", 
                   square_name, sq64, KNIGHT_MOVE_COUNT[sq64]);
            
            for (int i = 0; i < KNIGHT_MOVE_COUNT[sq64]; ++i) {
                int target = KNIGHT_MOVES[sq64][i];
                int target_rank = square_to_rank(target);
                int target_file = square_to_file(target);
                printf("%c%d ", 'a' + target_file, target_rank + 1);
            }
            printf("\n");
            
            if ((sq64 + 1) % 8 == 0) printf("\n"); // Line break every rank
        }
        
        printf("Bitboard verification:\n");
        printf("======================\n");
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            if (KNIGHT_MOVE_COUNT[sq64] != builtin_popcountll(KNIGHT_ATTACKS[sq64])) {
                printf("ERROR: Mismatch at square %d\n", sq64);
            }
        }
        printf("Lookup table initialization complete.\n\n");
    }

} // namespace KnightLookupTables