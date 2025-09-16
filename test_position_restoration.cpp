#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Function to dump detailed position state
void dump_position_state(const BitboardPosition& pos, const std::string& label) {
    std::cout << label << ":" << std::endl;
    std::cout << "  ply: " << pos.ply << std::endl;
    std::cout << "  fullmove_number: " << pos.fullmove_number << std::endl;
    std::cout << "  side_to_move: " << ((int)pos.side_to_move ? "BLACK" : "WHITE") << std::endl;
    std::cout << "  castling_rights: " << (int)pos.castling_rights << std::endl;
    std::cout << "  ep_square: " << pos.ep_square_64 << std::endl;
    std::cout << "  halfmove_clock: " << pos.halfmove_clock << std::endl;
    std::cout << "  zobrist_key: 0x" << std::hex << pos.zobrist_key << std::dec << std::endl;
    std::cout << "  king_square[WHITE]: " << pos.king_square_64[0] << std::endl;
    std::cout << "  king_square[BLACK]: " << pos.king_square_64[1] << std::endl;
    std::cout << "  material_score[WHITE]: " << pos.material_score[0] << std::endl;
    std::cout << "  material_score[BLACK]: " << pos.material_score[1] << std::endl;
}

// Function to check if two positions are identical
bool positions_identical(const BitboardPosition& pos1, const BitboardPosition& pos2) {
    bool identical = true;
    
    if (pos1.ply != pos2.ply) {
        std::cout << "MISMATCH: ply (" << pos1.ply << " vs " << pos2.ply << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.fullmove_number != pos2.fullmove_number) {
        std::cout << "MISMATCH: fullmove_number (" << pos1.fullmove_number << " vs " << pos2.fullmove_number << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.side_to_move != pos2.side_to_move) {
        std::cout << "MISMATCH: side_to_move (" << (int)pos1.side_to_move << " vs " << (int)pos2.side_to_move << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.castling_rights != pos2.castling_rights) {
        std::cout << "MISMATCH: castling_rights (" << (int)pos1.castling_rights << " vs " << (int)pos2.castling_rights << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.ep_square_64 != pos2.ep_square_64) {
        std::cout << "MISMATCH: ep_square_64 (" << pos1.ep_square_64 << " vs " << pos2.ep_square_64 << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.halfmove_clock != pos2.halfmove_clock) {
        std::cout << "MISMATCH: halfmove_clock (" << pos1.halfmove_clock << " vs " << pos2.halfmove_clock << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.zobrist_key != pos2.zobrist_key) {
        std::cout << "MISMATCH: zobrist_key (0x" << std::hex << pos1.zobrist_key << " vs 0x" << pos2.zobrist_key << std::dec << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.king_square_64[0] != pos2.king_square_64[0]) {
        std::cout << "MISMATCH: king_square[WHITE] (" << pos1.king_square_64[0] << " vs " << pos2.king_square_64[0] << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.king_square_64[1] != pos2.king_square_64[1]) {
        std::cout << "MISMATCH: king_square[BLACK] (" << pos1.king_square_64[1] << " vs " << pos2.king_square_64[1] << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.material_score[0] != pos2.material_score[0]) {
        std::cout << "MISMATCH: material_score[WHITE] (" << pos1.material_score[0] << " vs " << pos2.material_score[0] << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.material_score[1] != pos2.material_score[1]) {
        std::cout << "MISMATCH: material_score[BLACK] (" << pos1.material_score[1] << " vs " << pos2.material_score[1] << ")" << std::endl;
        identical = false;
    }
    
    // Check piece bitboards
    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 7; piece++) {
            if (pos1.piece_bitboards[color][piece] != pos2.piece_bitboards[color][piece]) {
                std::cout << "MISMATCH: piece_bitboards[" << color << "][" << piece << "] (0x" 
                         << std::hex << pos1.piece_bitboards[color][piece] << " vs 0x" 
                         << pos2.piece_bitboards[color][piece] << std::dec << ")" << std::endl;
                identical = false;
            }
        }
    }
    
    // Check color bitboards
    if (pos1.color_bitboards[0] != pos2.color_bitboards[0]) {
        std::cout << "MISMATCH: color_bitboards[WHITE] (0x" << std::hex << pos1.color_bitboards[0] 
                 << " vs 0x" << pos2.color_bitboards[0] << std::dec << ")" << std::endl;
        identical = false;
    }
    
    if (pos1.color_bitboards[1] != pos2.color_bitboards[1]) {
        std::cout << "MISMATCH: color_bitboards[BLACK] (0x" << std::hex << pos1.color_bitboards[1] 
                 << " vs 0x" << pos2.color_bitboards[1] << std::dec << ")" << std::endl;
        identical = false;
    }
    
    // Check occupied bitboard
    if (pos1.occupied_bitboard != pos2.occupied_bitboard) {
        std::cout << "MISMATCH: occupied_bitboard (0x" << std::hex << pos1.occupied_bitboard 
                 << " vs 0x" << pos2.occupied_bitboard << std::dec << ")" << std::endl;
        identical = false;
    }
    
    return identical;
}

