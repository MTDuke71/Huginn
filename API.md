# Huginn Chess Engine API Guide

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
- **Mapping:**  
  - 64↔120 translation: `MAILBOX_MAPS.to120`, `MAILBOX_MAPS.to64`
- **Move Generation:**  
  - Knight moves: `for (int d : KNIGHT_DELTAS)`
  - Sliding moves: `for (int to = from + NORTH; is_playable(to); to += NORTH)`

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
- **Hash Tables:**
  - `Piece[PIECE_NB][64]` — piece-square hash values
  - `Side` — side to move hash
  - `Castle[16]` — castling rights hash (0..15)
  - `EpFile[8]` — en passant file hash (a..h)
- **Functions:**
  - `init_zobrist(seed)` — initialize hash tables with random values
  - `compute(const Position&)` — compute Zobrist hash for current position
- **Integration:** 
  - Works with modern `Position` struct
  - Automatically handles piece placement, side to move, castling rights, en passant

---

## position.hpp — Position & State API

- **Structs:**
  - `State { ep_square, castling_rights, halfmove_clock, captured }`
  - `Position { board[120], side_to_move, ep_square, castling_rights, halfmove_clock, fullmove_number, king_sq[2], pawns_bb[2], piece_counts[7], zobrist_key }`
- **Methods:**
  - `clear()`, `set_startpos()`, `at(int s)`, `set(int s, Piece p)`, `rebuild_counts()`
- **Move Handling:**
  - `make_move(Position&, const Move&, State&)`
  - `unmake_move(Position&, const Move&, const State&)`

