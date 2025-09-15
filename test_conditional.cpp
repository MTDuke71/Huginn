#include <iostream>
#include <cstdint>

// Test our conditional compilation setup
#define ENABLE_FATHOM 1

#ifdef ENABLE_FATHOM
#define FATHOM_AVAILABLE 1
// Mock some Fathom constants for testing
#define TB_WPAWN 1
#define TB_BPAWN 2
#define TB_WKNIGHT 3
#define TB_BKNIGHT 4
#define TB_WBISHOP 5
#define TB_BBISHOP 6
#define TB_WROOK 7
#define TB_BROOK 8
#define TB_WQUEEN 9
#define TB_BQUEEN 10
#define TB_WKING 11
#define TB_BKING 12
#define TB_NONE 0
#define TB_NOSQUARE 64
#define TB_RESULT_FAILED 0xFFFFFFFF
#define TB_LOSS 0
#define TB_BLESSED_LOSS 1
#define TB_DRAW 2
#define TB_CURSED_WIN 3
#define TB_WIN 4
#else
#define FATHOM_AVAILABLE 0
#endif

// Test piece values from Huginn
enum { wP = 1, bP = 7, wN = 2, bN = 8, wB = 3, bB = 9, wR = 4, bR = 10, wQ = 5, bQ = 11, wK = 6, bK = 12, EMPTY = 0, OFFBOARD = 100 };

unsigned fathom_piece_from_huginn(int huginn_piece) {
    switch (huginn_piece) {
        case wP: return TB_WPAWN;
        case bP: return TB_BPAWN;
        case wN: return TB_WKNIGHT;
        case bN: return TB_BKNIGHT;
        case wB: return TB_WBISHOP;
        case bB: return TB_BBISHOP;
        case wR: return TB_WROOK;
        case bR: return TB_BROOK;
        case wQ: return TB_WQUEEN;
        case bQ: return TB_BQUEEN;
        case wK: return TB_WKING;
        case bK: return TB_BKING;
        default: return TB_NONE;
    }
}

int main() {
    std::cout << "Testing conditional compilation..." << std::endl;
    std::cout << "FATHOM_AVAILABLE: " << FATHOM_AVAILABLE << std::endl;
    
    #if FATHOM_AVAILABLE
    std::cout << "Fathom tablebase support is ENABLED" << std::endl;
    std::cout << "White pawn maps to: " << fathom_piece_from_huginn(wP) << std::endl;
    std::cout << "Black queen maps to: " << fathom_piece_from_huginn(bQ) << std::endl;
    #else
    std::cout << "Fathom tablebase support is DISABLED" << std::endl;
    #endif
    
    return 0;
}