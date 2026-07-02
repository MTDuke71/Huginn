/**
 * @file magic_bitboards.cpp
 * @brief Init-time magic finder + attack table generation.
 * @see magic_bitboards.hpp for the public interface and design notes.
 */
#include "magic_bitboards.hpp"
#include "bit_utils.hpp"  // cross-platform __builtin_popcountll / __builtin_ctzll

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace Magic {

// ---- Storage (filled by init_magic_bitboards) ----------------------
uint64_t ROOK_MASKS[64];     ///< Per-square rook relevant-occupancy mask.
uint64_t BISHOP_MASKS[64];   ///< Per-square bishop relevant-occupancy mask.
uint64_t ROOK_MAGICS[64];    ///< Per-square rook magic multiplier.
uint64_t BISHOP_MAGICS[64];  ///< Per-square bishop magic multiplier.
uint64_t ROOK_ATTACK_TABLE[64][ROOK_TABLE_SIZE_PER_SQ];     ///< Rook attacks indexed by magic hash.
uint64_t BISHOP_ATTACK_TABLE[64][BISHOP_TABLE_SIZE_PER_SQ]; ///< Bishop attacks indexed by magic hash.

namespace {

// ---- Reference ray-walker ------------------------------------------
//
// Used for (a) populating the attack tables during init, and
// (b) verifying every magic lookup against ground truth before we
// commit to it. Independent of bitboard.cpp's slow_*_attacks (which
// remain in place behind the public bishop_attacks/rook_attacks
// wrappers); duplicating ~20 lines here keeps magic_bitboards.cpp
// self-contained and lets us drop the bitboard.cpp ray-walker later
// without breaking init.

// Step deltas as (drank, dfile). Rook = NESW; bishop = 4 diagonals.
struct Step { int dr, df; };
constexpr Step ROOK_STEPS[4]   = {{ 1, 0}, {-1, 0}, { 0, 1}, { 0,-1}};
constexpr Step BISHOP_STEPS[4] = {{ 1, 1}, { 1,-1}, {-1, 1}, {-1,-1}};

/// @brief Reference slider attacks from @p sq under @p occupied along @p steps
///        (stop at the first blocker, inclusive). Ground truth for init + verify.
uint64_t ray_attacks(int sq, uint64_t occupied, const Step (&steps)[4]) {
    uint64_t attacks = 0;
    const int r0 = sq / 8;
    const int f0 = sq % 8;
    for (const Step& s : steps) {
        int r = r0 + s.dr, f = f0 + s.df;
        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            const int target = r * 8 + f;
            attacks |= (1ULL << target);
            if (occupied & (1ULL << target)) break;
            r += s.dr;
            f += s.df;
        }
    }
    return attacks;
}

// ---- Relevant-occupancy masks --------------------------------------
//
// "Relevant" = squares whose occupancy could block this piece's
// attacks from `sq`. EXCLUDES the squares at the edge of the board
// along the ray, because whether or not those are occupied doesn't
// change the attack set (the ray ends at the edge anyway).
//
// Example: rook on a1, north ray reaches a2..a8. The blocker mask
// includes a2..a7 (six squares) but excludes a8: putting an enemy
// on a8 vs leaving a8 empty both produce the same attack set
// (a2..a8 — the rook attacks the edge square either way).

/// @brief Rook relevant-occupancy mask for @p sq (ray squares excluding board edges).
uint64_t compute_rook_mask(int sq) {
    uint64_t mask = 0;
    const int r0 = sq / 8, f0 = sq % 8;
    for (int r = r0 + 1; r <= 6; ++r) mask |= 1ULL << (r * 8 + f0);
    for (int r = r0 - 1; r >= 1; --r) mask |= 1ULL << (r * 8 + f0);
    for (int f = f0 + 1; f <= 6; ++f) mask |= 1ULL << (r0 * 8 + f);
    for (int f = f0 - 1; f >= 1; --f) mask |= 1ULL << (r0 * 8 + f);
    return mask;
}

/// @brief Bishop relevant-occupancy mask for @p sq (diagonal squares excluding edges).
uint64_t compute_bishop_mask(int sq) {
    uint64_t mask = 0;
    const int r0 = sq / 8, f0 = sq % 8;
    for (int r = r0 + 1, f = f0 + 1; r <= 6 && f <= 6; ++r, ++f) mask |= 1ULL << (r * 8 + f);
    for (int r = r0 + 1, f = f0 - 1; r <= 6 && f >= 1; ++r, --f) mask |= 1ULL << (r * 8 + f);
    for (int r = r0 - 1, f = f0 + 1; r >= 1 && f <= 6; --r, ++f) mask |= 1ULL << (r * 8 + f);
    for (int r = r0 - 1, f = f0 - 1; r >= 1 && f >= 1; --r, --f) mask |= 1ULL << (r * 8 + f);
    return mask;
}

// ---- Subset enumeration --------------------------------------------
//
// Carry-Rippler trick: iterate every subset of a bitmask in
// 2^popcount(mask) iterations. Used to enumerate every possible
// blocker pattern for a given mask.
//
// The Nth subset (n = 0..2^bits-1) is obtained by bit-spreading n
// into the bit positions set in mask.

/// @brief The @p n-th subset of @p mask's set bits (spread @p n into mask's bit
///        positions) — enumerates all 2^popcount(mask) blocker patterns.
uint64_t nth_subset_of_mask(uint64_t mask, uint64_t n) {
    uint64_t result = 0;
    uint64_t m = mask;
    uint64_t bit_index = 0;
    while (m) {
        const int low = __builtin_ctzll(m);
        if (n & (1ULL << bit_index)) {
            result |= (1ULL << low);
        }
        m &= m - 1;
        ++bit_index;
    }
    return result;
}

#if !ENABLE_PEXT
// ---- xorshift64* PRNG (deterministic, seeded once) -----------------
//
// Sparse random numbers (AND of three xorshift outputs) have low
// popcount, which empirically yields valid magics in fewer trials —
// this is the standard trick from the chessprogramming.org Magic
// Bitboards entry.

struct XorShift64 {
    uint64_t state;
    explicit XorShift64(uint64_t seed) : state(seed) {}
    uint64_t next() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    }
    // AND of 2 (popcount ~16) is the sweet spot for fixed-shift magic
    // bitboards: sparser candidates (AND of 3, popcount ~8) fail to
    // disperse the 4096 subsets in worst-case corner squares; denser
    // candidates lose the hit-rate benefit.
    uint64_t sparse() { return next() & next(); }
};

