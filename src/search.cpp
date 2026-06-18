#include "search.hpp"
#include "evaluation.hpp"
#include "chess_types.hpp"
#include "attack_detection.hpp"
#include "attack_tables.hpp"
#include "bitboard.hpp"
#include "square.hpp"
#include "input_checking.hpp"
#include "msvc_optimizations.hpp"
#include "see.hpp"
#include <cassert>
#include <cstdlib>
#include <cmath>     // for std::log used by the LMR-table initializer
#include <array>
#include <iostream>
#include <algorithm>
#include <iomanip>  // For std::setw
#include <string>
#include <unordered_set>  // For PV repetition truncation in searchPosition

// Backlog #13 bisection result (2026-05-06): ply tracking + TT-mate is
// shipped; counter-move is gated off pending a separate re-attempt.
//
// Bisection results vs huginn_t2 (100g, tc=10+0.1):
//   2a (both off, ply-tracking only):    +10 ± 61 Elo, LOS 63%   (neutral)
//   2b (counter-move @ 15K only):        +96 ± 66 Elo, LOS 99.9%
//   2c (TT-mate only, this build):      +104 ± 62 Elo, LOS 99.98%
//   2d (both on):                        +31 ± 52 Elo, LOS 88%   (anti-compound)
//   attempt 2 (both on, c-m at 700K):   -114 ± 64 Elo, LOS 0.01% (700K was the bug)
//
//   ENABLE_PLY_TRACKED_COUNTERMOVE: counter-move read in ordering + write on
//     beta cutoff. Score 15K (below promotions, above history). Gated off
//     because the counter-move + TT-mate combination anti-compounds — see
//     the followup BACKLOG entry to revisit with a different score slot.
//   ENABLE_PLY_TRACKED_TT_MATE: TT-store / TT-probe mate-distance adjustment,
//     mate-leaf encoding via info.ply (consistent under check extensions),
//     and a cap clamp so store_score stays inside (-MATE, MATE) after the
//     ply-add. The cap clamp is what rescued attempt 2's TT pollution case.
#ifndef ENABLE_PLY_TRACKED_COUNTERMOVE
#define ENABLE_PLY_TRACKED_COUNTERMOVE 1  // BACKLOG #15 re-attempt vs t7 @ score 1500
#endif
#ifndef ENABLE_PLY_TRACKED_TT_MATE
#define ENABLE_PLY_TRACKED_TT_MATE 1
#endif
// ENABLE_PRUNING_STATS: emit per-depth "info string ... Futility/Razoring cuts"
// diagnostics during search. Off by default — this is engine debug telemetry,
// not information a UCI GUI consumes. Build with -DENABLE_PRUNING_STATS=1 to
// re-enable. The counters themselves (info.futility_cuts / razoring_cuts) are
// always maintained; only the printing is gated.
#ifndef ENABLE_PRUNING_STATS
#define ENABLE_PRUNING_STATS 0
#endif
// ENABLE_TAPERED_EVAL: BACKLOG #35. Replace the hard `is_endgame` boolean
// (which flips king-PST + mobility weight at a 1150-material threshold) with
// a smooth game-phase blend in [0,256]. The mg/eg accumulators differ ONLY in
// the king PST and mobility weight — exactly the two terms the boolean gated —
// so flag-OFF is byte-identical to the pre-#35 eval, and flag-ON only smooths
// those two transitions. Tapered material values are a SEPARATE later step.
#ifndef ENABLE_TAPERED_EVAL
#define ENABLE_TAPERED_EVAL 1
#endif
// ENABLE_TAPERED_MATERIAL: BACKLOG #35 Experiment 2. Give the eg accumulator
// its own endgame piece values (PIECE_VALUES_EG) instead of reusing MG, so
// material worth tapers by phase too. Meant to pair with ENABLE_TAPERED_EVAL
// (the eg sum only reaches the score via the phase blend). Flag-off → eg uses
// MG material → byte-identical to the tapered-eval foundation (baseline-t10).
#ifndef ENABLE_TAPERED_MATERIAL
#define ENABLE_TAPERED_MATERIAL 1
#endif
// ENABLE_KING_SAFETY: BACKLOG #35 Experiment 3. Multi-attacker king-ring danger
// + open-file shelter, added to the MG accumulator only so it tapers out toward
// the endgame (the #2 attempt regressed -126 Elo by NOT tapering — KS poisons
// endgames where the king should be active). Requires ENABLE_TAPERED_EVAL.
#ifndef ENABLE_KING_SAFETY
#define ENABLE_KING_SAFETY 1
#endif

// ENABLE_NMP_VERIFICATION: BACKLOG #43 sub-lever 1. Guard the null-move cutoff
// against zugzwang false-positives. Huginn's NMP is flat R=4 with NO
// verification — a genuine over-pruning / tactical-leak suspect (the Stash
// v12/v13 changelog added exactly this). On a null fail-high at high depth AND
// low non-pawn material (zugzwang is an endgame phenomenon — the material gate
// keeps the re-search off the middlegame, where an unconditional version cost
// +53% nodes), re-search the actual position (null undone) at reduced depth
// with null pruning disabled; only take the cutoff if it ALSO fails high.
// DEFAULT OFF — REJECTED (2026-06-16). The bundled +62 vs t16 was all the #44
// repetition fix: the clean isolation test (NMP-off vs NMP-on, both carrying
// #44) put NMP-off AHEAD at +14.6 ± 16.6, LOS ~96% — i.e. NMP-on is
// neutral-to-slightly-harmful, no measurable benefit. Per the complexity gate
// (don't ship unproven complexity), the verification stays OFF; the code is
// kept behind the flag for reference / a future re-formulation. baseline-t17 is
// the #44 fix alone. Build the ON arm with -DENABLE_NMP_VERIFICATION=1.
#ifndef ENABLE_NMP_VERIFICATION
#define ENABLE_NMP_VERIFICATION 0
#endif

// ENABLE_MATE_DISTANCE_PRUNING: BACKLOG #43 sub-lever 3. At node entry, clamp
// the [alpha,beta] window to the mate envelope: we can do no better than
// mating immediately (MATE - ply) nor worse than being mated immediately
// (-MATE + ply). If the window collapses, this node can't beat an already-known
// mate, so cut. Standard, sound, and cheap — never changes the chosen move in a
// non-mate search; in mate searches it steers toward shorter mates and saves
// nodes. DEFAULT OFF pending the fixed-depth + fixed-time SPRT (complexity
// gate); build the ON arm with -DENABLE_MATE_DISTANCE_PRUNING=1.
#ifndef ENABLE_MATE_DISTANCE_PRUNING
#define ENABLE_MATE_DISTANCE_PRUNING 0
#endif
// ENABLE_SEARCH_INTEGRITY_ASSERTS: BACKLOG #37 diagnostic. In debug or
// explicitly-instrumented builds, assert after search make/unmake operations
// that the Position caches still agree with the per-piece bitboards and full
// Zobrist recomputation. Release default is OFF: no Elo/NPS or behavior change.
#ifndef ENABLE_SEARCH_INTEGRITY_ASSERTS
#ifdef DEBUG
#define ENABLE_SEARCH_INTEGRITY_ASSERTS 1
#else
#define ENABLE_SEARCH_INTEGRITY_ASSERTS 0
#endif
#endif

namespace Huginn {

static inline void assert_search_position_integrity(const Position& pos, const char* context) {
#if ENABLE_SEARCH_INTEGRITY_ASSERTS
    std::string reason;
    if (!pos.is_consistent(&reason)) {
        std::cerr << "Position integrity failure";
        if (context && *context) std::cerr << " (" << context << ")";
        std::cerr << ": " << reason << "\nFEN: " << pos.to_fen() << std::endl;
        std::abort();
    }
#else
    (void)pos;
    (void)context;
#endif
}

#if ENABLE_SEARCH_INTEGRITY_ASSERTS
struct SearchPositionSnapshot {
    Color side_to_move;
    int ep_square;
    uint8_t castling_rights;
    uint16_t halfmove_clock;
    uint16_t fullmove_number;
    std::array<int, 2> king_sq;
    std::array<std::array<Bitboard, int(PieceType::_Count)>, 2> piece_bitboards;
    std::array<Bitboard, 2> color_bitboards;
    Bitboard occupied_bitboard;
    uint64_t zobrist_key;
    std::array<int, 2> material_score;
    int ply;
};

static inline SearchPositionSnapshot capture_search_position(const Position& pos) {
    return {pos.side_to_move, pos.ep_square, pos.castling_rights,
            pos.halfmove_clock, pos.fullmove_number, pos.king_sq,
            pos.piece_bitboards, pos.color_bitboards, pos.occupied_bitboard,
            pos.zobrist_key, pos.material_score, pos.ply};
}

static inline void assert_search_position_unchanged(
        const Position& pos, const SearchPositionSnapshot& before, const char* context) {
    if (pos.side_to_move != before.side_to_move ||
        pos.ep_square != before.ep_square ||
        pos.castling_rights != before.castling_rights ||
        pos.halfmove_clock != before.halfmove_clock ||
        pos.fullmove_number != before.fullmove_number ||
        pos.king_sq != before.king_sq ||
        pos.piece_bitboards != before.piece_bitboards ||
        pos.color_bitboards != before.color_bitboards ||
        pos.occupied_bitboard != before.occupied_bitboard ||
        pos.zobrist_key != before.zobrist_key ||
        pos.material_score != before.material_score ||
        pos.ply != before.ply) {
        std::cerr << "Position changed across search boundary";
        if (context && *context) std::cerr << " (" << context << ")";
        std::cerr << "\nFEN: " << pos.to_fen() << std::endl;
        std::abort();
    }
    assert_search_position_integrity(pos, context);
}
#else
struct SearchPositionSnapshot {};
static inline SearchPositionSnapshot capture_search_position(const Position&) { return {}; }
static inline void assert_search_position_unchanged(
        const Position&, const SearchPositionSnapshot&, const char*) {}
#endif

// Contempt — penalty applied to draw scores from the side-to-move's
// perspective. Biases the search away from drawing lines: when the
// engine has a non-draw alternative, it'll prefer that alternative
// over a draw if the alternative is within `CONTEMPT` cp of equal.
// Filed as BACKLOG #16, motivated by 2026-05-08 game where Huginn
// (losing) accepted a fragile 0.00 repetition that black escaped
// from. Single static value is the standard simple implementation;
// can be made asymmetric (only at root) or material-dependent
// later if needed.
constexpr int CONTEMPT = 25;  // cp — tunable; gauntlet to validate

// If the root side is already clearly ahead, do not let an immediate
// threefold repetition score as a pleasant contempt draw. Losing and equal
// positions keep the normal repetition behavior so the engine can still
// rescue bad games.
constexpr int WINNING_REPETITION_AVOID_THRESHOLD = 300;
constexpr int WINNING_REPETITION_DRAW_SCORE = -800;

// LMR reduction table indexed by (depth, move-index). Reduction grows
// with both depth and move number. Formula: R = log(d) * log(m) / 2,
// truncated. Matches the MTLChess src/search.zig:63 table. Computed
// once at static initialization via lambda IIFE so it lives in .data
// (no per-call cost).
static const std::array<std::array<int, 64>, 64> LMR_TABLE = []() {
    std::array<std::array<int, 64>, 64> t{};
    for (int d = 1; d < 64; ++d) {
        for (int m = 1; m < 64; ++m) {
            t[d][m] = static_cast<int>(std::log(double(d)) * std::log(double(m)) / 2.0);
        }
    }
    return t;
}();

// VICE Tutorial: Mirror arrays for evaluation symmetry testing
// mirror64: maps 64-square indices to their vertical mirror (rank 1 <-> rank 8)
static const int mirror64[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,  // rank 1 -> rank 8
    48, 49, 50, 51, 52, 53, 54, 55,  // rank 2 -> rank 7
    40, 41, 42, 43, 44, 45, 46, 47,  // rank 3 -> rank 6
    32, 33, 34, 35, 36, 37, 38, 39,  // rank 4 -> rank 5
    24, 25, 26, 27, 28, 29, 30, 31,  // rank 5 -> rank 4
    16, 17, 18, 19, 20, 21, 22, 23,  // rank 6 -> rank 3
     8,  9, 10, 11, 12, 13, 14, 15,  // rank 7 -> rank 2
     0,  1,  2,  3,  4,  5,  6,  7   // rank 8 -> rank 1
};

// Game phase in [0,256]: 256 = full opening material, 0 = bare kings (+pawns).
// Standard non-pawn phase weights (N=1, B=1, R=2, Q=4); start position sums to
// 24. Used by the tapered eval (#35) to blend mg/eg term values smoothly
// instead of the hard `is_endgame` boolean flip.
static inline int game_phase_256(const Position& pos) {
    int npm =
        popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)] |
                 pos.piece_bitboards[int(Color::Black)][int(PieceType::Knight)]) * 1 +
        popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Bishop)] |
                 pos.piece_bitboards[int(Color::Black)][int(PieceType::Bishop)]) * 1 +
        popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Rook)] |
                 pos.piece_bitboards[int(Color::Black)][int(PieceType::Rook)]) * 2 +
        popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Queen)] |
                 pos.piece_bitboards[int(Color::Black)][int(PieceType::Queen)]) * 4;
    if (npm > 24) npm = 24;  // early-promotion safety: extra queens cap at full phase
    return (npm * 256 + 12) / 24;  // +12 rounds to nearest
}

