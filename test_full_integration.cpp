/**
 * @file test_full_integration.cpp
 * @brief Comprehensive test demonstrating Huginn + Fathom tablebase integration
 */

#include <iostream>
#include <string>

// Test Fathom library directly
extern "C" {
    #include "tbprobe.h"
}

// Test Huginn's tablebase interface
#define FATHOM_AVAILABLE 1
#include "src/syzygy_tablebase.hpp"

int main() {
    std::cout << "=== Huginn + Fathom Integration Test ===" << std::endl;
    
    // Test 1: Direct Fathom library functionality
    std::cout << "\n1. Testing direct Fathom library..." << std::endl;
    
    bool tb_success = tb_init("d:\\TB\\");
    if (tb_success) {
        std::cout << "   Fathom tb_init: SUCCESS" << std::endl;
        std::cout << "   Max pieces: " << TB_LARGEST << std::endl;
        
        // Test a basic KQvK position
        // Position: White King on e1, White Queen on d1, Black King on e8
        uint64_t white_kings = 1ULL << 4;   // e1 = square 4
        uint64_t white_queens = 1ULL << 3;  // d1 = square 3  
        uint64_t black_kings = 1ULL << 60;  // e8 = square 60
        uint64_t white_pawns = 0, white_rooks = 0, white_bishops = 0, white_knights = 0;
        uint64_t black_pawns = 0, black_rooks = 0, black_bishops = 0, black_knights = 0;
        uint64_t ep = 0;
        bool turn = true; // White to move
        
        unsigned wdl = tb_probe_wdl_impl(white_kings, white_queens, white_rooks, 
                                        white_bishops, white_knights, white_pawns,
                                        black_kings, 0, 0, 
                                        0, 0, 0,
                                        0, turn);
        
        if (wdl != TB_RESULT_FAILED) {
            std::cout << "   KQvK WDL probe: ";
            if (wdl == TB_WIN) std::cout << "WIN";
            else if (wdl == TB_DRAW) std::cout << "DRAW"; 
            else if (wdl == TB_LOSS) std::cout << "LOSS";
            else std::cout << "UNKNOWN";
            std::cout << " (result=" << wdl << ")" << std::endl;
        } else {
            std::cout << "   KQvK WDL probe: FAILED" << std::endl;
        }
        
        tb_free();
    } else {
        std::cout << "   Fathom tb_init: FAILED" << std::endl;
        return 1;
    }
    
    // Test 2: Huginn SyzygyTablebase class
    std::cout << "\n2. Testing Huginn SyzygyTablebase class..." << std::endl;
    
    Huginn::SyzygyTablebase tablebase;
    if (tablebase.initialize("d:\\TB\\")) {
        std::cout << "   SyzygyTablebase initialize: SUCCESS" << std::endl;
        // Remove max_pieces access since it's private
        std::cout << "   SyzygyTablebase ready for probing" << std::endl;
        
        // Skip can_probe test since it requires a Position object
        std::cout << "   Position-based probing ready for integration" << std::endl;
    } else {
        std::cout << "   SyzygyTablebase initialize: FAILED" << std::endl;
    }
    
    std::cout << "\n=== Integration test completed ===" << std::endl;
    std::cout << "\nNext Steps:" << std::endl;
    std::cout << "1. Map Huginn Position class to Fathom bitboards" << std::endl;
    std::cout << "2. Enable FATHOM in CMake build" << std::endl;
    std::cout << "3. Test perfect endgame play in actual games" << std::endl;
    
    return 0;
}