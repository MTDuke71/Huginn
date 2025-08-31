#include "minimal_search.hpp"
#include "position.hpp"
#include "init.hpp"
#include "board.hpp"
#include <iostream>
#include <string>
#include <sstream>

using namespace Huginn;

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main() {
    // Initialize the engine
    init();
    
    MinimalEngine engine;
    Position pos;
    
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "uci") {
            std::cout << "id name Huginn Minimal Engine" << std::endl;
            std::cout << "id author Huginn Team" << std::endl;
            std::cout << "uciok" << std::endl;
            
        } else if (command == "isready") {
            std::cout << "readyok" << std::endl;
            
        } else if (command == "ucinewgame") {
            pos.set_from_fen(STARTING_FEN);
            
        } else if (command == "position") {
            std::string type;
            iss >> type;
            
            if (type == "startpos") {
                pos.set_from_fen(STARTING_FEN);
                
                std::string moves_cmd;
                if (iss >> moves_cmd && moves_cmd == "moves") {
                    std::string move_str;
                    while (iss >> move_str) {
                        // Find and make the move
                        S_MOVELIST move_list;
                        generate_legal_moves_enhanced(pos, move_list);
                        
                        bool found = false;
                        for (int i = 0; i < move_list.count; ++i) {
                            if (engine.move_to_uci(move_list.moves[i]) == move_str) {
                                if (pos.MakeMove(move_list.moves[i]) == 1) {
                                    found = true;
                                    break;
                                } else {
                                    pos.TakeMove();
                                }
                            }
                        }
                        
                        if (!found) {
                            std::cerr << "Illegal move: " << move_str << std::endl;
                            break;
                        }
                    }
                }
                
            } else if (type == "fen") {
                std::string fen;
                std::string word;
                while (iss >> word && word != "moves") {
                    if (!fen.empty()) fen += " ";
                    fen += word;
                }
                
                pos.set_from_fen(fen);
                
                if (word == "moves") {
                    std::string move_str;
                    while (iss >> move_str) {
                        // Find and make the move
                        S_MOVELIST move_list;
                        generate_legal_moves_enhanced(pos, move_list);
                        
                        bool found = false;
                        for (int i = 0; i < move_list.count; ++i) {
                            if (engine.move_to_uci(move_list.moves[i]) == move_str) {
                                if (pos.MakeMove(move_list.moves[i]) == 1) {
                                    found = true;
                                    break;
                                } else {
                                    pos.TakeMove();
                                }
                            }
                        }
                        
                        if (!found) {
                            std::cerr << "Illegal move: " << move_str << std::endl;
                            break;
                        }
                    }
                }
            }
            
        } else if (command == "go") {
            MinimalLimits limits;
            limits.max_depth = 6;
            limits.max_time_ms = 5000;
            limits.infinite = false;
            
            std::string param;
            while (iss >> param) {
                if (param == "depth") {
                    iss >> limits.max_depth;
                } else if (param == "movetime") {
                    iss >> limits.max_time_ms;
                } else if (param == "infinite") {
                    limits.infinite = true;
                } else if (param == "wtime" || param == "btime") {
                    int time_ms;
                    iss >> time_ms;
                    // Ultra-conservative time management like the main engine
                    limits.max_time_ms = time_ms / 20;
                }
            }
            
            // Ensure we don't search too deep or too long
            limits.max_depth = std::min(limits.max_depth, 10);
            if (!limits.infinite) {
                limits.max_time_ms = std::min(limits.max_time_ms, 10000);
            }
            
            S_MOVE best_move = engine.search(pos, limits);
            std::cout << "bestmove " << engine.move_to_uci(best_move) << std::endl;
            
        } else if (command == "stop") {
            engine.should_stop = true;
            
        } else if (command == "quit") {
            break;
            
        } else if (command == "eval") {
            // Debug command to show current evaluation
            int eval = engine.evaluate(pos);
            std::cout << "Evaluation: " << eval << " cp" << std::endl;
            
        } else if (command == "print") {
            // Debug command to print the board
            print_position(pos);
        }
    }
    
    return 0;
}
