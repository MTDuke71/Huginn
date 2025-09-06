/**
 * Piece-Square Table Asymmetry Investigation
 * 
 * This demo investigates the specific pieces causing the piece-square table asymmetry.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

void investigate_pst_asymmetry() {
    using namespace Huginn;
    
    std::cout << "Piece-Square Table Asymmetry Investigation\n";
    std::cout << "==========================================\n\n";
    
    // King's Gambit positions
    std::string original_fen = "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2";
    std::string mirrored_fen = "rnbqkbnr/pppp2pp/8/4pp2/4P3/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 2";
    
    Position original_pos, mirrored_pos;
    original_pos.set_from_fen(original_fen);
    mirrored_pos.set_from_fen(mirrored_fen);
    
    std::cout << "Original Position Analysis:\n";
    std::cout << "===========================\n";
    int original_pst_total = 0;
    
    for (int sq = 21; sq <= 98; ++sq) {
        if (original_pos.board[sq] == Piece::Offboard || original_pos.board[sq] == Piece::None) continue;
        
        Piece piece = original_pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue;
        
        int pst_sq64 = sq64;
        if (piece_color == Color::Black) {
            pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8); // Mirror for black pieces
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
            original_pst_total += pst_value;
        } else {
            original_pst_total -= pst_value;
        }
        
        // Convert square64 back to algebraic for display
        int display_file = sq64 % 8;
        int display_rank = sq64 / 8;
        char square_name[3];
        square_name[0] = 'a' + display_file;
        square_name[1] = '1' + display_rank;
        square_name[2] = '\0';
        
        char piece_char = '?';
        switch (piece) {
            case Piece::WhitePawn:   piece_char = 'P'; break;
            case Piece::WhiteKnight: piece_char = 'N'; break;
            case Piece::WhiteBishop: piece_char = 'B'; break;
            case Piece::WhiteRook:   piece_char = 'R'; break;
            case Piece::WhiteQueen:  piece_char = 'Q'; break;
            case Piece::WhiteKing:   piece_char = 'K'; break;
            case Piece::BlackPawn:   piece_char = 'p'; break;
            case Piece::BlackKnight: piece_char = 'n'; break;
            case Piece::BlackBishop: piece_char = 'b'; break;
            case Piece::BlackRook:   piece_char = 'r'; break;
            case Piece::BlackQueen:  piece_char = 'q'; break;
            case Piece::BlackKing:   piece_char = 'k'; break;
            default: piece_char = '?'; break;
        }
        
        int contribution = (piece_color == Color::White) ? pst_value : -pst_value;
        if (contribution != 0) {  // Only show pieces that contribute to PST score
            std::cout << piece_char << " on " << square_name 
                      << " (sq64=" << sq64 << ", pst_sq64=" << pst_sq64 
                      << ") -> " << std::setw(4) << contribution << std::endl;
        }
    }
    
    std::cout << "\nOriginal PST Total: " << original_pst_total << "\n\n";
    
    std::cout << "Mirrored Position Analysis:\n";
    std::cout << "===========================\n";
    int mirrored_pst_total = 0;
    
    for (int sq = 21; sq <= 98; ++sq) {
        if (mirrored_pos.board[sq] == Piece::Offboard || mirrored_pos.board[sq] == Piece::None) continue;
        
        Piece piece = mirrored_pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue;
        
        int pst_sq64 = sq64;
        if (piece_color == Color::Black) {
            pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8); // Mirror for black pieces
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
            mirrored_pst_total += pst_value;
        } else {
            mirrored_pst_total -= pst_value;
        }
        
        // Convert square64 back to algebraic for display
        int display_file = sq64 % 8;
        int display_rank = sq64 / 8;
        char square_name[3];
        square_name[0] = 'a' + display_file;
        square_name[1] = '1' + display_rank;
        square_name[2] = '\0';
        
        char piece_char = '?';
        switch (piece) {
            case Piece::WhitePawn:   piece_char = 'P'; break;
            case Piece::WhiteKnight: piece_char = 'N'; break;
            case Piece::WhiteBishop: piece_char = 'B'; break;
            case Piece::WhiteRook:   piece_char = 'R'; break;
            case Piece::WhiteQueen:  piece_char = 'Q'; break;
            case Piece::WhiteKing:   piece_char = 'K'; break;
            case Piece::BlackPawn:   piece_char = 'p'; break;
            case Piece::BlackKnight: piece_char = 'n'; break;
            case Piece::BlackBishop: piece_char = 'b'; break;
            case Piece::BlackRook:   piece_char = 'r'; break;
            case Piece::BlackQueen:  piece_char = 'q'; break;
            case Piece::BlackKing:   piece_char = 'k'; break;
            default: piece_char = '?'; break;
        }
        
        int contribution = (piece_color == Color::White) ? pst_value : -pst_value;
        if (contribution != 0) {  // Only show pieces that contribute to PST score
            std::cout << piece_char << " on " << square_name 
                      << " (sq64=" << sq64 << ", pst_sq64=" << pst_sq64 
                      << ") -> " << std::setw(4) << contribution << std::endl;
        }
    }
    
    std::cout << "\nMirrored PST Total: " << mirrored_pst_total << "\n\n";
    
    std::cout << "Difference: " << (original_pst_total - mirrored_pst_total) << "\n";
    
    if (original_pst_total != mirrored_pst_total) {
        std::cout << "\n❌ ASYMMETRY CONFIRMED in piece-square tables!\n";
        std::cout << "This suggests the piece-square table application logic has a bug.\n";
    } else {
        std::cout << "\n✓ Piece-square tables are symmetric.\n";
    }
}

int main() {
    Huginn::init();
    investigate_pst_asymmetry();
    return 0;
}
