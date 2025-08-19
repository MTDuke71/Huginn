#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"
#include "sq_attacked.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

// Convert S_MOVE to algebraic notation for debugging
static std::string move_to_string(const S_MOVE& move) {
    int from = move.get_from();
    int to = move.get_to();
    
    std::string result;
    result += char('a' + int(file_of(from)));
    result += char('1' + int(rank_of(from)));
    result += char('a' + int(file_of(to)));
    result += char('1' + int(rank_of(to)));
    
    if (move.is_promotion()) {
        switch (move.get_promoted()) {
            case PieceType::Queen: result += 'q'; break;
            case PieceType::Rook: result += 'r'; break;
            case PieceType::Bishop: result += 'b'; break;
            case PieceType::Knight: result += 'n'; break;
            default: break;
        }
    }
    
    return result;
}

// Tiny perft harness (uses legal moves; grow as you add rules)
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        State st{};
        make_move(pos, m, st);
        nodes += perft(pos, depth-1);
        unmake_move(pos, m, st);
    }
    return nodes;
}

// Perft divide function for debugging - shows breakdown by move
static uint64_t perft_divide(Position& pos, int depth, bool print_moves = true) {
    if (depth == 0) return 1;
    
    MoveList list; 
    generate_legal_moves(pos, list);
    uint64_t total_nodes = 0;
    
    if (print_moves) {
        std::cout << "\nPerft divide at depth " << depth << ":\n";
        std::cout << "Move count: " << list.v.size() << "\n";
        std::cout << std::string(30, '-') << "\n";
    }
    
    for (const auto& m : list.v) {
        State st{};
        make_move(pos, m, st);
        uint64_t nodes = perft(pos, depth-1);
        unmake_move(pos, m, st);
        
        total_nodes += nodes;
        
        if (print_moves) {
            std::cout << std::setw(6) << move_to_string(m) 
                      << ": " << std::setw(8) << nodes << "\n";
        }
    }
    
    if (print_moves) {
        std::cout << std::string(30, '-') << "\n";
        std::cout << "Total: " << std::setw(8) << total_nodes << "\n\n";
    }
    
    return total_nodes;
}

// Smoke: start position is initialized correctly
TEST(Perft, StartposSmoke_KingsAndPawns) {
    Position pos; pos.set_startpos();
    int whitePawns=0, blackPawns=0, whiteKings=0, blackKings=0;
    for (int r=0; r<8; ++r) {
        for (int f=0; f<8; ++f) {
            int s = sq(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = pos.at(s);
            if (is_none(p)) continue;
            if (p == Piece::WhitePawn) ++whitePawns;
            if (p == Piece::BlackPawn) ++blackPawns;
            if (p == Piece::WhiteKing) ++whiteKings;
            if (p == Piece::BlackKing) ++blackKings;
        }
    }
    EXPECT_EQ(whitePawns, 8);
    EXPECT_EQ(blackPawns, 8);
    EXPECT_EQ(whiteKings, 1);
    EXPECT_EQ(blackKings, 1);
}

// These are disabled until your generator covers all piece types & rules.
// Enable one depth at a time as you pass prior tests.

TEST(Perft, Startpos_d1_is_20) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 1), 20u);
}

TEST(Perft, Startpos_d2_is_400) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 2), 400u);
}

TEST(Perft, Startpos_d3_is_8902) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 3), 8902u);
}

// Kiwipete covers castling, pins, etc. Enable after specials work.
TEST(Perft, Kiwipete_d1_48_d2_2039) {
    Position pos;
    // Set up the famous Kiwipete position using FEN parsing
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    // Test the known perft values for this position
    // Now working correctly with proper legal move generation and en passant
    EXPECT_EQ(perft(pos, 1), 48u);   // depth 1: 48 moves ✓
    EXPECT_EQ(perft(pos, 2), 2039u); // depth 2: 2039 moves ✓ (fixed illegal moves + en passant)
}

