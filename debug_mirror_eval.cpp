// Debug mirror evaluation issues with VICE Part 82
#include <iostream>
#include "src/position.hpp"
#include "src/minimal_search.hpp"

using namespace Huginn;

void debug_material_counts(const Position& pos, const std::string& label) {
    std::cout << "\n" << label << " material analysis:" << std::endl;
    
    // Calculate material manually to match evaluation logic
    int white_material = 0;
    int black_material = 0;
    
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        int material_value = 0;
        switch (piece_type) {
            case PieceType::Pawn:   material_value = 100; break;
            case PieceType::Knight: material_value = 320; break;
            case PieceType::Bishop: material_value = 330; break;
            case PieceType::Rook:   material_value = 500; break;
            case PieceType::Queen:  material_value = 900; break;
            case PieceType::King:   material_value = 0; break;  // Excluded from material count
            default: material_value = 0; break;
        }
        
        if (piece_color == Color::White) {
            white_material += material_value;
        } else {
            black_material += material_value;
        }
    }
    
    int total_material = white_material + black_material;
    bool is_endgame = (total_material <= 1300 * 2);  // Our current logic
    
    std::cout << "  White material: " << white_material << " cp" << std::endl;
    std::cout << "  Black material: " << black_material << " cp" << std::endl;
    std::cout << "  Total material: " << total_material << " cp" << std::endl;
    std::cout << "  Endgame threshold: " << (1300 * 2) << " cp" << std::endl;
    std::cout << "  Is endgame: " << (is_endgame ? "YES" : "NO") << std::endl;
    std::cout << "  Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
}

int main() {
    std::cout << "=== Debug Mirror Evaluation Issues ===" << std::endl;
    
    MinimalEngine engine;
    
    // Test the exact failing position from mirror test
    Position original;
    original.set_from_fen("1qr3k1/p2nbppp/bp2p3/3p4/3P4/1P2PNP1/P2Q1PBP/1N2R1K1 b - - 0 1");
    
    Position mirrored = engine.mirrorBoard(original);
    
    debug_material_counts(original, "Original position");
    debug_material_counts(mirrored, "Mirrored position");
    
    int eval1 = engine.evaluate(original);
    int eval2 = engine.evaluate(mirrored);
    
    std::cout << "\nEvaluations:" << std::endl;
    std::cout << "  Original: " << eval1 << " cp (from " << (original.side_to_move == Color::White ? "White" : "Black") << "'s perspective)" << std::endl;
    std::cout << "  Mirrored: " << eval2 << " cp (from " << (mirrored.side_to_move == Color::White ? "White" : "Black") << "'s perspective)" << std::endl;
    
    // For proper mirror comparison, we need to compare from the same perspective
    // Since original is Black to move and mirrored is White to move, 
    // we need to negate one of them to compare from the same perspective
    int eval2_black_perspective = -eval2;  // Convert white perspective to black perspective
    
    std::cout << "\nCorrected comparison (both from Black's perspective):" << std::endl;
    std::cout << "  Original: " << eval1 << " cp" << std::endl;
    std::cout << "  Mirrored: " << eval2_black_perspective << " cp" << std::endl;
    std::cout << "  Difference: " << (eval1 - eval2_black_perspective) << " cp" << std::endl;
    
    // Test our fixed MirrorAvailTest function
    std::cout << "\nUsing fixed MirrorAvailTest function:" << std::endl;
    engine.MirrorAvailTest(original);
    
    return 0;
}
