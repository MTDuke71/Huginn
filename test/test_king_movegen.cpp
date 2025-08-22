#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// Test king moves from center of board
TEST(KingMoveGen, KingMovesFromCenter) {
    Position pos;
    pos.reset();
    
    // Place white king on e4 (center)
    int king_sq = sq(File::E, Rank::R4);
    pos.set(king_sq, Piece::WhiteKing);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // King on e4 should have 8 moves (all directions clear)
    EXPECT_EQ(moves.size(), 8);
    
    // Verify moves are to the correct squares
    std::vector<int> expected_targets = {
        sq(File::D, Rank::R3), // SW
        sq(File::E, Rank::R3), // S
        sq(File::F, Rank::R3), // SE
        sq(File::D, Rank::R4), // W
        sq(File::F, Rank::R4), // E
        sq(File::D, Rank::R5), // NW
        sq(File::E, Rank::R5), // N
        sq(File::F, Rank::R5)  // NE
    };
    
    std::vector<int> actual_targets;
    for (const auto& move : moves.v) {
        actual_targets.push_back(move.get_to());
    }
    
    std::sort(expected_targets.begin(), expected_targets.end());
    std::sort(actual_targets.begin(), actual_targets.end());
    
    EXPECT_EQ(actual_targets, expected_targets);
}

// Test king moves from corner (limited mobility)
TEST(KingMoveGen, KingMovesFromCorner) {
    Position pos;
    pos.reset();
    
    // Place white king on a1 (corner)
    int king_sq = sq(File::A, Rank::R1);
    pos.set(king_sq, Piece::WhiteKing);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // King on a1 should have 3 moves (N, E, NE)
    EXPECT_EQ(moves.size(), 3);
    
    std::vector<int> expected_targets = {
        sq(File::A, Rank::R2), // N
        sq(File::B, Rank::R1), // E
        sq(File::B, Rank::R2)  // NE
    };
    
    std::vector<int> actual_targets;
    for (const auto& move : moves.v) {
        actual_targets.push_back(move.get_to());
    }
    
    std::sort(expected_targets.begin(), expected_targets.end());
    std::sort(actual_targets.begin(), actual_targets.end());
    
    EXPECT_EQ(actual_targets, expected_targets);
}

// Test king moves blocked by own pieces
TEST(KingMoveGen, KingMovesBlockedByOwnPieces) {
    Position pos;
    pos.reset();
    
    // Place white king on e4
    int king_sq = sq(File::E, Rank::R4);
    pos.set(king_sq, Piece::WhiteKing);
    
    // Block some squares with white pieces
    pos.set(sq(File::D, Rank::R4), Piece::WhitePawn); // W
    pos.set(sq(File::E, Rank::R5), Piece::WhiteRook); // N
    pos.set(sq(File::F, Rank::R3), Piece::WhiteBishop); // SE
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count only king moves
    int king_moves = 0;
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhiteKing) {
            king_moves++;
        }
    }
    
    // Should have 5 king moves (8 total - 3 blocked by own pieces)
    EXPECT_EQ(king_moves, 5);
    
    // Verify blocked squares are not in king move list
    std::vector<int> blocked_squares = {
        sq(File::D, Rank::R4), // W - blocked by pawn
        sq(File::E, Rank::R5), // N - blocked by rook
        sq(File::F, Rank::R3)  // SE - blocked by bishop
    };
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhiteKing) {
            EXPECT_TRUE(std::find(blocked_squares.begin(), blocked_squares.end(), move.get_to()) == blocked_squares.end())
                << "King move to blocked square should not be generated";
        }
    }
}

// Test king captures enemy pieces
TEST(KingMoveGen, KingCapturesEnemyPieces) {
    Position pos;
    pos.reset();
    
    // Place white king on e4
    int king_sq = sq(File::E, Rank::R4);
    pos.set(king_sq, Piece::WhiteKing);
    
    // Place enemy pieces to capture
    pos.set(sq(File::D, Rank::R4), Piece::BlackPawn); // W
    pos.set(sq(File::E, Rank::R5), Piece::BlackRook); // N
    pos.set(sq(File::F, Rank::R3), Piece::BlackBishop); // SE
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Should have 8 moves (including 3 captures)
    EXPECT_EQ(moves.size(), 8);
    
    // Check for capture moves
    int capture_count = 0;
    for (const auto& move : moves.v) {
        if (move.get_captured() != PieceType::None) {
            capture_count++;
        }
    }
    EXPECT_EQ(capture_count, 3);
    
    // Verify specific captures
    bool found_pawn_capture = false, found_rook_capture = false, found_bishop_capture = false;
    for (const auto& move : moves.v) {
        if (move.get_to() == sq(File::D, Rank::R4) && move.get_captured() == PieceType::Pawn) {
            found_pawn_capture = true;
        }
        if (move.get_to() == sq(File::E, Rank::R5) && move.get_captured() == PieceType::Rook) {
            found_rook_capture = true;
        }
        if (move.get_to() == sq(File::F, Rank::R3) && move.get_captured() == PieceType::Bishop) {
            found_bishop_capture = true;
        }
    }
    
    EXPECT_TRUE(found_pawn_capture);
    EXPECT_TRUE(found_rook_capture);
    EXPECT_TRUE(found_bishop_capture);
}

// Test king moves from edge of board
TEST(KingMoveGen, KingMovesFromEdge) {
    Position pos;
    pos.reset();
    
    // Place white king on e1 (back rank)
    int king_sq = sq(File::E, Rank::R1);
    pos.set(king_sq, Piece::WhiteKing);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // King on e1 should have 5 moves (can't move south)
    EXPECT_EQ(moves.size(), 5);
    
    std::vector<int> expected_targets = {
        sq(File::D, Rank::R1), // W
        sq(File::F, Rank::R1), // E
        sq(File::D, Rank::R2), // NW
        sq(File::E, Rank::R2), // N
        sq(File::F, Rank::R2)  // NE
    };
    
    std::vector<int> actual_targets;
    for (const auto& move : moves.v) {
        actual_targets.push_back(move.get_to());
    }
    
    std::sort(expected_targets.begin(), expected_targets.end());
    std::sort(actual_targets.begin(), actual_targets.end());
    
    EXPECT_EQ(actual_targets, expected_targets);
}

// Test both kings on board (integration test)
TEST(KingMoveGen, BothKingsOnBoard) {
    Position pos;
    pos.reset();
    
    // Place white king on e4 and black king on e6
    pos.set(sq(File::E, Rank::R4), Piece::WhiteKing);
    pos.set(sq(File::E, Rank::R6), Piece::BlackKing);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // White king should have 8 moves (black king far enough away)
    EXPECT_EQ(moves.size(), 8);
    
    // Test black's turn
    pos.side_to_move = Color::Black;
    moves.clear();
    generate_pseudo_legal_moves(pos, moves);
    
    // Black king should also have 8 moves
    EXPECT_EQ(moves.size(), 8);
}
