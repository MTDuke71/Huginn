/**
 * @file uci_utils.cpp
 * @brief Implementation of the UCI helpers declared in uci_utils.hpp.
 */
#include "uci_utils.hpp"
#include "position.hpp"
#include "move.hpp"
#include <algorithm>

/**
 * @brief Parses a UCI move string and returns the corresponding legal move.
 *
 * This function takes a move in UCI (Universal Chess Interface) notation (e.g., "e2e4", "e7e8q")
 * and attempts to find the corresponding legal move in the given chess position.
 * If the move is not legal or the notation is invalid, an empty S_MOVE is returned.
 *
 * @param uci_move The move in UCI notation as a string.
 * @param position The current chess position.
 * @return S_MOVE The corresponding legal move if found; otherwise, an empty S_MOVE.
 */

S_MOVE parse_uci_move(const std::string& uci_move, const Position& position) {
	if (uci_move.length() < 4) return S_MOVE();
	int from_file = uci_move[0] - 'a';
	int from_rank = uci_move[1] - '1';
	if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7) return S_MOVE();
	int from = sq64(File(from_file), Rank(from_rank));
	int to_file = uci_move[2] - 'a';
	int to_rank = uci_move[3] - '1';
	if (to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) return S_MOVE();
	int to = sq64(File(to_file), Rank(to_rank));
	PieceType promoted = PieceType::None;
	if (uci_move.length() == 5) {
		char promo_char = uci_move[4];
		switch (promo_char) {
			case 'q': promoted = PieceType::Queen; break;
			case 'r': promoted = PieceType::Rook; break;
			case 'b': promoted = PieceType::Bishop; break;
			case 'n': promoted = PieceType::Knight; break;
			default: return S_MOVE();
		}
	}
	S_MOVELIST move_list;
	generate_legal_moves(const_cast<Position&>(position), move_list);
	for (int i = 0; i < move_list.count; ++i) {
		const S_MOVE& move = move_list.moves[i];
		if (move.get_from() == from && move.get_to() == to) {
			if (promoted != PieceType::None) {
				if (move.get_promoted() == promoted) return move;
			} else if (move.get_promoted() == PieceType::None) {
				return move;
			}
		}
	}
	return S_MOVE();
}

