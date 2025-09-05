#include "uci.hpp"
#include "init.hpp"
#include "board120.hpp"
#include "uci_utils.hpp"
#include "movegen_enhanced.hpp"

UCIInterface::UCIInterface() {
    // Initialize the chess engine
    Huginn::init();
    
    // Set starting position
    position.set_startpos();
    
    // Initialize search engine
    search_engine = std::make_unique<Huginn::MinimalEngine>();  // Changed from SimpleEngine
}

void UCIInterface::run() {
    // Set stdin and stdout to unbuffered mode for immediate GUI communication
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        auto tokens = split_string(line);
        if (tokens.empty()) continue;
        const std::string& command = tokens[0];

        if (debug_mode) std::cout << "info string Received command: " << line << std::endl;

        if (command == "uci") {
            send_id();
            send_options();
            std::cout << "uciok" << std::endl;
            std::cout.flush();
        }
        else if (command == "debug") {
            if (tokens.size() > 1) debug_mode = (tokens[1] == "on");
        }
        else if (command == "isready") {
            std::cout << "readyok" << std::endl;
            std::cout.flush();
        }
        else if (command == "setoption") {
            handle_setoption(tokens);
        }
        else if (command == "register") {
            if (debug_mode) std::cout << "info string Registration not required" << std::endl;
        }
        else if (command == "ucinewgame") {
            position.set_startpos();
            search_engine->reset();
            if (debug_mode) std::cout << "info string New game started" << std::endl;
        }
        else if (command == "position") {
            try {
                handle_position(tokens);
            } catch (const std::exception& e) {
                if (debug_mode) std::cout << "info string Error in position command: " << e.what() << std::endl;
            }
        }
        else if (command == "go") {
            try {
                handle_go(tokens);
            } catch (const std::exception& e) {
                if (debug_mode) std::cout << "info string Error in go command: " << e.what() << std::endl;
            }
        }
        else if (command == "d") {
            // Debug display
            std::cout << "info string Current FEN: " << position.to_fen() << std::endl;
            std::cout << "info string Side to move: " << (position.side_to_move == Color::White ? "White" : "Black") << std::endl;
            std::cout << "info string White King at: " << position.king_sq[0] << std::endl;
            std::cout << "info string Black King at: " << position.king_sq[1] << std::endl;
            std::cout << "info string Castling rights: " << int(position.castling_rights) << std::endl;
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
                        if (color_of(p) == Color::Black) piece_char = tolower(piece_char);
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
            Huginn::SearchInfo* info_ptr = running_info.load();
            if (info_ptr) {
                info_ptr->stop_time = std::chrono::high_resolution_clock::now();
                info_ptr->stopped = true;
            }
            std::cout.flush(); // Ensure immediate response to GUI
        }
        else if (command == "ponderhit") {
            if (debug_mode) std::cout << "info string Ponder hit" << std::endl;
        }
        else if (command == "quit") {
            break;
        }
        else {
            if (debug_mode) std::cout << "info string Unknown command: " << command << std::endl;
        }
    }
}

void UCIInterface::send_id() {
    std::cout << "id name Huginn 1.1" << std::endl;
    std::cout << "id author MTDuke71" << std::endl;
}

void UCIInterface::send_options() {
    std::cout << "option name Threads type spin default 1 min 1 max 64" << std::endl;
    std::cout << "option name Ponder type check default false" << std::endl;
}

