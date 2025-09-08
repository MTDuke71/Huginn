# Mate Score Bug Investigation and Next Steps

## Overview
Investigation into the mate score reporting bug revealed a partial fix and identified a deeper underlying issue requiring further investigation.

## Bug Report Summary
**Original Issue**: Engine reports positive mate scores when being mated, violating UCI specification requirement for negative mate scores when engine is losing.

**Example**: Position where Black is being mated shows `mate 1` instead of required `mate -1`.

## Investigation Results

### What Was Fixed ✅
1. **UCI Mate Score Formatting**: Enhanced `format_uci_score()` function with proper UCI compliance
   - Correctly converts plies to moves using `(mate_in_plies + 1) / 2`
   - Properly handles positive/negative mate score formatting
   - Uses correct `MATE` constant instead of hardcoded values

2. **Code Improvements**:
   - Updated function signature to include `Color side_to_move` parameter
   - Fixed mate distance calculation for both winning and losing scenarios
   - Added comprehensive comments explaining UCI specification requirements

### Root Cause Discovered ⚠️
Through systematic debugging with various test positions, identified a **fundamental search evaluation bug**:

- **Symptom**: All position evaluations return scores with incorrect signs
- **Evidence**: 
  - Black shows positive scores (+20,000 to +29,000) when losing
  - White shows negative scores (-20,000) when winning
  - Both sides consistently evaluate losing positions as winning

- **Impact**: Affects all score reporting, not just mate scores
- **Scope**: Indicates systematic error in search negation logic or evaluation function

### Test Results
```
Position: k7/pppppppp/8/8/8/8/8/7R b - - 0 1 (Black losing)
Result: score=20350 (positive, should be negative)

Position: k7/pppppppp/8/8/8/8/8/7R w - - 0 1 (White winning)  
Result: score=-20205 (negative, should be positive)
```

## Next Steps - Priority Order

### 1. Search Logic Investigation (HIGH PRIORITY)
- **File**: `src/minimal_search.cpp`
- **Focus Areas**:
  - Minimax negation in `AlphaBeta()` recursive calls
  - Score return logic in iterative deepening
  - Verify alternating min/max behavior is correct

- **Specific Lines to Investigate**:
  ```cpp
  Line 872:  int score = -AlphaBeta(pos, -INFINITE, INFINITE, depth - 1, temp_info, true, false);
  Line 1120: int score = -AlphaBeta(pos, -beta, -alpha, depth - 1, info, true, false);
  Line 1290: int score = -AlphaBeta(pos, -30000, 30000, current_depth - 1, info, true, false);
  ```

### 2. Evaluation Function Verification (MEDIUM PRIORITY)
- **File**: `src/evaluation.cpp` 
- **Check**: Ensure evaluation returns scores from correct perspective
- **Verify**: Material counting, positional evaluation signs are consistent

### 3. Position State Validation (MEDIUM PRIORITY)
- **Files**: `src/position.cpp`, `src/board.cpp`
- **Check**: Verify `side_to_move` state is correctly maintained
- **Verify**: Move making/unmaking preserves game state correctly

### 4. Test Framework Creation (LOW PRIORITY)
- Create systematic test suite for score validation
- Add known positions with expected score ranges
- Automated testing for search consistency

## Technical Details

### Current Mate Score Logic (WORKING)
```cpp
if (score > MATE - 100) {
    // Positive mate: engine winning
    int mate_in_plies = MATE - score;
    int mate_in_moves = (mate_in_plies + 1) / 2;
    return "mate " + std::to_string(mate_in_moves);
} else if (score < -MATE + 100) {
    // Negative mate: engine losing  
    int mate_in_plies = MATE + score;  // score is negative
    int mate_in_moves = (mate_in_plies + 1) / 2;
    return "mate -" + std::to_string(mate_in_moves);
}
```

### Checkmate Detection (WORKING)
```cpp
return -MATE + (info.max_depth - depth); // Correct: negative when side_to_move mated
```

## Debugging Commands Used
```bash
# Test mate positions with debug output
echo "uci\nposition fen 7k/5ppp/6K1/6Q1/8/8/8/8 b - - 0 1\ngo depth 3\nquit" > test_mate.txt
Get-Content test_mate.txt | .\huginn.exe

# Compare same position with opposite side to move
echo "uci\nposition fen 7k/5ppp/6K1/6Q1/8/8/8/8 w - - 0 1\ngo depth 3\nquit" > test_mate.txt
```

## Files Modified in This Fix
- `src/minimal_search.cpp`: Enhanced `format_uci_score()` function
- `src/minimal_search.hpp`: Updated function signature

## Commit Status
Ready for commit - mate score formatting is improved and UCI compliant, but underlying search evaluation bug requires separate investigation.
