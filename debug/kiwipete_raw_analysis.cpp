#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "position.hpp"
#include "bitboard_movegen.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// Kiwipete position FEN
const std::string KIWIPETE_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

// Convert move to string notation
std::string move_to_string(const S_MOVE& move) {
    int from = move.get_from();
    int to = move.get_to();
    
    // Convert to algebraic notation
    std::string from_str = "";
    std::string to_str = "";
    
    // Convert 120-square to algebraic
    if (from >= 21 && from <= 98) {
        int file = (from % 10) - 1;
        int rank = (from / 10) - 2;
        if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            from_str = char('a' + file);
            from_str += char('1' + rank);
        }
    }
    
    if (to >= 21 && to <= 98) {
        int file = (to % 10) - 1;
        int rank = (to / 10) - 2;
        if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            to_str = char('a' + file);
            to_str += char('1' + rank);
        }
    }
    
    if (from_str.empty() || to_str.empty()) {
        return "invalid_move";
    }
    
    std::string result = from_str + to_str;
    
    // Add promotion piece if applicable
    if (move.get_promoted() != PieceType::None) {
        switch (move.get_promoted()) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    
    return result;
}

// Analyze move generation and validation
void analyze_moves(Position& pos, const std::string& method_name, void (*generate_func)(const Position&, S_MOVELIST&)) {
    std::cout << method_name << " ANALYSIS:\n";
    std::cout << "=================\n";
    
    S_MOVELIST list;
    generate_func(pos, list);
    
    std::cout << "Raw moves generated: " << list.count << std::endl;
    
    std::vector<std::string> raw_moves, valid_moves, invalid_moves;
    
    for (int i = 0; i < list.count; i++) {
        const auto& move = list.moves[i];
        std::string move_str = move_to_string(move);
        raw_moves.push_back(move_str);
        
        // Test if move is valid by making it
        if (pos.MakeMove(move) == 1) {
            valid_moves.push_back(move_str);
            pos.TakeMove();
        } else {
            invalid_moves.push_back(move_str);
        }
    }
    
    std::sort(raw_moves.begin(), raw_moves.end());
    std::sort(valid_moves.begin(), valid_moves.end());
    std::sort(invalid_moves.begin(), invalid_moves.end());
    
    std::cout << "Valid moves after MakeMove: " << valid_moves.size() << std::endl;
    std::cout << "Invalid moves filtered out: " << invalid_moves.size() << std::endl;
    
    if (!invalid_moves.empty()) {
        std::cout << "Moves filtered out by MakeMove:\n";
        for (const auto& move : invalid_moves) {
            std::cout << "  " << move << " (rejected by MakeMove)\n";
        }
    }
    
    std::cout << "\nValid moves:\n";
    for (const auto& move : valid_moves) {
        std::cout << "  " << move << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    std::cout << "Kiwipete Raw Move Generation Analysis\n";
    std::cout << "====================================\n";
    std::cout << "FEN: " << KIWIPETE_FEN << std::endl;
    std::cout << "Expected: 48 legal moves\n";
    std::cout << "This analysis shows what moves are generated vs what passes MakeMove validation\n\n";
    
    Huginn::init();
    
    Position pos;
    if (!pos.set_from_fen(KIWIPETE_FEN)) {
        std::cout << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "Position loaded successfully.\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << std::endl;
    
    // Analyze piece list method
    analyze_moves(pos, "PIECE LIST", [](const Position& p, S_MOVELIST& l) { 
        l.count = 0; 
        generate_all_moves(p, l); 
    });
    
    // Analyze bitboard method  
    analyze_moves(pos, "BITBOARD", [](const Position& p, S_MOVELIST& l) { 
        l.count = 0; 
        BitboardMoveGen::generate_all_moves_bitboard(p, l); 
    });
    
    return 0;
}