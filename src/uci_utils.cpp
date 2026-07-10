/**
 * @file uci_utils.cpp
 * @brief Implementation of the UCI helpers declared in uci_utils.hpp.
 */
#include "uci_utils.hpp"
#include "position.hpp"
#include "move.hpp"

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
