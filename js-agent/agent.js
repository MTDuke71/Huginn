#!/usr/bin/env node
'use strict';
/*
 * Single-file UCI-move chess agent for a sandboxed competition.
 *
 * Protocol: read one line from stdin per move — a FEN, optionally followed
 * by " moves <uci...>" (the full game history) — and write one UCI move
 * ("e2e4", "e7e8q") to stdout, then wait for the next line.
 *
 * No dependencies beyond 'readline'. No fs, no child_process, no network.
 * Board: 0x88 mailbox. Search: iterative-deepening negamax/alpha-beta with
 * a Zobrist-keyed transposition table, quiescence search, and a standard
 * pruning stack (null-move, late-move reductions, MOVE-level futility).
 *
 * Design notes (ported lessons, not code, from a sibling C++ project):
 *  - Futility pruning is implemented at MOVE level (skip only the quiet,
 *    non-checking move), never at NODE level (bailing the whole node before
 *    the move loop also skips captures/checks — a latent correctness bug
 *    class, not just an efficiency one).
 *  - A winning root move that would recreate an already-seen position
 *    (single repetition, not just the rule threefold) is treated with
 *    suspicion — the spec's own warning ("you can draw a won game by
 *    accident") is a real, previously-hit bug class, not a hypothetical.
 */

// ===================================================================
// Constants & board representation (0x88)
// ===================================================================

const EMPTY = 0;
const PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
const WHITE = 1, BLACK = -1;

const FILE_OF = (sq) => sq & 7;
const RANK_OF = (sq) => sq >> 4;
const SQ = (file, rank) => (rank << 4) | file;
const OFFBOARD = (sq) => (sq & 0x88) !== 0;
const SQ64 = (sq) => (RANK_OF(sq) << 3) | FILE_OF(sq); // 0x88 -> 0..63 for PST/eval lookups

const FILE_NAMES = 'abcdefgh';
function sqName(sq) { return FILE_NAMES[FILE_OF(sq)] + (RANK_OF(sq) + 1); }
function sqFromName(s) {
  const f = s.charCodeAt(0) - 97;
  const r = s.charCodeAt(1) - 49;
  return SQ(f, r);
}

const KNIGHT_OFFSETS = [-33, -31, -18, -14, 14, 18, 31, 33];
const KING_OFFSETS = [-17, -16, -15, -1, 1, 15, 16, 17];
const BISHOP_DIRS = [-17, -15, 15, 17];
const ROOK_DIRS = [-16, -1, 1, 16];
const QUEEN_DIRS = BISHOP_DIRS.concat(ROOK_DIRS);

// Texel-tuned material values, seeded from Huginn (src/chess_types.hpp).
const PIECE_VALUES_MG = [0, 80, 397, 392, 542, 1231, 20000];
const PIECE_VALUES_EG = [0, 93, 288, 304, 566, 1072, 20000];
// Phase weight per piece type (knight/bishop=1, rook=2, queen=4), summed and
// scaled to 0..256 (256 = full opening material, 0 = bare kings) — same
// shape as Huginn's game_phase_256().
const PHASE_WEIGHT = [0, 0, 1, 1, 2, 4, 0];
const PHASE_TOTAL = 24; // 4N+4B+4R+2Q worth of weight at the start position

// Castling right bits
const WK_CASTLE = 1, WQ_CASTLE = 2, BK_CASTLE = 4, BQ_CASTLE = 8;

// Move flags
const FLAG_CAPTURE = 1;
const FLAG_EP = 2;
const FLAG_CASTLE_K = 4;
const FLAG_CASTLE_Q = 8;
const FLAG_DOUBLE = 16;

// ===================================================================
// Piece-square tables (generated, not hand-transcribed — see header note)
// ===================================================================

function buildPST() {
  const pst = {}; // pst[pieceType].mg / .eg -> Int16Array(64), white POV, a1=0..h8=63
  for (const pt of [PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING]) {
    pst[pt] = { mg: new Int16Array(64), eg: new Int16Array(64) };
  }
  for (let rank = 0; rank < 8; rank++) {
    for (let file = 0; file < 8; file++) {
      const i = rank * 8 + file;
      const centerFileDist = Math.abs(file - 3.5); // 0.5..3.5
      const centerRankDist = Math.abs(rank - 3.5);
      const centerDist = centerFileDist + centerRankDist; // Manhattan, 1..7

      // Pawns: reward central files and advancing; EG rewards advancing more.
      const pawnCenterBonus = Math.round((3.5 - centerFileDist) * 4);
      pst[PAWN].mg[i] = pawnCenterBonus + rank * 2;
      pst[PAWN].eg[i] = pawnCenterBonus + rank * 10;

      // Knights/Bishops/Queen: centralization, mild.
      const central = Math.round((7 - centerDist) * 4);
      pst[KNIGHT].mg[i] = central;
      pst[KNIGHT].eg[i] = central;
      pst[BISHOP].mg[i] = Math.round(central * 0.6);
      pst[BISHOP].eg[i] = Math.round(central * 0.6);
      pst[QUEEN].mg[i] = Math.round(central * 0.3);
      pst[QUEEN].eg[i] = Math.round(central * 0.5);

      // Rooks: 7th-rank + central-file bonus (open-file handled in eval()).
      pst[ROOK].mg[i] = (rank === 6 ? 15 : 0) + Math.round((3.5 - centerFileDist) * 2);
      pst[ROOK].eg[i] = Math.round((3.5 - centerFileDist) * 2);

      // King: MG wants the corners/back rank (castled safety); EG wants the
      // center (king becomes a fighting piece).
      pst[KING].mg[i] = Math.round((centerDist - 3.5) * 8) - rank * 6;
      pst[KING].eg[i] = Math.round((7 - centerDist) * 6);
    }
  }
  return pst;
}
const PST = buildPST();

// ===================================================================
// Zobrist hashing — two independent 32-bit lanes (index + verification
// lock), combined with XOR only, never > 32-bit ops, so it stays fast and
// allocation-free in the search hot path.
// ===================================================================

