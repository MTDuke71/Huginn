/**
 * @file test_special_moves.cpp
 * @brief Comprehensive tests for special chess moves: castling, promotion, en passant
 * 
 * This test suite validates the UltraPosition and move generation systems
 * for all special chess rules and edge cases.
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#include <iostream>
#include <cassert>
#include <iomanip>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include "ultra_engine/ultra_position.hpp"
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

// Helper function to print move lists for debugging
void print_moves(const UltraMoveList& moves, const std::string& title) {
    std::cout << "\n" << title << " (" << moves.size() << " moves):\n";
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        const int from = move.from();
        const int to = move.to();
        const char from_file = 'a' + (from % 8);
        const char from_rank = '1' + (from / 8);
        const char to_file = 'a' + (to % 8);
        const char to_rank = '1' + (to / 8);
        
        std::cout << "  " << from_file << from_rank << to_file << to_rank;
        
        if (move.is_promotion()) {
            const char pieces[] = "?PNBRQK";
            std::cout << "=" << pieces[move.promoted()];
        }
        if (move.is_capture()) std::cout << " (capture)";
        if (move.is_castle()) std::cout << " (castle)";
        if (move.is_ep()) std::cout << " (e.p.)";
        
        std::cout << "\n";
    }
}

// Helper function to check if a specific move exists in move list
bool has_move(const UltraMoveList& moves, int from, int to, 
              bool is_promotion = false, int promo_piece = 0) {
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (move.from() == from && move.to() == to) {
            if (is_promotion) {
                return move.is_promotion() && move.promoted() == promo_piece;
            } else {
                return !move.is_promotion();
            }
        }
    }
    return false;
}

// ============================================================================
// CASTLING TESTS
// ============================================================================

void test_castling_basic() {
    std::cout << "\n=== BASIC CASTLING TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: White can castle both sides
    std::cout << "\nTest 1: White castling from starting position with clear path...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Should have kingside castling (e1-g1)
    assert(has_move(moves, 4, 6)); // e1-g1
    std::cout << "✓ White kingside castling available (e1-g1)\n";
    
    // Should have queenside castling (e1-c1)
    assert(has_move(moves, 4, 2)); // e1-c1
    std::cout << "✓ White queenside castling available (e1-c1)\n";
    
    // Test 2: Black can castle both sides
    std::cout << "\nTest 2: Black castling...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");
    
    moves.clear();
    pos.generate_all_moves(moves);
    
    // Should have kingside castling (e8-g8)
    assert(has_move(moves, 60, 62)); // e8-g8
    std::cout << "✓ Black kingside castling available (e8-g8)\n";
    
    // Should have queenside castling (e8-c8)
    assert(has_move(moves, 60, 58)); // e8-c8
    std::cout << "✓ Black queenside castling available (e8-c8)\n";
    
    std::cout << "✓ Basic castling tests passed\n";
}

void test_castling_blocked() {
    std::cout << "\n=== CASTLING BLOCKED TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: Pieces blocking castling
    std::cout << "\nTest 1: Pieces blocking castling paths...\n";
    
    // Knight blocks kingside
    pos.set_fen("r3k1nr/pppppppp/8/8/8/8/PPPPPPPP/R3K1NR w KQkq - 0 1");
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    assert(!has_move(moves, 4, 6)); // e1-g1 should not be available
    std::cout << "✓ Kingside castling blocked by knight\n";
    
    // Bishop blocks queenside  
    pos.set_fen("r1b1k2r/pppppppp/8/8/8/8/PPPPPPPP/R1B1K2R w KQkq - 0 1");
    moves.clear();
    pos.generate_all_moves(moves);
    
    assert(!has_move(moves, 4, 2)); // e1-c1 should not be available
    std::cout << "✓ Queenside castling blocked by bishop\n";
    
    std::cout << "✓ Castling blocked tests passed\n";
}

void test_castling_under_attack() {
    std::cout << "\n=== CASTLING UNDER ATTACK TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: King in check (can't castle)
    std::cout << "\nTest 1: King in check...\n";
    pos.set_fen("r3k2r/pppp1ppp/8/4r3/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    assert(!has_move(moves, 4, 6)); // No kingside castling
    assert(!has_move(moves, 4, 2)); // No queenside castling
    std::cout << "✓ Cannot castle when king is in check\n";
    
    // Test 2: King passes through attacked square
    std::cout << "\nTest 2: King passes through attacked square...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/5r2/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    moves.clear();
    pos.generate_all_moves(moves);
    
    assert(!has_move(moves, 4, 6)); // f1 is attacked, no kingside castling
    std::cout << "✓ Cannot castle through attacked square\n";
    
    // Test 3: King ends on attacked square
    std::cout << "\nTest 3: King would end on attacked square...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/6r1/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    moves.clear();
    pos.generate_all_moves(moves);
    
    assert(!has_move(moves, 4, 6)); // g1 is attacked, no kingside castling
    std::cout << "✓ Cannot castle to attacked square\n";
    
    std::cout << "✓ Castling under attack tests passed\n";
}

void test_castling_rights_lost() {
    std::cout << "\n=== CASTLING RIGHTS LOST TESTS ===\n";
    
    UltraPosition pos;
    
    // Test: No castling rights
    std::cout << "\nTesting positions with no castling rights...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w - - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    assert(!has_move(moves, 4, 6)); // No kingside castling
    assert(!has_move(moves, 4, 2)); // No queenside castling
    std::cout << "✓ No castling when rights are lost\n";
    
    // Test: Partial castling rights
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Kq - 0 1");
    moves.clear();
    pos.generate_all_moves(moves);
    
    assert(has_move(moves, 4, 6));  // White kingside available
    assert(!has_move(moves, 4, 2)); // White queenside not available
    std::cout << "✓ Partial castling rights work correctly\n";
    
    std::cout << "✓ Castling rights tests passed\n";
}

// ============================================================================
// PROMOTION TESTS
// ============================================================================

void test_promotion_basic() {
    std::cout << "\n=== BASIC PROMOTION TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: White pawn promotion on 8th rank
    std::cout << "\nTest 1: White pawn promotion...\n";
    pos.set_fen("8/P7/8/8/8/8/8/8 w - - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Should have 4 promotion moves: Queen, Rook, Bishop, Knight
    assert(has_move(moves, 48, 56, true, UltraPosition::Queen));  // a7-a8=Q
    assert(has_move(moves, 48, 56, true, UltraPosition::Rook));   // a7-a8=R
    assert(has_move(moves, 48, 56, true, UltraPosition::Bishop)); // a7-a8=B
    assert(has_move(moves, 48, 56, true, UltraPosition::Knight)); // a7-a8=N
    
    std::cout << "✓ White pawn promotes to all 4 pieces\n";
    
    // Test 2: Black pawn promotion on 1st rank
    std::cout << "\nTest 2: Black pawn promotion...\n";
    pos.set_fen("8/8/8/8/8/8/p7/8 b - - 0 1");
    
    moves.clear();
    pos.generate_all_moves(moves);
    
    // Should have 4 promotion moves
    assert(has_move(moves, 8, 0, true, UltraPosition::Queen));   // a2-a1=Q
    assert(has_move(moves, 8, 0, true, UltraPosition::Rook));    // a2-a1=R
    assert(has_move(moves, 8, 0, true, UltraPosition::Bishop));  // a2-a1=B
    assert(has_move(moves, 8, 0, true, UltraPosition::Knight));  // a2-a1=N
    
    std::cout << "✓ Black pawn promotes to all 4 pieces\n";
    
    std::cout << "✓ Basic promotion tests passed\n";
}

void test_promotion_captures() {
    std::cout << "\n=== PROMOTION CAPTURE TESTS ===\n";
    
    UltraPosition pos;
    
    // Test: White pawn captures and promotes
    std::cout << "\nTesting promotion captures...\n";
    pos.set_fen("1r6/P7/8/8/8/8/8/8 w - - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Should have promotion moves for capture
    assert(has_move(moves, 48, 57, true, UltraPosition::Queen));  // a7xb8=Q
    assert(has_move(moves, 48, 57, true, UltraPosition::Rook));   // a7xb8=R
    assert(has_move(moves, 48, 57, true, UltraPosition::Bishop)); // a7xb8=B
    assert(has_move(moves, 48, 57, true, UltraPosition::Knight)); // a7xb8=N
    
    // Should also have straight promotion moves
    assert(has_move(moves, 48, 56, true, UltraPosition::Queen));  // a7-a8=Q
    
    std::cout << "✓ Promotion captures work correctly\n";
    
    std::cout << "✓ Promotion capture tests passed\n";
}

void test_promotion_blocked() {
    std::cout << "\n=== PROMOTION BLOCKED TESTS ===\n";
    
    UltraPosition pos;
    
    // Test: Pawn blocked from promotion
    std::cout << "\nTesting blocked promotion...\n";
    pos.set_fen("r7/P7/8/8/8/8/8/8 w - - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Should not have straight promotion (blocked by rook)
    assert(!has_move(moves, 48, 56, true, UltraPosition::Queen)); // a7-a8 blocked
    
    // But should still have no legal moves since no captures available
    bool has_promotion_move = false;
    for (int i = 0; i < moves.size(); i++) {
        if (moves.get_move(i).is_promotion()) {
            has_promotion_move = true;
            break;
        }
    }
    assert(!has_promotion_move);
    
    std::cout << "✓ Blocked promotion correctly prevented\n";
    
    std::cout << "✓ Promotion blocked tests passed\n";
}

// ============================================================================
// EN PASSANT TESTS
// ============================================================================

void test_en_passant_basic() {
    std::cout << "\n=== BASIC EN PASSANT TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: White en passant capture
    std::cout << "\nTest 1: White en passant capture...\n";
    // Black just played e7-e5, white pawn on d5 can capture en passant
    pos.set_fen("rnbqkbnr/pppp1ppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Find en passant move
    bool found_en_passant = false;
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves.get_move(i);
        if (move.is_en_passant() && move.get_from() == 28 && move.get_to() == 37) { // e5xe6
            found_en_passant = true;
            break;
        }
    }
    assert(found_en_passant);
    std::cout << "✓ White en passant capture found (e5xe6)\n";
    
    // Test 2: Black en passant capture  
    std::cout << "\nTest 2: Black en passant capture...\n";
    // White just played e2-e4, black pawn on d4 can capture en passant
    pos.set_fen("rnbqkbnr/pppp1ppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    moves.clear();
    pos.generate_all_moves(moves);
    
    // Find en passant move
    found_en_passant = false;
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves.get_move(i);
        if (move.is_en_passant() && move.get_from() == 27 && move.get_to() == 20) { // d4xe3
            found_en_passant = true;
            break;
        }
    }
    assert(found_en_passant);
    std::cout << "✓ Black en passant capture found (d4xe3)\n";
    
    std::cout << "✓ Basic en passant tests passed\n";
}

void test_en_passant_edge_cases() {
    std::cout << "\n=== EN PASSANT EDGE CASES ===\n";
    
    UltraPosition pos;
    
    // Test 1: No en passant when not available
    std::cout << "\nTest 1: No en passant when not set...\n";
    pos.set_fen("rnbqkbnr/pppp1ppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Should not have en passant move
    bool found_en_passant = false;
    for (int i = 0; i < moves.size(); i++) {
        if (moves.get_move(i).is_en_passant()) {
            found_en_passant = true;
            break;
        }
    }
    assert(!found_en_passant);
    std::cout << "✓ No en passant when not available\n";
    
    // Test 2: Multiple pawns can capture en passant
    std::cout << "\nTest 2: Multiple pawns can capture en passant...\n";
    pos.set_fen("rnbqkbnr/ppp1pppp/8/2PpP3/8/8/PP1P1PPP/RNBQKBNR w KQkq d6 0 1");
    
    moves.clear();
    pos.generate_all_moves(moves);
    
    // Both c5 and e5 pawns should be able to capture on d6
    int en_passant_count = 0;
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves.get_move(i);
        if (move.is_en_passant() && move.get_to() == 43) { // d6
            en_passant_count++;
        }
    }
    assert(en_passant_count == 2);
    std::cout << "✓ Multiple pawns can capture en passant\n";
    
    std::cout << "✓ En passant edge case tests passed\n";
}

// ============================================================================
// MOVE EXECUTION TESTS
// ============================================================================

void test_move_execution() {
    std::cout << "\n=== MOVE EXECUTION TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: Execute castling move
    std::cout << "\nTest 1: Execute castling move...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    // Create kingside castling move
    UltraMove castle_move = UltraMove::create_castling(4, 6); // e1-g1
    
    // Execute move
    UltraPosition::GameState state;
    pos.make_move(castle_move, state);
    
    // Verify king and rook positions
    assert(pos.get_piece_type_at(6) == UltraPosition::King);   // King on g1
    assert(pos.get_piece_color_at(6) == UltraPosition::White);
    assert(pos.get_piece_type_at(5) == UltraPosition::Rook);   // Rook on f1
    assert(pos.get_piece_color_at(5) == UltraPosition::White);
    assert(pos.get_piece_at(4) == -1); // e1 empty
    assert(pos.get_piece_at(7) == -1); // h1 empty
    
    std::cout << "✓ Castling move executed correctly\n";
    
    // Test 2: Execute promotion move
    std::cout << "\nTest 2: Execute promotion move...\n";
    pos.set_fen("8/P7/8/8/8/8/8/8 w - - 0 1");
    
    // Create promotion move
    UltraMove promo_move = UltraMove::create_promotion(48, 56, UltraPosition::Queen);
    
    pos.make_move(promo_move, state);
    
    // Verify queen on 8th rank
    assert(pos.get_piece_type_at(56) == UltraPosition::Queen);
    assert(pos.get_piece_color_at(56) == UltraPosition::White);
    assert(pos.get_piece_at(48) == -1); // Original square empty
    
    std::cout << "✓ Promotion move executed correctly\n";
    
    // Test 3: Execute en passant move
    std::cout << "\nTest 3: Execute en passant move...\n";
    pos.set_fen("rnbqkbnr/pppp1ppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1");
    
    // Create en passant move
    UltraMove ep_move = UltraMove::create_en_passant(28, 37); // e5xe6
    
    pos.make_move(ep_move, state);
    
    // Verify captured pawn removed and attacking pawn moved
    assert(pos.get_piece_type_at(37) == UltraPosition::Pawn); // White pawn on e6
    assert(pos.get_piece_color_at(37) == UltraPosition::White);
    assert(pos.get_piece_at(28) == -1); // e5 empty
    assert(pos.get_piece_at(29) == -1); // e5 (captured black pawn) empty
    
    std::cout << "✓ En passant move executed correctly\n";
    
    std::cout << "✓ Move execution tests passed\n";
}

// ============================================================================
// MOVE UNDO TESTS
// ============================================================================

void test_move_undo() {
    std::cout << "\n=== MOVE UNDO TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: Undo castling
    std::cout << "\nTest 1: Undo castling move...\n";
    const std::string original_fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1";
    pos.set_fen(original_fen);
    
    UltraMove castle_move = UltraMove::create_castling(4, 6);
    UltraPosition::GameState state;
    
    pos.make_move(castle_move, state);
    pos.unmake_move(castle_move, state);
    
    assert(pos.get_fen() == original_fen);
    std::cout << "✓ Castling move undone correctly\n";
    
    // Test 2: Undo promotion
    std::cout << "\nTest 2: Undo promotion move...\n";
    const std::string promo_fen = "8/P7/8/8/8/8/8/8 w - - 0 1";
    pos.set_fen(promo_fen);
    
    UltraMove promo_move = UltraMove::create_promotion(48, 56, UltraPosition::Queen);
    pos.make_move(promo_move, state);
    pos.unmake_move(promo_move, state);
    
    assert(pos.get_fen() == promo_fen);
    std::cout << "✓ Promotion move undone correctly\n";
    
    // Test 3: Undo en passant
    std::cout << "\nTest 3: Undo en passant move...\n";
    const std::string ep_fen = "rnbqkbnr/pppp1ppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 1";
    pos.set_fen(ep_fen);
    
    UltraMove ep_move = UltraMove::create_en_passant(28, 37);
    pos.make_move(ep_move, state);
    pos.unmake_move(ep_move, state);
    
    assert(pos.get_fen() == ep_fen);
    std::cout << "✓ En passant move undone correctly\n";
    
    std::cout << "✓ Move undo tests passed\n";
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_special_move_performance() {
    std::cout << "\n=== SPECIAL MOVE PERFORMANCE TESTS ===\n";
    
    UltraPosition pos;
    
    // Test castling performance
    std::cout << "\nTesting castling performance...\n";
    pos.set_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    const int iterations = 1000000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        UltraMoveList moves;
        pos.generate_all_moves(moves);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    const double ops_per_sec = (iterations * 1e9) / duration.count();
    std::cout << "Castling position: " << std::fixed << std::setprecision(2) 
              << ops_per_sec / 1e6 << " M ops/sec\n";
    
    // Test promotion performance
    std::cout << "\nTesting promotion performance...\n";
    pos.set_fen("8/PPPPPPPP/8/8/8/8/pppppppp/8 w - - 0 1");
    
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        UltraMoveList moves;
        pos.generate_all_moves(moves);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    const double promo_ops_per_sec = (iterations * 1e9) / duration.count();
    std::cout << "Promotion position: " << std::fixed << std::setprecision(2) 
              << promo_ops_per_sec / 1e6 << " M ops/sec\n";
    
    std::cout << "✓ Performance tests completed\n";
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    std::cout << "=======================================================\n";
    std::cout << "       ULTRA-ENGINE SPECIAL MOVES TEST SUITE         \n";
    std::cout << "=======================================================\n";
    
    try {
        // Initialize attacks tables
        UltraAttacks::init();
        
        // Run all test suites
        test_castling_basic();
        test_castling_blocked();
        test_castling_under_attack();
        test_castling_rights_lost();
        
        test_promotion_basic();
        test_promotion_captures();
        test_promotion_blocked();
        
        test_en_passant_basic();
        test_en_passant_edge_cases();
        
        test_move_execution();
        test_move_undo();
        
        test_special_move_performance();
        
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