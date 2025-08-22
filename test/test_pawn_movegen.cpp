#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// Test basic pawn forward moves
TEST(PawnMoveGen, PawnForwardMoves) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e2 (starting position)
    pos.set(sq(File::E, Rank::R2), Piece::WhitePawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count pawn moves
    int pawn_moves = 0;
    bool found_single_move = false, found_double_move = false;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn) {
            pawn_moves++;
            if (move.get_to() == sq(File::E, Rank::R3)) {
                found_single_move = true;
            }
            if (move.get_to() == sq(File::E, Rank::R4) && move.is_pawn_start()) {
                found_double_move = true;
            }
        }
    }
    
    EXPECT_EQ(pawn_moves, 2); // Single and double move
    EXPECT_TRUE(found_single_move);
    EXPECT_TRUE(found_double_move);
}

// Test pawn captures
TEST(PawnMoveGen, PawnCaptures) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e4
    pos.set(sq(File::E, Rank::R4), Piece::WhitePawn);
    // Place black pieces to capture
    pos.set(sq(File::D, Rank::R5), Piece::BlackRook);
    pos.set(sq(File::F, Rank::R5), Piece::BlackBishop);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count pawn moves
    int pawn_moves = 0;
    int pawn_captures = 0;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn) {
            pawn_moves++;
            if (move.get_captured() != PieceType::None) {
                pawn_captures++;
            }
        }
    }
    
    EXPECT_EQ(pawn_moves, 3); // One forward move + two captures
    EXPECT_EQ(pawn_captures, 2); // Two diagonal captures
}

// Test pawn promotions
TEST(PawnMoveGen, PawnPromotions) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e7 (one square from promotion)
    pos.set(sq(File::E, Rank::R7), Piece::WhitePawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count promotion moves
    int promotion_moves = 0;
    std::vector<PieceType> promoted_pieces;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn && move.is_promotion()) {
            promotion_moves++;
            promoted_pieces.push_back(move.get_promoted());
        }
    }
    
    EXPECT_EQ(promotion_moves, 4); // Queen, Rook, Bishop, Knight
    
    // Verify all promotion types (in the order they're generated)
    std::sort(promoted_pieces.begin(), promoted_pieces.end());
    std::vector<PieceType> expected = {PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen};
    EXPECT_EQ(promoted_pieces, expected);
}

// Test pawn capture promotions
TEST(PawnMoveGen, PawnCapturePromotions) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e7
    pos.set(sq(File::E, Rank::R7), Piece::WhitePawn);
    // Place black piece to capture on promotion
    pos.set(sq(File::D, Rank::R8), Piece::BlackQueen);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count capture promotion moves
    int capture_promotions = 0;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn && 
            move.is_promotion() && 
            move.get_captured() == PieceType::Queen) {
            capture_promotions++;
        }
    }
    
    EXPECT_EQ(capture_promotions, 4); // Four promotion types for the capture
}

// Test en passant captures
TEST(PawnMoveGen, EnPassantCaptures) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e5
    pos.set(sq(File::E, Rank::R5), Piece::WhitePawn);
    // Set up en passant square (as if black pawn just moved from d7 to d5)
    pos.ep_square = sq(File::D, Rank::R6);
    // Place the black pawn that just made the double move
    pos.set(sq(File::D, Rank::R5), Piece::BlackPawn);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count en passant moves
    int en_passant_moves = 0;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn && move.is_en_passant()) {
            en_passant_moves++;
            EXPECT_EQ(move.get_to(), sq(File::D, Rank::R6));
            EXPECT_EQ(move.get_captured(), PieceType::Pawn);
        }
    }
    
    EXPECT_EQ(en_passant_moves, 1);
}

// Test black pawn moves
TEST(PawnMoveGen, BlackPawnMoves) {
    Position pos;
    pos.reset();
    
    // Place black pawn on e7 (starting position)
    pos.set(sq(File::E, Rank::R7), Piece::BlackPawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::Black;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count black pawn moves
    int pawn_moves = 0;
    bool found_single_move = false, found_double_move = false;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::BlackPawn) {
            pawn_moves++;
            if (move.get_to() == sq(File::E, Rank::R6)) {
                found_single_move = true;
            }
            if (move.get_to() == sq(File::E, Rank::R5) && move.is_pawn_start()) {
                found_double_move = true;
            }
        }
    }
    
    EXPECT_EQ(pawn_moves, 2); // Single and double move
    EXPECT_TRUE(found_single_move);
    EXPECT_TRUE(found_double_move);
}

// Test black pawn promotions
TEST(PawnMoveGen, BlackPawnPromotions) {
    Position pos;
    pos.reset();
    
    // Place black pawn on e2 (one square from promotion for black)
    pos.set(sq(File::E, Rank::R2), Piece::BlackPawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::Black;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count promotion moves
    int promotion_moves = 0;
    
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::BlackPawn && move.is_promotion()) {
            promotion_moves++;
        }
    }
    
    EXPECT_EQ(promotion_moves, 4); // Queen, Rook, Bishop, Knight
}

// Test pawn blocked by own piece
TEST(PawnMoveGen, PawnBlockedByOwnPiece) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e2 and white piece in front
    pos.set(sq(File::E, Rank::R2), Piece::WhitePawn);
    pos.set(sq(File::E, Rank::R3), Piece::WhiteRook);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count pawn moves (should be 0)
    int pawn_moves = 0;
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn) {
            pawn_moves++;
        }
    }
    
    EXPECT_EQ(pawn_moves, 0);
}

// Test pawn can't capture own pieces
TEST(PawnMoveGen, PawnCantCaptureOwnPieces) {
    Position pos;
    pos.reset();
    
    // Place white pawn and white pieces diagonally
    pos.set(sq(File::E, Rank::R4), Piece::WhitePawn);
    pos.set(sq(File::D, Rank::R5), Piece::WhiteRook);
    pos.set(sq(File::F, Rank::R5), Piece::WhiteBishop);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    MoveList moves;
    generate_pseudo_legal_moves(pos, moves);
    
    // Count pawn moves (should only be forward move)
    int pawn_moves = 0;
    for (const auto& move : moves.v) {
        if (pos.at(move.get_from()) == Piece::WhitePawn) {
            pawn_moves++;
            EXPECT_EQ(move.get_to(), sq(File::E, Rank::R5)); // Only forward move
        }
    }
    
    EXPECT_EQ(pawn_moves, 1);
}
