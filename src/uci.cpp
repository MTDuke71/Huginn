/**
 * @file uci.cpp
 * @brief Implements the UCI (Universal Chess Interface) protocol for the Huginn chess engine.
 *
 * This file contains the implementation of the UCIInterface class, which manages communication
 * between the Huginn chess engine and chess GUIs using the UCI protocol. It handles command
 * parsing, engine initialization, position setup, search operations, option management, and
 * opening book loading. The interface supports all standard UCI commands, robust error handling,
 * and debug output for engine development and integration.
 *
 * Key features:
 * - Full UCI protocol support (uci, isready, setoption, position, go, stop, quit, etc.)
 * - Flexible position setup from FEN or move list
 * - Time management for various time controls (classical, increment, sudden death)
 * - Opening book loading from multiple possible file locations
 * - Debug mode for verbose engine communication
 * - Thread and option management for engine configuration
 *
 * @author MTDuke71
 * @version 1.2
 */
#include "uci.hpp"
#include "init.hpp"
#include "board120.hpp"
#include "uci_utils.hpp"
#include "movegen_enhanced.hpp"
#include <fstream>

/**
 * @brief Constructs a new UCIInterface instance and initializes the chess engine.
 *
 * This constructor performs the essential initialization steps for the UCI interface:
 * - Initializes the Huginn chess engine core systems
 * - Sets the position to the standard starting position
 * - Creates a MinimalEngine instance for search operations
 * - Loads the opening book if enabled
 *
 * The constructor ensures the engine is ready to receive UCI commands immediately
 * after instantiation.
 */
UCIInterface::UCIInterface() {
    // Initialize the chess engine
    Huginn::init();
    
    // Set starting position
    position.set_startpos();
    
    // Initialize search engine
    search_engine = std::make_unique<Huginn::MinimalEngine>();  // Changed from SimpleEngine
    
    // Load opening book if enabled
    if (own_book) {
        load_opening_book();
    }
}

/**
 * @brief Main UCI command processing loop that handles communication with chess GUIs.
 *
 * This function implements the UCI (Universal Chess Interface) protocol by reading commands
 * from stdin and processing them appropriately. It sets up unbuffered I/O for immediate
 * communication with chess GUIs and continuously processes commands until a "quit" command
 * is received.
 *
 * Supported UCI commands include:
 * - uci: Engine identification and initialization
 * - debug: Toggle debug mode for verbose output
 * - isready: Engine readiness confirmation
 * - setoption: Configure engine parameters
 * - register: Registration handling (not required)
 * - ucinewgame: Start a new game
 * - position: Set board position from FEN or moves
 * - go: Start search with specified parameters
 * - d: Debug display of current position
 * - stop: Halt current search
 * - ponderhit: Ponder mode hit confirmation
 * - quit: Exit the engine
 *
 * @note All commands are processed with error handling and optional debug output.
 */
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

/**
 * @brief Sends engine identification information to the GUI.
 *
 * This function outputs the required UCI identification strings that inform the chess GUI
 * about the engine's name and author. This information is displayed in the GUI and used
 * for engine selection and tournament management.
 */
void UCIInterface::send_id() {
    std::cout << "id name Huginn 1.2" << std::endl;
    std::cout << "id author MTDuke71" << std::endl;
}

/**
 * @brief Sends available engine options to the GUI for configuration.
 *
 * This function declares the configurable options that the chess GUI can modify to
 * customize engine behavior. Each option includes its type, default value, and valid
 * range where applicable. The options include:
 * - Threads: Number of search threads (1-64)
 * - Ponder: Enable/disable pondering mode
 * - OwnBook: Enable/disable opening book usage
 * - BookFile: Path to the opening book file
 */
void UCIInterface::send_options() {
    std::cout << "option name Threads type spin default 1 min 1 max 64" << std::endl;
    std::cout << "option name Ponder type check default false" << std::endl;
    std::cout << "option name OwnBook type check default true" << std::endl;
    std::cout << "option name BookFile type string default src/performance.bin" << std::endl;
}

