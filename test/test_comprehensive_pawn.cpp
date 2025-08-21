#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"

// Comprehensive test showing all pawn move types in one test
TEST(ComprehensivePawnTest, AllPawnMoveTypesDemo) {
    Position pos;
    pos.reset();
    
    // Set up a position that demonstrates all pawn move types
    
    // Normal pawn moves
    pos.set(sq(File::A, Rank::R2), Piece::WhitePawn); // Can move 1 or 2 squares
    pos.set(sq(File::B, Rank::R4), Piece::WhitePawn); // Can move 1 square only
    
    // Pawn captures
    pos.set(sq(File::C, Rank::R5), Piece::WhitePawn);
    pos.set(sq(File::D, Rank::R6), Piece::BlackRook); // Available for capture
    
    // Promotion
    pos.set(sq(File::E, Rank::R7), Piece::WhitePawn); // About to promote
    
    // Promotion with capture
    pos.set(sq(File::F, Rank::R7), Piece::WhitePawn);
    pos.set(sq(File::G, Rank::R8), Piece::BlackQueen); // Available for capture promotion
    
    // En passant setup
    pos.set(sq(File::H, Rank::R5), Piece::WhitePawn);
    pos.ep_square = sq(File::G, Rank::R6); // As if black pawn moved g7-g5
    pos.set(sq(File::G, Rank::R5), Piece::BlackPawn); // The pawn that just moved
    
    // Black pawns for variety
    pos.set(sq(File::A, Rank::R7), Piece::BlackPawn); // Can move 1 or 2 squares
    pos.set(sq(File::B, Rank::R2), Piece::BlackPawn); // About to promote
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count different types of pawn moves
    int normal_moves = 0;
    int double_moves = 0;
    int captures = 0;
    int promotions = 0;
    int en_passant_moves = 0;
    
    for (const auto& move : moves.v) {
        Piece piece = pos.at(move.get_from());
        if (type_of(piece) == PieceType::Pawn && color_of(piece) == Color::White) {
            if (move.is_en_passant()) {
                en_passant_moves++;
            } else if (move.is_promotion()) {
                promotions++;
            } else if (move.is_pawn_start()) {
                double_moves++;
            } else if (move.get_captured() != PieceType::None) {
                captures++;
            } else {
                normal_moves++;
            }
        }
    }
    
    // Verify we have all types of pawn moves
    EXPECT_GT(normal_moves, 0) << "Should have normal pawn moves";
    EXPECT_GT(double_moves, 0) << "Should have double pawn moves";
    EXPECT_GT(captures, 0) << "Should have pawn captures";
    EXPECT_GT(promotions, 0) << "Should have pawn promotions";
    EXPECT_EQ(en_passant_moves, 1) << "Should have exactly one en passant move";
    
    std::cout << "Pawn move summary - Normal: " << normal_moves 
              << ", Double: " << double_moves 
              << ", Captures: " << captures 
              << ", Promotions: " << promotions 
              << ", En passant: " << en_passant_moves << std::endl;
    
    // Verify total count makes sense
    int total_pawn_moves = normal_moves + double_moves + captures + promotions + en_passant_moves;
    EXPECT_GT(total_pawn_moves, 10) << "Should have many pawn moves in this position";
}

// Test that all piece types work together with pawns
TEST(ComprehensivePawnTest, AllPieceTypesWithPawns) {
    Position pos;
    pos.reset();
    
    // Set up starting position
    pos.set_startpos();
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // In starting position, we should have exactly 20 moves
    EXPECT_EQ(moves.size(), 20);
    
    // Count moves by piece type
    int pawn_moves = 0, knight_moves = 0;
    
    for (const auto& move : moves.v) {
        PieceType type = type_of(pos.at(move.get_from()));
        if (type == PieceType::Pawn) pawn_moves++;
        else if (type == PieceType::Knight) knight_moves++;
    }
    
    EXPECT_EQ(pawn_moves, 16) << "Should have 16 pawn moves (8 pawns × 2 moves each)";
    EXPECT_EQ(knight_moves, 4) << "Should have 4 knight moves (2 knights × 2 moves each)";
}
