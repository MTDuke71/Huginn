/**
 * @file test_special_moves_simple.cpp
 * @brief Simplified tests for special chess moves in UltraPosition
 * 
 * This test suite validates castling, promotion, and en passant generation
 * using the actual UltraEngine interfaces.
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#include <iostream>
#include <cassert>
#include <iomanip>
#include <chrono>
#include "ultra_engine/ultra_position.hpp"
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

// Helper function to check if a specific move exists in move list
bool has_move(const UltraMoveList& moves, int from, int to, 
              bool check_promotion = false, int promo_piece = 0,
              bool check_castle = false, bool check_ep = false) {
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (move.from() == from && move.to() == to) {
            if (check_promotion) {
                return move.is_promotion() && move.promoted() == promo_piece;
            } else if (check_castle) {
                return move.is_castle();
            } else if (check_ep) {
                return move.is_ep();
            } else {
                return true; // Just check from/to match
            }
        }
    }
    return false;
}

void test_castling_basic() {
    std::cout << "\n=== BASIC CASTLING TESTS ===\n";
    
    UltraPosition pos;
    
    // Test: White can castle both sides
    std::cout << "Testing white castling from starting position with clear path...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Check for castling moves by examining move flags
    bool found_kingside = false;
    bool found_queenside = false;
    
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (move.is_castle()) {
            if (move.from() == 4 && move.to() == 6) { // e1-g1
                found_kingside = true;
            } else if (move.from() == 4 && move.to() == 2) { // e1-c1
                found_queenside = true;
            }
        }
    }
    
    assert(found_kingside);
    std::cout << "✓ White kingside castling found\n";
    
    assert(found_queenside);
    std::cout << "✓ White queenside castling found\n";
    
    std::cout << "✓ Basic castling tests passed\n";
}

void test_promotion_basic() {
    std::cout << "\n=== BASIC PROMOTION TESTS ===\n";
    
    UltraPosition pos;
    
    // Test: White pawn promotion on 8th rank
    std::cout << "Testing white pawn promotion...\n";
    pos.set_fen("8/P7/8/8/8/8/8/8 w - - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Should have 4 promotion moves: Queen, Rook, Bishop, Knight
    int promotion_count = 0;
    bool found_queen = false, found_rook = false, found_bishop = false, found_knight = false;
    
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (move.is_promotion() && move.from() == 48 && move.to() == 56) { // a7-a8
            promotion_count++;
            switch (move.promoted()) {
                case UltraPosition::Queen:  found_queen = true; break;
                case UltraPosition::Rook:   found_rook = true; break;
                case UltraPosition::Bishop: found_bishop = true; break;
                case UltraPosition::Knight: found_knight = true; break;
            }
        }
    }
    
    assert(found_queen && found_rook && found_bishop && found_knight);
    assert(promotion_count == 4);
    std::cout << "✓ White pawn promotes to all 4 pieces (found " << promotion_count << " promotion moves)\n";
    
    std::cout << "✓ Basic promotion tests passed\n";
}

void test_en_passant_basic() {
    std::cout << "\n=== BASIC EN PASSANT TESTS ===\n";
    
    UltraPosition pos;
    
    // Test: White en passant capture
    std::cout << "Testing white en passant capture...\n";
    // Black just played e7-e5, white pawn on d5 can capture en passant
    pos.set_fen("rnbqkbnr/pppp1ppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Find en passant move
    bool found_en_passant = false;
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (move.is_ep() && move.from() == 28 && move.to() == 37) { // e5xe6
            found_en_passant = true;
            break;
        }
    }
    
    assert(found_en_passant);
    std::cout << "✓ White en passant capture found (e5xe6)\n";
    
    std::cout << "✓ Basic en passant tests passed\n";
}

void test_move_generation_comprehensive() {
    std::cout << "\n=== COMPREHENSIVE MOVE GENERATION TESTS ===\n";
    
    UltraPosition pos;
    
    // Test complex position with multiple special moves
    std::cout << "Testing complex position with castling, promotion, and captures...\n";
    pos.set_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    std::cout << "Generated " << moves.size() << " moves in complex position\n";
    
    // Count different types of moves
    int captures = 0, promotions = 0, quiet_moves = 0, castles = 0, en_passant = 0;
    
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (move.is_capture()) captures++;
        if (move.is_promotion()) promotions++;
        if (move.is_castle()) castles++;
        if (move.is_ep()) en_passant++;
        if (move.is_quiet()) quiet_moves++;
    }
    
    std::cout << "  Captures: " << captures << "\n";
    std::cout << "  Promotions: " << promotions << "\n";
    std::cout << "  Castles: " << castles << "\n";
    std::cout << "  En passant: " << en_passant << "\n";
    std::cout << "  Quiet moves: " << quiet_moves << "\n";
    
    assert(moves.size() > 0);
    std::cout << "✓ Complex position generates valid moves\n";
    
    std::cout << "✓ Comprehensive move generation tests passed\n";
}

void test_performance() {
    std::cout << "\n=== PERFORMANCE TESTS ===\n";
    
    UltraPosition pos;
    
    // Test move generation performance on starting position
    std::cout << "Testing move generation performance...\n";
    pos.setup_starting_position();
    
    const int iterations = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        UltraMoveList moves;
        pos.generate_all_moves(moves);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    const double ops_per_sec = (iterations * 1e9) / duration.count();
    std::cout << "Move generation: " << std::fixed << std::setprecision(2) 
              << ops_per_sec / 1e6 << " M ops/sec\n";
    
    // Test move counting performance  
    std::cout << "Testing move counting performance...\n";
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        int count = pos.count_all_moves();
        (void)count; // Prevent optimization
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    const double count_ops_per_sec = (iterations * 1e9) / duration.count();
    std::cout << "Move counting: " << std::fixed << std::setprecision(2) 
              << count_ops_per_sec / 1e6 << " M ops/sec\n";
    
    std::cout << "✓ Performance tests completed\n";
}

void test_special_positions() {
    std::cout << "\n=== SPECIAL POSITION TESTS ===\n";
    
    UltraPosition pos;
    
    // Test known perft positions
    std::cout << "Testing known perft positions...\n";
    
    // Position 2 from perft test suite
    pos.set_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    std::cout << "Position 2: " << moves.size() << " moves\n";
    
    // Position 3 from perft test suite  
    pos.set_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    moves.clear();
    pos.generate_all_moves(moves);
    std::cout << "Position 3: " << moves.size() << " moves\n";
    
    // Position 4 - en passant and castling
    pos.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    moves.clear();
    pos.generate_all_moves(moves);
    std::cout << "Position 4: " << moves.size() << " moves\n";
    
    std::cout << "✓ Special position tests completed\n";
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "      ULTRA-ENGINE SPECIAL MOVES TEST SUITE          \n";
    std::cout << "=======================================================\n";
    
    try {
        // Initialize attacks tables
        UltraAttacks::initialize();
        
        // Run test suites
        test_castling_basic();
        test_promotion_basic();
        test_en_passant_basic();
        test_move_generation_comprehensive();
        test_performance();
        test_special_positions();
        
        std::cout << "\n=======================================================\n";
        std::cout << "              ALL TESTS PASSED! ✓                     \n";
        std::cout << "=======================================================\n";
        std::cout << "\nUltra-engine special move handling is working correctly!\n";
        std::cout << "Ready for complete perft integration.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ TEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n❌ UNKNOWN TEST FAILURE" << std::endl;
        return 1;
    }
    
    return 0;
}