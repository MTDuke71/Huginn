/**
 * @file test_randomized_invariants.cpp
 * @brief BACKLOG #60 remaining item: randomized legal make/unmake/null-move
 *        invariants.
 *
 * Hand-picked make/unmake regressions only ever exercise the specific moves
 * someone thought to write down. These tests instead take random legal-move
 * walks from several structurally different seed positions (opening,
 * midgame with castling rights, dense tactics, near-promotion endgames) and
 * assert, at every ply:
 *   - Position::is_consistent() holds before and after every MakeMove /
 *     TakeMove / MakeNullMove / TakeNullMove (catches internal desyncs
 *     between the bitboards, king_sq cache, and Zobrist key).
 *   - Unwinding a walk via TakeMove restores the EXACT FEN + Zobrist key at
 *     every ply, not just the final one (catches an undo bug that happens
 *     to cancel out over a short fixed sequence but not over an arbitrary
 *     one).
 *   - A MakeNullMove/TakeNullMove round trip is a no-op wherever it's legal
 *     (side to move not in check).
 *
 * Fixed RNG seed (matches the project's existing convention in
 * test_sq_attacked_performance.cpp) for reproducible failures.
 */
#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"
#include "attack_detection.hpp"
#include <random>
#include <string>
#include <vector>

namespace {

// Structurally varied seeds: startpos, Kiwipete (castling rights all sides,
// en passant, promotions reachable), an active king endgame, a position one
// ply from promotion on both sides, and a dense middlegame with a queenside
// promotion already on the board -- between them the random walks exercise
// castling (both sides, both wings), en passant, captures, promotions
// (including underpromotion via movegen's normal ordering), and checks.
constexpr const char* kSeedFens[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
};

// FEN + Zobrist key together: FEN alone can't catch a corrupted key (the
// exact class of bug the incremental Zobrist update in MakeMove/TakeMove
// could introduce), so both must round-trip exactly.
std::string snapshot(const Position& pos) {
    return pos.to_fen() + " z=" + std::to_string(pos.zobrist_key);
}

} // namespace

class RandomizedInvariantsTest : public ::testing::Test {
protected:
    std::mt19937 rng{42};
};

TEST_F(RandomizedInvariantsTest, MakeUnmakeRestoresExactStateAcrossRandomWalks) {
    constexpr int kWalksPerSeed = 200;
    constexpr int kMaxPly = 40;

    for (const char* fen : kSeedFens) {
        Position root;
        ASSERT_TRUE(root.set_from_fen(fen)) << "seed FEN: " << fen;

        for (int walk = 0; walk < kWalksPerSeed; ++walk) {
            Position pos = root;
            std::vector<std::string> trail{snapshot(pos)};

            int reached_ply = 0;
            for (; reached_ply < kMaxPly; ++reached_ply) {
                S_MOVELIST moves;
                generate_legal_moves(pos, moves);
                if (moves.count == 0) break;  // checkmate/stalemate: end this walk

                std::uniform_int_distribution<int> pick(0, moves.count - 1);
                S_MOVE mv = moves.moves[pick(rng)];

                std::string reason;
                ASSERT_TRUE(pos.is_consistent(&reason))
                    << "seed \"" << fen << "\" walk " << walk << ": inconsistent BEFORE ply " << reached_ply << ": " << reason;

                ASSERT_EQ(pos.MakeMove(mv), 1)
                    << "seed \"" << fen << "\" walk " << walk
                    << ": a move generate_legal_moves produced was rejected by MakeMove at ply " << reached_ply;

                ASSERT_TRUE(pos.is_consistent(&reason))
                    << "seed \"" << fen << "\" walk " << walk << ": inconsistent AFTER ply " << reached_ply << ": " << reason;
                trail.push_back(snapshot(pos));
            }

            // Unwind the entire walk, checking exact restoration at every ply
            // (not just after the last TakeMove) -- an undo bug that only
            // shows up a few plies deep would be invisible to a check that
            // only compares the fully-unwound end state to the start state.
            for (int ply = reached_ply - 1; ply >= 0; --ply) {
                pos.TakeMove();
                std::string reason;
                EXPECT_TRUE(pos.is_consistent(&reason))
                    << "seed \"" << fen << "\" walk " << walk << ": inconsistent after TakeMove to ply " << ply << ": " << reason;
                EXPECT_EQ(snapshot(pos), trail[static_cast<size_t>(ply)])
                    << "seed \"" << fen << "\" walk " << walk << ": TakeMove did not exactly restore ply " << ply;
            }
        }
    }
}

TEST_F(RandomizedInvariantsTest, NullMoveRoundTripIsANoOpWhereverLegal) {
    constexpr int kWalksPerSeed = 100;
    constexpr int kMaxPly = 20;

    for (const char* fen : kSeedFens) {
        Position root;
        ASSERT_TRUE(root.set_from_fen(fen)) << "seed FEN: " << fen;

        for (int walk = 0; walk < kWalksPerSeed; ++walk) {
            Position pos = root;

            for (int ply = 0; ply < kMaxPly; ++ply) {
                const int king_sq = pos.king_sq[static_cast<int>(pos.side_to_move)];
                const bool in_check = king_sq >= 0 &&
                    Huginn::SqAttackedBB(king_sq, pos, !pos.side_to_move);

                if (!in_check) {
                    const std::string before = snapshot(pos);
                    pos.MakeNullMove();
                    std::string reason;
                    EXPECT_TRUE(pos.is_consistent(&reason))
                        << "seed \"" << fen << "\" walk " << walk << " ply " << ply << " (after null): " << reason;
                    pos.TakeNullMove();
                    EXPECT_TRUE(pos.is_consistent(&reason))
                        << "seed \"" << fen << "\" walk " << walk << " ply " << ply << " (after un-null): " << reason;
                    EXPECT_EQ(snapshot(pos), before)
                        << "seed \"" << fen << "\" walk " << walk << " ply " << ply
                        << ": MakeNullMove/TakeNullMove was not a no-op";
                }

                S_MOVELIST moves;
                generate_legal_moves(pos, moves);
                if (moves.count == 0) break;  // checkmate/stalemate: end this walk
                std::uniform_int_distribution<int> pick(0, moves.count - 1);
                ASSERT_EQ(pos.MakeMove(moves.moves[pick(rng)]), 1);
            }
        }
    }
}
