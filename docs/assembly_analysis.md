# Assembly Generation and Performance Analysis

The Huginn chess engine includes comprehensive assembly generation capabilities for performance analysis and optimization. This allows you to view the generated assembly code alongside your C++ source for detailed performance analysis.

## Quick Start

### Generate Assembly Files
```bash
# Simple method (recommended)
.\generate_asm_simple.bat

# Advanced method with CMake
cmake --preset msvc-x64-release-asm
cmake --build build/msvc-x64-release-asm --target generate_assembly
```

### View Results
All generated files are organized in the `asm_output/` directory:
- `asm_output/cpp/` - C++ source files for reference
- `asm_output/asm/` - Generated assembly files
- `asm_output/README.md` - Detailed analysis guide

## Available Methods

### 1. Simple Batch Script (Recommended)
- **File**: `generate_asm_simple.bat`
- **Pros**: Simple, fast, reliable
- **Output**: `asm_output/` directory
- **Usage**: Just run the script

### 2. CMake Integration
- **Files**: Enhanced `CMakeLists.txt` with assembly options
- **Presets**: `msvc-x64-release-asm`, `gcc-x64-release-asm`
- **Pros**: Integrated with build system
- **Usage**: Enable with `-DGENERATE_ASSEMBLY=ON`

### 3. Cross-Platform Scripts
- **Windows**: `generate_assembly.bat`
- **Linux/macOS**: `generate_assembly.sh`
- **Features**: Automated organization and analysis setup

## Generated Files

### Performance-Critical Components

| File | Size | Focus Area |
|------|------|------------|
| `bitboard.asm` | 769KB | Bit manipulation, intrinsics |
| `movegen_enhanced.asm` | 1.3MB | Move generation algorithms |
| `attack_detection.asm` | 63KB | Square attack detection |
| `position.asm` | 981KB | Position management, moves |
| `evaluation.asm` | 1.8MB | Position evaluation |
| `search.asm` | 2.6MB | Alpha-beta search |
| `hybrid_evaluation.asm` | 185KB | Advanced evaluation |
| `simple_search.asm` | 1.4MB | Basic search algorithms |

## Analysis Tools

### Function Finder
```bash
# Search for specific functions across all files
.\show_asm_function.bat setBit
.\show_asm_function.bat generate_legal_moves
.\show_asm_function.bat evaluate_position
```

### IDE Integration
- **Visual Studio Code**: Install "x86 and x86_64 Assembly" extension
- **Visual Studio**: Built-in assembly syntax highlighting
- **Side-by-side viewing**: Compare .cpp and .asm files

## Performance Analysis Guide

### Key Optimization Indicators

1. **Bit Manipulation Efficiency**
   - Look for `popcnt`, `bsf`, `bsr` instructions
   - Check if intrinsics are properly utilized
   - Analyze loop unrolling in bit operations

2. **Move Generation Optimization**
   - Check for vectorization in piece move loops
   - Analyze branch prediction patterns
   - Look for inlined function calls

3. **Search Algorithm Efficiency**
   - Examine alpha-beta pruning code paths
   - Check transposition table access patterns
   - Analyze move ordering optimizations

### Common Assembly Patterns to Look For

#### Optimized Bit Operations
```assembly
popcnt  rax, rcx        ; Efficient bit counting
bsf     rax, rcx        ; Bit scan forward (find first set bit)
bsr     rax, rcx        ; Bit scan reverse (find last set bit)
```

#### Loop Optimizations
```assembly
; Unrolled loops for better performance
lea     rax, [rcx+4]    ; Efficient address calculation
mov     [rdx], eax      ; Multiple operations per iteration
mov     [rdx+4], eax
mov     [rdx+8], eax
```

#### SIMD Instructions
```assembly
movdqa  xmm0, [rcx]     ; 128-bit parallel operations
paddd   xmm0, xmm1      ; Parallel addition
```

## Integration with Development Workflow

### Performance Optimization Workflow

1. **Profile** the engine to identify bottlenecks
2. **Generate assembly** for hot functions
3. **Analyze** compiler optimizations
4. **Identify** improvement opportunities
5. **Implement** optimizations
6. **Regenerate** assembly to verify improvements
7. **Benchmark** to measure actual performance gains

### Before/After Comparisons

```bash
# Before optimization
.\generate_asm_simple.bat
copy asm_output\asm\search.asm asm_output\search_before.asm

# After optimization (make your changes)
.\generate_asm_simple.bat
fc asm_output\search_before.asm asm_output\asm\search.asm > search_diff.txt
```

## Compiler Settings

The assembly is generated with optimal performance settings:
- **Compiler**: MSVC x64 (Visual Studio 2022)
- **Optimization**: `/O2` (maximize speed)
- **Standard**: C++17
- **Defines**: Release build definitions
- **Target**: x64 architecture with full optimization

## Tips for Effective Analysis

### 1. Focus on Hot Paths
- Use profiling data to identify the most critical functions
- Prioritize analysis of functions called millions of times per second
- Look at move generation, evaluation, and search functions first

### 2. Understand Compiler Optimizations
- **Inlining**: Small functions are embedded directly
- **Loop unrolling**: Loops are optimized for better performance
- **Dead code elimination**: Unused code is removed
- **Constant folding**: Compile-time calculations

### 3. Identify Optimization Opportunities
- Manual SIMD instructions for parallel operations
- Better data structures for cache efficiency
- Algorithm improvements based on assembly patterns
- Compiler hints for better optimization

### 4. Validate Changes
- Always benchmark actual performance, not just assembly quality
- Some assembly optimizations may not translate to real-world gains
- Consider cache effects and memory access patterns

## Advanced Features

### Custom Assembly Generation
For specific files or functions, you can use the compiler directly:
```bash
cl /O2 /std:c++17 /FA /Fa"custom.asm" /c src/specific_file.cpp
```

### Debugging Information
Include debugging symbols for better analysis:
```bash
cl /O2 /std:c++17 /FA /Zi /Fa"debug.asm" /c src/file.cpp
```

### Different Optimization Levels
Compare different optimization settings:
```bash
cl /O1 /FA /Fa"file_O1.asm" /c src/file.cpp    # Optimize for size
cl /O2 /FA /Fa"file_O2.asm" /c src/file.cpp    # Optimize for speed
```

## Troubleshooting

### Common Issues
1. **Missing files**: Ensure Visual Studio x64 tools are in PATH
2. **Compilation errors**: Check that the regular build works first
3. **Large files**: Some assembly files are several MB - use appropriate editors

### Performance Impact
- Assembly generation adds significant compilation time
- Only generate when needed for optimization work
- Consider generating specific files rather than all files

## See Also
- `asm_output/README.md` - Detailed analysis guide
- `BUILD_SETUP.md` - Build environment setup
- Engine documentation for performance tuning
