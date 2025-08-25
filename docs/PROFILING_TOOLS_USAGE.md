# Move Generation Profiling Tools

## Overview

This directory contains comprehensive profiling tools for analyzing the performance of move generation functions in the Huginn chess engine.

## Available Tools

### 1. Move Generation Profiler (`movegen_profiler`)

**Purpose**: Detailed analysis of individual move generation functions
**Usage**: `./movegen_profiler`

**Features**:
- Individual timing for each piece type (pawn, knight, bishop, rook, queen, king)
- Legal move filtering performance analysis
- Multiple test positions with varying characteristics
- Comprehensive optimization recommendations
- Statistical analysis with nanosecond precision

**Sample Output**:
```
Huginn Chess Engine - Move Generation Profiler
=== COMPREHENSIVE MOVE GENERATION PROFILING ===

Profiling: Starting Position
  Position: Starting Position (20 moves)
  Function       Time (ns)   % Total   Moves   ns/move     
  Pawn Moves     131740.0    38.7      16000   8.2         
  Knight Moves   51706.0     15.2      4000    12.9        
  ...

=== OPTIMIZATION RECOMMENDATIONS ===
1. **Pawn Moves** consumes 23.9% of move generation time
   - Consider optimizing pawn promotion handling
   - En passant detection could be streamlined
```

### 2. Profiler Validation Test (`profiler_test`)

**Purpose**: Verify profiler accuracy and correctness
**Usage**: `./profiler_test`

**Features**:
- Validates that profiled move generation produces identical results
- Ensures timing doesn't affect move generation correctness
- Quick smoke test for profiler functionality

### 3. Parallel Benchmark (`parallel_benchmark`)

**Purpose**: Compare sequential vs parallel move generation performance
**Usage**: `./parallel_benchmark`

**Features**:
- Benchmarks parallel vs sequential implementations
- Analysis of multithreading effectiveness
- Performance comparison across different positions

## Building

```bash
mkdir -p build && cd build
cmake ..
make movegen_profiler profiler_test parallel_benchmark -j4
```

## Analysis Workflow

1. **Run Validation**: `./profiler_test` - Ensure profiler accuracy
2. **Run Profiling**: `./movegen_profiler` - Get detailed analysis
3. **Review Results**: Check `docs/MOVEGEN_PROFILING_ANALYSIS.md` for interpretation
4. **Compare Parallel**: `./parallel_benchmark` - See multithreading performance

## Key Findings

The profiling analysis reveals:

| Function | % of Time | Optimization Priority |
|----------|-----------|----------------------|
| Pawn Moves | 23.9% | **HIGH** - Complex promotions/en passant |
| King Moves | 18.5% | **MEDIUM** - Castling complexity |
| Queen Moves | 15.3% | **MEDIUM** - Sliding piece optimization |
| Legal Filter | 1.1% | **LOW** - Already efficient |

## Optimization Targets

Based on profiling data:

1. **Pawn move generation** - Highest priority (23.9% of time)
   - Optimize promotion move creation
   - Streamline en passant detection
   - Consider bitboard-based pawn pushes

2. **King move generation** - Medium priority (18.5% of time)
   - Separate ordinary moves from castling
   - Pre-compute castling legality

3. **Sliding pieces** - Medium priority (~30% combined)
   - Consider magic bitboard implementation
   - Ray attack lookup tables

## Files

- `src/movegen_profiler.hpp` - Profiling framework interface
- `src/movegen_profiler.cpp` - Detailed profiling implementation
- `src/movegen_profiler_main.cpp` - Main profiler executable
- `src/profiler_test.cpp` - Validation test
- `docs/MOVEGEN_PROFILING_ANALYSIS.md` - Comprehensive analysis document