function rand32() { return (Math.random() * 0x100000000) >>> 0; }

const ZOBRIST_PIECE_A = new Uint32Array(13 * 128); // index [ (piece+6)*128 + sq ]
const ZOBRIST_PIECE_B = new Uint32Array(13 * 128);
const ZOBRIST_SIDE_A = rand32(), ZOBRIST_SIDE_B = rand32();
const ZOBRIST_CASTLE_A = new Uint32Array(16), ZOBRIST_CASTLE_B = new Uint32Array(16);
const ZOBRIST_EP_A = new Uint32Array(8), ZOBRIST_EP_B = new Uint32Array(8);
for (let i = 0; i < ZOBRIST_PIECE_A.length; i++) { ZOBRIST_PIECE_A[i] = rand32(); ZOBRIST_PIECE_B[i] = rand32(); }
for (let i = 0; i < 16; i++) { ZOBRIST_CASTLE_A[i] = rand32(); ZOBRIST_CASTLE_B[i] = rand32(); }
for (let i = 0; i < 8; i++) { ZOBRIST_EP_A[i] = rand32(); ZOBRIST_EP_B[i] = rand32(); }
function pieceZobristIndex(piece, sq) { return (piece + 6) * 128 + sq; }

// ===================================================================
// Position state
// ===================================================================

function newPosition() {
  return {
    board: new Int8Array(128),      // 0x88 mailbox, piece = color*type
    side: WHITE,
    castling: 0,
    epSquare: -1,
    halfmoveClock: 0,
    fullmoveNumber: 1,
    kingSq: { [WHITE]: -1, [BLACK]: -1 },
    keyA: 0, keyB: 0,
    history: [],                    // Zobrist keyA per ply, for repetition detection
    stateStack: [],                 // make/unmake undo records
  };
}

function computeZobrist(pos) {
  let a = 0, b = 0;
  for (let sq = 0; sq < 128; sq++) {
    if (OFFBOARD(sq)) continue;
    const p = pos.board[sq];
    if (p !== EMPTY) {
      const idx = pieceZobristIndex(p, sq);
      a ^= ZOBRIST_PIECE_A[idx]; b ^= ZOBRIST_PIECE_B[idx];
    }
  }
  if (pos.side === BLACK) { a ^= ZOBRIST_SIDE_A; b ^= ZOBRIST_SIDE_B; }
  a ^= ZOBRIST_CASTLE_A[pos.castling]; b ^= ZOBRIST_CASTLE_B[pos.castling];
  if (pos.epSquare !== -1) { a ^= ZOBRIST_EP_A[FILE_OF(pos.epSquare)]; b ^= ZOBRIST_EP_B[FILE_OF(pos.epSquare)]; }
  return { a: a >>> 0, b: b >>> 0 };
}

function setPiece(pos, sq, piece) {
  const old = pos.board[sq];
  if (old !== EMPTY) {
    const idx = pieceZobristIndex(old, sq);
    pos.keyA ^= ZOBRIST_PIECE_A[idx]; pos.keyB ^= ZOBRIST_PIECE_B[idx];
  }
  pos.board[sq] = piece;
  if (piece !== EMPTY) {
    const idx = pieceZobristIndex(piece, sq);
    pos.keyA ^= ZOBRIST_PIECE_A[idx]; pos.keyB ^= ZOBRIST_PIECE_B[idx];
    if (Math.abs(piece) === KING) pos.kingSq[Math.sign(piece)] = sq;
  }
}

// ===================================================================
// FEN parsing
// ===================================================================

function parseFEN(fen) {
  const pos = newPosition();
  const parts = fen.trim().split(/\s+/);
  const [placement, sideChar, castlingStr, epStr, halfmoveStr, fullmoveStr] = parts;

  const rows = placement.split('/'); // rows[0] = rank8 .. rows[7] = rank1
  for (let r = 0; r < 8; r++) {
    const rank = 7 - r;
    let file = 0;
    for (const ch of rows[r]) {
      if (ch >= '1' && ch <= '8') {
        file += Number(ch);
      } else {
        const type = { p: PAWN, n: KNIGHT, b: BISHOP, r: ROOK, q: QUEEN, k: KING }[ch.toLowerCase()];
        const color = ch === ch.toUpperCase() ? WHITE : BLACK;
        const sq = SQ(file, rank);
        pos.board[sq] = color * type;
        if (type === KING) pos.kingSq[color] = sq;
        file++;
      }
    }
  }

  pos.side = sideChar === 'w' ? WHITE : BLACK;

  pos.castling = 0;
  if (castlingStr && castlingStr !== '-') {
    if (castlingStr.includes('K')) pos.castling |= WK_CASTLE;
    if (castlingStr.includes('Q')) pos.castling |= WQ_CASTLE;
    if (castlingStr.includes('k')) pos.castling |= BK_CASTLE;
    if (castlingStr.includes('q')) pos.castling |= BQ_CASTLE;
  }

  pos.epSquare = (epStr && epStr !== '-') ? sqFromName(epStr) : -1;
  pos.halfmoveClock = halfmoveStr ? Number(halfmoveStr) : 0;
  pos.fullmoveNumber = fullmoveStr ? Number(fullmoveStr) : 1;

  const { a, b } = computeZobrist(pos);
  pos.keyA = a; pos.keyB = b;
  pos.history = [pos.keyA];
  return pos;
}

// ===================================================================
// Attack detection
// ===================================================================

