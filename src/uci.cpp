#include "uci.hpp"
#include "init.hpp"
#include "board120.hpp"
#include "uci_utils.hpp"
#include "movegen_enhanced.hpp"

UCIInterface::UCIInterface() {
    // Initialize the chess engine
    Huginn::init();
    
    // Set starting position
    // Ensure uci_utils.hpp is included at the top of the file
    position.set_startpos();
    
    // Initialize search engine
    search_engine = std::make_unique<Huginn::SimpleEngine>();
}

void UCIInterface::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        
        auto tokens = split_string(line);
        if (tokens.empty()) continue;
        
        const std::string& command = tokens[0];
        
        if (debug_mode) {
            std::cout << "info string Received command: " << line << std::endl;
        }
        
        if (command == "uci") {
            send_id();
            send_options();
            std::cout << "uciok" << std::endl;
            std::cout.flush();
        }
        else if (command == "debug") {
            if (tokens.size() > 1) {
                debug_mode = (tokens[1] == "on");
            }
        }
        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
            std::cout.flush();
            // No move parsing needed for isready
        }
        else if (command == "setoption") {
            handle_setoption(tokens);
        }
        else if (command == "register") {
            // No registration needed for this engine
            if (debug_mode) {
                std::cout << "info string Registration not required" << std::endl;
            }
        }
        else if (command == "ucinewgame") {
            // Reset for new game
            position.set_startpos();
            search_engine->reset(); // Reset search state
            if (debug_mode) {
                std::cout << "info string New game started" << std::endl;
            }
        }
        else if (command == "position") {
            handle_position(tokens);
        }
        else if (command == "go") {
            handle_go(tokens);
        }
        else if (command == "d") {
            // Debug command to display position
            std::cout << "info string Current FEN: " << position.to_fen() << std::endl;
            std::cout << "info string Side to move: " << (position.side_to_move == Color::White ? "White" : "Black") << std::endl;
            std::cout << "info string White King at: " << position.king_sq[0] << std::endl;
            std::cout << "info string Black King at: " << position.king_sq[1] << std::endl;
            std::cout << "info string Castling rights: " << int(position.castling_rights) << std::endl;
            
            // Print board representation
            std::cout << "info string Board:" << std::endl;
            for (int rank = 7; rank >= 0; rank--) {
                std::string board_line = "info string ";
                for (int file = 0; file < 8; file++) {
                    int square = sq(File(file), Rank(rank));
                    Piece p = position.at(square);
                    char piece_char = '.';
                    if (p != Piece::None) {
                        switch (type_of(p)) {
                            case PieceType::Pawn:   piece_char = 'P'; break;
                            case PieceType::Knight: piece_char = 'N'; break;
                            case PieceType::Bishop: piece_char = 'B'; break;
                            case PieceType::Rook:   piece_char = 'R'; break;
                            case PieceType::Queen:  piece_char = 'Q'; break;
                            case PieceType::King:   piece_char = 'K'; break;
                            default: piece_char = '?'; break;
                        }
                        if (color_of(p) == Color::Black) {
                            piece_char = tolower(piece_char);
                        }
                    }
                    board_line += piece_char;
                    board_line += ' ';
                }
                std::cout << board_line << std::endl;
            }
        }
        else if (command == "stop") {
            should_stop = true;
            search_engine->stop();
        }
        else if (command == "ponderhit") {
            // Continue search without pondering
            if (debug_mode) {
                std::cout << "info string Ponder hit" << std::endl;
            }
        }
        else if (command == "quit") {
            break;
        }
        else {
            if (debug_mode) {
                std::cout << "info string Unknown command: " << command << std::endl;
            }
        }
    }
}

void UCIInterface::send_id() {
    std::cout << "id name Huginn 1.1" << std::endl;
    std::cout << "id author MTDuke71" << std::endl;
}

void UCIInterface::send_options() {
    // Send basic UCI options
    std::cout << "option name Threads type spin default 4 min 1 max 64" << std::endl;
    std::cout << "option name Ponder type check default false" << std::endl;
}