/**
 * @brief Splits a string into tokens separated by whitespace.
 *
 * This utility function tokenizes UCI command strings for easier parsing. It handles
 * multiple consecutive spaces and returns a vector of individual words/parameters
 * that can be processed by command handlers.
 *
 * @param str The input string to tokenize.
 * @return A vector of strings containing the individual tokens.
 */
std::vector<std::string> UCIInterface::split_string(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

/**
 * @brief Handles the UCI "position" command to set up the chess position.
 *
 * This function processes the position command which can specify either a starting
 * position ("startpos") or a custom FEN string ("fen"). It also handles any
 * subsequent moves that should be played from the given position. The moves
 * parameter contains moves in algebraic notation that are applied sequentially.
 *
 * @param tokens Vector containing the tokenized position command parts.
 *              Expected format: ["position", "startpos"|"fen", [fen_string], "moves", move1, move2, ...]
 */
void UCIInterface::handle_position(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) return;
    
    size_t move_index = 0;
    
    if (tokens[1] == "startpos") {
        position.set_startpos();
        move_index = 2;
    }
    else if (tokens[1] == "fen") {
        if (tokens.size() < 3) return; // Need at least "position fen [something]"
        
        // Build FEN string from tokens until we find "moves" or reach end
        std::string fen;
        size_t i = 2;
        while (i < tokens.size() && tokens[i] != "moves") {
            if (i > 2) fen += ' ';
            fen += tokens[i];
            i++;
        }
        
        if (fen.empty()) {
            if (debug_mode) std::cout << "info string Empty FEN string" << std::endl;
            return;
        }
        
        if (!position.set_from_fen(fen)) {
            if (debug_mode) std::cout << "info string Invalid FEN: " << fen << std::endl;
            return;
        }
        
        move_index = i; // Points to "moves" or end of tokens
    }
    else {
        if (debug_mode) std::cout << "info string Unknown position type: " << tokens[1] << std::endl;
        return;
    }

    // Process moves if present
    if (move_index < tokens.size() && tokens[move_index] == "moves") {
        for (size_t i = move_index + 1; i < tokens.size(); ++i) {
            S_MOVE move = parse_uci_move(tokens[i], position);
            if (move.move != 0) {
                if (position.MakeMove(move) != 1) {
                    if (debug_mode) std::cout << "info string Illegal move: " << tokens[i] << std::endl;
                }
            } else if (debug_mode) {
                std::cout << "info string Invalid move: " << tokens[i] << std::endl;
            }
        }
    }

    if (debug_mode) std::cout << "info string Position set, FEN: " << position.to_fen() << std::endl;
}

/**
 * @brief Handles the "go" command from the UCI protocol, parsing search parameters and starting the search.
 *
 * This function interprets the tokens from a UCI "go" command, extracting search parameters such as depth, movetime,
 * time controls (wtime, btime, winc, binc, movestogo), and infinite search mode. It sets up the search limits accordingly,
 * applying logic for classical, increment, and sudden death time controls, and ensures safe time allocation.
 * After parsing and configuring the search parameters, it initiates the search for the best move.
 *
 * @param tokens A vector of strings representing the tokenized "go" command and its parameters.
 *
 * @note If debug_mode is enabled, detailed info about parsing and parameter selection is printed to std::cout.
 */