function isSquareAttacked(pos, sq, byColor) {
  const board = pos.board;
  // Pawn attacks (byColor's pawns attacking `sq`)
  const pawnDir = byColor === WHITE ? -16 : 16; // squares pawns of byColor would come FROM
  for (const df of [-1, 1]) {
    const from = sq + pawnDir + df;
    if (!OFFBOARD(from) && board[from] === byColor * PAWN) return true;
  }
  // Knights
  for (const off of KNIGHT_OFFSETS) {
    const from = sq + off;
    if (!OFFBOARD(from) && board[from] === byColor * KNIGHT) return true;
  }
  // King
  for (const off of KING_OFFSETS) {
    const from = sq + off;
    if (!OFFBOARD(from) && board[from] === byColor * KING) return true;
  }
  // Sliders: bishops/queens on diagonals, rooks/queens on files/ranks
  for (const dir of BISHOP_DIRS) {
    let s = sq + dir;
    while (!OFFBOARD(s)) {
      const p = board[s];
      if (p !== EMPTY) {
        if (p === byColor * BISHOP || p === byColor * QUEEN) return true;
        break;
      }
      s += dir;
    }
  }
  for (const dir of ROOK_DIRS) {
    let s = sq + dir;
    while (!OFFBOARD(s)) {
      const p = board[s];
      if (p !== EMPTY) {
        if (p === byColor * ROOK || p === byColor * QUEEN) return true;
        break;
      }
      s += dir;
    }
  }
  return false;
}

function inCheck(pos, color) {
  return isSquareAttacked(pos, pos.kingSq[color], -color);
}

// ===================================================================
// Move generation (pseudo-legal). Moves are plain objects for clarity;
// only the transposition table stores a packed-integer encoding (see
// encodeMoveInt / matchIntMove below).
// ===================================================================

function addMove(list, from, to, promotion, flags, board) {
  list.push({ from, to, promotion: promotion || 0, flags: flags || 0, captured: board[to] });
}

function generatePseudoMoves(pos) {
  const board = pos.board;
  const us = pos.side;
  const them = -us;
  const moves = [];

  for (let sq = 0; sq < 128; sq++) {
    if (OFFBOARD(sq)) continue;
    const piece = board[sq];
    if (piece === EMPTY || Math.sign(piece) !== us) continue;
    const type = Math.abs(piece);

    if (type === PAWN) {
      const forward = us === WHITE ? 16 : -16;
      const startRank = us === WHITE ? 1 : 6;
      const promoRank = us === WHITE ? 7 : 0;
      const one = sq + forward;
      if (!OFFBOARD(one) && board[one] === EMPTY) {
        if (RANK_OF(one) === promoRank) {
          for (const promo of [QUEEN, ROOK, BISHOP, KNIGHT]) addMove(moves, sq, one, promo, 0, board);
        } else {
          addMove(moves, sq, one, 0, 0, board);
          if (RANK_OF(sq) === startRank) {
            const two = sq + 2 * forward;
            if (board[two] === EMPTY) addMove(moves, sq, two, 0, FLAG_DOUBLE, board);
          }
        }
      }
      for (const df of [-1, 1]) {
        const to = sq + forward + df;
        if (OFFBOARD(to)) continue;
        if (board[to] !== EMPTY && Math.sign(board[to]) === them) {
          if (RANK_OF(to) === promoRank) {
            for (const promo of [QUEEN, ROOK, BISHOP, KNIGHT]) addMove(moves, sq, to, promo, FLAG_CAPTURE, board);
          } else {
            addMove(moves, sq, to, 0, FLAG_CAPTURE, board);
          }
        } else if (to === pos.epSquare) {
          addMove(moves, sq, to, 0, FLAG_CAPTURE | FLAG_EP, board);
        }
      }
    } else if (type === KNIGHT) {
      for (const off of KNIGHT_OFFSETS) {
        const to = sq + off;
        if (OFFBOARD(to)) continue;
        if (board[to] === EMPTY) addMove(moves, sq, to, 0, 0, board);
        else if (Math.sign(board[to]) === them) addMove(moves, sq, to, 0, FLAG_CAPTURE, board);
      }
    } else if (type === KING) {
      for (const off of KING_OFFSETS) {
        const to = sq + off;
        if (OFFBOARD(to)) continue;
        if (board[to] === EMPTY) addMove(moves, sq, to, 0, 0, board);
        else if (Math.sign(board[to]) === them) addMove(moves, sq, to, 0, FLAG_CAPTURE, board);
      }
      // Castling
      if (us === WHITE) {
        if ((pos.castling & WK_CASTLE) && board[SQ(5, 0)] === EMPTY && board[SQ(6, 0)] === EMPTY &&
            !isSquareAttacked(pos, SQ(4, 0), them) && !isSquareAttacked(pos, SQ(5, 0), them) && !isSquareAttacked(pos, SQ(6, 0), them)) {
          addMove(moves, SQ(4, 0), SQ(6, 0), 0, FLAG_CASTLE_K, board);
        }
        if ((pos.castling & WQ_CASTLE) && board[SQ(3, 0)] === EMPTY && board[SQ(2, 0)] === EMPTY && board[SQ(1, 0)] === EMPTY &&
            !isSquareAttacked(pos, SQ(4, 0), them) && !isSquareAttacked(pos, SQ(3, 0), them) && !isSquareAttacked(pos, SQ(2, 0), them)) {
          addMove(moves, SQ(4, 0), SQ(2, 0), 0, FLAG_CASTLE_Q, board);
        }
      } else {
        if ((pos.castling & BK_CASTLE) && board[SQ(5, 7)] === EMPTY && board[SQ(6, 7)] === EMPTY &&
            !isSquareAttacked(pos, SQ(4, 7), them) && !isSquareAttacked(pos, SQ(5, 7), them) && !isSquareAttacked(pos, SQ(6, 7), them)) {
          addMove(moves, SQ(4, 7), SQ(6, 7), 0, FLAG_CASTLE_K, board);
        }
        if ((pos.castling & BQ_CASTLE) && board[SQ(3, 7)] === EMPTY && board[SQ(2, 7)] === EMPTY && board[SQ(1, 7)] === EMPTY &&
            !isSquareAttacked(pos, SQ(4, 7), them) && !isSquareAttacked(pos, SQ(3, 7), them) && !isSquareAttacked(pos, SQ(2, 7), them)) {
          addMove(moves, SQ(4, 7), SQ(2, 7), 0, FLAG_CASTLE_Q, board);
        }
      }
    } else {
      const dirs = type === BISHOP ? BISHOP_DIRS : type === ROOK ? ROOK_DIRS : QUEEN_DIRS;
      for (const dir of dirs) {
        let to = sq + dir;
        while (!OFFBOARD(to)) {
          if (board[to] === EMPTY) {
            addMove(moves, sq, to, 0, 0, board);
          } else {
            if (Math.sign(board[to]) === them) addMove(moves, sq, to, 0, FLAG_CAPTURE, board);
            break;
          }
          to += dir;
        }
      }
    }
  }
  return moves;
}

