#include <gtest/gtest.h>
#include "debug.hpp"
#include "position.hpp"
#include "init.hpp"

class DebugValidationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Huginn::init();
    }
};

TEST_F(DebugValidationTest, ValidateStartingPositionConsistency) {
    Position pos;
    pos.set_startpos();
    
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    // This should pass all validation checks
    bool result = Debug::validate_position_consistency(pos, starting_fen);
    EXPECT_TRUE(result) << "Starting position should pass all consistency checks";
}

TEST_F(DebugValidationTest, ValidateKiwipetePositionConsistency) {
    Position pos;
    pos.reset();
    
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    // Set position from FEN
    bool fen_success = pos.set_from_fen(kiwipete_fen);
    EXPECT_TRUE(fen_success) << "FEN parsing should succeed";
    
    // This should pass all validation checks
    bool result = Debug::validate_position_consistency(pos, kiwipete_fen);
    EXPECT_TRUE(result) << "Kiwipete position should pass all consistency checks";
}

TEST_F(DebugValidationTest, ValidateAfterMovesConsistency) {
    Position pos;
    pos.set_startpos();
    
    // Make some moves: e2-e4, e7-e5, Nf3, Nc6
    S_MOVE move1 = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
    int result1 = pos.MakeMove(move1);
    EXPECT_EQ(1, result1) << "Move e2-e4 should be legal";
    
    S_MOVE move2 = make_move(sq(File::E, Rank::R7), sq(File::E, Rank::R5));
    int result2 = pos.MakeMove(move2);
    EXPECT_EQ(1, result2) << "Move e7-e5 should be legal";
    
    S_MOVE move3 = make_move(sq(File::G, Rank::R1), sq(File::F, Rank::R3));
    int result3 = pos.MakeMove(move3);
    EXPECT_EQ(1, result3) << "Move Nf3 should be legal";
    
    S_MOVE move4 = make_move(sq(File::B, Rank::R8), sq(File::C, Rank::R6));
    int result4 = pos.MakeMove(move4);
    EXPECT_EQ(1, result4) << "Move Nc6 should be legal";
    
    // Expected FEN after these moves
    std::string expected_fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3";
    
    // This should pass all validation checks
    bool result = Debug::validate_position_consistency(pos, expected_fen);
    EXPECT_TRUE(result) << "Position after moves should pass all consistency checks";
}

TEST_F(DebugValidationTest, ValidateIndividualComponents) {
    Position pos;
    pos.set_startpos();
    
    // Test individual validation functions
    EXPECT_TRUE(Debug::validate_bitboards_consistency(pos)) 
        << "Bitboards should be consistent in starting position";
    
    EXPECT_TRUE(Debug::validate_piece_counts_consistency(pos))
        << "Piece counts should be consistent in starting position";
    
    EXPECT_TRUE(Debug::validate_piece_lists_consistency(pos))
        << "Piece lists should be consistent in starting position";
    
    EXPECT_TRUE(Debug::validate_material_scores_consistency(pos))
        << "Material scores should be consistent in starting position";
    
    EXPECT_TRUE(Debug::validate_king_squares_consistency(pos))
        << "King squares should be consistent in starting position";
    
    EXPECT_TRUE(Debug::validate_zobrist_consistency(pos))
        << "Zobrist hash should be consistent in starting position";
}

TEST_F(DebugValidationTest, ValidateAfterCaptureMove) {
    Position pos;
    pos.reset();
    
    // Set up a position with pawns that can capture each other
    pos.board[sq(File::E, Rank::R4)] = Piece::WhitePawn;
    pos.board[sq(File::D, Rank::R5)] = Piece::BlackPawn;
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteKing;
    pos.board[sq(File::A, Rank::R8)] = Piece::BlackKing;
    pos.side_to_move = Color::White;
    
    pos.rebuild_counts();
    
    // Make a capture move: exd5
    S_MOVE capture_move = make_capture(sq(File::E, Rank::R4), sq(File::D, Rank::R5), PieceType::Pawn);
    int capture_result = pos.MakeMove(capture_move);
    EXPECT_EQ(1, capture_result) << "Capture move exd5 should be legal";
    
    // Validate all components after capture
    EXPECT_TRUE(Debug::validate_bitboards_consistency(pos))
        << "Bitboards should be consistent after capture";
    
    EXPECT_TRUE(Debug::validate_piece_counts_consistency(pos))
        << "Piece counts should be consistent after capture";
    
    EXPECT_TRUE(Debug::validate_material_scores_consistency(pos))
        << "Material scores should be consistent after capture";
}

