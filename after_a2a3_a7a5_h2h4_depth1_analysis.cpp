#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <set>

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
    result += char('a' + (move.from_64 % 8));
    result += char('1' + (move.from_64 / 8));
    result += char('a' + (move.to_64 % 8));
    result += char('1' + (move.to_64 / 8));
    return result;
}

// Square to algebraic for en passant display
std::string square_to_alg(int square) {
    if (square == -1) return "-";
    return std::string(1, 'a' + (square % 8)) + std::string(1, '1' + (square / 8));
}

int main() {
    std::cout << "=== After a2a3 a7a5 h2h4 Depth 1 Analysis ===\n\n";
    
    // Reference data from user after a2a3 a7a5 h2h4
    std::set<std::string> expected_moves = {
        "a5a4", "a8a6", "a8a7", "b7b5", "b7b6", "b8a6", "b8c6", "c7c5", "c7c6",
        "d7d5", "d7d6", "e7e5", "e7e6", "f7f5", "f7f6", "g7g5", "g7g6", "g8f6",
        "g8h6", "h7h5", "h7h6"
    };
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Playing sequence: a2a3 a7a5 h2h4\n\n";
    
    // Make a2a3 move
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;   // a2
    a2a3.to_64 = 16;    // a3
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a2a3);
    std::cout << "After a2a3: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make a7a5 move (black pawn 2-square move)
    SimpleBitboardMove a7a5;
    a7a5.from_64 = 48;  // a7
    a7a5.to_64 = 32;    // a5
    a7a5.is_capture = false;
    a7a5.is_ep_capture = false;
    a7a5.is_castling = false;
    a7a5.is_promotion = false;
    a7a5.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a7a5);
    std::cout << "After a7a5: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make h2h4 move (white pawn 2-square move)
    SimpleBitboardMove h2h4;
    h2h4.from_64 = 15;  // h2
    h2h4.to_64 = 31;    // h4
    h2h4.is_capture = false;
    h2h4.is_ep_capture = false;
    h2h4.is_castling = false;
    h2h4.is_promotion = false;
    h2h4.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(h2h4);
    std::cout << "After h2h4: ep_square = " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    std::cout << "Position after a2a3 a7a5 h2h4:\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "White pawn on a3: " << (pos.piece_at(16) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "Black pawn on a5: " << (pos.piece_at(32) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "White pawn on h4: " << (pos.piece_at(31) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "En passant square: " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    // Critical analysis: en passant should be h3 after h2h4
    if (pos.ep_square_64 == 23) {  // h3 square
        std::cout << "✅ En passant square correctly set to h3 (after h2h4)\n";
        std::cout << "Black should be able to play g5xh3 en passant if g5 pawn exists.\n\n";
    } else {
        std::cout << "❌ En passant square unexpectedly set to " << square_to_alg(pos.ep_square_64) << "\n";
        std::cout << "Expected h3 (23) after h2h4 move.\n\n";
    }
    
    // Now test all moves from this position
    std::cout << "Testing all moves after a2a3 a7a5 h2h4:\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::set<std::string> our_moves;
    std::set<std::string> extra_moves;
    std::set<std::string> missing_moves;
    
    std::cout << "Our Legal Moves:\n";
    std::cout << "----------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            our_moves.insert(move_str);
            
            std::cout << move_str;
            if (simple_move.is_ep_capture) {
                std::cout << " (EN PASSANT!)";
            }
            if (simple_move.is_capture) {
                std::cout << " (CAPTURE)";
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "\n=== Comparison Analysis ===\n";
    std::cout << "Expected: " << expected_moves.size() << " moves\n";
    std::cout << "Our count: " << our_moves.size() << " moves\n\n";
    
    // Find extra moves (in ours but not in expected)
    for (const auto& move : our_moves) {
        if (expected_moves.find(move) == expected_moves.end()) {
            extra_moves.insert(move);
        }
    }
    
    // Find missing moves (in expected but not in ours)
    for (const auto& move : expected_moves) {
        if (our_moves.find(move) == our_moves.end()) {
            missing_moves.insert(move);
        }
    }
    
    if (!extra_moves.empty()) {
        std::cout << "🎯 EXTRA MOVES (our bug):\n";
        for (const auto& move : extra_moves) {
            std::cout << "❌ " << move << " (should NOT exist)\n";
        }
        std::cout << "\n";
    }
    
    if (!missing_moves.empty()) {
        std::cout << "❌ MISSING MOVES:\n";
        for (const auto& move : missing_moves) {
            std::cout << "❌ " << move << " (should exist)\n";
        }
        std::cout << "\n";
    }
    
    if (extra_moves.empty() && missing_moves.empty()) {
        std::cout << "✅ Perfect match! All moves correct.\n\n";
    }
    
    std::cout << "=== Bug Analysis ===\n";
    if (!extra_moves.empty()) {
        std::cout << "🎯 FOUND THE BUG!\n";
        std::cout << "We are generating " << extra_moves.size() << " extra move(s).\n";
        std::cout << "This explains the +1 over-generation.\n";
        
        for (const auto& extra_move : extra_moves) {
            std::cout << "\nAnalyzing extra move: " << extra_move << "\n";
            
            // Check if it's en passant related
            for (const auto& move : moves.moves) {
                SimpleBitboardMove simple_move = convert_move(move);
                if (pos.is_legal_move(simple_move) && move_to_string(simple_move) == extra_move) {
                    if (simple_move.is_ep_capture) {
                        std::cout << "This is an INVALID EN PASSANT CAPTURE!\n";
                        std::cout << "From square: " << simple_move.from_64 << "\n";
                        std::cout << "To square: " << simple_move.to_64 << "\n";
                        std::cout << "En passant square: " << pos.ep_square_64 << "\n";
                    }
                    break;
                }
            }
        }
    }
    
    return 0;
}