#if ENABLE_KING_SAFETY
// King-safety MG score, white-positive (#35 Experiment 3). For each side,
// computes a "danger" = non-linear attacker pressure on the king ring
// (fires on >= 1 attacker; #9 round 7 removed the old gate) plus an open-file shelter
// penalty, then returns Black's danger minus White's (White gains when Black's
// king is unsafe). The caller adds this to the MG accumulator ONLY, so the
// tapered blend fades it to zero in the endgame. Fully colour-symmetric (ring
// + file occupancy carry no rank direction), so eval mirror-symmetry holds.
static int king_safety_white_mg(const Position& pos) {
    const uint64_t occ = pos.occupied_bitboard;

    auto danger_for = [&](Color c) -> int {
        const int ksq = pos.king_sq[int(c)];
        if (ksq < 0) return 0;
        const Color them = (c == Color::White) ? Color::Black : Color::White;
        const uint64_t zone = king_attacks[ksq] | (1ULL << ksq);
        const auto& ep = pos.piece_bitboards[int(them)];

        int units = 0;
        uint64_t bb;

        bb = ep[int(PieceType::Knight)];
        while (bb) { int s = pop_lsb(bb); int h = popcount(knight_attacks[s] & zone);
            units += EvalParams::KS_ATTACK_WEIGHT[int(PieceType::Knight)] * h; }
        bb = ep[int(PieceType::Bishop)];
        while (bb) { int s = pop_lsb(bb); int h = popcount(bishop_attacks(s, occ) & zone);
            units += EvalParams::KS_ATTACK_WEIGHT[int(PieceType::Bishop)] * h; }
        bb = ep[int(PieceType::Rook)];
        while (bb) { int s = pop_lsb(bb); int h = popcount(rook_attacks(s, occ) & zone);
            units += EvalParams::KS_ATTACK_WEIGHT[int(PieceType::Rook)] * h; }
        bb = ep[int(PieceType::Queen)];
        while (bb) { int s = pop_lsb(bb); int h = popcount(queen_attacks(s, occ) & zone);
            units += EvalParams::KS_ATTACK_WEIGHT[int(PieceType::Queen)] * h; }

        // No min-attacker gate (#9 round 7): danger fires whenever units > 0.
        // The square still concentrates danger on heavy / multi-piece attacks,
        // but the term is now non-zero on most middlegame positions so the
        // Texel tuner can constrain the weights.
        int danger = units * units / EvalParams::KS_ATTACK_DIVISOR;
        if (danger > EvalParams::KS_ATTACK_CAP) danger = EvalParams::KS_ATTACK_CAP;

        // Shelter: open files on/adjacent to the king's file (no own pawn).
        const uint64_t own_pawns = pos.piece_bitboards[int(c)][int(PieceType::Pawn)];
        const int kf = ksq & 7;
        const int lo = (kf > 0) ? kf - 1 : 0;
        const int hi = (kf < 7) ? kf + 1 : 7;
        for (int f = lo; f <= hi; ++f) {
            if ((own_pawns & EvalParams::FILE_MASKS[f]) == 0)
                danger += EvalParams::KS_OPEN_FILE_PENALTY;
        }
        return danger;
    };

    return danger_for(Color::Black) - danger_for(Color::White);
}
#endif // ENABLE_KING_SAFETY

// Function to swap piece colors using the bit-packed Piece enum
Piece swapPieceColor(Piece piece) {
    if (piece == Piece::None || piece == Piece::Offboard) return piece;
    
    // Extract color and piece type
    Color color = color_of(piece);
    PieceType type = type_of(piece);
    
    // Flip the color
    Color new_color = (color == Color::White) ? Color::Black : Color::White;
    
    // Create the new piece with flipped color
    return make_piece(new_color, type);
}

int Engine::evaluate(const Position& pos) {
    // VICE Part 82: Check for material draw first (2:03)
    if (pos.get_white_pawns() == 0 && pos.get_black_pawns() == 0 && MaterialDraw(pos)) {
        return -CONTEMPT; // Insufficient material draw — contempt-biased (BACKLOG #16)
    }
    
    // VICE Part 56: Basic Evaluation with piece-square tables
    int score = 0;
    
    // VICE Part 82: Use pre-existing material tracking for endgame detection
    // This is much more efficient than manually counting material
    [[maybe_unused]] int total_material = pos.get_total_material();
    [[maybe_unused]] bool is_endgame = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD);
#if ENABLE_TAPERED_EVAL
    const int phase = game_phase_256(pos);  // 256 = opening, 0 = endgame (#35)
#endif

    // Material + PST accumulated into separate middlegame (mg) and endgame (eg)
    // sums (white-positive). They diverge ONLY in the king PST table — the one
    // term the legacy `is_endgame` boolean flipped — so the flag-off combine
    // (`is_endgame ? eg : mg`) is byte-identical to the pre-#35 eval, while the
    // flag-on combine blends them smoothly by game phase. Material stays MG for
    // both sums for now (tapered material values are a separate #35 step).
    int mg_pst = 0, eg_pst = 0;
    for (int color = 0; color <= 1; ++color) {
        Color piece_color = static_cast<Color>(color);
        // PST tables are stored from White's perspective; Black pieces look up
        // the vertically-mirrored square. mirror_square_64(sq) == sq ^ 56 (XOR
        // flips the 3 rank bits, leaves the file bits), so the whole mirror is
        // a single XOR with a per-color mask — no per-piece function call (that
        // call was ~2.5% of total time before this). Mask is hoisted out of the
        // inner loop since it depends only on color.
        const int sq_flip = (piece_color == Color::Black) ? 56 : 0;
        for (int piece_type = int(PieceType::Pawn); piece_type <= int(PieceType::King); ++piece_type) {
            PieceType pt = static_cast<PieceType>(piece_type);
            uint64_t bb = pos.piece_bitboards[color][piece_type];
            int mg_material = PIECE_VALUES_MG[piece_type];
#if ENABLE_TAPERED_MATERIAL
            int eg_material = PIECE_VALUES_EG[piece_type];  // #35 Exp 2
#else
            int eg_material = mg_material;
#endif
            while (bb) {
                int sq64 = pop_lsb(bb);
                int table_index = sq64 ^ sq_flip;
                int mg_val, eg_val;
                if (pt == PieceType::King) {
                    mg_val = mg_material + EvalParams::KING_TABLE[table_index];
                    eg_val = eg_material + EvalParams::KING_TABLE_ENDGAME[table_index];
                } else {
                    // Separate MG/EG piece-square tables (#9 round 2, tapered PSTs).
                    int pst_mg = 0, pst_eg = 0;
                    switch (pt) {
                        case PieceType::Pawn:   pst_mg = EvalParams::PAWN_TABLE[table_index];   pst_eg = EvalParams::PAWN_TABLE_EG[table_index];   break;
                        case PieceType::Knight: pst_mg = EvalParams::KNIGHT_TABLE[table_index]; pst_eg = EvalParams::KNIGHT_TABLE_EG[table_index]; break;
                        case PieceType::Bishop: pst_mg = EvalParams::BISHOP_TABLE[table_index]; pst_eg = EvalParams::BISHOP_TABLE_EG[table_index]; break;
                        case PieceType::Rook:   pst_mg = EvalParams::ROOK_TABLE[table_index];   pst_eg = EvalParams::ROOK_TABLE_EG[table_index];   break;
                        case PieceType::Queen:  pst_mg = EvalParams::QUEEN_TABLE[table_index];  pst_eg = EvalParams::QUEEN_TABLE_EG[table_index];  break;
                        default: break;
                    }
                    mg_val = mg_material + pst_mg;
                    eg_val = eg_material + pst_eg;
                }
                if (piece_color == Color::White) { mg_pst += mg_val; eg_pst += eg_val; }
                else                             { mg_pst -= mg_val; eg_pst -= eg_val; }
            }
        }
    }
    
    // VICE Part 80: Enhanced pawn structure evaluation with pre-computed masks
    // Evaluate isolated pawns (2:13, 3:07) and passed pawns (2:21, 4:25)
    int pawn_structure_score = 0;
    
    // Get bitboards for efficient pawn structure analysis
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();

    // Connected-pawn sets (#9 round 4): a pawn is connected if it is phalanx
    // (own pawn on an adjacent file, same rank) or supported (defended by an
    // own pawn). Computed set-wise once, membership-tested per pawn below.
    const uint64_t FILE_A_BB = EvalParams::FILE_MASKS[0];
    const uint64_t FILE_H_BB = EvalParams::FILE_MASKS[7];
    const uint64_t w_pawn_attacks = ((white_pawns & ~FILE_A_BB) << 7) | ((white_pawns & ~FILE_H_BB) << 9);
    const uint64_t b_pawn_attacks = ((black_pawns & ~FILE_A_BB) >> 9) | ((black_pawns & ~FILE_H_BB) >> 7);
    const uint64_t w_neighbors = ((white_pawns & ~FILE_H_BB) << 1) | ((white_pawns & ~FILE_A_BB) >> 1);
    const uint64_t b_neighbors = ((black_pawns & ~FILE_H_BB) << 1) | ((black_pawns & ~FILE_A_BB) >> 1);
    const uint64_t w_connected = white_pawns & (w_neighbors | w_pawn_attacks);
    const uint64_t b_connected = black_pawns & (b_neighbors | b_pawn_attacks);

    // Iterate white pawns directly via bitboard
    uint64_t bb = white_pawns;
    while (bb) {
        int sq64 = pop_lsb(bb);
        int file_idx = sq64 & 7;
        int rank_idx = sq64 >> 3;

        // Isolated / connected / backward are mutually exclusive: connected
        // implies a neighbor on an adjacent file (not isolated) at the same
        // rank or behind (not backward); isolated pawns can't be backward by
        // definition here (no neighbors at all — already penalized).
        if ((white_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx]) == 0) {
            pawn_structure_score -= EvalParams::ISOLATED_PAWN_PENALTY;
        } else if ((1ULL << sq64) & w_connected) {
            mg_pst += EvalParams::CONNECTED_PAWN_BONUS_MG[rank_idx];
            eg_pst += EvalParams::CONNECTED_PAWN_BONUS_EG[rank_idx];
        } else {
            // Backward: no own pawn on an adjacent file at the same rank or
            // behind, and the stop square is controlled by an enemy pawn
            // (enemy pawn on an adjacent file two ranks ahead).
            const uint64_t behind_or_eq = (1ULL << (8 * (rank_idx + 1))) - 1;
            if ((white_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx] & behind_or_eq) == 0 &&
                rank_idx + 2 <= 7 &&
                (black_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx] & EvalParams::RANK_MASKS[rank_idx + 2]) != 0) {
                mg_pst -= EvalParams::BACKWARD_PAWN_PENALTY_MG;
                eg_pst -= EvalParams::BACKWARD_PAWN_PENALTY_EG;
            }
        }
        if ((black_pawns & EvalParams::WHITE_PASSED_PAWN_MASKS[sq64]) == 0) {
            pawn_structure_score += EvalParams::PASSED_PAWN_BONUS[rank_idx];
        }
    }

    // Iterate black pawns directly via bitboard
    bb = black_pawns;
    while (bb) {
        int sq64 = pop_lsb(bb);
        int file_idx = sq64 & 7;
        int rank_idx = sq64 >> 3;

        if ((black_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx]) == 0) {
            pawn_structure_score += EvalParams::ISOLATED_PAWN_PENALTY;
        } else if ((1ULL << sq64) & b_connected) {
            mg_pst -= EvalParams::CONNECTED_PAWN_BONUS_MG[7 - rank_idx];
            eg_pst -= EvalParams::CONNECTED_PAWN_BONUS_EG[7 - rank_idx];
        } else {
            const uint64_t ahead_or_eq = ~0ULL << (8 * rank_idx);
            if ((black_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx] & ahead_or_eq) == 0 &&
                rank_idx - 2 >= 0 &&
                (white_pawns & EvalParams::ISOLATED_PAWN_MASKS[file_idx] & EvalParams::RANK_MASKS[rank_idx - 2]) != 0) {
                mg_pst += EvalParams::BACKWARD_PAWN_PENALTY_MG;
                eg_pst += EvalParams::BACKWARD_PAWN_PENALTY_EG;
            }
        }
        if ((white_pawns & EvalParams::BLACK_PASSED_PAWN_MASKS[sq64]) == 0) {
            int mirror_rank = 7 - rank_idx;
            pawn_structure_score -= EvalParams::PASSED_PAWN_BONUS[mirror_rank];
        }
    }

    // Doubled pawn penalty: each extra own pawn on the same file is a liability.
    // Two pawns on a file → −P; three pawns → −2P; etc.
    for (int f = 0; f < 8; ++f) {
        uint64_t file_mask = EvalParams::FILE_MASKS[f];
        int wpc = popcount(white_pawns & file_mask);
        int bpc = popcount(black_pawns & file_mask);
        if (wpc > 1) pawn_structure_score -= (wpc - 1) * EvalParams::DOUBLED_PAWN_PENALTY;
        if (bpc > 1) pawn_structure_score += (bpc - 1) * EvalParams::DOUBLED_PAWN_PENALTY;
    }

    score += pawn_structure_score;
    
    // VICE Part 81: Open and semi-open file bonuses for rooks and queens
    // Evaluate rooks and queens on open files (no pawns) or semi-open files (no own pawns)
    int file_bonus_score = 0;
    
    uint64_t all_pawns = white_pawns | black_pawns;
    
    // Sum the file bonus for one piece set as a positive magnitude; the
    // caller applies the sign (+ for White, − for Black). Open = no pawns
    // on the file at all; semi-open = no *own* pawns on the file.
    auto file_bonus = [&](uint64_t pieces_bb, uint64_t own_pawns,
                          int open_bonus, int semi_bonus) -> int {
        int s = 0;
        while (pieces_bb) {
            int sq64 = pop_lsb(pieces_bb);
            uint64_t file_mask = EvalParams::FILE_MASKS[sq64 & 7];
            if ((all_pawns & file_mask) == 0) {
                s += open_bonus;
            } else if ((own_pawns & file_mask) == 0) {
                s += semi_bonus;
            }
        }
        return s;
    };

    const auto& wbb = pos.piece_bitboards[int(Color::White)];
    const auto& bbb = pos.piece_bitboards[int(Color::Black)];

    file_bonus_score += file_bonus(wbb[int(PieceType::Rook)],  white_pawns,
                                   EvalParams::ROOK_OPEN_FILE_BONUS,  EvalParams::ROOK_SEMI_OPEN_FILE_BONUS);
    file_bonus_score -= file_bonus(bbb[int(PieceType::Rook)],  black_pawns,
                                   EvalParams::ROOK_OPEN_FILE_BONUS,  EvalParams::ROOK_SEMI_OPEN_FILE_BONUS);
    file_bonus_score += file_bonus(wbb[int(PieceType::Queen)], white_pawns,
                                   EvalParams::QUEEN_OPEN_FILE_BONUS, EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS);
    file_bonus_score -= file_bonus(bbb[int(PieceType::Queen)], black_pawns,
                                   EvalParams::QUEEN_OPEN_FILE_BONUS, EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS);

    score += file_bonus_score;

    // Outposts (#9 round 8 candidate): knights/bishops on advanced holes,
    // supported by own pawns, where enemy pawns on adjacent files cannot
    // advance to challenge the square. Tapered and colour-symmetric.
    {
        auto outpost_count = [&](Color us, PieceType pt) -> int {
            const bool white = (us == Color::White);
            const int color = int(us);
            const uint64_t own_pawn_attacks = white ? w_pawn_attacks : b_pawn_attacks;
            const uint64_t enemy_pawns = white ? black_pawns : white_pawns;

            int count = 0;
            uint64_t pieces = pos.piece_bitboards[color][int(pt)];
            while (pieces) {
                const int sq64 = pop_lsb(pieces);
                const int file = sq64 & 7;
                const int rank = sq64 >> 3;
                const uint64_t bit = 1ULL << sq64;

                if ((own_pawn_attacks & bit) == 0) continue;
                if (white) {
                    if (rank < EvalParams::WHITE_KNIGHT_OUTPOST_MIN_RANK) continue;
                } else {
                    if (rank > EvalParams::BLACK_KNIGHT_OUTPOST_MAX_RANK) continue;
                }

                uint64_t adjacent_files = 0ULL;
                if (file > 0) adjacent_files |= EvalParams::FILE_MASKS[file - 1];
                if (file < 7) adjacent_files |= EvalParams::FILE_MASKS[file + 1];

                uint64_t challenge_ranks = 0ULL;
                if (white) {
                    challenge_ranks = (rank < 7) ? (~0ULL << (8 * (rank + 1))) : 0ULL;
                } else {
                    challenge_ranks = (rank > 0) ? ((1ULL << (8 * rank)) - 1ULL) : 0ULL;
                }
                if (enemy_pawns & adjacent_files & challenge_ranks) continue;

                ++count;
            }
            return count;
        };

        const int white_knight_outposts = outpost_count(Color::White, PieceType::Knight);
        const int black_knight_outposts = outpost_count(Color::Black, PieceType::Knight);
        const int white_bishop_outposts = outpost_count(Color::White, PieceType::Bishop);
        const int black_bishop_outposts = outpost_count(Color::Black, PieceType::Bishop);

        mg_pst += (white_knight_outposts - black_knight_outposts) * EvalParams::KNIGHT_OUTPOST_BONUS_MG;
        eg_pst += (white_knight_outposts - black_knight_outposts) * EvalParams::KNIGHT_OUTPOST_BONUS_EG;
        mg_pst += (white_bishop_outposts - black_bishop_outposts) * EvalParams::BISHOP_OUTPOST_BONUS_MG;
        eg_pst += (white_bishop_outposts - black_bishop_outposts) * EvalParams::BISHOP_OUTPOST_BONUS_EG;
    }
    
    // VICE Part 83: Bishop pair bonus
    int white_bishops = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Bishop)]);
    int black_bishops = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Bishop)]);

    if (white_bishops >= 2) {
        score += EvalParams::BISHOP_PAIR_BONUS;
    }
    if (black_bishops >= 2) {
        score -= EvalParams::BISHOP_PAIR_BONUS;
    }

    // Rook on the relative 7th rank (#9 round 5). White's 7th = rank index 6;
    // Black's 7th = rank index 1. Gated on a target — enemy king on its back
    // rank, or enemy pawns on the rook's rank — so a rook on the 7th in a bare
    // endgame isn't over-rewarded. Tapered into the mg/eg accumulators
    // (white-positive); colour-symmetric, so eval mirror-symmetry holds.
    {
        const uint64_t RANK7 = EvalParams::RANK_MASKS[6];  // White's 7th
        const uint64_t RANK2 = EvalParams::RANK_MASKS[1];  // Black's 7th
        const int wk_rank = pos.king_sq[int(Color::White)] >> 3;
        const int bk_rank = pos.king_sq[int(Color::Black)] >> 3;
        if (bk_rank == 7 || (black_pawns & RANK7)) {
            int n = popcount(wbb[int(PieceType::Rook)] & RANK7);
            mg_pst += n * EvalParams::ROOK_ON_7TH_MG;
            eg_pst += n * EvalParams::ROOK_ON_7TH_EG;
        }
        if (wk_rank == 0 || (white_pawns & RANK2)) {
            int n = popcount(bbb[int(PieceType::Rook)] & RANK2);
            mg_pst -= n * EvalParams::ROOK_ON_7TH_MG;
            eg_pst -= n * EvalParams::ROOK_ON_7TH_EG;
        }
    }

    // Threats (#9 round 6): bonus per enemy piece attacked by a cheaper / more
    // dangerous attacker. Computed per side and folded white-positive into the
    // tapered accumulators. Reuses the pawn-attack spans from the pawn-structure
    // block; minor/rook attack unions are computed here. Colour-symmetric (each
    // side uses its own pawn direction), so eval mirror-symmetry holds.
    {
        const uint64_t occ = pos.occupied_bitboard;
        auto threats_for = [&](Color us, uint64_t pawn_att, int& mg, int& eg) {
            const Color them = (us == Color::White) ? Color::Black : Color::White;
            const auto& mp = pos.piece_bitboards[int(us)];
            const auto& ep = pos.piece_bitboards[int(them)];

            uint64_t minor_att = 0, rook_att = 0, b;
            b = mp[int(PieceType::Knight)]; while (b) minor_att |= knight_attacks[pop_lsb(b)];
            b = mp[int(PieceType::Bishop)]; while (b) minor_att |= bishop_attacks(pop_lsb(b), occ);
            b = mp[int(PieceType::Rook)];   while (b) rook_att  |= rook_attacks(pop_lsb(b), occ);

            const uint64_t e_minor = ep[int(PieceType::Knight)] | ep[int(PieceType::Bishop)];
            const uint64_t e_rook  = ep[int(PieceType::Rook)];
            const uint64_t e_queen = ep[int(PieceType::Queen)];

            const int pm = popcount(pawn_att  & e_minor);
            const int pr = popcount(pawn_att  & e_rook);
            const int pq = popcount(pawn_att  & e_queen);
            const int mr = popcount(minor_att & e_rook);
            const int mq = popcount(minor_att & e_queen);
            const int rq = popcount(rook_att  & e_queen);

            mg += pm * EvalParams::THREAT_PAWN_ON_MINOR_MG  + pr * EvalParams::THREAT_PAWN_ON_ROOK_MG
                + pq * EvalParams::THREAT_PAWN_ON_QUEEN_MG  + mr * EvalParams::THREAT_MINOR_ON_ROOK_MG
                + mq * EvalParams::THREAT_MINOR_ON_QUEEN_MG + rq * EvalParams::THREAT_ROOK_ON_QUEEN_MG;
            eg += pm * EvalParams::THREAT_PAWN_ON_MINOR_EG  + pr * EvalParams::THREAT_PAWN_ON_ROOK_EG
                + pq * EvalParams::THREAT_PAWN_ON_QUEEN_EG  + mr * EvalParams::THREAT_MINOR_ON_ROOK_EG
                + mq * EvalParams::THREAT_MINOR_ON_QUEEN_EG + rq * EvalParams::THREAT_ROOK_ON_QUEEN_EG;
        };
        int wmg = 0, weg = 0, bmg = 0, beg = 0;
        threats_for(Color::White, w_pawn_attacks, wmg, weg);
        threats_for(Color::Black, b_pawn_attacks, bmg, beg);
        mg_pst += wmg - bmg;
        eg_pst += weg - beg;
    }

    // -----------------------------------------------------------------
    // Mobility: count squares each non-pawn, non-king piece can move to
    // (excluding squares occupied by own pieces). Weighted per phase.
    // Pawns are excluded (their move semantics aren't "attack squares")
    // and kings are excluded (their squares are evaluated elsewhere).
    // -----------------------------------------------------------------
    // Signed mobility "units" (white count − black count); the per-phase weight
    // is applied at the mg/eg combine below so mobility tapers smoothly too.
    int mobility_units = 0;
    {
        const uint64_t occ = pos.occupied_bitboard;

        for (int color = 0; color <= 1; ++color) {
            const uint64_t own = pos.color_bitboards[color];
            int count = 0;

            // Knights
            uint64_t bb = pos.piece_bitboards[color][int(PieceType::Knight)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(knight_attacks[sq] & ~own);
            }
            // Bishops
            bb = pos.piece_bitboards[color][int(PieceType::Bishop)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(bishop_attacks(sq, occ) & ~own);
            }
            // Rooks
            bb = pos.piece_bitboards[color][int(PieceType::Rook)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(rook_attacks(sq, occ) & ~own);
            }
            // Queens
            bb = pos.piece_bitboards[color][int(PieceType::Queen)];
            while (bb) {
                int sq = pop_lsb(bb);
                count += popcount(queen_attacks(sq, occ) & ~own);
            }

            if (color == int(Color::White)) mobility_units += count;
            else                            mobility_units -= count;
        }
    }

    // Combine the phase-dependent material+PST+mobility sums. `score` already
    // holds the phase-neutral terms (pawn structure, file bonuses, bishop pair).
