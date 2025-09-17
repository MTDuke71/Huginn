# En Passant Bug Investigation Summary

## Current Status
- **Engine**: Pure Bitboard Chess Engine (Huginn)
- **Primary Issue**: En passant capture generation bugs causing systematic perft inaccuracies
- **Current State**: Multiple en passant bugs partially fixed, but introduced new over-generation issues

## Perft Accuracy Status

### Before Investigation
- Perfect accuracy through depth 5 (4,865,609 nodes)
- -5,248 nodes error at depth 6 (119,055,076 vs 119,060,324 expected)

### Current Status (After Partial Fixes)
- **REGRESSION**: Now failing at depth 4 with +246 nodes over-generation
- Previous depth 5 accuracy lost
- Indicates en passant fix introduced new bugs

## Bug Pattern Analysis

### Original Pattern
- 17 out of 19 moves in `a7a5` scenario missing exactly 1 node each
- Systematic -1 node pattern across multiple move sequences
- Strongly suggested missing en passant captures

### Test Sequences Analyzed
```
a2a3 a7a5 [various White moves] → 17/19 moves missing -1 node
- c2c3: expected 9709, got 9708 (-1)
- d2d3: expected 12688, got 12687 (-1)  
- e2e3: expected 14016, got 14015 (-1)
[etc. - 17 moves total with -1 error]
```

## Specific Bug Cases Identified

### Case 1: White Pawn En Passant (FIXED)
**Sequence**: `a2a3 a7a5 a1a2 a5a4 b2b4`
- **Missing Move**: `a4xb3` en passant capture
- **Root Cause**: File boundary logic error in White en passant generation
- **Fix Applied**: Corrected file boundary checks in `generate_white_pawn_moves_optimized()`
- **Status**: ✅ WORKING (verified with `verify_en_passant_bug.exe`)

### Case 2: Black Pawn En Passant (ATTEMPTED FIX - PROBLEMATIC)
**Sequence**: `a2a3 c7c5 d2d4`
- **Missing Move**: `c5xd3` en passant capture  
- **Root Cause**: Incorrect rank/file calculation in Black en passant generation
- **Fix Attempted**: Rewrote Black en passant logic in `generate_black_pawn_moves_optimized()`
- **Status**: ❌ OVER-GENERATING (+246 nodes at depth 4)

## Technical Implementation Details

### File Location
`src/bitboard_movegen_pure.cpp` - Lines ~220-260 (Black en passant section)

### White En Passant Logic (Working)
```cpp
// En passant target on rank 6 for white pawns (squares 40-47)
if (ep_square >= 40 && ep_square <= 47) {
    // Check diagonal captures with proper file boundary checks
    // Fixed: Corrected file boundary conditions for left/right captures
}
```

### Black En Passant Logic (Currently Broken)
```cpp
// En passant target on rank 3 for black pawns (squares 16-23)  
if (ep_square >= 16 && ep_square <= 23) {
    // ISSUE: Current implementation over-generates moves
    // Need to properly check rank 4 (squares 32-39) for Black pawns
}
```

## Key Insights from Investigation

### Rank/Square Mapping (0-indexed)
- Rank 0 (1st): squares 0-7 (a1-h1)
- Rank 1 (2nd): squares 8-15 (a2-h2) 
- Rank 2 (3rd): squares 16-23 (a3-h3)
- Rank 3 (4th): squares 24-31 (a4-h4)
- Rank 4 (5th): squares 32-39 (a5-h5)

### En Passant Rules
- **White captures**: Target on rank 6, capturing pawns on rank 5
- **Black captures**: Target on rank 3, capturing pawns on rank 4

### Critical Discovery
The `a4xb3` scenario revealed confusion about which en passant logic applies:
- Black pawn on a4 (rank 3, square 24) capturing to b3 (rank 2, square 17)
- This should be Black en passant logic, but target is rank 2, not rank 3
- Suggests fundamental misunderstanding of en passant target calculation

## Debugging Infrastructure Created

### Test Files
1. `verify_en_passant_bug.cpp` - Verifies specific a4xb3 scenario
2. `a7a5_depth4_reference_test.cpp` - Tests systematic -1 node pattern  
3. `comprehensive_perft_test.cpp` - Overall perft accuracy validation
4. `test_correct_adjacent_en_passant.cpp` - Tests Black en passant scenarios
5. `manual_square_check.cpp` - Square calculation verification utility

### Key Test Results
- `a4xb3` capture: ✅ Working (22/22 moves found)
- `c5xd3` capture: ✅ Found but causes over-generation elsewhere
- Overall perft: ❌ Regressed from perfect depth 5 to failing depth 4

## Current Problem

### The Regression
After attempting to fix Black en passant:
- Lost previous perfect accuracy through depth 5
- Now over-generating +246 nodes at depth 4
- Suggests the Black en passant fix is generating illegal moves or double-counting

### Suspected Issues
1. **Logic Conflict**: Black en passant fix may interfere with White en passant
2. **Illegal Move Generation**: May be generating en passant captures that leave king in check
3. **Double Counting**: Same moves might be generated through multiple code paths
4. **Range Error**: Incorrect rank ranges for en passant target detection

## Recommended Next Steps

### Immediate Actions
1. **Revert Black en passant changes** to restore depth 5 accuracy
2. **Isolate the problem** - test Black en passant fix in isolation
3. **Review en passant target calculation** - verify when targets are on rank 2 vs rank 3

### Systematic Approach
1. **Validate core assumptions** about en passant target squares
2. **Create minimal test cases** for each en passant scenario type
3. **Implement incremental fixes** with validation at each step
4. **Add legal move validation** to en passant generation

### Code Review Focus
- Verify rank range calculations (16-23 vs 8-15 for Black targets)
- Check for code path conflicts between White and Black en passant
- Ensure proper file adjacency checks
- Validate that generated moves pass legal move validation

## Test Data Reference

### Expected Perft Values
- Depth 4: 197,281 nodes (currently getting 197,527 = +246 error)
- Depth 5: 4,865,609 nodes (previously perfect)
- Depth 6: 119,060,324 nodes (target)

### Critical Test Positions
```
Starting: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
After a2a3 a7a5: en passant target a6
After a2a3 c7c5 d2d4: en passant target d3  
After a2a3 a7a5 a1a2 a5a4 b2b4: en passant target b3
```

## Files Modified
- `src/bitboard_movegen_pure.cpp` - Core en passant generation logic
- `CMakeLists.txt` - Added multiple debugging utilities
- Multiple test files created for validation

## Status: NEEDS EXPERT REVIEW
The investigation has identified the core issues but the attempted fix introduced regressions. A fresh perspective is needed to:
1. Properly understand en passant target square calculation
2. Implement correct Black en passant logic without breaking White en passant
3. Restore perfect perft accuracy through depth 5 and beyond