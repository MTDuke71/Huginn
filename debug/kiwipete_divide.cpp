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

// Generate moves using bitboard method
void generate_moves_bitboard(const Position& pos, S_MOVELIST& list) {
    list.count = 0;
    BitboardMoveGen::generate_all_moves_bitboard(pos, list);
}

// Generate moves using piece list method (VICE style)
void generate_moves_piecelist(const Position& pos, S_MOVELIST& list) {
    list.count = 0;
    generate_all_moves(pos, list);
}

// Convert move to string notation
std::string move_to_string(const S_MOVE& move, const Position& pos) {
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

// Perft divide - count nodes after each legal move
void perft_divide(Position& pos, const std::string& method_name, void (*generate_func)(const Position&, S_MOVELIST&)) {
    std::cout << method_name << " DIVIDE:\n";
    std::cout << "================\n";
    
    S_MOVELIST list;
    generate_func(pos, list);
    
    std::vector<std::pair<std::string, int>> move_counts;
    uint64_t total_nodes = 0;
    
    for (int i = 0; i < list.count; i++) {
        const auto& move = list.moves[i];
        
        // Validate move by making it
        if (pos.MakeMove(move) == 1) {
            std::string move_str = move_to_string(move, pos);
            move_counts.push_back({move_str, 1}); // Depth 1, so each legal move contributes 1 node
            total_nodes++;
            pos.TakeMove();
        }
    }
    
    // Sort moves alphabetically for easy comparison
    std::sort(move_counts.begin(), move_counts.end());
    
    for (const auto& mc : move_counts) {
        std::cout << "  " << mc.first << std::endl;
    }
    
    std::cout << "\nTotal moves: " << total_nodes << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "Kiwipete Position Divide Analysis (Depth 1)\n";
    std::cout << "==========================================\n";
    std::cout << "FEN: " << KIWIPETE_FEN << std::endl;
    std::cout << "Expected: 48 moves\n";
    std::cout << "Bitboard result: 46 moves (2 missing)\n\n";
    
    Huginn::init();
    
    Position pos;
    if (!pos.set_from_fen(KIWIPETE_FEN)) {
        std::cout << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "Position loaded successfully.\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << std::endl;
    
    // Generate moves with both methods
    std::cout << "=== MOVE GENERATION COMPARISON ===\n\n";
    
    // Piece list method first
    perft_divide(pos, "PIECE LIST", generate_moves_piecelist);
    
    // Bitboard method second
    perft_divide(pos, "BITBOARD", generate_moves_bitboard);
    
    // Now compare the differences
    S_MOVELIST piece_list, bitboard_list;
    generate_moves_piecelist(pos, piece_list);
    generate_moves_bitboard(pos, bitboard_list);
    
    // Collect valid moves from each method
    std::vector<std::string> piece_moves, bitboard_moves;
    
    for (int i = 0; i < piece_list.count; i++) {
        if (pos.MakeMove(piece_list.moves[i]) == 1) {
            piece_moves.push_back(move_to_string(piece_list.moves[i], pos));
            pos.TakeMove();
        }
    }
    
    for (int i = 0; i < bitboard_list.count; i++) {
        if (pos.MakeMove(bitboard_list.moves[i]) == 1) {
            bitboard_moves.push_back(move_to_string(bitboard_list.moves[i], pos));
            pos.TakeMove();
        }
    }
    
    // Sort for comparison
    std::sort(piece_moves.begin(), piece_moves.end());
    std::sort(bitboard_moves.begin(), bitboard_moves.end());
    
    std::cout << "=== DIFFERENCE ANALYSIS ===\n";
    std::cout << "Moves in PIECE LIST but NOT in BITBOARD:\n";
    
    bool found_missing = false;
    for (const auto& move : piece_moves) {
        if (std::find(bitboard_moves.begin(), bitboard_moves.end(), move) == bitboard_moves.end()) {
            std::cout << "  " << move << " *** MISSING FROM BITBOARD ***\n";
            found_missing = true;
        }
    }
    
    if (!found_missing) {
        std::cout << "  (none)\n";
    }
    
    std::cout << "\nMoves in BITBOARD but NOT in PIECE LIST:\n";
    bool found_extra = false;
    for (const auto& move : bitboard_moves) {
        if (std::find(piece_moves.begin(), piece_moves.end(), move) == piece_moves.end()) {
            std::cout << "  " << move << " *** EXTRA IN BITBOARD ***\n";
            found_extra = true;
        }
    }
    
    if (!found_extra) {
        std::cout << "  (none)\n";
    }
    
    std::cout << "\n=== SUMMARY ===\n";
    std::cout << "Piece list moves: " << piece_moves.size() << std::endl;
    std::cout << "Bitboard moves: " << bitboard_moves.size() << std::endl;
    std::cout << "Difference: " << (int)piece_moves.size() - (int)bitboard_moves.size() << std::endl;
    
    return 0;
}