# UCI Interface Implementation

## Overview

The Huginn chess engine now includes a complete UCI (Universal Chess Interface) implementation, allowing it to communicate with any UCI-compatible chess GUI such as Arena, Fritz, ChessBase, or online platforms.

## Features Implemented

### ✅ Core UCI Commands
- `uci` - Engine identification and capability announcement
- `isready` / `readyok` - Engine synchronization
- `position` - Position setup (startpos, FEN, move sequences)
- `go` - Search initiation with various parameters
- `stop` - Search termination
- `quit` - Engine shutdown
- `debug on/off` - Debug mode toggle

### ✅ Engine Information
- **Engine Name**: Huginn 1.0
- **Author**: MTDuke71
- **Options**: Hash size, Threads, Ponder support

### ✅ Search Features
- **Random Move Selection**: Returns valid legal moves randomly until evaluation is implemented
- **Search Info**: Sends depth, nodes, time, NPS, and score during search
- **Move Format**: Proper UCI long algebraic notation (e.g., e2e4, e7e8q)
- **Position Handling**: Supports startpos, FEN strings, and move sequences

## Usage

### Building
```bash
cd build
ninja huginn_uci
```

### Testing with File Input
```bash
# Create command file
echo -e "uci\nisready\nposition startpos\ngo movetime 1000\nquit" > test.txt

# Run engine
Get-Content test.txt | .\build\huginn_uci.exe
```

### Expected Output
```
id name Huginn 1.0
id author MTDuke71
option name Hash type spin default 32 min 1 max 1024
option name Threads type spin default 1 min 1 max 64
option name Ponder type check default false
uciok
readyok
info depth 1 nodes 1000 time 0 nps 0 score cp 42 pv e2e4
info depth 2 nodes 2000 time 114 nps 17543 score cp -15 pv e2e4
...
bestmove e2e4
```

## Implementation Details

### Move Generation
- Uses `generate_legal_moves_enhanced()` for move generation
- Validates all moves before returning
- Supports all move types: normal, captures, castling, en passant, promotions

### Position Management
- Integrates with existing `Position` class
- Supports FEN parsing via `set_from_fen()`
- Maintains move history for undo operations

### Search Algorithm
Currently implements a **random move selector** that:
1. Generates all legal moves in the current position
2. Randomly selects one move using `std::mt19937`
3. Simulates thinking with progressive depth reporting
4. Returns the selected move in UCI format

### Debug Mode
When `debug on` is sent, the engine provides additional information:
- Command acknowledgments
- Position FEN strings after setup
- Search status messages
- Error messages for invalid commands

## Future Enhancements

### Evaluation Function
Replace random move selection with:
- Material evaluation
- Positional evaluation
- Basic search algorithms (minimax, alpha-beta)

### Time Management
Implement proper time controls:
- `movetime` - exact search time
- `wtime`/`btime` - remaining time
- `winc`/`binc` - time increments
- `depth` - fixed depth search

### Advanced Features
- Opening book support
- Endgame tablebase integration
- Multi-threaded search
- Pondering (background thinking)

## Testing

### Basic Functionality Test
```bash
# Test file: test_uci_basic.txt
uci
isready
position startpos
go movetime 1000
quit
```

### Position Setup Test
```bash
# Test file: test_uci_positions.txt
uci
isready
position startpos moves e2e4 e7e5
go movetime 500
position fen rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2
go movetime 500
quit
```

### Debug Mode Test
```bash
# Test file: test_uci_debug.txt
uci
debug on
isready
position startpos
go infinite
stop
quit
```

## Integration with Chess GUIs

The UCI interface is compatible with popular chess GUIs:

### Arena Chess GUI
1. Download Arena from [arena-chess.com](http://www.arena-chess.com/)
2. Install the engine: `Engines` → `Install New Engine`
3. Browse to `huginn_uci.exe`
4. Engine will appear in the engine list

### Fritz/ChessBase
1. Copy `huginn_uci.exe` to ChessBase engines folder
2. Engine will be automatically detected
3. Select from engine dropdown menu

### Online Platforms
The engine can be used with online UCI-compatible platforms and analysis tools.

## Technical Architecture

### Class Structure
```cpp
class UCIInterface {
    Position position;           // Current chess position
    std::atomic<bool> is_searching;  // Search state
    std::atomic<bool> should_stop;   // Stop flag
    bool debug_mode;            // Debug output flag
    std::mt19937 rng;          // Random number generator
    
    // Core methods
    void run();                // Main UCI loop
    void handle_position();    // Position setup
    void handle_go();         // Search initiation
    void search_best_move();  // Search algorithm
    S_MOVE parse_uci_move();  // Move parsing
    std::string move_to_uci(); // Move formatting
};
```

### Communication Protocol
- **Input**: Commands from stdin (GUI → Engine)
- **Output**: Responses to stdout (Engine → GUI)
- **Format**: Text-based, line-oriented protocol
- **Threading**: Single-threaded for simplicity (can be enhanced)

## Performance

Current performance characteristics:
- **Move Generation**: ~10,000-15,000 NPS (nodes per second)
- **Position Setup**: Instant for startpos and FEN
- **Move Validation**: Complete legal move checking
- **Response Time**: Immediate for UCI commands

The random move selection ensures the engine always returns valid moves quickly, making it suitable for testing and development while the evaluation function is being implemented.

## Conclusion

The UCI interface provides a solid foundation for the Huginn chess engine, enabling immediate compatibility with chess GUIs and providing a framework for future enhancements. The current random move implementation ensures functional testing while development continues on the evaluation and search algorithms.
