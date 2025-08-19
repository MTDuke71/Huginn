#include "debug.hpp"
#include "position.hpp"
#include "board120.hpp"
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "chess_types.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <algorithm>

namespace Debug {

bool validate_position_consistency(const Position& pos, const std::string& expected_fen) {
    std::cout << "\n=== Position Consistency Validation ===" << std::endl;
    std::cout << "Expected FEN: " << expected_fen << std::endl;
    
    // Parse the expected FEN to create a reference position
    Position expected_pos;
    expected_pos.reset();
    
    if (!expected_pos.set_from_fen(expected_fen)) {
        std::cout << "ERROR: Invalid FEN string provided" << std::endl;
        assert(false && "Invalid FEN string");
        return false;
    }
    
    bool all_consistent = true;
    
    // Validate all internal consistency first
    if (!validate_bitboards_consistency(pos)) {
        std::cout << "FAILED: Bitboards consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_piece_counts_consistency(pos)) {
        std::cout << "FAILED: Piece counts consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_piece_lists_consistency(pos)) {
        std::cout << "FAILED: Piece lists consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_material_scores_consistency(pos)) {
        std::cout << "FAILED: Material scores consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_king_squares_consistency(pos)) {
        std::cout << "FAILED: King squares consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_zobrist_consistency(pos)) {
        std::cout << "FAILED: Zobrist hash consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_en_passant_consistency(pos)) {
        std::cout << "FAILED: En passant consistency check" << std::endl;
        all_consistent = false;
    }
    
    if (!validate_castling_consistency(pos)) {
        std::cout << "FAILED: Castling consistency check" << std::endl;
        all_consistent = false;
    }
    
    // Compare against expected FEN position
    std::cout << "\n--- Comparing against expected FEN ---" << std::endl;
    
    // Check board squares
    for (int sq = 0; sq < 120; ++sq) {
        if (is_playable(sq)) {
            Piece actual = pos.board[sq];
            Piece expected = expected_pos.board[sq];
            
            if (actual != expected) {
                File file_enum = file_of(sq);
                Rank rank_enum = rank_of(sq);
                int file = int(file_enum);
                int rank = int(rank_enum);
                char file_char = 'a' + file;
                char rank_char = '1' + rank;
                
                std::cout << "BOARD MISMATCH at " << file_char << rank_char 
                         << ": expected " << to_char(expected) 
                         << ", got " << to_char(actual) << std::endl;
                assert(false && "Board square mismatch with expected FEN");
                all_consistent = false;
            }
        }
    }
    
    // Check side to move
    if (pos.side_to_move != expected_pos.side_to_move) {
        std::cout << "SIDE TO MOVE MISMATCH: expected " 
                 << (expected_pos.side_to_move == Color::White ? "White" : "Black")
                 << ", got " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
        assert(false && "Side to move mismatch");
        all_consistent = false;
    }
    
    // Check castling rights
    if (pos.castling_rights != expected_pos.castling_rights) {
        std::cout << "CASTLING RIGHTS MISMATCH: expected " << (int)expected_pos.castling_rights
                 << ", got " << (int)pos.castling_rights << std::endl;
        assert(false && "Castling rights mismatch");
        all_consistent = false;
    }
    
    // Check en passant square
    if (pos.ep_square != expected_pos.ep_square) {
        std::cout << "EN PASSANT MISMATCH: expected " << expected_pos.ep_square
                 << ", got " << pos.ep_square << std::endl;
        assert(false && "En passant square mismatch");
        all_consistent = false;
    }
    
    // Check halfmove clock
    if (pos.halfmove_clock != expected_pos.halfmove_clock) {
        std::cout << "HALFMOVE CLOCK MISMATCH: expected " << expected_pos.halfmove_clock
                 << ", got " << pos.halfmove_clock << std::endl;
        assert(false && "Halfmove clock mismatch");
        all_consistent = false;
    }
    
    // Check fullmove number
    if (pos.fullmove_number != expected_pos.fullmove_number) {
        std::cout << "FULLMOVE NUMBER MISMATCH: expected " << expected_pos.fullmove_number
                 << ", got " << pos.fullmove_number << std::endl;
        assert(false && "Fullmove number mismatch");
        all_consistent = false;
    }
    
    if (all_consistent) {
        std::cout << "✓ All consistency checks PASSED" << std::endl;
    } else {
        std::cout << "✗ Some consistency checks FAILED" << std::endl;
    }
    
    std::cout << "===================================" << std::endl;
    return all_consistent;
}

bool validate_bitboards_consistency(const Position& pos) {
    std::cout << "\n--- Validating Bitboards Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // Reconstruct bitboards from board array
    uint64_t expected_white_pawns = 0ULL;
    uint64_t expected_black_pawns = 0ULL;
    
    for (int sq = 0; sq < 120; ++sq) {
        if (is_playable(sq)) {
            Piece piece = pos.board[sq];
            if (piece == Piece::WhitePawn) {
                int sq64 = MAILBOX_MAPS.to64[sq];
                expected_white_pawns |= (1ULL << sq64);
            } else if (piece == Piece::BlackPawn) {
                int sq64 = MAILBOX_MAPS.to64[sq];
                expected_black_pawns |= (1ULL << sq64);
            }
        }
    }
    
    uint64_t expected_all_pawns = expected_white_pawns | expected_black_pawns;
    
    // Check white pawns bitboard
    if (pos.get_white_pawns() != expected_white_pawns) {
        std::cout << "WHITE PAWNS BITBOARD MISMATCH:" << std::endl;
        std::cout << "  Expected: " << expected_white_pawns << std::endl;
        std::cout << "  Actual:   " << pos.get_white_pawns() << std::endl;
        assert(false && "White pawns bitboard inconsistent with board array");
        consistent = false;
    }
    
    // Check black pawns bitboard
    if (pos.get_black_pawns() != expected_black_pawns) {
        std::cout << "BLACK PAWNS BITBOARD MISMATCH:" << std::endl;
        std::cout << "  Expected: " << expected_black_pawns << std::endl;
        std::cout << "  Actual:   " << pos.get_black_pawns() << std::endl;
        assert(false && "Black pawns bitboard inconsistent with board array");
        consistent = false;
    }
    
    // Check combined pawns bitboard
    if (pos.get_all_pawns_bitboard() != expected_all_pawns) {
        std::cout << "ALL PAWNS BITBOARD MISMATCH:" << std::endl;
        std::cout << "  Expected: " << expected_all_pawns << std::endl;
        std::cout << "  Actual:   " << pos.get_all_pawns_bitboard() << std::endl;
        assert(false && "Combined pawns bitboard inconsistent");
        consistent = false;
    }
    
    // Verify that all_pawns_bb equals white | black
    uint64_t union_pawns = pos.get_white_pawns() | pos.get_black_pawns();
    if (pos.get_all_pawns_bitboard() != union_pawns) {
        std::cout << "ALL PAWNS BITBOARD NOT UNION OF WHITE AND BLACK:" << std::endl;
        std::cout << "  All pawns:     " << pos.get_all_pawns_bitboard() << std::endl;
        std::cout << "  White | Black: " << union_pawns << std::endl;
        assert(false && "Combined pawns bitboard not union of individual bitboards");
        consistent = false;
    }
    
    if (consistent) {
        std::cout << "✓ Bitboards are consistent with board array" << std::endl;
    }
    
    return consistent;
}

bool validate_piece_counts_consistency(const Position& pos) {
    std::cout << "\n--- Validating Piece Counts Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // Count pieces on the board
    int actual_counts[2][7] = {{0}}; // [color][piece_type]
    
    for (int sq = 0; sq < 120; ++sq) {
        if (is_playable(sq)) {
            Piece piece = pos.board[sq];
            if (piece != Piece::None) {
                Color color = color_of(piece);
                PieceType type = type_of(piece);
                actual_counts[int(color)][int(type)]++;
            }
        }
    }
    
    // Check piece counts
    for (int c = 0; c < 2; ++c) {
        Color color = Color(c);
        for (int pt = 1; pt < int(PieceType::_Count); ++pt) {
            PieceType piece_type = PieceType(pt);
            int expected = actual_counts[c][pt];
            int stored = pos.pCount[c][pt];
            
            if (expected != stored) {
                std::cout << "PIECE COUNT MISMATCH for " 
                         << (color == Color::White ? "White" : "Black") << " "
                         << to_char(make_piece(color, piece_type)) << ":" << std::endl;
                std::cout << "  Expected: " << expected << std::endl;
                std::cout << "  Stored:   " << stored << std::endl;
                assert(false && "Piece count mismatch");
                consistent = false;
            }
        }
    }
    
    if (consistent) {
        std::cout << "✓ Piece counts are consistent with board array" << std::endl;
    }
    
    return consistent;
}

bool validate_piece_lists_consistency(const Position& pos) {
    std::cout << "\n--- Validating Piece Lists Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // For each piece type and color, collect squares from board and compare with piece lists
    for (int c = 0; c < 2; ++c) {
        Color color = Color(c);
        for (int pt = 1; pt < int(PieceType::_Count); ++pt) {
            PieceType piece_type = PieceType(pt);
            Piece target_piece = make_piece(color, piece_type);
            
            // Collect squares from board array
            std::vector<int> board_squares;
            for (int sq = 0; sq < 120; ++sq) {
                if (is_playable(sq) && pos.board[sq] == target_piece) {
                    board_squares.push_back(sq);
                }
            }
            
            // Get squares from piece list
            std::vector<int> list_squares;
            int count = pos.pCount[c][pt];
            for (int i = 0; i < count; ++i) {
                int sq = pos.pList[c][pt][i];
                list_squares.push_back(sq);
            }
            
            // Sort both lists for comparison
            std::sort(board_squares.begin(), board_squares.end());
            std::sort(list_squares.begin(), list_squares.end());
            
            if (board_squares != list_squares) {
                std::cout << "PIECE LIST MISMATCH for " 
                         << (color == Color::White ? "White" : "Black") << " "
                         << to_char(make_piece(color, piece_type)) << ":" << std::endl;
                std::cout << "  Board squares: ";
                for (int sq : board_squares) {
                    std::cout << sq << " ";
                }
                std::cout << std::endl;
                std::cout << "  List squares:  ";
                for (int sq : list_squares) {
                    std::cout << sq << " ";
                }
                std::cout << std::endl;
                assert(false && "Piece list mismatch");
                consistent = false;
            }
        }
    }
    
    if (consistent) {
        std::cout << "✓ Piece lists are consistent with board array" << std::endl;
    }
    
    return consistent;
}

bool validate_material_scores_consistency(const Position& pos) {
    std::cout << "\n--- Validating Material Scores Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // Calculate expected material scores
    int expected_material[2] = {0, 0};
    
    for (int sq = 0; sq < 120; ++sq) {
        if (is_playable(sq)) {
            Piece piece = pos.board[sq];
            if (piece != Piece::None) {
                Color color = color_of(piece);
                PieceType type = type_of(piece);
                
                // Exclude kings from material score (as per our implementation)
                if (type != PieceType::King) {
                    expected_material[int(color)] += value_of(piece);
                }
            }
        }
    }
    
    // Check material scores
    for (int c = 0; c < 2; ++c) {
        Color color = Color(c);
        int expected = expected_material[c];
        int stored = pos.get_material_score(color);
        
        if (expected != stored) {
            std::cout << "MATERIAL SCORE MISMATCH for " 
                     << (color == Color::White ? "White" : "Black") << ":" << std::endl;
            std::cout << "  Expected: " << expected << std::endl;
            std::cout << "  Stored:   " << stored << std::endl;
            assert(false && "Material score mismatch");
            consistent = false;
        }
    }
    
    // Check material balance
    int expected_balance = expected_material[0] - expected_material[1];
    int stored_balance = pos.get_material_balance();
    
    if (expected_balance != stored_balance) {
        std::cout << "MATERIAL BALANCE MISMATCH:" << std::endl;
        std::cout << "  Expected: " << expected_balance << std::endl;
        std::cout << "  Stored:   " << stored_balance << std::endl;
        assert(false && "Material balance mismatch");
        consistent = false;
    }
    
    if (consistent) {
        std::cout << "✓ Material scores are consistent with board array" << std::endl;
    }
    
    return consistent;
}

bool validate_king_squares_consistency(const Position& pos) {
    std::cout << "\n--- Validating King Squares Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // Find kings on the board
    int found_white_king = -1;
    int found_black_king = -1;
    
    for (int sq = 0; sq < 120; ++sq) {
        if (is_playable(sq)) {
            if (pos.board[sq] == Piece::WhiteKing) {
                if (found_white_king != -1) {
                    std::cout << "MULTIPLE WHITE KINGS found at " << found_white_king 
                             << " and " << sq << std::endl;
                    assert(false && "Multiple white kings on board");
                    consistent = false;
                }
                found_white_king = sq;
            } else if (pos.board[sq] == Piece::BlackKing) {
                if (found_black_king != -1) {
                    std::cout << "MULTIPLE BLACK KINGS found at " << found_black_king 
                             << " and " << sq << std::endl;
                    assert(false && "Multiple black kings on board");
                    consistent = false;
                }
                found_black_king = sq;
            }
        }
    }
    
    // Check white king square
    if (found_white_king != pos.king_sq[int(Color::White)]) {
        std::cout << "WHITE KING SQUARE MISMATCH:" << std::endl;
        std::cout << "  Found on board: " << found_white_king << std::endl;
        std::cout << "  Stored:         " << pos.king_sq[int(Color::White)] << std::endl;
        assert(false && "White king square mismatch");
        consistent = false;
    }
    
    // Check black king square
    if (found_black_king != pos.king_sq[int(Color::Black)]) {
        std::cout << "BLACK KING SQUARE MISMATCH:" << std::endl;
        std::cout << "  Found on board: " << found_black_king << std::endl;
        std::cout << "  Stored:         " << pos.king_sq[int(Color::Black)] << std::endl;
        assert(false && "Black king square mismatch");
        consistent = false;
    }
    
    if (consistent) {
        std::cout << "✓ King squares are consistent with board array" << std::endl;
    }
    
    return consistent;
}

bool validate_zobrist_consistency(const Position& pos) {
    std::cout << "\n--- Validating Zobrist Hash Consistency ---" << std::endl;
    
    // Create a copy and recalculate Zobrist hash from scratch
    Position temp_pos = pos;
    temp_pos.update_zobrist_key();
    
    bool consistent = (pos.zobrist_key == temp_pos.zobrist_key);
    
    if (!consistent) {
        std::cout << "ZOBRIST HASH MISMATCH:" << std::endl;
        std::cout << "  Current:      0x" << std::hex << pos.zobrist_key << std::endl;
        std::cout << "  Recalculated: 0x" << std::hex << temp_pos.zobrist_key << std::dec << std::endl;
        assert(false && "Zobrist hash mismatch");
    } else {
        std::cout << "✓ Zobrist hash is consistent (0x" << std::hex << pos.zobrist_key << std::dec << ")" << std::endl;
    }
    
    return consistent;
}

bool validate_en_passant_consistency(const Position& pos) {
    std::cout << "\n--- Validating En Passant Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // If no en passant square is set, that's always valid
    if (pos.ep_square == -1) {
        std::cout << "✓ No en passant square set (valid)" << std::endl;
        return true;
    }
    
    // En passant square must be on a playable square
    if (!is_playable(pos.ep_square)) {
        std::cout << "EN PASSANT SQUARE NOT PLAYABLE: " << pos.ep_square << std::endl;
        consistent = false;
        return consistent;
    }
    
    // Get the rank of the en passant square
    Rank ep_rank = rank_of(pos.ep_square);
    
    // Validate en passant rank based on side to move:
    // - If White to move: Black just moved, so ep square should be on rank 6 (Black's 3rd rank)
    // - If Black to move: White just moved, so ep square should be on rank 3 (White's 4th rank)
    Rank expected_rank;
    if (pos.side_to_move == Color::White) {
        expected_rank = Rank::R6;  // Black pawn moved from rank 7 to rank 5, ep square on rank 6
    } else {
        expected_rank = Rank::R3;  // White pawn moved from rank 2 to rank 4, ep square on rank 3
    }
    
    if (ep_rank != expected_rank) {
        std::cout << "EN PASSANT RANK MISMATCH:" << std::endl;
        std::cout << "  Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
        std::cout << "  EP square: " << pos.ep_square << " (rank " << int(ep_rank) + 1 << ")" << std::endl;
        std::cout << "  Expected rank: " << int(expected_rank) + 1 << std::endl;
        std::cout << "  Logic: If " << (pos.side_to_move == Color::White ? "White" : "Black") 
                 << " to move, then " << (pos.side_to_move == Color::White ? "Black" : "White")
                 << " just moved a pawn 2 squares" << std::endl;
        consistent = false;
    }
    
    // Additional validation: Check that the en passant square is actually empty
    if (pos.board[pos.ep_square] != Piece::None) {
        std::cout << "EN PASSANT SQUARE NOT EMPTY:" << std::endl;
        std::cout << "  EP square " << pos.ep_square << " contains: " << to_char(pos.board[pos.ep_square]) << std::endl;
        consistent = false;
    }
    
    // Additional validation: Check that there's a pawn of the opposite color on the adjacent rank
    // that could have made the two-square move
    Color ep_pawn_color = (pos.side_to_move == Color::White) ? Color::Black : Color::White;
    Piece expected_pawn = make_piece(ep_pawn_color, PieceType::Pawn);
    
    // Calculate where the pawn should be
    int pawn_square;
    if (pos.side_to_move == Color::White) {
        // Black pawn should be on rank 5 (one rank behind the ep square on rank 6)
        pawn_square = pos.ep_square - 10;  // Move one rank down from ep square
    } else {
        // White pawn should be on rank 4 (one rank forward from ep square on rank 3)  
        pawn_square = pos.ep_square + 10;  // Move one rank up from ep square
    }
    
    if (is_playable(pawn_square) && pos.board[pawn_square] != expected_pawn) {
        std::cout << "EN PASSANT PAWN MISSING:" << std::endl;
        std::cout << "  Expected " << to_char(expected_pawn) << " at square " << pawn_square << std::endl;
        std::cout << "  Found: " << to_char(pos.board[pawn_square]) << std::endl;
        consistent = false;
    }
    
    if (consistent) {
        std::cout << "✓ En passant square is valid (square " << pos.ep_square 
                 << " on rank " << int(ep_rank) + 1 << ")" << std::endl;
    }
    
    return consistent;
}

bool validate_castling_consistency(const Position& pos) {
    std::cout << "\n--- Validating Castling Consistency ---" << std::endl;
    
    bool consistent = true;
    
    // Starting squares for castling validation
    const int WHITE_KING_START = sq(File::E, Rank::R1);  // e1
    const int WHITE_ROOK_KS = sq(File::H, Rank::R1);     // h1 (kingside)
    const int WHITE_ROOK_QS = sq(File::A, Rank::R1);     // a1 (queenside)
    const int BLACK_KING_START = sq(File::E, Rank::R8);  // e8
    const int BLACK_ROOK_KS = sq(File::H, Rank::R8);     // h8 (kingside)
    const int BLACK_ROOK_QS = sq(File::A, Rank::R8);     // a8 (queenside)
    
    // Check White castling rights
    if (pos.castling_rights & CASTLE_WK) {
        // White kingside castling allowed - check king and kingside rook
        if (pos.board[WHITE_KING_START] != Piece::WhiteKing) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  White kingside castling allowed but King not on e1" << std::endl;
            std::cout << "  Expected WhiteKing at e1, found: " << to_char(pos.board[WHITE_KING_START]) << std::endl;
            consistent = false;
        }
        if (pos.board[WHITE_ROOK_KS] != Piece::WhiteRook) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  White kingside castling allowed but Rook not on h1" << std::endl;
            std::cout << "  Expected WhiteRook at h1, found: " << to_char(pos.board[WHITE_ROOK_KS]) << std::endl;
            consistent = false;
        }
    }
    
    if (pos.castling_rights & CASTLE_WQ) {
        // White queenside castling allowed - check king and queenside rook
        if (pos.board[WHITE_KING_START] != Piece::WhiteKing) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  White queenside castling allowed but King not on e1" << std::endl;
            std::cout << "  Expected WhiteKing at e1, found: " << to_char(pos.board[WHITE_KING_START]) << std::endl;
            consistent = false;
        }
        if (pos.board[WHITE_ROOK_QS] != Piece::WhiteRook) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  White queenside castling allowed but Rook not on a1" << std::endl;
            std::cout << "  Expected WhiteRook at a1, found: " << to_char(pos.board[WHITE_ROOK_QS]) << std::endl;
            consistent = false;
        }
    }
    
    // Check Black castling rights
    if (pos.castling_rights & CASTLE_BK) {
        // Black kingside castling allowed - check king and kingside rook
        if (pos.board[BLACK_KING_START] != Piece::BlackKing) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  Black kingside castling allowed but King not on e8" << std::endl;
            std::cout << "  Expected BlackKing at e8, found: " << to_char(pos.board[BLACK_KING_START]) << std::endl;
            consistent = false;
        }
        if (pos.board[BLACK_ROOK_KS] != Piece::BlackRook) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  Black kingside castling allowed but Rook not on h8" << std::endl;
            std::cout << "  Expected BlackRook at h8, found: " << to_char(pos.board[BLACK_ROOK_KS]) << std::endl;
            consistent = false;
        }
    }
    
    if (pos.castling_rights & CASTLE_BQ) {
        // Black queenside castling allowed - check king and queenside rook
        if (pos.board[BLACK_KING_START] != Piece::BlackKing) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  Black queenside castling allowed but King not on e8" << std::endl;
            std::cout << "  Expected BlackKing at e8, found: " << to_char(pos.board[BLACK_KING_START]) << std::endl;
            consistent = false;
        }
        if (pos.board[BLACK_ROOK_QS] != Piece::BlackRook) {
            std::cout << "CASTLING INCONSISTENCY:" << std::endl;
            std::cout << "  Black queenside castling allowed but Rook not on a8" << std::endl;
            std::cout << "  Expected BlackRook at a8, found: " << to_char(pos.board[BLACK_ROOK_QS]) << std::endl;
            consistent = false;
        }
    }
    
    if (consistent) {
        if (pos.castling_rights == CASTLE_NONE) {
            std::cout << "✓ No castling rights set (valid)" << std::endl;
        } else {
            std::cout << "✓ Castling rights are consistent with piece positions" << std::endl;
            std::cout << "  Rights: ";
            if (pos.castling_rights & CASTLE_WK) std::cout << "K";
            if (pos.castling_rights & CASTLE_WQ) std::cout << "Q";
            if (pos.castling_rights & CASTLE_BK) std::cout << "k";
            if (pos.castling_rights & CASTLE_BQ) std::cout << "q";
            std::cout << std::endl;
        }
    }
    
    return consistent;
}

} // namespace Debug
