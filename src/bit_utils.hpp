#pragma once
// bit_utils.hpp - Cross-platform bit manipulation utilities

#include <cstdint>

// MSVC compatibility
#ifdef _MSC_VER
    #include <intrin.h>
    
    // MSVC implementations using correct intrinsics
    inline int __builtin_popcountll(uint64_t x) {
        #ifdef _M_X64
            return (int)__popcnt64(x);
        #else
            // 32-bit fallback for __popcnt64
            return (int)(__popcnt((uint32_t)(x)) + __popcnt((uint32_t)(x >> 32)));
        #endif
    }
    
    inline int __builtin_popcount(uint32_t x) {
        return (int)__popcnt(x);
    }
    
    inline int __builtin_ctzll(uint64_t x) {
        if (x == 0) return 64;
        unsigned long index;
        #ifdef _M_X64
            _BitScanForward64(&index, x);
        #else
            // 32-bit fallback for _BitScanForward64
            if ((uint32_t)x != 0) {
                _BitScanForward(&index, (uint32_t)x);
            } else {
                _BitScanForward(&index, (uint32_t)(x >> 32));
                index += 32;
            }
        #endif
        return (int)index;
    }
    
    inline int __builtin_ctz(uint32_t x) {
        if (x == 0) return 32;
        unsigned long index;
        _BitScanForward(&index, x);
        return (int)index;
    }
    
#elif defined(__GNUC__) || defined(__clang__)
    // GCC/Clang already have these intrinsics built-in
    // No need to redefine them
    
#else
    // Fallback implementations for other compilers
    
    inline int __builtin_popcountll(uint64_t x) {
        // Brian Kernighan's algorithm
        int count = 0;
        while (x) {
            x &= x - 1;  // Clear the lowest set bit
            count++;
        }
        return count;
    }
    
    inline int __builtin_popcount(uint32_t x) {
        return __builtin_popcountll(x);
    }
    
    inline int __builtin_ctzll(uint64_t x) {
        if (x == 0) return 64;
        int count = 0;
        while ((x & 1) == 0) {
            x >>= 1;
            count++;
        }
        return count;
    }
    
    inline int __builtin_ctz(uint32_t x) {
        if (x == 0) return 32;
        return __builtin_ctzll(x);
    }
    
#endif