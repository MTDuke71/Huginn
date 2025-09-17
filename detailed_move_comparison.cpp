#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace BitboardMoveGen;

SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_promotion = move.is_promotion;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    return simple_move;
}

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    std::string result = std::string(1, from_file) + from_rank + to_file + to_rank;

    if (move.is_promotion) {
        switch (move.promotion_type) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }

    return result;
}

uint64_t perft_with_move_tracking(BitboardPosition& pos, int depth, std::map<std::string, uint64_t>& move_nodes) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        uint64_t nodes = (depth == 1) ? 1 : perft_with_move_tracking(pos, depth - 1, move_nodes);
        pos.unmake_move(simple_move, undo_info);

        if (depth == 1) {
            move_nodes[move_str] = nodes;
        }
        total_nodes += nodes;
    }

    return total_nodes;
}

std::map<std::string, uint64_t> get_depth2_breakdown(BitboardPosition& pos) {
    std::map<std::string, uint64_t> breakdown;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);

        // Count moves at depth 1 (immediate next moves)
        BitboardMoveList next_moves;
        generate_legal_moves(pos, next_moves);
        breakdown[move_str] = next_moves.size();

        pos.unmake_move(simple_move, undo_info);
    }

    return breakdown;
}

void analyze_position_in_detail(const std::string& fen, const std::string& position_name) {
    std::cout << "\n=== Detailed Analysis: " << position_name << " ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;

    BitboardPosition pos;
    pos.set_from_fen(fen);

    // Get basic move counts
    BitboardMoveList all_moves, legal_moves;
    generate_all_moves(pos, all_moves);
    generate_legal_moves(pos, legal_moves);

    std::cout << "All moves generated: " << all_moves.size() << std::endl;
    std::cout << "Legal moves: " << legal_moves.size() << std::endl;

    if (all_moves.size() != legal_moves.size()) {
        std::cout << "WARNING: " << (all_moves.size() - legal_moves.size()) << " illegal moves filtered out" << std::endl;
    }

    // Get depth 2 breakdown
    std::map<std::string, uint64_t> depth2_breakdown = get_depth2_breakdown(pos);

    std::cout << "\nDepth 2 breakdown (move -> resulting legal moves):" << std::endl;
    uint64_t total_depth2_nodes = 0;
    for (const auto& pair : depth2_breakdown) {
        std::cout << "  " << pair.first << " -> " << pair.second << " moves" << std::endl;
        total_depth2_nodes += pair.second;
    }
    std::cout << "Total depth 2 nodes: " << total_depth2_nodes << std::endl;

    // Look for any suspicious patterns
    std::cout << "\nMove analysis:" << std::endl;

    // Group moves by piece type
    std::map<std::string, std::vector<std::string>> moves_by_piece;
    for (const auto& move : legal_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        std::string move_str = move_to_string(sm);

        PieceType piece_type = pos.piece_type_at(sm.from_64);
        std::string piece_name;
        switch (piece_type) {
            case PieceType::Pawn: piece_name = "Pawn"; break;
            case PieceType::Knight: piece_name = "Knight"; break;
            case PieceType::Bishop: piece_name = "Bishop"; break;
            case PieceType::Rook: piece_name = "Rook"; break;
            case PieceType::Queen: piece_name = "Queen"; break;
            case PieceType::King: piece_name = "King"; break;
            default: piece_name = "Unknown"; break;
        }

        moves_by_piece[piece_name].push_back(move_str);
    }

    for (const auto& pair : moves_by_piece) {
        std::cout << "  " << pair.first << " moves (" << pair.second.size() << "): ";
        for (size_t i = 0; i < pair.second.size(); ++i) {
            std::cout << pair.second[i];
            if (i < pair.second.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Check for duplicates
    std::set<std::string> unique_moves;
    std::vector<std::string> duplicates;
    for (const auto& move : legal_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        std::string move_str = move_to_string(sm);

        if (unique_moves.find(move_str) != unique_moves.end()) {
            duplicates.push_back(move_str);
        } else {
            unique_moves.insert(move_str);
        }
    }

    if (!duplicates.empty()) {
        std::cout << "WARNING: Duplicate moves found: ";
        for (const auto& dup : duplicates) {
            std::cout << dup << " ";
        }
        std::cout << std::endl;
    }

    // Check for any promotion moves
    bool has_promotions = false;
    for (const auto& move : legal_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        if (sm.is_promotion) {
            if (!has_promotions) {
                std::cout << "Promotion moves found:" << std::endl;
                has_promotions = true;
            }
            std::cout << "  " << move_to_string(sm) << std::endl;
        }
    }

    // Check for any en passant moves
    bool has_ep = false;
    for (const auto& move : legal_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        if (sm.is_ep_capture) {
            if (!has_ep) {
                std::cout << "En passant moves found:" << std::endl;
                has_ep = true;
            }
            std::cout << "  " << move_to_string(sm) << std::endl;
        }
    }

    // Check for any castling moves
    bool has_castling = false;
    for (const auto& move : legal_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        if (sm.is_castling) {
            if (!has_castling) {
                std::cout << "Castling moves found:" << std::endl;
                has_castling = true;
            }
            std::cout << "  " << move_to_string(sm) << std::endl;
        }
    }
}

void compare_knight_capture_positions() {
    std::cout << "=== Knight Capture Position Comparison ===" << std::endl;

    // Position after e5f7 (knight captures pawn on f7)
    analyze_position_in_detail(
        "r3k2r/p1ppqpb1/bn2pNp1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "After e5f7 (Nxf7)"
    );

    // Position after e5g6 (knight captures pawn on g6)
    analyze_position_in_detail(
        "r3k2r/p1ppqpb1/bn2pnN1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "After e5g6 (Nxg6)"
    );

    // For comparison, analyze the original Kiwipete position
    analyze_position_in_detail(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Original Kiwipete Position"
    );

    // Also test a simpler position for baseline
    analyze_position_in_detail(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position (Baseline)"
    );
}

int main() {
    std::cout << "=== Detailed Move-by-Move Comparison Tool ===" << std::endl;

    compare_knight_capture_positions();

    return 0;
}