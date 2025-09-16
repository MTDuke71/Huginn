#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>

int main() {
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN\n";
        return 1;
    }
    
    // Print piece at e5
    Piece e5_piece = pos.piece_at(36);
    std::cout << "Piece at e5: type=" << (int)type_of(e5_piece) << ", color=" << (int)color_of(e5_piece) << "\n";
    
    // Print all white piece bitboards
    std::cout << "White pieces:\n";
    for (int pt = 1; pt <= 6; pt++) {
        uint64_t bb = pos.get_pieces(Color::White, static_cast<PieceType>(pt));
        std::cout << "  Type " << pt << ": 0x" << std::hex << bb << std::dec;
        if (bb & (1ULL << 36)) std::cout << " (includes e5)";
        std::cout << "\n";
    }
    
    // Test each generator individually
    BitboardMoveList moves;
    
    std::cout << "\nTesting individual generators:\n";
    
    // Pawn moves
    moves.clear();
    BitboardMoveGen::generate_pawn_moves(pos, moves);
    int pawn_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) pawn_e5_count++;
    }
    std::cout << "Pawn generator: " << pawn_e5_count << " moves from e5\n";
    
    // Knight moves
    moves.clear();
    BitboardMoveGen::generate_knight_moves(pos, moves);
    int knight_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) knight_e5_count++;
    }
    std::cout << "Knight generator: " << knight_e5_count << " moves from e5\n";
    
    // Bishop moves
    moves.clear();
    BitboardMoveGen::generate_bishop_moves(pos, moves);
    int bishop_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) bishop_e5_count++;
    }
    std::cout << "Bishop generator: " << bishop_e5_count << " moves from e5\n";
    
    // Rook moves
    moves.clear();
    BitboardMoveGen::generate_rook_moves(pos, moves);
    int rook_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) rook_e5_count++;
    }
    std::cout << "Rook generator: " << rook_e5_count << " moves from e5\n";
    
    // Queen moves
    moves.clear();
    BitboardMoveGen::generate_queen_moves(pos, moves);
    int queen_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) queen_e5_count++;
    }
    std::cout << "Queen generator: " << queen_e5_count << " moves from e5\n";
    
    // King moves
    moves.clear();
    BitboardMoveGen::generate_king_moves(pos, moves);
    int king_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) king_e5_count++;
    }
    std::cout << "King generator: " << king_e5_count << " moves from e5\n";
    
    // Test generate_all_moves (pseudo-legal)
    moves.clear();
    BitboardMoveGen::generate_all_moves(pos, moves);
    int all_e5_count = 0;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) all_e5_count++;
    }
    std::cout << "generate_all_moves: " << all_e5_count << " moves from e5\n";
    
    // Test generate_legal_moves (after legality filtering)
    moves.clear();
    BitboardMoveGen::generate_legal_moves(pos, moves);
    int legal_e5_count = 0;
    std::vector<int> legal_e5_targets;
    for (auto& move : moves.moves) {
        if (move.from_64 == 36) {
            legal_e5_count++;
            legal_e5_targets.push_back(move.to_64);
        }
    }
    std::cout << "generate_legal_moves: " << legal_e5_count << " moves from e5\n";
    std::cout << "Legal e5 targets: ";
    for (int target : legal_e5_targets) {
        char file = 'a' + (target % 8);
        char rank = '1' + (target / 8);
        std::cout << file << rank << " ";
    }
    std::cout << "\n";
    
    return 0;
}