// BACKLOG #54: structural / legal-position gate for the UCI boundary.
// set_from_fen stays permissive about piece counts so unit tests can build
// partial positions; everything the ENGINE assumes about a root position is
// enforced here instead, once, where GUI input enters.
bool validate_uci_position(const Position& pos, std::string* reason) {
	auto fail = [&](const char* why) {
		if (reason) *reason = why;
		return false;
	};

	// Exactly one king per side: king_sq[], evaluation, and check detection
	// all assume it.
	if (popcount(pos.piece_bitboards[0][int(PieceType::King)]) != 1 ||
	    popcount(pos.piece_bitboards[1][int(PieceType::King)]) != 1) {
		return fail("each side must have exactly one king");
	}

	// Piece-count sanity: at most 8 pawns / 16 units per side. This is the
	// deliberate second boundary in front of the fixed-capacity move list
	// (BACKLOG #55) — real games can never exceed it.
	for (int c = 0; c < 2; ++c) {
		if (popcount(pos.piece_bitboards[c][int(PieceType::Pawn)]) > 8) {
			return fail("more than 8 pawns for one side");
		}
		if (popcount(pos.color_bitboards[c]) > 16) {
			return fail("more than 16 pieces for one side");
		}
	}

	// Pawns cannot stand on rank 1 or rank 8.
	constexpr Bitboard BACK_RANKS = 0x00000000000000FFULL | 0xFF00000000000000ULL;
	if ((pos.piece_bitboards[0][int(PieceType::Pawn)] |
	     pos.piece_bitboards[1][int(PieceType::Pawn)]) & BACK_RANKS) {
		return fail("pawn on rank 1 or 8");
	}

	// The side NOT to move must not be in check — its king would be
	// capturable, and search/movegen assume that never happens.
	const Color stm = pos.side_to_move;
	const int idle_king = pos.king_sq[int(!stm)];
	if (idle_king >= 0 && Huginn::SqAttackedBB(idle_king, pos, stm)) {
		return fail("side not to move is in check");
	}

	// Castling rights must match the board: the castle move executor assumes
	// king and rook are on their home squares when a right is set.
	const Piece wk = make_piece(Color::White, PieceType::King);
	const Piece wr = make_piece(Color::White, PieceType::Rook);
	const Piece bk = make_piece(Color::Black, PieceType::King);
	const Piece br = make_piece(Color::Black, PieceType::Rook);
	if ((pos.castling_rights & CASTLE_WK) && (pos.at_sq64(4) != wk || pos.at_sq64(7) != wr)) {
		return fail("castling right K without king on e1 and rook on h1");
	}
	if ((pos.castling_rights & CASTLE_WQ) && (pos.at_sq64(4) != wk || pos.at_sq64(0) != wr)) {
		return fail("castling right Q without king on e1 and rook on a1");
	}
	if ((pos.castling_rights & CASTLE_BK) && (pos.at_sq64(60) != bk || pos.at_sq64(63) != br)) {
		return fail("castling right k without king on e8 and rook on h8");
	}
	if ((pos.castling_rights & CASTLE_BQ) && (pos.at_sq64(60) != bk || pos.at_sq64(56) != br)) {
		return fail("castling right q without king on e8 and rook on a8");
	}

	// En passant coherence: the double-pushed enemy pawn must sit in front of
	// the target square and both the target and the push origin must be
	// empty — otherwise MakeMove's EP executor would desync the board.
	if (pos.ep_square >= 0) {
		const bool white_to_move = (stm == Color::White);
		const int pawn_sq = white_to_move ? pos.ep_square - 8 : pos.ep_square + 8;
		const int origin_sq = white_to_move ? pos.ep_square + 8 : pos.ep_square - 8;
		const Piece pushed_pawn = make_piece(!stm, PieceType::Pawn);
		if (pawn_sq < 0 || pawn_sq >= 64 || origin_sq < 0 || origin_sq >= 64 ||
		    pos.at_sq64(pawn_sq) != pushed_pawn ||
		    pos.at_sq64(pos.ep_square) != Piece::None ||
		    pos.at_sq64(origin_sq) != Piece::None) {
			return fail("en passant square does not match a double pawn push");
		}
	}

	return true;
}

bool parse_spin_clamped(const std::string& s, long long lo, long long hi, long long& out) {
	if (s.empty()) return false;
	size_t idx = (s[0] == '-' || s[0] == '+') ? 1 : 0;
	if (idx == s.size()) return false;
	long long v = 0;
	for (; idx < s.size(); ++idx) {
		if (s[idx] < '0' || s[idx] > '9') return false;
		v = v * 10 + (s[idx] - '0');
		if (v > 1000000000LL) { v = 1000000000LL; break; }  // overflow guard: saturate
	}
	if (s[0] == '-') v = -v;
	out = std::max(lo, std::min(hi, v));
	return true;
}

