# Huginn Chess Engine API Guide

## Recent Changes

# Huginn Chess Engine v1.1 API Guide

## Recent Changes

### 🎉 Engine3 Architecture - Huginn v1.1 (Current Version)
- **Complete Engine Rewrite** - Brand new Engine3 architecture with dramatically improved chess playing strength
- **Hybrid Evaluation System** (`Engine3_src/hybrid_evaluation.hpp/cpp`):
  - **Engine2 Integration**: Advanced bitboard evaluation techniques adapted for mailbox position representation
  - **Comprehensive Analysis**: Material, pawn structure, piece activity, king safety, and mobility evaluation
  - **Game Phase Detection**: Dynamic evaluation adjustments for opening/middlegame/endgame
  - **Piece-Square Tables**: Sophisticated positional scoring with phase interpolation
  - **Advanced Pawn Analysis**: Passed, isolated, doubled pawn detection with strategic scoring
  - **King Safety**: Pawn shield analysis and attack zone evaluation
  - **Piece Activity**: Outpost detection, bishop pairs, open files, and development bonuses
- **Single-threaded Alpha-Beta Search** (`Engine3_src/simple_search.hpp/cpp`):
  - **Alpha-Beta with Quiescence**: Robust search avoiding horizon effect with tactical extensions
  - **Iterative Deepening**: Progressive depth increase with time management
  - **Move Ordering**: Captures, killer moves, and history heuristic for improved cut-offs
  - **Principal Variation Collection**: Complete PV lines showing best tactical sequences
  - **Performance**: 70k-90k nodes per second with stable search behavior
  - **UCI Integration**: Real-time depth, nodes, time, score, and PV reporting
- **Critical Bug Fixes**:
  - **Evaluation Perspective**: Fixed major bug where positions were incorrectly assessed (was +578cp, now realistic evaluations)
  - **Move Encoding**: Proper UCI move output (d2d4, e2e4) replacing broken "0000" moves
  - **Search Stability**: Proper termination conditions and PV collection
  - **Mate Detection**: Accurate mate scoring (+/-32000 cp) and detection
- **Performance Achievements**: 
  - **Dramatic Quality Improvement**: Engine3 shows realistic position assessments vs legacy evaluation bugs
  - **Reliable Move Output**: All moves properly encoded and transmitted via UCI
  - **Competition Ready**: Full UCI protocol compliance with proper timing and node statistics

### Legacy Engine (Preserved as huginn_legacy.exe)
- **Original Implementation**: Complete legacy engine preserved for comparison and regression testing
- **Legacy Evaluation System** (`src/evaluation.hpp/cpp`): Original evaluation with known perspective bugs

### Move Generation Optimization System (Previous Version)
- **Comprehensive optimization architecture** delivering 69% overall performance improvement
- **Modular optimization system** with specialized modules for each piece type:
  - **Pawn optimizations**: Batch promotion generation (69% overall improvement)
  - **Knight optimizations**: Template-based compile-time optimization (2.1% improvement)
  - **Sliding piece optimizations**: Combined Bishop/Rook/Queen optimization (4.3% + 678% early exit)
  - **King optimizations**: Separated castling logic for improved organization
- **Performance achievements**: 34+ million moves/second, 22.5s vs 71s baseline in perft tests
- **Template-based approach**: Compile-time optimization and instruction cache efficiency
- **Dual architecture**: Optimized production functions + legacy functions for profiling/testing

### Legacy Move Structure Removal (Previous Version)
- **Complete elimination** of legacy `Move` struct and all compatibility functions
- **Pure S_MOVE architecture** - all move operations now use the high-performance S_MOVE structure
- **Simplified codebase** - no more dual interfaces or conversion functions
- **Performance improvement** - elimination of legacy adapter overhead
- **API cleaning** - `make_move_with_undo(const Move&)`, `to_s_move()`, `from_s_move()`, `generate_pseudo_legal_moves()`, `generate_legal_moves()` with MoveList, and legacy test functions removed

> **Important**: As of the latest version, all legacy Move structures and MoveList compatibility functions have been completely removed. The engine now uses only the S_MOVE and S_MOVELIST architecture for optimal performance and simplicity.

### Enhanced Legal Move Generation
- **True legal move filtering** - `generate_legal_moves()` now properly filters moves that would leave king in check
- **King safety validation** - comprehensive check detection prevents illegal king moves
- **Castling validation** - proper validation of castling legality including through-check prevention
- **Pin detection** - moves by pinned pieces are correctly handled and filtered
- **Perft validation** - extensive testing suite validates move generation correctness against standard positions

### Advanced Debugging & Validation Suite
- **Comprehensive perft testing** - supports standard chess engine validation positions
- **Performance benchmarking** - timing analysis for move generation optimization
- **Position integrity validation** - multi-layered consistency checking for all data structures
- **Error detection & reporting** - detailed diagnostics for debugging engine development

---

## init.hpp — Engine Initialization

- **Initialization Functions:**
  - `Huginn::init()` — Initialize all engine subsystems (call once at startup)
  - `Huginn::is_initialized()` — Check if engine has been properly initialized
- **Usage:**
  ```cpp
  #include "init.hpp"
  
  int main() {
      Huginn::init();  // Initialize all subsystems
      // ... use engine functionality
  }
  ```
- **Subsystems Initialized:**
  - Zobrist hashing tables for position identification
  - (Future: Attack tables, evaluation tables, opening books, etc.)

---

## bitboard.hpp — Bitboard API

- **Type Definition:**
  - `Bitboard` — alias for `uint64_t`, represents 64 squares using bits
- **Bit Manipulation Macros (optimized with pre-computed masks):**
  - `setBit(bb, sq)` / `addBit(bb, sq)` / `SETBIT(bb, sq)` — Set bit at square position
  - `popBit(bb, sq)` / `PopBit(bb, sq)` / `CLRBIT(bb, sq)` — Clear bit at square position
  - `getBit(bb, sq)` — Check if bit is set at square position
- **Utility Macros:**
  - `POP(bb)` — Pop and return least significant bit (modifies bitboard)
  - `CNT(bb)` — Count number of set bits (non-destructive)
- **Constants:**
  - `EMPTY_BB` (0), `FULL_BB` (all bits set)
  - `BIT_MASK[64]` — Pre-computed bit masks for each square (performance optimization)
  - `CLEAR_MASK[64]` — Pre-computed inverted masks for clearing bits (performance optimization)
  - File bitboards: `FILE_A` through `FILE_H`, `FILE_BB[8]` array
  - Rank bitboards: `RANK_1` through `RANK_8`, `RANK_BB[8]` array
- **Visualization:**
  - `printBitboard(Bitboard bb)` — Print with 'x' occupied, '-' empty
  - `printBitboard(Bitboard bb, char occupied, char empty)` — Custom characters
  - Output format: Rank 8 at top, Rank 1 at bottom, with file/rank borders
- **Utility Functions:**
  - `popcount(Bitboard bb)` — Count number of set bits (optimized with GCC builtin)
  - `countBit(Bitboard bb)` — Count number of set bits - alias for popcount (optimized with GCC builtin)
  - `get_lsb(Bitboard bb)` — Get least significant bit position (optimized with GCC builtin)
  - `pop_lsb(Bitboard& bb)` — Pop and return least significant bit (optimized with GCC builtin)
  - `is_empty(Bitboard bb)` — Check if bitboard is empty
  - `is_set(Bitboard bb, int square)` — Check if specific square is set
- **Square Conversion:**
  - `square_from_file_rank(int file, int rank)` — Convert file/rank to square (0-63)
  - `file_of_square(int square)` / `rank_of_square(int square)` — Extract file/rank
  - `sq64_to_sq120(int sq64)` / `sq120_to_sq64(int sq120)` — Convert between indexing systems (uses pre-computed MAILBOX_MAPS arrays)
  - `SQ120(sq64)` / `SQ64(sq120)` — Convenient macros for square conversion (direct MAILBOX_MAPS access)
- **Usage Examples:**
  ```cpp
  Bitboard bb = EMPTY_BB;
  setBit(bb, 28);           // Set e4
  if (getBit(bb, 28)) {     // Check e4
      printBitboard(bb);    // Visualize board
  }
  
  // Alternative syntax options
  SETBIT(bb, 35);           // Set d5 using SETBIT alias
  CLRBIT(bb, 28);           // Clear e4 using CLRBIT alias
  
  // Using utility macros
  Bitboard pieces = 0x0F00ULL;  // Some pieces on rank 2
  int count = CNT(pieces);      // Count bits (4)
  while (!is_empty(pieces)) {
      int square = POP(pieces); // Pop LSB and get its position
      // Process piece at 'square'...
  }
  
  // Square conversion macros (fast direct access)
  int sq64 = 28;                // e4 in 64-square indexing
  int sq120 = SQ120(sq64);      // Convert to 120-square (55)
  int back = SQ64(sq120);       // Convert back to 64-square (28)
  ```

---

## board120.hpp — Mailbox-120 Board API

- **Enums:**  
  - `File` (A–H), `Rank` (R1–R8), `Square` (Offboard)
- **Square Indexing:**  
  - `sq(File, Rank)` → int (21..98 for playable squares)
  - `file_of(int sq120)` / `rank_of(int sq120)` → File/Rank
- **Board Layout:**  
  - 10x12 grid, playable squares: (sq % 10) in [1..8], (sq / 10) in [2..9]
- **Directions:**  
  - `NORTH`, `SOUTH`, `EAST`, `WEST`, `NE`, `NW`, `SE`, `SW`
  - `KNIGHT_DELTAS[8]`, `KING_DELTAS[8]`
- **Helpers:**  
  - `is_playable(int sq120)` / `is_offboard(int sq120)`
  - `from_algebraic(std::string_view)` / `to_algebraic(int sq120, char out[3])`
- **Pawn Directions:**  
  - `pawn_push_dir_white()`, `pawn_push_dir_black()`, etc.
- **64↔120 Mapping:**  
  - `MAILBOX_MAPS.to120[64]`, `MAILBOX_MAPS.to64[120]`

---

## chess_types.hpp — Piece & Color API

- **Enums:**  
  - `Color` (White, Black, None)
  - `PieceType` (None, Pawn, Knight, Bishop, Rook, Queen, King)
  - `Piece` (WhitePawn, BlackPawn, etc.)
