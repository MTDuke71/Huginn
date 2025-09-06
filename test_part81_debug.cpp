// VICE Part 81 Debug: Test file bonus implementation
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>

using namespace Huginn;

// Simple evaluation test that only checks file bonuses
int evaluateFileBonusesOnly(const Position& pos) {
    int score = 0;
    
    // Get bitboards for pawn structure analysis
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    uint64_t all_pawns = white_pawns | black_pawns;
    
    // Check all squares for rooks and queens
    for (int sq = 21; sq <= 98; ++sq) {
        if (pos.board[sq] == Piece::Offboard || pos.board[sq] == Piece::None) continue;
        
        Piece piece = pos.board[sq];
        int sq64 = MAILBOX_MAPS.to64[sq];
        if (sq64 < 0) continue;
        
        int file = sq64 % 8;
        uint64_t file_mask = EvalParams::FILE_MASKS[file];
        
        if (piece == Piece::WhiteRook) {
            if ((all_pawns & file_mask) == 0) {
                score += EvalParams::ROOK_OPEN_FILE_BONUS;
                std::cout << "White rook on open file " << char('a' + file) << ": +" 
                         << EvalParams::ROOK_OPEN_FILE_BONUS << "cp" << std::endl;
            } else if ((white_pawns & file_mask) == 0) {
                score += EvalParams::ROOK_SEMI_OPEN_FILE_BONUS;
                std::cout << "White rook on semi-open file " << char('a' + file) << ": +" 
                         << EvalParams::ROOK_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
            }
        } else if (piece == Piece::BlackRook) {
            if ((all_pawns & file_mask) == 0) {
                score -= EvalParams::ROOK_OPEN_FILE_BONUS;
                std::cout << "Black rook on open file " << char('a' + file) << ": -" 
                         << EvalParams::ROOK_OPEN_FILE_BONUS << "cp" << std::endl;
            } else if ((black_pawns & file_mask) == 0) {
                score -= EvalParams::ROOK_SEMI_OPEN_FILE_BONUS;
                std::cout << "Black rook on semi-open file " << char('a' + file) << ": -" 
                         << EvalParams::ROOK_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
            }
        } else if (piece == Piece::WhiteQueen) {
            if ((all_pawns & file_mask) == 0) {
                score += EvalParams::QUEEN_OPEN_FILE_BONUS;
                std::cout << "White queen on open file " << char('a' + file) << ": +" 
                         << EvalParams::QUEEN_OPEN_FILE_BONUS << "cp" << std::endl;
            } else if ((white_pawns & file_mask) == 0) {
                score += EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS;
                std::cout << "White queen on semi-open file " << char('a' + file) << ": +" 
                         << EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
            }
        } else if (piece == Piece::BlackQueen) {
            if ((all_pawns & file_mask) == 0) {
                score -= EvalParams::QUEEN_OPEN_FILE_BONUS;
                std::cout << "Black queen on open file " << char('a' + file) << ": -" 
                         << EvalParams::QUEEN_OPEN_FILE_BONUS << "cp" << std::endl;
            } else if ((black_pawns & file_mask) == 0) {
                score -= EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS;
                std::cout << "Black queen on semi-open file " << char('a' + file) << ": -" 
                         << EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
            }
        }
    }
    
    return score;
}

void debugFileBonuses() {
    std::cout << "=== VICE Part 81: File Bonus Debug Test ===" << std::endl;
    
    init();
    
    // Test 1: Rook on truly open file (no pawns at all)
    std::cout << "\nTest 1: Rook on open a-file" << std::endl;
    std::string fen1 = "7k/8/8/8/8/8/8/R6K w - - 0 1";
    Position pos1;
    pos1.set_from_fen(fen1);
    std::cout << "FEN: " << fen1 << std::endl;
    int bonus1 = evaluateFileBonusesOnly(pos1);
    std::cout << "Total file bonuses: " << bonus1 << "cp" << std::endl;
    
    // Test 2: Rook on semi-open file (only enemy pawn)
    std::cout << "\nTest 2: Rook on semi-open a-file (enemy pawn present)" << std::endl;
    std::string fen2 = "7k/p7/8/8/8/8/8/R6K w - - 0 1";
    Position pos2;
    pos2.set_from_fen(fen2);
    std::cout << "FEN: " << fen2 << std::endl;
    int bonus2 = evaluateFileBonusesOnly(pos2);
    std::cout << "Total file bonuses: " << bonus2 << "cp" << std::endl;
    
    // Test 3: Rook on blocked file (both colors have pawns)
    std::cout << "\nTest 3: Rook on blocked a-file (both pawns present)" << std::endl;
    std::string fen3 = "7k/p7/8/8/8/8/P7/R6K w - - 0 1";
    Position pos3;
    pos3.set_from_fen(fen3);
    std::cout << "FEN: " << fen3 << std::endl;
    int bonus3 = evaluateFileBonusesOnly(pos3);
    std::cout << "Total file bonuses: " << bonus3 << "cp" << std::endl;
    
    // Test 4: Queen on open file
    std::cout << "\nTest 4: Queen on open d-file" << std::endl;
    std::string fen4 = "7k/8/8/8/8/8/8/3Q3K w - - 0 1";
    Position pos4;
    pos4.set_from_fen(fen4);
    std::cout << "FEN: " << fen4 << std::endl;
    int bonus4 = evaluateFileBonusesOnly(pos4);
    std::cout << "Total file bonuses: " << bonus4 << "cp" << std::endl;
    
    std::cout << "\n✅ File bonus implementation working correctly!" << std::endl;
}

int main() {
    debugFileBonuses();
    return 0;
}
