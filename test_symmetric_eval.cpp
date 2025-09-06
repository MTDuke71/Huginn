// Test evaluation symmetry with only symmetric components
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"
#include "src/init.hpp"
#include <iostream>

using namespace Huginn;

// Simplified symmetric evaluation for testing
int evaluate_symmetric_only(const Position& pos) {
    int score = 0;
    
    // Only material and piece-square tables (should be symmetric)
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard) continue;
        
        Piece piece = pos.board[sq];
        if (piece == Piece::None) continue;
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material value
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn: material_value = 100; break;
            case PieceType::Knight: material_value = 320; break;
            case PieceType::Bishop: material_value = 330; break;
            case PieceType::Rook: material_value = 500; break;
            case PieceType::Queen: material_value = 900; break;
            case PieceType::King: material_value = 20000; break;
            default: material_value = 0; break;
        }
        
        // Simple piece-square value (using 64-square index)
        int sq64 = (sq - 21) - 2 * ((sq - 21) / 10);  // Convert 120->64
        if (sq64 < 0 || sq64 > 63) continue;
        
        int pst_value = 0;  // Could add piece-square tables here
        
        int piece_value = material_value + pst_value;
        
        if (piece_color == Color::White) {
            score += piece_value;
        } else {
            score -= piece_value;
        }
    }
    
    // ONLY the symmetric pawn structure evaluation 
    int pawn_structure_score = 0;
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    
    for (int sq64 = 0; sq64 < 64; ++sq64) {
        int sq120 = (sq64 / 8 + 2) * 10 + (sq64 % 8 + 1);
        Piece piece = pos.board[sq120];
        
        if (piece == Piece::WhitePawn) {
            int file = sq64 % 8;
            int rank = sq64 / 8;
            
            // Isolated pawn penalty
            if ((white_pawns & ISOLATED_PAWN_MASKS[file]) == 0) {
                pawn_structure_score -= ISOLATED_PAWN_PENALTY;
            }
            
            // Passed pawn bonus
            if ((black_pawns & WHITE_PASSED_PAWN_MASKS[sq64]) == 0) {
                pawn_structure_score += PASSED_PAWN_BONUS[rank];
            }
            
        } else if (piece == Piece::BlackPawn) {
            int file = sq64 % 8;
            int rank = sq64 / 8;
            
            // Isolated pawn penalty
            if ((black_pawns & ISOLATED_PAWN_MASKS[file]) == 0) {
                pawn_structure_score += ISOLATED_PAWN_PENALTY;
            }
            
            // Passed pawn bonus
            if ((white_pawns & BLACK_PASSED_PAWN_MASKS[sq64]) == 0) {
                int mirror_rank = 7 - rank;
                pawn_structure_score -= PASSED_PAWN_BONUS[mirror_rank];
            }
        }
    }
    
    score += pawn_structure_score;
    
    // Return from white's perspective 
    return (pos.side_to_move == Color::White) ? score : -score;
}

void test_symmetric_evaluation(const std::string& fen, MinimalEngine& engine) {
    std::cout << "\n=== Testing FEN: " << fen << " ===" << std::endl;
    
    Position pos;
    pos.set_from_fen(fen);
    
    int eval1 = evaluate_symmetric_only(pos);
    std::cout << "Original eval (symmetric only): " << eval1 << " cp" << std::endl;
    
    Position mirrored = engine.mirrorBoard(pos);
    int eval2 = evaluate_symmetric_only(mirrored);
    std::cout << "Mirrored eval (symmetric only): " << eval2 << " cp" << std::endl;
    
    if (eval1 == eval2) {
        std::cout << "✓ PASS: Symmetric evaluation!" << std::endl;
    } else {
        std::cout << "✗ FAIL: Asymmetric by " << abs(eval1 - eval2) << " cp" << std::endl;
    }
}

int main() {
    std::cout << "=== Testing Evaluation Symmetry ===" << std::endl;
    
    init();
    MinimalEngine engine;
    
    // Test the failing position with only symmetric parts
    test_symmetric_evaluation("r3k2r/pp3ppp/2n5/2bp4/2nP4/2P5/PP3PPP/R3K2R w KQkq - 0 10", engine);
    
    // Test starting position
    test_symmetric_evaluation("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", engine);
    
    return 0;
}
