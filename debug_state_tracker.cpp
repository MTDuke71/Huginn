#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert function
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

// Move to string function
std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result;
    result += char('a' + file_of_64(move.from_64));
    result += char('1' + rank_of_64(move.from_64));
    result += char('a' + file_of_64(move.to_64));
    result += char('1' + rank_of_64(move.to_64));
    return result;
}

// Function to dump detailed position state
void dump_position_state(const BitboardPosition& pos, const std::string& label) {
    std::cout << label << ":" << std::endl;
    std::cout << "  ply: " << pos.ply << std::endl;
    std::cout << "  fullmove_number: " << pos.fullmove_number << std::endl;
    std::cout << "  side_to_move: " << ((int)pos.side_to_move ? "BLACK" : "WHITE") << std::endl;
    std::cout << "  castling_rights: " << (int)pos.castling_rights << std::endl;
    std::cout << "  ep_square: " << pos.ep_square_64 << std::endl;
    std::cout << "  halfmove_clock: " << pos.halfmove_clock << std::endl;
}

// Perft WITH legal checking
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "Deep Position State Analysis - Tracking Cumulative Corruption" << std::endl;
    std::cout << "=============================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "\nInitial position state:" << std::endl;
    dump_position_state(pos, "INITIAL");
    
    // Generate all moves to get the exact sequence
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "\nFirst, test h2h3 in pure isolation:" << std::endl;
    
    // Find h2h3 move
    SimpleBitboardMove h2h3_move;
    bool found_h2h3 = false;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (move_to_string(simple_move) == "h2h3") {
            h2h3_move = simple_move;
            found_h2h3 = true;
            break;
        }
    }
    
    if (!found_h2h3) {
        std::cout << "ERROR: h2h3 not found!" << std::endl;
        return 1;
    }
    
    // Test h2h3 in isolation
    std::cout << "h2h3 legal: " << (pos.is_legal_move(h2h3_move) ? "YES" : "NO") << std::endl;
    if (pos.is_legal_move(h2h3_move)) {
        dump_position_state(pos, "BEFORE h2h3 isolation test");
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(h2h3_move);
        uint64_t nodes = perft_legal(pos, 2);
        pos.unmake_move(h2h3_move, undo_info);
        dump_position_state(pos, "AFTER h2h3 isolation test");
        std::cout << "h2h3 perft(2) in isolation: " << nodes << std::endl;
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Now processing moves in sequence to find corruption:" << std::endl;
    
    int move_count = 0;
    for (const auto& move : moves.moves) {
        move_count++;
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        std::cout << "\n--- Move " << move_count << ": " << move_str << " ---" << std::endl;
        
        // Dump position state BEFORE processing this move
        dump_position_state(pos, "BEFORE " + move_str);
        
        if (pos.is_legal_move(simple_move)) {
            std::cout << "Legal: YES" << std::endl;
            
            // Record position state before make_move/unmake_move cycle
            int ply_before = pos.ply;
            int fullmove_before = pos.fullmove_number;
            Color side_before = pos.side_to_move;
            int castling_before = pos.castling_rights;
            int ep_before = pos.ep_square_64;
            int halfmove_before = pos.halfmove_clock;
            
            // Perform the move/unmove cycle (simulating divide function)
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            uint64_t nodes = (3 <= 1) ? 1 : perft_legal(pos, 2);
            pos.unmake_move(simple_move, undo_info);
            
            // Check if position state was corrupted
            bool corrupted = false;
            if (pos.ply != ply_before) {
                std::cout << "*** CORRUPTION: ply changed from " << ply_before << " to " << pos.ply << " ***" << std::endl;
                corrupted = true;
            }
            if (pos.fullmove_number != fullmove_before) {
                std::cout << "*** CORRUPTION: fullmove_number changed from " << fullmove_before << " to " << pos.fullmove_number << " ***" << std::endl;
                corrupted = true;
            }
            if (pos.side_to_move != side_before) {
                std::cout << "*** CORRUPTION: side_to_move changed from " << (int)side_before << " to " << (int)pos.side_to_move << " ***" << std::endl;
                corrupted = true;
            }
            if (pos.castling_rights != castling_before) {
                std::cout << "*** CORRUPTION: castling_rights changed from " << castling_before << " to " << (int)pos.castling_rights << " ***" << std::endl;
                corrupted = true;
            }
            if (pos.ep_square_64 != ep_before) {
                std::cout << "*** CORRUPTION: ep_square changed from " << ep_before << " to " << pos.ep_square_64 << " ***" << std::endl;
                corrupted = true;
            }
            if (pos.halfmove_clock != halfmove_before) {
                std::cout << "*** CORRUPTION: halfmove_clock changed from " << halfmove_before << " to " << pos.halfmove_clock << " ***" << std::endl;
                corrupted = true;
            }
            
            std::cout << "Nodes: " << nodes << std::endl;
            
            if (corrupted) {
                std::cout << "*** CORRUPTION DETECTED AFTER " << move_str << " ***" << std::endl;
                dump_position_state(pos, "AFTER CORRUPTION");
            }
            
            // If this is h2h3, test it again to show the corruption effect
            if (move_str == "h2h3") {
                std::cout << "\n*** This is h2h3 - testing again after processing " << (move_count-1) << " previous moves ***" << std::endl;
                
                if (pos.is_legal_move(h2h3_move)) {
                    BitboardPosition::UndoInfo undo_info2 = pos.make_move_with_undo(h2h3_move);
                    uint64_t nodes2 = perft_legal(pos, 2);
                    pos.unmake_move(h2h3_move, undo_info2);
                    std::cout << "h2h3 perft(2) after " << (move_count-1) << " moves: " << nodes2 << std::endl;
                } else {
                    std::cout << "h2h3 is no longer legal!" << std::endl;
                }
                break;  // Stop here to focus on h2h3
            }
            
        } else {
            std::cout << "Legal: NO" << std::endl;
        }
        
        // Stop at move 10 to focus on the area around h2h3
        if (move_count >= 10) break;
    }
    
    return 0;
}