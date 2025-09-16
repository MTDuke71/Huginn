#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <map>

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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

// Perft divide function
uint64_t perft_divide(BitboardPosition& pos, int depth, const std::string& move_prefix = "", bool print_top_level = false) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        // Make move and recurse
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft_divide(pos, depth - 1);
        pos.unmake_move(simple_move, undo);

        if (print_top_level && depth == 3) {  // Print top-level moves for depth 3
            std::cout << move_str << " - " << nodes << "\n";
        }

        total_nodes += nodes;
    }

    return total_nodes;
}

int main() {
    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Position: " << fen << "\n\n";
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }

    // Quick sanity: verify piece on e5 is a Knight and membership in bitboards
    {
        uint64_t white_knights = pos.get_pieces(Color::White, PieceType::Knight);
        uint64_t white_rooks = pos.get_pieces(Color::White, PieceType::Rook);
        uint64_t white_queens = pos.get_pieces(Color::White, PieceType::Queen);
        uint64_t white_bishops = pos.get_pieces(Color::White, PieceType::Bishop);
        uint64_t white_pawns = pos.get_pieces(Color::White, PieceType::Pawn);
        
        std::cout << "WHITE PIECE BITBOARDS:\n";
        std::cout << "  Knights: 0x" << std::hex << white_knights << std::dec << "\n";
        std::cout << "  Rooks:   0x" << std::hex << white_rooks << std::dec << "\n";
        std::cout << "  Queens:  0x" << std::hex << white_queens << std::dec << "\n";
        std::cout << "  Bishops: 0x" << std::hex << white_bishops << std::dec << "\n";
        std::cout << "  Pawns:   0x" << std::hex << white_pawns << std::dec << "\n";
        
        auto print_piece = [&](int sq, const char* name){
            Piece p = pos.piece_at(sq);
            std::cout << "Piece at " << name << " sq=" << sq << ": type=" << (int)type_of(p)
                      << ", color=" << (int)color_of(p)
                      << ", in KN bb=" << (((white_knights >> sq) & 1ULL) ? 1 : 0)
                      << ", in R bb=" << (((white_rooks >> sq) & 1ULL) ? 1 : 0)
                      << ", in Q bb=" << (((white_queens >> sq) & 1ULL) ? 1 : 0)
                      << "\n";
        };
        int e5 = 4 + 4 * 8; // 36
        int c3 = 2 + 2 * 8; // 18
        print_piece(e5, "e5");
        print_piece(c3, "c3");
    }

    // Test just depth 1 move generation to isolate the bug
    std::cout << "\n=== ISOLATED DEPTH 1 MOVE GENERATION TEST ===\n";
    BitboardMoveList test_moves;
    BitboardMoveGen::generate_legal_moves(pos, test_moves);
    std::cout << "Generated " << test_moves.moves.size() << " moves at depth 1:\n";
    
    // Convert to algebraic and check for e5 moves
    std::vector<std::string> found_e5_moves;
    for (size_t i = 0; i < test_moves.moves.size(); i++) {
        auto& move = test_moves.moves[i];
        // Simple algebraic conversion for 64-square format
        auto sq_to_alg = [](int sq) -> std::string {
            char file = 'a' + (sq % 8);
            char rank = '1' + (sq / 8);
            return std::string(1, file) + std::string(1, rank);
        };
        std::string move_str = sq_to_alg(move.from_64) + sq_to_alg(move.to_64);
        if (move.from_64 == 36) { // e5
            found_e5_moves.push_back(move_str);
        }
    }
    
    std::cout << "Found " << found_e5_moves.size() << " moves from e5: ";
    for (const auto& move : found_e5_moves) {
        std::cout << move << " ";
    }
    std::cout << "\n\n";
    std::cout << "\n=== ROOT PERFT DIVIDE AT DEPTH 4 (ENGINE VS REFERENCE) ===\n";
    // Reference results for depth 4 from user
    struct RootRefMove { std::string move; uint64_t ref_nodes; };
    std::vector<RootRefMove> root_ref = {
        {"a1b1",83348},{"a1c1",83263},{"a1d1",79695},{"a2a3",94405},{"a2a4",90978},{"b2b3",81066},{"c3a4",91447},{"c3b1",84773},{"c3b5",81498},{"c3d1",84782},
        {"d2c1",83037},{"d2e3",90274},{"d2f4",84869},{"d2g5",87951},{"d2h6",82323},{"d5d6",79551},{"d5e6",97464},{"e1c1",79803},{"e1d1",79989},{"e1f1",77887},
        {"e1g1",86975},{"e2a6",69334},{"e2b5",79739},{"e2c4",84835},{"e2d1",74963},{"e2d3",85119},{"e2f1",88728},{"e5c4",77752},{"e5c6",83885},{"e5d3",77431},
        {"e5d7",93913},{"e5f7",88799},{"e5g4",79912},{"e5g6",83866},{"f3d3",83727},{"f3e3",92505},{"f3f4",90488},{"f3f5",104992},{"f3f6",77838},{"f3g3",94461},
        {"f3g4",92037},{"f3h3",98524},{"f3h5",95034},{"g2g3",77468},{"g2g4",75677},{"g2h3",82759},{"h1f1",81563},{"h1g1",84876}
    };
    std::map<std::string, uint64_t> root_ref_map;
    for (const auto& r : root_ref) root_ref_map[r.move] = r.ref_nodes;
    // Generate all legal moves at root
    BitboardMoveList root_moves;
    generate_legal_moves(pos, root_moves);
    std::map<std::string, uint64_t> root_engine_map;
    uint64_t total_engine = 0, total_ref = 0;
    for (const auto& move : root_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft_divide(pos, 3);
        pos.unmake_move(simple_move, undo);
        root_engine_map[move_str] = nodes;
        total_engine += nodes;
    }
    // Print comparison, highlight mismatches, missing, and extra moves
    std::cout << "\nMove         Engine      Ref      Diff\n";
    std::cout << "------------------------------------------\n";
    for (const auto& r : root_ref) {
        uint64_t engine_nodes = root_engine_map.count(r.move) ? root_engine_map[r.move] : 0;
        int64_t diff = (int64_t)engine_nodes - (int64_t)r.ref_nodes;
        std::string flag = (engine_nodes == r.ref_nodes) ? "" : (root_engine_map.count(r.move) ? " <-- MISMATCH" : " <-- MISSING");
        std::cout << std::setw(8) << r.move << std::setw(12) << engine_nodes << std::setw(10) << r.ref_nodes << std::setw(10) << diff << flag << "\n";
        total_ref += r.ref_nodes;
    }
    // Print any extra moves generated by engine
    for (const auto& e : root_engine_map) {
        if (!root_ref_map.count(e.first)) {
            std::cout << std::setw(8) << e.first << std::setw(12) << e.second << std::setw(10) << 0 << std::setw(10) << e.second << " <-- EXTRA\n";
            total_engine += e.second;
        }
    }
    std::cout << "------------------------------------------\n";
    std::cout << std::setw(8) << "TOTAL" << std::setw(12) << total_engine << std::setw(10) << total_ref << std::setw(10) << ((int64_t)total_engine - (int64_t)total_ref) << "\n";
    // --- End: Root perft divide at depth 4 ---
    
    // Test depths 1, 2, 3
    std::vector<std::pair<int, uint64_t>> depth_tests = {
        {1, 48}, {2, 2039}, {3, 97862}
    };
    
    for (const auto& test : depth_tests) {
        uint64_t result = perft_divide(pos, test.first);
        std::cout << "Depth " << test.first << ": " << result;
        if (result == test.second) {
            std::cout << " ✅ CORRECT\n";
        } else {
            std::cout << " ❌ Expected: " << test.second << " (diff: " << (int64_t)result - (int64_t)test.second << ")\n";
        }
    }
    
    std::cout << "\n=== DETAILED ANALYSIS OF f3f5 POSITION ===\n";
    
    // Make the f3f5 move to analyze the position after it
    SimpleBitboardMove f3f5_move;
    f3f5_move.from_64 = 21;  // f3
    f3f5_move.to_64 = 37;    // f5
    f3f5_move.is_capture = false;
    f3f5_move.is_ep_capture = false;
    f3f5_move.is_castling = false;
    f3f5_move.is_promotion = false;
    
    std::cout << "Analyzing position after f3f5 move...\n";
    BitboardPosition::UndoInfo f3f5_undo = pos.make_move_with_undo(f3f5_move);
    
    // Get FEN after f3f5
    std::string f3f5_fen = pos.to_fen();
    std::cout << "FEN after f3f5: " << f3f5_fen << "\n\n";
    
    // Check if kings are in check
    int white_king_sq = pos.king_square_64[0];
    int black_king_sq = pos.king_square_64[1];
    bool white_king_in_check = pos.is_square_attacked(white_king_sq, Color::Black);
    bool black_king_in_check = pos.is_square_attacked(black_king_sq, Color::White);
    
    std::cout << "White king at " << white_king_sq << " (square " << (white_king_sq % 8) + 'a' << (white_king_sq / 8) + 1 << ") - in check: " << (white_king_in_check ? "YES" : "NO") << "\n";
    std::cout << "Black king at " << black_king_sq << " (square " << (black_king_sq % 8) + 'a' << (black_king_sq / 8) + 1 << ") - in check: " << (black_king_in_check ? "YES" : "NO") << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    // Generate moves from this position
    BitboardMoveList f3f5_moves;
    generate_legal_moves(pos, f3f5_moves);
    std::cout << "Legal moves from f3f5 position: " << f3f5_moves.moves.size() << "\n";
    

    // Reference breakdown for f3f5 subtree at depth 3
    struct RefMove {
        std::string move;
        uint64_t ref_nodes;
    };
    std::vector<RefMove> f3f5_ref = {
        {"a6b5",2448},{"a6b7",2398},{"a6c4",2404},{"a6c8",2076},{"a6d3",2398},{"a6e2",2086},{"a8b8",2442},{"a8c8",2281},{"a8d8",2284},{"b4b3",2533},{"b4c3",2436},{"b6a4",2334},{"b6c4",2347},{"b6c8",2064},{"b6d5",2274},{"c7c5",2322},{"c7c6",2429},{"d7d6",2353},{"e6d5",2398},{"e6f5",2050},{"e7c5",2766},{"e7d6",2432},{"e7d8",2189},{"e7f8",2184},{"e8c8",2304},{"e8d8",2254},{"e8f8",2208},{"e8g8",2230},{"f6d5",2505},{"f6e4",2921},{"f6g4",2555},{"f6g8",2349},{"f6h5",2453},{"f6h7",2349},{"g6f5",2007},{"g6g5",2295},{"g7f8",2173},{"g7h6",2420},{"h3g2",2697},{"h8f8",2015},{"h8g8",2120},{"h8h4",2426},{"h8h5",2331},{"h8h6",2226},{"h8h7",2226}
    };
    // Map for quick lookup
    std::map<std::string, uint64_t> ref_map;
    for (const auto& r : f3f5_ref) ref_map[r.move] = r.ref_nodes;

    // Gather engine results
    std::map<std::string, uint64_t> engine_map;
    uint64_t f3f5_total = 0, f3f5_ref_total = 0;
    std::cout << "\nDepth 3 moves from f3f5 position (engine vs reference):\n";
    for (const auto& move : f3f5_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        // After f3f5 (ply 1), to reach overall depth 3 we need depth 2 from here
        uint64_t nodes = perft_divide(pos, 2);
        pos.unmake_move(simple_move, undo);
        engine_map[move_str] = nodes;
    }
    // Print comparison, highlight mismatches and missing/extra moves
    for (const auto& r : f3f5_ref) {
        uint64_t engine_nodes = engine_map.count(r.move) ? engine_map[r.move] : 0;
        std::string flag = (engine_nodes == r.ref_nodes) ? "" : " <-- MISMATCH";
        if (!engine_map.count(r.move)) flag = " <-- MISSING";
        std::cout << "  " << r.move << " - engine: " << engine_nodes << ", ref: " << r.ref_nodes << flag << "\n";
        f3f5_total += engine_nodes;
        f3f5_ref_total += r.ref_nodes;
    }
    // Print any extra moves generated by engine
    for (const auto& e : engine_map) {
        if (!ref_map.count(e.first)) {
            std::cout << "  " << e.first << " - engine: " << e.second << ", ref: 0 <-- EXTRA\n";
            f3f5_total += e.second;
        }
    }
    std::cout << "\nTotal engine nodes: " << f3f5_total << "\n";
    std::cout << "Total reference nodes: " << f3f5_ref_total << "\n";
    std::cout << "Difference: " << (int64_t)f3f5_total - (int64_t)f3f5_ref_total << "\n";
    
    // Unmake f3f5
    pos.unmake_move(f3f5_move, f3f5_undo);

    // --- Begin: Detailed perft divide for f3f5-h3g2 subtree ---
    std::cout << "\n=== DETAILED PERFT DIVIDE FOR f3f5-h3g2 SUBTREE (DEPTH 3) ===\n";
    {
        BitboardPosition pos_f3f5 = pos;
        BitboardPosition::UndoInfo undo_f3f5 = pos_f3f5.make_move_with_undo(f3f5_move);
        BitboardMoveList moves_after_f3f5;
        generate_legal_moves(pos_f3f5, moves_after_f3f5);
        // Find h3g2 move
        SimpleBitboardMove h3g2_move;
        bool found_h3g2 = false;
        for (const auto& move : moves_after_f3f5.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string move_str = move_to_string(simple_move);
            if (move_str == "h3g2") {
                h3g2_move = simple_move;
                found_h3g2 = true;
                break;
            }
        }
        if (!found_h3g2) {
            std::cout << "ERROR: h3g2 not found after f3f5!\n";
        } else {
            BitboardPosition::UndoInfo undo_h3g2 = pos_f3f5.make_move_with_undo(h3g2_move);
            // Now at f3f5-h3g2 position, White to move
            BitboardMoveList moves_after_h3g2;
            generate_legal_moves(pos_f3f5, moves_after_h3g2);
            std::cout << "Legal moves after f3f5-h3g2: " << moves_after_h3g2.moves.size() << "\n";
            // For each move, print move and perft to depth 2 (i.e., 2 plies after h3g2)
            std::map<std::string, uint64_t> engine_map;
            uint64_t total = 0;
            for (const auto& move : moves_after_h3g2.moves) {
                SimpleBitboardMove simple_move = convert_move(move);
                std::string move_str = move_to_string(simple_move);
                BitboardPosition::UndoInfo undo = pos_f3f5.make_move_with_undo(simple_move);
                std::string fen = pos_f3f5.to_fen();
                int white_king_sq = pos_f3f5.king_square_64[0];
                int black_king_sq = pos_f3f5.king_square_64[1];
                bool white_king_in_check = pos_f3f5.is_square_attacked(white_king_sq, Color::Black);
                bool black_king_in_check = pos_f3f5.is_square_attacked(black_king_sq, Color::White);
                uint64_t nodes = perft_divide(pos_f3f5, 2);
                pos_f3f5.unmake_move(simple_move, undo);
                engine_map[move_str] = nodes;
                total += nodes;
                std::cout << "  " << move_str << " - nodes: " << nodes << "\n";
                std::cout << "    FEN: " << fen << "\n";
                std::cout << "    White king in check: " << (white_king_in_check ? "YES" : "NO") << ", Black king in check: " << (black_king_in_check ? "YES" : "NO") << "\n";
            }
            std::cout << "Total nodes after f3f5-h3g2: " << total << "\n";
            pos_f3f5.unmake_move(h3g2_move, undo_h3g2);
        }
        // Don't forget to unmake f3f5 on the copy!
        pos_f3f5.unmake_move(f3f5_move, undo_f3f5);
    }
    // --- End: Detailed perft divide for f3f5-h3g2 subtree ---
    
    // --- Begin: Detailed perft divide for a1b1 subtree (overall depth 3) ---
    std::cout << "\n=== DETAILED PERFT DIVIDE FOR a1b1 SUBTREE (DEPTH 3) ===\n";
    {
        // Find a1b1 at root
        BitboardMoveList root_moves_for_a1b1;
        generate_legal_moves(pos, root_moves_for_a1b1);
        SimpleBitboardMove a1b1_move{};
        bool found_a1b1 = false;
        for (const auto& m : root_moves_for_a1b1.moves) {
            SimpleBitboardMove sm = convert_move(m);
            if (move_to_string(sm) == "a1b1") { a1b1_move = sm; found_a1b1 = true; break; }
        }
        if (!found_a1b1) {
            std::cout << "ERROR: a1b1 not found at root!\n";
        } else {
            BitboardPosition::UndoInfo undo_a1b1 = pos.make_move_with_undo(a1b1_move);
            // Now at a1b1 position (Black to move). Compare per-move counts at remaining depth 2.
            struct A1B1Ref { std::string move; uint64_t ref_nodes; };
            std::vector<A1B1Ref> a1b1_ref = {
                {"a6b5",2003},{"a6b7",1971},{"a6c4",1961},{"a6c8",1697},{"a6d3",1950},{"a6e2",1821},{"a8b8",2003},{"a8c8",1866},{"a8d8",1868},
                {"b4b3",2085},{"b4c3",2080},{"b6a4",1905},{"b6c4",1916},{"b6c8",1681},{"b6d5",1857},{"c7c5",1904},{"c7c6",1996},{"d7d6",1919},
                {"e6d5",2000},{"e7c5",2311},{"e7d6",2021},{"e7d8",1816},{"e7f8",1811},{"e8c8",1880},{"e8d8",1831},{"e8f8",1792},{"e8g8",1821},
                {"f6d5",2095},{"f6e4",2464},{"f6g4",2174},{"f6g8",1965},{"f6h5",2054},{"f6h7",1964},{"g6g5",1911},{"g7f8",1773},{"g7h6",1985},
                {"h3g2",2246},{"h8f8",1638},{"h8g8",1728},{"h8h4",1992},{"h8h5",1956},{"h8h6",1818},{"h8h7",1819}
            };
            std::map<std::string, uint64_t> ref_map_a1b1;
            for (const auto& r : a1b1_ref) ref_map_a1b1[r.move] = r.ref_nodes;

            BitboardMoveList after_a1b1_moves;
            generate_legal_moves(pos, after_a1b1_moves);
            std::map<std::string, uint64_t> engine_map_a1b1;
            uint64_t total_engine_a1b1 = 0, total_ref_a1b1 = 0;
            for (const auto& m : after_a1b1_moves.moves) {
                SimpleBitboardMove sm = convert_move(m);
                std::string ms = move_to_string(sm);
                BitboardPosition::UndoInfo u = pos.make_move_with_undo(sm);
                uint64_t nodes = perft_divide(pos, 2);
                pos.unmake_move(sm, u);
                engine_map_a1b1[ms] = nodes;
            }
            // Print diff
            for (const auto& r : a1b1_ref) {
                uint64_t eng = engine_map_a1b1.count(r.move) ? engine_map_a1b1[r.move] : 0;
                std::string flag = (eng == r.ref_nodes) ? "" : (engine_map_a1b1.count(r.move) ? " <-- MISMATCH" : " <-- MISSING");
                std::cout << "  " << r.move << " - engine: " << eng << ", ref: " << r.ref_nodes << flag << "\n";
                total_engine_a1b1 += eng;
                total_ref_a1b1 += r.ref_nodes;
            }
            // Extras
            for (const auto& e : engine_map_a1b1) {
                if (!ref_map_a1b1.count(e.first)) {
                    std::cout << "  " << e.first << " - engine: " << e.second << ", ref: 0 <-- EXTRA\n";
                    total_engine_a1b1 += e.second;
                }
            }
            std::cout << "\nTotal engine nodes (a1b1 depth3): " << total_engine_a1b1 << "\n";
            std::cout << "Total reference nodes (a1b1 depth3): " << total_ref_a1b1 << "\n";
            std::cout << "Difference: " << (int64_t)total_engine_a1b1 - (int64_t)total_ref_a1b1 << "\n";
            pos.unmake_move(a1b1_move, undo_a1b1);
        }
    }
    // --- End: Detailed perft divide for a1b1 subtree ---
    
    // Test attack detection on specific squares
    std::cout << "\n=== ATTACK DETECTION TEST ===\n";
    
    // Test squares around the black king
    int test_squares[] = {60, 61, 62, 59, 58, 52, 53, 54};
    const char* square_names[] = {"e8", "f8", "g8", "d8", "c8", "e7", "f7", "g7"};
    
    for (int i = 0; i < 8; i++) {
        int sq = test_squares[i];
        bool attacked_by_white = pos.is_square_attacked(sq, Color::White);
        bool attacked_by_black = pos.is_square_attacked(sq, Color::Black);
        std::cout << "Square " << square_names[i] << " (" << sq << "): attacked by White=" << (attacked_by_white ? "YES" : "NO")
                  << ", attacked by Black=" << (attacked_by_black ? "YES" : "NO") << "\n";
    }
    
    // Test some white piece attacks
    std::cout << "\nWhite piece attack patterns:\n";
    int white_piece_squares[] = {4, 5, 12, 13, 18, 37};  // e1, f1, e2, f2, c3, f5 (queen moved here)
    const char* piece_names[] = {"e1(king)", "f1", "e2", "f2", "c3(knight)", "f5(queen)"};
    
    for (int i = 0; i < 6; i++) {
        int sq = white_piece_squares[i];
        Piece piece = pos.piece_at(sq);
        if (piece != Piece::None) {
            std::cout << piece_names[i] << " (" << static_cast<int>(type_of(piece)) << ") attacks: ";
            int attack_count = 0;
            for (int target = 0; target < 64; target++) {
                if (pos.is_square_attacked(target, Color::White)) {
                    attack_count++;
                }
            }
            std::cout << attack_count << " squares\n";
        }
    }
    
    std::cout << "\n=== CONTINUING WITH DEPTH 4 ANALYSIS ===\n";    // Check depth 1 move count first
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    std::cout << "=== DEPTH 1 MOVE ANALYSIS ===\n";
    std::cout << "Moves generated: " << moves.moves.size() << "\n";
    std::cout << "Expected: 48\n";

    if (moves.moves.size() != 48) {
        std::cout << "❌ DEPTH 1 BUG FOUND! Wrong number of moves at depth 1.\n\n";

        std::cout << "Generated moves:\n";
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string move_str = move_to_string(simple_move);
            std::cout << move_str << " ";
        }
        std::cout << "\n\n";

        // Expected moves from reference
        std::vector<std::string> expected = {
            "a1b1", "a1c1", "a1d1", "a2a3", "a2a4", "b2b3", "c3a4", "c3b1", "c3b5", "c3d1",
            "d2c1", "d2e3", "d2f4", "d2g5", "d2h6", "d5d6", "d5e6", "e1c1", "e1d1", "e1f1",
            "e1g1", "e2a6", "e2b5", "e2c4", "e2d1", "e2d3", "e2f1", "e5c4", "e5c6", "e5d3",
            "e5d7", "e5f7", "e5g4", "e5g6", "f3d3", "f3e3", "f3f4", "f3f5", "f3f6", "f3g3",
            "f3g4", "f3h3", "f3h5", "g2g3", "g2g4", "g2h3", "h1f1", "h1g1"
        };

        std::cout << "Expected moves:\n";
        for (const auto& move : expected) {
            std::cout << move << " ";
        }
        std::cout << "\n\n";

    } else {
        std::cout << "✅ Depth 1 correct! The bug must be deeper.\n\n";

        // Run depth 4 perft divide and compare with expected results
        std::cout << "=== KIWIPETE DEPTH 4 DIVIDE COMPARISON ===\n";

        // Expected results from the reference you provided
        struct MoveResult { std::string move; uint64_t expected; };
        std::vector<MoveResult> expected_results = {
            {"a1b1", 83348}, {"a1c1", 83263}, {"a1d1", 79695}, {"a2a3", 94405}, {"a2a4", 90978},
            {"b2b3", 81066}, {"c3a4", 91447}, {"c3b1", 84773}, {"c3b5", 81498}, {"c3d1", 84782},
            {"d2c1", 83037}, {"d2e3", 90274}, {"d2f4", 84869}, {"d2g5", 87951}, {"d2h6", 82323},
            {"d5d6", 79551}, {"d5e6", 97464}, {"e1c1", 79803}, {"e1d1", 79989}, {"e1f1", 77887},
            {"e1g1", 86975}, {"e2a6", 69334}, {"e2b5", 79739}, {"e2c4", 84835}, {"e2d1", 74963},
            {"e2d3", 85119}, {"e2f1", 88728}, {"e5c4", 77752}, {"e5c6", 83885}, {"e5d3", 77431},
            {"e5d7", 93913}, {"e5f7", 88799}, {"e5g4", 79912}, {"e5g6", 83866}, {"f3d3", 83727},
            {"f3e3", 92505}, {"f3f4", 90488}, {"f3f5", 104992}, {"f3f6", 77838}, {"f3g3", 94461},
            {"f3g4", 92037}, {"f3h3", 98524}, {"f3h5", 95034}, {"g2g3", 77468}, {"g2g4", 75677},
            {"g2h3", 82759}, {"h1f1", 81563}, {"h1g1", 84876}
        };

        uint64_t total_actual = 0;
        uint64_t total_expected = 0;
        int mismatches = 0;

        for (const auto& move_result : expected_results) {
            // Find the move in our generated moves
            bool found = false;
            for (const auto& move : moves.moves) {
                SimpleBitboardMove simple_move = convert_move(move);
                std::string move_str = move_to_string(simple_move);

                if (move_str == move_result.move) {
                    found = true;

                    // Calculate actual nodes for this move
                    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
                    uint64_t actual = perft_divide(pos, 3, "", false);
                    pos.unmake_move(simple_move, undo);

                    total_actual += actual;
                    total_expected += move_result.expected;

                    if (actual != move_result.expected) {
                        mismatches++;
                        int64_t diff = (int64_t)actual - (int64_t)move_result.expected;
                        std::cout << move_result.move << ": " << actual << " (expected " << move_result.expected << ", diff " << diff << ")\n";
                    }
                    break;
                }
            }

            if (!found) {
                std::cout << "ERROR: Move " << move_result.move << " not found in generated moves!\n";
            }
        }

        std::cout << "\nSummary:\n";
        std::cout << "Total actual: " << total_actual << "\n";
        std::cout << "Total expected: " << total_expected << "\n";
        std::cout << "Difference: " << (int64_t)total_actual - (int64_t)total_expected << "\n";
        std::cout << "Mismatched moves: " << mismatches << "/" << expected_results.size() << "\n";
    }

    return 0;
}