function generateCaptures(pos) {
  return generatePseudoMoves(pos).filter(m => (m.flags & FLAG_CAPTURE) !== 0);
}

// ===================================================================
// Make / unmake move
// ===================================================================

function makeMove(pos, m) {
  const board = pos.board;
  const us = pos.side;
  const them = -us;
  const piece = board[m.from];
  const type = Math.abs(piece);

  pos.stateStack.push({
    castling: pos.castling, epSquare: pos.epSquare, halfmoveClock: pos.halfmoveClock,
    keyA: pos.keyA, keyB: pos.keyB, capturedPiece: m.captured, epCapturedSquare: -1,
  });

  const prevEpFile = pos.epSquare !== -1 ? FILE_OF(pos.epSquare) : -1;

  // En-passant capture removes a pawn NOT on the destination square.
  if (m.flags & FLAG_EP) {
    const capSq = m.to + (us === WHITE ? -16 : 16);
    pos.stateStack[pos.stateStack.length - 1].epCapturedSquare = capSq;
    pos.stateStack[pos.stateStack.length - 1].capturedPiece = board[capSq];
    setPiece(pos, capSq, EMPTY);
  }

  setPiece(pos, m.from, EMPTY);
  setPiece(pos, m.to, m.promotion ? us * m.promotion : piece);

  // Castling: also move the rook.
  if (m.flags & FLAG_CASTLE_K) {
    const rank = us === WHITE ? 0 : 7;
    setPiece(pos, SQ(7, rank), EMPTY);
    setPiece(pos, SQ(5, rank), us * ROOK);
  } else if (m.flags & FLAG_CASTLE_Q) {
    const rank = us === WHITE ? 0 : 7;
    setPiece(pos, SQ(0, rank), EMPTY);
    setPiece(pos, SQ(3, rank), us * ROOK);
  }

  // Castling rights: king move clears both; rook move/capture clears one;
  // touching a corner (from OR to) always clears that corner's right.
  let newCastling = pos.castling;
  if (type === KING) newCastling &= us === WHITE ? ~(WK_CASTLE | WQ_CASTLE) : ~(BK_CASTLE | BQ_CASTLE);
  const corner = (sq) => {
    if (sq === SQ(0, 0)) newCastling &= ~WQ_CASTLE;
    else if (sq === SQ(7, 0)) newCastling &= ~WK_CASTLE;
    else if (sq === SQ(0, 7)) newCastling &= ~BQ_CASTLE;
    else if (sq === SQ(7, 7)) newCastling &= ~BK_CASTLE;
  };
  corner(m.from); corner(m.to);
  pos.keyA ^= ZOBRIST_CASTLE_A[pos.castling]; pos.keyB ^= ZOBRIST_CASTLE_B[pos.castling];
  pos.castling = newCastling;
  pos.keyA ^= ZOBRIST_CASTLE_A[pos.castling]; pos.keyB ^= ZOBRIST_CASTLE_B[pos.castling];

  // En-passant target square
  if (prevEpFile !== -1) { pos.keyA ^= ZOBRIST_EP_A[prevEpFile]; pos.keyB ^= ZOBRIST_EP_B[prevEpFile]; }
  pos.epSquare = (m.flags & FLAG_DOUBLE) ? (m.from + m.to) / 2 : -1;
  if (pos.epSquare !== -1) { const f = FILE_OF(pos.epSquare); pos.keyA ^= ZOBRIST_EP_A[f]; pos.keyB ^= ZOBRIST_EP_B[f]; }

  pos.halfmoveClock = (type === PAWN || (m.flags & FLAG_CAPTURE)) ? 0 : pos.halfmoveClock + 1;
  if (us === BLACK) pos.fullmoveNumber++;

  pos.side = them;
  pos.keyA ^= ZOBRIST_SIDE_A; pos.keyB ^= ZOBRIST_SIDE_B;

  pos.history.push(pos.keyA);
}

function unmakeMove(pos, m) {
  const them = pos.side;   // side that is about to move again (the mover)
  const us = -them;        // side whose move we are undoing
  pos.side = us;
  pos.history.pop();

  const st = pos.stateStack.pop();
  pos.castling = st.castling;
  pos.epSquare = st.epSquare;
  pos.halfmoveClock = st.halfmoveClock;
  pos.keyA = st.keyA; pos.keyB = st.keyB;
  if (us === BLACK) pos.fullmoveNumber--; // makeMove only increments after Black's move

  const board = pos.board;
  const movedPiece = m.promotion ? us * PAWN : board[m.to];

  if (m.flags & FLAG_CASTLE_K) {
    const rank = us === WHITE ? 0 : 7;
    board[SQ(7, rank)] = us * ROOK;
    board[SQ(5, rank)] = EMPTY;
  } else if (m.flags & FLAG_CASTLE_Q) {
    const rank = us === WHITE ? 0 : 7;
    board[SQ(0, rank)] = us * ROOK;
    board[SQ(3, rank)] = EMPTY;
  }

  board[m.from] = movedPiece;
  board[m.to] = EMPTY;
  if (Math.abs(movedPiece) === KING) pos.kingSq[us] = m.from;

  if (m.flags & FLAG_EP) {
    board[st.epCapturedSquare] = st.capturedPiece;
  } else if (m.flags & FLAG_CAPTURE) {
    board[m.to] = st.capturedPiece;
  }
}

// A move is legal iff, after making it, the mover's own king is not attacked.
function isLegal(pos, m) {
  makeMove(pos, m);
  const legal = !inCheck(pos, -pos.side);
  unmakeMove(pos, m);
  return legal;
}

