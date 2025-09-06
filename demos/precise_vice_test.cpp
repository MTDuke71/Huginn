/**
 * Precise VICE Test - Exact Engine Logic Replication
 * 
 * Replicates the exact evaluation logic from MinimalEngine::evaluate()
 * but allows us to toggle center control on/off to isolate its impact.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

int evaluate_with_center_control_toggle(const Position& pos, bool include_center_control = true) {
    using namespace Huginn;
    
    int score = 0;
    
    // Material and piece-square tables
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values
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
        
        // Piece-square tables
        int sq64 = MAILBOX_MAPS.to64[sq];
        int pst_value = 0;
        if (sq64 >= 0) {
            int pst_sq64 = sq64;
            if (piece_color == Color::Black) {
                pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
            }
            
            switch (piece_type) {
                case PieceType::Pawn:   pst_value = Huginn::EvalParams::PAWN_TABLE[pst_sq64]; break;
                case PieceType::Knight: pst_value = Huginn::EvalParams::KNIGHT_TABLE[pst_sq64]; break;
                case PieceType::Bishop: pst_value = Huginn::EvalParams::BISHOP_TABLE[pst_sq64]; break;
                case PieceType::Rook:   pst_value = Huginn::EvalParams::ROOK_TABLE[pst_sq64]; break;
                case PieceType::Queen:  pst_value = Huginn::EvalParams::QUEEN_TABLE[pst_sq64]; break;
                case PieceType::King:   pst_value = Huginn::EvalParams::KING_TABLE[pst_sq64]; break;
                default: pst_value = 0; break;
            }
        }
        
        int total_piece_value = piece_value + pst_value;
        
        if (piece_color == Color::White) {
            score += total_piece_value;
        } else {
            score -= total_piece_value;
        }
    }
    
    // Center control (toggleable) - CORRECT VALUES FROM ENGINE
    if (include_center_control) {
        // Direct indices: e4=54, d4=53, e5=64, d5=65, e3=44, d3=43, e6=74, d6=75
        // Convert to 64-square: e4=28, d4=27, e5=36, d5=35, e3=20, d3=19, e6=44, d6=43
        
        // MASSIVE bonuses for 4th rank center pawns (100 points each)
        int sq_e4 = MAILBOX_MAPS.to120[28]; // e4 in 64-square = 28
        int sq_d4 = MAILBOX_MAPS.to120[27]; // d4 in 64-square = 27  
        int sq_e5 = MAILBOX_MAPS.to120[36]; // e5 in 64-square = 36
        int sq_d5 = MAILBOX_MAPS.to120[35]; // d5 in 64-square = 35
        
        if (sq_e4 >= 0 && pos.board[sq_e4] == Piece::WhitePawn) score += 100; // e4
        if (sq_d4 >= 0 && pos.board[sq_d4] == Piece::WhitePawn) score += 100; // d4
        if (sq_e5 >= 0 && pos.board[sq_e5] == Piece::BlackPawn) score -= 100; // e5  
        if (sq_d5 >= 0 && pos.board[sq_d5] == Piece::BlackPawn) score -= 100; // d5
        
        // Small bonus for supporting center from 3rd rank (20 points each)
        int sq_e3 = MAILBOX_MAPS.to120[20]; // e3 in 64-square = 20
        int sq_d3 = MAILBOX_MAPS.to120[19]; // d3 in 64-square = 19
        int sq_e6 = MAILBOX_MAPS.to120[44]; // e6 in 64-square = 44
        int sq_d6 = MAILBOX_MAPS.to120[43]; // d6 in 64-square = 43
        
        if (sq_e3 >= 0 && pos.board[sq_e3] == Piece::WhitePawn) score += 20; // e3
        if (sq_d3 >= 0 && pos.board[sq_d3] == Piece::WhitePawn) score += 20; // d3
        if (sq_e6 >= 0 && pos.board[sq_e6] == Piece::BlackPawn) score -= 20; // e6
        if (sq_d6 >= 0 && pos.board[sq_d6] == Piece::BlackPawn) score -= 20; // d6
    }
    
    // Apply side-to-move perspective (like the real engine)
    return (pos.side_to_move == Color::White) ? score : -score;
}

void precise_vice_test() {
    using namespace Huginn;
    
    std::cout << "Precise VICE Test - Center Control Toggle\n";
    std::cout << "==========================================\n\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    std::cout << "Original Position (1.e4 c6):\n";
    std::cout << "FEN: " << e4_c6_fen << "\n";
    std::cout << "Side to move: " << (original_pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    std::cout << "Mirrored Position:\n";
    std::cout << "FEN: " << mirrored_pos.to_fen() << "\n";
    std::cout << "Side to move: " << (mirrored_pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    // Test WITH center control
    int orig_with_center = evaluate_with_center_control_toggle(original_pos, true);
    int mirr_with_center = evaluate_with_center_control_toggle(mirrored_pos, true);
    
    std::cout << "WITH Center Control:\n";
    std::cout << "====================\n";
    std::cout << "Original evaluation: " << std::setw(4) << orig_with_center << "\n";
    std::cout << "Mirrored evaluation: " << std::setw(4) << mirr_with_center << "\n";
    std::cout << "Difference:          " << std::setw(4) << (orig_with_center - mirr_with_center) << "\n";
    std::cout << "Sum (should be 0):   " << std::setw(4) << (orig_with_center + mirr_with_center) << "\n";
    std::cout << "Symmetry: " << (orig_with_center == -mirr_with_center ? "✓ PERFECT" : "❌ FAILED") << "\n\n";
    
    // Test WITHOUT center control
    int orig_without_center = evaluate_with_center_control_toggle(original_pos, false);
    int mirr_without_center = evaluate_with_center_control_toggle(mirrored_pos, false);
    
    std::cout << "WITHOUT Center Control:\n";
    std::cout << "=======================\n";
    std::cout << "Original evaluation: " << std::setw(4) << orig_without_center << "\n";
    std::cout << "Mirrored evaluation: " << std::setw(4) << mirr_without_center << "\n";
    std::cout << "Difference:          " << std::setw(4) << (orig_without_center - mirr_without_center) << "\n";
    std::cout << "Sum (should be 0):   " << std::setw(4) << (orig_without_center + mirr_without_center) << "\n";
    std::cout << "Symmetry: " << (orig_without_center == -mirr_without_center ? "✓ PERFECT" : "❌ FAILED") << "\n\n";
    
    // Calculate center control impact
    int center_impact_orig = orig_with_center - orig_without_center;
    int center_impact_mirr = mirr_with_center - mirr_without_center;
    
    std::cout << "Center Control Impact Analysis:\n";
    std::cout << "===============================\n";
    std::cout << "Impact on original: " << center_impact_orig << " points\n";
    std::cout << "Impact on mirrored: " << center_impact_mirr << " points\n";
    std::cout << "Impact difference:  " << (center_impact_orig - center_impact_mirr) << " points\n\n";
    
    // Verify against actual engine
    int engine_orig = engine.evaluate(original_pos);
    int engine_mirr = engine.evaluate(mirrored_pos);
    
    std::cout << "Verification Against Actual Engine:\n";
    std::cout << "===================================\n";
    std::cout << "Engine original: " << engine_orig << " vs Our with center: " << orig_with_center << "\n";
    std::cout << "Engine mirrored: " << engine_mirr << " vs Our with center: " << mirr_with_center << "\n";
    std::cout << "Match: " << ((engine_orig == orig_with_center && engine_mirr == mirr_with_center) ? "✓ PERFECT" : "❌ MISMATCH") << "\n";
}

int main() {
    Huginn::init();
    precise_vice_test();
    return 0;
}