void UCIInterface::handle_go(const std::vector<std::string>& tokens) {
    if (debug_mode) std::cout << "info string Starting search" << std::endl;
    should_stop = false;

    Huginn::MinimalLimits limits;
    limits.infinite = false;
    limits.max_time_ms = 0;  // Default to no time limit
    limits.max_depth = 25;   // Default search depth
    
    if (debug_mode) std::cout << "info string Debug: Parsing go command with " << tokens.size() << " tokens" << std::endl;

    // VICE Part 69: Parse go command parameters
    bool depth_specified = false;
    int winc = 0, binc = 0, movestogo = 0, wtime = -1, btime = -1;
    int movetime = -1;  // Track movetime separately for better debugging
    
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth" && i + 1 < tokens.size()) {
            try {
                limits.max_depth = std::stoi(tokens[i + 1]);
                depth_specified = true;
                if (debug_mode) std::cout << "info string Parsed depth: " << limits.max_depth << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid depth value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "movetime" && i + 1 < tokens.size()) {
            try {
                movetime = std::stoi(tokens[i + 1]);
                limits.max_time_ms = movetime;
                if (debug_mode) std::cout << "info string Parsed movetime: " << movetime << "ms" << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid movetime value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "wtime" && i + 1 < tokens.size()) {
            try {
                wtime = std::stoi(tokens[i + 1]);
                if (debug_mode) std::cout << "info string Parsed wtime: " << wtime << "ms" << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid wtime value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "btime" && i + 1 < tokens.size()) {
            try {
                btime = std::stoi(tokens[i + 1]);
                if (debug_mode) std::cout << "info string Parsed btime: " << btime << "ms" << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid btime value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "winc" && i + 1 < tokens.size()) {
            try {
                winc = std::stoi(tokens[i + 1]);
                if (debug_mode) std::cout << "info string Parsed winc: " << winc << "ms" << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid winc value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "binc" && i + 1 < tokens.size()) {
            try {
                binc = std::stoi(tokens[i + 1]);
                if (debug_mode) std::cout << "info string Parsed binc: " << binc << "ms" << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid binc value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "movestogo" && i + 1 < tokens.size()) {
            try {
                movestogo = std::stoi(tokens[i + 1]);
                if (debug_mode) std::cout << "info string Parsed movestogo: " << movestogo << std::endl;
                i++;
            } catch (const std::exception&) {
                if (debug_mode) std::cout << "info string Invalid movestogo value: " << tokens[i + 1] << std::endl;
                i++;
            }
        }
        else if (tokens[i] == "infinite") {
            limits.infinite = true;
            limits.max_time_ms = 0;
            if (debug_mode) std::cout << "info string Parsed infinite search mode" << std::endl;
        }
        else if (debug_mode) {
            std::cout << "info string Unknown go parameter: " << tokens[i] << std::endl;
        }
    }

    // VICE Part 69: Set search mode based on parsed parameters
    if (depth_specified) {
        limits.infinite = true;  // Depth-only search ignores time
        limits.max_time_ms = 0;
        if (debug_mode) std::cout << "info string Using depth-only search: " << limits.max_depth << std::endl;
    }
    else if (movetime > 0) {
        // Fixed time per move
        if (debug_mode) std::cout << "info string Using fixed movetime: " << movetime << "ms" << std::endl;
    }
    else if (!limits.infinite && (wtime > 0 || btime > 0)) {
        // Time control calculation (VICE Part 69 style with our enhancements)
        int side_time = (position.side_to_move == Color::White) ? wtime : btime;
        int side_inc = (position.side_to_move == Color::White) ? winc : binc;
        
        if (side_time > 0) {
            int alloc;
            if (movestogo > 0) {
                // Classical time control: divide remaining time by moves to go
                alloc = side_time / std::max(1, movestogo);
                alloc += side_inc / 2;  // Add half the increment
                if (debug_mode) std::cout << "info string Classical time control: " << side_time << "ms / " << movestogo << " moves" << std::endl;
            } else {
                // Sudden death or increment-only time control
                alloc = std::max(50, side_time / 20 + side_inc / 4);
                if (debug_mode) std::cout << "info string Sudden death time control: " << side_time << "ms base" << std::endl;
            }
            
            // Apply reserve time (don't use all available time)
            int reserve = std::min(1000, std::max(50, side_time / 10));
            alloc = std::max(50, alloc - reserve / std::max(1, movestogo > 0 ? movestogo : 1));
            
            // Cap at 60% of remaining time for safety
            alloc = std::min(alloc, static_cast<int>(side_time * 0.6));
            
            limits.max_time_ms = std::max(50, alloc);
            if (debug_mode) std::cout << "info string Calculated time allocation: " << limits.max_time_ms << "ms (reserve: " << reserve << "ms)" << std::endl;
        } else {
            // No time available, use increment if present
            if (movestogo == 0 && (winc > 0 || binc > 0)) {
                limits.max_time_ms += ((position.side_to_move == Color::White) ? winc : binc) / 4;
                if (debug_mode) std::cout << "info string Using increment-only time: " << limits.max_time_ms << "ms" << std::endl;
            }
        }
    }

    if (debug_mode) {
        std::cout << "info string Final search parameters: depth=" << limits.max_depth 
                  << " time=" << limits.max_time_ms << "ms infinite=" << (limits.infinite ? "true" : "false") << std::endl;
    }

    search_best_move(limits);
}

/**
 * @brief Handles UCI "setoption" commands to configure engine parameters.
 *
 * This function processes setoption commands that modify engine configuration
 * during UCI communication. It parses option names and values, applying them
 * to the appropriate engine settings. Common options include hash table size,
 * threads, and various engine-specific parameters.
 *
 * @param tokens Vector containing the tokenized setoption command parts.
 *               Expected format: ["setoption", "name", option_name, "value", option_value]
 */
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
        else if (option_name == "OwnBook") {
            bool new_own_book = (option_value == "true");
            if (new_own_book != own_book) {
                own_book = new_own_book;
                if (own_book) {
                    load_opening_book();
                } else {
                    // Disable book by clearing it
                    if (search_engine) {
                        search_engine->opening_book.clear();
                    }
                    if (debug_mode) {
                        std::cout << "info string Opening book disabled" << std::endl;
                    }
                }
            }
            if (debug_mode) {
                std::cout << "info string OwnBook set to " << (own_book ? "true" : "false") << std::endl;
            }
        }
        else if (option_name == "BookFile") {
            book_file = option_value;
            if (own_book) {
                load_opening_book();  // Reload with new file
            }
            if (debug_mode) {
                std::cout << "info string BookFile set to " << book_file << std::endl;
            }
        }
    }
}

/**
 * @brief Initiates the best move search with specified time and depth limits.
 *
 * This function starts the chess engine's search process to find the best move
 * from the current position. It configures the search with time controls,
 * depth limits, and other parameters, then begins the search process. The
 * search runs in the background and communicates results via UCI protocol.
 *
 * @param limits Search constraints including time controls, depth limits,
 *               and other search parameters that control how long and deep
 *               the engine should search.
 */
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

/**
 * @brief Loads the opening book from various possible file locations.
 *
 * This function attempts to load a Polyglot opening book file for the chess
 * engine. It searches through multiple potential file paths including user-
 * specified locations, current directory, and common installation directories.
 * The opening book provides the engine with known good opening moves to
 * improve early game performance.
 */
void UCIInterface::load_opening_book() {
    if (!search_engine) return;
    
    // Try multiple possible paths for the book file
    std::vector<std::string> possible_paths = {
        book_file,                       // User-specified path
        "src/performance.bin",           // From project root
        "performance.bin",               // Same directory as executable
        "../../../src/performance.bin", // From release bin directory to project src
        "../../src/performance.bin"     // From debug bin directory to project src
    };
    
    std::string found_path;
    bool found = false;
    
    for (const auto& path : possible_paths) {
        std::ifstream test_file(path);
        if (test_file.good()) {
            found_path = path;
            found = true;
            break;
        }
    }
    
    if (found) {
        if (search_engine->load_opening_book(found_path)) {
            if (debug_mode) {
                std::cout << "info string Opening book loaded: " << found_path << std::endl;
            }
        } else {
            if (debug_mode) {
                std::cout << "info string Failed to load opening book: " << found_path << std::endl;
            }
        }
    } else {
        if (debug_mode) {
            std::cout << "info string Opening book file not found. Tried:" << std::endl;
            for (const auto& path : possible_paths) {
                std::cout << "info string   " << path << std::endl;
            }
        }
    }
}

/**
 * @brief Signals the search engine to stop the current search operation.
 *
 * This function immediately terminates any ongoing search process when
 * the "stop" UCI command is received. It sets the stop flag, notifies
 * the search engine to halt, and records the stop time for accurate
 * performance tracking. This ensures responsive control from chess GUIs.
 */
void UCIInterface::signal_stop() {
    should_stop = true;
    if (search_engine) search_engine->stop();
    Huginn::SearchInfo* info_ptr = running_info.load();
    if (info_ptr) {
        info_ptr->stop_time = std::chrono::high_resolution_clock::now();
        info_ptr->stopped = true;
    }
}