- **Piece List Constants:**
  - `MAX_PIECES_PER_TYPE 10` — Maximum pieces per type per side
  - `MAX_PIECE_TYPES` — Number of piece types
  - `PieceList` — Type alias for 2D piece location array
- **Castling Rights:**
  - `CASTLE_NONE`, `CASTLE_WK`, `CASTLE_WQ`, `CASTLE_BK`, `CASTLE_BQ`, `CASTLE_ALL`
  - Legacy aliases: `WKCA`, `WQCA`, `BKCA`, `BQCA`
  - Utility functions: `can_castle_kingside()`, `can_castle_queenside()`, `can_castle()`, `remove_castling_rights()`
- **Helpers:**  
  - `operator!(Color)` — flip color
  - `is_none(Piece)`, `type_of(Piece)`, `color_of(Piece)`
  - `make_piece(Color, PieceType)`
- **Piece Properties:**  
  - `is_slider(PieceType)`, `is_minor(PieceType)`, `is_major(PieceType)`
- **Char Conversion:**  
  - `to_char(Piece)` — FEN char
  - `from_char(char)` — Piece from FEN char
- **Values:**  
  - `value_of(Piece)` — material value
- **Iteration:**  
  - `PieceTypeIter` — for (auto t : PieceTypeIter()) {...}

---

## main.cpp — Example Usage

- **Board Setup:**  
  - `Position pos; pos.set_startpos();`
  - Modern `Position` struct with `std::array<Piece, 120> board`
- **Square Operations:**  
  - Use `sq(File, Rank)`, algebraic conversion, FEN char printing
- **Iteration:**  
  - Iterate playable squares: `for (int sq = 21; sq <= 98; ++sq) if (is_playable(sq))`
  - Iterate pieces by type: `for (int i = 0; i < pos.pCount[color][type]; ++i)`
- **Mapping:**  
  - 64↔120 translation: `MAILBOX_MAPS.to120`, `MAILBOX_MAPS.to64`
  - Bitboard conversion: `sq64_to_sq120()`, `sq120_to_sq64()`
- **Move Generation:**  
  - Knight moves: `for (int d : KNIGHT_DELTAS)`
  - Sliding moves: `for (int to = from + NORTH; is_playable(to); to += NORTH)`
- **Bitboard Integration:**
  ```cpp
  // Convert position to bitboard representation
  Bitboard white_pawns = EMPTY_BB;
  for (int i = 0; i < pos.pCount[0][int(PieceType::Pawn)]; ++i) {
      int sq120 = pos.pList[0][int(PieceType::Pawn)][i];
      int sq64 = sq120_to_sq64(sq120);
      if (sq64 >= 0) setBit(white_pawns, sq64);
  }
  printBitboard(white_pawns);  // Visualize pawn positions
  ```

---

## attack_detection.hpp — Attack Detection API

- **Core Function:**
  - `SqAttacked(int sq, const Position& pos, Color attacking_color)` — Check if square is under attack by specified color
- **Attack Detection Features:**
  - **Pawn Attacks**: Diagonal captures for both White (SE/SW from target) and Black (NE/NW from target)
  - **Knight Attacks**: All 8 L-shaped knight moves from attacking pieces
  - **King Attacks**: All 8 adjacent squares from attacking kings
  - **Sliding Piece Attacks**: 
    - **Rook/Queen**: Ranks and files until blocked by any piece
    - **Bishop/Queen**: Diagonals until blocked by any piece
  - **Blocking Detection**: Properly stops sliding attacks when pieces intervene
  - **Color Differentiation**: Only considers pieces of the specified attacking color
  - **Boundary Checking**: Safely handles offboard squares and edge cases
- **Performance Optimizations:**
  - **Piece List Optimization**: Uses `pCount[color][type]` and `pList[color][type][index]` for 5-20x speedup
  - **Smart Fallback**: Automatically detects inconsistent piece lists and falls back to board scanning
  - **Ultra-Fast Execution**: 3.9-8.6 ns/call depending on position complexity
  - **Endgame Acceleration**: Performance improves as pieces are captured (fewer pieces = faster checks)
  - **Optimized Helper Functions**: Dedicated functions for each piece type's attack patterns
- **Helper Functions:**
  - `pawn_attacks_square(int pawn_sq, int target_sq, Color pawn_color)` — Check pawn diagonal attacks
  - `knight_attacks_square(int knight_sq, int target_sq)` — Check knight L-shaped attacks  
  - `king_attacks_square(int king_sq, int target_sq)` — Check king adjacent attacks
  - `sliding_attacks_rank_file(int piece_sq, int target_sq, const Position& pos)` — Check rook/queen rank/file attacks
  - `sliding_attacks_diagonal(int piece_sq, int target_sq, const Position& pos)` — Check bishop/queen diagonal attacks
- **Usage Examples:**
  ```cpp
  Position pos;
  pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  
  // Check if e4 is attacked by Black pieces
  bool attacked = SqAttacked(sq(File::E, Rank::R4), pos, Color::Black);
  
  // Check if king is in check
  int king_square = pos.king_sq[int(Color::White)];
  bool in_check = SqAttacked(king_square, pos, Color::Black);
  
  // Generate attack map for position analysis (optimized for performance)
  for (int rank = 0; rank < 8; ++rank) {
      for (int file = 0; file < 8; ++file) {
          int square = sq(static_cast<File>(file), static_cast<Rank>(rank));
          bool white_attacks = SqAttacked(square, pos, Color::White);
          bool black_attacks = SqAttacked(square, pos, Color::Black);
          // Process attack information...
      }
  }
  
  // Works with both FEN-based positions (optimized) and manual positions (fallback)
  Position manual_pos;
  manual_pos.reset();
  manual_pos.set(sq(File::E, Rank::R4), Piece::WhiteQueen);  // Manual piece placement
  bool attacks_d5 = SqAttacked(sq(File::D, Rank::R5), manual_pos, Color::White);  // Uses fallback
  
  // Check if position is legal (king not in check for side not to move)
  bool position_legal = !SqAttacked(pos.king_sq[1 - int(pos.side_to_move)], pos, pos.side_to_move);
  ```

---

## move.hpp — Enhanced Move Representation API

> **Note**: As of the latest version, all legacy Move structures and compatibility functions have been completely removed. The engine now uses only the S_MOVE architecture for optimal performance and simplicity.

### **Key Improvements in S_MOVE Architecture**

The `S_MOVE` structure represents a modern, high-performance move representation that has completely replaced the legacy Move system:

- **Memory Efficiency**: 8 bytes total vs 12+ bytes for separate fields (33% reduction)
- **Cache Performance**: Compact 25-bit encoding improves memory bandwidth utilization
- **Integrated Scoring**: Built-in move ordering without separate data structures
- **Bit-Packed Design**: All move information encoded in a single 32-bit integer
- **Fast Operations**: Single integer comparisons and efficient bit manipulation
- **Pure Architecture**: No legacy compatibility layer - S_MOVE is the only move representation

### **S_MOVE Structure - High-Performance Packed Move Representation**

- **Primary Structure:**
  ```cpp
  struct S_MOVE {
      int move;   // Packed move data (25 bits used)
      int score;  // Move score for ordering/evaluation
  };
  ```

### **Bit Layout Diagram (25 bits total in `int move`):**

```
Bit Position:  31    24 23  20 19 18 17    14 13    7 6     0
               │      │  │   │  │  │  │      │  │     │       │
Field:         │unused│promoted│C│P│E│captured│ to   │ from  │
               │      │  piece │A│S│P│ piece  │square│square │
               │      │ (4bits)│S│T│ │(4bits) │(7bit)│(7bit) │
               │      │        │T│A│ │        │      │       │
               │      │        │L│R│ │        │      │       │
               │      │        │E│T│ │        │      │       │

Bit Usage:     7 bits │ 4 bits│1│1│1│4 bits │7 bits│7 bits │
               unused │       │ │ │ │        │      │       │
                     │       │ │ │ │        │      │       │
Legend:              │       │ │ │ └─En Passant flag       │
                     │       │ │ └───Pawn Start flag       │
                     │       │ └─────Castle flag           │
                     │       └───────Promoted piece type   │
                     └───────────────Captured piece type   │
                                                           │
Total: 25 bits used, 7 bits available for future extensions
```

### **Bit Field Details:**
- **Bits 0-6 (7 bits):** `from` square (0-127, supports 120-square notation)
- **Bits 7-13 (7 bits):** `to` square (0-127, supports 120-square notation)  
- **Bits 14-17 (4 bits):** `captured` piece type (PieceType enum: 0-15)
- **Bit 18 (1 bit):** `en_passant` capture flag
- **Bit 19 (1 bit):** `pawn_start` double-push flag
- **Bits 20-23 (4 bits):** `promoted` piece type (PieceType enum: 0-15)
- **Bit 24 (1 bit):** `castle` move flag
- **Bits 25-31 (7 bits):** Available for future extensions

### **Construction & Encoding:**
- **Constructor:**
  ```cpp
  S_MOVE(int from, int to, PieceType captured = PieceType::None, 
         bool en_passant = false, bool pawn_start = false, 
         PieceType promoted = PieceType::None, bool castle = false)
  ```
- **Static Encoding:**
  ```cpp
  static int encode_move(int from, int to, PieceType captured, 
                        bool en_passant, bool pawn_start, 
                        PieceType promoted, bool castle)
  ```

### **Decoding & Access Methods:**
- **Square Access:**
  - `int get_from() const` — Extract from square (bits 0-6)
  - `int get_to() const` — Extract to square (bits 7-13)
- **Piece Information:**
  - `PieceType get_captured() const` — Extract captured piece type (bits 14-17)
  - `PieceType get_promoted() const` — Extract promoted piece type (bits 20-23)
- **Move Flags:**
  - `bool is_en_passant() const` — Check en passant flag (bit 18)
  - `bool is_pawn_start() const` — Check pawn double-push flag (bit 19)
  - `bool is_castle() const` — Check castle flag (bit 24)

### **Convenience Query Methods:**
- **Move Classification:**
  - `bool is_capture() const` — True if captured piece or en passant
  - `bool is_promotion() const` — True if promoted piece set
  - `bool is_quiet() const` — True if no capture, promotion, castle, or en passant
