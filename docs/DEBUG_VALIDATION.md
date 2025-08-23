# Debug Position Validation System

The Huginn chess engine includes a comprehensive debug validation system that can detect inconsistencies in position data structures. This is invaluable for debugging move generation, position updates, and ensuring data integrity.

## Features

The debug system validates all major position components:

- **Bitboards**: Ensures pawn bitboards match the board array
- **Piece Counts**: Verifies piece count arrays match actual pieces on board
- **Piece Lists**: Confirms piece lists contain correct square locations
- **Material Scores**: Validates material tracking matches pieces on board
- **King Squares**: Ensures king position tracking is accurate
- **Zobrist Hash**: Confirms hash matches the current position state
- **FEN Comparison**: Compares position against expected FEN string

## Usage

### Comprehensive Validation

```cpp
#include "debug.hpp"

Position pos;
pos.set_startpos();

std::string expected_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
bool is_consistent = Debug::validate_position_consistency(pos, expected_fen);

if (!is_consistent) {
    std::cout << "Position has inconsistencies!" << std::endl;
    // Detailed error messages will be printed automatically
}
```

### Individual Component Validation

```cpp
// Check specific components individually
bool bitboards_ok = Debug::validate_bitboards_consistency(pos);
bool counts_ok = Debug::validate_piece_counts_consistency(pos);
bool lists_ok = Debug::validate_piece_lists_consistency(pos);
bool material_ok = Debug::validate_material_scores_consistency(pos);
bool kings_ok = Debug::validate_king_squares_consistency(pos);
bool zobrist_ok = Debug::validate_zobrist_consistency(pos);
```

## Error Detection Examples

The system can detect various types of data corruption:

### Bitboard Inconsistencies
```
WHITE PAWNS BITBOARD MISMATCH:
  Expected: 61184
  Actual:   65280
```

### Piece Count Mismatches
```
PIECE COUNT MISMATCH for White P:
  Expected: 7
  Stored:   8
```

### Piece List Corruption
```
PIECE LIST MISMATCH for White P:
  Board squares: 31 32 33 34 36 37 38
  List squares:  31 32 33 34 35 36 37 38
```

### Material Score Errors
```
MATERIAL SCORE MISMATCH for White:
  Expected: 3900
  Stored:   4000
```

### Board vs FEN Mismatches
```
BOARD MISMATCH at e2: expected P, got .
```

## Integration with Tests

The debug validation is integrated into the test suite:

```bash
# Run all debug validation tests
./build/huginn_tests.exe --gtest_filter="*DebugValidation*"

# Run the debug demo
./build/debug_demo.exe
```

## Assertions vs Return Values

- All functions return `bool` (true = consistent, false = inconsistent)
- When inconsistencies are found, detailed error messages are printed
- Assertions are triggered for debugging builds to halt execution
- Production builds can check return values and handle errors gracefully

## Best Practices

1. **After Move Operations**: Validate position after make/unmake moves
2. **During Development**: Use comprehensive validation during debugging
3. **Performance Testing**: Use individual component checks for targeted validation
4. **Integration Tests**: Compare against known-good FEN positions

## Performance Notes

- Validation is designed for debugging, not production performance
- Individual component checks are faster than comprehensive validation
- Material score validation is very fast (O(board scan))
- Piece list validation is the most expensive (requires sorting)

## Example Integration

```cpp
void make_move_debug(Position& pos, const Move& move) {
    // Store original state for validation
    Position original = pos;
    
    // Make the move
    pos.make_move_with_undo(move);
    
    // Validate all internal consistency
    if (!Debug::validate_bitboards_consistency(pos) ||
        !Debug::validate_piece_counts_consistency(pos) ||
        !Debug::validate_material_scores_consistency(pos)) {
        std::cerr << "Move corrupted position data!" << std::endl;
        // Handle error or abort
    }
}
```

This debug system provides comprehensive position validation that can catch subtle bugs early in development and ensure data integrity throughout the engine's operation.
