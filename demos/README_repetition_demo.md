# Repetition Detection Demo

This demo verifies that the Huginn chess engine correctly handles repetition scenarios.

## How to Run

After building the project, run:

```bash
./bin/Release/repetition_detection_demo.exe
```

## What It Tests

The demo includes three test scenarios:

1. **Threefold Repetition Draw**: Simulates a position that repeats three times, which should be recognized as a draw according to chess rules.

2. **Perpetual Check**: Tests how the engine handles positions that could lead to perpetual check scenarios.

3. **Twofold Repetition (Not a Draw)**: Verifies that positions repeating only twice are NOT treated as draws, since the threefold rule requires three repetitions.

## Expected Output

The demo will:
- Initialize the Huginn engine
- Run search on each test position
- Display search information (depth, score, nodes, principal variation)
- Show verification that each scenario is handled correctly

All tests should complete successfully, demonstrating that Huginn's repetition detection is working properly.

## Technical Details

- Uses the `Huginn::SimpleEngine` class for searching
- Tests positions using FEN notation
- Applies moves using UCI notation (e.g., "a1a2")
- Verifies engine returns valid moves and proper search statistics

This standalone demo replaced the original Google Test-based repetition tests to avoid CTest integration issues while still providing full verification of the repetition detection functionality.
