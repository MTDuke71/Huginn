#pragma once
// MSVC-specific performance optimizations
// Provides MSVC compiler-specific optimizations and hints

#ifdef _MSC_VER

#include <intrin.h>

// ---- Compiler Optimization Hints ----

// Force function inlining for critical functions
#define FORCE_INLINE __forceinline

// Likely/unlikely branch prediction hints using C++20 attributes
// Note: These need to be used as statement attributes, not expression attributes
#define LIKELY_IF(condition) if (condition) [[likely]]
#define UNLIKELY_IF(condition) if (condition) [[unlikely]]

// For expression-based hints, use __builtin_expect or simple passthrough
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)

// Memory prefetch hints
#define PREFETCH_READ(ptr)  _mm_prefetch((const char*)(ptr), _MM_HINT_T0)
#define PREFETCH_WRITE(ptr) _mm_prefetch((const char*)(ptr), _MM_HINT_T0)

// ---- MSVC-Optimized Bit Operations ----

// Fast bit manipulation using BMI instructions when available
namespace MSVCOptimizations {

    // Fast population count using hardware instruction
    FORCE_INLINE int popcount(uint64_t x) {
        #ifdef _M_X64
            return (int)__popcnt64(x);
        #else
            return (int)(__popcnt((uint32_t)(x)) + __popcnt((uint32_t)(x >> 32)));
        #endif
    }

    // Fast trailing zero count using BSF instruction
    FORCE_INLINE int ctz(uint64_t x) {
        if (x == 0) [[unlikely]] return 64;
        unsigned long index;
        #ifdef _M_X64
            _BitScanForward64(&index, x);
        #else
            if ((uint32_t)x != 0) {
                _BitScanForward(&index, (uint32_t)x);
            } else {
                _BitScanForward(&index, (uint32_t)(x >> 32));
                index += 32;
            }
        #endif
        return (int)index;
    }

    // Fast leading zero count using BSR instruction
    FORCE_INLINE int clz(uint64_t x) {
        if (x == 0) [[unlikely]] return 64;
        unsigned long index;
        #ifdef _M_X64
            _BitScanReverse64(&index, x);
            return 63 - (int)index;
        #else
            if (x >> 32) {
                _BitScanReverse(&index, (uint32_t)(x >> 32));
                return 31 - (int)index;
            } else {
                _BitScanReverse(&index, (uint32_t)x);
                return 63 - (int)index;
            }
        #endif
    }

    // Extract lowest set bit using BLSI instruction if available
    FORCE_INLINE uint64_t blsi(uint64_t x) {
        #ifdef __BMI__
            return _blsi_u64(x);
        #else
            return x & (0ULL - x);  // Fallback: isolate lowest set bit (avoid signed warning)
        #endif
    }

    // Reset lowest set bit using BLSR instruction if available  
    FORCE_INLINE uint64_t blsr(uint64_t x) {
        #ifdef __BMI__
            return _blsr_u64(x);
        #else
            return x & (x - 1);  // Fallback: clear lowest set bit
        #endif
    }

    // ---- MSVC-Optimized Array Access ----
    
    // Bounds-check elimination hint for known safe array access
    template<typename T, size_t N>
    FORCE_INLINE T& array_access_unsafe(T (&arr)[N], size_t index) {
        __assume(index < N);  // Tell MSVC this access is always valid
        return arr[index];
    }
}

#else // Non-MSVC compiler fallbacks

// Fallback definitions for non-MSVC compilers (GCC, Clang)
#define FORCE_INLINE inline
#define LIKELY_IF(condition) if (condition)
#define UNLIKELY_IF(condition) if (condition)
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#define PREFETCH_READ(ptr)  __builtin_prefetch((ptr), 0, 3)
#define PREFETCH_WRITE(ptr) __builtin_prefetch((ptr), 1, 3)

// GCC doesn't have __assume, but we can use __builtin_unreachable for similar effect
#define __assume(x) do { if (!(x)) __builtin_unreachable(); } while(0)

// GCC/Clang equivalents for bit operations
namespace MSVCOptimizations {
    inline int popcount(uint64_t x) { return __builtin_popcountll(x); }
    inline int ctz(uint64_t x) { return x ? __builtin_ctzll(x) : 64; }
    inline int clz(uint64_t x) { return x ? __builtin_clzll(x) : 64; }
    inline uint64_t blsi(uint64_t x) { return x & (0ULL - x); }
    inline uint64_t blsr(uint64_t x) { return x & (x - 1); }
    
    template<typename T, size_t N>
    inline T& array_access_unsafe(T (&arr)[N], size_t index) {
        return arr[index];  // GCC optimizes bounds checks well
    }
}

#endif // _MSC_VER
