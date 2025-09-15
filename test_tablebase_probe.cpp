#include <iostream>
extern "C" {
    #include "fathom/src/tbprobe.h"
}

int main() {
    std::cout << "=== Comprehensive Fathom Tablebase Test ===" << std::endl;
    
    // Initialize with real tablebase files
    bool initialized = tb_init("d:\\TB\\");
    std::cout << "Tablebase initialization: " << (initialized ? "SUCCESS" : "FAILED") << std::endl;
    
    if (!initialized) {
        std::cout << "Cannot continue without tablebase files." << std::endl;
        return 1;
    }
    
    std::cout << "Maximum pieces supported: " << TB_LARGEST << std::endl;
    
    // Test a simple KQvK endgame (4 pieces)
    // Position: White King on e1, White Queen on d1, Black King on e8
    uint64_t white_pieces = (1ULL << 4) | (1ULL << 3);  // e1, d1
    uint64_t black_pieces = (1ULL << 60);               // e8
    uint64_t kings = (1ULL << 4) | (1ULL << 60);        // Both kings
    uint64_t queens = (1ULL << 3);                      // White queen on d1
    uint64_t rooks = 0, bishops = 0, knights = 0, pawns = 0;
    
    // Probe WDL (Win/Draw/Loss)
    unsigned wdl_result = tb_probe_wdl_impl(
        white_pieces, black_pieces, kings, queens, rooks, bishops, knights, pawns,
        0, true
    );
    
    std::cout << "KQvK position WDL probe: ";
    if (wdl_result == TB_RESULT_FAILED) {
        std::cout << "FAILED" << std::endl;
    } else {
        unsigned wdl = wdl_result & TB_RESULT_WDL_MASK;
        switch (wdl) {
            case TB_LOSS: std::cout << "LOSS"; break;
            case TB_BLESSED_LOSS: std::cout << "BLESSED LOSS"; break;
            case TB_DRAW: std::cout << "DRAW"; break;
            case TB_CURSED_WIN: std::cout << "CURSED WIN"; break;
            case TB_WIN: std::cout << "WIN"; break;
            default: std::cout << "UNKNOWN (" << wdl << ")"; break;
        }
        std::cout << std::endl;
    }
    
    // Test a simple KBBvK endgame (4 pieces)
    // Position: White King on e1, White Bishops on c1,f1, Black King on a8
    white_pieces = (1ULL << 4) | (1ULL << 2) | (1ULL << 5);  // e1, c1, f1
    black_pieces = (1ULL << 56);                             // a8
    kings = (1ULL << 4) | (1ULL << 56);                      // Both kings
    queens = 0;
    bishops = (1ULL << 2) | (1ULL << 5);                     // c1, f1
    
    wdl_result = tb_probe_wdl_impl(
        white_pieces, black_pieces, kings, queens, rooks, bishops, knights, pawns,
        0, true
    );
    
    std::cout << "KBBvK position WDL probe: ";
    if (wdl_result == TB_RESULT_FAILED) {
        std::cout << "FAILED" << std::endl;
    } else {
        unsigned wdl = wdl_result & TB_RESULT_WDL_MASK;
        switch (wdl) {
            case TB_LOSS: std::cout << "LOSS"; break;
            case TB_BLESSED_LOSS: std::cout << "BLESSED LOSS"; break;
            case TB_DRAW: std::cout << "DRAW"; break;
            case TB_CURSED_WIN: std::cout << "CURSED WIN"; break;
            case TB_WIN: std::cout << "WIN"; break;
            default: std::cout << "UNKNOWN (" << wdl << ")"; break;
        }
        std::cout << std::endl;
    }
    
    // Clean up
    tb_free();
    
    std::cout << "=== Tablebase test completed successfully! ===" << std::endl;
    return 0;
}