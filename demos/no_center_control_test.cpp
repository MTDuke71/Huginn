/**
 * Test VICE e4-c6 Claim Without Center Control
 * 
 * Tests the 1.e4 c6 position asymmetry when we eliminate the center control
 * component from evaluation to see if that's causing the VICE claim to fail.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

struct ModifiedEvaluation {
    int material = 0;
    int piece_square_tables = 0;
    int development = 0;
    // Removed center_control
    int pawn_penalties = 0;
    int total = 0;
};

ModifiedEvaluation evaluate_without_center_control(const Position& pos) {
    using namespace Huginn;
    ModifiedEvaluation result;
    
    // Material calculation and piece-square tables
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material
        int piece_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   piece_value = 100; break;
            case PieceType::Knight: piece_value = 320; break;
            case PieceType::Bishop: piece_value = 330; break;
            case PieceType::Rook:   piece_value = 500; break;
            case PieceType::Queen:  piece_value = 900; break;
            case PieceType::King:   piece_value = 20000; break;
            default: piece_value = 0; break;
        }
        
        if (piece_color == Color::White) {
            result.material += piece_value;
        } else {
            result.material -= piece_value;
        }
        
        // Piece-square tables
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 >= 0) {
            int pst_sq64 = sq64;
            if (piece_color == Color::Black) {
                pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
            }
            
            int pst_value = 0;
            switch (piece_type) {
                case PieceType::Pawn:   pst_value = Huginn::EvalParams::PAWN_TABLE[pst_sq64]; break;
                case PieceType::Knight: pst_value = Huginn::EvalParams::KNIGHT_TABLE[pst_sq64]; break;
                case PieceType::Bishop: pst_value = Huginn::EvalParams::BISHOP_TABLE[pst_sq64]; break;
                case PieceType::Rook:   pst_value = Huginn::EvalParams::ROOK_TABLE[pst_sq64]; break;
                case PieceType::Queen:  pst_value = Huginn::EvalParams::QUEEN_TABLE[pst_sq64]; break;
                case PieceType::King:   pst_value = Huginn::EvalParams::KING_TABLE[pst_sq64]; break;
                default: pst_value = 0; break;
            }
            
            if (piece_color == Color::White) {
                result.piece_square_tables += pst_value;
            } else {
                result.piece_square_tables -= pst_value;
            }
        }
    }
    
    // NO CENTER CONTROL CALCULATION
    
    result.total = result.material + result.piece_square_tables + result.development + result.pawn_penalties;
    return result;
}

void test_without_center_control() {
    using namespace Huginn;
    
    std::cout << "VICE e4-c6 Test WITHOUT Center Control\n";
    std::cout << "======================================\n\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    // Create mirrored position
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    std::string mirrored_fen = mirrored_pos.to_fen();
    
    std::cout << "Original Position (1.e4 c6):\n";
    std::cout << "FEN: " << e4_c6_fen << "\n\n";
    
    std::cout << "Mirrored Position:\n";
    std::cout << "FEN: " << mirrored_fen << "\n\n";
    
    // Evaluate without center control
    ModifiedEvaluation orig_eval = evaluate_without_center_control(original_pos);
    ModifiedEvaluation mirr_eval = evaluate_without_center_control(mirrored_pos);
    
    std::cout << "Evaluation WITHOUT Center Control:\n";
    std::cout << "==================================\n";
    std::cout << std::setw(20) << "Component" << std::setw(10) << "Original" << std::setw(10) << "Mirrored" << std::setw(12) << "Difference" << "\n";
    std::cout << std::string(52, '-') << "\n";
    
    std::cout << std::setw(20) << "Material:" << std::setw(10) << orig_eval.material 
              << std::setw(10) << mirr_eval.material 
              << std::setw(12) << (orig_eval.material - mirr_eval.material) << "\n";
              
    std::cout << std::setw(20) << "Piece-Square Tables:" << std::setw(10) << orig_eval.piece_square_tables 
              << std::setw(10) << mirr_eval.piece_square_tables 
              << std::setw(12) << (orig_eval.piece_square_tables - mirr_eval.piece_square_tables) << "\n";
              
    std::cout << std::setw(20) << "TOTAL:" << std::setw(10) << orig_eval.total 
              << std::setw(10) << mirr_eval.total 
              << std::setw(12) << (orig_eval.total - mirr_eval.total) << "\n\n";
    
    // Apply side-to-move perspective manually
    int orig_from_white_perspective = (original_pos.side_to_move == Color::White) ? orig_eval.total : -orig_eval.total;
    int mirr_from_black_perspective = (mirrored_pos.side_to_move == Color::White) ? mirr_eval.total : -mirr_eval.total;
    
    std::cout << "With Side-to-Move Perspective:\n";
    std::cout << "==============================\n";
    std::cout << "Original (White to move): " << std::setw(4) << orig_from_white_perspective << "\n";
    std::cout << "Mirrored (Black to move): " << std::setw(4) << mirr_from_black_perspective << "\n";
    std::cout << "Difference:               " << std::setw(4) << (orig_from_white_perspective - mirr_from_black_perspective) << "\n";
    std::cout << "Sum (should be 0):        " << std::setw(4) << (orig_from_white_perspective + mirr_from_black_perspective) << "\n\n";
    
    // Check for symmetry
    if (orig_from_white_perspective == -mirr_from_black_perspective) {
        std::cout << "✓ VICE CLAIM CONFIRMED WITHOUT CENTER CONTROL!\n";
        std::cout << "  Removing center control evaluation makes the positions symmetric.\n";
        std::cout << "  This proves center control was the source of asymmetry.\n";
    } else {
        std::cout << "❌ Still asymmetric even without center control.\n";
        std::cout << "  The asymmetry must come from piece-square tables or other factors.\n";
    }
    
    std::cout << "\nComparison with Full Evaluation:\n";
    std::cout << "================================\n";
    int full_orig = engine.evaluate(original_pos);
    int full_mirr = engine.evaluate(mirrored_pos);
    std::cout << "Full evaluation - Original: " << full_orig << ", Mirrored: " << full_mirr << "\n";
    std::cout << "Without center control would be: " << orig_from_white_perspective << " vs " << mirr_from_black_perspective << "\n";
    std::cout << "Center control impact: " << (full_orig - orig_from_white_perspective) << " points on original\n";
    std::cout << "Center control impact: " << (full_mirr - mirr_from_black_perspective) << " points on mirrored\n";
}

int main() {
    Huginn::init();
    test_without_center_control();
    return 0;
}
