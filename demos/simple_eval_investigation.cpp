#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "../src/position.hpp"
#include "../src/board120.hpp"
#include "../src/evaluation.hpp"

using namespace Huginn;

// Simple evaluation: Material + PST only
int simple_evaluate(const Position& pos) {
    int score = 0;
    
    // Material difference
    score = pos.get_material_score(Color::White) - pos.get_material_score(Color::Black);
    
    // Add piece-square table values
    for (int sq120 = 21; sq120 <= 98; ++sq120) {
        if ((sq120 % 10) < 1 || (sq120 % 10) > 8) continue; // Skip offboard
        
        Piece piece = pos.at(sq120);
        if (is_none(piece)) continue;
        
        Color color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // Convert to 64-board square for PST lookup
        int file = (sq120 % 10) - 1;
        int rank = (sq120 / 10) - 2;
        int sq64 = rank * 8 + file;
        
        // Get PST value (flip for black pieces)
        int pst_value = 0;
        int pst_sq = (color == Color::White) ? sq64 : (56 - rank) * 8 + file;
        
        switch (piece_type) {
            case PieceType::Pawn:
                pst_value = EvalParams::PAWN_TABLE[pst_sq];
                break;
            case PieceType::Knight:
                pst_value = EvalParams::KNIGHT_TABLE[pst_sq];
                break;
            case PieceType::Bishop:
                pst_value = EvalParams::BISHOP_TABLE[pst_sq];
                break;
            case PieceType::Rook:
                pst_value = EvalParams::ROOK_TABLE[pst_sq];
                break;
            case PieceType::Queen:
                pst_value = EvalParams::QUEEN_TABLE[pst_sq];
                break;
            case PieceType::King:
                pst_value = EvalParams::KING_MG_TABLE[pst_sq];
                break;
            default:
                break;
        }
        
        if (color == Color::White) {
            score += pst_value;
        } else {
            score -= pst_value;
        }
    }
    
    // Return from side-to-move perspective
    return (pos.side_to_move == Color::White) ? score : -score;
}

// Mirror board function (simplified)
Position mirror_position(const Position& pos) {
    // Create FEN for mirrored position
    std::string fen = pos.fen();
    
    // Parse current FEN
    std::istringstream fen_stream(fen);
    std::string board_part, side_part, castle_part, en_passant_part, halfmove_part, fullmove_part;
    fen_stream >> board_part >> side_part >> castle_part >> en_passant_part >> halfmove_part >> fullmove_part;
    
    // Mirror the board part
    std::vector<std::string> ranks;
    std::stringstream board_stream(board_part);
    std::string rank;
    
    while (std::getline(board_stream, rank, '/')) {
        ranks.push_back(rank);
    }
    
    // Reverse ranks and swap piece colors
    std::reverse(ranks.begin(), ranks.end());
    for (auto& rank : ranks) {
        for (auto& c : rank) {
            if (std::isalpha(c)) {
                c = std::islower(c) ? std::toupper(c) : std::tolower(c);
            }
        }
    }
    
    // Rebuild board part
    std::string mirrored_board = "";
    for (size_t i = 0; i < ranks.size(); ++i) {
        if (i > 0) mirrored_board += "/";
        mirrored_board += ranks[i];
    }
    
    // Flip side to move
    std::string mirrored_side = (side_part == "w") ? "b" : "w";
    
    // Mirror castling rights
    std::string mirrored_castle = "";
    for (char c : castle_part) {
        switch (c) {
            case 'K': mirrored_castle += 'k'; break;
            case 'Q': mirrored_castle += 'q'; break;
            case 'k': mirrored_castle += 'K'; break;
            case 'q': mirrored_castle += 'Q'; break;
            case '-': mirrored_castle += '-'; break;
        }
    }
    
    // Mirror en passant (if present)
    std::string mirrored_en_passant = "-";
    if (en_passant_part != "-") {
        char file = en_passant_part[0];
        char rank = en_passant_part[1];
        char mirrored_rank = (rank == '3') ? '6' : (rank == '6') ? '3' : rank;
        mirrored_en_passant = std::string(1, file) + mirrored_rank;
    }
    
    // Construct mirrored FEN
    std::string mirrored_fen = mirrored_board + " " + mirrored_side + " " + mirrored_castle + " " + 
                              mirrored_en_passant + " " + halfmove_part + " " + fullmove_part;
    
    return Position(mirrored_fen);
}

