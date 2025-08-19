#include <gtest/gtest.h>
#include "position.hpp"
#include "init.hpp"

class FENGenerationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Huginn::init();
    }
    
    Position pos;
};

TEST_F(FENGenerationTest, GenerateStartingPositionFEN) {
    // Test that starting position generates correct FEN
    pos.set_startpos();
    std::string generated_fen = pos.to_fen();
    std::string expected_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    EXPECT_EQ(generated_fen, expected_fen)
        << "Generated FEN should match starting position FEN";
}

TEST_F(FENGenerationTest, GenerateKiwipetePositionFEN) {
    // Test with a complex position (Kiwipete)
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    std::string generated_fen = pos.to_fen();
    
    EXPECT_EQ(generated_fen, kiwipete_fen)
        << "Generated FEN should match input FEN for Kiwipete position";
}

TEST_F(FENGenerationTest, GeneratePositionWithEnPassant) {
    // Test position with en passant square
    const std::string en_passant_fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(en_passant_fen));
    std::string generated_fen = pos.to_fen();
    
    EXPECT_EQ(generated_fen, en_passant_fen)
        << "Generated FEN should match input FEN with en passant square";
}

TEST_F(FENGenerationTest, GeneratePositionWithPartialCastling) {
    // Test position with only some castling rights
    const std::string partial_castle_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(partial_castle_fen));
    std::string generated_fen = pos.to_fen();
    
    EXPECT_EQ(generated_fen, partial_castle_fen)
        << "Generated FEN should match input FEN with partial castling rights";
}

TEST_F(FENGenerationTest, GeneratePositionWithNoCastling) {
    // Test position with no castling rights
    const std::string no_castle_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(no_castle_fen));
    std::string generated_fen = pos.to_fen();
    
    EXPECT_EQ(generated_fen, no_castle_fen)
        << "Generated FEN should match input FEN with no castling rights";
}

TEST_F(FENGenerationTest, GeneratePositionWithMoveCounts) {
    // Test position with non-zero move counts
    const std::string move_counts_fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3";
    
    ASSERT_TRUE(pos.set_from_fen(move_counts_fen));
    std::string generated_fen = pos.to_fen();
    
    EXPECT_EQ(generated_fen, move_counts_fen)
        << "Generated FEN should match input FEN with move counts";
}

TEST_F(FENGenerationTest, RoundTripFENConversion) {
    // Test that FEN -> Position -> FEN preserves the original
    std::vector<std::string> test_fens = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
        "rnbqkb1r/pppppppp/5n2/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 1 2",
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 50 25",
        "8/8/8/8/8/8/8/8 b - - 0 1"
    };
    
    for (const auto& original_fen : test_fens) {
        ASSERT_TRUE(pos.set_from_fen(original_fen)) 
            << "Failed to parse FEN: " << original_fen;
            
        std::string generated_fen = pos.to_fen();
        
        EXPECT_EQ(generated_fen, original_fen)
            << "Round-trip FEN conversion failed for: " << original_fen;
    }
}

TEST_F(FENGenerationTest, EmptyBoardFEN) {
    // Test empty board
    pos.reset();
    pos.side_to_move = Color::Black;
    pos.halfmove_clock = 0;
    pos.fullmove_number = 1;
    
    std::string generated_fen = pos.to_fen();
    std::string expected_fen = "8/8/8/8/8/8/8/8 b - - 0 1";
    
    EXPECT_EQ(generated_fen, expected_fen)
        << "Empty board should generate correct FEN";
}