function generateLegalMoves(pos) {
  return generatePseudoMoves(pos).filter(m => isLegal(pos, m));
}

function moveToUCI(m) {
  const promoChar = m.promotion ? { [KNIGHT]: 'n', [BISHOP]: 'b', [ROOK]: 'r', [QUEEN]: 'q' }[m.promotion] : '';
  return sqName(m.from) + sqName(m.to) + promoChar;
}

// ===================================================================
// Perft (movegen self-test — not used in the live protocol path)
// ===================================================================

function perft(pos, depth) {
  if (depth === 0) return 1;
  let nodes = 0;
  for (const m of generatePseudoMoves(pos)) {
    makeMove(pos, m);
    if (!inCheck(pos, -pos.side)) nodes += perft(pos, depth - 1);
    unmakeMove(pos, m);
  }
  return nodes;
}

// ===================================================================
// Repetition / fifty-move detection
// ===================================================================

function repetitionCount(pos, keyA) {
  let count = 0;
  const hist = pos.history;
  // Only need to look back halfmoveClock plies (repetition requires no
  // capture/pawn move in between).
  const limit = Math.max(0, hist.length - 1 - pos.halfmoveClock);
  for (let i = hist.length - 2; i >= limit; i -= 2) {
    if (hist[i] === keyA) count++;
  }
  return count;
}

// ===================================================================
// Evaluation — tapered material + PST + mobility + king safety + pawns.
// White-positive; negated for black at the call site.
// ===================================================================

function gamePhase(pos) {
  let phase = 0;
  for (let sq = 0; sq < 128; sq++) {
    if (OFFBOARD(sq)) continue;
    const p = pos.board[sq];
    if (p !== EMPTY) phase += PHASE_WEIGHT[Math.abs(p)];
  }
  if (phase > PHASE_TOTAL) phase = PHASE_TOTAL;
  return Math.round((phase * 256) / PHASE_TOTAL);
}

function evaluate(pos) {
  let mg = 0, eg = 0;
  const board = pos.board;

  for (let sq = 0; sq < 128; sq++) {
    if (OFFBOARD(sq)) continue;
    const p = board[sq];
    if (p === EMPTY) continue;
    const color = Math.sign(p);
    const type = Math.abs(p);
    const sq64 = SQ64(sq);
    const pstSq = color === WHITE ? sq64 : (56 - (sq64 & ~7)) + (sq64 & 7); // mirror rank for black

    mg += color * (PIECE_VALUES_MG[type] + PST[type].mg[pstSq]);
    eg += color * (PIECE_VALUES_EG[type] + PST[type].eg[pstSq]);

    // Mobility: cheap approximation via attack-ray/offset counts, minors+rooks+queen only.
    if (type === KNIGHT || type === BISHOP || type === ROOK || type === QUEEN) {
      const mob = countMobility(pos, sq, type, color);
      mg += color * mob * 2;
      eg += color * mob * 3;
    }

    // Pawn structure: doubled / isolated (cheap file-based scan).
    if (type === PAWN) {
      const file = FILE_OF(sq);
      let doubled = false, isolated = true;
      for (let r = 0; r < 8; r++) {
        const s = SQ(file, r);
        if (s !== sq && board[s] === p) doubled = true;
      }
      for (const df of [-1, 1]) {
        const f = file + df;
        if (f < 0 || f > 7) continue;
        for (let r = 0; r < 8; r++) {
          if (board[SQ(f, r)] === p) { isolated = false; break; }
        }
      }
      if (doubled) { mg -= color * 10; eg -= color * 20; }
      if (isolated) { mg -= color * 8; eg -= color * 12; }
    }
  }

  // King safety (MG-only, fades out in the endgame): penalize an open file
  // on/adjacent to the king's own file with no shelter pawn.
  for (const color of [WHITE, BLACK]) {
    const ksq = pos.kingSq[color];
    if (ksq === -1) continue;
    const kf = FILE_OF(ksq);
    let shelterPenalty = 0;
    for (let f = Math.max(0, kf - 1); f <= Math.min(7, kf + 1); f++) {
      let hasPawn = false;
      for (let r = 0; r < 8; r++) {
        if (board[SQ(f, r)] === color * PAWN) { hasPawn = true; break; }
      }
      if (!hasPawn) shelterPenalty += 12;
    }
    mg -= color * shelterPenalty;
  }

  const phase = gamePhase(pos);
  const score = Math.round((mg * phase + eg * (256 - phase)) / 256);
  return pos.side === WHITE ? score : -score;
}

function countMobility(pos, sq, type, color) {
  const board = pos.board;
  let count = 0;
  if (type === KNIGHT) {
    for (const off of KNIGHT_OFFSETS) {
      const to = sq + off;
      if (!OFFBOARD(to) && Math.sign(board[to]) !== color) count++;
    }
    return count;
  }
  const dirs = type === BISHOP ? BISHOP_DIRS : type === ROOK ? ROOK_DIRS : QUEEN_DIRS;
  for (const dir of dirs) {
    let to = sq + dir;
    while (!OFFBOARD(to)) {
      if (board[to] === EMPTY) { count++; }
      else { if (Math.sign(board[to]) !== color) count++; break; }
      to += dir;
    }
  }
  return count;
}

// ===================================================================
// Move ordering helpers
// ===================================================================

const MVV_LVA_VICTIM = [0, 100, 320, 330, 500, 900, 10000];
const MVV_LVA_ATTACKER = [0, 1, 2, 3, 4, 5, 6];

function mvvLvaScore(m, board) {
  const attacker = Math.abs(board[m.from]);
  const victim = m.flags & FLAG_EP ? PAWN : Math.abs(m.captured);
  return MVV_LVA_VICTIM[victim] * 10 - MVV_LVA_ATTACKER[attacker];
}

function encodeMoveInt(m) {
  return (m.from & 127) | ((m.to & 127) << 7) | ((m.promotion & 7) << 14) | ((m.flags & 63) << 17);
}
function matchesMoveInt(m, moveInt) {
  return moveInt !== 0 && encodeMoveInt(m) === moveInt;
}

