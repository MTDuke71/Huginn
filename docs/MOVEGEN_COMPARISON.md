# Board Representation & Move Generation: Huginn vs MTLChess

User observation (2026-05-05): in 5-min head-to-head games, MTLChess
plays **~2× faster effective NPS** and reaches **2-3 plies deeper per
move** than Huginn. Earlier startpos depth-11 benches showed both at
~2.0-2.3 Mnps, so the in-game gap suggests game-position move-gen and
make/unmake have larger relative cost than the startpos bench captures
— or eval/qsearch interaction is doing more work in Huginn.

This document compares the two engines side-by-side at the level of
data structures, move-gen flow, and per-move cost. It's a roadmap for
where the speed gap likely lives, not a measurement of Elo gap.

> **Status (updated 2026-06-01): partially superseded.** This is a
> 2026-05-05 snapshot. Since then, Phase 4.8b **removed the mailbox-120
> board and piece lists entirely** — squares are now pure 0–63 (a1=0,
> h8=63; see [square.hpp](../src/square.hpp)) and `S_MOVE` from/to fields
> carry sq64 values. So every "Hybrid 120/64 indexing" / "120-mailbox
> legacy" framing below is **historical** and the "migrate to 64-square /
> delete the 120-mailbox indirection" recommendations (§6 #1, §8 #5) are
> **DONE**. Note the proposed fix "add a 64-square mailbox to `Position`
> for direct `at()`" was tried as the `board64[64]` cache (BACKLOG #26)
> and **reverted** — the +64 B footprint cost as much as the scan saved.
> Still accurate as written: the `at_sq64()` bitboard-scan cost, the
> heap `std::vector<S_UNDO>` history vs MTL's stack array, the move-list
> footprint, and the eager-MVV-LVA points. Treat per-NPS estimates as
> dated.

---

## 1. Board representation

