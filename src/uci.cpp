#include "uci.hpp"
#include "init.hpp"
#include "board120.hpp"

UCIInterface::UCIInterface() {
    // Initialize the chess engine
    Huginn::init();
    
    // Set starting position
    position.set_startpos();
    
    // Initialize search engine
    search_engine = std::make_unique<Search::Engine>();
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
        }
        else if (command == "debug") {
            if (tokens.size() > 1) {
                debug_mode = (tokens[1] == "on");
            }
        }
        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
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
            search_engine->clear_hash(); // Clear transposition table
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
    std::cout << "id name Huginn 1.0" << std::endl;
    std::cout << "id author MTDuke71" << std::endl;
}

void UCIInterface::send_options() {
    // Send basic UCI options
    std::cout << "option name Hash type spin default 32 min 1 max 1024" << std::endl;
    std::cout << "option name Threads type spin default 1 min 1 max 64" << std::endl;
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
            S_MOVE move = parse_uci_move(tokens[i]);
            if (move.move != 0) {
                position.make_move_with_undo(move);
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
    Search::SearchLimits limits;
    limits.infinite = false;
    limits.max_depth = 6; // Default depth
    limits.max_time = std::chrono::milliseconds(5000); // Default 5 seconds
    
    // Parse go parameters
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth" && i + 1 < tokens.size()) {
            limits.max_depth = std::stoi(tokens[i + 1]);
            i++;
        }
        else if (tokens[i] == "movetime" && i + 1 < tokens.size()) {
            limits.max_time = std::chrono::milliseconds(std::stoi(tokens[i + 1]));
            i++;
        }
        else if (tokens[i] == "wtime" && i + 1 < tokens.size()) {
            int wtime = std::stoi(tokens[i + 1]);
            if (position.side_to_move == Color::White) {
                limits.max_time = std::chrono::milliseconds(std::max(100, wtime / 30)); // Use 1/30th of remaining time
            }
            i++;
        }
        else if (tokens[i] == "btime" && i + 1 < tokens.size()) {
            int btime = std::stoi(tokens[i + 1]);
            if (position.side_to_move == Color::Black) {
                limits.max_time = std::chrono::milliseconds(std::max(100, btime / 30)); // Use 1/30th of remaining time
            }
            i++;
        }
        else if (tokens[i] == "infinite") {
            limits.infinite = true;
            limits.max_time = std::chrono::milliseconds(0); // No time limit
        }
    }
    
    // Start search in separate thread
    std::thread search_thread(&UCIInterface::search_best_move, this, limits);
    search_thread.detach();
}

void UCIInterface::handle_setoption(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) return; // Need at least "setoption name X value Y"
    
    if (tokens[1] != "name") return;
    
    std::string option_name = tokens[2];
    
    if (tokens.size() >= 5 && tokens[3] == "value") {
        std::string option_value = tokens[4];
        
        if (option_name == "Hash") {
            int hash_size = std::stoi(option_value);
            search_engine->set_hash_size(hash_size);
            if (debug_mode) {
                std::cout << "info string Hash size set to " << hash_size << " MB" << std::endl;
            }
        }
        else if (option_name == "Threads") {
            int threads = std::stoi(option_value);
            // For now, we don't support multiple threads, so just acknowledge
            if (debug_mode) {
                std::cout << "info string Threads set to " << threads << " (single-threaded engine)" << std::endl;
            }
        }
        else if (option_name == "Ponder") {
            bool ponder = (option_value == "true");
            // For now, we don't support pondering, so just acknowledge
            if (debug_mode) {
                std::cout << "info string Ponder set to " << (ponder ? "true" : "false") << " (not supported)" << std::endl;
            }
        }
    }
}

void UCIInterface::search_best_move(const Search::SearchLimits& limits) {
    is_searching = true;
    
    // Set up search info callback to send UCI info
    search_engine->set_info_callback([this](const Search::SearchInfo& info) {
        std::cout << "info depth " << info.depth
                  << " nodes " << info.nodes
                  << " time " << info.time_ms
                  << " nps " << (info.time_ms > 0 ? (info.nodes * 1000) / info.time_ms : 0)
                  << " score cp " << info.score;
        
        if (!info.pv.empty()) {
            std::cout << " pv";
            for (const auto& move : info.pv) {
                std::cout << " " << move_to_uci(move);
            }
        }
        std::cout << std::endl;
    });
    
    // Perform the search
    S_MOVE best_move = search_engine->search(position, limits);
    
    // Send the best move
    if (best_move.move != 0) {
        std::string uci_move = move_to_uci(best_move);
        std::cout << "bestmove " << uci_move << std::endl;
    } else {
        std::cout << "bestmove 0000" << std::endl;
    }
    
    is_searching = false;
}

S_MOVE UCIInterface::parse_uci_move(const std::string& uci_move) {
    if (uci_move.length() < 4) return S_MOVE();
    
    // Parse from square (e.g., "e2")
    int from_file = uci_move[0] - 'a';
    int from_rank = uci_move[1] - '1';
    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7) {
        return S_MOVE();
    }
    int from = sq(File(from_file), Rank(from_rank));
    
    // Parse to square (e.g., "e4")
    int to_file = uci_move[2] - 'a';
    int to_rank = uci_move[3] - '1';
    if (to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) {
        return S_MOVE();
    }
    int to = sq(File(to_file), Rank(to_rank));
    
    // Check for promotion
    PieceType promoted = PieceType::None;
    if (uci_move.length() == 5) {
        char promo_char = uci_move[4];
        switch (promo_char) {
            case 'q': promoted = PieceType::Queen; break;
            case 'r': promoted = PieceType::Rook; break;
            case 'b': promoted = PieceType::Bishop; break;
            case 'n': promoted = PieceType::Knight; break;
            default: return S_MOVE(); // Invalid promotion
        }
    }
    
    // Generate legal moves to find the matching move with proper flags
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(position, move_list);
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        if (move.get_from() == from && move.get_to() == to) {
            // Check promotion match
            if (promoted != PieceType::None) {
                if (move.get_promoted() == promoted) {
                    return move;
                }
            } else if (move.get_promoted() == PieceType::None) {
                return move;
            }
        }
    }
    
    return S_MOVE(); // Move not found
}

std::string UCIInterface::move_to_uci(const S_MOVE& move) {
    int from = move.get_from();
    int to = move.get_to();
    
    File from_file = file_of(from);
    Rank from_rank = rank_of(from);
    File to_file = file_of(to);
    Rank to_rank = rank_of(to);
    
    // Convert to UCI notation
    char from_file_char = 'a' + static_cast<int>(from_file);
    char from_rank_char = '1' + static_cast<int>(from_rank);
    char to_file_char = 'a' + static_cast<int>(to_file);
    char to_rank_char = '1' + static_cast<int>(to_rank);
    
    std::string uci_move;
    uci_move += from_file_char;
    uci_move += from_rank_char;
    uci_move += to_file_char;
    uci_move += to_rank_char;
    
    // Add promotion piece if present
    PieceType promoted = move.get_promoted();
    if (promoted != PieceType::None) {
        switch (promoted) {
            case PieceType::Queen:  uci_move += 'q'; break;
            case PieceType::Rook:   uci_move += 'r'; break;
            case PieceType::Bishop: uci_move += 'b'; break;
            case PieceType::Knight: uci_move += 'n'; break;
            default: break;
        }
    }
    
    return uci_move;
}