- **Move Ordering:**
  - `bool operator<(const S_MOVE&) const` — Compare by score (ascending)
  - `bool operator>(const S_MOVE&) const` — Compare by score (descending)
  - `bool operator==(const S_MOVE&) const` — Compare move data only

### **Convenience Factory Functions:**
```cpp
S_MOVE make_move(int from, int to)                              // Simple move
S_MOVE make_capture(int from, int to, PieceType captured)       // Capture move
S_MOVE make_en_passant(int from, int to)                        // En passant capture
S_MOVE make_pawn_start(int from, int to)                        // Pawn double-push
S_MOVE make_promotion(int from, int to, PieceType promoted,     // Promotion move
                     PieceType captured = PieceType::None)
S_MOVE make_castle(int from, int to)                            // Castle move
```

### **Performance Benefits:**
- **Memory Efficiency:** 8 bytes total (4 bytes move + 4 bytes score) vs 12+ bytes for separate fields
- **Cache Performance:** Compact representation reduces memory bandwidth
- **Fast Operations:** Single integer comparisons and bit manipulation
- **Integrated Scoring:** Built-in move ordering without separate data structures
- **Pure Architecture:** No legacy compatibility overhead or conversion costs

### **Usage Examples:**
```cpp
// Create different types of moves
S_MOVE quiet = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
S_MOVE capture = make_capture(sq(File::D, Rank::R4), sq(File::E, Rank::R5), PieceType::Pawn);
S_MOVE promotion = make_promotion(sq(File::A, Rank::R7), sq(File::A, Rank::R8), PieceType::Queen);
S_MOVE castle = make_castle(sq(File::E, Rank::R1), sq(File::G, Rank::R1));

// Query move properties
if (capture.is_capture()) {
    PieceType captured = capture.get_captured();
    // Handle capture logic...
}

// Move ordering by score
std::vector<S_MOVE> moves = {...};
std::sort(moves.begin(), moves.end(), [](const S_MOVE& a, const S_MOVE& b) {
    return a.score > b.score;  // Higher scores first
});

// Access move information
int from_square = move.get_from();
int to_square = move.get_to();
bool is_promotion = move.is_promotion();
```

---

## movegen_enhanced.hpp — Unified Move Generation API

**Note**: The enhanced move generation system uses the high-performance S_MOVELIST container for optimal performance and cache efficiency.

- **Primary Interface - S_MOVELIST:**
  ```cpp
  S_MOVELIST moves;
  generate_all_moves(pos, moves);              // Fast pseudo-legal generation
  generate_legal_moves_enhanced(pos, moves);   // Legal move generation
  ```

- **S_MOVELIST Structure:**
  ```cpp
  struct S_MOVELIST {
      S_MOVE moves[MAX_POSITION_MOVES];  // 256 move capacity
      int count;
  };
  ```
- **S_MOVELIST Methods:**
  - `clear()` — Clear all moves from list
  - `add_quiet_move(const S_MOVE& move)` — Add quiet move (score: 0)
  - `add_capture_move(const S_MOVE& move, const Position& pos)` — Add capture with MVV-LVA scoring
  - `add_en_passant_move(const S_MOVE& move)` — Add en passant capture (score: 1,000,105)
  - `add_promotion_move(const S_MOVE& move)` — Add promotion (score: 2,000,000+)
  - `add_castle_move(const S_MOVE& move)` — Add castling move (score: 50,000)
  - `size() const` — Get number of moves in list
  - `operator[](size_t i)` — Access move by index
  - `sort_by_score()` — Sort moves by score (highest scores first) for move ordering
- **Core Functions:**
  - `generate_all_moves(const Position&, S_MOVELIST&)` — **Optimized** pseudo-legal move generation using advanced optimization modules
  - `generate_legal_moves_enhanced(const Position&, S_MOVELIST&)` — Generate legal moves (filters out moves that leave king in check)

## Move Generation Optimization System

The Huginn engine features a comprehensive **optimization architecture** that delivers significant performance improvements over standard move generation:

### **Optimization Modules (Production System):**
- **`PawnOptimizations::generate_pawn_moves_optimized()`** — 69% overall performance improvement through batch promotion generation and pre-computed square bounds
- **`KnightOptimizations::generate_knight_moves_template()`** — Template-based compile-time optimization with direction unrolling (2.1% improvement)
- **`SlidingPieceOptimizations::generate_all_sliding_moves_optimized()`** — Combined Bishop/Rook/Queen optimization targeting 45%+ of generation time (4.3% improvement + 678% early exit optimization)
- **`KingOptimizations::generate_king_moves_optimized()`** — Separated castling logic with template-based early returns for improved code organization

### **Performance Achievements:**
- **Overall System Performance:** 34+ million moves per second
- **Optimization Impact:** ~69% improvement from original baseline (22.5s vs 71s in perft tests)
- **Early Exit Optimization:** 678% improvement for positions with no sliding pieces
- **Cache Efficiency:** Template-based approach optimizes instruction cache utilization

### **Legacy Functions (Profiling & Testing Only):**
> **Important:** The following individual functions are **NOT used in production** but retained for development infrastructure:
- `generate_pawn_moves()`, `generate_knight_moves()`, `generate_bishop_moves()`, `generate_rook_moves()`, `generate_queen_moves()`, `generate_king_moves()`
- **Purpose:** Profiling system (`movegen_profiler.cpp`), test validation, and development analysis
- **Production Path:** All optimized functions called through `generate_all_moves()`

### **Optimization Headers:**
```cpp
#include "pawn_optimizations.hpp"       // Batch promotion optimization
#include "knight_optimizations.hpp"     // Template-based knight moves  
#include "sliding_piece_optimizations.hpp" // Combined sliding piece optimization
#include "king_optimizations.hpp"       // Castling separation optimization
```

### **Optimization Usage Examples:**
```cpp
// Standard optimized move generation (recommended for production)
Position pos;
pos.set_startpos();

S_MOVELIST moves;
generate_all_moves(pos, moves);  // Uses all optimization modules automatically
std::cout << "Generated " << moves.size() << " moves optimally" << std::endl;

// Individual optimization modules (for development/testing only)
S_MOVELIST pawn_moves, knight_moves, sliding_moves, king_moves;

// Pawn optimization - 69% performance improvement
PawnOptimizations::generate_pawn_moves_optimized(pos, pawn_moves, pos.side_to_move);

// Knight optimization - Template-based with 2.1% improvement  
KnightOptimizations::generate_knight_moves_template(pos, knight_moves, pos.side_to_move);

// Sliding piece optimization - 4.3% improvement + 678% early exit
SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, sliding_moves, pos.side_to_move);

// King optimization - Separated castling logic
KingOptimizations::generate_king_moves_optimized(pos, king_moves, pos.side_to_move);

// Performance measurement example
auto start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < 1000000; ++i) {
    S_MOVELIST test_moves;
    generate_all_moves(pos, test_moves);
}
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Generated moves at " << (1000000.0 / duration.count() * 1000000) 
          << " moves/second" << std::endl;
```

### **Integration & Best Practices:**
- **Production Usage:** Always use `generate_all_moves()` for optimal performance - it automatically applies all optimizations
- **Development Testing:** Individual optimization modules are available for isolated testing and profiling
- **Performance Monitoring:** The system achieves 34+ million moves/second with comprehensive optimizations
- **Memory Efficiency:** S_MOVELIST provides cache-friendly move storage with minimal overhead
- **Compatibility:** Full backward compatibility with existing Position and S_MOVE structures
- **Future-Proof:** Modular optimization architecture allows easy addition of new optimization techniques

### **Performance Characteristics:**
- **Pawn Move Generation:** 69% faster than baseline through batch promotion and bounds optimization
- **Sliding Pieces:** 4.3% improvement in normal cases, 678% improvement with early exit optimization
- **Knight Moves:** 2.1% improvement through template-based compile-time optimization
- **Overall System:** Consistent 69% improvement across diverse chess positions
- **Cache Efficiency:** Template specialization improves instruction cache utilization
- **Scalability:** Performance improvements maintained across varying position complexities

### **Testing & Validation Infrastructure:**
- **Comprehensive Test Suite:** 42+ specialized tests validate optimization correctness
- **Perft Validation:** All optimizations validated against standard perft test positions (e.g., Kiwipete)
- **Performance Tracking:** Automated performance measurement with `perft/perf_test.ps1`
- **Regression Testing:** Move count and hash validation ensure optimization accuracy
- **Profiling Support:** Individual optimization modules enable precise performance measurement
- **Development Tools:** Assembly analysis tools (`tools/generate_asm.ps1`) for low-level optimization verification

### **File Structure:**
```
src/
├── pawn_optimizations.hpp/cpp        # 69% pawn move optimization
├── knight_optimizations.hpp/cpp      # Template-based knight optimization  
├── sliding_piece_optimizations.hpp/cpp # Sliding piece + early exit optimization
├── king_optimizations.hpp/cpp        # King + castling optimization
├── movegen_enhanced.hpp/cpp          # Main optimized interface
└── movegen_profiler.cpp              # Performance measurement tools

test/
├── test_optimization_*.cpp           # Individual optimization tests
├── test_comprehensive_optimization.cpp # Complete system validation
└── perft/                           # Performance testing infrastructure
```
- **Helper Functions:**
  - `in_check(const Position& pos)` — Check if current side to move is in check
  - `is_legal_move(const Position& pos, const S_MOVE& move)` — Test if a specific move is legal
- **Legal Move Generation Details:**
  - **Pseudo-Legal vs Legal**: Pseudo-legal moves include all moves that follow piece movement rules but may leave king in check
  - **King Safety Filtering**: Legal move generation tests each pseudo-legal move to ensure king safety
  - **Performance Considerations**: Use pseudo-legal for move ordering, legal for actual play
  - **Check Detection Integration**: Uses `SqAttacked()` function for efficient king safety validation
- **Integration:**
  - Seamlessly works with S_MOVE structure and scoring
  - Supports efficient move ordering with built-in sort functionality
  - Optimized for performance-critical move generation scenarios
  - Legal move generation suitable for game play and search algorithms
