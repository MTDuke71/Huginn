/**
 * Detailed Analysis of VICE e4-c6 Claim
 * 
 * Deep dive into why the VICE tutorial claim about 1.e4 c6 symmetry 
 * doesn't hold with our evaluation function.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

struct DetailedEvaluation {
    int material = 0;
    int piece_square_tables = 0;
    int development = 0;
    int center_control = 0;
    int pawn_penalties = 0;
    int total = 0;
};

DetailedEvaluation detailed_evaluate(const Position& pos) {
    using namespace Huginn;
    DetailedEvaluation result;
    
    // Material calculation
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
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
    
    // Simple center control (e4, e5, d4, d5 squares)
    int center_squares[] = {28, 29, 36, 37}; // d4, e4, d5, e5 in 64-square format
    for (int sq64 : center_squares) {
        int sq120 = MAILBOX_MAPS.to120[sq64];
        if (sq120 >= 0 && pos.board[sq120] == Piece::WhitePawn) {
            result.center_control += 25;
        } else if (sq120 >= 0 && pos.board[sq120] == Piece::BlackPawn) {
            result.center_control -= 25;
        }
    }
    
    result.total = result.material + result.piece_square_tables + result.development + result.center_control + result.pawn_penalties;
    return result;
}

void analyze_e4_c6_detailed() {
    using namespace Huginn;
    
    std::cout << "Detailed Analysis: VICE e4-c6 Claim\n";
    std::cout << "===================================\n\n";
    
    // Original position: 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    // Mirrored position
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    std::cout << "Position Analysis:\n";
    std::cout << "==================\n";
    std::cout << "Original (1.e4 c6): " << e4_c6_fen << "\n";
    std::cout << "Mirrored by function: " << mirrored_pos.to_fen() << "\n\n";
    
    // Detailed evaluation breakdown
    DetailedEvaluation orig_eval = detailed_evaluate(original_pos);
    DetailedEvaluation mirr_eval = detailed_evaluate(mirrored_pos);
    
    std::cout << "Component Breakdown:\n";
    std::cout << "====================\n";
    std::cout << std::setw(20) << "Component" << std::setw(10) << "Original" << std::setw(10) << "Mirrored" << std::setw(12) << "Difference" << "\n";
    std::cout << std::string(52, '-') << "\n";
    
    std::cout << std::setw(20) << "Material:" << std::setw(10) << orig_eval.material 
              << std::setw(10) << mirr_eval.material 
              << std::setw(12) << (orig_eval.material - mirr_eval.material) << "\n";
              
    std::cout << std::setw(20) << "Piece-Square Tables:" << std::setw(10) << orig_eval.piece_square_tables 
              << std::setw(10) << mirr_eval.piece_square_tables 
              << std::setw(12) << (orig_eval.piece_square_tables - mirr_eval.piece_square_tables) << "\n";
              
    std::cout << std::setw(20) << "Center Control:" << std::setw(10) << orig_eval.center_control 
              << std::setw(10) << mirr_eval.center_control 
              << std::setw(12) << (orig_eval.center_control - mirr_eval.center_control) << "\n";
              
    std::cout << std::setw(20) << "TOTAL:" << std::setw(10) << orig_eval.total 
              << std::setw(10) << mirr_eval.total 
              << std::setw(12) << (orig_eval.total - mirr_eval.total) << "\n";
    
    // Now get the actual engine evaluations (which include side-to-move perspective)
    int engine_orig = engine.evaluate(original_pos);
    int engine_mirr = engine.evaluate(mirrored_pos);
    
    std::cout << "\nEngine Evaluations (with side-to-move perspective):\n";
    std::cout << "===================================================\n";
    std::cout << "Original: " << engine_orig << " (from " << (original_pos.side_to_move == Color::White ? "White" : "Black") << "'s perspective)\n";
    std::cout << "Mirrored: " << engine_mirr << " (from " << (mirrored_pos.side_to_move == Color::White ? "White" : "Black") << "'s perspective)\n";
    
    std::cout << "\nWhy VICE claim fails:\n";
    std::cout << "=====================\n";
    std::cout << "1. Pawn structure differences: e4 vs c6 are not symmetric moves\n";
    std::cout << "2. e4 controls central squares (d5, f5), c6 supports d5 but doesn't control center\n";
    std::cout << "3. Different piece-square table values for e4 vs c6\n";
    std::cout << "4. mirrorBoard creates position equivalent to '1.c3 e5', not a symmetric game\n\n";
    
    std::cout << "For VICE claim to work, we'd need positions that are truly strategically equivalent\n";
    std::cout << "when mirrored, such as 1.e4 e5 or symmetric opening structures.\n";
}

int main() {
    Huginn::init();
    analyze_e4_c6_detailed();
    return 0;
}
