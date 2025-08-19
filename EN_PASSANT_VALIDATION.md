# En Passant Validation Enhancement

## Overview
Enhanced the debug validation system with robust en passant validation that verifies:
1. **Rank Consistency**: En passant squares must be on the correct rank based on side to move
2. **Empty Square**: The en passant square itself must be empty
3. **Pawn Presence**: The pawn that enables en passant capture must be present

## Validation Logic

### Rank Validation
- **White to move**: En passant square must be on rank 6
  - Logic: Black just moved a pawn 2 squares (e.g., e7→e5), so ep square is e6
- **Black to move**: En passant square must be on rank 3  
  - Logic: White just moved a pawn 2 squares (e.g., d2→d4), so ep square is d3

### Pawn Location Validation
The function validates that the pawn which enables the en passant capture is present:
- **White to move**: Black pawn should be one rank behind the ep square (ep_square - 10)
- **Black to move**: White pawn should be one rank ahead of the ep square (ep_square + 10)

## Implementation

### Function: `validate_en_passant_consistency()`
Located in `src/debug.cpp`, this function:
1. Checks if en passant square is set (-1 means no en passant)
2. Validates the rank based on side to move
3. Ensures the en passant square is empty
4. Verifies the enabling pawn is present on the correct square

### Error Messages
The validation provides detailed error messages:
- **Rank mismatch**: Shows expected vs actual rank with logical explanation
- **Square not empty**: Reports what piece is incorrectly on the en passant square
- **Pawn missing**: Shows expected pawn type and location where it should be

## Testing
Comprehensive test suite in `test/test_debug_validation.cpp`:
- ✅ Valid en passant scenarios for both colors
- ✅ Invalid rank scenarios (wrong rank for side to move)
- ✅ No en passant set (should pass)
- ✅ Integration with main validation function

## Usage Example
```cpp
Position pos;
// ... set up position with en passant ...

// Validate en passant specifically
bool ep_valid = Debug::validate_en_passant_consistency(pos);

// Or use comprehensive validation
bool all_valid = Debug::validate_position_consistency(pos, expected_fen);
```

## Test Results
All tests pass ✅ (73/73 total tests)
- En passant validation correctly identifies valid and invalid scenarios
- Proper rank checking logic (corrected from initial implementation)
- Integrates seamlessly with existing debug validation system

## Key Correction Made
Initial implementation had reversed logic for pawn location checking. Fixed to:
- White to move: Look for Black pawn at `ep_square - 10` (behind ep square)
- Black to move: Look for White pawn at `ep_square + 10` (ahead of ep square)

This enhancement significantly strengthens the debug validation system's ability to detect en passant-related position corruption.