- **Usage Examples:**
  ```cpp
  Position pos;
  pos.set_startpos();
  
  // Generate all legal moves for current position
  S_MOVELIST legal_moves;
  generate_legal_moves_enhanced(pos, legal_moves);
  
  // Generate pseudo-legal moves for move ordering (faster)
  S_MOVELIST pseudo_moves;
  generate_all_moves(pos, pseudo_moves);
  
  // Check if position has any legal moves (detect checkmate/stalemate)
  bool has_moves = legal_moves.size() > 0;
  bool in_check = SqAttacked(pos.king_sq[int(pos.side_to_move)], pos, !pos.side_to_move);
  if (!has_moves) {
      if (in_check) {
          std::cout << "Checkmate!" << std::endl;
      } else {
          std::cout << "Stalemate!" << std::endl;
      }
  }
  
  // Sort moves by score for search optimization
  legal_moves.sort_by_score();
  ```

---

## zobrist.hpp — Position Hashing API

- **Namespace:** `Zobrist`
- **Hash Tables (optimized for sq120 indexing):**
  - `Piece[PIECE_NB][120]` — piece-square hash values (direct sq120 indexing, no conversion needed)
  - `Side` — side to move hash
  - `Castle[16]` — castling rights hash (0..15)
  - `EpFile[8]` — en passant file hash (a..h)
- **Functions:**
  - `init_zobrist(seed)` — initialize hash tables with SplitMix64 generator (faster than Mersenne Twister)
  - `compute(const Position&)` — compute Zobrist hash for current position (optimized, no sq120→sq64 conversion)
- **Integration:** 
  - Works with modern `Position` struct
  - Automatically handles piece placement, side to move, castling rights, en passant
  - Performance optimized: direct sq120 indexing eliminates array lookup conversions
  - Random number generation: SplitMix64 for superior speed and quality

---

## position.hpp — Position & State API

- **Constants:**
  - `MAX_POSITION_MOVES 256` — Maximum moves per position for S_MOVELIST capacity
- **Structs:**
  - `State { ep_square, castling_rights, halfmove_clock, captured }`
  - `S_UNDO { S_MOVE move, castling_rights, ep_square, halfmove_clock, zobrist_key, captured, king_sq_backup[2], pawns_bb_backup[2], piece_counts_backup[7], material_score_backup[2] }` — Complete undo state with incremental update support using S_MOVE structure
  - `Position { board[120], side_to_move, ep_square, castling_rights, halfmove_clock, fullmove_number, king_sq[2], pawns_bb[2], piece_counts[7], zobrist_key, pList[2], pCount[2], move_history (dynamic vector), ply }`
- **Position Management:**
  - `reset()` — Complete reset to empty state (all squares offboard/empty, all counters cleared)
  - `set_startpos()` — Set up standard chess starting position using FEN parsing
  - `set_from_fen(const std::string& fen)` — Parse FEN string and set position accordingly
  - `to_fen() const` — Generate FEN string from current position (perfect round-trip with set_from_fen)
  - `rebuild_counts()` — Recalculate all piece counts from current board state (used for FEN parsing and setup only)
- **High-Performance Incremental Updates:**
  - `save_derived_state(S_UNDO& undo)` — Save current derived state for O(1) restoration (internal function)
  - `restore_derived_state(const S_UNDO& undo)` — Restore derived state from backup in O(1) time (internal function)
  - `update_derived_state_for_move(const S_MOVE& m, Piece moving, Piece captured)` — Update derived state incrementally in O(1) time (internal function)
  - **Performance**: 24-40x faster than `rebuild_counts()` for make/unmake operations
- **Material Score Tracking:**
  - `material_score[2]` — Cached material values for both colors for O(1) evaluation (excludes kings)
  - `get_material_score(Color c)` — Get material score for specific color
  - `get_material_balance()` — Get material advantage (White - Black)
  - `get_total_material()` — Get total material on board (White + Black)
  - **King Exclusion**: Kings excluded from material calculations since they must always exist
  - **Incremental Updates**: Material scores updated automatically during make/unmake operations
  - **Performance**: O(1) material evaluation vs O(120) piece scanning
- **FEN Support:**
  - **Full FEN parsing**: Handles piece placement, side to move, castling rights, en passant, move counters
  - **FEN generation**: `to_fen()` method converts position back to standard FEN string
  - **Round-trip compatibility**: Perfect FEN → Position → FEN preservation for all components
  - **Error handling**: Returns `false` for invalid FEN strings, maintains position state on failure
  - **Standard compliance**: Supports all standard FEN notation including KQkq castling and algebraic en passant squares
- **Pawn Bitboard System:**
  - `pawns_bb[2]` — Individual pawn bitboards for White and Black
  - `all_pawns_bb` — Combined bitboard of all pawns (White | Black)
  - `get_white_pawns()`, `get_black_pawns()`, `get_all_pawns_bitboard()` — Accessor methods
  - **Incremental Updates**: Pawn bitboards automatically maintained during moves, captures, and promotions
  - **Performance**: Fast pawn-specific operations without board scanning
- **Piece List Optimization:**
  - `pList[color][piece_type][index] = square` — Track piece locations for fast iteration
  - `pCount[color][piece_type]` — Count of pieces per type per color
  - `add_piece_to_list()`, `remove_piece_from_list()`, `move_piece_in_list()` — Piece list management
- **Methods:**
  - `reset()`, `set_startpos()`, `at(int s)`, `set(int s, Piece p)`, `rebuild_counts()` (for setup only)
  - `make_move_with_undo(const S_MOVE& m)` — Make move with full undo support using incremental updates (O(1) performance, 24-40x faster than rebuild_counts)
  - `make_move_with_undo(const S_MOVE& m, S_UNDO& undo)` — Make move with explicit undo object for advanced use cases
  - `undo_move()` — Undo last move with perfect state restoration (O(1) performance, 24-40x faster than rebuild_counts)
- **Legal Move Validation:**
  - **King Safety**: All move validation ensures king is not left in check after move
  - **Castling Rules**: Proper validation of castling legality including path clearance and through-check prevention
  - **En Passant**: Correct en passant capture validation with king safety checks
  - **Pin Handling**: Moves by pinned pieces are properly validated to prevent exposing king to check
- **Move Encoding (integrated in S_MOVE structure):**
  - `S_MOVE::encode_move(from, to, captured, en_passant, pawn_start, promoted, castle)` — Pack move into integer with all flags
  - `S_MOVE::decode_move(encoded, from, to, promo)` — Unpack basic move information from integer
- **Performance Optimizations:**
  - **Incremental Updates:** Make/unmake moves use O(1) incremental updates instead of O(120) board scanning
  - **State Backup/Restore:** Perfect derived state restoration in O(1) time using saved backup data
  - **Search Performance:** 24-40x faster move making/unmaking enables deep search algorithms
  - **Memory Efficiency:** Dynamic vector storage with automatic resizing and minimal memory footprint
  - **Direct Access:** Vector indexing for maximum speed with automatic bounds management
  - **Selective Rebuilding:** `rebuild_counts()` only used for setup operations (FEN parsing), not during search

---

## board.hpp — Board Management API

- **Reset Function:**
  - `reset_board(Position& pos)` — Wrapper function calling `pos.reset()` for backward compatibility
- **Position Display:**
  - `print_position(const Position& pos)` — Visual ASCII board display with game state information
    - Shows 8x8 board with piece placement
    - Displays side to move, castling rights, en passant square
    - Shows halfmove clock and fullmove number
    - Lists piece counts by type
    - Shows material scores per color and material balance
    - Shows Zobrist position key in hexadecimal format
- **Reset Operations (via Position::reset()):**
  - **Board State:** Offboard squares set to `Piece::Offboard`, playable squares set to `Piece::None` (empty)
  - **Piece Counts:** All piece type counters cleared to 0
  - **Bitboards:** Pawn bitboards cleared to `0ULL` for both colors
  - **Piece Lists:** All piece counts and locations cleared (`pCount[color][type] = 0`, `pList` entries set to `-1`)
  - **King Squares:** Both king positions set to `-1` (no king)
  - **Game State:** Side to move set to `Color::None`, en passant square to `-1`
  - **Clock Variables:** Halfmove clock set to 0, fullmove number reset to 1, ply reset to 0
  - **Castling:** All castling rights cleared (`castling_rights = 0`)
  - **Zobrist:** Position key cleared (`zobrist_key = 0ULL`)
  - **Move History:** Complete move history array cleared (all `S_UNDO` entries reset)
- **Position Visualization Features:**
  - **ASCII Board Display:** Clean grid layout with rank/file labels and borders
  - **Piece Representation:** FEN notation (K/Q/R/B/N/P for white, k/q/r/b/n/p for black)
  - **Empty Squares:** Shown as dots (.) for clear visualization
  - **Game State Info:** Side to move, castling rights, en passant square, move counters
  - **Piece Statistics:** Complete piece counts by type for debugging
  - **Standard Format:** Rank 8 at top, Rank 1 at bottom, files a-h left to right
- **Move Generation Benefits:**
  - Offboard squares contain `Piece::Offboard` (value 255) for instant detection
  - Empty playable squares contain `Piece::None` (value 0)
  - Move generation can check `if (is_offboard(pos.at(square)))` to detect board boundaries
  - No need for separate bounds checking in sliding piece move generation
- **Usage Example:**
  ```cpp
  Position pos;
  pos.set_startpos();              // Set up starting position
  
  // Display the position visually
  print_position(pos);             // Shows ASCII board with game state
  
  // Move generation example using offboard detection
  int from = sq(File::A, Rank::R1);
  for (int to = from + WEST; !is_offboard(pos.at(to)); to += WEST) {
      if (pos.at(to) != Piece::None) break;  // Hit a piece
      // Process move to 'to'
  }
  
  reset_board(pos);                // Reset to completely empty state
  // Board is now ready for FEN loading or manual piece placement
  ```

---

## debug.hpp — Debug & Validation API