#if ENABLE_TAPERED_EVAL
    // Smooth blend: mg at full opening (phase=256), eg at bare kings (phase=0).
    int mg_total = mg_pst + mobility_units * EvalParams::MOBILITY_WEIGHT_DEFAULT;
    int eg_total = eg_pst + mobility_units * EvalParams::MOBILITY_WEIGHT_ENDGAME;
#if ENABLE_KING_SAFETY
    // MG-only: added before the blend so it fades to 0 as phase -> 0 (#35 Exp 3).
    mg_total += king_safety_white_mg(pos);
#endif
    score += (mg_total * phase + eg_total * (256 - phase)) / 256;
#else
    // Legacy hard boolean: pick one side of the blend at the 1150 threshold.
    const int mob_weight = is_endgame ? EvalParams::MOBILITY_WEIGHT_ENDGAME
                                      : EvalParams::MOBILITY_WEIGHT_DEFAULT;
    score += (is_endgame ? eg_pst : mg_pst) + mobility_units * mob_weight;
#endif

    // Return from current side's perspective (negate if black to move),
    // then add a tempo bonus (initiative goes to whoever moves next).
    int sided_score = (pos.side_to_move == Color::White) ? score : -score;
    return sided_score + EvalParams::TEMPO_BONUS;
}

// VICE Part 82/83: Material draw detection - Fixed to be more conservative
// Checks if the position is a theoretical draw based on insufficient material
bool Engine::MaterialDraw(const Position& pos) {
    int white_rooks   = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Rook)]);
    int black_rooks   = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Rook)]);
    int white_queens  = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Queen)]);
    int black_queens  = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Queen)]);
    int white_bishops = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Bishop)]);
    int black_bishops = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Bishop)]);
    int white_knights = popcount(pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)]);
    int black_knights = popcount(pos.piece_bitboards[int(Color::Black)][int(PieceType::Knight)]);
    
    // If either side has rooks or queens, not a draw
    if (white_rooks > 0 || black_rooks > 0 || white_queens > 0 || black_queens > 0) {
        return false;
    }
    
    // Only minor pieces remain (bishops and knights)
    int white_pieces = white_bishops + white_knights;
    int black_pieces = black_bishops + black_knights;
    
    // Classic insufficient material cases:
    // 1. K vs K
    if (white_pieces == 0 && black_pieces == 0) {
        return true;
    }
    
    // 2. K+N vs K or K+B vs K
    if ((white_pieces <= 1 && black_pieces == 0) || (black_pieces <= 1 && white_pieces == 0)) {
        return true;
    }
    
    // 3. K+B vs K+B with bishops on same color squares (more complex, skip for now)
    // 4. K+N vs K+N is generally drawn but can have winning positions
    
    // Be conservative - only claim draw for the most obvious cases
    return false;
}

// Helper functions for evaluation (Part 56)
// (mirror_square_64 retired — the PST lookup now mirrors Black squares inline
// via `sq ^ 56`; see evaluate(). No other callers existed.)

// VICE Tutorial: Mirror Board function for evaluation testing
// Creates a mirrored copy of the position for symmetry testing
Position Engine::mirrorBoard(const Position& pos) {
    Position mirrored_pos;
    mirrored_pos.reset();

    // Mirror all pieces on the board (write through the bitboard-aware set_sq64())
    for (int sq64 = 0; sq64 < 64; ++sq64) {
        Piece original_piece = pos.at_sq64(sq64);
        if (is_none(original_piece)) continue;

        mirrored_pos.set_sq64(mirror64[sq64], swapPieceColor(original_piece));
    }
    
    // Flip the side to move
    mirrored_pos.side_to_move = (pos.side_to_move == Color::White) ? Color::Black : Color::White;
    
    // Mirror castling permissions
    mirrored_pos.castling_rights = 0;
    if (pos.castling_rights & CASTLE_WK) mirrored_pos.castling_rights |= CASTLE_BK;
    if (pos.castling_rights & CASTLE_WQ) mirrored_pos.castling_rights |= CASTLE_BQ;
    if (pos.castling_rights & CASTLE_BK) mirrored_pos.castling_rights |= CASTLE_WK;
    if (pos.castling_rights & CASTLE_BQ) mirrored_pos.castling_rights |= CASTLE_WQ;
    
    // Mirror en passant square
    if (pos.ep_square >= 0) {
        mirrored_pos.ep_square = mirror64[pos.ep_square];  // ep_square is sq64
    } else {
        mirrored_pos.ep_square = -1;
    }
    
    // Copy other fields
    mirrored_pos.halfmove_clock = pos.halfmove_clock;
    mirrored_pos.fullmove_number = pos.fullmove_number;
    
    // Rebuild derived state for the mirrored position
    mirrored_pos.rebuild_counts();
    mirrored_pos.update_zobrist_key();
    
    return mirrored_pos;
}

std::string Engine::format_uci_score(int score, Color side_to_move) const {
    // Convert engine score to proper UCI format
    // UCI specification: 
    // - cp <x>: score from engine's point of view in centipawns
    // - mate <y>: mate in y MOVES (not plies). If engine is getting mated, use negative y
    
    // The search returns scores from the side-to-move's perspective:
    // - Positive score means side-to-move is winning
    // - Negative score means side-to-move is losing
    
    // MATE = 29000, so scores close to +/-MATE are mate scores
    if (score > MATE - 100) {
        // Positive mate score: side_to_move (engine) is mating opponent
        int mate_in_plies = MATE - score;
        int mate_in_moves = (mate_in_plies + 1) / 2;  // Convert plies to moves
        return "mate " + std::to_string(mate_in_moves);
    } else if (score < -MATE + 100) {
        // Negative mate score: side_to_move (engine) is being mated
        int mate_in_plies = MATE + score;  // score is negative, so this is MATE - abs(score)
        int mate_in_moves = (mate_in_plies + 1) / 2;  // Convert plies to moves
        return "mate -" + std::to_string(mate_in_moves);  // Negative because engine is being mated
    } else {
        // Regular centipawn score from engine's perspective
        return "cp " + std::to_string(score);
    }
}

