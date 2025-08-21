#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include "src/position.hpp"
#include "src/movegen.hpp"
#include "src/init.hpp"

// High-performance manual position restore for two-argument system
void restore_position(Position& pos, const S_MOVE& move, const S_UNDO& undo) {
    // Restore side to move first
    pos.side_to_move = !pos.side_to_move;
    if (pos.side_to_move == Color::Black) --pos.fullmove_number;
    
    // Get the piece that moved
    Piece moved = pos.at(move.get_to());
    
    // Handle promotion undo - restore piece lists
    if (move.is_promotion()) {
        // Remove promoted piece from lists and add pawn back
        pos.remove_piece_from_list(color_of(moved), type_of(moved), move.get_to());
        moved = make_piece(color_of(moved), PieceType::Pawn);
        pos.add_piece_to_list(color_of(moved), PieceType::Pawn, move.get_from());
    } else {
        // Regular move - update piece location in lists
        pos.move_piece_in_list(color_of(moved), type_of(moved), move.get_to(), move.get_from());
    }
    
    // Move piece back
    pos.set(move.get_from(), moved);
    
    // Handle castling undo - restore rook position
    if (move.is_castle()) {
        Color king_color = color_of(moved);
        int rook_from, rook_to;
        
        if (king_color == Color::White) {
            if (move.get_to() == sq(File::G, Rank::R1)) { // Kingside
                rook_from = sq(File::H, Rank::R1);
                rook_to = sq(File::F, Rank::R1);
            } else { // Queenside
                rook_from = sq(File::A, Rank::R1);
                rook_to = sq(File::D, Rank::R1);
            }
        } else {
            if (move.get_to() == sq(File::G, Rank::R8)) { // Kingside
                rook_from = sq(File::H, Rank::R8);
                rook_to = sq(File::F, Rank::R8);
            } else { // Queenside
                rook_from = sq(File::A, Rank::R8);
                rook_to = sq(File::D, Rank::R8);
            }
        }
        
        // Move the rook back to its original position
        Piece rook = pos.at(rook_to);
        pos.set(rook_from, rook);
        pos.set(rook_to, Piece::None);
        pos.move_piece_in_list(king_color, PieceType::Rook, rook_to, rook_from);
    }

    // Handle en passant undo
    if (move.is_en_passant()) {
        // Restore the captured pawn to its original square
        Color moving_color = color_of(moved);
        int captured_pawn_sq;
        if (moving_color == Color::White) {
            // White captured en passant - restore black pawn south of destination
            captured_pawn_sq = move.get_to() + SOUTH;
        } else {
            // Black captured en passant - restore white pawn north of destination
            captured_pawn_sq = move.get_to() + NORTH;
        }
        
        // Restore the captured pawn and update piece lists
        pos.set(captured_pawn_sq, undo.captured);
        pos.set(move.get_to(), Piece::None); // Clear the destination square
        
        // Restore the captured pawn to piece lists
        if (!is_none(undo.captured)) {
            pos.add_piece_to_list(color_of(undo.captured), PieceType::Pawn, captured_pawn_sq);
        }
    } else {
        pos.set(move.get_to(), undo.captured); // Restore captured piece (or Piece::None)
        
        // Restore captured piece to piece lists
        if (!is_none(undo.captured)) {
            pos.add_piece_to_list(color_of(undo.captured), type_of(undo.captured), move.get_to());
        }
    }
    
    // Restore position state
    pos.castling_rights = undo.castling_rights;
    pos.ep_square = undo.ep_square;
    pos.halfmove_clock = undo.halfmove_clock;
    pos.zobrist_key = undo.zobrist_key;
    
    // Restore derived state incrementally (much faster than rebuild_counts)
    pos.restore_derived_state(undo);
}

std::string square_to_algebraic(int square) {
    File f = file_of(square);
    Rank r = rank_of(square);
    std::string result;
    result += char('a' + int(f));
    result += char('1' + int(r));
    return result;
}

uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    MoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.v) {
        pos.make_move_with_undo(move);  // Single argument - high performance
        nodes += perft(pos, depth - 1);
        pos.undo_move();                // Automatic undo - no manual restore needed
    }
    return nodes;
}

// Perft divide - shows progress for each root move
uint64_t perft_divide(Position& pos, int depth, bool show_progress = true) {
    if (depth == 0) return 1;
    
    MoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    int move_count = 0;
    
    if (show_progress) {
        std::cout << "\n=== Perft Divide (Depth " << depth << ") ===" << std::endl;
        std::cout << std::left << std::setw(8) << "Move" << std::setw(12) << "Nodes" << std::setw(10) << "Progress" << std::endl;
        std::cout << "-----------------------------" << std::endl;
    }
    
    for (const auto& move : moves.v) {
        move_count++;
        
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        if (move.is_promotion()) {
            switch (move.get_promoted()) {
                case PieceType::Queen: move_str += "q"; break;
                case PieceType::Rook: move_str += "r"; break;
                case PieceType::Bishop: move_str += "b"; break;
                case PieceType::Knight: move_str += "n"; break;
                default: break;
            }
        }
        if (move.is_castle()) {
            move_str += " (castle)";
        }
        
        S_UNDO undo;
        pos.make_move_with_undo(move);  // Single argument - high performance
        
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t nodes = (depth == 1) ? 1 : perft(pos, depth - 1);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        pos.undo_move();  // Automatic undo - no manual restore needed
        
        total_nodes += nodes;
        
        if (show_progress) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << std::left << std::setw(8) << move_str 
                      << std::setw(12) << nodes 
                      << "(" << move_count << "/" << moves.size() << ") " 
                      << duration.count() << "ms" << std::endl;
        }
    }
    
    if (show_progress) {
        std::cout << "-----------------------------" << std::endl;
        std::cout << std::left << std::setw(8) << "TOTAL" << std::setw(12) << total_nodes << std::endl;
    }
    
    return total_nodes;
}

int main() {
    Huginn::init();
    
    std::cout << "=== High-Performance Kiwipete Position Analysis ===" << std::endl;
    
    // Set up Kiwipete position
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cout << "Failed to parse Kiwipete FEN" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << kiwipete_fen << std::endl;
    
    // Test at multiple depths to verify the manual restore works correctly
    std::cout << "\n=== Testing at multiple depths ===" << std::endl;
    
    uint64_t depth1_result = perft(pos, 1);
    std::cout << "Depth 1 - Expected: 48, Actual: " << depth1_result << ", Difference: " << (int64_t(depth1_result) - 48) << std::endl;
    
    if (depth1_result == 48) {
        std::cout << "✅ Depth 1 PASSED!" << std::endl;
        
        uint64_t depth2_result = perft(pos, 2);
        std::cout << "Depth 2 - Expected: 2039, Actual: " << depth2_result << ", Difference: " << (int64_t(depth2_result) - 2039) << std::endl;
        
        if (depth2_result == 2039) {
            std::cout << "✅ Depth 2 PASSED!" << std::endl;
            
            // Use perft_divide for depth 3 to show progress
            std::cout << "\n=== Depth 3 Analysis with Progress ===" << std::endl;
            uint64_t depth3_result = perft_divide(pos, 3);
            std::cout << "Depth 3 - Expected: 97862, Actual: " << depth3_result << ", Difference: " << (int64_t(depth3_result) - 97862) << std::endl;
            
            if (depth3_result == 97862) {
                std::cout << "🎉 Depth 3 PASSED! High-performance two-argument system working correctly!" << std::endl;
            } else {
                std::cout << "❌ Depth 3 FAILED. Need to investigate." << std::endl;
            }
        } else {
            std::cout << "❌ Depth 2 FAILED. Showing move breakdown:" << std::endl;
            perft_divide(pos, 2);
        }
    } else {
        std::cout << "❌ Depth 1 FAILED. Showing move breakdown:" << std::endl;
        perft_divide(pos, 1);
    }
    
    return 0;
}
