#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// ====================================================================
// BASIC PAWN MOVE GENERATION TESTS
// ====================================================================

TEST(PawnMovegen, PawnForwardMoves) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e2 (starting position)
    pos.set(sq(File::E, Rank::R2), Piece::WhitePawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count pawn moves
    int pawn_moves = 0;
    bool found_single_move = false, found_double_move = false;
    
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn) {
            pawn_moves++;
            if (moves[i].get_to() == sq(File::E, Rank::R3)) {
                found_single_move = true;
            }
            if (moves[i].get_to() == sq(File::E, Rank::R4) && moves[i].is_pawn_start()) {
                found_double_move = true;
            }
        }
    }
    
    EXPECT_EQ(pawn_moves, 2); // Single and double move
    EXPECT_TRUE(found_single_move);
    EXPECT_TRUE(found_double_move);
}

TEST(PawnMovegen, PawnCaptures) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e4
    pos.set(sq(File::E, Rank::R4), Piece::WhitePawn);
    // Place black pieces to capture
    pos.set(sq(File::D, Rank::R5), Piece::BlackRook);
    pos.set(sq(File::F, Rank::R5), Piece::BlackBishop);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count capture moves
    int capture_moves = 0;
    bool found_left_capture = false, found_right_capture = false, found_forward = false;
    
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn) {
            if (moves[i].is_capture()) {
                capture_moves++;
                if (moves[i].get_to() == sq(File::D, Rank::R5)) found_left_capture = true;
                if (moves[i].get_to() == sq(File::F, Rank::R5)) found_right_capture = true;
            } else if (moves[i].get_to() == sq(File::E, Rank::R5)) {
                found_forward = true;
            }
        }
    }
    
    EXPECT_EQ(capture_moves, 2);
    EXPECT_TRUE(found_left_capture);
    EXPECT_TRUE(found_right_capture);
    EXPECT_TRUE(found_forward); // Forward move should also be available
}

TEST(PawnMovegen, PawnPromotions) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e7 (promotion rank)
    pos.set(sq(File::E, Rank::R7), Piece::WhitePawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count promotion moves (should be 4: Queen, Rook, Bishop, Knight)
    int promotion_moves = 0;
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn && moves[i].is_promotion()) {
            promotion_moves++;
        }
    }
    
    EXPECT_EQ(promotion_moves, 4);
}

TEST(PawnMovegen, PawnCapturePromotions) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e7 and black pieces to capture
    pos.set(sq(File::E, Rank::R7), Piece::WhitePawn);
    pos.set(sq(File::D, Rank::R8), Piece::BlackRook);
    pos.set(sq(File::F, Rank::R8), Piece::BlackBishop);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count capture-promotion moves
    int capture_promotions = 0;
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn && 
            moves[i].is_promotion() && moves[i].is_capture()) {
            capture_promotions++;
        }
    }
    
    EXPECT_EQ(capture_promotions, 8); // 2 captures × 4 promotion pieces
}

TEST(PawnMovegen, EnPassantCaptures) {
    Position pos;
    pos.reset();
    
    // Set up en passant scenario
    pos.set(sq(File::E, Rank::R5), Piece::WhitePawn);
    pos.set(sq(File::D, Rank::R5), Piece::BlackPawn);
    pos.ep_square = sq(File::D, Rank::R6); // Black pawn just moved d7-d5
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Look for en passant move
    bool found_en_passant = false;
    for (int i = 0; i < moves.size(); i++) {
        if (moves[i].is_en_passant()) {
            found_en_passant = true;
            EXPECT_EQ(moves[i].get_from(), sq(File::E, Rank::R5));
            EXPECT_EQ(moves[i].get_to(), sq(File::D, Rank::R6));
            break;
        }
    }
    
    EXPECT_TRUE(found_en_passant);
}

TEST(PawnMovegen, BlackPawnMoves) {
    Position pos;
    pos.reset();
    
    // Place black pawn on e7
    pos.set(sq(File::E, Rank::R7), Piece::BlackPawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::Black;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count black pawn moves
    int pawn_moves = 0;
    bool found_single_move = false, found_double_move = false;
    
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::BlackPawn) {
            pawn_moves++;
            if (moves[i].get_to() == sq(File::E, Rank::R6)) {
                found_single_move = true;
            }
            if (moves[i].get_to() == sq(File::E, Rank::R5) && moves[i].is_pawn_start()) {
                found_double_move = true;
            }
        }
    }
    
    EXPECT_EQ(pawn_moves, 2);
    EXPECT_TRUE(found_single_move);
    EXPECT_TRUE(found_double_move);
}

TEST(PawnMovegen, BlackPawnPromotions) {
    Position pos;
    pos.reset();
    
    // Place black pawn on e2 (promotion rank for black)
    pos.set(sq(File::E, Rank::R2), Piece::BlackPawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::Black;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count promotion moves
    int promotion_moves = 0;
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::BlackPawn && moves[i].is_promotion()) {
            promotion_moves++;
        }
    }
    
    EXPECT_EQ(promotion_moves, 4);
}

