# En Passant Bug Status - Quick Reference

## CRITICAL REGRESSION
- **Before**: Perfect perft accuracy through depth 5 (4,865,609 nodes)
- **After attempted fix**: Over-generating +246 nodes at depth 4
- **Root cause**: Black en passant fix broke something

## Working vs Broken

### ✅ WORKING: White En Passant  
- Sequence: `a2a3 a7a5 a1a2 a5a4 b2b4` → `a4xb3` capture
- Fixed in `src/bitboard_movegen_pure.cpp` around line 100-130
- Verified with `verify_en_passant_bug.exe`

### ❌ BROKEN: Black En Passant
- Sequence: `a2a3 c7c5 d2d4` → `c5xd3` capture  
- Attempted fix around line 220-260
- **OVER-GENERATING** moves, causing +246 node error

## Key Technical Issue
Black en passant logic needs to check:
- En passant target on rank 3 (squares 16-23)
- Black pawns on rank 4 (squares 32-39) 
- Adjacent files only

Current implementation generates illegal moves or double-counts.

## Immediate Need
1. **Revert** Black en passant changes to restore depth 5 accuracy
2. **Isolate** the Black en passant problem  
3. **Fix** without breaking White en passant

## Test Files Available
- `comprehensive_perft_test.exe` - Overall accuracy check
- `verify_en_passant_bug.exe` - White en passant verification
- `test_correct_adjacent_en_passant.exe` - Black en passant test
- Multiple other debugging utilities

## Target Perft Values
- Depth 4: 197,281 (currently 197,527 = +246 error)
- Depth 5: 4,865,609 (was perfect, now broken)
- Depth 6: 119,060,324 (ultimate goal)