- **Namespace:** `Debug`
- **Comprehensive Position Validation:**
  - `validate_position_consistency(const Position& pos, const std::string& expected_fen)` — Master validation function comparing position against expected FEN
  - **Component-Specific Validators:**
    - `validate_bitboards_consistency(const Position& pos)` — Verify bitboards match board array
    - `validate_piece_counts_consistency(const Position& pos)` — Verify piece counts match actual pieces
    - `validate_piece_lists_consistency(const Position& pos)` — Verify piece lists contain correct squares
    - `validate_material_scores_consistency(const Position& pos)` — Verify material scores match pieces
    - `validate_king_squares_consistency(const Position& pos)` — Verify king positions are tracked correctly
    - `validate_zobrist_consistency(const Position& pos)` — Verify Zobrist hash matches position
    - `validate_en_passant_consistency(const Position& pos)` — Verify en passant squares are valid for side to move
    - `validate_castling_consistency(const Position& pos)` — Verify castling rights match piece positions
- **Data Integrity Protection:**
  - **Bitboard Validation**: Ensures pawn bitboards (individual and combined) match board array exactly
  - **Piece Count Validation**: Verifies stored piece counts match actual pieces on board
  - **Piece List Validation**: Ensures piece lists contain correct squares for each piece type
  - **Material Score Validation**: Verifies cached material scores match actual piece values
  - **King Position Validation**: Ensures king square tracking is accurate for both colors
  - **Zobrist Hash Validation**: Verifies incremental hash matches full recalculation
  - **En Passant Validation**: Ensures ep squares are on correct ranks (rank 6 for White to move, rank 3 for Black to move) with required pawn presence
  - **Castling Validation**: Ensures Kings and Rooks are on starting squares when castling rights are set
- **Error Detection & Reporting:**
  - **Detailed Error Messages**: Specific descriptions of what's wrong and expected vs actual values
  - **Assertion Integration**: Optional assertion failures with file/line information for debugging
  - **Visual Feedback**: Clear success (✓) and failure indicators with problem descriptions
  - **FEN Comparison**: Square-by-square comparison with expected position from FEN
- **Usage Examples:**
  ```cpp
  // Comprehensive validation
  Position pos;
  pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  bool valid = Debug::validate_position_consistency(pos, expected_fen);
  
  // Individual component validation
  if (!Debug::validate_castling_consistency(pos)) {
      std::cout << "Castling rights inconsistent with piece positions!" << std::endl;
  }
  
  // After making moves
  make_move(pos, move);
  if (!Debug::validate_position_consistency(pos, expected_fen_after_move)) {
      std::cout << "Position corruption detected after move!" << std::endl;
  }
  ```
- **Demo Applications:**
  - `debug_demo.exe` — Demonstrates comprehensive validation with passing and failing scenarios
  - `castling_demo.exe` — Shows castling validation detecting various inconsistency types
- **Development Benefits:**
  - **Bug Detection**: Catches position corruption, incremental update errors, and data structure inconsistencies
  - **Regression Testing**: Validates that changes don't break existing functionality
  - **Development Confidence**: Ensures data integrity throughout move making/unmaking
  - **Debugging Support**: Pinpoints exact location and nature of position corruption

---

## Debug Assertions — Development & Testing API

- **Assertion Macros:**
  - `DEBUG_ASSERT(condition, message)` — Detailed assertion with custom error message
  - `CHESS_ASSERT(condition)` — Simple assertion for basic checks
- **Conditional Compilation:**
  - Active when `DEBUG` is defined, completely compiled out otherwise
  - Zero performance overhead in release builds
- **Error Output:**
  - File name (`__FILE__`), line number (`__LINE__`), function name (`__FUNCTION__`)
  - Failed condition code and custom message
- **Build Configuration:**
  - CMake: `-DENABLE_DEBUG_ASSERTIONS=ON` or `-DCMAKE_BUILD_TYPE=Debug`
  - Manual: `-DDEBUG` compiler flag
- **Integration:**
  - Validates move parameters, square indices, piece list operations
  - Checks board state consistency and array bounds
  - Detects programming errors during development
- **Example Output:**
  ```
  ASSERTION FAILED: Too many pieces of this type on the board
    File: src/position.hpp
    Line: 175
    Function: add_piece_to_list
    Condition: pCount[color][type] < MAX_PIECES_PER_TYPE
  ```

---

## perft.hpp — Performance Testing & Move Generation Validation

- **Perft Function:**
  - `perft(Position& pos, int depth)` — Count all possible positions at given depth using legal move generation
  - **Recursive Implementation**: Explores game tree to specified depth, counting leaf nodes
  - **Legal Move Filtering**: Uses `generate_legal_moves()` to ensure only valid moves are counted
  - **Standard Chess Validation**: Implements industry-standard perft testing for engine verification
- **Performance Validation:**
  - **Known Test Positions**: Supports standard perft test suite including Kiwipete position
  - **Depth Testing**: Validates move generation correctness at depths 1-6
  - **Debugging Support**: Detailed move-by-move breakdown for perft analysis
- **Test Suite Integration:**
  - **Automated Testing**: Comprehensive test cases in `test_perft.cpp`
  - **Performance Benchmarks**: Timing measurements for optimization validation
  - **Regression Testing**: Ensures move generation changes don't break correctness
- **Standard Perft Positions:**
  - **Starting Position**: Depth 1: 20 moves, Depth 2: 400 nodes, Depth 3: 8,902 nodes
  - **Kiwipete Position**: `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`
    - Depth 1: 48 moves, Depth 2: 2,039 nodes, Depth 3: 97,862 nodes
  - **Complex Positions**: Tests castling, en passant, promotions, pins, and checks
- **Usage Examples:**
  ```cpp
  Position pos;
  pos.set_startpos();
  
  // Test starting position perft
  uint64_t depth3 = perft(pos, 3);  // Should be 8,902
  
  // Test Kiwipete position
  pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  uint64_t kiwipete_d1 = perft(pos, 1);  // Should be 48
  uint64_t kiwipete_d2 = perft(pos, 2);  // Should be 2,039
  
  // Per-move breakdown for debugging
  MoveList moves;
  generate_legal_moves(pos, moves);
  for (const auto& move : moves.v) {
      pos.make_move_with_undo(move);
      uint64_t count = perft(pos, depth - 1);
      pos.undo_move();
      std::cout << "Move: " << move_to_algebraic(move) << " -> " << count << std::endl;
  }
  ```
- **Demo Applications:**
  - `perft_suite_demo.exe` — Comprehensive perft testing against EPD test suite
  - `debug_kiwipete_perft.exe` — Detailed Kiwipete position analysis
  - `debug_perft.exe` — General perft debugging and validation

---

## Build System & Demo Applications

### **CMake Build Configuration**
- **Build Types:**
  - `Release` — Optimized production build with maximum performance
  - `Debug` — Development build with debug assertions and debugging symbols
  - `RelWithDebInfo` — Release optimization with debugging information
- **Build Commands:**
  ```powershell
  # Configure and build (PowerShell)
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  
  # Enable debug assertions
  cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_ASSERTIONS=ON
  cmake --build build_debug --config Debug
  ```
- **Build Targets:**
  - `huginn.exe` — Main engine executable
  - `huginn_tests.exe` — Comprehensive test suite
  - Demo executables: `debug_demo.exe`, `fen_demo.exe`, `sq_attacked_demo.exe`, `castling_demo.exe`
  - Perft executables: `perft_suite_demo.exe`, `debug_kiwipete_perft.exe`, `debug_perft.exe`

### **Demo Applications**
- **Position & Board Demos:**
  - `huginn.exe` — Main position display and basic functionality demo
  - `fen_demo.exe` — FEN parsing, generation, and position setup demonstration
  - `debug_demo.exe` — Comprehensive position validation and debugging features
- **Move Generation & Attack Demos:**
  - `sq_attacked_demo.exe` — Square attack detection performance and accuracy demonstration
  - `castling_demo.exe` — Castling validation and edge case handling
- **Performance & Validation Demos:**
  - `perft_suite_demo.exe` — Comprehensive perft testing against standard test positions
  - `debug_kiwipete_perft.exe` — Detailed analysis of the famous Kiwipete position
  - `debug_perft.exe` — General perft debugging with move-by-move breakdown
  - `debug_kiwipete_perft_compare.exe` — Compare perft results against expected values
- **Testing & Validation:**
  - `huginn_tests.exe` — Complete test suite with Google Test framework
  - Covers all major functionality: move generation, position management, attack detection, perft validation

### **Development Workflow**
- **Initial Setup:**
  ```powershell
  # Clone and build
  git clone <repository-url>
  cd Huginn
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  ```
- **Testing:**
  ```powershell
  # Run comprehensive test suite
  .\build\huginn_tests.exe
  
  # Run specific demos
  .\build\debug_demo.exe
  .\build\perft_suite_demo.exe
  ```
- **Performance Validation:**
  ```powershell
  # Validate move generation correctness
  .\build\debug_kiwipete_perft.exe
  
  # Test attack detection performance
  .\build\sq_attacked_demo.exe
  ```

---

## Engine Status & Development Roadmap

### **Current Engine Status**
The Huginn chess engine represents a **solid foundation** for high-performance chess engine development:

- **Core Architecture Complete**: Pure S_MOVE system with optimal performance characteristics
- **Position Management**: Full FEN support, incremental updates, comprehensive state tracking
- **Move Generation**: Complete pseudo-legal and legal move generation with comprehensive validation
- **Attack Detection**: Ultra-fast square attack detection with piece list optimization
- **Data Integrity**: Comprehensive validation suite ensuring position consistency
- **Performance Optimized**: 24-40x faster move making/unmaking through incremental updates

### **Ready for Advanced Development**
The engine is now positioned for **search algorithm implementation**:

- **S_MOVE Integration**: Built-in move scoring ready for search optimization
- **Performance Foundation**: O(1) make/unmake operations enable deep search
- **Validation Suite**: Comprehensive testing ensures reliability during development
- **Clean Architecture**: Modern C++ design facilitates feature additions

### **Planned Enhancements**
- **Search Engine**: Minimax with alpha-beta pruning, iterative deepening, transposition tables
- **Evaluation Function**: Material balance, piece-square tables, pawn structure, king safety
- **Move Ordering**: Killer moves, history heuristic, MVV-LVA, principal variation moves
- **UCI Protocol**: Standard chess engine communication protocol for GUI integration
- **Advanced Search**: Null move pruning, late move reductions, futility pruning, quiescence search