std::string Engine::move_to_uci(const S_MOVE& move) {
    if (move.move == 0) return "0000";
    
    std::string result;
    
    int from = move.get_from();
    int to = move.get_to();
    
    // from/to are sq64 indices: file = sq & 7, rank = sq >> 3
    if (from < 0 || from >= 64 || to < 0 || to >= 64) {
        return "0000";
    }
    File from_file = File(from & 7);
    Rank from_rank = Rank(from >> 3);
    File to_file = File(to & 7);
    Rank to_rank = Rank(to >> 3);
    
    result += char('a' + int(from_file));
    result += char('1' + int(from_rank));
    result += char('a' + int(to_file));
    result += char('1' + int(to_rank));
    
    // Add promotion piece if applicable
    if (move.is_promotion()) {
        PieceType promo = move.get_promoted();
        switch (promo) {
            case PieceType::Queen:  result += 'q'; break;
            case PieceType::Rook:   result += 'r'; break;
            case PieceType::Bishop: result += 'b'; break;
            case PieceType::Knight: result += 'n'; break;
            default: break;
        }
    }
    
    return result;
}

// Count how many times the current position key appears in the reachable
// halfmove-clock-bounded history window (including the current position).
// 1 = no prior match, 2 = one prior match (single repetition), 3+ = threefold.
static int repetition_count_in_history(const Position& pos) {
    // Conservative repetition detection to avoid false positives in mate searches
    // Only check for repetition in actual game positions, not during deep search

    // Don't check for repetition if move history is too short
    if (pos.ply < 6) {
        return 1; // Need at least 6 plies for meaningful repetition check
    }

    // Be very conservative - only detect clear 3-fold repetitions
    uint64_t current_key = pos.zobrist_key;
    int repetition_count = 1; // Count current position

    // Bound the lookback by the halfmove (fifty-move) clock rather than a
    // fixed window. Any irreversible move (capture / pawn move / castle)
    // resets halfmove_clock and makes the current position structurally
    // unreachable again, so a matching zobrist key within the last
    // `halfmove_clock` plies is necessarily a *true* repetition — never a
    // false positive. The old fixed 12-ply window silently missed slow
    // long-period shuffles (e.g. a K+Q vs K cycle 16-22 plies wide, which
    // let the engine draw a won game; see BACKLOG #28 case intel-R8).
    // BUG FIX (#44): use the CURRENT path length (pos.ply), NOT
    // move_history.size(). move_history is a reusable buffer grown to the
    // deepest ply the search ever reached (MakeMove resize-grows it, TakeMove
    // never shrinks it), so its size over-counts during search — entries past
    // pos.ply are stale undos from deeper/sibling lines. Using size() slid the
    // scan window off the real predecessors, so a true 3-fold read as a
    // non-repetition at deep iterations and the engine drew won games. The
    // current path is exactly move_history[0 .. pos.ply).
    const int history_len = pos.ply;
    const int scan_plies = std::min(history_len,
                                    static_cast<int>(pos.halfmove_clock));
    int start_check = std::max(0, history_len - scan_plies);

    for (int index = start_check; index < history_len - 1; ++index) {
        if (current_key == pos.move_history[index].zobrist_key) {
            repetition_count++;
        }
    }

    return repetition_count;
}

// Simple repetition detection - VICE tutorial style (made static as per Part 55)
bool Engine::isRepetition(const Position& pos) {
    const int repetition_count = repetition_count_in_history(pos);
    
    // Only return true for definite 3-fold repetition to be safe
    return repetition_count >= 3;
}

// Clear search tables - reset history and killers
void Engine::clear_search_tables() {
    // Age search history array instead of clearing completely
    // Aging preserves recent learning while gradually fading old patterns
    for (int piece = 0; piece < 13; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            // Reduce history scores by 75% (age by factor of 4)
            // This preserves 25% of existing knowledge while making room for new learning
            search_history[piece][sq] = search_history[piece][sq] / 4;
        }
    }
    
    // Clear search killers array (4:37)
    for (int depth = 0; depth < 64; ++depth) {
        search_killers[depth][0] = S_MOVE();  // Clear first killer
        search_killers[depth][1] = S_MOVE();  // Clear second killer
    }
    
    // Clear counter-moves table
    for (int from_sq = 0; from_sq < 64; ++from_sq) {
        for (int to_sq = 0; to_sq < 64; ++to_sq) {
            counter_moves[from_sq][to_sq] = S_MOVE();
        }
    }

#if ENABLE_CONTINUATION_HISTORY
    // BACKLOG #3: age the continuation-history table by /4 each new search,
    // mirroring search_history above — preserve 25% of cross-search learning
    // while making room for new. Construction zero-inits, so the first search
    // ages zeros (still zero); no nondeterminism (cf. #30). ~692K int16 ops
    // once per search — negligible against Mnps search.
    for (size_t k = 0; k < CH_SIZE; ++k) {
        continuation_history[k] = static_cast<int16_t>(continuation_history[k] / 4);
    }
#endif
}

// PV table helper functions
void Engine::store_pv_move(uint64_t position_key, const S_MOVE& move) {
    pv_table.store_move(position_key, move);
}

// Update search history when move improves alpha (3:55)
void Engine::update_search_history(const Position& pos, const S_MOVE& move, int depth) {
    if (move.move == 0) return;
    
    // Get piece and destination square
    int from = move.get_from();
    int to = move.get_to();
    
    if (from < 0 || from >= 64 || to < 0 || to >= 64) return;
    
    Piece piece = pos.at_sq64(from);
    int piece_index = static_cast<int>(piece) % 13;  // Ensure valid index
    
    // Increase history score for this piece-to-square combination
    search_history[piece_index][to] += depth * depth;  // Deeper moves get higher bonus
}

// Penalize history for moves that fail to improve alpha (negative history scoring)
void Engine::penalize_search_history(const Position& pos, const S_MOVE& move, int depth) {
    if (move.move == 0) return;
    
    // Get piece and destination square
    int from = move.get_from();
    int to = move.get_to();
    
    if (from < 0 || from >= 64 || to < 0 || to >= 64) return;
    
    Piece piece = pos.at_sq64(from);
    int piece_index = static_cast<int>(piece) % 13;  // Ensure valid index
    
    // Decrease history score for this piece-to-square combination
    search_history[piece_index][to] -= depth * depth;  // Penalize with same magnitude as bonus
}

// Apply periodic aging to prevent history scores from becoming too large
void Engine::age_search_history() {
    for (int piece = 0; piece < 13; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            // Reduce all history scores by 12.5% to maintain discrimination
            search_history[piece][sq] = (search_history[piece][sq] * 7) / 8;
        }
    }
}

// Update killer moves when move causes beta cutoff (4:37)  
void Engine::update_killer_moves(const S_MOVE& move, int depth) {
    if (move.move == 0 || depth < 0 || depth >= 64) return;
    
    // Only store non-capture moves as killers
    if (!move.is_capture()) {
        // If this move isn't already first killer, shift and add
        if (search_killers[depth][0].move != move.move) {
            search_killers[depth][1] = search_killers[depth][0];  // Second = old first
            search_killers[depth][0] = move;                      // First = new move
        }
    }
}

// Update counter-move table when move causes beta cutoff
void Engine::update_counter_move(const S_MOVE& previous_move, const S_MOVE& counter_move) {
    // Validate move parameters
    if (previous_move.move == 0 || counter_move.move == 0) return;
    if (counter_move.is_capture()) return;  // Only store quiet moves as counter-moves
    
    int from_sq = previous_move.get_from();
    int to_sq = previous_move.get_to();
    
    // Validate square indices (sq64: must be 0-63)
    if (from_sq < 0 || from_sq >= 64 || to_sq < 0 || to_sq >= 64) return;
    
    // Store the counter-move for this [from][to] combination
    counter_moves[from_sq][to_sq] = counter_move;
}

// Get counter-move for the opponent's last move
S_MOVE Engine::get_counter_move(const S_MOVE& previous_move) const {
    // Validate move parameter
    if (previous_move.move == 0) return S_MOVE();
    
    int from_sq = previous_move.get_from();
    int to_sq = previous_move.get_to();
    
    // Validate square indices
    if (from_sq < 0 || from_sq >= 64 || to_sq < 0 || to_sq >= 64) {
        return S_MOVE();
    }
    
    // Return stored counter-move, or empty move if none stored
    return counter_moves[from_sq][to_sq];
}

#if ENABLE_CONTINUATION_HISTORY
// BACKLOG #3: 1-ply continuation history. Both helpers take `pos` at the
// current node (after TakeMove / before MakeMove), where the parent move's
// piece sits on prev.get_to() and the current move's piece on move.get_from().
//
// Index validity: prev/move squares are sq64; piece_index uses the same
// `static_cast<int>(piece) % 13` convention as butterfly history. A piece of
// None on prev.get_to() (shouldn't happen for a real parent move, but guard
// anyway) maps to index 0 and is harmless.
namespace {
inline int ch_piece_index(Piece p) { return static_cast<int>(p) % 13; }
}

void Engine::update_continuation_history(const Position& pos, const S_MOVE& prev,
                                         const S_MOVE& move, int bonus) {
    if (prev.move == 0 || move.move == 0) return;
    int prev_to = prev.get_to();
    int from = move.get_from();
    int to = move.get_to();
    if (prev_to < 0 || prev_to >= 64 || from < 0 || from >= 64 || to < 0 || to >= 64) return;

    int pp = ch_piece_index(pos.at_sq64(prev_to));   // parent mover, now resting on prev_to
    int cp = ch_piece_index(pos.at_sq64(from));       // current mover, back on its from-square

    // Clamp into int16 range to avoid overflow under repeated +/-depth^2 bonuses.
    size_t idx = ch_index(pp, prev_to, cp, to);
    int updated = continuation_history[idx] + bonus;
    if (updated > 32000) updated = 32000;
    else if (updated < -32000) updated = -32000;
    continuation_history[idx] = static_cast<int16_t>(updated);
}

int Engine::get_continuation_history(const Position& pos, const S_MOVE& prev,
                                     const S_MOVE& move) const {
    if (prev.move == 0 || move.move == 0) return 0;
    int prev_to = prev.get_to();
    int from = move.get_from();
    int to = move.get_to();
    if (prev_to < 0 || prev_to >= 64 || from < 0 || from >= 64 || to < 0 || to >= 64) return 0;

    int pp = ch_piece_index(pos.at_sq64(prev_to));
    int cp = ch_piece_index(pos.at_sq64(from));
    return continuation_history[ch_index(pp, prev_to, cp, to)];
}
#endif

// Initialize MVV-LVA (Most Valuable Victim, Least Valuable Attacker) scoring table
void Engine::init_mvv_lva() {
    // INDEPENDENT ordering values — not a view of eval's PIECE_VALUES_MG.
    // MVV-LVA only needs the relative victim>attacker ordering, so absolute
    // magnitudes barely matter and this table is NOT meant to track eval's
    // Texel/SPSA tuning. It starts at the same numbers by coincidence but is
    // free to diverge (and already does: King=0 here, since king captures
    // are illegal, vs the 20000 sentinel eval/SEE use). Do not merge.
    int piece_values[7] = {
        0,    // None
        100,  // Pawn
        320,  // Knight
        330,  // Bishop
        500,  // Rook
        900,  // Queen
        0     // King (never captured -> 0, unlike eval/SEE)
    };
    
    // Initialize MVV-LVA scores
    // Higher scores = better captures to search first
    // Formula: (victim_value * 100) + (600 - attacker_value)
    // This prioritizes: valuable victims + cheap attackers
    for (int victim = 0; victim < 7; victim++) {
        for (int attacker = 0; attacker < 7; attacker++) {
            if (victim == 0) {
                // No victim = not a capture
                mvv_lva_scores[victim][attacker] = 0;
            } else {
                // Valuable victim + cheap attacker = high score
                // Example: Pawn(100) takes Queen(900) = (900 * 100) + (600 - 100) = 90,500
                // Example: Queen(900) takes Pawn(100) = (100 * 100) + (600 - 900) = 9,700
                mvv_lva_scores[victim][attacker] = (piece_values[victim] * 100) + (600 - piece_values[attacker]);
            }
        }
    }
}

// Get MVV-LVA score for a capture move
int Engine::get_mvv_lva_score(PieceType victim, PieceType attacker) const {
    int victim_index = static_cast<int>(victim);
    int attacker_index = static_cast<int>(attacker);

    assert(victim_index >= 0 && victim_index < 7);
    assert(attacker_index >= 0 && attacker_index < 7);
    __assume(victim_index >= 0 && victim_index < 7);
    __assume(attacker_index >= 0 && attacker_index < 7);

    return mvv_lva_scores[victim_index][attacker_index];
}

// Order moves using MVV-LVA and other heuristics
void Engine::order_moves(std::vector<S_MOVE>& moves, const Position& pos) const {
    // Assign scores to each move for ordering
    for (auto& move : moves) {
        int score = 0;
        
        if (move.is_capture()) {
            // Captures: Use MVV-LVA scoring
            PieceType victim = move.get_captured();
            
            // Get the attacking piece type from the position
            int from_sq = move.get_from();
            Piece attacking_piece = pos.at_sq64(from_sq);
            PieceType attacker = type_of(attacking_piece);
            
            score = get_mvv_lva_score(victim, attacker);
            
            // Bonus for en passant captures (always pawn takes pawn)
            if (move.is_en_passant()) {
                score += 10000;  // High priority for en passant
            }
            
        } else if (move.is_promotion()) {
            // Promotions: High priority, queen promotion highest
            PieceType promoted = move.get_promoted();
            switch (promoted) {
                case PieceType::Queen:  score = 90000; break;
                case PieceType::Rook:   score = 50000; break;
                case PieceType::Bishop: score = 33000; break;
                case PieceType::Knight: score = 32000; break;
                default: score = 25000; break;
            }
            
        } else {
            // Quiet moves: Lower priority
            // Could add killer moves, history heuristic here later
            score = 1000;  // Base score for quiet moves
        }
        
        move.score = score;
    }
    
    // Sort moves by score (highest first)
    std::sort(moves.begin(), moves.end(), [](const S_MOVE& a, const S_MOVE& b) {
        return a.score > b.score;
    });
}