// ===================================================================
// Transposition table — TypedArray-backed, fixed size, well under the
// 256MB budget. One entry = 6 Int32 words: keyB(lock), score, depth,
// flag, moveInt, staticEval.
// ===================================================================

const TT_BITS = 20; // 2^20 = 1,048,576 entries * 24 bytes ≈ 25MB
const TT_SIZE = 1 << TT_BITS;
const TT_MASK = TT_SIZE - 1;
const TT_LOCK = new Int32Array(TT_SIZE);
const TT_SCORE = new Int32Array(TT_SIZE);
const TT_DEPTH = new Int8Array(TT_SIZE);
const TT_FLAG = new Int8Array(TT_SIZE);   // 0=EXACT, 1=LOWER, 2=UPPER
const TT_MOVE = new Int32Array(TT_SIZE);
const TT_USED = new Uint8Array(TT_SIZE);  // 0 = never written (avoids treating garbage as a hit)

const TT_EXACT = 0, TT_LOWER = 1, TT_UPPER = 2;

function ttProbe(keyA, keyB) {
  const idx = keyA & TT_MASK;
  if (TT_USED[idx] === 0 || TT_LOCK[idx] !== (keyB | 0)) return null;
  return { score: TT_SCORE[idx], depth: TT_DEPTH[idx], flag: TT_FLAG[idx], moveInt: TT_MOVE[idx] };
}
function ttStore(keyA, keyB, score, depth, flag, moveInt) {
  const idx = keyA & TT_MASK;
  if (TT_USED[idx] !== 0 && TT_DEPTH[idx] > depth && TT_LOCK[idx] !== (keyB | 0)) return; // depth-preferred on collision
  TT_LOCK[idx] = keyB | 0;
  TT_SCORE[idx] = score;
  TT_DEPTH[idx] = depth;
  TT_FLAG[idx] = flag;
  TT_MOVE[idx] = moveInt;
  TT_USED[idx] = 1;
}

// ===================================================================
// Search
// ===================================================================

const MATE = 30000;
const INFINITY = 32000;
const MAX_PLY = 64;

const killers = [];        // killers[ply] = [move1, move2]
const historyTable = new Int32Array(128 * 128); // [from*128+to] (0x88 indices)
for (let i = 0; i < MAX_PLY; i++) killers.push([null, null]);

function scoreMoves(pos, moves, ttMoveInt, ply) {
  const board = pos.board;
  for (const m of moves) {
    if (matchesMoveInt(m, ttMoveInt)) { m.score = 3000000; continue; }
    if (m.flags & FLAG_CAPTURE) { m.score = 1000000 + mvvLvaScore(m, board); continue; }
    const k = killers[ply];
    if (k && (sameMove(m, k[0]))) { m.score = 900000; continue; }
    if (k && (sameMove(m, k[1]))) { m.score = 800000; continue; }
    m.score = historyTable[m.from * 128 + m.to];
  }
}
function sameMove(m, other) {
  return other && m.from === other.from && m.to === other.to && m.promotion === other.promotion;
}
function pickBest(moves, from) {
  let bestIdx = from;
  for (let i = from + 1; i < moves.length; i++) if (moves[i].score > moves[bestIdx].score) bestIdx = i;
  if (bestIdx !== from) { const t = moves[from]; moves[from] = moves[bestIdx]; moves[bestIdx] = t; }
  return moves[from];
}

class TimeUp extends Error {}

