#include <iostream>
#include <chrono>
#include <iomanip>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "chess_types.hpp"

/**
 * @brief Performance test (perft) for huginn2 bitboard move generation
 * 
 * Perft is a debugging function to walk the move generation tree of strictly
 * legal moves to count all the leaf nodes of a certain depth. It's essential
 * for testing move generation correctness.
 */

// Helper function to convert square index to algebraic notation
std::string square_to_algebraic(int square120) {
    // Convert from 120-square to file/rank
    int file = (square120 % 10) - 1;
    int rank = (square120 / 10) - 2;
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return "??"; // Invalid square
    }
    
    return std::string(1, 'a' + file) + std::to_string(rank + 1);
}

// Helper function to convert move to string
std::string move_to_string(const S_MOVE& move) {
    if (move.move == 0) return "null";
    
    std::string from_str = square_to_algebraic(move.get_from());
    std::string to_str = square_to_algebraic(move.get_to());
    
    std::string result = from_str + to_str;
    
    // Add promotion piece if applicable
    PieceType promoted = move.get_promoted();
    if (promoted != PieceType::None) {
        switch (promoted) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    
    return result;
}

uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    S_MOVELIST move_list;
    
    // Generate all legal moves using bitboard implementation
    generate_legal_moves_enhanced(pos, move_list);
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        
        // Make the move
        if (pos.MakeMove(move) == 1) {
            nodes += perft(pos, depth - 1);
            pos.TakeMove(); // Unmake the move
        }
    }
    
    return nodes;
}

uint64_t perft_divide(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    S_MOVELIST move_list;
    
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "\nDivide analysis for depth " << depth << ":\n";
    std::cout << "Move        Nodes\n";
    std::cout << "----        -----\n";
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        
        if (pos.MakeMove(move) == 1) {
            uint64_t move_nodes = perft(pos, depth - 1);
            nodes += move_nodes;
            
            // Convert move to string representation
            std::string move_str = move_to_string(move);
            std::cout << std::left << std::setw(12) << move_str 
                      << std::right << std::setw(8) << move_nodes << "\n";
            
            pos.TakeMove();
        }
    }
    
    std::cout << "\nTotal: " << nodes << " nodes\n";
    return nodes;
}

