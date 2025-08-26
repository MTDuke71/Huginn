# Assembly Analysis Guide for Huginn Chess Engine

## Overview
This directory contains C++ source files and their corresponding assembly output for performance analysis of the Huginn chess engine.

## Generated Files

### Directory Structure
```
asm_output/
??? cpp/                    # C++ source files for reference
?   ??? bitboard.cpp
?   ??? movegen_enhanced.cpp
?   ??? attack_detection.cpp
?   ??? position.cpp
?   ??? evaluation.cpp
?   ??? search.cpp
?   ??? hybrid_evaluation.cpp
?   ??? simple_search.cpp
??? asm/                    # Assembly output files
?   ??? bitboard.asm
?   ??? movegen_enhanced.asm
?   ??? attack_detection.asm
?   ??? position.asm
?   ??? evaluation.asm
?   ??? search.asm
?   ??? hybrid_evaluation.asm
?   ??? simple_search.asm
??? generation_log.txt      # Generation metadata
```

## Key Files for Performance Analysis

### 1. **bitboard.asm** (769KB)
- Bit manipulation operations
- Intrinsic functions (__builtin_ctzll, __builtin_popcountll)
- Performance-critical bit operations

### 2. **movegen_enhanced.asm** (1.3MB)
- Move generation algorithms
- Sliding piece move generation
- Pawn move optimizations

### 3. **attack_detection.asm** (63KB)
- Square attack detection
- Piece attack patterns
- King safety calculations

### 4. **position.asm** (981KB)
- Position representation
- Move making/unmaking
- Zobrist hashing

### 5. **evaluation.asm** (1.8MB)
- Position evaluation functions
- Material counting
- Positional factors

### 6. **search.asm** (2.6MB)
- Alpha-beta search
- Move ordering
- Transposition table

### 7. **hybrid_evaluation.asm** (185KB)
- Advanced evaluation features
- Game phase detection
- Piece-square tables

## How to Analyze

### Using Visual Studio Code
1. Install the "x86 and x86_64 Assembly" extension
2. Open both .cpp and .asm files side by side
3. Use Ctrl+\ to create split view
4. Compare C++ code with generated assembly

### Using Visual Studio
1. Open the .asm files in Visual Studio
2. Assembly syntax highlighting is built-in
3. Use the Compare Files feature to view side by side

### Analysis Tips

#### Performance Hotspots to Look For:
1. **Loop unrolling** - Compiler optimization in tight loops
2. **Vectorization** - SIMD instructions for parallel operations
3. **Inlining** - Function calls eliminated for performance
4. **Register usage** - Efficient register allocation
5. **Branch prediction** - Conditional jump optimization

#### Key Assembly Patterns:
- `popcnt` - Population count (bit counting)
- `bsf/bsr` - Bit scan forward/reverse
- `movzx/movsx` - Zero/sign extension
- `lea` - Efficient address calculation
- `test/cmp` - Condition testing

#### Critical Functions to Analyze:
1. **Bit manipulation functions** in bitboard.asm
2. **Move generation loops** in movegen_enhanced.asm
3. **Attack detection** in attack_detection.asm
4. **Search inner loops** in search.asm

## Compiler Information
- **Compiler**: MSVC x64 Release
- **Optimization**: /O2 (maximize speed)
- **Target**: x64 architecture
- **C++ Standard**: C++17
- **Generated**: See generation_log.txt for timestamp

## Performance Analysis Workflow

1. **Identify bottlenecks** using profiling tools
2. **Locate corresponding assembly** for hot functions
3. **Analyze compiler optimizations** - are they effective?
4. **Look for optimization opportunities**:
   - Manual loop unrolling
   - Algorithm changes
   - Data structure improvements
   - Compiler hint additions
5. **Make changes** and regenerate assembly
6. **Compare before/after** assembly output

## Regenerating Assembly
Run `generate_asm_simple.bat` to regenerate all assembly files after code changes.

## Notes
- Assembly files are generated with MSVC /O2 optimization
- Files include both optimized assembly and source comments
- Large files (search.asm, evaluation.asm) contain extensive optimizations
- Use search functionality in your editor to find specific functions