std::vector<std::string> UCIInterface::split_string(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

void UCIInterface::handle_position(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return;
    size_t move_index = 0;
    if (tokens[1] == "startpos") { position.set_startpos(); move_index = 2; }
    else if (tokens[1] == "fen") {
        if (tokens.size() < 8) return;
        std::string fen;
        for (size_t i = 2; i < 8 && i < tokens.size(); ++i) { if (i > 2) fen += ' '; fen += tokens[i]; }
        if (!position.set_from_fen(fen)) { if (debug_mode) std::cout << "info string Invalid FEN: " << fen << std::endl; return; }
        move_index = 8;
    }

    if (move_index < tokens.size() && tokens[move_index] == "moves") {
        for (size_t i = move_index + 1; i < tokens.size(); ++i) {
            S_MOVE move = parse_uci_move(tokens[i], position);
            if (move.move != 0) {
                if (position.MakeMove(move) != 1) { if (debug_mode) std::cout << "info string Illegal move: " << tokens[i] << std::endl; }
            } else if (debug_mode) { std::cout << "info string Invalid move: " << tokens[i] << std::endl; }
        }
    }

    if (debug_mode) std::cout << "info string Position set, FEN: " << position.to_fen() << std::endl;
}

void UCIInterface::handle_go(const std::vector<std::string>& tokens) {
    if (debug_mode) std::cout << "info string Starting search" << std::endl;
    should_stop = false;

    Huginn::MinimalLimits limits; limits.infinite = false;
    if (debug_mode) std::cout << "info string Debug: Parsing go command with " << tokens.size() << " tokens" << std::endl;

    bool depth_specified = false;
    int winc = 0, binc = 0; int movestogo = 0; int wtime = -1, btime = -1;
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth" && i + 1 < tokens.size()) { limits.max_depth = std::stoi(tokens[i + 1]); depth_specified = true; i++; }
        else if (tokens[i] == "movetime" && i + 1 < tokens.size()) { limits.max_time_ms = std::stoi(tokens[i + 1]); i++; }
        else if (tokens[i] == "wtime" && i + 1 < tokens.size()) { wtime = std::stoi(tokens[i + 1]); i++; }
        else if (tokens[i] == "btime" && i + 1 < tokens.size()) { btime = std::stoi(tokens[i + 1]); i++; }
        else if (tokens[i] == "winc" && i + 1 < tokens.size()) { winc = std::stoi(tokens[i + 1]); i++; }
        else if (tokens[i] == "binc" && i + 1 < tokens.size()) { binc = std::stoi(tokens[i + 1]); i++; }
        else if (tokens[i] == "movestogo" && i + 1 < tokens.size()) { movestogo = std::stoi(tokens[i + 1]); i++; }
        else if (tokens[i] == "infinite") { limits.infinite = true; limits.max_time_ms = 0; }
    }

    if (depth_specified) { limits.infinite = true; limits.max_time_ms = 0; }

    if (!depth_specified && !limits.infinite) {
        int side_time = (position.side_to_move == Color::White) ? wtime : btime;
        int side_inc = (position.side_to_move == Color::White) ? winc : binc;
        if (side_time > 0) {
            int alloc = limits.max_time_ms;
            if (movestogo > 0) { alloc = side_time / std::max(1, movestogo); alloc += side_inc / 2; }
            else { alloc = std::max(50, side_time / 20 + side_inc / 4); }
            int reserve = std::min(1000, std::max(50, side_time / 10));
            alloc = std::max(50, alloc - reserve / std::max(1, movestogo > 0 ? movestogo : 1));
            alloc = std::min(alloc, static_cast<int>(side_time * 0.6));
            limits.max_time_ms = std::max(50, alloc);
        } else {
            if (movestogo == 0 && (winc > 0 || binc > 0)) limits.max_time_ms += ((position.side_to_move == Color::White) ? winc : binc) / 4;
        }
    }

    if (debug_mode) std::cout << "info string Debug: Starting search with depth " << limits.max_depth << " time " << limits.max_time_ms << "ms" << std::endl;

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

void UCIInterface::search_best_move(const Huginn::MinimalLimits& limits) {  // Changed from SearchLimits
    is_searching = true;
    
    // Reset the search engine
    search_engine->reset();
    
    // MinimalEngine uses a different search interface - searchPosition
    Huginn::SearchInfo info;
    info.max_depth = limits.max_depth;
    info.stopped = false;
    info.infinite = limits.infinite;
    
    // Convert time limits
    auto search_start = std::chrono::high_resolution_clock::now();
    info.start_time = search_start;
    info.stop_time = search_start + std::chrono::milliseconds(limits.max_time_ms);
    
    // Perform the search using MinimalEngine interface
    // Perform the search using MinimalEngine interface
    S_MOVE best_move;
    // Publish pointer to running SearchInfo so 'stop' can update it
    running_info.store(&info);
    try {
        best_move = search_engine->searchPosition(position, info);  // MinimalEngine method
    } catch (const std::exception& e) {
        std::cout << "info string Search threw exception: " << e.what() << std::endl;
        std::cout.flush();
        best_move.move = 0; // Set to invalid move to trigger fallback
    } catch (...) {
        std::cout << "info string Search threw unknown exception" << std::endl;
        std::cout.flush();
        best_move.move = 0; // Set to invalid move to trigger fallback
    }
    // Clear running_info pointer
    running_info.store(nullptr);
    
    // Emergency fallback: if search took too long or returned no move, get any legal move
    if (best_move.move == 0 || should_stop) {
        S_MOVELIST moves;
        generate_legal_moves_enhanced(position, moves);
        if (moves.count > 0) {
            best_move = moves.moves[0]; // Use first legal move as fallback
        }
    }
    
    // MinimalEngine already outputs complete UCI info during search
    // No need for additional summary output here
    
    // Send the best move - use MinimalEngine's move_to_uci
    if (best_move.move != 0) {
        std::string uci_move = search_engine->move_to_uci(best_move);  // MinimalEngine method
        std::cout << "bestmove " << uci_move << std::endl;
        std::cout.flush(); // Ensure immediate output
    } else {
        // Last resort fallback - this should never happen
        std::cout << "bestmove 0000" << std::endl;
        std::cout.flush(); // Ensure immediate output
    }
    
    is_searching = false;
}

void UCIInterface::signal_stop() {
    should_stop = true;
    if (search_engine) search_engine->stop();
    Huginn::SearchInfo* info_ptr = running_info.load();
    if (info_ptr) {
        info_ptr->stop_time = std::chrono::high_resolution_clock::now();
        info_ptr->stopped = true;
    }
}