| Aspect | Huginn | MTLChess | Notes |
|---|---|---|---|
| **Square indexing** | ~~Hybrid: 120-mailbox API + 64-bitboard underneath~~ → **now pure 64-square** (a1=0, h8=63) since Phase 4.8b | Pure 64-square (`Square = u6`) | **Resolved.** The 120↔64 boundary crossing this row described no longer exists. `S_MOVE` from/to are sq64 (still stored in 7-bit fields — see Square width row). Was Tier 4 #18. |
| **Piece bitboards** | `piece_bitboards[2][7]` (color × type, with type::None unused) — 14 boards | `pieces[12]` (color×type fused) — 12 boards | Functionally equivalent. MTL's flat layout is one fewer dimension — slightly better cache locality but small. |
| **Occupancy** | `color_bitboards[2]` + `occupied_bitboard` — computed and stored as 3 fields | `occupancy[3]` — `[white, black, all]` array | Same 3 bitboards either way. MTL's array form lets `colorBB(c)` and `allBB()` index uniformly. |
| **Piece-on-square** | `at_sq64(s)` → bitboard scan to find piece | `mailbox[64]` → direct `Piece` lookup | **Still true.** Huginn's `at_sq64()` scans the per-type bitboards (~6–14 tests); MTL's is a single array load. Hot path: move-gen fills captured-piece type via `pos.at_sq64(...)`. The `board64[64]` cache fix (BACKLOG #26) was reverted as net-negative. |
| **King squares** | `king_sq[2]` (now sq64) | Computed on demand via `kingSquare(c)` (lsb of king bitboard) | Roughly equivalent. |
| **Move history** | `std::vector<S_UNDO> move_history;` (heap, dynamic resize) | `[512]IrreversibleState history;` (stack, fixed) | **Huginn pays heap-allocator cost** on first deep search; MTL's stack array has no allocation cost. Vector reserve helps but reallocation can still happen. |
| **State stored per ply** | castling, ep, halfmove, zobrist, captured, king_sq backup, material backup | castling, ep, halfmove, captured, hash, psqt_score, phase | MTL stores incremental eval state (psqt + phase) for unmake; Huginn re-runs eval each call. |
| **Incremental Zobrist** | yes (XOR-based) | yes (XOR-based) | Equivalent. |
| **Side-to-move** | `Color` enum, separate field | `Color` enum, separate field | Equivalent. |
| **Sentinel for off-board** | `Piece::Offboard` returned by `at()` for sq < 21 or > 98 | n/a — pure 64-square has no off-board concept | Huginn's hybrid carries a legacy bookkeeping cost. |

### Position size (rough)

| Engine | Bitboards | Mailbox | Other state | History | Total state per instance |
|---|---|---|---|---|---|
| Huginn | 14×8B = 112B | (none — `at()` derives) | ~32B | `vector<S_UNDO>` heap | ~145B + heap |
| MTLChess | 12×8B + 3×8B = 120B | 64×1B = 64B | ~32B | 512×~32B = 16KB | ~16.2KB inline |

MTL trades larger inline footprint for zero heap-alloc and faster mailbox lookups. Huginn trades smaller inline state for heap allocation and bitboard-scanning piece lookups.

---

## 2. Move encoding

| Aspect | Huginn `S_MOVE` | MTLChess `Move` | Notes |
|---|---|---|---|
| **Total size** | 8 bytes (int move + int score) | 2 bytes (u16 packed) | **4× difference.** Huginn's move list of 256 moves: 2 KB. MTL's: 512 B. Cache-line footprint matters in qsearch where move lists are allocated millions of times. |
| **Move bits** | 25 bits used: from(7) to(7) captured-type(4) ep(1) pawn-start(1) promoted-type(4) castle(1) | 16 bits: from(6) to(6) flags(4) | MTL's 4-bit flag enum (16 values: quiet, double_push, capture, ep, 2 castles, 4 promos, 4 promo-captures) collapses Huginn's 4 separate flag bits into a single discriminator. |
| **Score field** | int separate field | none — scored externally during ordering | Huginn pays score-load cost on every move list iteration; MTL loads score from a parallel array only when ordering. |
| **Score storage at gen time** | MVV-LVA computed inside `add_capture_move` (calls `pos.at(from)` to find attacker) | none — generator emits bare moves only | Huginn does eager scoring; MTL does lazy. **Lazy scoring is faster when pruning by SEE early** because the generator doesn't waste work on moves that get pruned. |
| **Square width** | 7-bit fields holding sq64 values (0-63) — 1 bit/field now wasted | 6-bit (64 squares, packed efficiently) | The field width is still 7-bit (`0x7F` mask in [move.hpp](../src/move.hpp)); the values migrated to 0-63 in Phase 4.8b but the field was never shrunk to 6-bit. Minor packing waste. |

---

## 3. Move generation flow

```
Huginn:
  AlphaBeta loop
    └─> generate_legal_moves(pos, list)        ◄── (1) wrapper
          └─> generate_all_moves(pos, pseudo)  ◄── (2) bitboard generator
                ├─> generate_pawn_moves_bitboard
                ├─> generate_knight_moves_bitboard
                ├─> generate_bishop_moves_bitboard
                ├─> generate_rook_moves_bitboard
                ├─> generate_queen_moves_bitboard
                ├─> generate_king_moves_bitboard
                └─> generate_castling_moves_optimized
          └─> for each pseudo move:
                 pos.MakeMove (legality test)  ◄── (3) DUPLICATED: search
                 pos.TakeMove                       loop will MakeMove
                 add to legal list                  again right after
    └─> for each legal move:
          pos.MakeMove                         ◄── (4) the real search
          ... search ...                            MakeMove
          pos.TakeMove
```

```
MTLChess:
  negamax loop
    └─> movegen.generateLegalMoves(board, list)
          └─> generatePseudoLegalMoves
                ├─> generatePawnMoves      (split on color via comptime)
                ├─> generatePieceMoves     (knight/bishop/rook/queen/king
                │                           in a single dispatch)
                └─> generateCastlingMoves
          └─> for each pseudo move:
                 board.makeMove
                 if !isSquareAttackedBy(king, them) → add to legal list
                 board.unmakeMove
    └─> for each legal move:
          board.makeMove                     ◄── one make/unmake pair
          ... search ...                          per node
          board.unmakeMove
```

### The duplicated-make-unmake problem

**Huginn's 2026-05-05 commit `6865379` removed the `Position` copy** but
kept the per-move legality filter. The filter still does an additional
make/unmake pair on the real `pos` for every legal move, then the
search loop does another make/unmake to actually search.

**MTLChess's `generateLegalMoves` does the same pattern** (line 15-31 of
`movegen.zig`): make → check king attack → unmake. So MTL pays this
cost too — except the per-move check uses `isSquareAttackedBy(king,
them)` (a single SqAttacked call) rather than relying on `MakeMove`'s
return value, which avoids the rollback path.

