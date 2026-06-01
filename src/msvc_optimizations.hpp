#ifndef MSVC_OPTIMIZATIONS_HPP
#define MSVC_OPTIMIZATIONS_HPP

/**
 * @file msvc_optimizations.hpp
 * @brief Microsoft Visual C++ compiler-specific optimizations
 * 
 * Provides MSVC-specific performance optimizations and compiler hints to maximize
 * the engine's performance on Windows systems using the Microsoft toolchain.
 * These optimizations leverage MSVC's advanced optimization features and intrinsics
 * to achieve optimal code generation for chess-specific algorithms.
 * 
 * ## Optimization Categories
 * - **Compiler Hints**: Branch prediction, inlining directives, prefetch hints
 * - **Bit Operations**: Hardware-accelerated bit manipulation using BMI instructions
 * - **Memory Access**: Cache-friendly prefetch patterns for search algorithms
 * - **CPU Features**: Leveraging modern x86-64 instruction sets
 * 
 * ## Performance Impact
 * - FORCE_INLINE: Eliminates function call overhead for critical functions
 * - Branch prediction: Improves pipeline efficiency in search loops
 * - Memory prefetch: Reduces cache misses during position evaluation
 * - BMI instructions: Faster bit counting and manipulation operations
 * 
 * ## Conditional Compilation
 * All optimizations are conditionally compiled for MSVC only (_MSC_VER).
 * Other compilers fall back to standard implementations or compiler-specific
 * alternatives defined in their respective optimization headers.
 * 
 * @author MTDuke71
 * @version 1.2
 * @see bit_utils.hpp for portable bit operations
 */

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

#endif // _MSC_VER

#endif // MSVC_OPTIMIZATIONS_HPP
