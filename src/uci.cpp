#include "uci.hpp"
#include "init.hpp"
#include "board120.hpp"

UCIInterface::UCIInterface() {
    // Initialize the chess engine
    Huginn::init();
    
    // Set starting position
    position.set_startpos();
    
    // Initialize search engine
    search_engine = std::make_unique<Huginn::ThreadedEngine>();
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
    std::cout << "option name Threads type spin default 16 min 1 max 64" << std::endl;
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
    Huginn::SearchLimits limits;
    limits.infinite = false;
    limits.max_depth = 8; // Default depth
    limits.max_time_ms = 5000; // Default 5 seconds
    
    if (debug_mode) {
        std::cout << "info string Debug: Parsing go command with " << tokens.size() << " tokens" << std::endl;
    }
    
    // Parse go parameters
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth" && i + 1 < tokens.size()) {
            limits.max_depth = std::stoi(tokens[i + 1]);
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
                limits.max_time_ms = std::max(200, wtime / 20); // Use 1/20th of remaining time
            }
            i++;
        }
        else if (tokens[i] == "btime" && i + 1 < tokens.size()) {
            int btime = std::stoi(tokens[i + 1]);
            if (position.side_to_move == Color::Black) {
                limits.max_time_ms = std::max(200, btime / 20); // Use 1/20th of remaining time
            }
            i++;
        }
        else if (tokens[i] == "infinite") {
            limits.infinite = true;
            limits.max_time_ms = 0; // No time limit
            limits.max_nodes = UINT64_MAX; // No node limit
        }
    }
    
    if (debug_mode) {
        std::cout << "info string Debug: Starting search with depth " << limits.max_depth << std::endl;
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
    
    // Perform the search
    S_MOVE best_move = search_engine->search(position, modified_limits);
    
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
    
    // Send the best move
    if (best_move.move != 0) {
        std::string uci_move = Huginn::SimpleEngine::move_to_uci(best_move);
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
    // Use move_to_uci implementation
    return Huginn::SimpleEngine::move_to_uci(move);
}
