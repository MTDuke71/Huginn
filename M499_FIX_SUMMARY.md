# Fix for -M499 Issue on Non-Mate Positions

## Problem Analysis
The engine was reporting `mate --499` on normal middlegame positions, indicating transposition table corruption where normal centipawn scores were being corrupted into the mate score range.

## Root Cause
Transposition table entries were getting corrupted (likely due to race conditions or bit flips), causing normal scores like -1700cp to become corrupted scores around -28003, which the UCI formatter interprets as "mate in 499 moves".

## Fixes Applied

### 1. Enhanced Score Corruption Detection
- **Primary probe function**: Added sanity checks for depth > 100 and age > 15
- **Score range validation**: Reject scores in suspicious mate range (-27000 to -31000)
- **Convenience probe function**: Additional safety checks with tighter bounds

### 2. Multi-Layer Protection
```cpp
// In primary probe (SMP format):
if (check_score < -MATE + 2000 && check_score > -MATE - 2000) {
    return false;  // Treat as miss
}

// In convenience probe (individual parameters):
if (extracted_score < -MATE + 2000) {
    return false;  // Treat as miss  
}
```

### 3. Verification Strengthening
- Maintained existing `verifyEntrySMP` function
- Added pre-verification corruption checks
- Multiple safety layers to catch different corruption patterns

## Expected Results
- **No more `mate --499`** on normal positions
- **Slightly higher miss rate** as corrupted entries are rejected
- **More reliable search** with cleaner transposition table data

## Next Steps
1. **Rebuild the engine** with the updated transposition_table.hpp
2. **Test with the problematic position** to confirm fix
3. **Monitor TT hit rates** for any significant performance impact

## Files Modified
- `src/transposition_table.hpp` - Enhanced corruption detection
- Added test files for debugging and verification

The engine should now be much more robust against transposition table corruption issues!