int main() {
    std::cout << "Testing position restoration after a2a3 make/unmake" << std::endl;
    std::cout << "===================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Create a backup copy of the starting position
    BitboardPosition original_pos = pos;
    
    std::cout << "Original starting position:" << std::endl;
    dump_position_state(original_pos, "ORIGINAL");
    
    // Create a2a3 move
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;  // a2
    a2a3.to_64 = 16;   // a3
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    std::cout << "\nTesting a2a3 legality..." << std::endl;
    bool is_legal = pos.is_legal_move(a2a3);
    std::cout << "a2a3 is legal: " << (is_legal ? "YES" : "NO") << std::endl;
    
    if (!is_legal) {
        std::cout << "ERROR: a2a3 should be legal!" << std::endl;
        return 1;
    }
    
    std::cout << "\nChecking position state after is_legal_move call:" << std::endl;
    bool state_preserved = positions_identical(pos, original_pos);
    if (state_preserved) {
        std::cout << "✓ Position state is preserved after is_legal_move call" << std::endl;
    } else {
        std::cout << "✗ Position state was corrupted by is_legal_move call!" << std::endl;
        dump_position_state(pos, "AFTER is_legal_move");
        return 1;
    }
    
    std::cout << "\nMaking a2a3 move..." << std::endl;
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(a2a3);
    
    std::cout << "Position after making a2a3:" << std::endl;
    dump_position_state(pos, "AFTER MAKE");
    
    std::cout << "\nUnmaking a2a3 move..." << std::endl;
    pos.unmake_move(a2a3, undo_info);
    
    std::cout << "Position after unmaking a2a3:" << std::endl;
    dump_position_state(pos, "AFTER UNMAKE");
    
    std::cout << "\nComparing restored position with original:" << std::endl;
    bool fully_restored = positions_identical(pos, original_pos);
    
    if (fully_restored) {
        std::cout << "✓ SUCCESS: Position fully restored to original state!" << std::endl;
    } else {
        std::cout << "✗ FAILURE: Position not fully restored!" << std::endl;
    }
    
    std::cout << "\nTesting position again to see if FEN is identical:" << std::endl;
    std::string original_fen = original_pos.to_fen();
    std::string restored_fen = pos.to_fen();
    
    std::cout << "Original FEN:  " << original_fen << std::endl;
    std::cout << "Restored FEN:  " << restored_fen << std::endl;
    std::cout << "FEN identical: " << (original_fen == restored_fen ? "YES" : "NO") << std::endl;
    
    // Test multiple make/unmake cycles
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Testing multiple make/unmake cycles..." << std::endl;
    
    bool all_cycles_ok = true;
    for (int cycle = 1; cycle <= 5; cycle++) {
        std::cout << "\nCycle " << cycle << ":" << std::endl;
        
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(a2a3);
        pos.unmake_move(a2a3, undo);
        
        bool cycle_ok = positions_identical(pos, original_pos);
        std::cout << "  Cycle " << cycle << " result: " << (cycle_ok ? "✓ OK" : "✗ FAILED") << std::endl;
        
        if (!cycle_ok) {
            all_cycles_ok = false;
            break;
        }
    }
    
    std::cout << "\nMultiple cycles result: " << (all_cycles_ok ? "✓ ALL OK" : "✗ FAILED") << std::endl;
    
    return fully_restored && all_cycles_ok ? 0 : 1;
}