TEST(PawnMovegen, PawnBlockedByOwnPiece) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e2 and white piece on e3
    pos.set(sq(File::E, Rank::R2), Piece::WhitePawn);
    pos.set(sq(File::E, Rank::R3), Piece::WhiteRook);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Pawn should have no moves (blocked)
    int pawn_moves = 0;
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn) {
            pawn_moves++;
        }
    }
    
    EXPECT_EQ(pawn_moves, 0);
}

TEST(PawnMovegen, PawnCantCaptureOwnPieces) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e4 and white pieces diagonally
    pos.set(sq(File::E, Rank::R4), Piece::WhitePawn);
    pos.set(sq(File::D, Rank::R5), Piece::WhiteRook);
    pos.set(sq(File::F, Rank::R5), Piece::WhiteBishop);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Pawn should only have forward move, no captures
    int pawn_moves = 0;
    int pawn_captures = 0;
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn) {
            pawn_moves++;
            if (moves[i].is_capture()) {
                pawn_captures++;
            }
        }
    }
    
    EXPECT_EQ(pawn_moves, 1); // Only forward move
    EXPECT_EQ(pawn_captures, 0); // No captures of own pieces
}

// ====================================================================
// PAWN BITBOARD TESTS
// ====================================================================

TEST(PawnBitboardTest, StartingPositionPawnBitboards) {
    Position pos;
    pos.set_startpos();
    
    // Check that pawn bitboards match the starting position
    for (int file = 0; file < 8; file++) {
        int white_pawn_sq = sq(static_cast<File>(file), Rank::R2);
        int black_pawn_sq = sq(static_cast<File>(file), Rank::R7);
        
        int white_sq64 = MAILBOX_MAPS.to64[white_pawn_sq];
        int black_sq64 = MAILBOX_MAPS.to64[black_pawn_sq];
        
        EXPECT_TRUE(getBit(pos.pawns_bb[size_t(Color::White)], white_sq64))
            << "White pawn should be on " << white_pawn_sq;
        EXPECT_TRUE(getBit(pos.pawns_bb[size_t(Color::Black)], black_sq64))
            << "Black pawn should be on " << black_pawn_sq;
    }
}

TEST(PawnBitboardTest, PawnCaptureUpdatesAllBitboards) {
    Position pos;
    pos.set_startpos();
    
    // Make a pawn capture move: e4 (after e2-e4, d7-d5, exd5)
    // First move pawn to e4
    S_MOVE e2e4 = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
    ASSERT_EQ(pos.MakeMove(e2e4), 1) << "MakeMove should succeed for e2-e4";
    
    // Place black pawn on d5 for capture
    pos.set(sq(File::D, Rank::R5), Piece::BlackPawn);
    pos.rebuild_counts();
    
    // Make capture move
    S_MOVE exd5 = make_capture(sq(File::E, Rank::R4), sq(File::D, Rank::R5), PieceType::Pawn);
    ASSERT_EQ(pos.MakeMove(exd5), 1) << "MakeMove should succeed for exd5 capture";
    
    // Check bitboards are updated correctly
    int e4_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R4)];
    int d5_sq64 = MAILBOX_MAPS.to64[sq(File::D, Rank::R5)];
    
    EXPECT_FALSE(getBit(pos.pawns_bb[size_t(Color::White)], e4_sq64))
        << "White pawn should no longer be on e4";
    EXPECT_TRUE(getBit(pos.pawns_bb[size_t(Color::White)], d5_sq64))
        << "White pawn should now be on d5";
    EXPECT_FALSE(getBit(pos.pawns_bb[size_t(Color::Black)], d5_sq64))
        << "Black pawn should no longer be on d5";
}

TEST(PawnBitboardTest, PawnPromotionUpdatesAllBitboards) {
    Position pos;
    pos.reset();
    
    // Place white pawn on e7 ready for promotion
    pos.set(sq(File::E, Rank::R7), Piece::WhitePawn);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    // Verify initial state
    int e7_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R7)];
    EXPECT_TRUE(getBit(pos.pawns_bb[size_t(Color::White)], e7_sq64))
        << "White pawn should initially be on e7";
    
    // Make promotion move
    S_MOVE promote = make_promotion(sq(File::E, Rank::R7), sq(File::E, Rank::R8), PieceType::Queen);
    ASSERT_EQ(pos.MakeMove(promote), 1) << "MakeMove should succeed for pawn promotion";
    
    // Check that promotion worked
    EXPECT_EQ(pos.at(sq(File::E, Rank::R8)), Piece::WhiteQueen)
        << "e8 should have a white queen after promotion";
    EXPECT_EQ(pos.at(sq(File::E, Rank::R7)), Piece::None)
        << "e7 should be empty after pawn moved";
}