function quiescence(pos, alpha, beta, info) {
  info.nodes++;
  if ((info.nodes & 2047) === 0 && Date.now() > info.hardDeadline) throw new TimeUp();

  const standPat = evaluate(pos);
  if (standPat >= beta) return beta;
  if (standPat > alpha) alpha = standPat;

  const captures = generateCaptures(pos);
  scoreMoves(pos, captures, 0, 0);

  for (let i = 0; i < captures.length; i++) {
    const m = pickBest(captures, i);
    // Delta pruning
    const victimVal = m.flags & FLAG_EP ? PIECE_VALUES_MG[PAWN] : PIECE_VALUES_MG[Math.abs(m.captured)];
    if (standPat + victimVal + 200 <= alpha && !m.promotion) continue;

    makeMove(pos, m);
    if (inCheck(pos, -pos.side)) { unmakeMove(pos, m); continue; }
    const score = -quiescence(pos, -beta, -alpha, info);
    unmakeMove(pos, m);

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

function negamax(pos, depth, alpha, beta, ply, info, canNull) {
  info.nodes++;
  if ((info.nodes & 1023) === 0 && Date.now() > info.hardDeadline) throw new TimeUp();

  const isPV = beta - alpha > 1;
  const inChk = inCheck(pos, pos.side);
  if (inChk) depth++; // check extension

  if (depth <= 0) return quiescence(pos, alpha, beta, info);

  // Draw detection (repetition / fifty-move) — not at the root. A true
  // threefold needs TWO prior occurrences (this is the third); a bare
  // single repeat is normal chess and must NOT be scored as a draw.
  if (ply > 0) {
    if (pos.halfmoveClock >= 100) return 0;
    if (repetitionCount(pos, pos.keyA) >= 2) return 0;
  }

  const tt = ttProbe(pos.keyA, pos.keyB);
  let ttMoveInt = 0;
  if (tt) {
    ttMoveInt = tt.moveInt;
    if (tt.depth >= depth && !isPV) {
      if (tt.flag === TT_EXACT) return tt.score;
      if (tt.flag === TT_LOWER && tt.score >= beta) return tt.score;
      if (tt.flag === TT_UPPER && tt.score <= alpha) return tt.score;
    }
  }

  const staticEval = evaluate(pos);

  // Reverse futility / static null-move pruning.
  if (!inChk && !isPV && depth <= 6 && Math.abs(beta) < MATE - 1000) {
    const margin = 80 * depth;
    if (staticEval - margin >= beta) return staticEval - margin;
  }

  // Null-move pruning.
  if (!inChk && !isPV && canNull && depth >= 3 && hasNonPawnMaterial(pos, pos.side)) {
    const R = 2 + Math.floor(depth / 4);
    makeNullMove(pos);
    let score;
    try {
      score = -negamax(pos, depth - 1 - R, -beta, -beta + 1, ply + 1, info, false);
    } finally {
      unmakeNullMove(pos);
    }
    if (score >= beta) return beta;
  }

  const moves = generatePseudoMoves(pos);
  scoreMoves(pos, moves, ttMoveInt, ply);

  const origAlpha = alpha;
  let bestScore = -INFINITY;
  let bestMoveInt = 0;
  let legalCount = 0;

  // Move-level futility (never node-level — see header note): skip only a
  // quiet, non-checking move at low depth with a hopeless static eval. The
  // check-giving test needs the move made first, so this is decided inside
  // the loop (after makeMove/legality), not as a pre-loop node bail-out —
  // that node-level shortcut is exactly the bug class this design avoids.
  const futilityMargin = 100 + 60 * depth;
  const futilityEligible = !inChk && !isPV && depth <= 3 && Math.abs(alpha) < MATE - 1000 &&
    (staticEval + futilityMargin <= alpha);

  for (let i = 0; i < moves.length; i++) {
    const m = pickBest(moves, i);
    const isCapture = (m.flags & FLAG_CAPTURE) !== 0;
    const isPromo = m.promotion !== 0;

    makeMove(pos, m);
    if (inCheck(pos, -pos.side)) { unmakeMove(pos, m); continue; }
    legalCount++;
    const givesCheck = inCheck(pos, pos.side);

    if (futilityEligible && legalCount > 1 && !isCapture && !isPromo && !givesCheck) {
      unmakeMove(pos, m);
      // Raise best_score to the futility bound (not left at -Infinity) so a
      // node where every remaining move gets pruned still returns a sane
      // fail-low estimate instead of a bogus mate-adjacent score.
      const futilityScore = staticEval + futilityMargin;
      if (futilityScore > bestScore) bestScore = futilityScore;
      continue;
    }

    let score;
    const reduce = (!isCapture && !isPromo && !givesCheck && !inChk && depth >= 3 && legalCount > 3 && !isPV)
      ? Math.max(1, Math.round(Math.log(depth) * Math.log(legalCount) * 0.5))
      : 0;

    if (legalCount === 1) {
      score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1, info, true);
    } else {
      score = -negamax(pos, depth - 1 - reduce, -alpha - 1, -alpha, ply + 1, info, true);
      if (score > alpha && (reduce > 0 || isPV)) {
        score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1, info, true);
      }
    }
    unmakeMove(pos, m);

    if (score > bestScore) {
      bestScore = score;
      bestMoveInt = encodeMoveInt(m);
      if (score > alpha) {
        alpha = score;
        if (alpha >= beta) {
          recordCutoff(m, ply, depth);
          break;
        }
      }
    }
  }

  if (legalCount === 0) {
    return inChk ? -MATE + ply : 0;
  }

  const flag = bestScore <= origAlpha ? TT_UPPER : bestScore >= beta ? TT_LOWER : TT_EXACT;
  ttStore(pos.keyA, pos.keyB, bestScore, depth, flag, bestMoveInt);
  return bestScore;
}

function recordCutoff(m, ply, depth) {
  if (!(m.flags & FLAG_CAPTURE)) {
    const k = killers[ply];
    if (!sameMove(m, k[0])) { k[1] = k[0]; k[0] = { from: m.from, to: m.to, promotion: m.promotion }; }
    historyTable[m.from * 128 + m.to] += depth * depth;
  }
}

function hasNonPawnMaterial(pos, color) {
  const board = pos.board;
  for (let sq = 0; sq < 128; sq++) {
    if (OFFBOARD(sq)) continue;
    const p = board[sq];
    if (Math.sign(p) === color) {
      const t = Math.abs(p);
      if (t === KNIGHT || t === BISHOP || t === ROOK || t === QUEEN) return true;
    }
  }
  return false;
}

function makeNullMove(pos) {
  pos.stateStack.push({ castling: pos.castling, epSquare: pos.epSquare, halfmoveClock: pos.halfmoveClock, keyA: pos.keyA, keyB: pos.keyB, capturedPiece: 0, epCapturedSquare: -1, isNull: true });
  if (pos.epSquare !== -1) { const f = FILE_OF(pos.epSquare); pos.keyA ^= ZOBRIST_EP_A[f]; pos.keyB ^= ZOBRIST_EP_B[f]; }
  pos.epSquare = -1;
  pos.side = -pos.side;
  pos.keyA ^= ZOBRIST_SIDE_A; pos.keyB ^= ZOBRIST_SIDE_B;
  pos.history.push(pos.keyA);
}
function unmakeNullMove(pos) {
  pos.history.pop();
  const st = pos.stateStack.pop();
  pos.castling = st.castling; pos.epSquare = st.epSquare; pos.halfmoveClock = st.halfmoveClock;
  pos.keyA = st.keyA; pos.keyB = st.keyB;
  pos.side = -pos.side;
}

// ===================================================================
// Root search: iterative deepening + time management + repetition-aware
// move selection (mirrors Huginn's #44/root-twofold-avoid lesson).
// ===================================================================

