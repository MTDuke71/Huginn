#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

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

void find_all_problematic_submoves(const std::string& after_h7g6_fen) {
    std::cout << "=== Finding ALL +1 Problematic Submoves ===\n";
    std::cout << "Position after h7->g6: " << after_h7g6_fen << "\n\n";

    Position vice_orig;
    vice_orig.set_from_fen(after_h7g6_fen);

    BitboardPosition bb_orig;
    bb_orig.set_from_fen(after_h7g6_fen);

    // Get all moves from original position
    S_MOVELIST vice_moves;
    generate_all_moves(vice_orig, vice_moves);
    
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_orig, bb_moves);

    std::vector<std::string> problematic_moves;
    
    std::cout << "Testing all moves to find which cause +1 errors at depth 2...\n\n";
    
    // Test each move
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& vmove = vice_moves.moves[i];
        int v_from = square120_to_64(vmove.get_from());
        int v_to = square120_to_64(vmove.get_to());
        
        if (v_from < 0 || v_to < 0) continue;
        
        std::string move_notation = sq64_to_alg(v_from) + sq64_to_alg(v_to);
        
        // Make move with VICE
        Position vice_after = vice_orig;
        if (vice_after.MakeMove(vmove) != 1) continue;
        
        // Find corresponding BitboardPosition move
        BitboardPosition bb_after = bb_orig;
        bool bb_found = false;
        BitboardPosition::UndoInfo undo_info;
        
        for (const auto& bmove : bb_moves.moves) {
            if (bmove.from_64 == v_from && bmove.to_64 == v_to) {
                SimpleBitboardMove simple_move = convert_move(bmove);
                undo_info = bb_after.make_move_with_undo(simple_move);
                bb_found = true;
                break;
            }
        }
        
        if (!bb_found) continue;
        
        // Test depth 2 perft
        Position vice_test = vice_after;
        BitboardPosition bb_test = bb_after;
        
        uint64_t vice_depth2 = perft_vice(vice_test, 2);
        uint64_t bb_depth2 = perft_bitboard(bb_test, 2);
        
        if (vice_depth2 != bb_depth2) {
            int64_t diff = (int64_t)bb_depth2 - (int64_t)vice_depth2;
            std::cout << move_notation << ": VICE=" << vice_depth2 << ", BB=" << bb_depth2 
                     << " DIFF=" << diff << " ❌\n";
            
            if (diff == 2) {
                problematic_moves.push_back(move_notation);
                
                // Now find which submoves cause the +1 errors
                std::cout << "  --> Finding +1 submoves after " << move_notation << ":\n";
                
                S_MOVELIST vice_submoves;
                generate_all_moves(vice_after, vice_submoves);
                
                BitboardMoveList bb_submoves;
                generate_legal_moves(bb_after, bb_submoves);
                
                for (int j = 0; j < vice_submoves.count; j++) {
                    const auto& vsubmove = vice_submoves.moves[j];
                    int vs_from = square120_to_64(vsubmove.get_from());
                    int vs_to = square120_to_64(vsubmove.get_to());
                    
                    if (vs_from < 0 || vs_to < 0) continue;
                    
                    std::string submove_notation = sq64_to_alg(vs_from) + sq64_to_alg(vs_to);
                    
                    Position vice_sub_test = vice_after;
                    if (vice_sub_test.MakeMove(vsubmove) != 1) continue;
                    
                    // Find corresponding BB submove
                    BitboardPosition bb_sub_test = bb_after;
                    bool bb_sub_found = false;
                    
                    for (const auto& bsubmove : bb_submoves.moves) {
                        if (bsubmove.from_64 == vs_from && bsubmove.to_64 == vs_to) {
                            SimpleBitboardMove simple_submove = convert_move(bsubmove);
                            BitboardPosition::UndoInfo sub_undo = bb_sub_test.make_move_with_undo(simple_submove);
                            bb_sub_found = true;
                            
                            uint64_t vice_sub_depth1 = perft_vice(vice_sub_test, 1);
                            uint64_t bb_sub_depth1 = perft_bitboard(bb_sub_test, 1);
                            
                            if (vice_sub_depth1 != bb_sub_depth1) {
                                int64_t sub_diff = (int64_t)bb_sub_depth1 - (int64_t)vice_sub_depth1;
                                std::cout << "      " << submove_notation << ": VICE=" << vice_sub_depth1 
                                         << ", BB=" << bb_sub_depth1 << " DIFF=" << sub_diff << " ❌\n";
                            }
                            
                            bb_sub_test.unmake_move(simple_submove, sub_undo);
                            break;
                        }
                    }
                    
                    vice_sub_test.TakeMove();
                }
                std::cout << "\n";
            }
        }
        
        vice_after.TakeMove();
    }
    
    std::cout << "\nSummary: " << problematic_moves.size() << " moves cause +2 overcounting:\n";
    for (const auto& move : problematic_moves) {
        std::cout << "  " << move << "\n";
    }
}

int main() {
    Huginn::init();

    std::string after_h7g6_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/R3K2R w KQkq - 0 2";
    
    find_all_problematic_submoves(after_h7g6_fen);

    return 0;
}