void analyze_pst_asymmetry(const std::string& fen) {
    Position pos(fen);
    Position mirrored = mirror_position(pos);
    
    std::cout << "=== Simple Evaluation Analysis (Material + PST only) ===" << std::endl;
    std::cout << "Position: " << fen << std::endl;
    std::cout << "Mirrored: " << mirrored.fen() << std::endl;
    std::cout << std::endl;
    
    int original_score = simple_evaluate(pos);
    int mirrored_score = simple_evaluate(mirrored);
    
    std::cout << "Original evaluation: " << original_score << " (" << 
                 (pos.side_to_move == Color::White ? "White" : "Black") << " to move)" << std::endl;
    std::cout << "Mirrored evaluation: " << mirrored_score << " (" << 
                 (mirrored.side_to_move == Color::White ? "White" : "Black") << " to move)" << std::endl;
    std::cout << "Sum (should be 0): " << (original_score + mirrored_score) << std::endl;
    std::cout << "Difference: " << abs(original_score - mirrored_score) << std::endl;
    std::cout << std::endl;
    
    // Detailed breakdown
    std::cout << "=== Detailed PST Analysis ===" << std::endl;
    
    // Analyze original position
    std::cout << "\nOriginal Position PST Values:" << std::endl;
    int total_white_pst = 0, total_black_pst = 0;
    
    for (Square sq = A1; sq <= H8; ++sq) {
        Piece piece = pos.piece_at(sq);
        if (piece == NO_PIECE) continue;
        
        Color color = piece_color(piece);
        PieceType piece_type = piece_type_of(piece);
        Square pst_sq = (color == Color::White) ? sq : flip_square(sq);
        
        int pst_value = 0;
        std::string piece_name;
        
        switch (piece_type) {
            case PieceType::Pawn:
                pst_value = EvalParams::PAWN_TABLE[pst_sq];
                piece_name = "Pawn";
                break;
            case PieceType::Knight:
                pst_value = EvalParams::KNIGHT_TABLE[pst_sq];
                piece_name = "Knight";
                break;
            case PieceType::Bishop:
                pst_value = EvalParams::BISHOP_TABLE[pst_sq];
                piece_name = "Bishop";
                break;
            case PieceType::Rook:
                pst_value = EvalParams::ROOK_TABLE[pst_sq];
                piece_name = "Rook";
                break;
            case PieceType::Queen:
                pst_value = EvalParams::QUEEN_TABLE[pst_sq];
                piece_name = "Queen";
                break;
            case PieceType::King:
                pst_value = EvalParams::KING_MG_TABLE[pst_sq];
                piece_name = "King";
                break;
            default:
                continue;
        }
        
        std::cout << (color == Color::White ? "White " : "Black ") << piece_name 
                  << " on " << square_to_string(sq) << " (PST sq: " << square_to_string(pst_sq) 
                  << "): " << pst_value << std::endl;
        
        if (color == Color::White) {
            total_white_pst += pst_value;
        } else {
            total_black_pst += pst_value;
        }
    }
    
    std::cout << "\nOriginal Position PST Summary:" << std::endl;
    std::cout << "White PST total: " << total_white_pst << std::endl;
    std::cout << "Black PST total: " << total_black_pst << std::endl;
    std::cout << "PST difference (W-B): " << (total_white_pst - total_black_pst) << std::endl;
    std::cout << "Material balance: " << pos.material_balance() << std::endl;
    std::cout << "Total score: " << (pos.material_balance() + total_white_pst - total_black_pst) << std::endl;
    
    if (original_score + mirrored_score != 0) {
        std::cout << "\n❌ ASYMMETRY DETECTED!" << std::endl;
        std::cout << "The evaluation is not symmetric. This suggests:" << std::endl;
        std::cout << "1. PST tables may not be perfectly symmetric" << std::endl;
        std::cout << "2. Mirror function may have bugs" << std::endl;
        std::cout << "3. Position representation differences" << std::endl;
    } else {
        std::cout << "\n✅ Perfect symmetry achieved!" << std::endl;
    }
}

int main() {
    try {
        std::cout << "Simple Evaluation Investigation" << std::endl;
        std::cout << "===============================" << std::endl;
        std::cout << "Testing material + PST evaluation only" << std::endl;
        std::cout << std::endl;
        
        // Test VICE e4-c6 position
        std::cout << "### VICE e4-c6 Test Position ###" << std::endl;
        analyze_pst_asymmetry("rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        
        // Test starting position
        std::cout << "\n### Starting Position ###" << std::endl;
        analyze_pst_asymmetry("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        
        // Test King's Gambit position
        std::cout << "\n### King's Gambit Position ###" << std::endl;
        analyze_pst_asymmetry("rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
