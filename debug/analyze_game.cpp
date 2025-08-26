#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/evaluation.hpp"
#include "../src/search.hpp"
#include <iostream>
#include <vector>
#include <string>

std::string square_to_algebraic(int sq) {
    if (sq < 0 || sq >= 64) return "??";
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string piece_to_char(Piece piece) {
    switch (piece) {
        case W_PAWN: return "P";
        case W_KNIGHT: return "N";
        case W_BISHOP: return "B";
        case W_ROOK: return "R";
        case W_QUEEN: return "Q";
        case W_KING: return "K";
        case B_PAWN: return "p";
        case B_KNIGHT: return "n";
        case B_BISHOP: return "b";
        case B_ROOK: return "r";
        case B_QUEEN: return "q";
        case B_KING: return "k";
        default: return ".";
    }
}

void print_board(const Position& pos) {
    std::cout << "   a b c d e f g h\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << "  ";
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            std::cout << piece_to_char(pos.piece_at(sq)) << " ";
        }
        std::cout << " " << (rank + 1) << "\n";
    }
    std::cout << "   a b c d e f g h\n\n";
}

std::string move_to_algebraic(const S_MOVE& move, const Position& pos) {
    std::string from = square_to_algebraic(move.from_sq);
    std::string to = square_to_algebraic(move.to_sq);
    
    Piece moving_piece = pos.piece_at(move.from_sq);
    Piece captured_piece = pos.piece_at(move.to_sq);
    
    std::string notation;
    
    // Add piece symbol (except for pawns)
    if (moving_piece != W_PAWN && moving_piece != B_PAWN) {
        notation += std::toupper(piece_to_char(moving_piece)[0]);
    }
    
    // Add capture notation
    if (captured_piece != EMPTY) {
        if (moving_piece == W_PAWN || moving_piece == B_PAWN) {
            notation += from[0]; // pawn captures show file
        }
        notation += "x";
    }
    
    notation += to;
    
    return notation;
}

void print_position_info(const Position& pos, int move_num, const std::string& last_move) {
    std::cout << "=== Move " << move_num << ": " << last_move << " ===\n";
    print_board(pos);
    
    std::cout << "Turn: " << (pos.side_to_move() == WHITE ? "White" : "Black") << "\n";
    
    // Check if king is in check
    if (pos.is_in_check(WHITE)) std::cout << "White king in CHECK!\n";
    if (pos.is_in_check(BLACK)) std::cout << "Black king in CHECK!\n";
    
    // Evaluate position
    int eval = evaluate(pos);
    std::cout << "Evaluation: " << eval << " cp (White's perspective)\n";
    
    if (eval > 500) {
        std::cout << "*** White has a significant advantage! ***\n";
    } else if (eval < -500) {
        std::cout << "*** Black has a significant advantage! ***\n";
    }
    
    // Generate legal moves
    MoveList moves;
    generate_legal_moves(pos, moves);
    std::cout << "Legal moves: " << moves.size() << "\n";
    
    if (moves.size() == 0) {
        if (pos.is_in_check(pos.side_to_move())) {
            std::cout << "*** CHECKMATE! ***\n";
        } else {
            std::cout << "*** STALEMATE! ***\n";
        }
    }
    
    std::cout << "\n";
}

void analyze_moves_from_startpos(const std::vector<std::string>& move_list) {
    Position pos;
    pos.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "=== GAME ANALYSIS ===\n";
    std::cout << "White: Huginn (Computer)\n";
    std::cout << "Black: Opponent\n\n";
    
    std::cout << "Starting position:\n";
    print_board(pos);
    
    int prev_eval = evaluate(pos);
    
    for (size_t i = 0; i < move_list.size(); ++i) {
        const std::string& move_str = move_list[i];
        
        // Convert UCI move to S_MOVE
        S_MOVE move = uci_to_move(move_str, pos);
        if (move.from_sq == 64 || move.to_sq == 64) {
            std::cout << "Invalid move: " << move_str << "\n";
            break;
        }
        
        // Convert to algebraic notation before making the move
        std::string algebraic = move_to_algebraic(move, pos);
        
        // Make the move
        pos.make_move(move);
        
        int current_eval = evaluate(pos);
        int eval_change = current_eval - prev_eval;
        
        // Highlight significant evaluation changes
        bool significant_change = std::abs(eval_change) > 200;
        
        if (significant_change || (i + 1) % 4 == 0) { // Show every 4 moves or significant changes
            std::string player = (i % 2 == 0) ? "Huginn (White)" : "Black";
            std::cout << "Move " << (i + 1) << ": " << player << " plays " << algebraic 
                      << " (" << move_str << ")\n";
            
            if (significant_change) {
                std::cout << "*** SIGNIFICANT EVALUATION CHANGE! ***\n";
                std::cout << "Evaluation change: " << eval_change << " cp\n";
                
                if (eval_change < -500 && i % 2 == 0) {
                    std::cout << "*** WHITE (HUGINN) MADE A BAD MOVE! ***\n";
                } else if (eval_change > 500 && i % 2 == 1) {
                    std::cout << "*** BLACK MADE A BAD MOVE! ***\n";
                }
            }
            
            print_position_info(pos, i + 1, algebraic);
        } else {
            std::string player = (i % 2 == 0) ? "Huginn" : "Black";
            std::cout << (i + 1) << ". " << player << ": " << algebraic 
                      << " (eval: " << current_eval << " cp)\n";
        }
        
        prev_eval = current_eval;
        
        // If checkmate or stalemate, stop
        MoveList moves;
        generate_legal_moves(pos, moves);
        if (moves.size() == 0) {
            break;
        }
    }
    
    std::cout << "\n=== GAME SUMMARY ===\n";
    std::cout << "Final evaluation: " << prev_eval << " cp\n";
    if (prev_eval > 1000) {
        std::cout << "White (Huginn) is winning decisively\n";
    } else if (prev_eval < -1000) {
        std::cout << "Black is winning decisively\n";
    } else {
        std::cout << "Position is roughly balanced\n";
    }
}

int main() {
    std::vector<std::string> moves = {
        "a2a4", "b8c6", "e2e3", "g8f6", "f2f3", "d7d5", "e1f2", "d5d4",
        "f1b5", "d4e3", "f2f1", "d8d6", "f3f4", "c8g4", "d2d4", "g4d1",
        "b5c6", "d6c6", "g1f3", "c6c2", "g2g4"
    };
    
    std::cout << "Analyzing game moves:\n\n";
    analyze_moves_from_startpos(moves);
    
    return 0;
}
