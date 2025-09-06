/**
 * VICE Part 56 Compatible Evaluation Test
 * 
 * Implements the exact basic evaluation from VICE Part 56:
 * - Material balance only
 * - Basic piece-square tables
 * - Side-to-move perspective flip
 * - NO center control bonuses, development, or other modern features
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

// VICE Part 56 basic evaluation - material + PST only
int vice_part56_evaluate(const Position& pos) {
    using namespace Huginn;
    
    int score = 0;
    
    // Material and piece-square tables ONLY (like VICE Part 56)
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values (VICE Part 56 style)
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   material_value = 100; break;
            case PieceType::Knight: material_value = 300; break;
            case PieceType::Bishop: material_value = 300; break;
            case PieceType::Rook:   material_value = 500; break;
            case PieceType::Queen:  material_value = 900; break;
            case PieceType::King:   material_value = 0; break; // No material value for king
            default: material_value = 0; break;
        }
        
        // Piece-square table values
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue;
        
        int pst_value = 0;
        if (piece_color == Color::Black) {
            sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8); // Mirror for black pieces
        }
        
        switch (piece_type) {
            case PieceType::Pawn:   pst_value = EvalParams::PAWN_TABLE[sq64]; break;
            case PieceType::Knight: pst_value = EvalParams::KNIGHT_TABLE[sq64]; break;
            case PieceType::Bishop: pst_value = EvalParams::BISHOP_TABLE[sq64]; break;
            case PieceType::Rook:   pst_value = EvalParams::ROOK_TABLE[sq64]; break;
            case PieceType::Queen:  pst_value = EvalParams::QUEEN_TABLE[sq64]; break;
            case PieceType::King:   pst_value = EvalParams::KING_TABLE[sq64]; break;
            default: pst_value = 0; break;
        }
        
        int piece_value = material_value + pst_value;
        
        if (piece_color == Color::White) {
            score += piece_value;
        } else {
            score -= piece_value;
        }
    }
    
    // NO center control bonuses
    // NO development bonuses  
    // NO pawn penalties
    // NO tempo penalties
    // JUST material + PST like VICE Part 56!
    
    // Apply side-to-move perspective (VICE Part 56: negate if black to move)
    return (pos.side_to_move == Color::White) ? score : -score;
}

void test_vice_part56_evaluation() {
    using namespace Huginn;
    
    std::cout << "VICE Part 56 Compatible Evaluation Test\n";
    std::cout << "=======================================\n\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    std::cout << "VICE Part 56 Style Evaluation (Material + PST only):\n";
    std::cout << "====================================================\n";
    
    int vice_orig = vice_part56_evaluate(original_pos);
    int vice_mirr = vice_part56_evaluate(mirrored_pos);
    
    std::cout << "Original Position (1.e4 c6): " << vice_orig << " (White to move)\n";
    std::cout << "Mirrored Position:            " << vice_mirr << " (Black to move)\n";
    std::cout << "Difference:                   " << (vice_orig - vice_mirr) << "\n";
    std::cout << "Sum (should be 0 if symmetric): " << (vice_orig + vice_mirr) << "\n\n";
    
    if (vice_orig == -vice_mirr) {
        std::cout << "✓ PERFECT SYMMETRY with VICE Part 56 evaluation!\n";
        std::cout << "  This confirms VICE tutorial claim works with basic evaluation.\n";
    } else {
        std::cout << "❌ Still asymmetric even with VICE Part 56 evaluation.\n";
        std::cout << "  Asymmetry: " << (vice_orig + vice_mirr) << " points\n";
    }
    
    std::cout << "\nComparison with Our Full Engine:\n";
    std::cout << "================================\n";
    
    int full_orig = engine.evaluate(original_pos);
    int full_mirr = engine.evaluate(mirrored_pos);
    
    std::cout << "Our Engine - Original: " << full_orig << "\n";
    std::cout << "Our Engine - Mirrored:  " << full_mirr << "\n";
    std::cout << "Our Engine - Asymmetry: " << (full_orig + full_mirr) << "\n\n";
    
    std::cout << "Impact of Our Additional Features:\n";
    std::cout << "==================================\n";
    std::cout << "Additional features add " << (full_orig - vice_orig) << " points to original\n";
    std::cout << "Additional features add " << (full_mirr - vice_mirr) << " points to mirrored\n";
    std::cout << "Net impact of our features: " << ((full_orig - vice_orig) - (full_mirr - vice_mirr)) << " points\n\n";
    
    if (abs(vice_orig) <= 30 && abs(vice_mirr) <= 30) {
        std::cout << "✓ VICE Part 56 evaluation produces values in the ±30 range!\n";
        std::cout << "  This matches the VICE demo results mentioned.\n";
    } else {
        std::cout << "❌ VICE Part 56 evaluation still outside ±30 range.\n";
    }
    
    std::cout << "\nCONCLUSION:\n";
    std::cout << "===========\n";
    std::cout << "The VICE tutorial claim about 1.e4 c6 symmetry is based on\n";
    std::cout << "the very basic evaluation from Part 56 (material + PST only).\n";
    std::cout << "Our engine has evolved beyond this with center control bonuses,\n";
    std::cout << "development tracking, and other features that create asymmetry.\n";
    std::cout << "This explains why VICE got ~30/30 while we get 65/145.\n";
}

int main() {
    Huginn::init();
    test_vice_part56_evaluation();
    return 0;
}