// Hardcoded seed — the only "magic constant" in this file. Any
// non-zero 64-bit value works (the search finds valid magics for any
// seed eventually); this one happens to converge quickly for every
// square. Change at your peril, but it doesn't change correctness —
// only how many candidates the search has to try before it lands on
// a hit.
constexpr uint64_t MAGIC_SEED = 0x12345678ABCDEFULL;

// ---- Magic finder --------------------------------------------------
//
// For a given square + reference attack function, search the PRNG
// stream for a 64-bit multiplier such that
//     index(s) = (s * magic) >> shift
// is a perfect hash from "every subset of the relevant-occupancy
// mask" into "[0, table_size)". Collisions are allowed only if both
// colliding subsets produce the same attack set (those are redundant
// occupancy patterns from the slider's POV).
//
// Writes the discovered magic to *out_magic and fills the per-square
// attack table at *attack_table.

/// @brief Search the PRNG stream for a magic multiplier that perfectly hashes
///        every blocker subset of @p mask into `[0, table_size)` via
///        `(subset*magic) >> shift` (collisions allowed only when the attack
///        set matches). Writes the magic to @p out_magic and fills @p attack_table.
/// @return true if a magic was found.
template <typename StepArr>
bool find_magic_for_square(int sq,
                           uint64_t mask,
                           int shift,
                           uint64_t* attack_table,
                           int table_size,
                           const StepArr& steps,
                           XorShift64& rng,
                           uint64_t* out_magic) {
    const int bits = __builtin_popcountll(mask);
    const int n_subsets = 1 << bits;

    // Precompute (subset, attack) pairs once — reused across magic
    // candidates.
    std::vector<uint64_t> subsets(n_subsets);
    std::vector<uint64_t> attacks(n_subsets);
    for (int i = 0; i < n_subsets; ++i) {
        subsets[i] = nth_subset_of_mask(mask, i);
        attacks[i] = ray_attacks(sq, subsets[i], steps);
    }

    // Try sparse candidates until one produces a collision-free map.
    // 1M attempts is far more than the typical convergence (hundreds
    // to low thousands of tries) — gives us headroom if a particular
    // PRNG stream wanders through an unproductive region.
    constexpr int MAX_ATTEMPTS = 1'000'000;
    std::vector<uint64_t> used(table_size, 0);
    std::vector<bool> filled(table_size, false);

    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        const uint64_t candidate = rng.sparse();

        // Cheap pre-filter: skip candidates whose top byte of
        // (mask * candidate) has fewer than 4 set bits — those tend
        // not to disperse subsets into the high-bit range we extract.
        // Threshold relaxed from 6 to 4 after observing fixed-shift
        // corner squares (12-bit masks) miss valid magics at 6.
        if (__builtin_popcountll((mask * candidate) & 0xFF00000000000000ULL) < 4) {
            continue;
        }

        // Reset bookkeeping for this attempt.
        std::fill(filled.begin(), filled.end(), false);

        bool ok = true;
        for (int i = 0; i < n_subsets && ok; ++i) {
            const uint64_t idx = (subsets[i] * candidate) >> shift;
            if (!filled[idx]) {
                used[idx] = attacks[i];
                filled[idx] = true;
            } else if (used[idx] != attacks[i]) {
                ok = false;  // real collision (different attack sets)
            }
            // Equal-attack collisions are fine — we just overwrite
            // with the same value.
        }

        if (ok) {
            // Commit: copy filled slots into the public table.
            for (int i = 0; i < table_size; ++i) {
                attack_table[i] = filled[i] ? used[i] : 0;
            }
            *out_magic = candidate;
            return true;
        }
    }
    return false;
}
#endif // !ENABLE_PEXT

