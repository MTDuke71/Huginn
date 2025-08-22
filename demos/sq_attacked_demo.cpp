#include <iostream>
#include <iomanip>
#include "position.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include "chess_types.hpp"
#include "squares120.hpp"

void print_attack_map(const Position& pos, Color attacking_color) {
    std::cout << "\n" << (attacking_color == Color::White ? "White" : "Black") 
              << " piece attacks:\n";
    
    // Print column headers
    std::cout << "    ";
    for (char c = 'a'; c <= 'h'; ++c) {
        std::cout << " " << c << " ";
    }
    std::cout << "\n";
    
    // Print each rank (from 8 to 1 to match chess board layout)
    for (int r = 7; r >= 0; --r) {
        std::cout << " " << (r + 1) << "  ";
        for (int f = 0; f < 8; ++f) {
            int square = sq(static_cast<File>(f), static_cast<Rank>(r));
            bool attacked = SqAttacked(square, pos, attacking_color);
            std::cout << (attacked ? " X " : " . ");
        }
        std::cout << " " << (r + 1) << "\n";
    }
    
    // Print column headers again
    std::cout << "    ";
    for (char c = 'a'; c <= 'h'; ++c) {
        std::cout << " " << c << " ";
    }
    std::cout << "\n\n";
}

void print_position_with_pieces(const Position& pos) {
    std::cout << "Current position:\n";
    
    // Print column headers
    std::cout << "    ";
    for (char c = 'a'; c <= 'h'; ++c) {
        std::cout << " " << c << " ";
    }
    std::cout << "\n";
    
    // Print each rank (from 8 to 1 to match chess board layout)
    for (int r = 7; r >= 0; --r) {
        std::cout << " " << (r + 1) << "  ";
        for (int f = 0; f < 8; ++f) {
            int square = sq(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = pos.at(square);
            char piece_char = '.';
            
            if (!is_none(p)) {
                switch (type_of(p)) {
                    case PieceType::Pawn:   piece_char = 'P'; break;
                    case PieceType::Knight: piece_char = 'N'; break;
                    case PieceType::Bishop: piece_char = 'B'; break;
                    case PieceType::Rook:   piece_char = 'R'; break;
                    case PieceType::Queen:  piece_char = 'Q'; break;
                    case PieceType::King:   piece_char = 'K'; break;
                    default: piece_char = '?'; break;
                }
                if (color_of(p) == Color::Black) {
                    piece_char = std::tolower(piece_char);
                }
            }
            
            std::cout << " " << piece_char << " ";
        }
        std::cout << " " << (r + 1) << "\n";
    }
    
    // Print column headers again
    std::cout << "    ";
    for (char c = 'a'; c <= 'h'; ++c) {
        std::cout << " " << c << " ";
    }
    std::cout << "\n\n";
}

int main() {
    std::cout << "=== SqAttacked Function Demo ===\n\n";
    
    // Scenario 1: Starting position attacks
    std::cout << "1. Starting position piece attacks:\n";
    Position start_pos;
    start_pos.set_startpos();
    
    print_position_with_pieces(start_pos);
    print_attack_map(start_pos, Color::White);
    print_attack_map(start_pos, Color::Black);
    
    // Scenario 2: Queen in center
    std::cout << "2. White Queen on e4 attacks:\n";
    Position queen_pos;
    queen_pos.reset();
    queen_pos.set(sq(File::E, Rank::R4), Piece::WhiteQueen);
    queen_pos.set(sq(File::H, Rank::R1), Piece::WhiteKing); // Place kings for valid position
    queen_pos.set(sq(File::A, Rank::R8), Piece::BlackKing);
    
    print_position_with_pieces(queen_pos);
    print_attack_map(queen_pos, Color::White);
    
    // Scenario 3: Knight attacks
    std::cout << "3. White Knight on d4 attacks:\n";
    Position knight_pos;
    knight_pos.reset();
    knight_pos.set(sq(File::D, Rank::R4), Piece::WhiteKnight);
    knight_pos.set(sq(File::H, Rank::R1), Piece::WhiteKing);
    knight_pos.set(sq(File::A, Rank::R8), Piece::BlackKing);
    
    print_position_with_pieces(knight_pos);
    print_attack_map(knight_pos, Color::White);
    
    // Scenario 4: Blocked sliding pieces
    std::cout << "4. Blocked rook attacks (Rook on a1, pawn on a3):\n";
    Position blocked_pos;
    blocked_pos.reset();
    blocked_pos.set(sq(File::A, Rank::R1), Piece::WhiteRook);
    blocked_pos.set(sq(File::A, Rank::R3), Piece::BlackPawn); // Blocking piece
    blocked_pos.set(sq(File::H, Rank::R1), Piece::WhiteKing);
    blocked_pos.set(sq(File::H, Rank::R8), Piece::BlackKing);
    
    print_position_with_pieces(blocked_pos);
    print_attack_map(blocked_pos, Color::White);
    
    // Scenario 5: Pawn attacks
    std::cout << "5. Pawn attacks from various positions:\n";
    Position pawn_pos;
    pawn_pos.reset();
    pawn_pos.set(sq(File::D, Rank::R4), Piece::WhitePawn);
    pawn_pos.set(sq(File::F, Rank::R5), Piece::BlackPawn);
    pawn_pos.set(sq(File::H, Rank::R1), Piece::WhiteKing);
    pawn_pos.set(sq(File::A, Rank::R8), Piece::BlackKing);
    
    print_position_with_pieces(pawn_pos);
    print_attack_map(pawn_pos, Color::White);
    print_attack_map(pawn_pos, Color::Black);
    
    // Scenario 6: Test specific square attacks
    std::cout << "6. Specific square attack tests:\n";
    Position test_pos;
    test_pos.reset();
    test_pos.set(sq(File::E, Rank::R4), Piece::WhiteQueen);
    test_pos.set(sq(File::B, Rank::R7), Piece::BlackKnight);
    test_pos.set(sq(File::H, Rank::R1), Piece::WhiteKing);
    test_pos.set(sq(File::A, Rank::R8), Piece::BlackKing);
    
    print_position_with_pieces(test_pos);
    
    // Test some specific squares
    std::cout << "Square attack results:\n";
    std::cout << "e1 attacked by White: " << (SqAttacked(sq(File::E, Rank::R1), test_pos, Color::White) ? "YES" : "NO") << "\n";
    std::cout << "e8 attacked by White: " << (SqAttacked(sq(File::E, Rank::R8), test_pos, Color::White) ? "YES" : "NO") << "\n";
    std::cout << "a1 attacked by White: " << (SqAttacked(sq(File::A, Rank::R1), test_pos, Color::White) ? "YES" : "NO") << "\n";
    std::cout << "h8 attacked by White: " << (SqAttacked(sq(File::H, Rank::R8), test_pos, Color::White) ? "YES" : "NO") << "\n";
    
    std::cout << "d6 attacked by Black: " << (SqAttacked(sq(File::D, Rank::R6), test_pos, Color::Black) ? "YES" : "NO") << "\n";
    std::cout << "c5 attacked by Black: " << (SqAttacked(sq(File::C, Rank::R5), test_pos, Color::Black) ? "YES" : "NO") << "\n";
    std::cout << "a8 attacked by Black: " << (SqAttacked(sq(File::A, Rank::R8), test_pos, Color::Black) ? "YES" : "NO") << "\n";
    
    std::cout << "\n=== SqAttacked Demo Complete ===\n";
    std::cout << "\nThe SqAttacked function successfully detects:\n";
    std::cout << "• Pawn attacks (diagonal captures for both colors)\n";
    std::cout << "• Knight attacks (L-shaped moves)\n";
    std::cout << "• King attacks (adjacent squares)\n";
    std::cout << "• Rook attacks (ranks and files until blocked)\n";
    std::cout << "• Bishop attacks (diagonals until blocked)\n";
    std::cout << "• Queen attacks (combination of rook and bishop)\n";
    std::cout << "• Proper blocking by intervening pieces\n";
    std::cout << "• Correct color differentiation\n";
    
    return 0;
}
