#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>

using namespace BitboardMoveGen;

// Create f3f5 move manually 
SimpleBitboardMove create_f3f5_move() {
    SimpleBitboardMove move;
    move.from_64 = 21; // f3
    move.to_64 = 37;   // f5  
    move.is_capture = false;
    move.is_ep_capture = false;
    move.is_castling = false;
    move.is_promotion = false;
    move.promotion_type = static_cast<PieceType>(0);
    return move;
}

void print_position_state(const BitboardPosition& pos, const std::string& label) {
    std::cout << "\n=== " << label << " ===" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "En passant square: " << pos.ep_square_64 << std::endl;
    std::cout << "Castling rights: " << pos.castling_rights << std::endl;
    std::cout << "Halfmove clock: " << pos.halfmove_clock << std::endl;
    std::cout << "Fullmove number: " << pos.fullmove_number << std::endl;
    std::cout << "Ply: " << pos.ply << std::endl;
    
    // Print piece positions for key squares
    std::cout << "Key pieces:" << std::endl;
    std::cout << "  f3 (21): " << static_cast<int>(pos.piece_type_at(21)) << std::endl;
    std::cout << "  f5 (37): " << static_cast<int>(pos.piece_type_at(37)) << std::endl;
    std::cout << "  e8 (60): " << static_cast<int>(pos.piece_type_at(60)) << std::endl;
    std::cout << "  h3 (23): " << static_cast<int>(pos.piece_type_at(23)) << std::endl;
    
    // Check legal move count
    BitboardMoveList moves;
    generate_legal_moves(const_cast<BitboardPosition&>(pos), moves);
    std::cout << "Legal moves: " << moves.moves.size() << std::endl;
    
    // Print first few moves to see differences
    std::cout << "First 10 moves:" << std::endl;
    for (size_t i = 0; i < std::min(10ULL, moves.moves.size()); ++i) {
        const auto& move = moves.moves[i];
        char from_file = 'a' + (move.from_64 % 8);
        char from_rank = '1' + (move.from_64 / 8);
        char to_file = 'a' + (move.to_64 % 8);
        char to_rank = '1' + (move.to_64 / 8);
        std::cout << "  " << from_file << from_rank << to_file << to_rank << std::endl;
    }
}

int main() {
    std::cout << "=== POSITION STATE COMPARISON ===" << std::endl;
    
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const std::string f3f5_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PNQ2/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 1 1";
    
    // Method 1: Direct FEN parsing (TRUSTED)
    BitboardPosition pos1;
    if (!pos1.set_from_fen(f3f5_fen)) {
        std::cerr << "Failed to parse f3f5 FEN!" << std::endl;
        return 1;
    }
    print_position_state(pos1, "DIRECT FEN PARSING (TRUSTED)");
    
    // Method 2: Make move via engine (SUSPECTED BUG)
    BitboardPosition pos2;
    if (!pos2.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    SimpleBitboardMove f3f5_move = create_f3f5_move();
    std::cout << "\nMaking f3f5 move via engine..." << std::endl;
    BitboardPosition::UndoInfo undo = pos2.make_move_with_undo(f3f5_move);
    print_position_state(pos2, "ENGINE MOVE MAKING (SUSPECTED BUG)");
    
    // Method 3: Validate the f3f5 FEN is correct
    std::cout << "\n=== VALIDATION ===" << std::endl;
    std::cout << "f3f5 FEN: " << f3f5_fen << std::endl;
    std::cout << "This should represent position after f3f5 from Kiwipete" << std::endl;
    
    return 0;
}