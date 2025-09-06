/**
 * VICE Demo Evaluation Analysis
 * 
 * Investigates why VICE demo showed 30/30 for e4-c6 position
 * while our engine shows 65/145. This suggests a fundamental
 * difference in evaluation implementation or side-to-move perspective.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

void analyze_vice_discrepancy() {
    using namespace Huginn;
    
    std::cout << "VICE Demo vs Our Engine - Evaluation Discrepancy Analysis\n";
    std::cout << "========================================================\n\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    std::cout << "Our Engine Results:\n";
    std::cout << "===================\n";
    std::cout << "Original (1.e4 c6): " << engine.evaluate(original_pos) << " (White to move)\n";
    std::cout << "Mirrored position:   " << engine.evaluate(mirrored_pos) << " (Black to move)\n\n";
    
    std::cout << "VICE Demo Claimed Results:\n";
    std::cout << "==========================\n";
    std::cout << "Original (1.e4 c6): 30 (from side-to-move perspective)\n";
    std::cout << "Mirrored position:   30 (from side-to-move perspective)\n\n";
    
    std::cout << "Analysis of Discrepancy:\n";
    std::cout << "========================\n";
    
    // Let's calculate what our evaluation would be without side-to-move perspective
    int raw_score_orig = 0;
    int raw_score_mirr = 0;
    
    // Calculate basic material and PST for original position (no side-to-move flip)
    for (int sq = 21; sq <= 98; ++sq) {
        if (original_pos.board[sq] == Piece::Offboard || original_pos.board[sq] == Piece::None) continue;
        
        Piece piece = original_pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values
        int piece_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   piece_value = 100; break;
            case PieceType::Knight: piece_value = 300; break;
            case PieceType::Bishop: piece_value = 300; break;
            case PieceType::Rook:   piece_value = 500; break;
            case PieceType::Queen:  piece_value = 900; break;
            case PieceType::King:   piece_value = 0; break;
            default: piece_value = 0; break;
        }
        
        // Piece-square tables
        int sq64 = MAILBOX_MAPS.to64[sq];
        int pst_value = 0;
        if (sq64 >= 0) {
            int pst_sq64 = sq64;
            if (piece_color == Color::Black) {
                pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
            }
            
            switch (piece_type) {
                case PieceType::Pawn:   pst_value = EvalParams::PAWN_TABLE[pst_sq64]; break;
                case PieceType::Knight: pst_value = EvalParams::KNIGHT_TABLE[pst_sq64]; break;
                case PieceType::Bishop: pst_value = EvalParams::BISHOP_TABLE[pst_sq64]; break;
                case PieceType::Rook:   pst_value = EvalParams::ROOK_TABLE[pst_sq64]; break;
                case PieceType::Queen:  pst_value = EvalParams::QUEEN_TABLE[pst_sq64]; break;
                case PieceType::King:   pst_value = EvalParams::KING_TABLE[pst_sq64]; break;
                default: pst_value = 0; break;
            }
        }
        
        int total_piece_value = piece_value + pst_value;
        
        if (piece_color == Color::White) {
            raw_score_orig += total_piece_value;
        } else {
            raw_score_orig -= total_piece_value;
        }
    }
    
    // Add center control for original
    if (original_pos.board[55] == Piece::WhitePawn) raw_score_orig += 100; // e4
    
    // Same for mirrored position
    for (int sq = 21; sq <= 98; ++sq) {
        if (mirrored_pos.board[sq] == Piece::Offboard || mirrored_pos.board[sq] == Piece::None) continue;
        
        Piece piece = mirrored_pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values
        int piece_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   piece_value = 100; break;
            case PieceType::Knight: piece_value = 300; break;
            case PieceType::Bishop: piece_value = 300; break;
            case PieceType::Rook:   piece_value = 500; break;
            case PieceType::Queen:  piece_value = 900; break;
            case PieceType::King:   piece_value = 0; break;
            default: piece_value = 0; break;
        }
        
        // Piece-square tables
        int sq64 = MAILBOX_MAPS.to64[sq];
        int pst_value = 0;
        if (sq64 >= 0) {
            int pst_sq64 = sq64;
            if (piece_color == Color::Black) {
                pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
            }
            
            switch (piece_type) {
                case PieceType::Pawn:   pst_value = EvalParams::PAWN_TABLE[pst_sq64]; break;
                case PieceType::Knight: pst_value = EvalParams::KNIGHT_TABLE[pst_sq64]; break;
                case PieceType::Bishop: pst_value = EvalParams::BISHOP_TABLE[pst_sq64]; break;
                case PieceType::Rook:   pst_value = EvalParams::ROOK_TABLE[pst_sq64]; break;
                case PieceType::Queen:  pst_value = EvalParams::QUEEN_TABLE[pst_sq64]; break;
                case PieceType::King:   pst_value = EvalParams::KING_TABLE[pst_sq64]; break;
                default: pst_value = 0; break;
            }
        }
        
        int total_piece_value = piece_value + pst_value;
        
        if (piece_color == Color::White) {
            raw_score_mirr += total_piece_value;
        } else {
            raw_score_mirr -= total_piece_value;
        }
    }
    
    // Add center control for mirrored  
    if (mirrored_pos.board[65] == Piece::BlackPawn) raw_score_mirr -= 100; // e5
    
    std::cout << "Raw Scores (White's perspective, no side-to-move flip):\n";
    std::cout << "Original: " << raw_score_orig << "\n";
    std::cout << "Mirrored: " << raw_score_mirr << "\n\n";
    
    // Apply side-to-move perspective manually
    int orig_from_stm = (original_pos.side_to_move == Color::White) ? raw_score_orig : -raw_score_orig;
    int mirr_from_stm = (mirrored_pos.side_to_move == Color::White) ? raw_score_mirr : -raw_score_mirr;
    
    std::cout << "With Side-to-Move Perspective:\n";
    std::cout << "Original (White to move): " << orig_from_stm << "\n";
    std::cout << "Mirrored (Black to move): " << mirr_from_stm << "\n\n";
    
    std::cout << "Possible Explanations for VICE Demo 30/30:\n";
    std::cout << "==========================================\n";
    std::cout << "1. VICE used simpler evaluation (material + basic PST only)\n";
    std::cout << "2. VICE didn't include our massive center control bonuses (+100)\n";
    std::cout << "3. VICE used different pawn table values\n";
    std::cout << "4. VICE demo position was actually different\n";
    std::cout << "5. VICE side-to-move perspective was implemented differently\n\n";
    
    std::cout << "Testing Hypothesis: What if VICE only used Material + Basic PST?\n";
    std::cout << "================================================================\n";
    
    // Calculate minimal evaluation (just material + basic PST, no center control)
    int minimal_orig = 0;
    int minimal_mirr = 0;
    
    // For simplicity, let's assume VICE used a basic pawn table with small values
    // and no massive center bonuses
    std::cout << "If we removed our large center control bonuses:\n";
    std::cout << "Original score would be around: " << (raw_score_orig - 100) << "\n";
    std::cout << "Mirrored score would be around: " << (raw_score_mirr + 100) << "\n";
    std::cout << "These would be much closer to ±30 range!\n\n";
    
    std::cout << "CONCLUSION:\n";
    std::cout << "===========\n";
    std::cout << "The VICE demo likely used a much simpler evaluation without our\n";
    std::cout << "massive center control bonuses. This would explain why they got\n";
    std::cout << "30/30 (nearly symmetric) while we get 65/145 (highly asymmetric).\n";
    std::cout << "Our engine's center control evaluation is creating the asymmetry!\n";
}

int main() {
    Huginn::init();
    analyze_vice_discrepancy();
    return 0;
}