// Order moves in S_MOVELIST using MVV-LVA and other heuristics
void Engine::order_moves(S_MOVELIST& move_list, const Position& pos) const {
    // Assign scores to each move for ordering
    for (int i = 0; i < move_list.count; i++) {
        S_MOVE& move = move_list.moves[i];
        int score = 0;
        
        if (move.is_capture()) {
            // Captures: Use MVV-LVA scoring
            PieceType victim = move.get_captured();
            
            // Get the attacking piece type from the position
            int from_sq = move.get_from();
            Piece attacking_piece = pos.at_sq64(from_sq);
            PieceType attacker = type_of(attacking_piece);
            
            score = get_mvv_lva_score(victim, attacker);
            
            // Bonus for en passant captures (always pawn takes pawn)
            if (move.is_en_passant()) {
                score += 10000;  // High priority for en passant
            }
            
        } else if (move.is_promotion()) {
            // Promotions: High priority, queen promotion highest
            PieceType promoted = move.get_promoted();
            switch (promoted) {
                case PieceType::Queen:  score = 90000; break;
                case PieceType::Rook:   score = 50000; break;
                case PieceType::Bishop: score = 33000; break;
                case PieceType::Knight: score = 32000; break;
                default: score = 25000; break;
            }
            
        } else {
            // Quiet moves: Lower priority
            // Could add killer moves, history heuristic here later
            score = 1000;  // Base score for quiet moves
        }
        
        move.score = score;
    }
    
    // Sort moves by score (highest first) using C-style array sort
    std::sort(&move_list.moves[0], &move_list.moves[move_list.count], 
              [](const S_MOVE& a, const S_MOVE& b) {
                  return a.score > b.score;
              });
}

// VICE Part 62: Pick Next Move - Select best move from remaining moves
// This is more efficient than sorting all moves upfront
int Engine::pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos, const SearchInfo& info, int depth, const S_MOVE& iid_move) const {
    // For the first call (move_num == 0), score all moves using VICE Part 64 ordering
    if (move_num == 0) {
        // VICE Part 84: Check for transposition table move (highest priority)
        int tt_score;
        uint8_t tt_depth, tt_node_type; 
        uint32_t tt_best_move;
        bool has_tt_move = tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
        
        // Validate TT move to ensure it's reasonable for this position
        bool tt_move_valid = false;
        if (has_tt_move && tt_best_move != 0) {
            // Basic sanity checks on the TT move
            S_MOVE tt_move;
            tt_move.move = static_cast<int>(tt_best_move);
            
            int from = tt_move.get_from();
            int to = tt_move.get_to();
            
            // Validate square bounds and that there's a piece to move
            if (from >= 0 && from < 64 && to >= 0 && to < 64 &&
                pos.at_sq64(from) != Piece::None &&
                color_of(pos.at_sq64(from)) == pos.side_to_move) {
                tt_move_valid = true;
            }
        }
        
        // Get PV move for this position (if any)
        S_MOVE pv_move;
        bool has_pv_move = pv_table.probe_move(pos.zobrist_key, pv_move);
        
        // Score all moves for ordering
        for (int i = 0; i < move_list.count; i++) {
            S_MOVE& move = move_list.moves[i];
            int score = 0;
            
            // VICE Part 84: TT move gets absolute highest priority (3,000,000) - but only if validated
            if (tt_move_valid && move.move == static_cast<int>(tt_best_move)) {
                score = 3000000;
                
            // VICE Part 64: PV move gets second highest priority (2,000,000)
            } else if (has_pv_move && move.move == pv_move.move) {
                score = 2000000;
                
            // IID move gets third highest priority (1,500,000) - between PV and captures
            } else if (iid_move.move != 0 && move.move == iid_move.move) {
                score = 1500000;
                
            } else if (move.is_capture()) {
                // VICE Part 64: Captures get 1,000,000 + MVV-LVA score
                PieceType victim = move.get_captured();
                
                // Get the attacking piece type from the position
                int from_sq = move.get_from();
                Piece attacking_piece = pos.at_sq64(from_sq);
                PieceType attacker = type_of(attacking_piece);
                
                score = 1000000 + get_mvv_lva_score(victim, attacker);
                
                // Bonus for en passant captures (always pawn takes pawn)
                if (move.is_en_passant()) {
                    score += 10000;  // High priority for en passant
                }
                
            } else {
                // Check for killer moves (non-captures only)
                bool is_killer = false;
                if (depth >= 0 && depth < 64) {
                    // VICE Part 64: First killer = 900,000, Second killer = 800,000
                    if (search_killers[depth][0].move == move.move) {
                        score = 900000;
                        is_killer = true;
                    } else if (search_killers[depth][1].move == move.move) {
                        score = 800000;
                        is_killer = true;
                    }
                }
                
                // Check for counter-move (if not a killer move)
                bool is_counter_move = false;
#if ENABLE_PLY_TRACKED_COUNTERMOVE
                if (!is_killer && info.ply > 0 && info.ply < 64) {
                    // Get the previous move from search stack
                    S_MOVE previous_move = info.search_stack[info.ply - 1];
                    if (previous_move.move != 0) {
                        S_MOVE counter_move = get_counter_move(previous_move);
                        if (counter_move.move == move.move) {
                            // Slot just above the history range (~1K), below
                            // promotions (25K-90K). BACKLOG #15: 1500 beat 15000
                            // on t4 (+8.7 vs -10.4 Elo); re-testing 1500 on t7,
                            // whose stronger ordering favors a gentler bonus.
                            score = 1500;
                            is_counter_move = true;
                        }
                    }
                }
#endif
                
                if (!is_killer && !is_counter_move) {
                    if (move.is_promotion()) {
                        // Promotions: High priority, queen promotion highest
                        PieceType promoted = move.get_promoted();
                        switch (promoted) {
                            case PieceType::Queen:  score = 90000; break;
                            case PieceType::Rook:   score = 50000; break;
                            case PieceType::Bishop: score = 33000; break;
                            case PieceType::Knight: score = 32000; break;
                            default: score = 25000; break;
                        }
                    } else {
                        // VICE Part 64: History heuristic for remaining quiet moves
                        int from = move.get_from();
                        int to = move.get_to();
                        
                        if (from >= 0 && from < 64 && to >= 0 && to < 64) {
                            Piece piece = pos.at_sq64(from);
                            int piece_index = static_cast<int>(piece) % 13;
                            score = search_history[piece_index][to];  // History score
#if ENABLE_CONTINUATION_HISTORY
                            // BACKLOG #3: blend 1-ply continuation history into
                            // the quiet-move score (additive, same scale as
                            // butterfly history). Only the parent (ply-1) move
                            // conditions it; killers/counters/promotions above
                            // keep their fixed scores. Weight is a tuning knob.
                            if (info.ply > 0 && info.ply < 64) {
                                S_MOVE prev = info.search_stack[info.ply - 1];
                                int contrib = CONTHIST_ORDER_WEIGHT *
                                              get_continuation_history(pos, prev, move);
                                if (contrib > CONTHIST_ORDER_CAP) contrib = CONTHIST_ORDER_CAP;
                                else if (contrib < -CONTHIST_ORDER_CAP) contrib = -CONTHIST_ORDER_CAP;
                                score += contrib;
                            }
#endif
                        } else {
                            score = 1000;  // Base score for quiet moves
                        }
                    }
                }
            }
            
            move.score = score;
        }
    }
    
    // Find the best move from move_num onwards
    int best_score = -1;
    int best_index = move_num;
    
    for (int i = move_num; i < move_list.count; i++) {
        if (move_list.moves[i].score > best_score) {
            best_score = move_list.moves[i].score;
            best_index = i;
        }
    }
    
    // Swap the best move to the current position
    if (best_index != move_num) {
        S_MOVE temp = move_list.moves[move_num];
        move_list.moves[move_num] = move_list.moves[best_index];
        move_list.moves[best_index] = temp;
    }
    
    return best_score;
}

// VICE Part 55 - Search Function Definitions
// This implements the core search infrastructure following the VICE tutorial:
// - evalPosition: Position evaluation function 
// - checkup: Time management and GUI interrupt checking
// - clearForSearch: Initialize search tables before new search
// - AlphaBeta: Core recursive search with alpha-beta pruning
// - quiescence: Search only captures to handle horizon effect

// Position evaluation (0:34) - Returns score from current side's perspective
int Engine::evalPosition(const Position& pos) {
    // For now, use the existing evaluate function
    return evaluate(pos);
}

// Check time limits and GUI interrupts (1:34)
void Engine::checkup(SearchInfo& info) {
    // Check if we should stop due to time limit
    if (info.quit || info.stopped) return;

    // VICE Part 70: Check for GUI input during search (3:23).
    // The Windows console poll (GetNumberOfConsoleInputEvents / PeekConsoleInput)
    // is ~5us — profiling a `go depth` run showed ~6% of total time spent here
    // because checkup() fires every 2048 nodes. Time management needs that
    // cadence (steady_clock is cheap), but catching a "stop"/"quit" keystroke
    // does not, so the input poll is gated 16x coarser. INPUT_CHECK_MASK must be
    // a multiple of checkup()'s 2048-node interval so it still lands on a call.
    constexpr uint64_t INPUT_CHECK_MASK = 32767;  // poll stdin every ~32768 nodes
    if ((info.nodes & INPUT_CHECK_MASK) == 0 && input_is_waiting()) {
        read_input(info);
    }

    // Skip time management if this is a depth-only search (UCI go depth command)
    if (info.depth_only) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time);
    
    // Check time limit (if not infinite search)
    if (!info.infinite) {
        // Use SearchInfo stop_time if available, otherwise fall back to default
        auto stop_time = info.stop_time;
        if (stop_time == std::chrono::steady_clock::time_point{}) {
            // No stop_time set, use default 5 seconds
            stop_time = info.start_time + std::chrono::milliseconds(5000);
        }
        
        if (now >= stop_time) {
            info.stopped = true;
        }
    }
    
    // Note: Node counting is done in AlphaBeta and quiescence functions
}

// Clear search tables and PV before new search (2:25)
// VICE Part 57 - Clear To Search: Prepare engine for clean search
void Engine::clearForSearch(Engine& engine, SearchInfo& info) {
    // Clear the history and killers arrays (0:57)
    engine.clear_search_tables();
    
    // Clear the principal variation (PV) table (2:17)
    engine.pv_table.clear();
    
    // Reset the ply counter to zero (2:21)
    info.ply = 0;
    
    // Initialize start_time, set stop to zero, and reset nodes count (2:58)
    info.start_time = std::chrono::steady_clock::now();
    info.stopped = false;    // Set stop to zero (false)
    info.quit = false;       // Reset quit flag as well
    info.nodes = 0;          // Reset nodes count
    
    // Reset engine state for new search
    engine.should_stop = false;     // Reset stop flag
    engine.nodes_searched = 0;      // Reset nodes count
}