TEST_F(DebugValidationTest, ValidateEnPassantConsistency) {
    Position pos;
    pos.reset();
    
    // Test 1: Position with no en passant (should pass)
    pos.side_to_move = Color::White;
    pos.ep_square = -1;
    
    EXPECT_TRUE(Debug::validate_en_passant_consistency(pos))
        << "No en passant square should be valid";
    
    // Test 2: Valid en passant for White to move (Black just moved pawn 2 squares)
    // Black pawn moved e7-e5, so ep square should be e6 (rank 6)
    pos.side_to_move = Color::White;
    pos.ep_square = sq(File::E, Rank::R6);  // e6 - correct rank for White to move
    pos.board[sq(File::E, Rank::R5)] = Piece::BlackPawn;  // Black pawn on e5 (just moved from e7)
    pos.board[sq(File::E, Rank::R6)] = Piece::None;       // e6 should be empty
    pos.board[sq(File::E, Rank::R7)] = Piece::None;       // e7 should now be empty (pawn moved from here)
    
    EXPECT_TRUE(Debug::validate_en_passant_consistency(pos))
        << "Valid en passant for White to move should pass";
    
    // Test 3: Valid en passant for Black to move (White just moved pawn 2 squares)
    // White pawn moved d2-d4, so ep square should be d3 (rank 3)
    pos.reset();  // Reset to clean state
    pos.side_to_move = Color::Black;
    pos.ep_square = sq(File::D, Rank::R3);  // d3 - correct rank for Black to move
    pos.board[sq(File::D, Rank::R4)] = Piece::WhitePawn;  // White pawn on d4 (just moved from d2)
    pos.board[sq(File::D, Rank::R3)] = Piece::None;       // d3 should be empty
    pos.board[sq(File::D, Rank::R2)] = Piece::None;       // d2 should now be empty (pawn moved from here)
    
    EXPECT_TRUE(Debug::validate_en_passant_consistency(pos))
        << "Valid en passant for Black to move should pass";
    
    // Test 4: Invalid en passant rank (White to move but ep on rank 3)
    pos.reset();
    pos.side_to_move = Color::White;
    pos.ep_square = sq(File::C, Rank::R3);  // c3 - WRONG rank for White to move
    
    EXPECT_FALSE(Debug::validate_en_passant_consistency(pos))
        << "En passant on wrong rank should fail validation";
    
    // Test 5: Invalid en passant rank (Black to move but ep on rank 6)
    pos.reset();
    pos.side_to_move = Color::Black;
    pos.ep_square = sq(File::F, Rank::R6);  // f6 - WRONG rank for Black to move
    
    EXPECT_FALSE(Debug::validate_en_passant_consistency(pos))
        << "En passant on wrong rank should fail validation";
}

TEST_F(DebugValidationTest, ValidateCastlingConsistency) {
    Position pos;
    
    // Test 1: Starting position with all castling rights (should pass)
    pos.set_startpos();
    
    EXPECT_TRUE(Debug::validate_castling_consistency(pos))
        << "Starting position should have valid castling rights";
    
    // Test 2: Position with no castling rights (should pass)
    pos.reset();
    pos.castling_rights = CASTLE_NONE;
    
    EXPECT_TRUE(Debug::validate_castling_consistency(pos))
        << "Position with no castling rights should be valid";
    
    // Test 3: Position with partial castling rights and correct piece placement
    pos.reset();
    pos.castling_rights = CASTLE_WK | CASTLE_BQ;  // White kingside, Black queenside
    pos.board[sq(File::E, Rank::R1)] = Piece::WhiteKing;    // White king on e1
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteRook;    // White rook on h1
    pos.board[sq(File::E, Rank::R8)] = Piece::BlackKing;    // Black king on e8
    pos.board[sq(File::A, Rank::R8)] = Piece::BlackRook;    // Black rook on a8
    
    EXPECT_TRUE(Debug::validate_castling_consistency(pos))
        << "Partial castling rights with correct pieces should be valid";
    
    // Test 4: Invalid - White kingside castling but king not on e1
    pos.reset();
    pos.castling_rights = CASTLE_WK;
    pos.board[sq(File::E, Rank::R2)] = Piece::WhiteKing;    // King moved to e2
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteRook;    // Rook still on h1
    
    EXPECT_FALSE(Debug::validate_castling_consistency(pos))
        << "Kingside castling with king not on e1 should fail";
    
    // Test 5: Invalid - White queenside castling but rook not on a1
    pos.reset();
    pos.castling_rights = CASTLE_WQ;
    pos.board[sq(File::E, Rank::R1)] = Piece::WhiteKing;    // King on e1
    pos.board[sq(File::A, Rank::R2)] = Piece::WhiteRook;    // Rook moved to a2
    
    EXPECT_FALSE(Debug::validate_castling_consistency(pos))
        << "Queenside castling with rook not on a1 should fail";
    
    // Test 6: Invalid - Black kingside castling but pieces missing
    pos.reset();
    pos.castling_rights = CASTLE_BK;
    // No pieces placed - board is empty
    
    EXPECT_FALSE(Debug::validate_castling_consistency(pos))
        << "Black kingside castling with no pieces should fail";
    
    // Test 7: Complex case - mixed valid/invalid castling rights
    pos.reset();
    pos.castling_rights = CASTLE_WK | CASTLE_WQ | CASTLE_BK;  // All except Black queenside
    pos.board[sq(File::E, Rank::R1)] = Piece::WhiteKing;    // White king correct
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteRook;    // White kingside rook correct
    pos.board[sq(File::A, Rank::R1)] = Piece::WhiteRook;    // White queenside rook correct
    pos.board[sq(File::E, Rank::R8)] = Piece::BlackKing;    // Black king correct
    pos.board[sq(File::H, Rank::R8)] = Piece::BlackRook;    // Black kingside rook correct
    // No Black queenside rook (a8 empty) - this should be fine since CASTLE_BQ not set
    
    EXPECT_TRUE(Debug::validate_castling_consistency(pos))
        << "Mixed castling rights with correct pieces should be valid";
}