#if ENABLE_PEXT
// ---- PEXT table fill (BACKLOG #32) ----------------------------------
//
// The pext index of a blocker subset is its subset ordinal:
// pext(nth_subset_of_mask(mask, n), mask) == n by construction
// (nth_subset_of_mask spreads n's bits into mask's bit positions;
// pext extracts them back out). So the table fill is direct — entry n
// is the ray attack set of the n-th subset. No search, no collisions;
// slots above 2^popcount(mask) are never indexed (pext yields
// < 2^popcount(mask)) and stay zero.

/// @brief Fill @p attack_table with ray attacks indexed by pext(subset, mask).
template <typename StepArr>
void fill_pext_table(int sq, uint64_t mask, uint64_t* attack_table,
                     const StepArr& steps) {
    const int bits = __builtin_popcountll(mask);
    const int n_subsets = 1 << bits;
    for (int i = 0; i < n_subsets; ++i) {
        attack_table[i] = ray_attacks(sq, nth_subset_of_mask(mask, i), steps);
    }
}
#endif // ENABLE_PEXT

// ---- Verifier ------------------------------------------------------
//
// After init, walk every (square, every subset of its mask) and
// confirm magic_*_attacks() agrees with the ray-walker. Verifies
// whichever indexing is active (magic multiply or pext). Bug catcher:
// if a magic is silently wrong for some occupancy, the engine would
// generate illegal moves or miss legal ones in obscure positions.