// Core AlphaBeta search function (2:58)
int Engine::AlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull, bool isRoot) {
    // Increment node count for every position visited (except root calls)
    if (!isRoot) {
        info.nodes++;
    }

    // Standard UCI `seldepth` tracking: max ply ever reached during the
    // current iteration. Quiescence enters from a leaf and recurses further
    // (incrementing info.ply each time), so AlphaBeta seeing ply 17 at
    // depth 0 means qsearch already pushed to ply 17 below — accurate to
    // within a few plies of true max. Cheap (one branch, one store, well
    // predicted), always on (standard info field, not diagnostic).
    if (info.ply > info.seldepth) info.seldepth = info.ply;

    // Triangular PV: this node starts with an empty line. Set this before any
    // early return (TT cutoff, repetition/50-move draw, leaf) so a parent that
    // reads pv_length[ply+1] after the child returns sees 0, not a stale value
    // left by a previously searched sibling subtree.
    if (info.ply < 64) {
        info.pv_length[info.ply] = 0;
    }

    // Priority 6 (PERFORMANCE_ARCHITECTURE_REVIEW): compute the static eval at
    // most once per node, lazily, and share it across every block that needs
    // it. The position doesn't change before the move loop, so reverse-futility,
    // futility, and razoring were each calling evalPosition() on the *same*
    // position — up to 3 full evals at the shallow nodes that dominate the tree
    // (plus the rare winning-single-rep probe). Accuracy-safe: identical value,
    // fewer calls.
    int static_eval = 0;
    bool has_static_eval = false;
    auto get_static_eval = [&]() -> int {
        if (!has_static_eval) {
            static_eval = evalPosition(pos);
            has_static_eval = true;
        }
        return static_eval;
    };

    // BACKLOG #28 Part 2: TT-safe repetition handling.
    // Repetition draw scores are path-dependent (history-sensitive), while TT
    // keys are path-independent (position-only). So for repetition-draw nodes,
    // return immediately before any TT probe/store interaction.
    if (!isRoot) {
        const int repetition_count = repetition_count_in_history(pos);

        // A true threefold is always a draw by rule — score it unconditionally.
        const bool threefold_repetition_draw = (repetition_count >= 3);

        // Zarkov-style single repetition: one prior key match past the root
        // neighborhood (ply > 2) is treated as a draw, but ONLY when the side
        // to move is clearly winning. The rule exists so the WON side routes
        // around shuffles toward the real win (BACKLOG #28 Part 2); confining
        // it to winning evals keeps that bug-fix (provably-thrown won games)
        // without meddling in equal/unclear positions, where scoring a lone
        // repetition as a draw could discard a legitimate resource. Eval is
        // probed only when a single-rep candidate actually exists, so the cost
        // is paid only on real history matches, not every node.
        bool winning_single_rep = false;
        if (!threefold_repetition_draw && repetition_count >= 2 && info.ply > 2) {
            winning_single_rep = (get_static_eval() >= WINNING_REPETITION_AVOID_THRESHOLD);
        }

        if (threefold_repetition_draw || winning_single_rep) {
            return -CONTEMPT; // Repetition draw — contempt-biased (BACKLOG #16)
        }
    }

    // BACKLOG #29: Fifty-move-rule draw — TT-safe, same rationale as
    // repetition. halfmove_clock is NOT part of the zobrist key (see the TB
    // note below), so a 50-move draw score is path-dependent and must be
    // returned before any TT probe/store, never cached. At 100 plies without
    // a pawn move or capture the position is a draw — UNLESS the side to move
    // is checkmated, which takes precedence over the rule. We therefore score
    // the draw only when the side to move is not in check; if it is in check
    // we fall through so normal movegen distinguishes checkmate from a legal
    // escape (in which case it is still really a draw, but that in-check-at-
    // ply-100 corner is rare enough to leave to the search).
    if (!isRoot && pos.halfmove_clock >= 100) {
        const int stm_king = pos.king_sq[int(pos.side_to_move)];
        const bool stm_in_check = (stm_king >= 0) &&
            SqAttackedBB(stm_king, pos, !pos.side_to_move);
        if (!stm_in_check) {
            return -CONTEMPT; // Fifty-move-rule draw — contempt-biased (BACKLOG #16)
        }
    }

#if ENABLE_MATE_DISTANCE_PRUNING
    // BACKLOG #43 sub-lever 3: mate-distance pruning. A mate found elsewhere in
    // the tree bounds what THIS node can return. Clamp the window to the mate
    // envelope and cut if it collapses; placed before the TT probe so a TT
    // cutoff respects the tightened bounds. Mate scores use MATE - info.ply.
    if (!isRoot) {
        const int mating_value = MATE - info.ply;   // best case: we deliver mate now
        if (mating_value < beta) {
            beta = mating_value;
            if (alpha >= beta) return mating_value; // can't beat a known faster mate
        }
        const int mated_value = -MATE + info.ply;   // worst case: we are mated now
        if (mated_value > alpha) {
            alpha = mated_value;
            if (alpha >= beta) return mated_value;
        }
    }
#endif

    // VICE Part 84: Transposition Table Probe
    // Check if we've already searched this position to sufficient depth
    int tt_score;
    uint8_t tt_depth, tt_node_type;
    uint32_t tt_best_move;
    bool tt_hit = tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_node_type, tt_best_move);
    
    if (tt_hit && tt_depth >= depth && !isRoot) {
#if ENABLE_PLY_TRACKED_TT_MATE
        // Adjust mate scores to current ply (VICE Part 84: 5:13)
        if (tt_score > MATE - 1000) {
            tt_score -= info.ply;
        } else if (tt_score < -MATE + 1000) {
            tt_score += info.ply;
        }
#endif

        // Use transposition table score if it provides exact bounds (6:01)
        if (tt_node_type == TTEntry::EXACT) {
            return tt_score;  // Exact score
        } else if (tt_node_type == TTEntry::LOWER_BOUND && tt_score >= beta) {
            return beta;  // Beta cutoff
        } else if (tt_node_type == TTEntry::UPPER_BOUND && tt_score <= alpha) {
            return alpha;  // Alpha cutoff  
        }
    }
    
    // Syzygy Tablebase Probe (BACKLOG #10 closure).
    // Only probe at leaf nodes (depth <= 1). Do NOT store the result in TT:
    // halfmove_clock is not part of the zobrist key, so the same key can be
    // reached with different rule50 values, but TB scores are only valid
    // for the rule50=0 case the safe wrapper requires. Caching a TB score
    // pollutes the TT for any future visit to the same piece-placement at
    // a different rule50 — measurable Elo loss in tournament play.
    if (depth <= 1 && tablebase && tablebase->is_available()) {
        int wdl_score;
        if (probe_tablebase_wdl(pos, wdl_score)) {
            info.tbhits++;  // standard UCI `tbhits`
            return wdl_score;
        }
    }
    
    // Check for early exit conditions
    if (depth == 0) {
        return quiescence(pos, alpha, beta, info, 0);  // Enter quiescence search at leaf nodes
    }
    
    // VICE Part 76: In check extension (3:01)
    // If the side to move is in check, extend the search depth by 1
    // This helps prevent the engine from getting checkmated by forcing sequences
    bool in_check = false;
    int king_sq = pos.king_sq[int(pos.side_to_move)];
    if (king_sq >= 0) {
        in_check = SqAttackedBB(king_sq, pos, !pos.side_to_move);
        if (in_check) {
            depth++; // Extend search depth when in check
        }
    }
    
    // Periodically check time and node limits
    if ((info.nodes & 2047) == 0) {  // Check every 2048 nodes
        checkup(info);
    }
    
    if (info.stopped || info.quit) {
        return 0;
    }

    // Reverse futility / static null-move pruning.
    // At low depth, if our static eval is so far above beta that even giving
    // up `margin` centipawns per ply would still beat beta, we can return
    // immediately. Cheaper than null-move pruning (no make/unmake/recurse).
    const int REVERSE_FUTILITY_MAX_DEPTH = 6;
    const int REVERSE_FUTILITY_MARGIN = 80;  // cp per ply
    if (!in_check && !isRoot && depth > 0 && depth <= REVERSE_FUTILITY_MAX_DEPTH
            && beta < MATE - 1000 && beta > -(MATE - 1000)) {
        int eval = get_static_eval();
        int margin = REVERSE_FUTILITY_MARGIN * depth;
        if (eval - margin >= beta) {
            return eval - margin;
        }
    }

    // VICE Part 83: Null Move Pruning
    // Only try null move if:
    // 1. We're allowed to do null move (doNull = true)
    // 2. Not in check (zugzwang safety)
    // 3. Not at root level
    // 4. Depth is sufficient (at least 4 for R=3 reduction)
    // 5. Side to move has non-pawn material (big pieces)
    const int NULL_MOVE_REDUCTION = 4;  // R = 4, more aggressive pruning
    const int MIN_NULL_MOVE_DEPTH = 5;  // Minimum depth to try null move (increased for R=4)
#if ENABLE_NMP_VERIFICATION
    // #43: gate verification on (a) high depth — keep the re-search on the thin
    // upper slice of the tree, and (b) low non-pawn material — zugzwang only
    // happens with few pieces, so the gate skips the middlegame entirely (where
    // an ungated version cost +53% nodes). game_phase_256: 256 = opening,
    // 0 = bare kings; <= 96 ≈ entering the endgame. Both tunable.
    const int NMP_VERIFICATION_MIN_DEPTH = 10;
    const int NMP_VERIFICATION_MAX_PHASE = 96;
#endif
    
    if (doNull && !in_check && !isRoot && depth >= MIN_NULL_MOVE_DEPTH && 
        pos.has_non_pawn_material(pos.side_to_move)) {
        
        // DEBUG: Uncomment to see null move attempts
        // std::cout << "Trying null move at depth " << depth << std::endl;
        
        // Make null move (give opponent a free move)
        pos.MakeNullMove();
        assert_search_position_integrity(pos, "after MakeNullMove");

        // Record null marker so the child's counter-move guard
        // (previous_move.move != 0) skips lookup over a null parent move.
        if (info.ply >= 0 && info.ply < 64) {
            info.search_stack[info.ply] = S_MOVE();
        }

        // Search with reduced depth and narrow window around beta
        const auto null_child = capture_search_position(pos);
        ++info.ply;
        int null_score = -AlphaBeta(pos, -beta, -beta + 1, depth - 1 - NULL_MOVE_REDUCTION, info, false, false);
        --info.ply;
        assert_search_position_unchanged(pos, null_child, "after null-move search");

        // Undo null move
        pos.TakeNullMove();
        assert_search_position_integrity(pos, "after TakeNullMove");
        
        // Check if we should stop
        if (info.stopped || info.quit) {
            return 0;
        }
        
        // If null move search shows position is already too good (>= beta),
        // then our actual moves should easily beat beta - prune this node
        if (null_score >= beta) {
            bool verified = true;
#if ENABLE_NMP_VERIFICATION
            // #43: verification search. The null move has been undone above, so
            // `pos` is this node's actual position. Re-search it at reduced
            // depth with null pruning disabled — same node, so NO ply bump.
            // Confirm the cutoff only if this also fails high; otherwise the
            // null fail-high was a zugzwang illusion and we fall through to the
            // normal move search instead of pruning.
            if (depth >= NMP_VERIFICATION_MIN_DEPTH &&
                game_phase_256(pos) <= NMP_VERIFICATION_MAX_PHASE) {
                const auto verify_position = capture_search_position(pos);
                int verify_score = AlphaBeta(pos, beta - 1, beta,
                                             depth - NULL_MOVE_REDUCTION,
                                             info, false, false);
                assert_search_position_unchanged(pos, verify_position, "after NMP verification search");
                if (info.stopped || info.quit) {
                    return 0;
                }
                verified = (verify_score >= beta);
            }
#endif
            if (verified) {
                // Null move cutoff - this position is too good for the opponent
#if ENABLE_INFO_DIAGNOSTICS
                info.null_cut++;
#endif
                return beta;
            }
            // else (#43): zugzwang false-positive — do not prune; fall through
            // to the normal move search below.
        }
    }

    // Futility Pruning (Forward Pruning at Pre-Frontier Nodes)
    // Skip move search if position evaluation + safety margin is still <= alpha
    // This is safe because even the best possible move won't improve alpha enough
    const int FUTILITY_MARGIN_BASE = 100;      // Base margin in centipawns
    const int FUTILITY_MARGIN_PER_PLY = 50;    // Additional margin per remaining ply
    const int MAX_FUTILITY_DEPTH = 3;          // Maximum depth to apply futility pruning
    
    bool futility_prune = false;
    int futility_margin = 0;
    
    if (depth <= MAX_FUTILITY_DEPTH && !in_check && !isRoot) {
        // Reuse the node's cached static eval (computed once, see top of node)
        int eval = get_static_eval();

        // Calculate safety margin based on remaining depth
        futility_margin = FUTILITY_MARGIN_BASE + (FUTILITY_MARGIN_PER_PLY * depth);
        
        // If even with the safety margin we can't reach alpha, prune this node
        if (eval + futility_margin <= alpha) {
            futility_prune = true;

#if ENABLE_INFO_DIAGNOSTICS
            info.futility_cuts++;
#endif
            
            // Return alpha (or slightly better) since no move can improve it significantly
            return alpha;
        }
    }

    // Razoring: at low depth, if static eval + a generous margin is still
    // below alpha, the position is hopeless — reduce search depth by 1
    // rather than full search. Soft pruning (depth reduction, not return).
    {
        const int RAZORING_MARGIN = 400;        // 4 pawns of slack
        const int MAX_RAZORING_DEPTH = 4;
        if (depth >= 2 && depth <= MAX_RAZORING_DEPTH && !in_check && !isRoot) {
            int eval = get_static_eval();
            if (eval + RAZORING_MARGIN < alpha) {
                depth--;
#if ENABLE_INFO_DIAGNOSTICS
                info.razoring_cuts++;
#endif
            }
        }
    }

    // Generate pseudo-legal moves; legality is checked per-move via MakeMove
    // below, and mate/stalemate is detected after the loop via legal_count.
    S_MOVELIST move_list;
    generate_all_moves(pos, move_list);

    // Internal Iterative Deepening for PV nodes without hash move
    S_MOVE iid_move;
    iid_move.move = 0;
    
    // Check if we should perform IID (PV node without hash move)
    if (!isRoot && !tt_hit && depth >= 4) {
        // Likely PV node with full alpha-beta window and no hash move
        bool likely_pv_node = (beta - alpha > 1);
        if (likely_pv_node) {
            iid_move = internal_iterative_deepening(pos, alpha, beta, depth, info);
        }
    }
    
    int best_score = -30000;
    S_MOVE best_move;  // Track best move for transposition table storage
    best_move.move = 0;
    int legal_count = 0;  // For mate/stalemate detection after the loop

    // Capture alpha as it was on entry — the move loop below mutates `alpha`
    // when a move improves it, but TT bound classification must compare
    // best_score against the ORIGINAL window. Using the mutated alpha caused
    // every alpha-improving and beta-cutoff node to be stored as UPPER_BOUND,
    // crippling TT pruning. See docs/PERFORMANCE_ARCHITECTURE_REVIEW.md (Priority 1).
    const int original_alpha = alpha;

    // Try each move
    for (int i = 0; i < move_list.count; ++i) {
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, info, depth, iid_move);

        if (pos.MakeMove(move_list.moves[i]) != 1) {
            assert_search_position_integrity(pos, "after illegal AlphaBeta MakeMove rollback");
            continue; // Skip illegal moves
        }
        assert_search_position_integrity(pos, "after AlphaBeta MakeMove");
        ++legal_count;

        // Track move in search stack for counter-move heuristic
        if (info.ply >= 0 && info.ply < 64) {
            info.search_stack[info.ply] = move_list.moves[i];
        }
        
        int score;

        // Late Move Reduction (LMR) implementation
        // Reduce depth for moves that are unlikely to be best
        const int LMR_MIN_DEPTH = 3;           // Minimum depth to apply LMR
        const int LMR_FULL_DEPTH_MOVES = 4;   // First N moves searched at full depth

        bool needs_full_search = true;

        // BACKLOG #1 (P1a re-attempt): also exempt moves that give check.
        // Check-giving moves drive forcing sequences and shouldn't be
        // depth-reduced. After MakeMove, pos.side_to_move is the opponent,
        // so opp_king = pos.king_sq[side_to_move] and the attacker side
        // is !side_to_move. Computed lazily via lambda — only fires when
        // the other LMR conditions are already met, so per-move cost is
        // paid only on the small subset that would otherwise be reduced.
        auto gives_check = [&]() {
            int opp_king = pos.king_sq[int(pos.side_to_move)];
            return opp_king >= 0 && SqAttackedBB(opp_king, pos, !pos.side_to_move);
        };

        if (depth >= LMR_MIN_DEPTH && i >= LMR_FULL_DEPTH_MOVES &&
            !in_check && !move_list.moves[i].is_capture() &&
            !move_list.moves[i].is_promotion() &&
            !gives_check()) {

            // Tuned reduction: log(d)*log(m)/2 lookup, clamped to leave at
            // least one ply of search after reduction. Replaces the prior
            // R=1 / R=2 step function — provides finer granularity at high
            // depth and high move number.
            int d_idx = std::min(depth, 63);
            int m_idx = std::min(i, 63);
            int reduction = LMR_TABLE[d_idx][m_idx];
            reduction = std::max(reduction, 1);
            reduction = std::min(reduction, depth - 2);

            // Try reduced search first
            int reduced_depth = depth - 1 - reduction;
            if (reduced_depth >= 1) {
#if ENABLE_INFO_DIAGNOSTICS
                info.lmr_attempts++;
#endif
                ++info.ply;
                const auto child = capture_search_position(pos);
                score = -AlphaBeta(pos, -alpha - 1, -alpha, reduced_depth, info, true, false);
                --info.ply;
                assert_search_position_unchanged(pos, child, "after reduced LMR search");

                // If reduced search fails high, we need full search
                if (score > alpha) {
#if ENABLE_INFO_DIAGNOSTICS
                    info.lmr_failures++;
#endif
                    needs_full_search = true;
                } else {
                    needs_full_search = false;
                }
            }
        }

        // Full depth search (either initial search or re-search after LMR fail-high)
        if (needs_full_search) {
            if (i == 0 || alpha == best_score) {
                // First move or no improvement yet - use full window
                ++info.ply;
                const auto child = capture_search_position(pos);
                score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
                --info.ply;
                assert_search_position_unchanged(pos, child, "after full-window child search");
            } else {
                // PVS: Try null window first, then re-search if it fails high
                ++info.ply;
                const auto child = capture_search_position(pos);
                score = -AlphaBeta(pos, -alpha - 1, -alpha, depth - 1, info, true, false);
                --info.ply;
                assert_search_position_unchanged(pos, child, "after PVS null-window child search");
                if (score > alpha && score < beta) {
                    // Null window search failed high, re-search with full window
                    ++info.ply;
                    const auto research_child = capture_search_position(pos);
                    score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
                    --info.ply;
                    assert_search_position_unchanged(pos, research_child, "after PVS re-search child search");
                }
            }
        }
        
        pos.TakeMove();
        assert_search_position_integrity(pos, "after AlphaBeta TakeMove");
        
        if (info.stopped || info.quit) {
            return 0;
        }
        
        if (score > best_score) {
            best_score = score;
            best_move = move_list.moves[i];  // Track best move for TT storage
            if (score > alpha) {
                alpha = score;

                // Triangular PV: prepend this (new best) move to the best
                // child's line, building the exact PV bottom-up as the search
                // unwinds. info.ply+1 is where the child stored its line.
                if (info.ply + 1 < 64) {
                    info.pv_line[info.ply][0] = move_list.moves[i];
                    int child_len = info.pv_length[info.ply + 1];
                    for (int j = 0; j < child_len; ++j) {
                        info.pv_line[info.ply][j + 1] = info.pv_line[info.ply + 1][j];
                    }
                    info.pv_length[info.ply] = child_len + 1;
                }

                // Store best move in PV table (VICE tutorial style)
                store_pv_move(pos.zobrist_key, move_list.moves[i]);
                
                // VICE Part 64: Update history heuristic for non-capture moves that improve alpha
                if (!move_list.moves[i].is_capture()) {
                    update_search_history(pos, move_list.moves[i], depth);
#if ENABLE_CONTINUATION_HISTORY
                    // BACKLOG #3: mirror butterfly history's +depth^2 bonus on
                    // the parent-conditioned conthist table. pos is at the
                    // current node here (TakeMove already ran), so prev's piece
                    // sits on prev.get_to() as the helper expects.
                    if (info.ply > 0 && info.ply < 64) {
                        S_MOVE prev = info.search_stack[info.ply - 1];
                        update_continuation_history(pos, prev, move_list.moves[i], depth * depth);
                    }
#endif
                }

                if (alpha >= beta) {
                    // VICE Part 60: Track fail high statistics (0:13)
                    info.fh++; // Increment fail high count
                    if (i == 0) {
                        info.fhf++; // Fail high first (first move caused beta cutoff)
                    }

                    // Beta cutoff - update killer moves and history
                    update_killer_moves(move_list.moves[i], depth);
                    
#if ENABLE_PLY_TRACKED_COUNTERMOVE
                    // Update counter-move table if we have a previous move
                    if (info.ply > 0 && info.ply < 64) {
                        S_MOVE previous_move = info.search_stack[info.ply - 1];
                        if (previous_move.move != 0) {
                            update_counter_move(previous_move, move_list.moves[i]);
                        }
                    }
#endif
                    
                    break;
                }
            }
        } else {
            // Move didn't improve alpha - apply negative history scoring for quiet moves
            if (!move_list.moves[i].is_capture() && depth > 0) {
                penalize_search_history(pos, move_list.moves[i], depth);
#if ENABLE_CONTINUATION_HISTORY
                // BACKLOG #3: mirror butterfly history's -depth^2 penalty on conthist.
                if (info.ply > 0 && info.ply < 64) {
                    S_MOVE prev = info.search_stack[info.ply - 1];
                    update_continuation_history(pos, prev, move_list.moves[i], -depth * depth);
                }
#endif
            }
        }
    }

    // No legal moves found: checkmate or stalemate. Detect after the loop
    // since pseudo-legal generation can yield moves that all turn out to be
    // illegal (king-into-check, pinned piece, EP self-check).
    if (legal_count == 0 && !info.stopped && !info.quit) {
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttackedBB(king_sq, pos, !pos.side_to_move)) {
#if ENABLE_PLY_TRACKED_TT_MATE
            // Use info.ply so the leaf encoding matches the TT store/probe
            // adjustment that uses info.ply. The two diverge under check
            // extensions (which bump depth) — using info.max_depth-depth
            // would corrupt the TT for check-extended mate paths.
            return -MATE + info.ply;
#else
            return -MATE + (info.max_depth - depth); // Checkmate (loss with distance)
#endif
        }
        return -CONTEMPT; // Stalemate — contempt-biased (BACKLOG #16)
    }

    // VICE Part 84: Store result in transposition table (6:38)
    // NOTE: must compare against `original_alpha` (captured pre-loop), not the
    // mutated `alpha`. See comment at the original_alpha declaration above.
    uint8_t node_type;
    if (best_score <= original_alpha) {
        node_type = TTEntry::UPPER_BOUND;  // All moves failed low (upper bound)
    } else if (best_score >= beta) {
        node_type = TTEntry::LOWER_BOUND;  // Beta cutoff (lower bound)
    } else {
        node_type = TTEntry::EXACT;        // Exact score within alpha-beta window
    }
    
    // Adjust mate scores for storage (VICE Part 84: 5:13)
    int store_score = best_score;
