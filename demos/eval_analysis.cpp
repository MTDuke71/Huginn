/**
 * Detailed Evaluation Analysis
 * 
 * This demo breaks down the evaluation function component by component
 * to identify exactly where the remaining asymmetry comes from.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

// Custom evaluation function that returns components separately
struct EvalBreakdown {
    int material = 0;
    int piece_square_tables = 0;
    int development = 0;
    int center_control = 0;
    int pawn_penalties = 0;
    int pawn_structure = 0;
    int total = 0;
};

EvalBreakdown detailed_evaluate(const Position& pos) {
    EvalBreakdown result;
    
    // Material and piece-square tables
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard) continue;
        
        Piece piece = pos.board[sq];
        if (piece == Piece::None) continue;
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Material values
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   material_value = 100; break;
            case PieceType::Knight: material_value = 300; break;
            case PieceType::Bishop: material_value = 300; break;
            case PieceType::Rook:   material_value = 500; break;
            case PieceType::Queen:  material_value = 900; break;
            case PieceType::King:   material_value = 0; break;
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
            case PieceType::Pawn:   pst_value = Huginn::EvalParams::PAWN_TABLE[sq64]; break;
            case PieceType::Knight: pst_value = Huginn::EvalParams::KNIGHT_TABLE[sq64]; break;
            case PieceType::Bishop: pst_value = Huginn::EvalParams::BISHOP_TABLE[sq64]; break;
            case PieceType::Rook:   pst_value = Huginn::EvalParams::ROOK_TABLE[sq64]; break;
            case PieceType::Queen:  pst_value = Huginn::EvalParams::QUEEN_TABLE[sq64]; break;
            case PieceType::King:   pst_value = Huginn::EvalParams::KING_TABLE[sq64]; break;
            default: pst_value = 0; break;
        }
        
        if (piece_color == Color::White) {
            result.material += material_value;
            result.piece_square_tables += pst_value;
        } else {
            result.material -= material_value;
            result.piece_square_tables -= pst_value;
        }
    }
    
    // Development bonus
    int white_development = 0;
    int black_development = 0;
    
    // Check if knights are developed
    if (pos.board[22] != Piece::WhiteKnight) white_development += 50; // b1
    if (pos.board[27] != Piece::WhiteKnight) white_development += 50; // g1
    if (pos.board[92] != Piece::BlackKnight) black_development += 50; // b8
    if (pos.board[97] != Piece::BlackKnight) black_development += 50; // g8
    
    // Check if bishops are developed
    if (pos.board[23] != Piece::WhiteBishop) white_development += 30; // c1
    if (pos.board[26] != Piece::WhiteBishop) white_development += 30; // f1
    if (pos.board[93] != Piece::BlackBishop) black_development += 30; // c8
    if (pos.board[96] != Piece::BlackBishop) black_development += 30; // f8
    
    // Bonus for castling
    if (pos.board[25] != Piece::WhiteKing) white_development += 40; // e1
    if (pos.board[95] != Piece::BlackKing) black_development += 40; // e8
    
    result.development = white_development - black_development;
    
    // Center control bonus
    int center_bonus = 0;
    
    if (pos.board[54] == Piece::WhitePawn) center_bonus += 100; // e4
    if (pos.board[53] == Piece::WhitePawn) center_bonus += 100; // d4
    if (pos.board[64] == Piece::BlackPawn) center_bonus -= 100; // e5  
    if (pos.board[65] == Piece::BlackPawn) center_bonus -= 100; // d5
    
    if (pos.board[44] == Piece::WhitePawn) center_bonus += 20; // e3
    if (pos.board[43] == Piece::WhitePawn) center_bonus += 20; // d3
    if (pos.board[74] == Piece::BlackPawn) center_bonus -= 20; // e6
    if (pos.board[75] == Piece::BlackPawn) center_bonus -= 20; // d6
    
    result.center_control = center_bonus;
    
    // Pawn penalties
    int white_pawn_moves = 0;
    int black_pawn_moves = 0;
    
    for (int sq = 31; sq <= 38; ++sq) {
        if (pos.board[sq] != Piece::WhitePawn) {
            white_pawn_moves++;
        }
    }
    
    for (int sq = 81; sq <= 88; ++sq) {
        if (pos.board[sq] != Piece::BlackPawn) {
            black_pawn_moves++;
        }
    }
    
    int pawn_penalty = 0;
    if (white_pawn_moves > 2) {
        int excess = white_pawn_moves - 2;
        pawn_penalty += excess * 50 + (excess * excess * 25);
    }
    if (black_pawn_moves > 2) {
        int excess = black_pawn_moves - 2;
        pawn_penalty -= excess * 50 + (excess * excess * 25);
    }
    
    int tempo_penalty = 0;
    if (white_pawn_moves >= 4 && white_development < 50) {
        tempo_penalty += 200;
    }
    if (black_pawn_moves >= 4 && black_development < 50) {
        tempo_penalty -= 200;
    }
    
    result.pawn_penalties = -(pawn_penalty + tempo_penalty);
    
    // Pawn structure (simplified for this analysis)
    result.pawn_structure = 0; // Skip for now
    
    result.total = result.material + result.piece_square_tables + result.development + 
                   result.center_control + result.pawn_penalties + result.pawn_structure;
    
    return result;
}

void analyze_positions() {
    using namespace Huginn;
    
    std::cout << "Detailed Evaluation Component Analysis\n";
    std::cout << "======================================\n\n";
    
    // King's Gambit positions
    std::string original_fen = "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2";
    std::string mirrored_fen = "rnbqkbnr/pppp2pp/8/4pp2/4P3/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 2";
    
    Position original_pos, mirrored_pos;
    original_pos.set_from_fen(original_fen);
    mirrored_pos.set_from_fen(mirrored_fen);
    
    EvalBreakdown original_eval = detailed_evaluate(original_pos);
    EvalBreakdown mirrored_eval = detailed_evaluate(mirrored_pos);
    
    std::cout << "                      Original    Mirrored    Difference\n";
    std::cout << "                      --------    --------    ----------\n";
    std::cout << "Material:             " << std::setw(8) << original_eval.material 
              << std::setw(12) << mirrored_eval.material 
              << std::setw(12) << (original_eval.material - mirrored_eval.material) << "\n";
    std::cout << "Piece-Square Tables:  " << std::setw(8) << original_eval.piece_square_tables 
              << std::setw(12) << mirrored_eval.piece_square_tables 
              << std::setw(12) << (original_eval.piece_square_tables - mirrored_eval.piece_square_tables) << "\n";
    std::cout << "Development:          " << std::setw(8) << original_eval.development 
              << std::setw(12) << mirrored_eval.development 
              << std::setw(12) << (original_eval.development - mirrored_eval.development) << "\n";
    std::cout << "Center Control:       " << std::setw(8) << original_eval.center_control 
              << std::setw(12) << mirrored_eval.center_control 
              << std::setw(12) << (original_eval.center_control - mirrored_eval.center_control) << "\n";
    std::cout << "Pawn Penalties:       " << std::setw(8) << original_eval.pawn_penalties 
              << std::setw(12) << mirrored_eval.pawn_penalties 
              << std::setw(12) << (original_eval.pawn_penalties - mirrored_eval.pawn_penalties) << "\n";
    std::cout << "Pawn Structure:       " << std::setw(8) << original_eval.pawn_structure 
              << std::setw(12) << mirrored_eval.pawn_structure 
              << std::setw(12) << (original_eval.pawn_structure - mirrored_eval.pawn_structure) << "\n";
    std::cout << "                      --------    --------    ----------\n";
    std::cout << "TOTAL:                " << std::setw(8) << original_eval.total 
              << std::setw(12) << mirrored_eval.total 
              << std::setw(12) << (original_eval.total - mirrored_eval.total) << "\n\n";
    
    std::cout << "Note: These totals should be identical for perfectly mirrored positions.\n";
    std::cout << "Any non-zero difference indicates an asymmetry in the evaluation function.\n";
}

int main() {
    Huginn::init();
    analyze_positions();
    return 0;
}