The difference is that **MTL's `negamax` calls `generateLegalMoves`,
not pseudo-legal**, so its search loop relies on legal-only output and
uses `moves_searched` as the legality witness. Huginn's loop has a
`if (MakeMove != 1) continue` guard that's redundant after a legal-only
filter has already run.

**Tracked as BACKLOG #14**: refactor Huginn's search loops to use
`generate_all_moves` (pseudo-legal) directly + track `legal_count` for
mate detection. The 2026-05-05 spike measured this at **35% faster**
at depth 11 startpos (4.09M nodes / 2.6s → 2.69M / 1.27s).

---

## 4. Per-piece move generation

### Pawn moves

| Aspect | Huginn | MTLChess |
|---|---|---|
| Color split | `if (us == White)` / `else` branches in single function | Separate generic functions specialized by color via `comptime is_white: bool` |
| Single push | `(pawns << 8) & ~occupied` | `bb.shiftNorth(pawns) & empty` |
| Double push | second shift + rank-3 mask | second shift + rank-3 mask |
| Captures | shift NW/NE & enemy occupancy | shift NW/NE & enemy occupancy |
| Promotions | rank-8 mask, emit 4 promo moves | rank-8 mask, emit 4 promo moves |
| EP | iterate enemy pawn squares, check ep target | check ep_square from board state, emit if matching |

Functionally similar. MTL's comptime specialization may give the compiler more inlining/vectorization opportunities.

### Slider moves (bishop / rook / queen)

| Aspect | Huginn | MTLChess |
|---|---|---|
| Method | Magic bitboards (`bishop_attacks(sq, occ)`, `rook_attacks(sq, occ)`) | Magic bitboards (`bishopAttacks(sq, occ)`, `rookAttacks(sq, occ)`) |
| Implementation | `src/bitboard.cpp` lines 145-156 | `src/attacks.zig` magics with comptime init |
| Queen | `queen_attacks = bishop_attacks \| rook_attacks` | `queenAttacks = bishopAttacks \| rookAttacks` |

Magic-bitboard implementations are equivalent. MTL's tables are computed at compile time via Zig's `comptime`; Huginn's are initialized at startup. Runtime-equivalent after init.

### Knight & king moves

Both engines use precomputed `[64]u64` attack tables. Identical.

---

## 5. Make/Unmake mechanics

### Huginn `MakeMove`

```cpp
int Position::MakeMove(const S_MOVE& move) {
    // resize history vector if needed (heap path)
    if (ply >= move_history.size()) move_history.resize(ply + 1);
    S_UNDO& undo = move_history[ply];
    // capture state into undo (zobrist, castling, ep, captured)
    // dispatch on flag bits: en passant / castling / promotion
    //   move_piece (bitboards + zobrist)
    //   handle special: rook move on castle, pawn capture on ep, etc.
    // increment ply, flip side
    update_zobrist_for_move(...)
    // POST-CONDITION: check king attack, undo via TakeMove if illegal,
    //                 return 0; otherwise return 1
}
```

### MTLChess `makeMove`

```zig
pub fn makeMove(self: *Board, move: Move) void {
    // store IrreversibleState into history[history_ply++]
    // dispatch on Move.flags (16-value enum, single switch)
    //   .quiet:           movePiece(p, from, to)
    //   .capture:         removePiece(captured, to); movePiece(p, from, to)
    //   .ep_capture:      removePiece(enemy_pawn, ep_target); movePiece(p, from, to)
    //   .king_castle:     movePiece(king, e->g); movePiece(rook, h->f)
    //   .promo*:          removePiece(pawn, from); putPiece(promo, to)
    //   .promo_capture*:  removePiece(captured, to); removePiece(pawn, from); putPiece(promo, to)
    // update castling rights via castling_mask[from] & castling_mask[to]
    // flip side, hash side, increment fullmove if black moved
    // (no legality post-check — caller does it)
}
```

### Differences

