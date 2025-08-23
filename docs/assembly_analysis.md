# Assembly Analysis for Huginn Chess Engine

Generated on: 08/23/2025 17:33:20
Compiler: g++ with -O3 -DNDEBUG optimization
Target: x64 architecture

## Generated Files

### movegen_enhanced.s
- **Source**: src/movegen_enhanced.cpp
- **Description**: Move generation with IS_PLAYABLE macro
- **Size**: 97425 bytes
- **Lines**: 5506

### attack_detection.s
- **Source**: src/attack_detection.cpp
- **Description**: Square attack detection
- **Size**: 28483 bytes
- **Lines**: 1890

### board120.s
- **Source**: src/board120.cpp
- **Description**: Board representation and lookup tables
- **Size**: 5320 bytes
- **Lines**: 435

### position.s
- **Source**: src/position.cpp
- **Description**: Position management
- **Size**: 66302 bytes
- **Lines**: 3396

### move.s
- **Source**: src/move.cpp
- **Description**: Move encoding/decoding
- **Size**: 1245 bytes
- **Lines**: 53

## Key Optimizations to Analyze

1. **IS_PLAYABLE Macro** (in movegen_enhanced.s)
   - Look for: cmpl $119, %reg (bounds check)
   - Look for: cmpb $0, (%rsi,%rbx) (lookup table access)
   - Location: Around FILE_RANK_LOOKUPS+240 references

2. **Lookup Table Usage** (in board120.s)
   - FILE_RANK_LOOKUPS table definitions
   - Static data organization

3. **Move Generation Loops**
   - Sliding piece move generation (most critical for performance)
   - Piece list iteration optimization

## Analysis Commands

`powershell
# Search for specific patterns
Select-String "FILE_RANK_LOOKUPS" *.s
Select-String "cmpl.*119" *.s  # Bounds checks
Select-String "cmpb.*0" *.s    # Playable checks

# Count function calls vs inline code
Select-String "call.*" *.s | Measure-Object  # Function calls
Select-String "cmpb.*0" *.s | Measure-Object # Direct comparisons
`