#if ENABLE_PLY_TRACKED_TT_MATE
    if (store_score > MATE - 1000) {
        store_score += info.ply;
        // Clamp to keep mate scores inside (-MATE, MATE) — see BACKLOG #13
        // step 4. Prevents `>= MATE` corruption if check extensions cause
        // encoding drift somewhere we didn't audit.
        if (store_score >= MATE) store_score = MATE - 1;
    } else if (store_score < -MATE + 1000) {
        store_score -= info.ply;
        if (store_score <= -MATE) store_score = -(MATE - 1);
    }
#endif

    tt_table.store(pos.zobrist_key, store_score, depth, node_type, best_move.move);

    return best_score;
}

// Internal Iterative Deepening for PV nodes without hash move
// Performs a shallow search to find a good move for ordering when no hash move is available
S_MOVE Engine::internal_iterative_deepening(Position& pos, int alpha, int beta, int depth, SearchInfo& info) {
    S_MOVE iid_move;
    iid_move.move = 0;  // Initialize to null move
    
    // Only perform IID if conditions are met:
    // 1. Sufficient depth to justify the overhead
    // 2. Likely PV node (full alpha-beta window)
    const int MIN_IID_DEPTH = 4;      // Minimum depth to perform IID
    const int IID_REDUCTION = 2;      // Reduce depth by 2 for IID search
    
    if (depth < MIN_IID_DEPTH) {
        return iid_move;  // Not worth the overhead at shallow depths
    }
    
    // Check if this looks like a PV node (not a null window search)
    bool likely_pv_node = (beta - alpha > 1);
    if (!likely_pv_node) {
        return iid_move;  // Only do IID for PV nodes
    }
    
    // Perform shallow search to find best move
    int iid_depth = depth - IID_REDUCTION;
    if (iid_depth >= 1) {
        // Generate pseudo-legal moves; the inner MakeMove guard handles legality.
        // No explicit mate detection here — IID is just an ordering hint, and
        // returning iid_move == 0 (its initial value) is fine if all moves are
        // illegal, since the parent AlphaBeta proceeds with its own search.
        S_MOVELIST iid_move_list;
        generate_all_moves(pos, iid_move_list);

        if (iid_move_list.count == 0) {
            return iid_move;  // No pseudo-legal moves at all
        }
        
        // Use simple move ordering for IID (no TT move dependency)
        order_moves(iid_move_list, pos);
        
        int best_score = -30000;
        
        // Try moves in IID search
        for (int i = 0; i < iid_move_list.count; ++i) {
            if (pos.MakeMove(iid_move_list.moves[i]) != 1) {
                assert_search_position_integrity(pos, "after illegal IID MakeMove rollback");
                continue;
            }
            assert_search_position_integrity(pos, "after IID MakeMove");

            // Track move in search stack for counter-move heuristic
            if (info.ply >= 0 && info.ply < 64) {
                info.search_stack[info.ply] = iid_move_list.moves[i];
            }

            ++info.ply;
            const auto child = capture_search_position(pos);
            int score = -AlphaBeta(pos, -beta, -alpha, iid_depth, info, true, false);
            --info.ply;
            assert_search_position_unchanged(pos, child, "after IID child search");

            pos.TakeMove();
            assert_search_position_integrity(pos, "after IID TakeMove");
            
            // Check for early termination
            if (info.stopped || info.quit) {
                break;
            }
            
            if (score > best_score) {
                best_score = score;
                iid_move = iid_move_list.moves[i];
                
                // Alpha-beta pruning in IID
                if (score >= beta) {
                    break;
                }
                if (score > alpha) {
                    alpha = score;
                }
            }
        }
    }
    
    return iid_move;
}