void verify_or_die() {
    for (int sq = 0; sq < 64; ++sq) {
        const uint64_t rmask = ROOK_MASKS[sq];
        const int rbits = __builtin_popcountll(rmask);
        for (uint64_t s = 0; s < (1ULL << rbits); ++s) {
            const uint64_t occ = nth_subset_of_mask(rmask, s);
            const uint64_t magic_result = magic_rook_attacks(sq, occ);
            const uint64_t ref_result   = ray_attacks(sq, occ, ROOK_STEPS);
            if (magic_result != ref_result) {
                std::fprintf(stderr,
                    "FATAL: rook magic mismatch sq=%d occ=0x%llx "
                    "magic=0x%llx ref=0x%llx\n",
                    sq, (unsigned long long)occ,
                    (unsigned long long)magic_result,
                    (unsigned long long)ref_result);
                std::abort();
            }
        }

        const uint64_t bmask = BISHOP_MASKS[sq];
        const int bbits = __builtin_popcountll(bmask);
        for (uint64_t s = 0; s < (1ULL << bbits); ++s) {
            const uint64_t occ = nth_subset_of_mask(bmask, s);
            const uint64_t magic_result = magic_bishop_attacks(sq, occ);
            const uint64_t ref_result   = ray_attacks(sq, occ, BISHOP_STEPS);
            if (magic_result != ref_result) {
                std::fprintf(stderr,
                    "FATAL: bishop magic mismatch sq=%d occ=0x%llx "
                    "magic=0x%llx ref=0x%llx\n",
                    sq, (unsigned long long)occ,
                    (unsigned long long)magic_result,
                    (unsigned long long)ref_result);
                std::abort();
            }
        }
    }
}

} // namespace

// ---- Public init ---------------------------------------------------

/// @brief Build all per-square masks, populate the attack tables (magic search
///        or direct pext fill per ENABLE_PEXT), and verify. Call once at
///        startup before magic_rook_attacks / magic_bishop_attacks. Aborts if
///        any magic search fails (should never happen with the fixed seed).
void init_magic_bitboards() {
    static bool done = false;
    if (done) return;

    // 1. Compute relevant-occupancy masks for all squares.
    for (int sq = 0; sq < 64; ++sq) {
        ROOK_MASKS[sq]   = compute_rook_mask(sq);
        BISHOP_MASKS[sq] = compute_bishop_mask(sq);
    }

#if ENABLE_PEXT
    // 2. Fill tables with the pext index (BACKLOG #32) — no magic search;
    //    MAGICS arrays stay zero (unused by the pext lookup).
    for (int sq = 0; sq < 64; ++sq) {
        fill_pext_table(sq, ROOK_MASKS[sq],   ROOK_ATTACK_TABLE[sq],   ROOK_STEPS);
        fill_pext_table(sq, BISHOP_MASKS[sq], BISHOP_ATTACK_TABLE[sq], BISHOP_STEPS);
    }
#else
    // 2. Find magics (deterministic via fixed PRNG seed).
    XorShift64 rng(MAGIC_SEED);
    for (int sq = 0; sq < 64; ++sq) {
        if (!find_magic_for_square(sq, ROOK_MASKS[sq], ROOK_SHIFT,
                                    ROOK_ATTACK_TABLE[sq],
                                    ROOK_TABLE_SIZE_PER_SQ,
                                    ROOK_STEPS, rng,
                                    &ROOK_MAGICS[sq])) {
            std::fprintf(stderr, "FATAL: no rook magic found for sq=%d\n", sq);
            std::abort();
        }
        if (!find_magic_for_square(sq, BISHOP_MASKS[sq], BISHOP_SHIFT,
                                    BISHOP_ATTACK_TABLE[sq],
                                    BISHOP_TABLE_SIZE_PER_SQ,
                                    BISHOP_STEPS, rng,
                                    &BISHOP_MAGICS[sq])) {
            std::fprintf(stderr, "FATAL: no bishop magic found for sq=%d\n", sq);
            std::abort();
        }
    }
#endif

    // 3. Exhaustive verification before any search code touches these.
    verify_or_die();

    done = true;
}

} // namespace Magic