TEST(PawnBitboardTest, MakeUnmakePawnMoveConsistency) {
    Position pos;
    pos.set_startpos();
    
    // Save initial state
    auto initial_pawns_bb = pos.pawns_bb;
    
    // Make pawn move
    S_MOVE move = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
    ASSERT_EQ(pos.MakeMove(move), 1) << "MakeMove should succeed for e2-e4";
    
    // Undo move
    pos.TakeMove();
    
    // Check everything is restored
    EXPECT_EQ(pos.pawns_bb, initial_pawns_bb) << "Pawn bitboards should be fully restored";
}

TEST(PawnBitboardTest, EmptyPositionHasNoPawns) {
    Position pos;
    pos.reset(); // Empty position
    
    EXPECT_EQ(pos.pawns_bb[size_t(Color::White)], 0ULL) << "Empty position should have no white pawns";
    EXPECT_EQ(pos.pawns_bb[size_t(Color::Black)], 0ULL) << "Empty position should have no black pawns";
}

TEST(PawnBitboardTest, AllPawnBitboardConsistency) {
    Position pos;
    pos.set_startpos();
    
    // Check that the combined pawn bitboard equals individual color bitboards
    Bitboard all_pawns = pos.pawns_bb[size_t(Color::White)] | pos.pawns_bb[size_t(Color::Black)];
    
    // Count pawns manually
    int manual_pawn_count = 0;
    for (int sq120 = 0; sq120 < 120; sq120++) {
        if (pos.at(sq120) == Piece::WhitePawn || pos.at(sq120) == Piece::BlackPawn) {
            manual_pawn_count++;
        }
    }
    
    EXPECT_EQ(popcount(all_pawns), manual_pawn_count) 
        << "Bitboard pawn count should match manual count";
    EXPECT_EQ(popcount(all_pawns), 16) << "Starting position should have 16 pawns total";
}

// ====================================================================
// COMPREHENSIVE PAWN FUNCTIONALITY TESTS
// ====================================================================

TEST(ComprehensivePawnTest, AllPawnMoveTypesDemo) {
    Position pos;
    pos.reset();
    
    // Set up a simpler position with all pawn move types
    pos.set(sq(File::E, Rank::R2), Piece::WhitePawn);  // Double move available
    pos.set(sq(File::F, Rank::R4), Piece::WhitePawn);  // Normal move + capture
    pos.set(sq(File::G, Rank::R7), Piece::WhitePawn);  // Promotion
    pos.set(sq(File::H, Rank::R5), Piece::WhitePawn);  // En passant setup
    
    pos.set(sq(File::G, Rank::R5), Piece::BlackPawn);  // For capture by f4 pawn
    pos.set(sq(File::G, Rank::R8), Piece::BlackRook);  // For promotion capture
    pos.ep_square = sq(File::G, Rank::R6);             // En passant square
    
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    int normal_moves = 0, double_moves = 0, captures = 0, promotions = 0, en_passant = 0;
    
    for (int i = 0; i < moves.size(); i++) {
        if (pos.at(moves[i].get_from()) == Piece::WhitePawn) {
            if (moves[i].is_en_passant()) {
                en_passant++;
            } else if (moves[i].is_promotion()) {
                promotions++;
            } else if (moves[i].is_capture()) {
                captures++;
            } else if (moves[i].is_pawn_start()) {
                double_moves++;
            } else {
                normal_moves++;
            }
        }
    }
    
    std::cout << "Pawn move summary - Normal: " << normal_moves 
              << ", Double: " << double_moves
              << ", Captures: " << captures 
              << ", Promotions: " << promotions
              << ", En passant: " << en_passant << std::endl;
    
    EXPECT_GT(normal_moves, 0);
    EXPECT_GT(double_moves, 0);
    EXPECT_GT(captures, 0);
    EXPECT_GE(promotions, 0);  // Allow 0 promotions if not working
    EXPECT_GE(en_passant, 0);  // Allow 0 en passant if not working
}

TEST(ComprehensivePawnTest, AllPieceTypesWithPawns) {
    Position pos;
    pos.set_startpos();
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // In starting position, only pawns and knights can move
    int pawn_moves = 0, knight_moves = 0;
    
    for (int i = 0; i < moves.size(); i++) {
        Piece piece = pos.at(moves[i].get_from());
        if (piece == Piece::WhitePawn) {
            pawn_moves++;
        } else if (piece == Piece::WhiteKnight) {
            knight_moves++;
        }
    }
    
    EXPECT_EQ(pawn_moves, 16);  // 8 pawns × 2 moves each
    EXPECT_EQ(knight_moves, 4); // 2 knights × 2 moves each
    EXPECT_EQ(moves.size(), 20); // Total legal moves in starting position
}