### **Architecture Highlights**
- **Memory Efficiency**: 8-byte S_MOVE structure vs 12+ bytes for traditional representations (33% reduction)
- **Cache Performance**: Compact bitboard operations and piece list optimization
- **Zero Legacy Overhead**: Pure modern architecture without compatibility layers
- **Incremental Updates**: Complete derived state maintained with O(1) operations
- **Comprehensive Testing**: Industry-standard perft validation and extensive unit testing

The Huginn engine demonstrates **professional-grade chess engine architecture** with performance characteristics suitable for competitive play once search and evaluation components are implemented.

---

## movegen_enhanced.hpp — Enhanced Move Generation System

### **S_MOVELIST Structure - High-Performance Move Container**

- **Fixed-Size Array Design:**
  ```cpp
  struct S_MOVELIST {
      S_MOVE moves[MAX_POSITION_MOVES];  // 256 move capacity
      int count;
  };
  ```
- **Performance Benefits:**
  - **Cache Efficiency**: Fixed-size array eliminates vector overhead and heap allocations
  - **Predictable Memory**: 256 * 8 bytes = 2KB per move list for optimal cache usage
  - **Fast Access**: Direct array indexing without dynamic allocation costs
  - **Stack Allocation**: Entire structure lives on stack for maximum performance

### **Specialized Move Addition Methods**

- **Move Type Specific Addition:**
  - `add_quiet_move(const S_MOVE& move)` — Non-capture moves (score: 0)
  - `add_capture_move(const S_MOVE& move, const Position& pos)` — Captures with MVV-LVA scoring
  - `add_en_passant_move(const S_MOVE& move)` — En passant captures (score: 1,000,105)
  - `add_promotion_move(const S_MOVE& move)` — Promotions (score: 2,000,000+)
  - `add_castle_move(const S_MOVE& move)` — Castling moves (score: 50,000)

### **Advanced Move Scoring System**

- **MVV-LVA (Most Valuable Victim - Least Valuable Attacker):**
  ```cpp
  // Capture scoring formula
  score = 1,000,000 + (10 * victim_value) - attacker_value
  ```
- **Score Ranges:**
  - **Promotions**: 2,000,000+ (highest priority)
  - **Captures**: 1,000,000+ (MVV-LVA based)
  - **En Passant**: 1,000,105 (captures pawn)
  - **Castling**: 50,000 (moderate priority)
  - **Quiet Moves**: 0 (lowest priority)

### **Piece-Type Specialized Generation**

- **Modular Architecture:**
  - `generate_pawn_moves()` — Complete pawn logic (pushes, captures, promotions, en passant)
  - `generate_knight_moves()` — Knight L-shaped moves
  - `generate_sliding_moves()` — Generic slider function for bishops, rooks, queens
  - `generate_king_moves()` — King moves and castling with full validation
- **Optimizations:**
  - **Piece List Usage**: Leverages `pCount[color][type]` and `pList[color][type][index]`
  - **Early Termination**: Sliding pieces stop when blocked
  - **Direction Arrays**: Pre-computed direction vectors for each piece type
  - **Boundary Detection**: Uses `is_playable()` for efficient edge detection

### **Enhanced Legal Move Generation**

- **Two-Phase Process:**
  1. **Generate Pseudo-Legal**: All moves following piece rules (fast)
  2. **Filter for Legality**: Remove moves leaving king in check (thorough)
- **King Safety Validation:**
  - Tests each pseudo-legal move by making it temporarily
  - Uses `SqAttacked()` to verify king safety after move
  - Restores position using `undo_move()` for perfect cleanup
- **Performance Characteristics:**
  - **Pseudo-Legal**: ~0.09 microseconds (suitable for move ordering)
  - **Legal**: ~2.0 microseconds (suitable for gameplay and search)

### **Container Interface & Utilities**

- **STL-Compatible Interface:**
  - `size()`, `clear()`, `operator[]`, `begin()`, `end()`
  - Range-based for loop support
  - Standard container semantics
- **Move Ordering:**
  - `sort_by_score()` — Sort moves by score (highest first)
  - Optimized for search algorithms requiring good move ordering
- **Iterator Support:**
  ```cpp
  S_MOVELIST moves;
  generate_all_moves(pos, moves);
  
  // Range-based iteration
  for (const auto& move : moves) {
      // Process move...
  }
  ```

### **Usage Examples**

```cpp
// Basic move generation
Position pos;
pos.set_startpos();

S_MOVELIST moves;
generate_all_moves(pos, moves);

std::cout << "Generated " << moves.size() << " moves" << std::endl;

// Move scoring and ordering
moves.sort_by_score();
std::cout << "Best move score: " << moves[0].score << std::endl;

// Legal move generation for gameplay
S_MOVELIST legal_moves;
generate_legal_moves_enhanced(pos, legal_moves);

// Check for checkmate/stalemate
Color us = pos.side_to_move;
bool in_check = SqAttacked(pos.king_sq[int(us)], pos, !us);
if (legal_moves.size() == 0) {
    if (in_check) {
        std::cout << "Checkmate!" << std::endl;
    } else {
        std::cout << "Stalemate!" << std::endl;
    }
}

// Performance testing
auto start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < 100000; ++i) {
    S_MOVELIST test_moves;
    generate_all_moves(pos, test_moves);
}
auto end = std::chrono::high_resolution_clock::now();
// Measure performance...

// Move type analysis
for (const auto& move : moves) {
    if (move.is_capture()) {
        std::cout << "Capture: score " << move.score << std::endl;
    } else if (move.is_promotion()) {
        std::cout << "Promotion: score " << move.score << std::endl;
    } else if (move.is_castle()) {
        std::cout << "Castle: score " << move.score << std::endl;
    }
}
```

### **Integration with Search Algorithms**

- **Move Ordering Ready**: Built-in scoring system optimized for alpha-beta search
- **Memory Efficient**: Stack-allocated containers reduce garbage collection pressure
- **Fast Generation**: Suitable for millions of calls during deep search
- **Legal Filtering**: Ensures all generated moves are playable
- **Score-Based Sorting**: Critical for search pruning and move ordering heuristics

---

## Engine3_src/hybrid_evaluation.hpp — Engine3 Hybrid Evaluation API

### **Namespace: Engine3**

#### **HybridEvaluator Class**
```cpp
class HybridEvaluator {
public:
    int evaluate(const Position& pos);
    
private:
    static GamePhase detect_game_phase(const Position& pos);
    static int evaluate_material(const Position& pos, GamePhase phase);
    static int evaluate_piece_square_tables(const Position& pos, GamePhase phase);
    static int evaluate_pawn_structure(const Position& pos);
    static int evaluate_piece_activity(const Position& pos, GamePhase phase);
    static int evaluate_king_safety(const Position& pos, GamePhase phase);
    static int evaluate_development(const Position& pos, GamePhase phase);
    static int evaluate_mobility(const Position& pos, GamePhase phase);
};
```

#### **Game Phase Detection**
- **GamePhase enum:** `OPENING`, `MIDDLEGAME`, `ENDGAME`
- **Opening Threshold:** 24+ pieces remaining
- **Middlegame Threshold:** 12-23 pieces remaining  
- **Endgame:** <12 pieces remaining

#### **Evaluation Components**
- **Material Values:**
  - Pawn: 100 cp, Knight: 300 cp, Bishop: 350 cp
  - Rook: 500 cp, Queen: 1000 cp, King: 10000 cp
- **Positional Features:**
  - Piece-Square Tables for all pieces with game phase interpolation
  - Pawn structure: isolated (-15cp), doubled (-10cp), passed pawns (bonus)
  - King safety: pawn shield (+40cp castle bonus), attack penalties
  - Piece activity: knight outposts (+25cp), bishop pairs (+50cp), open files (+15cp)
  - Development bonuses: early piece development (+15-20cp in opening)
- **Mobility Evaluation:** Dynamic piece mobility scoring with phase-dependent weights

---

## Engine3_src/simple_search.hpp — Engine3 Search Engine API

### **SimpleEngine Class**
```cpp
class SimpleEngine {
public:
    S_MOVE search(const Position& pos, const SearchLimits& limits);
    void stop();
    static std::string move_to_uci(const S_MOVE& move);
    
private:
    int alpha_beta(Position& pos, int alpha, int beta, int depth, PVLine& pv);
    int quiescence(Position& pos, int alpha, int beta, PVLine& pv);
    void order_moves(S_MOVELIST& moves, const Position& pos, S_MOVE hash_move);
};
```

#### **Search Features**
- **Alpha-Beta Pruning:** Minimax with alpha-beta pruning for efficient tree search
- **Quiescence Search:** Tactical extension search to avoid horizon effect
- **Iterative Deepening:** Progressive depth increase from 1 to target depth
- **Move Ordering:** Sophisticated ordering for improved alpha-beta cut-offs:
  - Hash move (from transposition table)
  - Captures ordered by MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
  - Killer moves (moves that caused cutoffs at same depth)
  - History heuristic (moves that historically performed well)

#### **Search Limits**
```cpp
struct SearchLimits {
    int max_depth = 0;      // Maximum search depth (0 = no limit)
    int max_time_ms = 0;    // Maximum search time in milliseconds
    int max_nodes = 0;      // Maximum nodes to search
    bool infinite = false;  // Search until stopped
};
```

#### **Principal Variation (PV)**
```cpp
struct PVLine {
    std::vector<S_MOVE> moves;  // Sequence of best moves
    void clear();
    void add_move(S_MOVE move);
    std::string to_uci() const;
};
```

#### **Performance Characteristics**
- **Search Speed:** 70,000-90,000 nodes per second (single-threaded)
- **Search Depth:** Typical depth 5-8 in tournament time controls
- **Move Ordering Efficiency:** High alpha-beta cut-off rate due to sophisticated ordering
- **Quiescence Depth:** Extends 4-8 plies to resolve tactical sequences

#### **UCI Integration**
- **Real-time Search Info:** Depth, nodes, time, NPS, score, and PV reported during search
- **Proper Move Encoding:** All moves correctly formatted as UCI (e.g., "e2e4", "e7e8q")
- **Time Management:** Respects time limits and can be stopped gracefully
- **Mate Detection:** Reports mate scores as +/-32000 cp

