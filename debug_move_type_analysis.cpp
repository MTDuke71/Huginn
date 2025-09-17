#include "src/bitboard_perft_optimized.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <map>

void analyze_move_types() {
    std::cout << "=== Move Type Analysis for Depth 4 Extra Moves ===\n";

    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Count moves by type at each depth
    std::map<std::string, int> move_type_count;

    BitboardMoveGen movegen;
    BitboardMoveList moves;
    movegen.generate_legal_moves(pos, moves);

    std::cout << "Depth 3 moves from starting position: " << moves.size() << "\n";

    uint64_t total_depth4_moves = 0;

    for (size_t i = 0; i < moves.size(); i++) {
        SimpleBitboardMove move(moves.moves[i].from_64, moves.moves[i].to_64);

        // Make the move
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(move);

        // Generate moves from this position (depth 1 = total depth 4)
        BitboardMoveList depth4_moves;
        movegen.generate_legal_moves(pos, depth4_moves);

        total_depth4_moves += depth4_moves.size();

        // Count each move type
        for (const auto& d4_move : depth4_moves.moves) {
            int from = d4_move.from_64;
            int to = d4_move.to_64;

            // Determine piece type
            PieceType piece_type = pos.piece_type_at(from);
            std::string move_type;

            switch (piece_type) {
                case PieceType::Pawn:
                    // Check for special pawn moves
                    if (abs(to - from) == 16) move_type = "pawn_double";
                    else if (abs(to - from) == 7 || abs(to - from) == 9) {
                        if (pos.piece_at(to) != Piece::None) move_type = "pawn_capture";
                        else move_type = "pawn_en_passant";
                    }
                    else move_type = "pawn_single";
                    break;
                case PieceType::King:
                    // Check for castling
                    if (abs(to - from) == 2) move_type = "king_castle";
                    else move_type = "king_normal";
                    break;
                case PieceType::Knight: move_type = "knight"; break;
                case PieceType::Bishop: move_type = "bishop"; break;
                case PieceType::Rook: move_type = "rook"; break;
                case PieceType::Queen: move_type = "queen"; break;
                default: move_type = "unknown"; break;
            }

            move_type_count[move_type]++;
        }

        // Unmake the move
        pos.unmake_move(move, undo_info);
    }

    std::cout << "Total depth 4 moves: " << total_depth4_moves << " (expected: 197281)\n";
    std::cout << "Extra moves: " << (total_depth4_moves - 197281) << "\n\n";

    std::cout << "Move type breakdown:\n";
    for (const auto& pair : move_type_count) {
        std::cout << "  " << pair.first << ": " << pair.second << "\n";
    }
}

int main() {
    analyze_move_types();
    return 0;
}