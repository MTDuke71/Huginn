/**
 * @file debug_movegen.cpp
 * @brief Debug move generation for starting position
 */

#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

void debug_board_state(const BitboardPosition& pos) {
    std::cout << "=== BOARD STATE DEBUG ===" << std::endl;
    
    // Check what pieces are on each square
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            Piece piece = pos.piece_at(square);
            
            if (piece == Piece::None) {
                std::cout << ". ";
            } else {
                Color color = pos.color_at(square);
                PieceType type = pos.piece_type_at(square);
                
                char piece_char = ' ';
                switch (type) {
                    case PieceType::Pawn: piece_char = 'P'; break;
                    case PieceType::Knight: piece_char = 'N'; break;
                    case PieceType::Bishop: piece_char = 'B'; break;
                    case PieceType::Rook: piece_char = 'R'; break;
                    case PieceType::Queen: piece_char = 'Q'; break;
                    case PieceType::King: piece_char = 'K'; break;
                    default: piece_char = '?'; break;
                }
                
                if (color == Color::Black) {
                    piece_char = tolower(piece_char);
                }
                
                std::cout << piece_char << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl;
    std::cout << std::endl;
}

void debug_moves_for_square(const BitboardPosition& pos, int square) {
    char file_char = 'a' + (square % 8);
    char rank_char = '1' + (square / 8);
    
    Piece piece = pos.piece_at(square);
    if (piece == Piece::None) {
        std::cout << "Square " << file_char << rank_char << ": EMPTY" << std::endl;
        return;
    }
    
    Color color = pos.color_at(square);
    PieceType type = pos.piece_type_at(square);
    
    std::cout << "Square " << file_char << rank_char << ": " 
              << (color == Color::White ? "White " : "Black ")
              << (type == PieceType::Pawn ? "Pawn" :
                  type == PieceType::Knight ? "Knight" :
                  type == PieceType::Bishop ? "Bishop" :
                  type == PieceType::Rook ? "Rook" :
                  type == PieceType::Queen ? "Queen" :
                  type == PieceType::King ? "King" : "Unknown")
              << std::endl;
}

void test_perft_depth_2(BitboardPosition& pos, int max_moves = 5) {
    std::cout << "\n=== TESTING PERFT DEPTH 2 ===" << std::endl;
    
    BitboardMoveList moves1;
    generate_all_moves(pos, moves1);
    
    int move_count = 0;
    for (const auto& move1 : moves1.moves) {
        if (move_count >= max_moves) break;
        
        // Convert move
        SimpleBitboardMove simple_move1;
        simple_move1.from_64 = move1.from_64;
        simple_move1.to_64 = move1.to_64;
        simple_move1.is_capture = move1.is_capture;
        simple_move1.is_ep_capture = move1.is_ep_capture;
        simple_move1.is_castling = move1.is_castling;
        simple_move1.is_promotion = move1.is_promotion;
        simple_move1.promotion_type = move1.promotion_type;
        
        std::cout << "\nFirst move: " << char('a' + (move1.from_64 % 8)) << char('1' + (move1.from_64 / 8))
                  << char('a' + (move1.to_64 % 8)) << char('1' + (move1.to_64 / 8)) << std::endl;
        
        BitboardPosition::UndoInfo undo_info1 = pos.make_move_with_undo(simple_move1);
        
        // Generate moves for second level
        BitboardMoveList moves2;
        generate_all_moves(pos, moves2);
        
        std::cout << "Generated " << moves2.moves.size() << " moves after first move" << std::endl;
        
        // Check first few second-level moves
        for (size_t i = 0; i < std::min(size_t(3), moves2.moves.size()); i++) {
            const auto& move2 = moves2.moves[i];
            std::cout << "  Second move " << (i+1) << ": " 
                      << char('a' + (move2.from_64 % 8)) << char('1' + (move2.from_64 / 8))
                      << char('a' + (move2.to_64 % 8)) << char('1' + (move2.to_64 / 8));
            
            // Check what piece is on the from square
            Piece piece = pos.piece_at(move2.from_64);
            if (piece == Piece::None) {
                std::cout << " - ERROR: No piece on from square!" << std::endl;
            } else {
                Color color = pos.color_at(move2.from_64);
                PieceType type = pos.piece_type_at(move2.from_64);
                std::cout << " - " << (color == Color::White ? "White " : "Black ")
                          << (type == PieceType::Pawn ? "Pawn" :
                              type == PieceType::Knight ? "Knight" :
                              type == PieceType::Bishop ? "Bishop" :
                              type == PieceType::Rook ? "Rook" :
                              type == PieceType::Queen ? "Queen" :
                              type == PieceType::King ? "King" : "Unknown") << std::endl;
            }
        }
        
        pos.unmake_move(simple_move1, undo_info1);
        move_count++;
    }
}

int main() {
    std::cout << "Debug Move Generation for Starting Position" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        std::cout << "Error: Failed to parse starting position FEN" << std::endl;
        return 1;
    }
    
    debug_board_state(pos);
    
    // Generate moves
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Generated " << moves.moves.size() << " moves:" << std::endl;
    
    // Show first 10 moves with details
    for (size_t i = 0; i < std::min(size_t(10), moves.moves.size()); i++) {
        const auto& move = moves.moves[i];
        
        std::cout << "Move " << (i+1) << ": ";
        std::cout << char('a' + (move.from_64 % 8)) << char('1' + (move.from_64 / 8));
        std::cout << char('a' + (move.to_64 % 8)) << char('1' + (move.to_64 / 8));
        
        std::cout << " (from square " << move.from_64 << " to square " << move.to_64 << ")";
        
        // Check what piece is moving
        std::cout << " - ";
        debug_moves_for_square(pos, move.from_64);
    }
    
    // Test knight moves specifically
    std::cout << "\n=== TESTING KNIGHT MOVES ===" << std::endl;
    for (size_t i = 16; i < moves.moves.size(); i++) {  // Knight moves start around index 16
        const auto& move = moves.moves[i];
        std::cout << "Move " << (i+1) << ": ";
        std::cout << char('a' + (move.from_64 % 8)) << char('1' + (move.from_64 / 8));
        std::cout << char('a' + (move.to_64 % 8)) << char('1' + (move.to_64 / 8));
        std::cout << " - ";
        debug_moves_for_square(pos, move.from_64);
    }
    
    // Test depth 2 perft
    test_perft_depth_2(pos, 3);
    
    return 0;
}