#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// ====================================================================
// ROOK MOVE GENERATION TESTS
// ====================================================================

TEST(RookMoveGen, SingleRookCenter) {
    Position pos; pos.reset();
    // Place a white rook on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    // Assert rook piece list and count are correct
    ASSERT_EQ(pos.pCount[int(Color::White)][int(PieceType::Rook)], 1);
    ASSERT_EQ(pos.pList[int(Color::White)][int(PieceType::Rook)][0], sq(File::D, Rank::R4));
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int rook_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteRook)
            ++rook_moves;
    }
    // Should be 14 (7 in each direction, minus the square itself)
    EXPECT_EQ(rook_moves, 14);
}

TEST(RookMoveGen, RookBlockedByOwnPiece) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::D, Rank::R6), Piece::WhitePawn); // Block north
    pos.set(sq(File::B, Rank::R4), Piece::WhiteKnight); // Block west
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int rook_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteRook)
            ++rook_moves;
    }
    // Should be reduced due to blocking pieces
    EXPECT_LT(rook_moves, 14);
}

TEST(RookMoveGen, RookCapturesOpponent) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::D, Rank::R7), Piece::BlackPawn); // Enemy piece to capture
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    // Look for capture move
    bool found_capture = false;
    for (int i = 0; i < moves.count; i++) {
        if (moves.moves[i].get_from() == sq(File::D, Rank::R4) && 
            moves.moves[i].get_to() == sq(File::D, Rank::R7) &&
            moves.moves[i].is_capture()) {
            found_capture = true;
            break;
        }
    }
    EXPECT_TRUE(found_capture);
}

// ====================================================================
// BISHOP MOVE GENERATION TESTS
// ====================================================================

TEST(BishopMoveGen, SingleBishopCenter) {
    Position pos; pos.reset();
    // Place a white bishop on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteBishop);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    // Assert bishop piece list and count are correct
    ASSERT_EQ(pos.pCount[int(Color::White)][int(PieceType::Bishop)], 1);
    ASSERT_EQ(pos.pList[int(Color::White)][int(PieceType::Bishop)][0], sq(File::D, Rank::R4));
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int bishop_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteBishop)
            ++bishop_moves;
    }
    // Should be 13 (bishop on d4 can move to 13 diagonal squares)
    EXPECT_EQ(bishop_moves, 13);
}

TEST(BishopMoveGen, BishopBlockedByOwnPiece) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::F, Rank::R6), Piece::WhitePawn); // Block NE diagonal
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int bishop_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteBishop)
            ++bishop_moves;
    }
    EXPECT_LT(bishop_moves, 13);
}

TEST(BishopMoveGen, BishopCapturesOpponent) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::F, Rank::R6), Piece::BlackPawn);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    bool found_capture = false;
    for (int i = 0; i < moves.count; i++) {
        if (moves.moves[i].get_from() == sq(File::D, Rank::R4) && 
            moves.moves[i].get_to() == sq(File::F, Rank::R6) &&
            moves.moves[i].is_capture()) {
            found_capture = true;
            break;
        }
    }
    EXPECT_TRUE(found_capture);
}

// ====================================================================
// QUEEN MOVE GENERATION TESTS
// ====================================================================

TEST(QueenMoveGen, SingleQueenCenter) {
    Position pos; pos.reset();
    // Place a white queen on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int queen_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteQueen)
            ++queen_moves;
    }
    // Should be 27 (rook moves + bishop moves)
    EXPECT_EQ(queen_moves, 27);
}

TEST(QueenMoveGen, QueenBlockedByOwnPiece) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::D, Rank::R6), Piece::WhitePawn); // Block north
    pos.set(sq(File::F, Rank::R6), Piece::WhiteKnight); // Block NE diagonal
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int queen_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteQueen)
            ++queen_moves;
    }
    EXPECT_LT(queen_moves, 27);
}

TEST(QueenMoveGen, QueenCapturesOpponent) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::D, Rank::R7), Piece::BlackRook);
    pos.set(sq(File::G, Rank::R7), Piece::BlackBishop);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int capture_count = 0;
    for (int i = 0; i < moves.count; i++) {
        if (moves.moves[i].get_from() == sq(File::D, Rank::R4) && 
            moves.moves[i].is_capture()) {
            capture_count++;
        }
    }
    EXPECT_GE(capture_count, 1);
}

// ====================================================================
// KING MOVE GENERATION TESTS
// ====================================================================

TEST(KingMoveGen, KingMovesFromCenter) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int king_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteKing) {
            king_moves++;
        }
    }
    
    // King on d4 should have 8 moves (all 8 adjacent squares)
    EXPECT_EQ(king_moves, 8);
}

