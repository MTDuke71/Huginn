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

    // ---- MSVC-Optimized Branching ----

    // Optimize switch statements with jump tables
    #define OPTIMIZE_SWITCH() __pragma(optimize("gt", on))

    // Restore normal optimization after switch
    #define RESTORE_OPTIMIZATION() __pragma(optimize("", on))

} // namespace MSVCOptimizations

#else // Not MSVC

// Provide fallback definitions for other compilers
#define FORCE_INLINE inline
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define PREFETCH_READ(ptr) ((void)0)
#define PREFETCH_WRITE(ptr) ((void)0)
#define OPTIMIZE_SWITCH()
#define RESTORE_OPTIMIZATION()

namespace MSVCOptimizations {
    inline int popcount(uint64_t x) { return __builtin_popcountll(x); }
    inline int ctz(uint64_t x) { return __builtin_ctzll(x); }
    inline int clz(uint64_t x) { return __builtin_clzll(x); }
    inline uint64_t blsi(uint64_t x) { return x & (0ULL - x); }
    inline uint64_t blsr(uint64_t x) { return x & (x - 1); }
    
    template<typename T, size_t N>
    inline T& array_access_unsafe(T (&arr)[N], size_t index) {
        return arr[index];
    }
}

#endif // _MSC_VER
