/**
 * @file magic_bitboards.hpp
 * @brief Real magic-bitboard slider attack tables (BACKLOG #24).
 *
 * Replaces the runtime ray-walking implementation of bishop_attacks()
 * and rook_attacks() in bitboard.cpp with O(1) magic-multiplication
 * lookups. Identical public API — callers in movegen_bb.cpp,
 * attack_detection.cpp, see.cpp, and the mobility eval in search.cpp
 * are unchanged.
 *
 * ## Layout: plain magic, fixed per-piece shifts
 *
 * - Rook: every square gets a 4096-entry attack table (= 2^12 since
 *   the rook's worst-case relevant-occupancy mask has 12 bits).
 *   Shift is a constant 52 for all squares.
 * - Bishop: every square gets a 512-entry table (= 2^9). Shift is 55.
 *
 * Total RAM: 64*4096*8 + 64*512*8 = ~2.25 MB. Larger than fancy magic
 * (~840 KB) but the code is simpler and the working set is still tiny
 * relative to modern caches.
 *
 * ## Magic provenance
 *
 * Magic numbers are *found* at init from a deterministic 64-bit
 * xorshift PRNG seeded with a hardcoded constant (see MAGIC_SEED in
 * magic_bitboards.cpp). The same seed always yields the same magics,
 * so behaviour is bit-identical across runs and machines — equivalent
 * to a hardcoded magic table, but expressed as ~30 lines of generator
 * code rather than 128 hand-transcribed uint64s.
 *
 * After magics are found, every (square, occupancy-subset) is
 * verified against the existing slow_*_attacks ray-walker. A mismatch
 * triggers std::abort — better to fail loudly at init than silently
 * corrupt search.
 *
 * @see bitboard.cpp for the public bishop_attacks()/rook_attacks()
 *      wrappers that delegate here, plus the slow_*_attacks reference
 *      kept for verification.
 */
#pragma once

#include <cstdint>

// ENABLE_PEXT: BACKLOG #32 — BMI2 PEXT slider-attack indexing. On BMI2 CPUs
// (Zen3+/modern Intel) `_pext_u64(occ, mask)` computes the table index in one
// instruction, replacing the magic mask + multiply + shift. KEY FACT: the
// PEXT index is a DIFFERENT permutation of the blocker subsets than the magic
// index, so the tables must be FILLED with the matching indexing at init —
// a magic-built table cannot be read with pext lookups. Layout and size are
// unchanged either way (each square uses 2^popcount(mask) <= per-square-size
// slots), so only the index computation differs; init under PEXT skips the
// magic search entirely (MAGICS stay zero, unused). Requires BMI2 at runtime
// — build-gated, not CPU-dispatched (per-machine builds handle that). Attack
// sets are identical in both arms (verify_or_die checks the active indexing
// against the ray-walker), so node counts/signatures must match exactly.
// Default ON on this branch (test arm); build the magic fallback arm
// (byte-identical t22 behavior) with -DENABLE_PEXT=0.
#ifndef ENABLE_PEXT
#define ENABLE_PEXT 1
#endif

#if ENABLE_PEXT
#include <immintrin.h>  // _pext_u64 (BMI2)
#endif

/**
 * @namespace Magic
 * @brief Magic-bitboard slider attack generation.
 *
 * Precomputed magic multipliers and per-square attack tables that turn a
 * rook/bishop occupancy bitboard into the set of attacked squares with a
 * single multiply-shift-index lookup (O(1) sliding-piece attacks), replacing
 * runtime ray walking.
 */
namespace Magic {

// ---- Table dimensions (fixed-shift plain magic) ---------------------
//
// Worst-case mask popcount: 12 for rooks (corner squares),
// 9 for bishops (center squares). Using a uniform per-piece shift
// means every square's attack table is the same size — simple
// indexing, no per-square pointer arithmetic.
constexpr int ROOK_MASK_BITS_MAX   = 12;
constexpr int BISHOP_MASK_BITS_MAX = 9;
constexpr int ROOK_SHIFT   = 64 - ROOK_MASK_BITS_MAX;     // 52
constexpr int BISHOP_SHIFT = 64 - BISHOP_MASK_BITS_MAX;   // 55
constexpr int ROOK_TABLE_SIZE_PER_SQ   = 1 << ROOK_MASK_BITS_MAX;   // 4096
constexpr int BISHOP_TABLE_SIZE_PER_SQ = 1 << BISHOP_MASK_BITS_MAX; //  512

// ---- Globally-visible tables (populated by init_magic_bitboards) ----
//
// MASKS[sq] is the set of squares whose occupancy can block this
// piece's attacks from sq (excludes edge squares since rays always
// stop at the edge regardless of occupancy there).
extern uint64_t ROOK_MASKS[64];
extern uint64_t BISHOP_MASKS[64];

// MAGICS[sq] is the per-square 64-bit multiplier that produces a
// collision-free perfect-hash from (occupancy & MASKS[sq]) into
// [0, TABLE_SIZE_PER_SQ).
extern uint64_t ROOK_MAGICS[64];
extern uint64_t BISHOP_MAGICS[64];

// ATTACK_TABLE[sq][index] is the precomputed attack bitboard for the
// occupancy pattern that produces `index` via the magic transform.
extern uint64_t ROOK_ATTACK_TABLE[64][ROOK_TABLE_SIZE_PER_SQ];
extern uint64_t BISHOP_ATTACK_TABLE[64][BISHOP_TABLE_SIZE_PER_SQ];

// Find magics, populate masks + attack tables, verify against
// ray-walker. std::abort on any verification mismatch.
// Idempotent: subsequent calls are no-ops.
void init_magic_bitboards();

// ---- Hot-path lookups ------------------------------------------------
//
// Inline so callers in bitboard.cpp's bishop_attacks/rook_attacks
// (which themselves are likely to be inlined further) compile down to
// a mask + multiply + shift + indexed load (magic) or a single pext +
// indexed load (ENABLE_PEXT). pext extracts exactly the mask bits, so
// the `occupied & mask` pre-mask is implicit.
inline uint64_t magic_rook_attacks(int sq, uint64_t occupied) {
#if ENABLE_PEXT
    const uint64_t index = _pext_u64(occupied, ROOK_MASKS[sq]);
#else
    const uint64_t blockers = occupied & ROOK_MASKS[sq];
    const uint64_t index    = (blockers * ROOK_MAGICS[sq]) >> ROOK_SHIFT;
#endif
    return ROOK_ATTACK_TABLE[sq][index];
}

inline uint64_t magic_bishop_attacks(int sq, uint64_t occupied) {
#if ENABLE_PEXT
    const uint64_t index = _pext_u64(occupied, BISHOP_MASKS[sq]);
#else
    const uint64_t blockers = occupied & BISHOP_MASKS[sq];
    const uint64_t index    = (blockers * BISHOP_MAGICS[sq]) >> BISHOP_SHIFT;
#endif
    return BISHOP_ATTACK_TABLE[sq][index];
}

} // namespace Magic