std::vector<std::string> UCIInterface::split_string(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void UCIInterface::handle_position(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return;
    
    size_t move_index = 0;
    
    if (tokens[1] == "startpos") {
        position.set_startpos();
        move_index = 2;
    }
    else if (tokens[1] == "fen") {
        if (tokens.size() < 8) return; // Need at least "position fen <6 fen parts>"
        
        // Reconstruct FEN string from tokens 2-7
        std::string fen;
        for (size_t i = 2; i < 8 && i < tokens.size(); ++i) {
            if (i > 2) fen += " ";
            fen += tokens[i];
        }
        
        if (!position.set_from_fen(fen)) {
            if (debug_mode) {
                std::cout << "info string Invalid FEN: " << fen << std::endl;
            }
            return;
        }
        move_index = 8;
    }
    
    // Apply moves if "moves" keyword is present
    if (move_index < tokens.size() && tokens[move_index] == "moves") {
        for (size_t i = move_index + 1; i < tokens.size(); ++i) {
            S_MOVE move = parse_uci_move(tokens[i], position);
            if (move.move != 0) {
                if (position.MakeMove(move) != 1) {
                    if (debug_mode) {
                        std::cout << "info string Illegal move: " << tokens[i] << std::endl;
                    }
                }
            } else if (debug_mode) {
                std::cout << "info string Invalid move: " << tokens[i] << std::endl;
            }
        }
    }
    
    if (debug_mode) {
        std::cout << "info string Position set, FEN: " << position.to_fen() << std::endl;
    }
}

void UCIInterface::handle_go(const std::vector<std::string>& tokens) {
    if (debug_mode) {
        std::cout << "info string Starting search" << std::endl;
    }
    
    should_stop = false;
    
    // Parse search limits from go command
    Huginn::SearchLimits limits;  // Uses defaults from SearchLimits struct
    limits.infinite = false;
    limits.threads = 4; // Use 4 threads for stability
    // limits.max_depth = 0 (unlimited depth by default from struct)
    // limits.max_time_ms uses default from SearchLimits (10000ms)
    
    if (debug_mode) {
        std::cout << "info string Debug: Parsing go command with " << tokens.size() << " tokens" << std::endl;
    }
    
    // Parse go parameters
    bool depth_specified = false;
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth" && i + 1 < tokens.size()) {
            limits.max_depth = std::stoi(tokens[i + 1]);
            depth_specified = true;
            i++;
        }
        else if (tokens[i] == "movetime" && i + 1 < tokens.size()) {
            limits.max_time_ms = std::stoi(tokens[i + 1]);
            i++;
        }
        else if (tokens[i] == "nodes" && i + 1 < tokens.size()) {
            limits.max_nodes = std::stoull(tokens[i + 1]);
            i++;
        }
        else if (tokens[i] == "wtime" && i + 1 < tokens.size()) {
            int wtime = std::stoi(tokens[i + 1]);
            if (position.side_to_move == Color::White) {
                // Better time allocation: 
                // - Use more time early in game, less time later
                // - Minimum 3 seconds, maximum 15 seconds per move
                // - For 60000ms (1 minute), allocate 5-8 seconds per move
                int base_time = wtime / 10;  // Use 1/10th of remaining time
                limits.max_time_ms = std::min(15000, std::max(3000, base_time));
                
                if (debug_mode) {
                    std::cout << "info string White time allocation: wtime=" << wtime 
                              << " base=" << base_time << " final=" << limits.max_time_ms << std::endl;
                }
            }
            i++;
        }
        else if (tokens[i] == "btime" && i + 1 < tokens.size()) {
            int btime = std::stoi(tokens[i + 1]);
            if (position.side_to_move == Color::Black) {
                // Better time allocation: 
                // - Use more time early in game, less time later
                // - Minimum 3 seconds, maximum 15 seconds per move
                // - For 60000ms (1 minute), allocate 5-8 seconds per move
                int base_time = btime / 10;  // Use 1/10th of remaining time
                limits.max_time_ms = std::min(15000, std::max(3000, base_time));
                
                if (debug_mode) {
                    std::cout << "info string Black time allocation: btime=" << btime 
                              << " base=" << base_time << " final=" << limits.max_time_ms << std::endl;
                }
            }
            i++;
        }
        else if (tokens[i] == "infinite") {
            limits.infinite = true;
            limits.max_time_ms = 0; // No time limit
            limits.max_nodes = UINT64_MAX; // No node limit
        }
    }
    
    // If depth is specified, ignore time limits to complete the requested depth
    if (depth_specified) {
        limits.infinite = true;
        limits.max_time_ms = 0; // No time limit for depth searches
        limits.max_nodes = UINT64_MAX; // No node limit for depth searches
    }
    
    if (debug_mode) {
        std::cout << "info string Debug: Starting search with depth " << limits.max_depth 
                  << " time " << limits.max_time_ms << "ms" << std::endl;
    }
    
    // Perform synchronous search
    search_best_move(limits);
}