function pickRootMove(pos, timeBudgetMs) {
  const start = Date.now();
  const info = { nodes: 0, hardDeadline: start + timeBudgetMs };

  let rootMoves = generateLegalMoves(pos);
  if (rootMoves.length === 0) return null; // checkmate/stalemate — caller handles
  if (rootMoves.length === 1) return rootMoves[0];

  let bestMove = rootMoves[0];
  let bestScoreOverall = -INFINITY;
  const rootStaticEval = evaluate(pos);
  const winning = rootStaticEval >= 150;

  scoreMoves(pos, rootMoves, 0, 0);
  let depth = 1;
  try {
    for (; depth <= MAX_PLY; depth++) {
      let alpha = -INFINITY, beta = INFINITY;
      let iterBest = null, iterBestScore = -INFINITY;

      for (let i = 0; i < rootMoves.length; i++) {
        const m = pickBest(rootMoves, i);
        makeMove(pos, m);
        // Repetition-avoidance: if we're clearly winning and this move would
        // recreate an already-seen position, treat it as a draw score for
        // ROOT ORDERING purposes only (the in-tree search below the root
        // already draw-scores true repetitions/fifty-move via negamax).
        let score;
        if (winning && repetitionCount(pos, pos.keyA) >= 1) {
          score = 0;
          // Still need a real number to compare/allow escape if every move repeats.
        } else {
          score = -negamax(pos, depth - 1, -beta, -alpha, 1, info, true);
        }
        unmakeMove(pos, m);

        m.score = score + 500000; // keep ordering stable for the next iteration
        if (score > iterBestScore) { iterBestScore = score; iterBest = m; }
        if (score > alpha) alpha = score;
      }

      if (iterBest) {
        bestMove = iterBest;
        bestScoreOverall = iterBestScore;
        ttStore(pos.keyA, pos.keyB, iterBestScore, depth, TT_EXACT, encodeMoveInt(iterBest));
      }
      if (Math.abs(iterBestScore) >= MATE - 1000) break; // found a mate line, no need to go deeper
    }
  } catch (e) {
    if (!(e instanceof TimeUp)) throw e;
  }
  return bestMove;
}

// ===================================================================
// UCI move history replay
// ===================================================================

function applyUCIMove(pos, uci) {
  const from = sqFromName(uci.slice(0, 2));
  const to = sqFromName(uci.slice(2, 4));
  const promoChar = uci.length > 4 ? uci[4] : null;
  const promotion = promoChar ? { n: KNIGHT, b: BISHOP, r: ROOK, q: QUEEN }[promoChar] : 0;

  const moves = generatePseudoMoves(pos);
  const m = moves.find(mm => mm.from === from && mm.to === to && (mm.promotion || 0) === (promotion || 0));
  if (!m) throw new Error('Illegal move in history: ' + uci);
  makeMove(pos, m);
}

// The arbiter's FEN field is the CURRENT position (it already reflects every
// move in the trailing "moves" list) -- "moves" is the full game history
// from the true game start, needed only for repetition detection. Replaying
// it on top of the (already-advanced) leading FEN double-applies every move
// and throws on move 3+ (confirmed against a real arbiter-shaped input line:
// an FEN already at fullmove 2 with "moves e2e4 e7e5" appended). So when
// "moves" is present, reconstruct from the true start position instead and
// replay the whole list -- this also gives full per-ply Zobrist history for
// repetitionCount(), which a single already-advanced FEN can never provide.
const STANDARD_START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

function buildPositionFromLine(line) {
  const parts = line.trim().split(/\s+/);
  const fen = parts.slice(0, 6).join(' ');
  const movesIdx = parts.indexOf('moves');
  if (movesIdx >= 0) {
    try {
      const pos = parseFEN(STANDARD_START_FEN);
      for (let i = movesIdx + 1; i < parts.length; i++) applyUCIMove(pos, parts[i]);
      return pos;
    } catch (e) {
      // Non-standard game start, or a history/FEN mismatch -- fall back to
      // trusting the leading FEN as the current position directly. Loses
      // full-game repetition history but never forfeits over a parsing
      // mismatch.
    }
  }
  return parseFEN(fen);
}

// ===================================================================
// Entry point
// ===================================================================

const TIME_BUDGET_MS = 4300; // hard-stop well under the 5s limit
const FIRST_MOVE_GRACE_MS = 4300; // startup grace is for loading only, not search

function chooseMove(line) {
  const pos = buildPositionFromLine(line);
  const legalMoves = generateLegalMoves(pos);
  if (legalMoves.length === 0) return '0000'; // no legal move (mate/stalemate) — nothing sensible to emit
  const best = pickRootMove(pos, TIME_BUDGET_MS);
  return best ? moveToUCI(best) : moveToUCI(legalMoves[0]);
}

function runSelfTestPerft() {
  const cases = [
    { fen: 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1', depth: 4, expected: 197281 },
    { fen: 'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1', depth: 3, expected: 97862 },
    { fen: '8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1', depth: 4, expected: 43238 },
    { fen: 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1', depth: 5, expected: 4865609 },
    { fen: 'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1', depth: 4, expected: 4085603 },
  ];
  let allPass = true;
  for (const c of cases) {
    const pos = parseFEN(c.fen);
    const start = Date.now();
    const n = perft(pos, c.depth);
    const ok = n === c.expected;
    if (!ok) allPass = false;
    console.log(`perft(${c.depth}) "${c.fen.split(' ')[0]}" = ${n} (expected ${c.expected}) ${ok ? 'OK' : 'FAIL'} [${Date.now() - start}ms]`);
  }
  process.exit(allPass ? 0 : 1);
}

if (process.argv[2] === '--perft') {
  runSelfTestPerft();
} else if (process.argv[2] === '--eval') {
  const fen = process.argv.slice(3).join(' ') || 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';
  const pos = parseFEN(fen);
  console.log(fen);
  console.log('side to move:', pos.side === WHITE ? 'white' : 'black');
  console.log('evaluate() [side-to-move POV, +=good for mover]:', evaluate(pos));
  process.exit(0);
} else {
  let firstLine = true;
  const readline = require('readline');
  const rl = readline.createInterface({ input: process.stdin, terminal: false });
  rl.on('line', (line) => {
    if (!line || !line.trim()) return;
    const budget = firstLine ? FIRST_MOVE_GRACE_MS : TIME_BUDGET_MS;
    firstLine = false;
    let move;
    try {
      move = chooseMoveWithBudget(line, budget);
    } catch (e) {
      move = '0000';
    }
    process.stdout.write(move + '\n');
  });
}

function chooseMoveWithBudget(line, budgetMs) {
  const pos = buildPositionFromLine(line);
  const legalMoves = generateLegalMoves(pos);
  if (legalMoves.length === 0) return '0000';
  const best = pickRootMove(pos, budgetMs);
  return best ? moveToUCI(best) : moveToUCI(legalMoves[0]);
}