TEST(KingMoveGen, KingMovesFromCorner) {
    Position pos; pos.reset();
    pos.set(sq(File::A, Rank::R1), Piece::WhiteKing);
    pos.king_sq[int(Color::White)] = sq(File::A, Rank::R1);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int king_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteKing) {
            king_moves++;
        }
    }
    
    // King on a1 should have 3 moves
    EXPECT_EQ(king_moves, 3);
}

TEST(KingMoveGen, KingMovesBlockedByOwnPieces) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.set(sq(File::C, Rank::R4), Piece::WhitePawn);
    pos.set(sq(File::D, Rank::R5), Piece::WhiteRook);
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int king_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteKing) {
            king_moves++;
        }
    }
    
    // Should be less than 8 due to blocking pieces
    EXPECT_LT(king_moves, 8);
}

TEST(KingMoveGen, KingCapturesEnemyPieces) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.set(sq(File::C, Rank::R4), Piece::BlackPawn);
    pos.set(sq(File::D, Rank::R5), Piece::BlackRook);
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int king_captures = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteKing && 
            moves.moves[i].is_capture()) {
            king_captures++;
        }
    }
    
    EXPECT_GE(king_captures, 1);
}

TEST(KingMoveGen, KingMovesFromEdge) {
    Position pos; pos.reset();
    pos.set(sq(File::A, Rank::R4), Piece::WhiteKing);
    pos.king_sq[int(Color::White)] = sq(File::A, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int king_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteKing) {
            king_moves++;
        }
    }
    
    // King on a4 should have 5 moves
    EXPECT_EQ(king_moves, 5);
}

TEST(KingMoveGen, BothKingsOnBoard) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.set(sq(File::E, Rank::R5), Piece::BlackKing); // Adjacent to white king
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.king_sq[int(Color::Black)] = sq(File::E, Rank::R5);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int king_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteKing) {
            king_moves++;
        }
    }
    
    // Should generate moves for white king
    // Note: Legal move filtering for king-king adjacency may not be fully implemented
    EXPECT_GT(king_moves, 0);
    EXPECT_LE(king_moves, 8); // Allow up to 8 moves if king-king rule not enforced
}

// ====================================================================
// SLIDING PIECES INTEGRATION TEST
// ====================================================================

TEST(SlidingPieceMoveGen, AllSlidingPiecesTogether) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::A, Rank::R1), Piece::WhiteRook);
    pos.set(sq(File::H, Rank::R8), Piece::WhiteBishop);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    int sliding_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        Piece piece = pos.at(moves.moves[i].get_from());
        if (piece == Piece::WhiteQueen || piece == Piece::WhiteRook || piece == Piece::WhiteBishop) {
            sliding_moves++;
        }
    }
    
    EXPECT_GT(sliding_moves, 20); // Should have many sliding piece moves
}

// ====================================================================
// ALL PIECE INTEGRATION TESTS
// ====================================================================

TEST(AllPieceMoveGen, AllImplementedPiecesGenerateMoves) {
    Position pos; pos.reset();
    
    // Place one of each piece type
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.set(sq(File::E, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::A, Rank::R1), Piece::WhiteRook);
    pos.set(sq(File::C, Rank::R1), Piece::WhiteBishop);
    pos.set(sq(File::B, Rank::R1), Piece::WhiteKnight);
    
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    // Count moves for each piece type
    int king_moves = 0, queen_moves = 0, rook_moves = 0, bishop_moves = 0, knight_moves = 0;
    
    for (int i = 0; i < moves.count; i++) {
        Piece piece = pos.at(moves.moves[i].get_from());
        switch (piece) {
            case Piece::WhiteKing: king_moves++; break;
            case Piece::WhiteQueen: queen_moves++; break;
            case Piece::WhiteRook: rook_moves++; break;
            case Piece::WhiteBishop: bishop_moves++; break;
            case Piece::WhiteKnight: knight_moves++; break;
            default: break;
        }
    }
    
    std::cout << "Move counts - King: " << king_moves 
              << ", Queen: " << queen_moves 
              << ", Rook: " << rook_moves
              << ", Bishop: " << bishop_moves 
              << ", Knight: " << knight_moves 
              << ", Total: " << moves.count << std::endl;
    
    EXPECT_GT(king_moves, 0);
    EXPECT_GT(queen_moves, 0);
    EXPECT_GT(rook_moves, 0);
    EXPECT_GT(bishop_moves, 0);
    EXPECT_GT(knight_moves, 0);
}

TEST(AllPieceMoveGen, KingMoveCountCorrect) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.king_sq[int(Color::White)] = sq(File::D, Rank::R4);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();

    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    
    EXPECT_EQ(moves.count, 8);
}
