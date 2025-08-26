#include "Engine3_src/simple_search.hpp"
#include "Engine3_src/hybrid_evaluation.hpp"
#include "src/position.hpp"
#include "src/init.hpp"
#include "src/movegen_enhanced.hpp"
#include <iostream>
#include <sstream>
#include <string>

class Huginn3_UCI {
private:
    Position position;
    Engine3::SimpleEngine engine;
    bool debug = false;
    
public:
    Huginn3_UCI() {
        position.set_startpos();
    }
    
    void run() {
        std::string line;
        
        std::cout << "Huginn3 Chess Engine v1.0" << std::endl;
        
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string command;
            iss >> command;
            
            if (command == "uci") {
                handle_uci();
            } else if (command == "debug") {
                std::string mode;
                iss >> mode;
                debug = (mode == "on");
            } else if (command == "isready") {
                std::cout << "readyok" << std::endl;
            } else if (command == "position") {
                handle_position(iss);
            } else if (command == "go") {
                handle_go(iss);
            } else if (command == "quit") {
                break;
            } else if (command == "stop") {
                engine.stop();
            } else {
                if (debug) {
                    std::cout << "info string Unknown command: " << command << std::endl;
                }
            }
        }
    }
    
private:
    void handle_uci() {
        std::cout << "id name Huginn3" << std::endl;
        std::cout << "id author MTDuke71" << std::endl;
        std::cout << "option name Debug type check default false" << std::endl;
        std::cout << "uciok" << std::endl;
    }
    
    void handle_position(std::istringstream& iss) {
        std::string token;
        iss >> token;
        
        if (token == "startpos") {
            position.set_startpos();
            iss >> token; // consume "moves" if present
        } else if (token == "fen") {
            std::string fen;
            std::string part;
            
            // Read FEN components
            for (int i = 0; i < 6 && iss >> part; ++i) {
                if (i > 0) fen += " ";
                fen += part;
            }
            
            if (debug) {
                std::cout << "info string Setting FEN: " << fen << std::endl;
            }
            
            position.set_from_fen(fen);
            iss >> token; // consume "moves" if present
        }
        
        // Handle moves
        if (token == "moves") {
            std::string move_str;
            while (iss >> move_str) {
                if (debug) {
                    std::cout << "info string Applying move: " << move_str << std::endl;
                }
                
                // Find and make the move
                S_MOVELIST legal_moves;
                generate_legal_moves_enhanced(position, legal_moves);
                
                bool move_found = false;
                for (int i = 0; i < legal_moves.count; ++i) {
                    std::string uci_move = Engine3::SimpleEngine::move_to_uci(legal_moves.moves[i]);
                    if (uci_move == move_str) {
                        position.make_move_with_undo(legal_moves.moves[i]);
                        move_found = true;
                        break;
                    }
                }
                
                if (!move_found && debug) {
                    std::cout << "info string Warning: Move not found: " << move_str << std::endl;
                }
            }
        }
    }
    
    void handle_go(std::istringstream& iss) {
        Engine3::SearchLimits limits;
        limits.max_depth = 6;        // Default depth
        limits.max_time_ms = 5000;   // Default 5 seconds
        limits.max_nodes = 1000000;  // Default 1M nodes
        
        std::string token;
        while (iss >> token) {
            if (token == "depth") {
                iss >> limits.max_depth;
            } else if (token == "movetime") {
                iss >> limits.max_time_ms;
            } else if (token == "nodes") {
                iss >> limits.max_nodes;
            } else if (token == "infinite") {
                limits.infinite = true;
                limits.max_time_ms = 1000000; // Very long time
            } else if (token == "wtime" || token == "btime") {
                int time_ms;
                iss >> time_ms;
                // Simple time management - use 1/30th of remaining time
                limits.max_time_ms = std::max(100, time_ms / 30);
            }
        }
        
        if (debug) {
            std::cout << "info string Starting search: depth=" << limits.max_depth 
                      << " time=" << limits.max_time_ms << "ms" << std::endl;
        }
        
        engine.reset();
        S_MOVE best_move = engine.search(position, limits);
        
        std::string best_move_uci = Engine3::SimpleEngine::move_to_uci(best_move);
        std::cout << "bestmove " << best_move_uci << std::endl;
    }
};

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Huginn3_UCI uci_interface;
    uci_interface.run();
    
    return 0;
}
