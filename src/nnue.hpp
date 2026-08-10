/**
 * @file nnue.hpp
 * @brief NNUE evaluation: (768 -> H) x 2 perspectives, clipped ReLU, linear to cp.
 *
 * Backlog #39. The net replaces `Engine::evaluate()`'s hand-crafted score when a
 * net file is loaded and `UseNNUE` is on; with it off the HCE path is untouched
 * and fixed-depth signatures stay bit-identical to 2.4 (the gate the SMP work
 * established — see docs/BACKLOG.md #40).
 *
 * ## Architecture
 * - Input: 768 features = colour(2) x piece-type(6) x square(64), built twice,
 *   once from each side's perspective. The black perspective flips both the
 *   piece colour and the square (`sq ^ 56`), so a mirrored position produces
 *   the mirrored feature set — the same colour symmetry the HCE mirror test
 *   asserts (INVARIANTS.md).
 * - Hidden: one shared feature transformer, H accumulators per perspective.
 * - Output: `concat(stm_acc, nstm_acc)` -> clipped ReLU -> single linear -> cp.
 *
 * ## Incremental updates
 * The accumulator lives inside ::Position and is maintained by the three atomic
 * piece operations (`add_piece_sq64` / `clear_piece_sq64` / `move_piece_sq64`).
 * `TakeMove` performs the exact inverse piece operations, so the accumulator
 * unwinds itself — no separate undo stack is needed, and no accumulator state
 * is stored in ::S_UNDO.
 *
 * ## Quantisation (must match tools/train_nnue.py exactly)
 * Feature weights and bias are scaled by A=256, output weights by B=256, and
 * the trainer emits centipawns/16, so
 * `cp = (sum(clamp(acc,0,256) * w2) + b2) / 4096` where 4096 = A*B/16.
 *
 * @see tools/train_nnue.py for the trainer and the binary layout it writes.
 */
#pragma once

#include <cstdint>
#include <string>

#if defined(__AVX2__)
#include <immintrin.h>
#define HUGINN_NNUE_AVX2 1
#endif

#include "chess_types.hpp"

class Position;