---

## Engine Architecture Overview

### **Engine3 vs Legacy Comparison**

| Feature | Engine3 (v1.1) | Legacy Engine |
|---------|-----------------|---------------|
| **Evaluation** | Hybrid system with game phase detection | Basic material + PST |
| **Search** | Alpha-beta + quiescence + iterative deepening | Multi-threaded with bugs |
| **Move Encoding** | Proper UCI (d2d4, e2e4) | Broken "0000" moves |
| **Performance** | 70k-90k nps, stable | Variable, evaluation bugs |
| **Position Assessment** | Realistic evaluations | +578cp perspective bug |
| **Mate Detection** | Accurate +/-32000cp | Inconsistent |
| **Code Quality** | Clean, single-threaded, reliable | Complex, threading issues |

### **Usage Example**
```cpp
#include "Engine3_src/simple_search.hpp"
#include "Engine3_src/hybrid_evaluation.hpp"

int main() {
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    // Evaluation
    Huginn::HybridEvaluator evaluator;
    int score = evaluator.evaluate(pos);
    
    // Search
    Huginn::SimpleEngine engine;
    Huginn::SearchLimits limits;
    limits.max_depth = 6;
    
    S_MOVE best_move = engine.search(pos, limits);
    std::string uci_move = Huginn::SimpleEngine::move_to_uci(best_move);
    
    return 0;
}
```

### **Performance Metrics**

- **Pseudo-Legal Generation**: 0.09 microseconds per position (starting position)
- **Legal Move Generation**: 2.0 microseconds per position (starting position)
- **Memory Usage**: 2KB per S_MOVELIST (256 moves × 8 bytes)
- **Cache Performance**: Optimized for L1/L2 cache efficiency
- **Scalability**: Performance scales well with piece reduction in endgames

### **Comprehensive Testing**

- **Unit Test Coverage**: 11 comprehensive test cases covering all functionality
- **Performance Validation**: Automated performance benchmarks with expected thresholds
- **Correctness Testing**: MVV-LVA scoring, legal move filtering, move type classification
- **Integration Testing**: Compatibility with existing position and move systems
- **Regression Testing**: All 184 existing tests continue to pass

The enhanced move generation system represents a **significant advancement in chess engine architecture**, providing the performance foundation needed for competitive search algorithms while maintaining code clarity and extensibility. Combined with the existing S_MOVE structure and position management, Huginn now has all the essential building blocks for implementing high-performance search and evaluation algorithms.

---

## evaluation.hpp — Position Evaluation API

The evaluation system provides comprehensive positional assessment for the Huginn chess engine, designed to work seamlessly with the search algorithm for accurate position scoring.

### **Core Constants & Material Values**

- **Material Values:**
  ```cpp
  constexpr int PAWN_VALUE = 100;
  constexpr int KNIGHT_VALUE = 320; 
  constexpr int BISHOP_VALUE = 330;
  constexpr int ROOK_VALUE = 500;
  constexpr int QUEEN_VALUE = 900;
  constexpr int KING_VALUE = 20000;
  ```

- **Evaluation Bounds:**
  ```cpp
  constexpr int INFINITE_SCORE = 30000;
  constexpr int MATE_SCORE = 29000;
  constexpr int DRAW_SCORE = 0;
  ```

### **Piece-Square Tables (PSTs)**

The evaluation system includes sophisticated piece-square tables for positional assessment:

- **Available PSTs:**
  - `PAWN_PST[64]` — Pawn positioning values (encourages center control and advancement)
  - `KNIGHT_PST[64]` — Knight positioning values (prefers central squares)
  - `BISHOP_PST[64]` — Bishop positioning values (favors long diagonals)
  - `ROOK_PST[64]` — Rook positioning values (emphasizes file control)
  - `QUEEN_PST[64]` — Queen positioning values (balanced central activity)
  - `KING_PST_MG[64]` — King positioning for middlegame (safety priority)
  - `KING_PST_EG[64]` — King positioning for endgame (activity priority)

- **PST Design Features:**
  - **Middlegame Focus:** Piece tables optimized for middle game positioning
  - **Endgame Adaptation:** Separate king tables for endgame centralization
  - **Incremental Updates:** Compatible with efficient position manipulation
  - **Tuned Values:** Empirically balanced for competitive play

### **Core Evaluation Functions**

- **Primary Interface:**
  ```cpp
  int evaluate_position(const Position& pos);  // Main evaluation function
  ```

- **Material Evaluation:**
  ```cpp
  int evaluate_material(const Position& pos);  // Pure material count
  int get_piece_value(PieceType piece_type);   // Individual piece values
  ```

- **Positional Evaluation:**
  ```cpp
  int evaluate_piece_square_tables(const Position& pos);  // PST-based positioning
  int evaluate_king_safety(const Position& pos, Color color);  // King safety assessment
  int evaluate_pawn_structure(const Position& pos, Color color);  // Pawn structure analysis
  ```

- **Game Phase Detection:**
  ```cpp
  bool is_endgame(const Position& pos);  // Detect endgame conditions
  int get_game_phase_score(const Position& pos);  // Material-based phase score
  ```

- **Special Position Detection:**
  ```cpp
  bool is_checkmate(const Position& pos);  // Detect checkmate positions
  bool is_stalemate(const Position& pos);  // Detect stalemate positions
  bool is_insufficient_material(const Position& pos);  // Detect insufficient material draws
  ```

### **Advanced Positional Features**

- **King Safety Evaluation:**
  - Evaluates attack zones around the king
  - Considers pawn shield quality
  - Assesses open file dangers
  - Weighs attacker piece values and proximity
  - Provides differential evaluation for both sides

- **Pawn Structure Analysis:**
  - **Doubled Pawns:** Penalty for pawns on same file (-20 centipawns)
  - **Isolated Pawns:** Penalty for unsupported pawns (-25 centipawns)
  - **Passed Pawns:** Bonus for unobstructed pawns (+30 centipawns)
  - **Pawn Chains:** Recognition of connected pawn structures
  - **Weak Squares:** Identification of holes in pawn structure

- **Piece Coordination:**
  - Bishop pair evaluation
  - Rook on open/semi-open files
  - Knight outposts on strong squares
  - Queen positioning relative to king safety

### **Performance Characteristics**

- **Evaluation Speed:** Optimized for millions of calls during search
- **Incremental Design:** Ready for future incremental update optimizations
- **Cache Efficiency:** Minimal memory footprint with efficient data access
- **Search Integration:** Designed specifically for alpha-beta search algorithms
- **Tunable Parameters:** Easily adjustable values for engine strength tuning

### **Usage Examples**

```cpp
// Basic position evaluation
Position pos;
pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

// Get comprehensive position score
int score = evaluate_position(pos);  // Returns score from White's perspective
std::cout << "Position evaluation: " << score << " centipawns" << std::endl;

// Individual evaluation components
int material = evaluate_material(pos);
int positional = evaluate_piece_square_tables(pos);
int king_safety_white = evaluate_king_safety(pos, Color::White);
int king_safety_black = evaluate_king_safety(pos, Color::Black);

// Game phase and special positions
if (is_checkmate(pos)) {
    std::cout << "Checkmate position detected" << std::endl;
} else if (is_stalemate(pos)) {
    std::cout << "Stalemate position detected" << std::endl;
} else if (is_endgame(pos)) {
    std::cout << "Endgame phase detected" << std::endl;
}

// Pawn structure evaluation
int white_pawn_score = evaluate_pawn_structure(pos, Color::White);
int black_pawn_score = evaluate_pawn_structure(pos, Color::Black);

// Side-relative evaluation (important for search)
Color side_to_move = pos.side_to_move;
int side_relative_score = (side_to_move == Color::White) ? score : -score;
```

### **Integration with Search**

- **Search Compatibility:** Designed for seamless integration with alpha-beta search
- **Move Ordering:** Evaluation scores used for move ordering and pruning decisions
- **Terminal Node Evaluation:** Provides leaf node scores for search tree
- **Checkmate Detection:** Returns appropriate mate scores for search algorithm
- **Draw Recognition:** Handles stalemate and insufficient material scenarios

### **Evaluation Philosophy**

The Huginn evaluation system follows classical chess evaluation principles:

- **Material First:** Strong emphasis on piece values and material balance
- **King Safety Priority:** Comprehensive king safety evaluation for tactical awareness
- **Positional Understanding:** Piece-square tables provide solid positional foundation
- **Pawn Structure:** Recognition of fundamental pawn weaknesses and strengths
- **Endgame Adaptation:** Phase-dependent evaluation for different game stages
- **Tunable Architecture:** Easy adjustment of evaluation parameters for strength tuning

---

## search.hpp — Chess Search Engine API

The search system implements a sophisticated minimax search with alpha-beta pruning, designed for competitive chess play with advanced search techniques and optimizations.

### **Search Architecture**

The search engine is built around several key components:

- **Main Search Engine:** `Search::Engine` class providing complete search functionality
- **Transposition Table:** Hash-based position caching for search speedup
- **Move Ordering:** Intelligent move prioritization for better pruning
- **Principal Variation:** Best move sequence tracking and reporting
- **Time Management:** Sophisticated time control and search limits

### **Core Search Constants**

```cpp
namespace Search {
    constexpr int MAX_DEPTH = 64;        // Maximum search depth
    constexpr int MAX_PLY = 100;         // Maximum ply for arrays
    constexpr int INFINITE_TIME = 1000000; // Infinite time constant
    constexpr int MATE_SCORE = 32000;    // Mate score base
    constexpr int MATE_IN_MAX_PLY = MATE_SCORE - MAX_PLY; // Mate bound
}
```

### **Search Limits & Controls**

- **SearchLimits Structure:**
  ```cpp
  struct SearchLimits {
      int max_depth = MAX_DEPTH;                         // Maximum depth to search
      std::chrono::milliseconds max_time{INFINITE_TIME}; // Time limit for search
      uint64_t max_nodes = UINT64_MAX;                   // Node limit for search
      bool infinite = false;                             // Infinite search flag
      
      // Time control support
      std::chrono::milliseconds remaining_time{0};       // Time remaining
      std::chrono::milliseconds increment{0};            // Time increment
      int moves_to_go = 0;                               // Moves to time control
  };
  ```

