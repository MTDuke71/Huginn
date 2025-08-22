#include <iostream>
#include <map>
#include <string>
#include <iomanip>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "move.hpp"
#include "board120.hpp"

// Reference breakdown for Kiwipete after White castles queenside, Black to move, depth 2
std::map<std::string, int> reference = {
	{"a6b5", 43}, {"a6b7", 44}, {"a6c4", 42}, {"a6c8", 44}, {"a6d3", 42}, {"a6e2", 41},
	{"a8b8", 44}, {"a8c8", 44}, {"a8d8", 44}, {"b4b3", 45}, {"b4c3", 45}, {"b6a4", 43},
	{"b6c4", 42}, {"b6c8", 44}, {"b6d5", 44}, {"c7c5", 45}, {"c7c6", 45}, {"d7d6", 43},
	{"e6d5", 44}, {"e7c5", 44}, {"e7d6", 43}, {"e7d8", 44}, {"e7f8", 44}, {"e8c8", 44},
	{"e8d8", 44}, {"e8f8", 44}, {"e8g8", 44}, {"f6d5", 45}, {"f6e4", 47}, {"f6g4", 43},
	{"f6g8", 45}, {"f6h5", 45}, {"f6h7", 45}, {"g6g5", 43}, {"g7f8", 44}, {"g7h6", 43},
	{"h3g2", 44}, {"h8f8", 44}, {"h8g8", 44}, {"h8h4", 44}, {"h8h5", 44}, {"h8h6", 44},
	{"h8h7", 44}
};



std::string move_to_alg(const S_MOVE& m) {
	char from[3], to[3];
	to_algebraic(m.get_from(), from);
	to_algebraic(m.get_to(), to);
	return std::string(from) + std::string(to);
}

uint64_t perft(Position& pos, int depth) {
	if (depth == 0) return 1;
	S_MOVELIST moves;
	generate_legal_moves_enhanced(pos, moves);
	uint64_t nodes = 0;
	for (int i = 0; i < moves.count; i++) {
		const auto& m = moves.moves[i];
		pos.make_move_with_undo(m);
		nodes += perft(pos, depth - 1);
		pos.undo_move();
	}
	return nodes;
}

int main() {
	std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	Position pos;
	if (!pos.set_from_fen(fen)) {
		std::cerr << "Failed to parse FEN" << std::endl;
		return 1;
	}
	S_MOVELIST moves;
	generate_legal_moves_enhanced(pos, moves);
	uint64_t total = 0;
	std::map<std::string, uint64_t> breakdown;
	S_MOVE e1c1_move;
	bool found_e1c1 = false;
	for (int i = 0; i < moves.count; i++) {
		const auto& m = moves.moves[i];
		std::string alg = move_to_alg(m);
		pos.make_move_with_undo(m);
		uint64_t count = perft(pos, 2);
		pos.undo_move();
		breakdown[alg] = count;
		total += count;
		if (alg == "e1c1") {
			e1c1_move = m;
			found_e1c1 = true;
		}
	}
	std::cout << "Depth: 3\n";
	std::cout << "Total: " << total << "\n";
	for (const auto& [alg, count] : breakdown) {
		std::cout << std::setw(6) << alg << " - " << std::setw(4) << count << std::endl;
	}
	if (found_e1c1) {
		pos.make_move_with_undo(e1c1_move);
	std::cout << "\nFEN after e1c1: " << pos.to_fen() << std::endl;
		pos.undo_move();
	} else {
		std::cout << "e1c1 not found in move list!" << std::endl;
	}
	return 0;
}
