#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "../src/position.hpp"
#include "../src/move.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/pawn_optimizations.hpp"
#include "../src/knight_optimizations.hpp"
#include "../src/sliding_piece_optimizations.hpp"
#include "../src/king_optimizations.hpp"
#include "../src/init.hpp"

class MoveGenDebugging {
private:
    Position pos;
    
    std::vector<S_MOVE> generate_piece_list_moves() {
        S_MOVELIST moves;
        moves.count = 0;
        
        Color us = pos.side_to_move;
        
        // Use the piece list generation from movegen_enhanced.cpp (the #else branch)
        PawnOptimizations::generate_pawn_moves_template(pos, moves, us);
        KnightOptimizations::generate_knight_moves_template(pos, moves, us);
        SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, moves, us);
        KingOptimizations::generate_king_moves_optimized(pos, moves, us);
        
        std::vector<S_MOVE> result;
        for (int i = 0; i < moves.count; ++i) {
            result.push_back(moves.moves[i]);
        }
        return result;
    }
    
    std::vector<S_MOVE> generate_bitboard_moves() {
        S_MOVELIST moves;
        moves.count = 0;
        
        // Use the bitboard generation
        BitboardMoveGen::generate_all_moves_bitboard(pos, moves);
        
        std::vector<S_MOVE> result;
        for (int i = 0; i < moves.count; ++i) {
            result.push_back(moves.moves[i]);
        }
        return result;
    }
    
    std::string move_to_string(const S_MOVE& move) {
        int from = move.get_from();
        int to = move.get_to();
        
        // Convert 120-square board indices to algebraic notation
        // Need to handle the board120 format conversion
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::string result = "";
        result += from_file;
        result += from_rank;
        result += to_file;
        result += to_rank;
        
        // Check for promotion
        PieceType promoted = move.get_promoted();
        if (promoted != PieceType::None) {
            switch (promoted) {
                case PieceType::Queen: result += "q"; break;
                case PieceType::Rook: result += "r"; break;
                case PieceType::Bishop: result += "b"; break;
                case PieceType::Knight: result += "n"; break;
                default: break;
            }
        }
        
        return result;
    }
public:
    void analyze_position(const std::string& fen, const std::string& name) {
        std::cout << "\n=== ANALYZING: " << name << " ===\n";
        std::cout << "FEN: " << fen << "\n\n";
        
        // Set up position
        pos.set_from_fen(fen);
        
        // Generate moves using both methods
        auto piece_list_moves = generate_piece_list_moves();
        auto bitboard_moves = generate_bitboard_moves();
        
        std::cout << "Piece List Moves: " << piece_list_moves.size() << "\n";
        std::cout << "Bitboard Moves:   " << bitboard_moves.size() << "\n";
        
        if (piece_list_moves.size() == bitboard_moves.size()) {
            std::cout << "✓ Move counts match!\n";
            return;
        }
        
        // Convert to string sets for comparison
        std::set<std::string> piece_list_set, bitboard_set;
        
        for (const S_MOVE& move : piece_list_moves) {
            piece_list_set.insert(move_to_string(move));
        }
        
        for (const S_MOVE& move : bitboard_moves) {
            bitboard_set.insert(move_to_string(move));
        }
        
        // Find differences
        std::vector<std::string> only_in_piece_list, only_in_bitboard;
        
        std::set_difference(piece_list_set.begin(), piece_list_set.end(),
                           bitboard_set.begin(), bitboard_set.end(),
                           std::back_inserter(only_in_piece_list));
                           
        std::set_difference(bitboard_set.begin(), bitboard_set.end(),
                           piece_list_set.begin(), piece_list_set.end(),
                           std::back_inserter(only_in_bitboard));
        
        std::cout << "\n🔍 MOVE DIFFERENCES:\n";
        
        if (!only_in_piece_list.empty()) {
            std::cout << "\nMoves ONLY in Piece List (" << only_in_piece_list.size() << "):\n";
            for (const auto& move : only_in_piece_list) {
                std::cout << "  " << move << "\n";
            }
        }
        
        if (!only_in_bitboard.empty()) {
            std::cout << "\nMoves ONLY in Bitboard (" << only_in_bitboard.size() << "):\n";
            for (const auto& move : only_in_bitboard) {
                std::cout << "  " << move << "\n";
            }
        }
        
        // Show some common moves for context
        std::vector<std::string> common_moves;
        std::set_intersection(piece_list_set.begin(), piece_list_set.end(),
                             bitboard_set.begin(), bitboard_set.end(),
                             std::back_inserter(common_moves));
        
        std::cout << "\nCommon moves (" << common_moves.size() << ") - showing first 10:\n";
        for (size_t i = 0; i < std::min(size_t(10), common_moves.size()); ++i) {
            std::cout << "  " << common_moves[i] << "\n";
        }
    }
    
    void run_analysis() {
        std::cout << "Huginn Move Generation Debugging Tool\n";
        std::cout << "=====================================\n";
        
        // Initialize engine
        Huginn::init();
        
        // Test the problematic positions from the benchmark
        analyze_position("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 
                        "Middlegame Position");
        
        analyze_position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 
                        "Tactical Position");
        
        // Also test a simple position for sanity check
        analyze_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 
                        "Starting Position");
    }
};

int main() {
    try {
        MoveGenDebugging debugger;
        debugger.run_analysis();
        
        std::cout << "\n\nDebugging complete! Check the differences above.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}