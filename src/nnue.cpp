/**
 * @file nnue.cpp
 * @brief NNUE net loading, accumulator refresh, and forward pass. Backlog #39.
 * @see nnue.hpp for the architecture and quantisation contract.
 */
#include "nnue.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

#include "position.hpp"

namespace Huginn {
namespace NNUE {

alignas(64) int16_t g_w1[kInputs][kHidden];
alignas(64) int16_t g_b1[kHidden];
alignas(64) int16_t g_w2[2 * kHidden];
int32_t g_b2 = 0;
bool g_enabled = false;
bool g_loaded = false;

namespace {
std::string g_description = "none";
}  // namespace

const std::string& description() { return g_description; }

bool load(const std::string& path) {
    // Layout (little-endian, written by tools/train_nnue.py):
    //   w1 : int16[768][H] feature-major
    //   b1 : int16[H]
    //   w2 : int16[2H]
    //   b2 : int32
    const size_t expect = sizeof(int16_t) * (size_t(kInputs) * kHidden + kHidden + 2 * kHidden) +
                          sizeof(int32_t);

    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size < 0 || size_t(size) != expect) {
        std::fprintf(stderr,
                     "info string NNUE: %s is %lld bytes, expected %zu (H=%d mismatch?)\n",
                     path.c_str(), static_cast<long long>(size), expect, kHidden);
        return false;
    }

    // Read into staging buffers first so a short/failed read cannot leave a
    // half-written net installed under a live search.
    std::vector<int16_t> w1(size_t(kInputs) * kHidden);
    std::vector<int16_t> b1(kHidden);
    std::vector<int16_t> w2(2 * kHidden);
    int32_t b2 = 0;

    in.read(reinterpret_cast<char*>(w1.data()), std::streamsize(w1.size() * sizeof(int16_t)));
    in.read(reinterpret_cast<char*>(b1.data()), std::streamsize(b1.size() * sizeof(int16_t)));
    in.read(reinterpret_cast<char*>(w2.data()), std::streamsize(w2.size() * sizeof(int16_t)));
    in.read(reinterpret_cast<char*>(&b2), sizeof(b2));
    if (!in) return false;

    std::memcpy(g_w1, w1.data(), w1.size() * sizeof(int16_t));
    std::memcpy(g_b1, b1.data(), b1.size() * sizeof(int16_t));
    std::memcpy(g_w2, w2.data(), w2.size() * sizeof(int16_t));
    g_b2 = b2;
    g_loaded = true;
    g_description = path + " (768->" + std::to_string(kHidden) + ")x2";
    return true;
}

void set_enabled(bool on) { g_enabled = on && g_loaded; }

void refresh(Position& pos) {
    if (!g_loaded) return;
    for (int p = 0; p < 2; ++p)
        for (int h = 0; h < kHidden; ++h) pos.acc.v[p][h] = g_b1[h];

    for (int c = 0; c < 2; ++c) {
        for (int pt = int(PieceType::Pawn); pt <= int(PieceType::King); ++pt) {
            Bitboard bb = pos.piece_bitboards[size_t(c)][size_t(pt)];
            while (bb) {
                const int sq = pop_lsb(bb);
                acc_add(pos.acc, Color(c), PieceType(pt), sq);
            }
        }
    }
}

int evaluate(const Position& pos) {
    const int stm = int(pos.side_to_move);
    const int nstm = stm ^ 1;

    // int32 accumulation, deliberately. The activation is clamped to [0, 256]
    // and the trainer clamps output weights to +-4.0 at B=256 (so |w2| <= 1024),
    // giving a worst case of 256 * 1024 * 512 = 1.34e8 — two orders of magnitude
    // inside int32. Accumulating in int64 instead costs roughly 2x here: it
    // forces 64-bit multiply-adds and blocks the AVX2 auto-vectorisation of a
    // loop that runs at every leaf.
    const int16_t* as = pos.acc.v[stm];
    const int16_t* an = pos.acc.v[nstm];

#if HUGINN_NNUE_AVX2
    // clamp to [0, 256] then multiply-accumulate. _mm256_madd_epi16 produces
    // int32 pairwise sums, which is exactly the accumulation width the scalar
    // path uses, so the two paths are bit-identical (verified by comparing
    // `eval` output against the reference implementation in
    // tools/train_nnue.py, which computes the same integer arithmetic).
    const __m256i lo = _mm256_setzero_si256();
    const __m256i hi = _mm256_set1_epi16(int16_t(kActivationMax));
    __m256i acc32 = _mm256_setzero_si256();
    const __m256i* a0 = reinterpret_cast<const __m256i*>(as);
    const __m256i* a1 = reinterpret_cast<const __m256i*>(an);
    const __m256i* w0 = reinterpret_cast<const __m256i*>(g_w2);
    const __m256i* w1v = reinterpret_cast<const __m256i*>(g_w2 + kHidden);
    for (int i = 0; i < kVecs; ++i) {
        __m256i c0 = _mm256_min_epi16(_mm256_max_epi16(_mm256_load_si256(a0 + i), lo), hi);
        __m256i c1 = _mm256_min_epi16(_mm256_max_epi16(_mm256_load_si256(a1 + i), lo), hi);
        acc32 = _mm256_add_epi32(acc32, _mm256_madd_epi16(c0, _mm256_load_si256(w0 + i)));
        acc32 = _mm256_add_epi32(acc32, _mm256_madd_epi16(c1, _mm256_load_si256(w1v + i)));
    }
    __m128i s128 = _mm_add_epi32(_mm256_castsi256_si128(acc32),
                                 _mm256_extracti128_si256(acc32, 1));
    s128 = _mm_add_epi32(s128, _mm_shuffle_epi32(s128, 0x4E));
    s128 = _mm_add_epi32(s128, _mm_shuffle_epi32(s128, 0xB1));
    const int32_t sum = _mm_cvtsi128_si32(s128);
#else
    int32_t sum = 0;
    for (int h = 0; h < kHidden; ++h) {
        const int a = as[h] < 0 ? 0 : (as[h] > kActivationMax ? kActivationMax : int(as[h]));
        sum += a * int(g_w2[h]);
    }
    for (int h = 0; h < kHidden; ++h) {
        const int a = an[h] < 0 ? 0 : (an[h] > kActivationMax ? kActivationMax : int(an[h]));
        sum += a * int(g_w2[kHidden + h]);
    }
#endif
    return (sum + g_b2) / kOutputDivisor;
}

int evaluate_from_scratch(Position& pos) {
    refresh(pos);
    return evaluate(pos);
}

}  // namespace NNUE
}  // namespace Huginn
