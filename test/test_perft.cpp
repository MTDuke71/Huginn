#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// Tiny perft harness (uses legal moves; grow as you add rules)
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        pos.make_move_with_undo(m);
        nodes += perft(pos, depth-1);
        pos.undo_move();
    }
    return nodes;
}

// Smoke: start position is initialized correctly
TEST(Perft, StartposSmoke_KingsAndPawns) {
    Position pos; pos.set_startpos();
    int whitePawns=0, blackPawns=0, whiteKings=0, blackKings=0;
    for (int r=0; r<8; ++r) {
        for (int f=0; f<8; ++f) {
            int s = sq(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = pos.at(s);
            if (is_none(p)) continue;
            if (p == Piece::WhitePawn) ++whitePawns;
            if (p == Piece::BlackPawn) ++blackPawns;
            if (p == Piece::WhiteKing) ++whiteKings;
            if (p == Piece::BlackKing) ++blackKings;
        }
    }
    EXPECT_EQ(whitePawns, 8);
    EXPECT_EQ(blackPawns, 8);
    EXPECT_EQ(whiteKings, 1);
    EXPECT_EQ(blackKings, 1);
}

// These are disabled until your generator covers all piece types & rules.
// Enable one depth at a time as you pass prior tests.

TEST(Perft, Startpos_d1_is_20) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 1), 20u);
}

TEST(Perft, Startpos_d1_partial) {
    Position pos; pos.set_startpos();
    uint64_t count = perft(pos, 1);
    // Starting position: 16 pawn moves + 4 knight moves = 20 total
    // Now we have all implemented pieces working correctly
    std::cout << "Move count at depth 1: " << count << std::endl;
    // We now expect exactly 20 moves
    EXPECT_EQ(count, 20u);
}

TEST(Perft, Startpos_d2_is_400) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 2), 400u);
}

TEST(Perft, Startpos_d3_is_8902) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 3), 8902u);
}

// Simple test with start position first
TEST(Perft, Debug_Startpos_Legal_Check) {
    Position pos;
    pos.set_startpos();
    
    std::cout << "Testing legality check with start position:" << std::endl;
    std::cout << "King in check: " << (in_check(pos) ? "YES" : "NO") << std::endl;
    
    MoveList pseudo_moves;
    generate_pseudo_legal_moves(pos, pseudo_moves);
    
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);
    
    std::cout << "Pseudo-legal: " << pseudo_moves.v.size() << ", Legal: " << legal_moves.v.size() << std::endl;
    
    // In start position, all pseudo-legal moves should be legal
    EXPECT_EQ(pseudo_moves.v.size(), legal_moves.v.size());
}

// Debug what's attacking the king in Kiwipete
TEST(Perft, Debug_Kiwipete_Check) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen)) << "Failed to parse Kiwipete FEN";
    
    std::cout << "White king position: " << pos.king_sq[0] << std::endl;
    std::cout << "Black queen attacks white king: " << SqAttacked(pos.king_sq[0], pos, Color::Black) << std::endl;
    
    // Test individual pieces attacking the king
    std::cout << "Checking what's attacking the white king on g1..." << std::endl;
    
    // Check if any black pieces can attack g1
    for (int sq = 0; sq < 120; ++sq) {
        if (!is_playable(sq)) continue;
        Piece p = pos.at(sq);
        if (is_none(p) || color_of(p) != Color::Black) continue;
        
        // Test if this black piece attacks the white king
        if (SqAttacked(pos.king_sq[0], pos, Color::Black)) {
            std::cout << "Found black piece at " << sq << " that could be attacking" << std::endl;
        }
    }
}

// Kiwipete covers castling, pins, etc.
TEST(Perft, Kiwipete_d1_48_d2_2039) {
    Position pos;
    // The famous Kiwipete position (Position 2 from Perft test suite)
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen)) << "Failed to parse Kiwipete FEN";
    
    // Output some debug info
    std::cout << "Kiwipete FEN: " << kiwipete_fen << std::endl;
    std::cout << "Parsed back: " << pos.to_fen() << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "King positions: White=" << pos.king_sq[0] << ", Black=" << pos.king_sq[1] << std::endl;
    
    // Generate pseudo-legal moves first
    MoveList pseudo_moves;
    generate_pseudo_legal_moves(pos, pseudo_moves);
    std::cout << "Generated " << pseudo_moves.v.size() << " pseudo-legal moves" << std::endl;
    
    // Check which moves are being rejected
    std::cout << "\nFirst 10 pseudo-legal moves and their legality:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(10), pseudo_moves.v.size()); ++i) {
        const auto& move = pseudo_moves.v[i];
        
        // Test if king is currently in check
        bool currently_in_check = in_check(pos);
        
        // Apply move manually to see what happens
        Position test_pos = pos;
        Color original_side = test_pos.side_to_move;  // Remember original side
        test_pos.make_move_with_undo(move);
        
        // Check if the ORIGINAL side's king is in check after the move
        int original_king_sq = test_pos.king_sq[int(original_side)];
        Color opponent_color = (original_side == Color::White) ? Color::Black : Color::White;
        bool king_in_check_after = SqAttacked(original_king_sq, test_pos, opponent_color);
        
        test_pos.undo_move();
        
        bool legal = is_legal_move(pos, move);
        std::cout << (i+1) << ". from=" << move.get_from() << " to=" << move.get_to() 
                  << " legal=" << (legal ? "YES" : "NO")
                  << " check_before=" << (currently_in_check ? "YES" : "NO")
                  << " check_after=" << (king_in_check_after ? "YES" : "NO") << std::endl;
    }
    
    // Let's test with a simple perft that ignores legality checks temporarily  
    std::cout << "\nTesting with pseudo-legal moves only (no legality check):" << std::endl;
    uint64_t pseudo_perft1 = pseudo_moves.v.size();  // Just count pseudo-legal moves
    std::cout << "Pseudo-perft(1) = " << pseudo_perft1 << " (generated pseudo-legal moves)" << std::endl;
    
    // Generate legal moves
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);
    std::cout << "Generated " << legal_moves.v.size() << " legal moves" << std::endl;
    
    // Show all legal moves for verification
    std::cout << "\nAll legal moves found:" << std::endl;
    for (size_t i = 0; i < legal_moves.v.size(); ++i) {
        const auto& move = legal_moves.v[i];
        std::cout << (i+1) << ". from=" << move.get_from() << " to=" << move.get_to();
        if (move.is_castle()) std::cout << " (castle)";
        if (move.is_promotion()) std::cout << " (promotion)";
        if (move.is_en_passant()) std::cout << " (en passant)";
        std::cout << std::endl;
    }
    
    // Test the known perft results for Kiwipete
    uint64_t perft1 = perft(pos, 1);
    uint64_t perft2 = perft(pos, 2);
    
    std::cout << "Perft(1) = " << perft1 << " (expected 48)" << std::endl;
    std::cout << "Perft(2) = " << perft2 << " (expected 2039)" << std::endl;
    
    EXPECT_EQ(perft1, 48u);   // Depth 1: 48 moves
    EXPECT_EQ(perft2, 2039u); // Depth 2: 2039 nodes
}
