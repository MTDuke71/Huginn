#include "uci_utils.hpp"

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
	int from = sq(File(from_file), Rank(from_rank));
	int to_file = uci_move[2] - 'a';
	int to_rank = uci_move[3] - '1';
	if (to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) return S_MOVE();
	int to = sq(File(to_file), Rank(to_rank));
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
	generate_legal_moves_enhanced(const_cast<Position&>(position), move_list);
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