- **Single switch on flag enum** (MTL) vs **multiple bit-flag tests** (Huginn). MTL's compiler-generated jump table is faster than Huginn's branching.
- **No legality rollback in MTL's `makeMove`** — caller is responsible. Huginn's `MakeMove` returns 0 and self-undoes via `TakeMove` on illegal moves; that path adds branches and is taken often (every time pseudo-legal filter runs).
- **MTL's castling rights update is a single `andMask`** with precomputed `castling_mask[64]` table. Huginn's is a series of conditional clears.
- **MTL's history stack is fixed-size**; no resize check. Huginn's `move_history.resize(ply + 1)` is a hot-path branch that only matters on first deep search but always pays the comparison.

---

## 6. Where the gap likely lives

User observation: MTLChess ~2× NPS in real games, 2-3 plies deeper.

Probable sources, in order of likely impact:

| # | Source | Estimated NPS impact | How to address |
|---|---|---|---|
| 1 | **`at_sq64()` is a bitboard scan** (the 120/64 boundary-crossing half of this is now gone — sq64 migration shipped in Phase 4.8b) | residual only | sq64 migration **DONE**. The remaining fix — a `board64[64]` cache for direct `at()` — was tried (BACKLOG #26) and **reverted** as net-negative. Scan kept. |
| 2 | **Search uses legal-only generation, double-make/unmake per move** | 7-35% (already partially shipped at 7-10%; surgical version measured 35% in spike) | Backlog #14 — search loops use pseudo-legal directly. |
| 3 | **`MakeMove` flag dispatch** — multiple bit tests vs single switch | 5-15% | Migrate move flags from independent bits to a 4-bit enum tag (matches MTL's design). |
| 4 | **`std::vector<S_UNDO>` history with heap path** | 2-10% | Replace with `S_UNDO history[MAX_PLY]` stack array. |
| 5 | **MoveList footprint** — 8B `S_MOVE` × 256 = 2 KB list, vs MTL's 512 B | 3-8% | Drop the `score` field from `S_MOVE`; score in a parallel array indexed by move-list position only when ordering. |
| 6 | **Eager MVV-LVA scoring at gen time** | 2-5% | Lazy scoring (matches MTL): generator emits bare moves, scoring done in `pick_next_move` on first call. |

Compounding these, a realistic ceiling for "match MTL's per-node speed" is **~50-70% NPS gain** if all six are addressed. The first two alone account for the bulk.

---

## 7. What this comparison does NOT explain

- **Eval cost difference.** Huginn's eval includes mobility + several pawn-structure terms; MTL's is PSQT + bishop pair + simple king safety. Huginn's eval is more expensive per call. Not a board-rep / move-gen issue.
- **Search efficiency** (tree shape per depth). The earlier session diagnosis showed huginn at ~20× MTL's nodes-per-depth on tactical positions. That's a search-quality issue (LMR tuning, move ordering quality, deferred features #1/#2/#7/#8) not a per-node-speed issue.
- **The 2-3 plies deeper observation** is partly NPS (this doc) and partly tree-shape (BACKLOG #13 chain). Both compound.

---

## 8. Recommended action ordering

If the goal is to close the per-node speed gap (NOT the search-shape gap):

1. **Ship BACKLOG #14** — surgical pseudo-legal generation in search loops. Bench-validated 35% gain. Already-warm context.
2. **Migrate `S_MOVE` flag bits to a 4-bit enum tag.** Replaces ~5 independent bit checks in `MakeMove` with a single switch. Touches every move site but is mechanical.
3. **Replace `std::vector<S_UNDO>` history with a fixed-size stack array** (`S_UNDO history[MAX_PLY]`). Removes heap path. ~half-day refactor.
4. **Drop the `score` field from `S_MOVE`** (parallel score array in `S_MOVELIST`). Halves move-list footprint. ~half-day refactor.
5. ~~**Migrate to 64-square mailbox** (delete the 120-mailbox indirection). Tier 4 #18.~~ **DONE in Phase 4.8b** — the 120-mailbox was removed and squares are pure sq64.
6. **Lazy MVV-LVA scoring.** Tiny but compounds. ~few hours.

Items 1-4 together would close most of the per-node gap with bounded risk. (#5, the 120-mailbox removal that these were meant to unblock, has since shipped independently in Phase 4.8b.)
