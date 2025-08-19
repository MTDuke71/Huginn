# Huginn Chess Engine API Guide

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
  - Iterate playable squares: `for (int s : Playable120{})`
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

## squares120.hpp — Square Constants & Iteration

- **Named Constants:**  
  - `Square120::A1`, `Square120::E4`, etc. (all 64 squares)
- **Arrays:**  
  - `ALL120[64]` — all playable 120 indices
- **Iteration:**  
  - `Playable120` — range-based for over all playable squares

---

## move.hpp — Move Representation API

- **Structs:**
  - `Move { int from, int to, PieceType promo }`
- **Helpers:**
  - `make_move(int from, int to, PieceType promo = PieceType::None)`
  - `operator==(const Move&, const Move&)`

---

## movegen.hpp — Move Generation API

- **Structs:**
  - `MoveList { std::vector<Move> v }`
- **Methods:**
  - `clear()`, `add(const Move&)`, `size()`, `operator[]`
- **Functions:**
  - `generate_pseudo_legal_moves(const Position&, MoveList&)`
  - `generate_legal_moves(const Position&, MoveList&)`

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
  - `MAXPLY 2048` — Maximum search depth / game length
- **Structs:**
  - `State { ep_square, castling_rights, halfmove_clock, captured }`
  - `S_UNDO { move, castling_rights, ep_square, halfmove_clock, zobrist_key, captured, king_sq_backup[2], pawns_bb_backup[2], piece_counts_backup[7], material_score_backup[2] }` — Complete undo state with incremental update support
  - `Position { board[120], side_to_move, ep_square, castling_rights, halfmove_clock, fullmove_number, king_sq[2], pawns_bb[2], piece_counts[7], zobrist_key, pList[2], pCount[2], move_history[MAXPLY], ply }`
- **Position Management:**
  - `reset()` — Complete reset to empty state (all squares offboard/empty, all counters cleared)
  - `set_startpos()` — Set up standard chess starting position using FEN parsing
  - `set_from_fen(const std::string& fen)` — Parse FEN string and set position accordingly
  - `to_fen() const` — Generate FEN string from current position (perfect round-trip with set_from_fen)
  - `rebuild_counts()` — Recalculate all piece counts from current board state (used for FEN parsing and setup only)
- **High-Performance Incremental Updates:**
  - `save_derived_state(S_UNDO& undo)` — Save current derived state for O(1) restoration (internal function)
  - `restore_derived_state(const S_UNDO& undo)` — Restore derived state from backup in O(1) time (internal function)
  - `update_derived_state_for_move(const Move& m, Piece moving, Piece captured)` — Update derived state incrementally in O(1) time (internal function)
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
  - `make_move_with_undo(const Move& m)` — Make move with full undo support using incremental updates (O(1) performance, 24-40x faster than rebuild_counts)
  - `undo_move()` — Undo last move with perfect state restoration (O(1) performance, 24-40x faster than rebuild_counts)
- **Move Handling:**
  - `make_move(Position&, const Move&, State&)` — Simple move making
  - `unmake_move(Position&, const Move&, const State&)` — Simple move unmaking
- **Move Encoding:**
  - `S_UNDO::encode_move(from, to, promo)` — Pack move into integer
  - `S_UNDO::decode_move(encoded, from, to, promo)` — Unpack move from integer
- **Performance Optimizations:**
  - **Incremental Updates:** Make/unmake moves use O(1) incremental updates instead of O(120) board scanning
  - **State Backup/Restore:** Perfect derived state restoration in O(1) time using saved backup data
  - **Search Performance:** 24-40x faster move making/unmaking enables deep search algorithms
  - **Memory Efficiency:** Fixed-size array storage for zero allocation overhead
  - **Direct Access:** Array indexing for maximum speed with overflow protection at MAXPLY limit
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