void run_perft_suite() {
    Position pos;
    
    std::cout << "=== Huginn2 Bitboard Perft Test Suite ===\n\n";
    
    // Test 1: Starting position
    std::cout << "Test 1: Starting Position\n";
    std::cout << "FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n";
    pos.set_startpos();
    
    // Debug: Check if knights are in bitboards
    uint64_t white_knights = pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)];
    uint64_t black_knights = pos.piece_bitboards[int(Color::Black)][int(PieceType::Knight)];
    std::cout << "Debug: White knights bitboard: 0x" << std::hex << white_knights << std::dec << "\n";
    std::cout << "Debug: Black knights bitboard: 0x" << std::hex << black_knights << std::dec << "\n";
    
    // Expected: White knights on b1(1) and g1(6) = bits 1 and 6 = 0x42
    // Expected: Black knights on b8(57) and g8(62) = bits 57 and 62 = 0x4200000000000000
    
    if (white_knights != 0x42) {
        std::cout << "❌ White knights bitboard incorrect! Expected: 0x42\n";
    }
    if (black_knights != 0x4200000000000000ULL) {
        std::cout << "❌ Black knights bitboard incorrect! Expected: 0x4200000000000000\n";
    }
    
    // Known perft values for starting position
    struct {
        int depth;
        uint64_t expected;
    } starting_tests[] = {
        {1, 20},
        {2, 400},
        {3, 8902},
        {4, 197281},
        {5, 4865609}  // This might take a while
    };
    
    for (auto& test : starting_tests) {
        if (test.depth <= 4) {  // Skip depth 5 for now unless user wants it
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t result = perft(pos, test.depth);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "Depth " << test.depth << ": " << result << " nodes";
            std::cout << " (expected: " << test.expected << ")";
            std::cout << " [" << duration.count() << "ms]";
            
            if (result == test.expected) {
                std::cout << " ✅ PASS\n";
            } else {
                std::cout << " ❌ FAIL\n";
            }
        }
    }
    
    // Test 2: Position with captures and en passant
    std::cout << "\nTest 2: Kiwipete Position (Complex middlegame)\n";
    std::string kiwipete = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::cout << "FEN: " << kiwipete << "\n";
    
    if (pos.set_from_fen(kiwipete)) {
        struct {
            int depth;
            uint64_t expected;
        } kiwipete_tests[] = {
            {1, 48},
            {2, 2039},
            {3, 97862}
        };
        
        for (auto& test : kiwipete_tests) {
            if (test.depth <= 3) {
                auto start = std::chrono::high_resolution_clock::now();
                uint64_t result = perft(pos, test.depth);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                std::cout << "Depth " << test.depth << ": " << result << " nodes";
                std::cout << " (expected: " << test.expected << ")";
                std::cout << " [" << duration.count() << "ms]";
                
                if (result == test.expected) {
                    std::cout << " ✅ PASS\n";
                } else {
                    std::cout << " ❌ FAIL\n";
                }
            }
        }
    } else {
        std::cout << "❌ Failed to set Kiwipete position\n";
    }
    
    // Test 3: En passant test
    std::cout << "\nTest 3: En Passant Position\n";
    std::string en_passant_fen = "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3";
    std::cout << "FEN: " << en_passant_fen << "\n";
    
    if (pos.set_from_fen(en_passant_fen)) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t result = perft(pos, 2);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Depth 2: " << result << " nodes [" << duration.count() << "ms]\n";
        std::cout << "(This should include en passant captures)\n";
    } else {
        std::cout << "❌ Failed to set en passant position\n";
    }
    
    // Test 4: Our recently fixed position
    std::cout << "\nTest 4: Recently Fixed Position (after e2e4 d7d5)\n";
    std::string fixed_fen = "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";
    std::cout << "FEN: " << fixed_fen << "\n";
    
    if (pos.set_from_fen(fixed_fen)) {
        // Test divide to see all possible moves
        uint64_t result = perft_divide(pos, 2);
        std::cout << "(This should include e4d5 pawn capture)\n";
    } else {
        std::cout << "❌ Failed to set fixed position\n";
    }
}

void compare_with_huginn() {
    std::cout << "\n=== Performance Comparison: huginn vs huginn2 ===\n";
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "Testing starting position at depth 4...\n";
    
    // Test huginn2 (this executable)
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t huginn2_nodes = perft(pos, 4);
    auto end = std::chrono::high_resolution_clock::now();
    auto huginn2_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "huginn2 (bitboard): " << huginn2_nodes << " nodes in " << huginn2_time.count() << "ms\n";
    std::cout << "Nodes per second: " << (huginn2_nodes * 1000 / huginn2_time.count()) << "\n";
    
    std::cout << "\nTo compare with original huginn, run:\n";
    std::cout << "echo \"perft 4\" | huginn.exe\n";
}

int main(int argc, char* argv[]) {
    std::cout << "Huginn2 Bitboard Perft Test\n";
    std::cout << "============================\n\n";
    
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "quick") {
            // Quick test - just depth 1-3
            Position pos;
            pos.set_startpos();
            
            for (int depth = 1; depth <= 3; ++depth) {
                auto start = std::chrono::high_resolution_clock::now();
                uint64_t result = perft(pos, depth);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                std::cout << "Depth " << depth << ": " << result << " nodes [" << duration.count() << "ms]\n";
            }
            return 0;
        } else if (arg == "divide") {
            // Divide test at depth specified
            int depth = (argc > 2) ? std::atoi(argv[2]) : 3;
            Position pos;
            pos.set_startpos();
            perft_divide(pos, depth);
            return 0;
        }
    }
    
    // Full test suite
    run_perft_suite();
    compare_with_huginn();
    
    return 0;
}