/**
 * @file msvc_intrinsics.hpp
 * @brief Shared MSVC intrinsics compatibility layer
 * @author Huginn Chess Engine Development Team
 * @date September 2025
 * @version 1.0
 * 
 * This header provides a unified compatibility layer for bit manipulation intrinsics
 * across different compilers. It prevents redefinition errors when multiple lookup
 * table headers are included together.
 */

#pragma once

#include <cstdint>

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(_BitScanForward64)
    
    #ifndef HUGINN_INTRINSICS_DEFINED
    #define HUGINN_INTRINSICS_DEFINED
    
    /**
     * @brief Count trailing zeros in 64-bit value (MSVC version)
     * @param value 64-bit integer to analyze
     * @return Number of trailing zero bits, or 64 if value is 0
     */
    inline int huginn_ctz64(uint64_t value) {
        unsigned long index;
        return _BitScanForward64(&index, value) ? index : 64;
    }
    
    /**
     * @brief Count set bits in 64-bit value (MSVC version)
     * @param value 64-bit integer to analyze
     * @return Number of set bits in the value
     */
    inline int huginn_popcount64(uint64_t value) {
        return __popcnt64(value);
    }
    
    #endif // HUGINN_INTRINSICS_DEFINED
    
    #define builtin_ctzll huginn_ctz64
    #define builtin_popcountll huginn_popcount64
#else
    /// GCC/Clang built-ins
    #define builtin_ctzll __builtin_ctzll
    #define builtin_popcountll __builtin_popcountll
#endif