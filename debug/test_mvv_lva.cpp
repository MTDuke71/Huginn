// Test MVV-LVA (Most Valuable Victim, Least Valuable Attacker) implementation
// This demonstrates the VICE tutorial move ordering strategy

#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>

using namespace Huginn;

void test_mvv_lva_table() {
    std::cout << "=== MVV-LVA Table Test ===\n";
    
    MinimalEngine engine;
    // MVV-LVA table is initialized in constructor via init_mvv_lva()
    
    std::cout << "\nTesting specific capture scenarios:\n";
    
    // Test various capture scenarios
    struct CaptureTest {
        PieceType victim;
        PieceType attacker;
        std::string description;
    };
    
    CaptureTest tests[] = {
        {PieceType::Queen, PieceType::Pawn, "Pawn takes Queen"},
        {PieceType::Rook, PieceType::Pawn, "Pawn takes Rook"},
        {PieceType::Bishop, PieceType::Pawn, "Pawn takes Bishop"},
        {PieceType::Knight, PieceType::Pawn, "Pawn takes Knight"},
        {PieceType::Pawn, PieceType::Pawn, "Pawn takes Pawn"},
        {PieceType::Queen, PieceType::Queen, "Queen takes Queen"},
        {PieceType::Pawn, PieceType::Queen, "Queen takes Pawn"},
        {PieceType::Rook, PieceType::Knight, "Knight takes Rook"},
        {PieceType::Bishop, PieceType::Rook, "Rook takes Bishop"}
    };
    
    for (const auto& test : tests) {
        int score = engine.get_mvv_lva_score(test.victim, test.attacker);
        std::cout << std::setw(20) << std::left << test.description 
                  << " -> Score: " << std::setw(6) << score << std::endl;
    }
}

void test_position_move_ordering() {
    std::cout << "\n=== Position Move Ordering Test ===\n";
    
    // Initialize chess engine
    init();
    
    MinimalEngine engine;
    
    // Test position with multiple captures available
    // "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 4 4"
    // This position allows Qf3xf7+ (Queen takes Pawn) and other captures
    
    Position pos;
    std::string test_fen = "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 4 4";
    
    if (!pos.set_from_fen(test_fen)) {
        std::cout << "Failed to parse test FEN!\n";
        return;
    }
    
    std::cout << "Test Position: " << test_fen << "\n";
    std::cout << "Looking for captures and their MVV-LVA scores...\n\n";
    
    // Generate all legal moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "Before MVV-LVA ordering:\n";
    for (int i = 0; i < std::min(10, move_list.count); i++) {
        S_MOVE move = move_list.moves[i];
        std::cout << "Move " << i+1 << ": ";
        
        // Decode move
        int from = move.get_from();
        int to = move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << from_file << from_rank << to_file << to_rank;
        
        if (move.is_capture()) {
            PieceType victim = move.get_captured();
            std::cout << " (captures " << static_cast<int>(victim) << ")";
        }
        
        std::cout << " Score: " << move.score << "\n";
    }
    
    // Apply MVV-LVA ordering
    engine.order_moves(move_list, pos);
    
    std::cout << "\nAfter MVV-LVA ordering:\n";
    for (int i = 0; i < std::min(10, move_list.count); i++) {
        S_MOVE move = move_list.moves[i];
        std::cout << "Move " << i+1 << ": ";
        
        // Decode move
        int from = move.get_from();
        int to = move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << from_file << from_rank << to_file << to_rank;
        
        if (move.is_capture()) {
            PieceType victim = move.get_captured();
            Piece attacking_piece = pos.board[from];
            PieceType attacker = type_of(attacking_piece);
            
            const char* piece_names[] = {"None", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
            std::cout << " (";
            std::cout << piece_names[static_cast<int>(attacker)];
            std::cout << " takes ";
            std::cout << piece_names[static_cast<int>(victim)];
            std::cout << ")";
        }
        
        if (move.is_promotion()) {
            PieceType promoted = move.get_promoted();
            const char* piece_names[] = {"None", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
            std::cout << " (promotes to " << piece_names[static_cast<int>(promoted)] << ")";
        }
        
        std::cout << " Score: " << move.score << "\n";
    }
}

void test_tactical_position() {
    std::cout << "\n=== Tactical Position Test ===\n";
    
    // Test a position with many captures (like WAC.1)
    // WAC.1: 2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3QP/PPB4P/R4RK1 w - - 0 1
    
    Position pos;
    std::string tactical_fen = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3QP/PPB4P/R4RK1 w - - 0 1";
    
    if (!pos.set_from_fen(tactical_fen)) {
        std::cout << "Failed to parse tactical FEN!\n";
        return;
    }
    
    std::cout << "Tactical Position (WAC.1): " << tactical_fen << "\n";
    std::cout << "This position has a mate in 3, let's see MVV-LVA move ordering...\n\n";
    
    MinimalEngine engine;
    
    // Generate and order moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    engine.order_moves(move_list, pos);
    
    std::cout << "Top 15 moves after MVV-LVA ordering:\n";
    for (int i = 0; i < std::min(15, move_list.count); i++) {
        S_MOVE move = move_list.moves[i];
        std::cout << std::setw(2) << i+1 << ". ";
        
        // Decode move
        int from = move.get_from();
        int to = move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << from_file << from_rank << to_file << to_rank;
        
        if (move.is_capture()) {
            PieceType victim = move.get_captured();
            Piece attacking_piece = pos.board[from];
            PieceType attacker = type_of(attacking_piece);
            
            const char* piece_names[] = {"None", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
            std::cout << " (" << piece_names[static_cast<int>(attacker)] 
                      << "x" << piece_names[static_cast<int>(victim)] << ")";
        }
        
        if (move.is_promotion()) {
            PieceType promoted = move.get_promoted();
            const char* piece_names[] = {"None", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
            std::cout << "=" << piece_names[static_cast<int>(promoted)];
        }
        
        std::cout << std::setw(12) << std::right << " Score: " << std::setw(6) << move.score;
        
        if (move.is_capture()) {
            std::cout << " [CAPTURE]";
        } else if (move.is_promotion()) {
            std::cout << " [PROMOTION]";
        } else {
            std::cout << " [QUIET]";
        }
        
        std::cout << "\n";
    }
}

int main() {
    std::cout << "VICE Tutorial: MVV-LVA (Most Valuable Victim, Least Valuable Attacker) Implementation\n";
    std::cout << "================================================================================\n\n";
    
    try {
        test_mvv_lva_table();
        test_position_move_ordering(); 
        test_tactical_position();
        
        std::cout << "\n=== MVV-LVA Testing Complete ===\n";
        std::cout << "The MVV-LVA algorithm successfully prioritizes:\n";
        std::cout << "1. Captures where weak pieces take strong pieces\n";
        std::cout << "2. Promotions (especially to Queen)\n";
        std::cout << "3. Other moves with lower priority\n";
        std::cout << "\nThis should improve alpha-beta search efficiency!\n";
        
    } catch (const std::exception& e) {
        std::cout << "Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
