#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert BitboardMoveList::BitboardMove to SimpleBitboardMove
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.promotion_type != PieceType::None;
    return simple_move;
}

// VICE perft function
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        if (pos.MakeMove(m) == 1) {
            nodes += perft_vice(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

// BitboardPosition perft function  
static uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        nodes += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return nodes;
}

// Convert 120-square to 64-square
int square120_to_64(int sq120) {
    int rank = sq120 / 10 - 2;
    int file = sq120 % 10 - 1;
    if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
    return rank * 8 + file;
}

// Convert 64-square to algebraic notation
std::string sq64_to_alg(int sq64) {
    if (sq64 < 0 || sq64 > 63) return "??";
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string piece_type_to_string(PieceType pt) {
    switch(pt) {
        case PieceType::Queen: return "Q";
        case PieceType::Rook: return "R";
        case PieceType::Bishop: return "B";
        case PieceType::Knight: return "N";
        case PieceType::None: return "";
        default: return "?";
    }
}

void debug_promotion_perft() {
    std::cout << "=== Debug Promotion Move Perft ===\n\n";

    // Position after b2b3
    std::string after_b2b3_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/PPN2Q2/2PBBPpP/R3K2R b KQkq - 0 2";
    
    Position vice_pos;
    vice_pos.set_from_fen(after_b2b3_fen);
    
    BitboardPosition bb_pos;
    bb_pos.set_from_fen(after_b2b3_fen);
    
    std::cout << "Testing perft after each g2 promotion move...\n\n";
    
    // Get promotion moves
    S_MOVELIST vice_moves;
    generate_all_moves(vice_pos, vice_moves);
    
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);
    
    // Test each g2 promotion move
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& vmove = vice_moves.moves[i];
        int from_64 = square120_to_64(vmove.get_from());
        int to_64 = square120_to_64(vmove.get_to());
        
        if (from_64 == 14) { // g2
            std::string move_notation = sq64_to_alg(from_64) + sq64_to_alg(to_64);
            
            // Make VICE move
            Position vice_after = vice_pos;
            if (vice_after.MakeMove(vmove) != 1) continue;
            
            // Find corresponding BitboardPosition move
            BitboardPosition bb_after = bb_pos;
            bool bb_found = false;
            BitboardPosition::UndoInfo undo_info;
            
            for (const auto& bmove : bb_moves.moves) {
                if (bmove.from_64 == from_64 && bmove.to_64 == to_64) {
                    // For promotion moves, we need to match the promotion type too
                    if (vmove.is_promotion() && bmove.is_promotion) {
                        // This is a simplified match - ideally we'd match the exact promotion piece
                        SimpleBitboardMove simple_move = convert_move(bmove);
                        undo_info = bb_after.make_move_with_undo(simple_move);
                        bb_found = true;
                        
                        std::cout << "=== " << move_notation << " promote to " << piece_type_to_string(bmove.promotion_type) << " ===\n";
                        
                        // Test perft depth 1
                        Position vice_test = vice_after;
                        BitboardPosition bb_test = bb_after;
                        
                        uint64_t vice_perft1 = perft_vice(vice_test, 1);
                        uint64_t bb_perft1 = perft_bitboard(bb_test, 1);
                        
                        std::cout << "VICE FEN:  " << vice_after.to_fen() << "\n";
                        std::cout << "BB FEN:    " << bb_after.to_fen() << "\n";
                        std::cout << "FEN Match: " << (vice_after.to_fen() == bb_after.to_fen() ? "✓" : "❌") << "\n";
                        std::cout << "Perft(1): VICE=" << vice_perft1 << ", BB=" << bb_perft1;
                        
                        if (vice_perft1 != bb_perft1) {
                            std::cout << " ❌ DIFF=" << ((int64_t)bb_perft1 - (int64_t)vice_perft1);
                        } else {
                            std::cout << " ✓";
                        }
                        std::cout << "\n\n";
                        
                        bb_after.unmake_move(simple_move, undo_info);
                        break;
                    }
                }
            }
            
            if (!bb_found) {
                std::cout << "❌ Could not find corresponding BitboardPosition move for " << move_notation << "\n";
            }
            
            vice_after.TakeMove();
        }
    }
}

int main() {
    Huginn::init();
    
    debug_promotion_perft();
    
    return 0;
}