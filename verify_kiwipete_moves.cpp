/**
 * @brief Comprehensive verification of UltraEngine move generation vs known Kiwipete results
 */
#include <iostream>
#include <vector>
#include <string>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

std::string square_to_algebraic(int sq64) {
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string ultra_move_to_string(const UltraMove& move) {
    std::string result = square_to_algebraic(move.from()) + square_to_algebraic(move.to());
    if (move.is_promotion()) {
        char pieces[] = {' ', 'n', 'b', 'r', 'q', ' ', 'k'};
        if (move.promoted() >= 1 && move.promoted() <= 5) {
            result += pieces[move.promoted()];
        }
    }
    return result;
}

void comprehensive_kiwipete_analysis() {
    std::cout << "=== Comprehensive Kiwipete Move Analysis ===\n";
    
    UltraPosition pos;
    pos.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    UltraMoveList all_moves;
    int total_count = pos.generate_all_moves(all_moves);
    
    std::cout << "Total moves: " << total_count << " (expected: 48)\n\n";
    
    // Categorize all moves
    std::vector<std::string> pawn_moves, knight_moves, bishop_moves, rook_moves, queen_moves, king_moves, castle_moves;
    
    for (int i = 0; i < all_moves.size(); ++i) {
        const UltraMove& move = all_moves[i];
        std::string move_str = ultra_move_to_string(move);
        
        // Categorize by from square (crude but effective for debugging)
        int from = move.from();
        
        if (move.is_castle()) {
            castle_moves.push_back(move_str);
        } else if (from == 4) { // e1 - king
            king_moves.push_back(move_str);
        } else if (from >= 8 && from <= 15) { // Second rank - pawns  
            pawn_moves.push_back(move_str);
        } else if (from == 18 || from == 19) { // c3, d3 - knights potentially
            knight_moves.push_back(move_str);
        } else {
            // Need to check the piece type more carefully
            // For now, categorize based on common squares in Kiwipete
            
            // Common piece squares in Kiwipete:
            // a1, h1 = rooks
            // d2, e2 = bishops  
            // f3 = queen
            // c3 = knight
            // e5 = knight
            
            if (from == 0 || from == 7) { // a1, h1 - rooks
                rook_moves.push_back(move_str);
            } else if (from == 11 || from == 12) { // d2, e2 - bishops
                bishop_moves.push_back(move_str);
            } else if (from == 21) { // f3 - queen
                queen_moves.push_back(move_str);
            } else if (from == 18 || from == 36) { // c3, e5 - knights
                knight_moves.push_back(move_str);
            } else {
                // Add logic for other pieces or print for manual categorization
                std::cout << "Uncategorized move: " << move_str << " from square " << from << "\n";
            }
        }
    }
    
    std::cout << "Move breakdown:\n";
    std::cout << "Pawn moves (" << pawn_moves.size() << "): ";
    for (const auto& move : pawn_moves) std::cout << move << " ";
    std::cout << "\n";
    
    std::cout << "Knight moves (" << knight_moves.size() << "): ";
    for (const auto& move : knight_moves) std::cout << move << " ";
    std::cout << "\n";
    
    std::cout << "Bishop moves (" << bishop_moves.size() << "): ";
    for (const auto& move : bishop_moves) std::cout << move << " ";
    std::cout << "\n";
    
    std::cout << "Rook moves (" << rook_moves.size() << "): ";
    for (const auto& move : rook_moves) std::cout << move << " ";
    std::cout << "\n";
    
    std::cout << "Queen moves (" << queen_moves.size() << "): ";
    for (const auto& move : queen_moves) std::cout << move << " ";
    std::cout << "\n";
    
    std::cout << "King moves (" << king_moves.size() << "): ";
    for (const auto& move : king_moves) std::cout << move << " ";
    std::cout << "\n";
    
    std::cout << "Castling moves (" << castle_moves.size() << "): ";
    for (const auto& move : castle_moves) std::cout << move << " ";
    std::cout << "\n";
    
    int categorized_total = pawn_moves.size() + knight_moves.size() + bishop_moves.size() + 
                           rook_moves.size() + queen_moves.size() + king_moves.size() + castle_moves.size();
    
    std::cout << "\nCategorized total: " << categorized_total << "\n";
    std::cout << "Actual total: " << total_count << "\n";
    std::cout << "Expected: 48\n";
    std::cout << "Missing: " << (48 - total_count) << " moves\n";
}

int main() {
    try {
        UltraAttacks::initialize();
        std::cout << "UltraEngine initialized successfully.\n\n";
        
        comprehensive_kiwipete_analysis();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}