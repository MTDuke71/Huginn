What S_BOARD (board_state) has in addition:
Piece counters:
pceNum[PIECE_NB]: count of each piece type (e.g., number of white pawns, black knights, etc.)
bigPce[2]: count of non-pawn pieces per side
majPce[2]: count of major pieces (rooks + queens) per side
minPce[2]: count of minor pieces (knights + bishops) per side
Pawn bitboards:
pawns[3]: bitboards for pawns (WHITE, BLACK, BOTH) for fast pawn operations
King locations:
KingSq[2]: location of each king (in 120-square indexing)
Zobrist hash:
posKey: position hash for fast position comparison and transposition tables
Move counters:
ply, hisPly: search depth and history ply (for search algorithms)
Classic enums/constants:
Piece codes, side codes, castling masks, etc.
What to carry over to Position:
If you want all the extra info, you should add the following members to your Position struct:

std::array<int, PIECE_NB> pceNum; // piece counts
std::array<int, 2> bigPce, majPce, minPce; // piece type counters
std::array<U64, 3> pawns; // pawn bitboards
std::array<int, 2> KingSq; // king locations
U64 posKey; // Zobrist hash
int ply, hisPly; // search/history counters
You will also need to implement or port the logic for updating these fields (see rebuild_counts() in board_state.hpp), and update them whenever the board changes.

Summary
To get all the extra features from board_state, add the above fields to Position and ensure you update them as needed. This will give you fast access to piece counts, pawn bitboards, king locations, and position hashing, which are useful for move generation, evaluation, and search.

Is it a lot of overhead (time) to update all of these structures

Updating all these extra structures (piece counters, pawn bitboards, king locations, etc.) does add some overhead, but in practice it is minimal compared to the benefits. The update is typically done in a single pass over the board array (e.g., in a function like rebuild_counts()), which is O(1) for chess (since the board size is fixed and small).

The extra time spent is usually negligible, especially since these structures allow much faster move generation, evaluation, and search. Most modern chess engines use similar bookkeeping for performance reasons. The only time you might notice overhead is if you rebuild everything after every move, but with incremental updates (only changing affected counters when a move is made/unmade), the cost is very low.

In summary:

Full rebuild: very fast (single pass over 64 squares)
Incremental update: even faster (only a few fields change per move)
The speed gain in other parts of the engine (movegen, eval) far outweighs the small update cost.
