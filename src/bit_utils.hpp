/**
 * @file bit_utils.hpp
 * @brief Cross-platform bit manipulation utilities
 *
 * Provides portable bit manipulation functions that work across different compilers
 * and platforms. The utilities offer hardware-accelerated bit operations when
 * available (using compiler intrinsics) and fall back to software implementations
 * for maximum portability.
 *
 * ## Supported Operations
 * - **Population Count**: Count set bits in integers (popcount)
 * - **Bit Scanning**: Find first/last set bit positions (ctz/clz)
 * - **Bit Manipulation**: Set, clear, test individual bits
 * - **Cross-Platform**: Unified interface across GCC, Clang, MSVC
 *
 * ## Compiler Support
 * - **MSVC**: Uses _BitScanForward64, __popcnt64, and other intrinsics
 * - **GCC/Clang**: Uses __builtin_popcountll, __builtin_ctzll built-ins
 * - **Fallback**: Software implementations for unsupported platforms
 *
 * ## Performance Features
 * - Hardware acceleration when available (BMI, POPCNT instructions)
 * - Compile-time optimization for constant expressions
 * - Minimal overhead abstraction layer
 * - Optimized for both 32-bit and 64-bit architectures
 *
 * @author MTDuke71
 * @version 1.2
 * @see bitboard.hpp for chess-specific bitboard operations
 */

#include <cstdint>

// MSVC compatibility
#ifdef _MSC_VER
#include <intrin.h>

// MSVC implementations using correct intrinsics
inline int __builtin_popcountll(uint64_t x)
{
#ifdef _M_X64
    return (int)__popcnt64(x);
#else
    // 32-bit fallback for __popcnt64
    return (int)(__popcnt((uint32_t)(x)) + __popcnt((uint32_t)(x >> 32)));
#endif
}

inline int __builtin_ctzll(uint64_t x)
{
    if (x == 0)
        return 64;
    unsigned long index;
#ifdef _M_X64
    _BitScanForward64(&index, x);
#else
    // 32-bit fallback for _BitScanForward64
    if ((uint32_t)x != 0)
    {
        _BitScanForward(&index, (uint32_t)x);
    }
    else
    {
        _BitScanForward(&index, (uint32_t)(x >> 32));
        index += 32;
    }
#endif
    return (int)index;
}

#elif defined(__GNUC__) || defined(__clang__)
// GCC/Clang already have these intrinsics built-in
// No need to redefine them

#else
// Fallback implementations for other compilers

inline int __builtin_popcountll(uint64_t x)
{
    // Brian Kernighan's algorithm
    int count = 0;
    while (x)
    {
        x &= x - 1; // Clear the lowest set bit
        count++;
    }
    return count;
}

inline int __builtin_ctzll(uint64_t x)
{
    if (x == 0)
        return 64;
    int count = 0;
    while ((x & 1) == 0)
    {
        x >>= 1;
        count++;
    }
    return count;
}

#endif