namespace Huginn {
namespace NNUE {

/// Number of input features: 2 colours x 6 piece types x 64 squares.
constexpr int kInputs = 768;
/// Hidden neurons per perspective. Must match the loaded net file.
constexpr int kHidden = 256;
/// Accumulator activation clamp (the A quantisation scale).
constexpr int kActivationMax = 256;
/// Final divisor: A * B / OUT_SCALE = 256 * 256 / 16.
constexpr int kOutputDivisor = 4096;

/**
 * @brief Per-position hidden-layer sums, one row per perspective.
 * @note Indexed `[perspective][neuron]`; perspective 0 = White's view.
 */
struct alignas(64) Accumulator {
    int16_t v[2][kHidden];
};

// ---------------------------------------------------------------------------
// Net storage. Exposed so the accumulator deltas below can inline into the
// Position piece operations, which are the hottest code in make/unmake.
// ---------------------------------------------------------------------------

/// Feature-transformer weights, `[feature][neuron]` (feature-major, as trained).
alignas(64) extern int16_t g_w1[kInputs][kHidden];
/// Feature-transformer bias, added once per perspective on refresh.
alignas(64) extern int16_t g_b1[kHidden];
/// Output-layer weights: first H for the side to move, second H for the other.
alignas(64) extern int16_t g_w2[2 * kHidden];
/// Output-layer bias (already scaled by A*B).
extern int32_t g_b2;
/// True when a net is loaded AND `UseNNUE` is on. Guards every accumulator op.
extern bool g_enabled;
/// True once a net file has been read successfully (independent of `UseNNUE`).
extern bool g_loaded;

/// @return Whether NNUE evaluation is active for this search.
inline bool enabled() { return g_enabled; }
/// @return Whether a net file has been loaded.
inline bool loaded() { return g_loaded; }

/**
 * @brief Loads a quantised net from disk.
 * @param path File written by `tools/train_nnue.py export`.
 * @return true on success; on failure the previous net (if any) is retained.
 */
bool load(const std::string& path);

/// Enables/disables NNUE evaluation. A no-op that leaves it off if no net is loaded.
void set_enabled(bool on);

/// @return Diagnostic description of the loaded net (path + architecture), or "none".
const std::string& description();

/**
 * @brief Feature index for one piece as seen from one perspective.
 * @param persp 0 = White's view, 1 = Black's view.
 * @param colour 0 = White piece, 1 = Black piece.
 * @param pt0 Piece type, zero-based (0 = pawn .. 5 = king).
 * @param sq Square (sq64).
 * @note Must match `fen_features()` in tools/train_nnue.py bit for bit.
 */
inline int feature_index(int persp, int colour, int pt0, int sq) {
    const int rel_colour = colour ^ persp;
    const int rel_sq = persp == 0 ? sq : (sq ^ 56);
    return (rel_colour * 6 + pt0) * 64 + rel_sq;
}

// The three accumulator deltas below run inside make/unmake, which is the
// hottest code in the engine — a quiet move touches 2 * kHidden int16 lanes and
// nothing else. Written as explicit AVX2 (16 int16 lanes per register, so
// kHidden = 256 is exactly 16 registers per perspective) rather than trusting
// auto-vectorisation, which MSVC declines to apply through the int16 narrowing
// casts the scalar form needs. The scalar fallback below is the reference
// semantics and must stay bit-identical to the vector path.
#if HUGINN_NNUE_AVX2
constexpr int kVecs = kHidden / 16;

inline void acc_add_row(int16_t* dst, const int16_t* w) {
    __m256i* d = reinterpret_cast<__m256i*>(dst);
    const __m256i* s = reinterpret_cast<const __m256i*>(w);
    for (int i = 0; i < kVecs; ++i)
        _mm256_store_si256(d + i, _mm256_add_epi16(_mm256_load_si256(d + i),
                                                   _mm256_load_si256(s + i)));
}

inline void acc_sub_row(int16_t* dst, const int16_t* w) {
    __m256i* d = reinterpret_cast<__m256i*>(dst);
    const __m256i* s = reinterpret_cast<const __m256i*>(w);
    for (int i = 0; i < kVecs; ++i)
        _mm256_store_si256(d + i, _mm256_sub_epi16(_mm256_load_si256(d + i),
                                                   _mm256_load_si256(s + i)));
}

inline void acc_move_row(int16_t* dst, const int16_t* wf, const int16_t* wt) {
    __m256i* d = reinterpret_cast<__m256i*>(dst);
    const __m256i* f = reinterpret_cast<const __m256i*>(wf);
    const __m256i* t = reinterpret_cast<const __m256i*>(wt);
    for (int i = 0; i < kVecs; ++i) {
        __m256i x = _mm256_sub_epi16(_mm256_load_si256(d + i), _mm256_load_si256(f + i));
        _mm256_store_si256(d + i, _mm256_add_epi16(x, _mm256_load_si256(t + i)));
    }
}
#else
inline void acc_add_row(int16_t* dst, const int16_t* w) {
    for (int h = 0; h < kHidden; ++h) dst[h] = int16_t(dst[h] + w[h]);
}
inline void acc_sub_row(int16_t* dst, const int16_t* w) {
    for (int h = 0; h < kHidden; ++h) dst[h] = int16_t(dst[h] - w[h]);
}
inline void acc_move_row(int16_t* dst, const int16_t* wf, const int16_t* wt) {
    for (int h = 0; h < kHidden; ++h) dst[h] = int16_t(dst[h] - wf[h] + wt[h]);
}
#endif

/// Adds one piece's contribution to both perspectives.
inline void acc_add(Accumulator& acc, Color c, PieceType pt, int sq) {
    const int colour = int(c);
    const int pt0 = int(pt) - 1;  // PieceType is 1-based (Pawn = 1)
    acc_add_row(acc.v[0], g_w1[feature_index(0, colour, pt0, sq)]);
    acc_add_row(acc.v[1], g_w1[feature_index(1, colour, pt0, sq)]);
}

/// Removes one piece's contribution from both perspectives.
inline void acc_sub(Accumulator& acc, Color c, PieceType pt, int sq) {
    const int colour = int(c);
    const int pt0 = int(pt) - 1;
    acc_sub_row(acc.v[0], g_w1[feature_index(0, colour, pt0, sq)]);
    acc_sub_row(acc.v[1], g_w1[feature_index(1, colour, pt0, sq)]);
}

/// Moves one piece's contribution from `from` to `to` (one pass, both perspectives).
inline void acc_move(Accumulator& acc, Color c, PieceType pt, int from, int to) {
    const int colour = int(c);
    const int pt0 = int(pt) - 1;
    acc_move_row(acc.v[0], g_w1[feature_index(0, colour, pt0, from)],
                 g_w1[feature_index(0, colour, pt0, to)]);
    acc_move_row(acc.v[1], g_w1[feature_index(1, colour, pt0, from)],
                 g_w1[feature_index(1, colour, pt0, to)]);
}

/// Rebuilds the accumulator from the position's bitboards (after FEN / rebuild).
void refresh(Position& pos);

/**
 * @brief Evaluates the position from the accumulator.
 * @return Centipawns, positive = side to move is better (same convention as
 *         `Engine::evaluate()`).
 */
int evaluate(const Position& pos);

/// Recomputes the accumulator from scratch and evaluates — for tests/probes.
int evaluate_from_scratch(Position& pos);

}  // namespace NNUE
}  // namespace Huginn