void UCIInterface::handle_setoption(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) return; // Need at least "setoption name X value Y"
    
    if (tokens[1] != "name") return;
    
    std::string option_name = tokens[2];
    
    if (tokens.size() >= 5 && tokens[3] == "value") {
        std::string option_value = tokens[4];
        
        if (option_name == "Hash") {
            // Hash tables not implemented yet, acknowledge but don't set
            if (debug_mode) {
                std::cout << "info string Hash setting acknowledged (not implemented yet)" << std::endl;
            }
        }
        else if (option_name == "Threads") {
            int thread_count = std::stoi(option_value);
            if (thread_count >= 1 && thread_count <= 64) {
                threads = thread_count;
                if (debug_mode) {
                    std::cout << "info string Threads set to " << threads << std::endl;
                }
            }
        }
        else if (option_name == "Ponder") {
            bool ponder = (option_value == "true");
            // Pondering not supported yet
            if (debug_mode) {
                std::cout << "info string Ponder set to " << (ponder ? "true" : "false") << " (not supported)" << std::endl;
            }
        }
    }
}

void UCIInterface::search_best_move(const Huginn::SearchLimits& limits) {
    is_searching = true;
    
    // Reset the search engine
    search_engine->reset();
    
    // Set thread count in limits
    Huginn::SearchLimits modified_limits = limits;
    modified_limits.threads = threads; // Use the UCI-configured thread count directly
    
    // Safety mechanism: ensure we always return a move within reasonable time
    // Add 500ms buffer to account for search overhead
    auto search_start = std::chrono::high_resolution_clock::now();
    auto max_search_time = std::chrono::milliseconds(modified_limits.max_time_ms + 500);
    
    // Perform the search
    S_MOVE best_move;
    try {
        best_move = search_engine->search(position, modified_limits);
    } catch (const std::exception& e) {
        std::cout << "info string Search threw exception: " << e.what() << std::endl;
        std::cout.flush();
        best_move.move = 0; // Set to invalid move to trigger fallback
    } catch (...) {
        std::cout << "info string Search threw unknown exception" << std::endl;
        std::cout.flush();
        best_move.move = 0; // Set to invalid move to trigger fallback
    }
    
    // Emergency fallback: if search took too long or returned no move, get any legal move
    if (best_move.move == 0 || should_stop) {
        S_MOVELIST moves;
        generate_legal_moves_enhanced(position, moves);
        if (moves.count > 0) {
            best_move = moves.moves[0]; // Use first legal move as fallback
        }
    }
    
    // Get search statistics
    const auto& stats = search_engine->get_stats();
    const auto& pv = search_engine->get_pv();
    
    // Send final search info
    std::cout << "info depth " << stats.max_depth_reached;
    std::cout << " nodes " << stats.nodes_searched;
    std::cout << " time " << stats.time_ms;
    if (stats.time_ms > 0) {
        std::cout << " nps " << (stats.nodes_searched * 1000) / stats.time_ms;
    }
    
    // Add PV if available
    if (pv.length > 0) {
        std::cout << " pv";
        for (int i = 0; i < pv.length; i++) {
            std::cout << " " << Huginn::SimpleEngine::move_to_uci(pv.moves[i]);
        }
    }
    std::cout << std::endl;
    
    // Send the best move - ALWAYS send something
    if (best_move.move != 0) {
        std::string uci_move = Huginn::SimpleEngine::move_to_uci(best_move);
        std::cout << "bestmove " << uci_move << std::endl;
        std::cout.flush(); // Ensure immediate output
    } else {
        // Last resort fallback - this should never happen
        std::cout << "bestmove 0000" << std::endl;
        std::cout.flush(); // Ensure immediate output
    }
    
    is_searching = false;
}


std::string UCIInterface::move_to_uci(const S_MOVE& move) {
    // Use move_to_uci implementation
    return Huginn::SimpleEngine::move_to_uci(move);
}
