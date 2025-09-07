#include <iostream>
#include <string>
#include <iomanip>
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"

using namespace Huginn;

void analyze_position_details(const Position& pos, const std::string& label) {
    std::cout << "\n=== " << label << " ===" << std::endl;
    std::cout << "FEN: " << pos.to_fen() << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Calculate total material first
    int white_material = 0;
    int black_material = 0;
    
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        if (piece_type == PieceType::King) continue;
        
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   material_value = 100; break;
            case PieceType::Knight: material_value = 320; break;
            case PieceType::Bishop: material_value = 330; break;
            case PieceType::Rook:   material_value = 500; break;
            case PieceType::Queen:  material_value = 900; break;
            default: material_value = 0; break;
        }
        
        if (piece_color == Color::White) {
            white_material += material_value;
        } else {
            black_material += material_value;
        }
    }
    
    int total_material = white_material + black_material;
    bool is_endgame = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD);
    
    std::cout << "White material: " << white_material << " cp" << std::endl;
    std::cout << "Black material: " << black_material << " cp" << std::endl;
    std::cout << "Total material: " << total_material << " cp" << std::endl;
    std::cout << "Endgame threshold: " << EvalParams::ENDGAME_MATERIAL_THRESHOLD << " cp" << std::endl;
    std::cout << "Is endgame: " << (is_endgame ? "YES" : "NO") << std::endl;
    std::cout << "King table used: " << (is_endgame ? "ENDGAME" : "OPENING") << std::endl;
    
    // Find and analyze kings
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        if (type_of(piece) != PieceType::King) continue;
        
        Color piece_color = color_of(piece);
        int sq64 = MAILBOX_MAPS.to64[sq];
        int table_index = (piece_color == Color::Black) ? mirror_square_64(sq64) : sq64;
        
        int pst_value;
        if (is_endgame) {
            pst_value = EvalParams::KING_TABLE_ENDGAME[table_index];
        } else {
            pst_value = EvalParams::KING_TABLE[table_index];
        }
        
        std::string sq_name = "";
        int file = (sq % 10) - 1;
        int rank = (sq / 10) - 2;
        sq_name += char('a' + file);
        sq_name += char('1' + rank);
        
        std::cout << (piece_color == Color::White ? "White" : "Black") << " king on " << sq_name 
                  << " (sq64=" << sq64 << ", table_index=" << table_index << "): " << pst_value << " cp" << std::endl;
    }
    
    MinimalEngine engine;
    int eval = engine.evaluate(pos);
    std::cout << "Final evaluation: " << eval << " cp" << std::endl;
}

int main() {
    // The failing position from debug_mirror_eval
    std::string fen1 = "1qr3k1/p2nbppp/bp2p3/3p4/3P4/1P2PNP1/P2Q1PBP/1N2R1K1 b - - 0 1";
    std::string fen2 = "1n2r1k1/p2q1pbp/1p2pnp1/3p4/3P4/BP2P3/P2NBPPP/1QR3K1 w - - 0 1";
    
    Position pos1, pos2;
    pos1.set_from_fen(fen1);
    pos2.set_from_fen(fen2);
    
    analyze_position_details(pos1, "POSITION 1 (Original)");
    analyze_position_details(pos2, "POSITION 2 (Mirrored)");
    
    std::cout << "\n=== COMPARISON ===" << std::endl;
    MinimalEngine engine;
    int eval1 = engine.evaluate(pos1);
    int eval2 = engine.evaluate(pos2);
    
    std::cout << "Position 1 eval: " << eval1 << " cp (Black to move)" << std::endl;
    std::cout << "Position 2 eval: " << eval2 << " cp (White to move)" << std::endl;
    std::cout << "Both from Black's perspective: " << eval1 << " vs " << (-eval2) << std::endl;
    std::cout << "Difference: " << (eval1 - (-eval2)) << " cp" << std::endl;
    
    return 0;
}
