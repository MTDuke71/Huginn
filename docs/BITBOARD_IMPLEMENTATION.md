# Bitboard Implementation

Bitboards are the **primary** piece representation in Huginn.
`Position` stores piece locations exclusively in
`piece_bitboards[Color][PieceType]` plus per-color and occupied
aggregates; the 120-square indexing scheme is retained for square
*coordinates* (king location, en-passant target, direction offsets)
but the legacy `board[120]` piece array, `pawns_bb` field, and
`pList`/`pCount` piece lists have all been removed.

This file documents the bitboard primitives in
[`src/bitboard.hpp`](../src/bitboard.hpp). See
[POSITION_AND_MOVEGEN_ARCHITECTURE.md](POSITION_AND_MOVEGEN_ARCHITECTURE.md)
for how `Position` uses them.

## Type and bit-manipulation macros

```cpp
using Bitboard = uint64_t;

#define setBit(bb, sq)   ((bb) |= (1ULL << (sq)))     // Set bit
#define popBit(bb, sq)   ((bb) &= ~(1ULL << (sq)))    // Clear bit
#define getBit(bb, sq)   ((bb) & (1ULL << (sq)))      // Check bit
```

## Pre-defined constants

```cpp
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;

constexpr Bitboard FILE_BB[8] = { FILE_A, FILE_B, ..., FILE_H };
constexpr Bitboard RANK_BB[8] = { RANK_1, RANK_2, ..., RANK_8 };
```

## Visualization

`printBitboard()` produces ASCII visualization useful when debugging
movegen or attack-table issues:

```
    a b c d e f g h
  +---------------+
8 | x - - - - - - x | 8
7 | - - - - - - - - | 7
6 | - - - - - - - - | 6
5 | - - - x x - - - | 5
4 | - - - x x - - - | 4
3 | - - - - - - - - | 3
2 | - - - - - - - - | 2
1 | x - - - - - - x | 1
  +---------------+
    a b c d e f g h
```

## Square indexing systems

- **64-square indexing**: `a1=0, b1=1, ..., h8=63`. This is the
  layout the bitboards use directly — `(1ULL << sq64)` is the bit
  for square `sq64`.
- **120-square indexing**: `a1=21, b1=22, ..., h8=98`. Used for
  square coordinates with sentinel-border arithmetic.
- **Conversion**: `MAILBOX_MAPS.to64[sq120]` and `MAILBOX_MAPS.to120[sq64]`.
  See [`src/board120.hpp`](../src/board120.hpp).

## Utility functions

```cpp
int  popcount(Bitboard bb);          // Count set bits
int  get_lsb(Bitboard bb);           // Index of least significant bit
int  pop_lsb(Bitboard& bb);          // Pop and return LSB
bool is_empty(Bitboard bb);
bool is_set(Bitboard bb, int sq64);
```

`pop_lsb` is the standard primitive for iterating pieces of a given
type/color: pull the bitboard for that combination, then loop while
non-zero, popping bits to get each square.

## How `Position` uses these

```cpp
// Iterate all white knights (current style — bitboard pop loop)
Bitboard knights = pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)];
while (knights) {
    int sq64 = pop_lsb(knights);
    int sq120 = MAILBOX_MAPS.to120[sq64];
    // ... generate moves for knight at sq120 ...
}
```

`Position::at(sq120)` derives the piece at a square by checking
which `piece_bitboards[*][*]` slot has the corresponding bit set,
or returns `Piece::None` if no slot does (and `Piece::Offboard` for
sentinel squares).

## Where to look

- Bitboard primitives, constants, popcount/LSB: [`src/bitboard.hpp`](../src/bitboard.hpp).
- Attack tables (sliding rays, knight/king attack masks): [`src/attack_tables.cpp`](../src/attack_tables.cpp).
- Bitboard movegen (per piece type): [`src/movegen_bb.cpp`](../src/movegen_bb.cpp).
- Position struct using bitboards as source of truth: [`src/position.hpp`](../src/position.hpp).