Huginn::MinimalLimits parse_go_command(const std::vector<std::string>& tokens,
                                        Color side_to_move,
                                        bool& infinite_requested) {
	Huginn::MinimalLimits limits;
	limits.infinite = false;
	limits.max_time_ms = 0;
	limits.max_depth = 25;

	bool depth_specified = false;
	infinite_requested = false;
	constexpr long long GO_TIME_MAX_MS = 1000000000LL;
	long long winc = 0, binc = 0, movestogo = 0, wtime = -1, btime = -1;
	long long movetime = -1;

	auto parse_go_number = [&](size_t& i, long long lo, long long hi, long long& out) -> bool {
		if (i + 1 >= tokens.size()) return false;
		++i;
		long long v = 0;
		if (!parse_spin_clamped(tokens[i], lo, hi, v)) return false;
		out = v;
		return true;
	};

	for (size_t i = 1; i < tokens.size(); i++) {
		if (tokens[i] == "depth") {
			long long depth = 0;
			if (parse_go_number(i, 1, Huginn::MAX_DEPTH, depth)) {
				limits.max_depth = static_cast<int>(depth);
				depth_specified = true;
			}
		}
		else if (tokens[i] == "movetime")  parse_go_number(i, 1, GO_TIME_MAX_MS, movetime);
		else if (tokens[i] == "wtime")     parse_go_number(i, 0, GO_TIME_MAX_MS, wtime);
		else if (tokens[i] == "btime")     parse_go_number(i, 0, GO_TIME_MAX_MS, btime);
		else if (tokens[i] == "winc")      parse_go_number(i, 0, GO_TIME_MAX_MS, winc);
		else if (tokens[i] == "binc")      parse_go_number(i, 0, GO_TIME_MAX_MS, binc);
		else if (tokens[i] == "movestogo") parse_go_number(i, 1, 500, movestogo);
		else if (tokens[i] == "infinite") {
			limits.infinite = true;
			infinite_requested = true;
			limits.max_time_ms = 0;
		}
	}

	// #56: `go infinite` must run until `stop` — give it the engine's full
	// depth range unless a depth was also specified.
	if (infinite_requested && !depth_specified) {
		limits.max_depth = Huginn::MAX_DEPTH;
	}

	if (depth_specified) {
		limits.infinite = true;  // Depth-only search ignores time
		limits.max_time_ms = 0;
	}
	else if (movetime > 0) {
		limits.max_time_ms = static_cast<int>(movetime);
	}
	else if (!limits.infinite && (wtime >= 0 || btime >= 0)) {
		// Clock-based allocation — strategy unchanged from the gauntleted #47
		// tuning.
		const bool white_to_move = (side_to_move == Color::White);
		const long long side_time = white_to_move ? wtime : btime;  // -1 = our clock not sent
		const long long side_inc = white_to_move ? winc : binc;
		limits.max_time_ms = static_cast<int>(compute_time_budget_ms(side_time, side_inc, movestogo));
	}
	else if (!limits.infinite) {
		// Bare `go` with no limits at all: a defined default budget.
		limits.max_time_ms = 5000;
	}

	return limits;
}

long long compute_time_budget_ms(long long time_ms, long long inc_ms, long long movestogo) {
	if (inc_ms < 0) inc_ms = 0;
	if (movestogo < 0) movestogo = 0;
	if (movestogo > 500) movestogo = 500;

	if (time_ms < 0) {
		// Our clock was not sent (opponent-clock-only / increment-only `go`).
		// Spend a conservative slice of the increment if there is one,
		// otherwise fall back to the bare-`go` default budget.
		return inc_ms > 0 ? std::max(50LL, inc_ms / 4) : 5000LL;
	}

	// Allocation strategy unchanged from #47 (gauntleted: t21 +127 self-play).
	long long alloc = (movestogo > 0)
	    ? time_ms / movestogo + inc_ms / 2   // classical: split the period
	    : time_ms / 20 + inc_ms / 2;         // sudden death / increment

	// Reserve a slice of the clock as a hard usage CAP (not a subtraction)
	// so low-time scrambles never flag. No floor on safe_max: when the whole
	// clock is inside the reserve, the safely usable remainder really is 0
	// (the old max(50, ...) here is what let a 60 ms clock budget 50 ms).
	long long reserve = std::max(50LL, std::min(1000LL, time_ms / 10));
	long long safe_max = std::max(0LL, time_ms - reserve);
	long long cap60 = (time_ms * 6) / 10;

	alloc = std::min(alloc, std::min(safe_max, cap60));

	// 50 ms quality floor for normal clocks — capped by the safely usable
	// remainder, never above it (#56: the old FINAL max(50, alloc) overdrew
	// tiny clocks by design). 1 ms absolute minimum: the first checkup stops
	// the search almost immediately and the depth-1 move goes out — an
	// instant legal move beats a time forfeit.
	if (alloc < 50) alloc = 50;
	if (alloc > safe_max) alloc = safe_max;
	if (alloc < 1) alloc = 1;
	return alloc;
}
