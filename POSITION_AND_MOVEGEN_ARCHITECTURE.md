# Huginn Chess Engine: Position Representation & Move Generation Architecture

## Table of Contents
1. [Overview](#overview)
2. [Board Representation](#board-representation)
3. [Position Structure](#position-structure)
4. [Move Representation](#move-representation)
5. [Move Generation System](#move-generation-system)
6. [Performance Optimizations](#performance-optimizations)
7. [Key Data Structures](#key-data-structures)
8. [Code Examples](#code-examples)

---

## Overview

The Huginn chess engine uses a sophisticated hybrid approach combining multiple board representations for optimal performance:

- **Primary Board**: 120-square mailbox for move generation and validation
- **Bitboards**: 64-bit integers for pawn-specific operations and fast position queries
- **Piece Lists**: Indexed arrays for efficient piece iteration
- **Incremental Updates**: Delta-based state changes for performance

This architecture provides excellent balance between simplicity, performance, and functionality.

---

## Board Representation

### 120-Square Mailbox System

The engine uses a 120-square mailbox layout (10×12 grid) where:

```
Index Layout:
  0   1   2   3   4   5   6   7   8   9   (Offboard border)
 10  11  12  13  14  15  16  17  18  19   (Offboard border)
 20  21  22  23  24  25  26  27  28  29   (Rank 1: A1=21, B1=22, ..., H1=28)
 30  31  32  33  34  35  36  37  38  39   (Rank 2: A2=31, B2=32, ..., H2=38)
 40  41  42  43  44  45  46  47  48  49   (Rank 3)
 50  51  52  53  54  55  56  57  58  59   (Rank 4)
 60  61  62  63  64  65  66  67  68  69   (Rank 5)
 70  71  72  73  74  75  76  77  78  79   (Rank 6)
 80  81  82  83  84  85  86  87  88  89   (Rank 7)
 90  91  92  93  94  95  96  97  98  99   (Rank 8: A8=91, B8=92, ..., H8=98)
100 101 102 103 104 105 106 107 108 109   (Offboard border)
110 111 112 113 114 115 116 117 118 119   (Offboard border)
```

**Key Features:**
- **Playable squares**: [21-98] with valid file/rank constraints
- **Offboard detection**: Automatic boundary checking
- **Simple arithmetic**: Natural move direction offsets
- **Fast validation**: Single bounds check determines validity

**Direction Offsets:**
```cpp
NORTH = +10    SOUTH = -10    EAST = +1     WEST = -1
NE = +11       NW = +9        SE = -9       SW = -11
KNIGHT_DELTAS = [+21, +19, +12, +8, -8, -12, -19, -21]
```

### Coordinate Conversion

```cpp
// File/Rank to 120-square index
constexpr int sq(File f, Rank r) {
    return 21 + int(f) + int(r) * 10;
}

// Examples:
// A1 = sq(File::A, Rank::R1) = 21 + 0 + 0*10 = 21
// E4 = sq(File::E, Rank::R4) = 21 + 4 + 3*10 = 55
// H8 = sq(File::H, Rank::R8) = 21 + 7 + 7*10 = 98
```

### Dual Representation: 120-Square ↔ 64-Square

The engine maintains conversion maps between 120-square mailbox and standard 64-square notation:

```cpp
struct MailboxMaps {
    std::array<int, 120> to64;    // 120-square → 64-square (-1 for offboard)
    std::array<int, 64> to120;    // 64-square → 120-square
};
```

---

## Position Structure

### Core Position Data

```cpp
struct Position {
    // Primary board representation
    std::array<Piece, 120> board{};          // Piece at each square
    
    // Game state
    Color side_to_move{Color::White};
    int ep_square{-1};                        // En passant target square
    uint8_t castling_rights{0};               // Castling permissions bitmask
    uint16_t halfmove_clock{0};               // 50-move rule counter
    uint16_t fullmove_number{1};              // Game move number
    
    // Fast lookups
    std::array<int, 2> king_sq{-1, -1};      // King positions [White, Black]
    std::array<uint64_t, 2> pawns_bb{0, 0};  // Pawn bitboards [White, Black]
    uint64_t all_pawns_bb{0};                 // Combined pawn bitboard
    
    // Piece tracking
    std::array<int, 7> piece_counts{};        // Count by piece type
    std::array<int, 2> material_score{0, 0}; // Material balance [White, Black]
    
    // Piece lists for iteration
    std::array<PieceList, 2> pList;          // [Color][PieceType][Index] = Square
    std::array<std::array<int, 7>, 2> pCount; // Piece count [Color][PieceType]
    
    // Zobrist hashing & move history
    uint64_t zobrist_key{0};
    std::vector<S_UNDO> move_history;
    int ply{0};
};
```

### Piece Representation

```cpp
enum class PieceType : uint8_t {
    None = 0, Pawn = 1, Knight = 2, Bishop = 3, 
    Rook = 4, Queen = 5, King = 6, _Count = 7
};

enum class Color : uint8_t {
    White = 0, Black = 1, None = 2
};

enum class Piece : uint8_t {
    None = 0,
    WhitePawn = 1, WhiteKnight = 2, WhiteBishop = 3,
    WhiteRook = 4, WhiteQueen = 5, WhiteKing = 6,
    BlackPawn = 9, BlackKnight = 10, BlackBishop = 11,
    BlackRook = 12, BlackQueen = 13, BlackKing = 14,
    Offboard = 15
};
```

**Encoding Details:**
- **Color extraction**: `color_of(piece) = Color((int(piece) - 1) >> 3)`
- **Type extraction**: `type_of(piece) = PieceType(((int(piece) - 1) & 7) + 1)`
- **Piece creation**: `make_piece(color, type) = Piece(((int(color) << 3) + int(type)))`

### Castling Rights

```cpp
enum CastlingRights : uint8_t {
    CASTLE_WK = 1,  // White kingside
    CASTLE_WQ = 2,  // White queenside  
    CASTLE_BK = 4,  // Black kingside
    CASTLE_BQ = 8   // Black queenside
};
```

---

## Move Representation

### S_MOVE Structure

Moves are encoded in a compact 25-bit format within a 32-bit integer:

```cpp
struct S_MOVE {
    int move;   // Packed move data (25 bits used)
    int score;  // Move ordering score
};
```

### Bit Layout

```
Bits  0-6:  From square (7 bits, 0-127)
Bits  7-13: To square (7 bits, 0-127)  
Bits 14-17: Captured piece type (4 bits)
Bit  18:    En passant capture flag
Bit  19:    Pawn start (double push) flag
Bits 20-23: Promoted piece type (4 bits)
Bit  24:    Castle move flag
```

### Move Types & Examples

```cpp
// Quiet move: e2-e4
S_MOVE quiet_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4), 
                  PieceType::None, false, true);  // pawn_start = true

// Capture: Nxf7
S_MOVE capture(sq(File::G, Rank::R5), sq(File::F, Rank::R7), 
               PieceType::Pawn);  // captured piece type

// Promotion: e8=Q
S_MOVE promotion(sq(File::E, Rank::R7), sq(File::E, Rank::R8), 
                 PieceType::None, false, false, PieceType::Queen);

// En passant: exd6
S_MOVE en_passant(sq(File::E, Rank::R5), sq(File::D, Rank::R6), 
                  PieceType::None, true);

// Castling: O-O
S_MOVE castle(sq(File::E, Rank::R1), sq(File::G, Rank::R1), 
              PieceType::None, false, false, PieceType::None, true);
```

---

## Move Generation System

### S_MOVELIST Structure

```cpp
struct S_MOVELIST {
    S_MOVE moves[MAX_POSITION_MOVES];  // Fixed-size array (256 moves max)
    int count;
    
    // Specialized add methods with automatic scoring
    void add_quiet_move(const S_MOVE& move);
    void add_capture_move(const S_MOVE& move, const Position& pos);
    void add_en_passant_move(const S_MOVE& move);
    void add_promotion_move(const S_MOVE& move);
    void add_castle_move(const S_MOVE& move);
};
```

### Generation Process

#### 1. Piece-by-Piece Generation

```cpp
void generate_all_moves(const Position& pos, S_MOVELIST& moves) {
    moves.clear();
    
    Color us = pos.side_to_move;
    Color them = !us;
    
    // Generate for each piece type using piece lists
    for (int piece_type = int(PieceType::Pawn); piece_type <= int(PieceType::King); ++piece_type) {
        int count = pos.pCount[int(us)][piece_type];
        
        for (int i = 0; i < count; ++i) {
            int from_sq = pos.pList[int(us)][piece_type][i];
            
            switch (PieceType(piece_type)) {
                case PieceType::Pawn:   generate_pawn_moves(pos, moves, from_sq, us); break;
                case PieceType::Knight: generate_knight_moves(pos, moves, from_sq, us); break;
                case PieceType::Bishop: generate_bishop_moves(pos, moves, from_sq, us); break;
                case PieceType::Rook:   generate_rook_moves(pos, moves, from_sq, us); break;
                case PieceType::Queen:  generate_queen_moves(pos, moves, from_sq, us); break;
                case PieceType::King:   generate_king_moves(pos, moves, from_sq, us); break;
            }
        }
    }
}
```

#### 2. Pawn Move Generation

```cpp
void generate_pawn_moves(const Position& pos, S_MOVELIST& moves, int from_sq, Color us) {
    int direction = (us == Color::White) ? NORTH : SOUTH;
    int start_rank = (us == Color::White) ? 1 : 6;
    int promo_rank = (us == Color::White) ? 7 : 0;
    
    // Forward moves
    int to_sq = from_sq + direction;
    if (pos.at(to_sq) == Piece::None) {
        if (rank_of_sq120(to_sq) == promo_rank) {
            // Promotion moves
            add_promotions(moves, from_sq, to_sq);
        } else {
            moves.add_quiet_move(S_MOVE(from_sq, to_sq));
            
            // Double push from starting rank
            if (rank_of_sq120(from_sq) == start_rank) {
                int double_sq = from_sq + 2 * direction;
                if (pos.at(double_sq) == Piece::None) {
                    moves.add_quiet_move(S_MOVE(from_sq, double_sq, 
                                               PieceType::None, false, true));
                }
            }
        }
    }
    
    // Capture moves (diagonal)
    for (int capture_dir : {direction + EAST, direction + WEST}) {
        int capture_sq = from_sq + capture_dir;
        Piece target = pos.at(capture_sq);
        
        if (!is_none(target) && color_of(target) != us) {
            if (rank_of_sq120(capture_sq) == promo_rank) {
                add_capture_promotions(moves, from_sq, capture_sq, type_of(target));
            } else {
                moves.add_capture_move(S_MOVE(from_sq, capture_sq, type_of(target)), pos);
            }
        }
        
        // En passant
        if (capture_sq == pos.ep_square) {
            moves.add_en_passant_move(S_MOVE(from_sq, capture_sq, 
                                           PieceType::None, true));
        }
    }
}
```

#### 3. Sliding Piece Generation

```cpp
void generate_sliding_moves(const Position& pos, S_MOVELIST& moves, 
                           int from_sq, Color us, const int* directions, int dir_count) {
    for (int i = 0; i < dir_count; ++i) {
        int direction = directions[i];
        
        for (int to_sq = from_sq + direction; ; to_sq += direction) {
            Piece target = pos.at(to_sq);
            
            if (target == Piece::Offboard) break;  // Hit board edge
            
            if (target == Piece::None) {
                // Empty square - quiet move
                moves.add_quiet_move(S_MOVE(from_sq, to_sq));
            } else {
                // Occupied square
                if (color_of(target) != us) {
                    // Enemy piece - capture
                    moves.add_capture_move(S_MOVE(from_sq, to_sq, type_of(target)), pos);
                }
                break;  // Can't continue in this direction
            }
        }
    }
}
```

#### 4. Legal Move Filtering

```cpp
S_MOVELIST generate_legal_moves_enhanced(const Position& pos) {
    S_MOVELIST pseudo_legal;
    S_MOVELIST legal_moves;
    
    generate_all_moves(pos, pseudo_legal);
    
    for (int i = 0; i < pseudo_legal.count; ++i) {
        Position temp_pos = pos;
        
        if (temp_pos.make_move_with_undo(pseudo_legal.moves[i])) {
            // Move is legal if king is not in check after move
            if (!SqAttacked(temp_pos.king_sq[int(pos.side_to_move)], 
                           !pos.side_to_move, temp_pos)) {
                legal_moves.moves[legal_moves.count++] = pseudo_legal.moves[i];
            }
            temp_pos.undo_move();
        }
    }
    
    return legal_moves;
}
```

### Move Ordering & Scoring

Moves are automatically scored during generation for optimal search ordering:

```cpp
// Scoring system (higher = better)
Captures:     1,000,000 + (10 × victim_value) - attacker_value  // MVV-LVA
Promotions:   900,000 + promoted_piece_value
En Passant:   1,000,105  // Captures pawn (value 100) + bonus
Castling:     50,000     // Moderate priority
Quiet Moves:  0          // Base score
```

---

## Performance Optimizations

### 1. Incremental Updates

Instead of rebuilding the entire position after each move, the engine uses delta updates:

```cpp
void update_derived_state_for_move(const S_MOVE& m, Piece moving, Piece captured) {
    // Update only what changed
    if (moving_type == PieceType::King) {
        king_sq[moving_color] = m.get_to();
    }
    
    if (moving_type == PieceType::Pawn) {
        // Update pawn bitboards
        int from_64 = MAILBOX_MAPS.to64[m.get_from()];
        int to_64 = MAILBOX_MAPS.to64[m.get_to()];
        popBit(pawns_bb[moving_color], from_64);
        setBit(pawns_bb[moving_color], to_64);
    }
    
    // Handle captures, promotions, etc.
}
```

### 2. Bitboard Operations

Fast pawn-specific operations using 64-bit arithmetic:

```cpp
// Count pawns on a file
int pawns_on_file(const Position& pos, Color color, File file) {
    uint64_t file_mask = FILE_MASKS[int(file)];
    return popcount(pos.pawns_bb[int(color)] & file_mask);
}

// Check for passed pawns
bool is_passed_pawn(const Position& pos, Color color, int square) {
    uint64_t passed_mask = PASSED_PAWN_MASKS[int(color)][square];
    return !(pos.pawns_bb[int(!color)] & passed_mask);
}
```

### 3. Piece Lists

O(1) piece iteration instead of O(64) board scanning:

```cpp
// Iterate all white rooks
Color color = Color::White;
int rook_count = pos.pCount[int(color)][int(PieceType::Rook)];
for (int i = 0; i < rook_count; ++i) {
    int rook_square = pos.pList[int(color)][int(PieceType::Rook)][i];
    // Generate moves for this rook
}
```

### 4. Zobrist Hashing

Incremental hash updates for fast position comparison:

```cpp
void update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, 
                            uint8_t old_castling, int old_ep) {
    // XOR out old state
    zobrist_key ^= ZOBRIST_PIECES[int(moving)][m.get_from()];
    zobrist_key ^= ZOBRIST_SIDE_TO_MOVE;
    zobrist_key ^= ZOBRIST_CASTLING[old_castling];
    if (old_ep != -1) zobrist_key ^= ZOBRIST_EP[file_of_sq120(old_ep)];
    
    // XOR in new state  
    zobrist_key ^= ZOBRIST_PIECES[int(moving)][m.get_to()];
    if (captured != Piece::None) {
        zobrist_key ^= ZOBRIST_PIECES[int(captured)][m.get_to()];
    }
    zobrist_key ^= ZOBRIST_CASTLING[castling_rights];
    if (ep_square != -1) zobrist_key ^= ZOBRIST_EP[file_of_sq120(ep_square)];
}
```

---

## Key Data Structures

### Move History with Undo

```cpp
struct S_UNDO {
    S_MOVE move;                              // The move made
    uint8_t castling_rights;                  // Previous castling state
    int ep_square;                            // Previous en passant square
    uint16_t halfmove_clock;                  // Previous 50-move counter
    uint64_t zobrist_key;                     // Previous position hash
    Piece captured;                           // Captured piece (if any)
    
    // Derived state backups for fast undo
    std::array<int, 2> king_sq_backup;
    std::array<uint64_t, 2> pawns_bb_backup;
    std::array<int, 7> piece_counts_backup;
    std::array<int, 2> material_score_backup;
    PieceList pList_backup[2];
    int pCount_backup[2][7];
};
```

### Piece Lists

```cpp
using PieceList = std::array<std::array<int, MAX_PIECES_PER_TYPE>, int(PieceType::_Count)>;

// Access pattern:
// pList[color][piece_type][index] = square
// pCount[color][piece_type] = count

// Example: Get all white knights
for (int i = 0; i < pos.pCount[int(Color::White)][int(PieceType::Knight)]; ++i) {
    int knight_sq = pos.pList[int(Color::White)][int(PieceType::Knight)][i];
    // Process knight at knight_sq
}
```

---

## Code Examples

### Complete Move Generation Example

```cpp
#include "position.hpp"
#include "movegen_enhanced.hpp"

void analyze_position() {
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Generate all pseudo-legal moves
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Pseudo-legal moves: " << moves.count << std::endl;
    
    // Generate only legal moves
    S_MOVELIST legal = generate_legal_moves_enhanced(pos);
    std::cout << "Legal moves: " << legal.count << std::endl;
    
    // Analyze move types
    int quiet = 0, captures = 0, promotions = 0, castles = 0;
    for (int i = 0; i < legal.count; ++i) {
        const S_MOVE& move = legal.moves[i];
        if (move.is_castle()) castles++;
        else if (move.is_promotion()) promotions++;
        else if (move.is_capture()) captures++;
        else quiet++;
    }
    
    std::cout << "Quiet: " << quiet << ", Captures: " << captures 
              << ", Promotions: " << promotions << ", Castles: " << castles << std::endl;
}
```

### Position Analysis Example

```cpp
void analyze_position_structure(const Position& pos) {
    std::cout << "=== Position Analysis ===" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: " << std::bitset<4>(pos.castling_rights) << std::endl;
    std::cout << "En passant: " << (pos.ep_square == -1 ? "None" : std::to_string(pos.ep_square)) << std::endl;
    std::cout << "Halfmove clock: " << pos.halfmove_clock << std::endl;
    
    // Material count
    std::cout << "\n=== Material ===" << std::endl;
    for (int color = 0; color < 2; ++color) {
        std::cout << (color == 0 ? "White: " : "Black: ");
        for (int piece = 1; piece <= 6; ++piece) {
            std::cout << PIECE_CHARS[piece] << ":" << pos.pCount[color][piece] << " ";
        }
        std::cout << "Material: " << pos.material_score[color] << std::endl;
    }
    
    // King positions
    std::cout << "\n=== King Positions ===" << std::endl;
    std::cout << "White king: " << pos.king_sq[0] << std::endl;
    std::cout << "Black king: " << pos.king_sq[1] << std::endl;
    
    // Pawn structure
    std::cout << "\n=== Pawn Bitboards ===" << std::endl;
    std::cout << "White pawns: " << std::bitset<64>(pos.pawns_bb[0]) << std::endl;
    std::cout << "Black pawns: " << std::bitset<64>(pos.pawns_bb[1]) << std::endl;
    std::cout << "All pawns: " << std::bitset<64>(pos.all_pawns_bb) << std::endl;
}
```

---

## Summary

The Huginn chess engine's position and move generation system provides:

✅ **Efficient Representation**: 120-square mailbox with bitboard acceleration  
✅ **Compact Move Encoding**: 25-bit moves with automatic scoring  
✅ **High Performance**: Incremental updates, piece lists, and optimized generation  
✅ **Complete Functionality**: All chess rules including castling, en passant, promotion  
✅ **Robust Architecture**: Type-safe enums, bounds checking, and comprehensive undo support  

This hybrid approach delivers excellent performance while maintaining code clarity and correctness, making it suitable for both engine development and chess programming education.
