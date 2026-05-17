// test_position_invariant.cpp
//
// BACKLOG #26 follow-up: catch any desync between Position::board64[]
// (the piece-on-square cache) and the underlying piece_bitboards. The
// perft suite happens to pass even when board64 is wrong because perft
// only counts move sequences — it never compares board64 against the
// bitboards. This file walks a move tree on diverse FENs and, after
// every MakeMove and TakeMove, asserts that for every 64-square index
// the cached piece matches the piece derived from piece_bitboards.
//
// If board64 ever lies about a square, this test fails. The bug
// surfaces in real games (via wrong capture targets in MakeMove's
// "if (!is_none(at(to))) clear_piece(to)" path, or via wrong piece
// types fed to zobrist XOR, or via mis-scored MVV-LVA) but doesn't
// surface in perft or in startpos depth-N bench because those paths
// never hit promotion/EP/castling/special-undo combinations that
// stress every Position mutator.
//
// Test FENs:
//   - startpos             (sanity, all normal moves)
//   - Kiwipete             (classic perft stress test: many captures,
//                           castles, promotions all available)
//   - promotion-rich       (white about to promote with capture choices)
//   - en-passant-now       (EP capture available next move)
//   - both-sides-castle    (kingside + queenside available for both)

#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"
#include "init.hpp"

// Derive piece-at-square the slow way from the piece bitboards, then
// compare against the cached value. Returns true on a fully consistent
// board64; on mismatch returns false and fills *bad_sq / *cached /
// *derived for diagnostic output.
static bool board64_matches_bitboards(const Position& pos,
                                      int* bad_sq, Piece* cached, Piece* derived) {
    for (int s64 = 0; s64 < 64; ++s64) {
        const uint64_t bit = 1ULL << s64;
        Piece bb_derived = Piece::None;
        if (pos.occupied_bitboard & bit) {
            const int c = (pos.color_bitboards[0] & bit) ? 0 : 1;
            for (int t = int(PieceType::Pawn); t <= int(PieceType::King); ++t) {
                if (pos.piece_bitboards[c][t] & bit) {
                    bb_derived = make_piece(Color(c), PieceType(t));
                    break;
                }
            }
        }
        if (pos.board64[s64] != bb_derived) {
            *bad_sq  = s64;
            *cached  = pos.board64[s64];
            *derived = bb_derived;
            return false;
        }
    }
    return true;
}

// Walk every legal move from `pos` up to `depth` plies, asserting the
// board64 invariant after every MakeMove and after every TakeMove.
// On the first mismatch, fails the GoogleTest and stops recursion.
static void invariant_walk(Position& pos, int depth, const char* fen) {
    if (depth == 0) return;

    S_MOVELIST list;
    generate_legal_moves(pos, list);

    for (int i = 0; i < list.count; ++i) {
        const S_MOVE move = list.moves[i];
        if (pos.MakeMove(move) != 1) continue;  // illegal — self-rolled-back

        // Post-MakeMove invariant
        {
            int bad_sq; Piece cached; Piece derived;
            if (!board64_matches_bitboards(pos, &bad_sq, &cached, &derived)) {
                ADD_FAILURE() << "board64 desync after MakeMove. FEN: " << fen
                              << "\n  sq64=" << bad_sq
                              << " cached=" << int(cached)
                              << " derived=" << int(derived)
                              << "\n  move from=" << move.get_from()
                              << " to=" << move.get_to()
                              << " promo=" << int(move.get_promoted())
                              << " ep=" << move.is_en_passant()
                              << " castle=" << move.is_castle();
                pos.TakeMove();
                return;
            }
        }

        invariant_walk(pos, depth - 1, fen);
        if (::testing::Test::HasFailure()) {
            pos.TakeMove();
            return;
        }

        pos.TakeMove();

        // Post-TakeMove invariant
        {
            int bad_sq; Piece cached; Piece derived;
            if (!board64_matches_bitboards(pos, &bad_sq, &cached, &derived)) {
                ADD_FAILURE() << "board64 desync after TakeMove. FEN: " << fen
                              << "\n  sq64=" << bad_sq
                              << " cached=" << int(cached)
                              << " derived=" << int(derived)
                              << "\n  reverted move from=" << move.get_from()
                              << " to=" << move.get_to()
                              << " promo=" << int(move.get_promoted())
                              << " ep=" << move.is_en_passant()
                              << " castle=" << move.is_castle();
                return;
            }
        }
    }
}

struct InvariantCase { const char* name; const char* fen; int depth; };

class PositionInvariantTest : public ::testing::TestWithParam<InvariantCase> {
protected:
    void SetUp() override { Huginn::init(); }
};

TEST_P(PositionInvariantTest, Board64MatchesBitboardsAfterEveryMove) {
    const InvariantCase& tc = GetParam();
    Position pos;
    ASSERT_TRUE(pos.set_from_fen(tc.fen)) << "Bad FEN: " << tc.fen;

    // Pre-walk invariant: FEN load must produce a consistent board64.
    {
        int bad_sq; Piece cached; Piece derived;
        ASSERT_TRUE(board64_matches_bitboards(pos, &bad_sq, &cached, &derived))
            << "board64 desync after set_from_fen. FEN: " << tc.fen
            << " sq64=" << bad_sq << " cached=" << int(cached)
            << " derived=" << int(derived);
    }

    invariant_walk(pos, tc.depth, tc.fen);
}

INSTANTIATE_TEST_SUITE_P(
    AllCases, PositionInvariantTest,
    ::testing::Values(
        // Normal play — all ordinary moves, no special cases.
        InvariantCase{"startpos", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4},
        // Kiwipete — the gold-standard perft stress test. Many captures,
        // both castles available, an EP-available state reachable.
        InvariantCase{"kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3},
        // Promotion-rich: white pawns on rank 7 with capture options.
        InvariantCase{"promo", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 4},
        // EP available next move (white double-pushed e2-e4).
        InvariantCase{"en_passant", "rnbqkbnr/pp1ppppp/8/2pP4/8/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 2", 3},
        // Both sides have kingside + queenside castle.
        InvariantCase{"castles", "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 3}
    ),
    [](const ::testing::TestParamInfo<InvariantCase>& info) {
        return std::string(info.param.name);
    }
);