// Debug test to analyze the Kiwipete perft discrepancy
TEST(Perft, KiwipeteDebugDivide) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::cout << "\n=== Kiwipete Perft Divide Debug ===\n";
    std::cout << "FEN: " << kiwipete_fen << "\n";
    
    // Show depth 1 breakdown
    uint64_t d1_result = perft_divide(pos, 1, true);
    std::cout << "Depth 1 result: " << d1_result << " (expected: 48)\n";
    
    // If depth 1 is correct, show depth 2 breakdown for a few key moves
    if (d1_result == 48) {
        std::cout << "\n=== Depth 2 Analysis (first 10 moves) ===\n";
        MoveList list; 
        generate_legal_moves(pos, list);
        
        uint64_t total_d2 = 0;
        int move_count = 0;
        
        for (const auto& m : list.v) {
            if (move_count >= 10) break; // Limit output
            
            State st{};
            make_move(pos, m, st);
            uint64_t nodes = perft_divide(pos, 1, false); // Don't print sub-moves
            unmake_move(pos, m, st);
            
            total_d2 += nodes;
            
            std::cout << move_to_string(m) << ": " << nodes << "\n";
            move_count++;
        }
        
        // Calculate total for all moves
        uint64_t full_d2_result = perft(pos, 2);
        std::cout << "Full depth 2 result: " << full_d2_result << " (expected: 2039)\n";
        std::cout << "Difference: " << (int64_t)full_d2_result - 2039 << "\n";
        
        // Let's specifically check the castling moves
        std::cout << "\n=== Castling Move Analysis ===\n";
        for (const auto& m : list.v) {
            std::string move_str = move_to_string(m);
            if (move_str == "e1g1" || move_str == "e1c1") {
                State st{};
                make_move(pos, m, st);
                uint64_t nodes = perft_divide(pos, 1, false);
                unmake_move(pos, m, st);
                std::cout << "Castling " << move_str << ": " << nodes << " moves\n";
            }
        }
    }
}

// Test to help identify the exact 4 extra moves in Kiwipete
TEST(Perft, KiwipeteDetailedAnalysis) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::cout << "\n=== Detailed Kiwipete Analysis ===\n";
    
    // Get all depth 1 moves and their depth 2 counts
    MoveList list; 
    generate_legal_moves(pos, list);
    
    std::vector<std::pair<std::string, uint64_t>> move_results;
    uint64_t total = 0;
    
    for (const auto& m : list.v) {
        State st{};
        make_move(pos, m, st);
        uint64_t nodes = perft(pos, 1);
        unmake_move(pos, m, st);
        
        std::string move_str = move_to_string(m);
        move_results.push_back({move_str, nodes});
        total += nodes;
    }
    
    // Sort by move string for consistent output
    std::sort(move_results.begin(), move_results.end());
    
    std::cout << "All moves with their depth 2 counts:\n";
    for (const auto& result : move_results) {
        std::cout << result.first << ": " << result.second << "\n";
    }
    
    std::cout << "\nTotal moves at depth 1: " << list.v.size() << "\n";
    std::cout << "Total nodes at depth 2: " << total << " (expected: 2039)\n";
    std::cout << "Extra moves: " << (int64_t)total - 2039 << "\n";
    
    // Look for the moves with highest counts - these might be the source of extra moves
    std::cout << "\n=== Moves with high depth-2 counts (>= 45) ===\n";
    for (const auto& result : move_results) {
        if (result.second >= 45) {
            std::cout << result.first << ": " << result.second << " (suspicious)\n";
        }
    }
}

// Test to examine the specific problematic moves in detail
TEST(Perft, ExamineSuspiciousMoves) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::vector<std::string> suspicious_moves = {"d5e6", "e5d7", "e5f7", "f3f5"};
    
    std::cout << "\n=== Examining Suspicious Moves ===\n";
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    for (const std::string& target_move : suspicious_moves) {
        for (const auto& m : list.v) {
            if (move_to_string(m) == target_move) {
                std::cout << "\n--- Move " << target_move << " ---\n";
                
                // Show position before move
                std::cout << "Before move:\n";
                std::cout << "From piece: " << char(to_char(pos.at(m.get_from()))) << "\n";
                std::cout << "To piece: " << char(to_char(pos.at(m.get_to()))) << "\n";
                std::cout << "Is capture: " << (m.is_capture() ? "Yes" : "No") << "\n";
                
                // Make the move and check resulting position
                State st{};
                make_move(pos, m, st);
                
                // Generate moves in resulting position
                MoveList response_list;
                generate_legal_moves(pos, response_list);
                
                std::cout << "Moves available after " << target_move << ": " << response_list.v.size() << "\n";
                
                // Show first few response moves for debugging
                std::cout << "First 5 response moves: ";
                for (size_t i = 0; i < std::min(size_t(5), response_list.v.size()); i++) {
                    std::cout << move_to_string(response_list.v[i]) << " ";
                }
                std::cout << "\n";
                
                unmake_move(pos, m, st);
                break;
            }
        }
    }
}