// Quiescence search to handle horizon effect (4:40)
int Engine::quiescence(Position& pos, int alpha, int beta, SearchInfo& info, int q_depth) {
    // Quiescence depth limit to prevent stack overflow and improve performance
    const int MAX_QUIESCENCE_DEPTH = 10;
    
    // If we've reached maximum quiescence depth, return stand pat evaluation
    if (q_depth >= MAX_QUIESCENCE_DEPTH) {
        return evalPosition(pos);
    }
    
    // Increment node count for every position visited
    info.nodes++;
    
    // Periodically check time
    if ((info.nodes & 2047) == 0) {
        checkup(info);
    }
    
    if (info.stopped || info.quit) {
        return 0;
    }
    
    // Stand pat - evaluate current position
    int stand_pat = evalPosition(pos);
    
    // Beta cutoff on stand pat
    if (stand_pat >= beta) {
        return beta;
    }
    
    // Alpha improvement
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    
    // VICE Part 65: Generate only capture moves for quiescence search.
    // Pseudo-legal: the per-move `MakeMove() != 1` guard below filters illegals.
    // Saves the per-capture Make/Unmake legality filter that the legal version did.
    S_MOVELIST move_list;
    generate_all_caps_pseudo(pos, move_list);
    
    // Search all capture moves
    for (int i = 0; i < move_list.count; ++i) {
        // VICE Part 62: Pick best move from remaining moves
        pick_next_move(move_list, i, pos, info, -1);  // No depth in quiescence

        S_MOVE move = move_list.moves[i];

        // Delta pruning: if even winning the captured piece can't lift the
        // stand-pat eval to within DELTA_MARGIN of alpha, this capture is
        // hopeless — skip it before paying for SEE. Uses the eval material
        // scale (PIECE_VALUES_MG) since the comparison is against stand_pat,
        // an eval. Promotions are exempt (the ~800cp promotion gain isn't in
        // the victim value); the victim is read from the board (robust for
        // en passant, where the destination square is empty). Mirrors the
        // existing stand_pat assumption above (no separate in-check guard).
        const int DELTA_MARGIN = 200;
        if (!move.is_promotion()) {
            PieceType victim = move.is_en_passant()
                ? PieceType::Pawn
                : type_of(pos.at_sq64(move.get_to()));
            if (stand_pat + PIECE_VALUES_MG[size_t(victim)] + DELTA_MARGIN <= alpha) {
                continue;
            }
        }

        // SEE pruning: skip captures that lose material on the recapture
        // sequence. Promotions are searched anyway because the value gain
        // from promotion can flip a "bad" capture into a sound one. King
        // captures are never SEE-pruned (king is the most valuable, so
        // SEE wouldn't classify them as losing anyway, but be explicit).
        if (!move.is_promotion() && Huginn::see(pos, move) < 0) {
            continue;
        }

        if (pos.MakeMove(move) != 1) {
            assert_search_position_integrity(pos, "after illegal quiescence MakeMove rollback");
            continue; // Skip illegal moves
        }
        assert_search_position_integrity(pos, "after quiescence MakeMove");

        const auto child = capture_search_position(pos);
        int score = -quiescence(pos, -beta, -alpha, info, q_depth + 1);
        assert_search_position_unchanged(pos, child, "after quiescence child search");
        pos.TakeMove();
        assert_search_position_integrity(pos, "after quiescence TakeMove");
        
        if (info.stopped || info.quit) {
            return 0;
        }
        
        if (score >= beta) {
            return beta; // Beta cutoff
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

// VICE-style iterative deepening search function (Part 58)
// Implements the two main benefits of iterative deepening:
// 1. Time Management: Return best move if time runs out (0:49)
// 2. Move Ordering Efficiency: Use PV and heuristics from shallower searches (1:49)
S_MOVE Engine::searchPosition(Position& pos, SearchInfo& info) {
    S_MOVE best_move;
    best_move.move = 0;
    
    // VICE Part 85: Check opening book first
    if (opening_book.is_book_loaded() && opening_book.has_book_moves(pos)) {
        S_MOVE book_move = opening_book.get_book_move(pos);
        if (book_move.move != 0) {
            // CRITICAL: Validate that the book move is actually legal in current position
            S_MOVELIST legal_moves;
            generate_legal_moves(pos, legal_moves);
            
            // Check if book move is in the legal move list
            bool book_move_is_legal = false;
            for (int i = 0; i < legal_moves.count; ++i) {
                if (legal_moves.moves[i].move == book_move.move) {
                    book_move_is_legal = true;
                    break;
                }
            }
            
            if (book_move_is_legal) {
                std::cout << "info string Found book move: " << move_to_uci(book_move) << std::endl;
                return book_move;
            } else {
                std::cout << "info string Book move " << move_to_uci(book_move) << " is illegal, ignoring" << std::endl;
            }
        }
    }
    
    // Syzygy Tablebase Root Probe - Check for perfect endgame move
    if (tablebase && tablebase->is_available()) {
        S_MOVE tablebase_move = probe_tablebase_root(pos);
        if (tablebase_move.move != 0) {
            std::cout << "info string Found tablebase move: " << move_to_uci(tablebase_move) << std::endl;
            return tablebase_move;
        }
    }
    
    // VICE Part 57: Clear everything before starting search
    clearForSearch(*this, info);
    
    // Set up search parameters
    info.start_time = std::chrono::steady_clock::now();
    const int root_static_eval = evalPosition(pos);
    
    // Iterative deepening loop (0:22) - search depth 1, then 2, then 3, etc.
    for (int current_depth = 1; current_depth <= info.max_depth; ++current_depth) {
        // Check if we should stop before starting new depth (time management)
        if (info.stopped || info.quit) {
            break;
        }

        // Reset per-iteration UCI counters so each `info` line reports
        // this iteration's seldepth/tbhits, not the cumulative since the
        // search started. nodes intentionally NOT reset — UCI `nodes`
        // is the cumulative search node count.
        info.seldepth = current_depth;  // base: this iteration searches to current_depth at minimum
        info.tbhits = 0;

        // Iteration-start time gate: if the next iteration likely won't fit in
        // the remaining budget, return the previous depth's best move instead
        // of wasting time on a partial iteration that we'll discard anyway.
        // Heuristic: assume next iteration is ~3x the elapsed time so far;
        // bail if elapsed > budget/4 (i.e. 4*elapsed > budget).
        if (current_depth > 1 && !info.infinite && !info.depth_only
                && info.stop_time != std::chrono::steady_clock::time_point{}) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time).count();
            auto budget_ms = std::chrono::duration_cast<std::chrono::milliseconds>(info.stop_time - info.start_time).count();
            if (budget_ms > 0 && elapsed_ms * 4 > budget_ms) {
                break;
            }
        }

        info.depth = current_depth;
        
        // Apply periodic aging to prevent history scores from becoming too large
        // Age every 3 depths to maintain move discrimination
        if (current_depth > 1 && (current_depth % 3) == 0) {
            age_search_history();
        }
        
        // Store best move from previous iteration for move ordering
        S_MOVE prev_best = best_move;
        
        // Root search: try all moves at root to find the best one. Pseudo-legal
        // generation; legality is checked per-move via MakeMove inside the loop.
        S_MOVELIST move_list;
        generate_all_moves(pos, move_list);

        if (move_list.count == 0) break; // No pseudo-legal moves at all

        int best_score = -30000;
        S_MOVE depth_best_move;
        depth_best_move.move = 0;
        int legal_count = 0;
        info.pv_length[0] = 0;  // reset root PV for this iteration

        // Order moves to try previous iteration's best move first for better alpha-beta cutoffs
        if (prev_best.move != 0) {
            for (int i = 0; i < move_list.count; ++i) {
                if (move_list.moves[i].move == prev_best.move) {
                    // Swap the previous best move to position 0
                    S_MOVE temp = move_list.moves[0];
                    move_list.moves[0] = move_list.moves[i];
                    move_list.moves[i] = temp;
                    break;
                }
            }
        }

        // Try each move at the root with PVS-style alpha tightening:
        // pass local_alpha (the best score found so far at root) as the
        // recursion's alpha, so subsequent subtrees can produce alpha-beta
        // cutoffs against it instead of searching with the full window.
        // The fail-high break is dormant here (beta = 30000 only fires on
        // found-mate) but is in place ready for aspiration windows.
        const int root_alpha_init = -30000;
        const int root_beta = 30000;
        int local_alpha = root_alpha_init;

        for (int i = 0; i < move_list.count; ++i) {
            if (info.stopped || info.quit) break;

            if (pos.MakeMove(move_list.moves[i]) != 1) {
                assert_search_position_integrity(pos, "after illegal root MakeMove rollback");
                continue; // Skip illegal moves
            }
            assert_search_position_integrity(pos, "after root MakeMove");
            ++legal_count;
            const bool immediate_repetition = isRepetition(pos);

            // Track move in search stack for counter-move heuristic.
            // info.ply is 0 at root; this writes search_stack[0].
            if (info.ply >= 0 && info.ply < 64) {
                info.search_stack[info.ply] = move_list.moves[i];
            }

            ++info.ply;
            const auto child = capture_search_position(pos);
            int score = -AlphaBeta(pos, -root_beta, -local_alpha, current_depth - 1, info, true, false);
            --info.ply;
            assert_search_position_unchanged(pos, child, "after root child search");

            if (immediate_repetition && root_static_eval >= WINNING_REPETITION_AVOID_THRESHOLD) {
                score = std::min(score, WINNING_REPETITION_DRAW_SCORE);
            }

            pos.TakeMove();
            assert_search_position_integrity(pos, "after root TakeMove");

            if (info.stopped || info.quit) break;

            if (score > best_score) {
                best_score = score;
                depth_best_move = move_list.moves[i];

                // Triangular PV at the root (ply 0): this move + the child's
                // line, which the recursion stored at ply 1.
                info.pv_line[0][0] = move_list.moves[i];
                int child_len = info.pv_length[1];
                for (int j = 0; j < child_len; ++j) {
                    info.pv_line[0][j + 1] = info.pv_line[1][j];
                }
                info.pv_length[0] = child_len + 1;

                if (best_score > local_alpha) local_alpha = best_score;
                if (best_score >= root_beta) break; // fail-high (no aspiration yet → only mate)
            }
        }

        // If search was interrupted, return previous best move (time management benefit)
        if (info.stopped || info.quit) {
            break;
        }

        // No legal moves at root: mate or stalemate. Stop iterative deepening
        // since deeper iterations would just repeat the same empty result.
        if (legal_count == 0) break;
        
        // Update best move for this iteration
        if (depth_best_move.move != 0) {
            best_move = depth_best_move;
            // Store in PV table for next iteration's move ordering
            store_pv_move(pos.zobrist_key, depth_best_move);
        }
        
        // Calculate elapsed time for output
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time);
        
        // Build the PV to display: take the triangular PV as the prefix,
        // then extend by walking the transposition table. When a root move
        // bottoms out via a TT cutoff, the child never populates its own
        // pv_line and the triangular array truncates to length 1 — but the
        // deeper line still lives in the TT, so pull it out for the GUI.
        // Repetition / rule-50 truncation runs across both phases: the
        // *displayed* PV stops at the first position that revisits a prior game
        // position or reaches the fifty-move draw horizon (GUI validators
        // reject tails that continue after either terminal condition).
        // One make/unmake walk paid once per depth, not per node.
        constexpr int PV_DISPLAY_CAP = 32;  // array bound (deeper than Huginn ever reaches)
        // BACKLOG #36: never display a PV longer than the depth actually
        // searched. The triangular prefix is search-truth (length <= depth);
        // the TT-walk extension below recovers a full-length line when the
        // triangular array truncates via a TT cutoff, but must not run past the
        // searched depth — that tail is unverified, and a collision there used
        // to print bogus moves and an over-long PV.
        const int pv_cap = std::min(PV_DISPLAY_CAP, current_depth);
        S_MOVE display_pv[PV_DISPLAY_CAP];
        int pv_moves = 0;
        {
            // Seen-set baseline: only the actual game-history positions
            // (move_history[0..pos.ply-1]). The full move_history vector
            // grows during search and retains stale zobrist keys from the
            // last move sequence each alpha-beta iteration happened to
            // try (TakeMove decrements pos.ply but doesn't clear the slot).
            // Iterating the full vector poisoned the seen-set with stale
            // keys from the search interior — when one of them collided
            // with a position our walk visited (e.g. position-after-e2e4
            // after the search's final tried sequence ran through e2e4),
            // the walk aborted at one move. pos.ply is the game ply at
            // root emission, so this gives the correct rep-detection
            // baseline without false positives.
            std::unordered_set<uint64_t> seen;
            seen.reserve(static_cast<size_t>(pos.ply) + PV_DISPLAY_CAP);
            for (int i = 0; i < pos.ply; ++i) {
                const uint64_t k = pos.move_history[i].zobrist_key;
                if (k != 0) seen.insert(k);
            }
            seen.insert(pos.zobrist_key);

            int made = 0;
            bool stop = false;

            // Phase 1 — triangular PV prefix (search-truth, definitely legal).
            const S_MOVE* tri_pv = info.pv_line[0];
            const int tri_len = info.pv_length[0];
            for (int i = 0; i < tri_len && pv_moves < pv_cap; ++i) {
                if (pos.MakeMove(tri_pv[i]) != 1) {
                    assert_search_position_integrity(pos, "after illegal PV-prefix MakeMove rollback");
                    stop = true;
                    break;
                }
                assert_search_position_integrity(pos, "after PV-prefix MakeMove");
                ++made;
                display_pv[pv_moves++] = tri_pv[i];
                if (pos.halfmove_clock >= 100) {
                    stop = true;  // keep the draw-creating move, drop the tail
                    break;
                }
                if (!seen.insert(pos.zobrist_key).second) {
                    stop = true;  // keep the rep-creating move, drop the tail
                    break;
                }
            }

            // Phase 2 — extend via TT walk. Same collision filter the main
            // search uses (search.cpp:853-868): sanity-check the raw best_move
            // before calling MakeMove, since a TT collision can hand back a
            // move whose from-square is empty or wrong-coloured in the current
            // position. MakeMove != 1 then catches king-in-check cases.
            while (!stop && pv_moves < pv_cap) {
                int tt_score;
                uint8_t tt_depth, tt_type;
                uint32_t tt_move_raw;
                if (!tt_table.probe(pos.zobrist_key, tt_score, tt_depth, tt_type, tt_move_raw)) break;
                if (tt_move_raw == 0) break;

                S_MOVE tt_move;
                tt_move.move = static_cast<int>(tt_move_raw);
                tt_move.score = 0;

                const int from = tt_move.get_from();
                const int to = tt_move.get_to();
                if (from < 0 || from >= 64 || to < 0 || to >= 64) break;

                // BACKLOG #36: validate the raw TT move against this position's
                // generated moves before applying it. A hash collision can hand
                // back a move that belongs to a *different* position but happens
                // to have a correctly-coloured from-square — the old check
                // trusted that and let a bogus move into the displayed PV.
                // Requiring membership in the pseudo-legal list (then MakeMove
                // for king safety) makes every displayed move genuinely legal.
                S_MOVELIST walk_moves;
                generate_all_moves(pos, walk_moves);
                bool tt_move_legal = false;
                for (int wi = 0; wi < walk_moves.count; ++wi) {
                    if (walk_moves.moves[wi].move == tt_move.move) { tt_move_legal = true; break; }
                }
                if (!tt_move_legal) break;

                if (pos.MakeMove(tt_move) != 1) {
                    assert_search_position_integrity(pos, "after illegal PV TT-walk MakeMove rollback");
                    break;
                }
                assert_search_position_integrity(pos, "after PV TT-walk MakeMove");
                ++made;
                display_pv[pv_moves++] = tt_move;
                if (pos.halfmove_clock >= 100) break;  // fifty-move rule
                if (!seen.insert(pos.zobrist_key).second) break;  // rep
            }

            for (int i = 0; i < made; ++i) {
                pos.TakeMove();
                assert_search_position_integrity(pos, "after PV display TakeMove");
            }
        }
        const S_MOVE* pv_array = display_pv;

        // Spec-compliant UCI `info` line: tokens in canonical order, PV last
        // (PV is variable-length and consumes to end-of-line per the spec).
        // depth/seldepth/multipv/score/nodes/nps/hashfull/tbhits/time are
        // the standard fields every UCI GUI and adjudication tool expects.
        const uint64_t nps = info.nodes * 1000ULL / std::max<int64_t>(elapsed.count(), 1);
        std::cout << "info depth " << current_depth
                  << " seldepth " << info.seldepth
                  << " multipv 1"
                  << " score " << format_uci_score(best_score, pos.side_to_move)
                  << " nodes " << info.nodes
                  << " nps " << nps
                  << " hashfull " << tt_table.permill_full()
                  << " tbhits " << info.tbhits
                  << " time " << elapsed.count()
                  << " pv ";
        for (int i = 0; i < pv_moves; ++i) {
            std::cout << move_to_uci(pv_array[i]);
            if (i < pv_moves - 1) std::cout << " ";
        }
        std::cout << std::endl;

#if ENABLE_INFO_DIAGNOSTICS
        // Engine-internal pruning/ordering counters. Gated off by default so
        // (a) GUIs don't see non-standard tokens in the `info` line and
        // (b) the increments above (null_cut, lmr_*, tt hits/misses/writes)
        // compile out from the hot path. Build with
        // `-DENABLE_INFO_DIAGNOSTICS=1` when tuning.
        std::cout << "info string diag"
                  << " nullcut " << info.null_cut
                  << " lmr " << info.lmr_attempts << "/" << info.lmr_failures
                  << " tthits " << tt_table.get_hits()
                  << " ttwrites " << tt_table.get_writes()
                  << std::endl;
#endif
        
#if ENABLE_PRUNING_STATS
        // Print futility pruning statistics for this depth
        if (info.futility_cuts > 0) {
            std::cout << "info string Depth " << current_depth << " - Futility cuts: "
                      << info.futility_cuts << " (" << std::fixed << std::setprecision(1)
                      << (double(info.futility_cuts) / info.nodes * 100.0) << "%)" << std::endl;
        }

        // Print razoring statistics for this depth
        if (info.razoring_cuts > 0) {
            std::cout << "info string Depth " << current_depth << " - Razoring cuts: "
                      << info.razoring_cuts << " (" << std::fixed << std::setprecision(1)
                      << (double(info.razoring_cuts) / info.nodes * 100.0) << "%)" << std::endl;
        }
#endif

        // Iteration-start time gating happens at the top of the loop; no
        // post-iteration time check needed here.
    }
    
    return best_move;
}

// VICE Part 85: Opening book functions
bool Engine::load_opening_book(const std::string& book_path) {
    return opening_book.load_book(book_path);
}

S_MOVE Engine::get_book_move(const Position& pos) const {
    return opening_book.get_book_move(pos);
}

// Syzygy Tablebase functions
bool Engine::probe_tablebase_wdl(const Position& pos, int& wdl_score) const {
    if (!tablebase || !tablebase->is_available()) {
        return false;  // No tablebase available
    }

    if (!tablebase->can_probe(pos)) {
        return false;  // Position cannot be probed
    }

    int result = tablebase->probe_wdl(pos);
    if (result == INT32_MAX) {
        return false;  // Probe failed
    }

    wdl_score = result;
    return true;
}

S_MOVE Engine::probe_tablebase_root(const Position& pos) const {
    S_MOVE null_move;
    null_move.move = 0;
    
    if (!tablebase || !tablebase->is_available()) {
        return null_move;  // No tablebase available
    }
    
    if (!tablebase->can_probe(pos)) {
        return null_move;  // Position cannot be probed
    }
    
    return tablebase->probe_root(pos);
}

} // namespace Huginn
