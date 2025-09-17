#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace BitboardMoveGen;

SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.is_promotion;
    simple_move.promotion_type = move.promotion_type;
    return simple_move;
}

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

int main() {
    std::cout << "=== FEN Integrity Test - Make/Unmake Verification ===\n";
    
    const char* original_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Original FEN: " << original_fen << "\n\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(original_fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    // Generate all legal moves
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    std::cout << "Testing " << moves.moves.size() << " legal moves...\n\n";
    
    bool all_fen_matches = true;
    int mismatch_count = 0;
    
    for (size_t i = 0; i < moves.moves.size(); i++) {
        const auto& move = moves.moves[i];
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        // Get FEN before move
        std::string fen_before = pos.to_fen();
        
        // Make the move
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        
        // Get FEN after move
        std::string fen_after = pos.to_fen();
        
        // Unmake the move
        pos.unmake_move(simple_move, undo);
        
        // Get FEN after unmake
        std::string fen_restored = pos.to_fen();
        
        // Check if FEN is properly restored
        if (fen_before != fen_restored) {
            std::cout << "❌ MISMATCH #" << (mismatch_count + 1) << " - Move: " << move_str << "\n";
            std::cout << "   Before:   " << fen_before << "\n";
            std::cout << "   After:    " << fen_after << "\n";
            std::cout << "   Restored: " << fen_restored << "\n";
            
            // Find the differences
            std::cout << "   Differences: ";
            for (size_t j = 0; j < std::min(fen_before.length(), fen_restored.length()); j++) {
                if (fen_before[j] != fen_restored[j]) {
                    std::cout << "pos[" << j << "]: '" << fen_before[j] << "' -> '" << fen_restored[j] << "' ";
                }
            }
            std::cout << "\n\n";
            
            all_fen_matches = false;
            mismatch_count++;
            
            // Stop after first few mismatches to avoid spam
            if (mismatch_count >= 5) {
                std::cout << "... (stopping after 5 mismatches)\n\n";
                break;
            }
        } else {
            // Optionally show first few successful moves
            if (i < 3) {
                std::cout << "✓ Move " << (i+1) << " (" << move_str << "): FEN correctly restored\n";
            }
        }
    }
    
    // Summary
    std::cout << "\n=== FEN INTEGRITY SUMMARY ===\n";
    if (all_fen_matches) {
        std::cout << "🎉 ALL " << moves.moves.size() << " MOVES: FEN correctly restored!\n";
        std::cout << "No make/unmake state corruption detected.\n";
    } else {
        std::cout << "❌ " << mismatch_count << " out of " << moves.moves.size() << " moves have FEN mismatches!\n";
        std::cout << "State corruption detected in make/unmake cycle.\n";
    }
    
    // Test specific problematic moves
    std::cout << "\n=== Testing Specific Problematic Moves ===\n";
    
    // Test e5f7 knight capture
    SimpleBitboardMove e5f7;
    e5f7.from_64 = 36; e5f7.to_64 = 53; e5f7.is_capture = true;
    e5f7.is_ep_capture = false; e5f7.is_castling = false; e5f7.is_promotion = false;
    
    std::string before_e5f7 = pos.to_fen();
    BitboardPosition::UndoInfo undo_e5f7 = pos.make_move_with_undo(e5f7);
    std::string after_e5f7 = pos.to_fen();
    pos.unmake_move(e5f7, undo_e5f7);
    std::string restored_e5f7 = pos.to_fen();
    
    std::cout << "e5f7 test:\n";
    std::cout << "  Before:   " << before_e5f7 << "\n";
    std::cout << "  After:    " << after_e5f7 << "\n";
    std::cout << "  Restored: " << restored_e5f7 << "\n";
    std::cout << "  Match:    " << (before_e5f7 == restored_e5f7 ? "✓ YES" : "❌ NO") << "\n\n";
    
    // Test e5g6 knight capture
    SimpleBitboardMove e5g6;
    e5g6.from_64 = 36; e5g6.to_64 = 46; e5g6.is_capture = true;
    e5g6.is_ep_capture = false; e5g6.is_castling = false; e5g6.is_promotion = false;
    
    std::string before_e5g6 = pos.to_fen();
    BitboardPosition::UndoInfo undo_e5g6 = pos.make_move_with_undo(e5g6);
    std::string after_e5g6 = pos.to_fen();
    pos.unmake_move(e5g6, undo_e5g6);
    std::string restored_e5g6 = pos.to_fen();
    
    std::cout << "e5g6 test:\n";
    std::cout << "  Before:   " << before_e5g6 << "\n";
    std::cout << "  After:    " << after_e5g6 << "\n";
    std::cout << "  Restored: " << restored_e5g6 << "\n";
    std::cout << "  Match:    " << (before_e5g6 == restored_e5g6 ? "✓ YES" : "❌ NO") << "\n";
    
    return 0;
}