### **Principal Variation (PV) System**

- **PVLine Structure:**
  ```cpp
  struct PVLine {
      S_MOVE moves[MAX_DEPTH];  // Principal variation moves
      int length = 0;           // Number of moves in PV
      
      void clear();             // Clear the PV line
      void copy_from(const PVLine& other);  // Copy PV from another line
  };
  ```

### **Search Statistics & Information**

- **SearchStats Structure:**
  ```cpp
  struct SearchStats {
      uint64_t nodes_searched = 0;      // Total nodes searched
      uint64_t qnodes_searched = 0;     // Quiescence nodes searched
      int depth_reached = 0;            // Maximum depth reached
      int selective_depth = 0;          // Selective search depth
      std::chrono::milliseconds time_elapsed{0};  // Time elapsed
      double nodes_per_second = 0.0;    // Nodes per second calculation
  };
  ```

- **SearchInfo Structure (UCI Communication):**
  ```cpp
  struct SearchInfo {
      int depth = 0;               // Current search depth
      int score = 0;               // Position score
      uint64_t nodes = 0;          // Nodes searched
      int time_ms = 0;             // Time in milliseconds
      std::vector<S_MOVE> pv;      // Principal variation
  };
  ```

### **Transposition Table System**

- **TTEntry Structure:**
  ```cpp
  struct TTEntry {
      uint64_t key = 0;           // Position hash key
      S_MOVE best_move;           // Best move from this position
      int16_t score = 0;          // Position score
      int16_t eval = 0;           // Static evaluation
      uint8_t depth = 0;          // Search depth
      TTFlag flag = TTFlag::EXACT; // Score type (EXACT, ALPHA, BETA)
      uint8_t age = 0;            // Search generation
  };
  ```

- **TranspositionTable Class:**
  ```cpp
  class TranspositionTable {
  public:
      TranspositionTable(size_t size_mb = 64);  // Constructor with size
      
      void clear();                             // Clear all entries
      void new_search();                        // New search generation
      bool probe(uint64_t key, TTEntry& entry) const;  // Probe for entry
      void store(uint64_t key, const S_MOVE& best_move, int score, 
                int eval, int depth, TTFlag flag);      // Store entry
      size_t get_hashfull() const;             // Get hash table usage
      void resize(size_t size_mb);             // Resize table
  };
  ```

### **Move Ordering System**

- **MoveOrderer Class:**
  ```cpp
  class MoveOrderer {
  public:
      void clear();  // Clear move ordering history
      void order_moves(S_MOVELIST& moves, Color color, int ply, 
                      const S_MOVE& pv_move = S_MOVE()) const;
      
      // Update move ordering statistics
      void update_killer(const S_MOVE& move, int ply);
      void update_history(const S_MOVE& move, Color color, int depth);
  };
  ```

- **Move Ordering Features:**
  - **Hash Move:** Best move from transposition table (highest priority)
  - **MVV-LVA:** Most Valuable Victim - Least Valuable Attacker for captures
  - **Killer Moves:** Non-capture moves that caused beta cutoffs
  - **History Heuristic:** Move success statistics across positions
  - **Promotion Ordering:** Queen promotions prioritized over other promotions

### **Main Search Engine**

- **Engine Class Interface:**
  ```cpp
  class Engine {
  public:
      Engine(size_t tt_size_mb = 64);  // Constructor
      
      // Main search interface
      S_MOVE search(const Position& pos, const SearchLimits& search_limits);
      void stop();                     // Stop current search
      
      // Configuration
      void set_hash_size(size_t size_mb);    // Set transposition table size
      void clear_hash();                     // Clear hash tables
      void set_info_callback(std::function<void(const SearchInfo&)> callback);
      
      // Information access
      const SearchStats& get_stats() const;
      const PVLine& get_pv() const;
  };
  ```

### **Search Algorithm Features**

- **Alpha-Beta Pruning:** Efficient minimax search with alpha-beta pruning
- **Principal Variation Search (PVS):** Optimized search for improved performance
- **Iterative Deepening:** Gradually increasing search depth with time management
- **Quiescence Search:** Extended search for tactical sequences and captures
- **Transposition Table:** Position caching for search speedup and move ordering
- **Move Ordering:** Intelligent move prioritization for maximum pruning efficiency
- **Time Management:** Sophisticated time allocation and search termination

### **Advanced Search Techniques**

- **Principal Variation Search (PVS):**
  - Searches the first move with full window
  - Subsequent moves searched with null window
  - Re-searches if null window search fails high
  - Provides significant performance improvement in many positions

- **Quiescence Search:**
  - Extends search until position is "quiet" (no captures)
  - Prevents horizon effect in tactical positions
  - Uses delta pruning for efficiency
  - Integrates with main search evaluation

- **Search Extensions:**
  - Check extension: Extend search when in check
  - Single reply extension: Extend when only one legal move
  - Recapture extension: Extend search for immediate recaptures

### **Performance Characteristics**

The search engine demonstrates excellent performance:

- **Search Speed:** 38,000+ nodes per second at depth 6
- **Branching Factor:** Efficient pruning reduces effective branching factor
- **Time Management:** Intelligent time allocation for optimal play
- **Memory Usage:** Configurable transposition table (default 64MB)
- **Scalability:** Performance scales well with increased hash size and depth

### **Usage Examples**

```cpp
// Basic search setup
Search::Engine engine(64);  // 64MB transposition table

Position pos;
pos.set_startpos();

// Configure search limits
Search::SearchLimits limits;
limits.max_depth = 6;
limits.max_time = std::chrono::milliseconds(5000);  // 5 seconds

// Set up search info callback for UCI communication
engine.set_info_callback([](const Search::SearchInfo& info) {
    std::cout << "info depth " << info.depth
              << " nodes " << info.nodes
              << " time " << info.time_ms
              << " score cp " << info.score;
    
    if (!info.pv.empty()) {
        std::cout << " pv";
        for (const auto& move : info.pv) {
            std::cout << " " << move_to_uci(move);
        }
    }
    std::cout << std::endl;
});

// Perform search
S_MOVE best_move = engine.search(pos, limits);

// Access search statistics
const Search::SearchStats& stats = engine.get_stats();
std::cout << "Searched " << stats.nodes_searched << " nodes in "
          << stats.time_elapsed.count() << "ms" << std::endl;
std::cout << "Performance: " << stats.nodes_per_second << " nodes/second" << std::endl;

// Get principal variation
const Search::PVLine& pv = engine.get_pv();
std::cout << "Principal variation: ";
for (int i = 0; i < pv.length; ++i) {
    std::cout << move_to_uci(pv.moves[i]) << " ";
}
std::cout << std::endl;
```

### **Time Management**

The engine includes sophisticated time management:

- **Time Control Support:** Handles various time control formats (movetime, wtime/btime, infinite)
- **Dynamic Allocation:** Intelligent time allocation based on position complexity
- **Search Stability:** Allows search completion while respecting time limits
- **Panic Mode:** Emergency time management for low-time situations

### **Integration with Evaluation**

- **Leaf Node Evaluation:** Uses evaluation system for terminal position assessment
- **Mate Detection:** Properly handles checkmate and stalemate positions
- **Score Conversion:** Converts evaluation scores to search-compatible format
- **Draw Recognition:** Integrates with evaluation for draw detection

### **UCI Communication Integration**

The search engine is designed for seamless UCI integration:

- **Info Callback:** Real-time search information reporting
- **Standard Format:** UCI-compatible depth, nodes, time, score, and PV reporting
- **Graceful Termination:** Proper handling of stop commands and time limits
- **Best Move Reporting:** Clear communication of search results

The search system represents the culmination of classical chess engine design, providing competitive-strength search capabilities while maintaining clean, extensible architecture. Combined with the evaluation system and optimized move generation, Huginn now has all components needed for strong chess play.

---

## Complete Engine Architecture Summary

Huginn Chess Engine now represents a **complete, competitive chess engine** with all essential components implemented and integrated:

### **Core Engine Components** ✅

1. **Board Representation** — Efficient mailbox-120 system with bitboard integration
2. **Move Generation** — Optimized system delivering 34+ million moves/second (69% improvement)
3. **Position Management** — Complete position handling with make/undo moves and Zobrist hashing
4. **Attack Detection** — Ultra-fast SqAttacked system using piece lists (3.9-8.6 ns/call)
5. **Position Evaluation** — Comprehensive evaluation with material, PSTs, king safety, pawn structure
6. **Search Engine** — Alpha-beta with PVS, quiescence, transposition table, move ordering
7. **UCI Interface** — Complete protocol implementation for GUI communication

### **Performance Characteristics** 🚀

- **Move Generation:** 34+ million moves/second with 69% optimization improvement
- **Search Performance:** 38,000+ nodes/second at depth 6
- **Attack Detection:** 3.9-8.6 nanoseconds per call with piece list optimization
- **Memory Efficiency:** Configurable 64MB transposition table with efficient replacement
- **Engine Strength:** Competitive play capability with sophisticated evaluation and search

### **Engine Capabilities** 🏆

- **Complete Game Play:** Handles all chess rules including castling, en passant, promotion
- **Tournament Ready:** UCI protocol support for integration with chess GUIs
- **Configurable Strength:** Adjustable search depth and time controls
- **Advanced Features:** Principal variation, transposition table, move ordering, time management
- **Robust Architecture:** Clean, extensible codebase ready for future enhancements

### **Development Achievement** 🎯

The Huginn chess engine has successfully evolved from:
- **Initial State:** Basic board representation and move generation
- **Optimization Phase:** 69% performance improvement in move generation
- **Complete Engine:** Full evaluation, search, and UCI implementation
- **Final Result:** Tournament-capable chess engine with competitive strength

### **Future Roadmap** 🔮

With the complete engine now implemented, future development can focus on:
- **Huginn_BB Project:** Bitboard-based engine variant for maximum performance
- **Strength Tuning:** Parameter optimization and evaluation refinement  
- **Advanced Features:** Opening books, endgame tablebases, parallel search
- **Tournament Testing:** Real-world competition and rating establishment

The Huginn Chess Engine project represents a comprehensive implementation of classical chess engine architecture, providing a solid foundation for competitive chess play and future advanced development.

```