// Test to check for duplicate moves in suspicious positions
TEST(Perft, CheckForDuplicateMoves) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    // Check the first suspicious move: d5e6
    std::cout << "\n=== Checking for duplicate moves after d5e6 ===\n";
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    for (const auto& m : list.v) {
        if (move_to_string(m) == "d5e6") {
            State st{};
            make_move(pos, m, st);
            
            // Generate moves in resulting position
            MoveList response_list;
            generate_legal_moves(pos, response_list);
            
            // Check for duplicates
            std::vector<std::string> move_strings;
            for (const auto& response_move : response_list.v) {
                move_strings.push_back(move_to_string(response_move));
            }
            
            std::sort(move_strings.begin(), move_strings.end());
            
            bool found_duplicates = false;
            for (size_t i = 1; i < move_strings.size(); i++) {
                if (move_strings[i] == move_strings[i-1]) {
                    std::cout << "DUPLICATE MOVE FOUND: " << move_strings[i] << "\n";
                    found_duplicates = true;
                }
            }
            
            if (!found_duplicates) {
                std::cout << "No duplicate moves found. Total unique moves: " << move_strings.size() << "\n";
                
                // Show all moves to see what's happening
                std::cout << "All moves after d5e6:\n";
                for (const auto& move_str : move_strings) {
                    std::cout << move_str << " ";
                }
                std::cout << "\n";
            }
            
            unmake_move(pos, m, st);
            break;
        }
    }
}

// Test to check if any generated moves leave the king in check (illegal moves)
TEST(Perft, CheckForIllegalMoves) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    // Check the suspicious move: d5e6
    std::cout << "\n=== Checking for illegal moves after d5e6 ===\n";
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    for (const auto& m : list.v) {
        if (move_to_string(m) == "d5e6") {
            State st{};
            make_move(pos, m, st);
            
            // Generate moves in resulting position  
            MoveList response_list;
            generate_legal_moves(pos, response_list);
            
            // Check each move to see if it leaves the king in check
            int illegal_count = 0;
            std::vector<std::string> illegal_moves;
            
            for (const auto& response_move : response_list.v) {
                State st2{};
                make_move(pos, response_move, st2);
                
                // Check if the king is in check after this move (which would make it illegal)
                Color moving_side = !pos.side_to_move; // The side that just moved
                Color opponent = !moving_side;
                int king_sq = pos.king_sq[static_cast<int>(moving_side)];
                
                bool king_in_check = SqAttacked(king_sq, pos, opponent);
                
                if (king_in_check) {
                    illegal_moves.push_back(move_to_string(response_move));
                    illegal_count++;
                }
                
                unmake_move(pos, response_move, st2);
            }
            
            std::cout << "Found " << illegal_count << " illegal moves (king left in check):\n";
            for (const auto& illegal_move : illegal_moves) {
                std::cout << "  " << illegal_move << "\n";
            }
            
            std::cout << "Legal moves: " << (response_list.v.size() - illegal_count) << "\n";
            std::cout << "Total generated: " << response_list.v.size() << "\n";
            
            unmake_move(pos, m, st);
            break;
        }
    }
}

// Test to check castling rights and moves in suspicious positions
TEST(Perft, CheckCastlingInSuspiciousPosition) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::cout << "\n=== Checking castling after d5e6 ===\n";
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    for (const auto& m : list.v) {
        if (move_to_string(m) == "d5e6") {
            State st{};
            make_move(pos, m, st);
            
            std::cout << "Castling rights after d5e6: ";
            if (pos.castling_rights & CASTLE_BK) std::cout << "k";
            if (pos.castling_rights & CASTLE_BQ) std::cout << "q";
            if (pos.castling_rights == 0) std::cout << "none";
            std::cout << "\n";
            
            // Generate moves and count castling moves
            MoveList response_list;
            generate_legal_moves(pos, response_list);
            
            int castling_moves = 0;
            std::vector<std::string> castling_move_list;
            
            for (const auto& response_move : response_list.v) {
                std::string move_str = move_to_string(response_move);
                if (move_str == "e8g8" || move_str == "e8c8") {
                    castling_moves++;
                    castling_move_list.push_back(move_str);
                    
                    // Verify this is actually a castle move, not just king move
                    if (response_move.is_castle()) {
                        std::cout << "Castling move found: " << move_str << " (encoded as castle)\n";
                    } else {
                        std::cout << "King move found: " << move_str << " (NOT encoded as castle - problem!)\n";
                    }
                }
            }
            
            std::cout << "Total castling moves found: " << castling_moves << "\n";
            std::cout << "Total moves in position: " << response_list.v.size() << "\n";
            
            unmake_move(pos, m, st);
            break;
        }
    }
}
