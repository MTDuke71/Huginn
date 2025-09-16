#include <iostream>
#include <iomanip>
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/init.hpp"
#include "../src/chess_types.hpp"

// Helper to print bitboard state
void print_bitboard_state(const Position& pos, const std::string& context) {
    std::cout << "\n=== BITBOARD STATE: " << context << " ===\n";
    
    // Print piece bitboards
    for (int color = 0; color < 2; ++color) {
        std::cout << (color == 0 ? "White" : "Black") << " pieces:\n";
        for (int piece = 0; piece < 6; ++piece) {
            uint64_t bb = pos.piece_bitboards[color][piece];
            if (bb != 0) {
                std::cout << "  " << piece << ": 0x" << std::hex << bb << std::dec;
                // Count bits
                int count = 0;
                uint64_t temp = bb;
                while (temp) {
                    count++;
                    temp &= temp - 1;
                }
                std::cout << " (" << count << " pieces)\n";
            }
        }
    }
    
    // Print color bitboards
    uint64_t white_pieces = pos.color_bitboards[0];
    uint64_t black_pieces = pos.color_bitboards[1];
    std::cout << "Color bitboards:\n";
    std::cout << "  White: 0x" << std::hex << white_pieces << std::dec << "\n";
    std::cout << "  Black: 0x" << std::hex << black_pieces << std::dec << "\n";
}

// Helper to convert move to string
std::string move_to_string(const S_MOVE& move) {
    int from = move.get_from();
    int to = move.get_to();
    
    // Convert 120-square to algebraic
    auto sq_to_algebraic = [](int sq120) {
        if (sq120 < 21 || sq120 > 98) return std::string("??");
        int file = (sq120 % 10) - 1;
        int rank = (sq120 / 10) - 2;
        if (file < 0 || file > 7 || rank < 0 || rank > 7) return std::string("??");
        return std::string(1, 'a' + file) + std::to_string(rank + 1);
    };
    
    return sq_to_algebraic(from) + sq_to_algebraic(to);
}

int main() {
    std::cout << "Bitboard State Debugging During Legal Move Filtering\n";
    std::cout << "====================================================\n";
    
    // Initialize engine
    Huginn::init();
    
    // Set up starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Print initial state
    print_bitboard_state(pos, "INITIAL POSITION");
    
    std::cout << "\n1. TESTING PSEUDO-LEGAL GENERATION:\n";
    S_MOVELIST pseudo_moves;
    BitboardMoveGen::generate_all_moves_bitboard(pos, pseudo_moves);
    std::cout << "Generated " << pseudo_moves.count << " pseudo-legal moves\n";
    
    // Print initial state again to check if generation corrupted anything
    print_bitboard_state(pos, "AFTER PSEUDO-LEGAL GENERATION");
    
    std::cout << "\n2. TESTING LEGAL MOVE FILTERING STEP BY STEP:\n";
    
    // Manually implement the legal move filtering to see where it breaks
    S_MOVELIST legal_moves;
    legal_moves.count = 0;
    
    // Make a copy to work with (same as generate_legal_moves_enhanced)
    Position temp_pos = pos;
    
    print_bitboard_state(temp_pos, "TEMP POSITION COPY");
    
    std::cout << "\nTesting each pseudo-legal move:\n";
    
    int knights_tested = 0;
    int knights_legal = 0;
    
    for (int i = 0; i < pseudo_moves.count; ++i) {
        const S_MOVE& move = pseudo_moves.moves[i];
        int from = move.get_from();
        std::string move_str = move_to_string(move);
        
        // Check if this is a knight move
        bool is_knight = (from == 22 || from == 27); // b1 or g1
        if (is_knight) {
            knights_tested++;
            std::cout << "\n--- TESTING KNIGHT MOVE " << knights_tested << ": " << move_str << " ---\n";
            print_bitboard_state(temp_pos, "BEFORE MAKE_MOVE");
        }
        
        // Try to make the move
        int result = temp_pos.MakeMove(move);
        
        if (is_knight) {
            std::cout << "MakeMove result: " << result << std::endl;
            print_bitboard_state(temp_pos, "AFTER MAKE_MOVE");
        }
        
        if (result == 1) {
            // Move is legal
            legal_moves.add_quiet_move(move);
            if (is_knight) {
                knights_legal++;
                std::cout << "✅ KNIGHT MOVE IS LEGAL\n";
            }
            
            // Unmake the move
            temp_pos.TakeMove();
            
            if (is_knight) {
                print_bitboard_state(temp_pos, "AFTER TAKE_MOVE");
            }
        } else {
            if (is_knight) {
                std::cout << "❌ KNIGHT MOVE REJECTED (result=" << result << ")\n";
            }
        }
    }
    
    std::cout << "\n3. FINAL RESULTS:\n";
    std::cout << "Total legal moves: " << legal_moves.count << std::endl;
    std::cout << "Knights tested: " << knights_tested << std::endl;
    std::cout << "Knights legal: " << knights_legal << std::endl;
    
    print_bitboard_state(pos, "FINAL ORIGINAL POSITION");
    
    return 0;
}