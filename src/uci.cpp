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
 * @version 2.0
 */
#include "uci.hpp"
#include "init.hpp"
#include "square.hpp"
#include "uci_utils.hpp"
#include "movegen.hpp"
#include "input_checking.hpp"
#include <fstream>
#include <algorithm>

/**
 * @brief Constructs a new UCIInterface instance and initializes the chess engine.
 *
 * This constructor performs the essential initialization steps for the UCI interface:
 * - Initializes the Huginn chess engine core systems
 * - Sets the position to the standard starting position
 * - Creates a Engine instance for search operations
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

    // #56: tablebases stay DISABLED until the GUI configures SyzygyPath.
    // The old constructor auto-probed a hard-coded c:\TB\ and the tablebase
    // layer wrote a raw status line to stdout before the GUI even sent `uci`
    // — unsolicited startup output the protocol forbids.
    tablebase = std::make_unique<Huginn::SyzygyTablebase>();

    // Initialize search engine with tablebase
    search_engine = std::make_unique<Huginn::Engine>(tablebase.get());

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
        if (!dispatch_command(line)) return;
        // #56: commands the mid-search pump queued (position/go/setoption/...)
        // replay in arrival order BEFORE reading stdin again — anything the GUI
        // sent after them is still sitting in the stdin buffer, so global
        // command order is preserved.
        while (!pending_commands.empty()) {
            std::string queued = std::move(pending_commands.front());
            pending_commands.pop_front();
            if (!dispatch_command(queued)) return;
        }
    }
}

bool UCIInterface::dispatch_command(const std::string& line) {
    if (quit_received) return false;  // quit arrived mid-search: exit before anything else
    if (line.empty()) return true;
    auto tokens = split_string(line);
    if (tokens.empty()) return true;
    const std::string& command = tokens[0];

    if (debug_mode) std::cout << "info string Received command: " << line << std::endl;

    {
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
            // New game: wipe cross-game state so stale entries from the previous
            // game can't be probed on a transposition (there is no TT aging yet,
            // #42). This MUST live here, not in reset(): reset() also runs once
            // per `go` (search_best_move), so clearing the TT there would destroy
            // it every move. (#46)
            search_engine->tt_table.clear();
            search_engine->clear_search_tables();
            if (debug_mode) std::cout << "info string New game started (TT + search tables cleared)" << std::endl;
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
                    int square = sq64(File(file), Rank(rank));
                    Piece p = position.at_sq64(square);
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
        else if (command == "eval") {
            // Static evaluation from the side-to-move's perspective
            // (matches the search's internal convention). Used by the
            // BACKLOG #28 repetition tooling to check the t6 winning-
            // repetition demotion threshold against real positions.
            int cp = search_engine->evalPosition(position);
            std::cout << "info string static eval " << cp
                      << " cp (side-to-move POV)" << std::endl;
            std::cout << "eval " << cp << std::endl;
        }
        else if (command == "stop") {
            // #56: searches are synchronous — a `stop` DURING a search is
            // consumed by the mid-search pump, so by the time this loop sees
            // one there is no search running and the bestmove is already out.
            // A stale stop is ignored per protocol.
            if (debug_mode) std::cout << "info string stop with no search running (ignored)" << std::endl;
        }
        else if (command == "ponderhit") {
            if (debug_mode) std::cout << "info string Ponder hit" << std::endl;
        }
        else if (command == "quit") {
            return false;
        }
        else {
            if (debug_mode) std::cout << "info string Unknown command: " << command << std::endl;
        }
    }
    return !quit_received;  // quit may also arrive via the mid-search pump
}

/**
 * @brief Sends engine identification information to the GUI.
 *
 * This function outputs the required UCI identification strings that inform the chess GUI
 * about the engine's name and author. This information is displayed in the GUI and used
 * for engine selection and tournament management.
 */
void UCIInterface::send_id() {
    std::cout << "id name Huginn 2.2" << std::endl;
    std::cout << "id author MTDuke71" << std::endl;
}

/**
 * @brief Sends available engine options to the GUI for configuration.
 *
 * This function declares the configurable options that the chess GUI can modify to
 * customize engine behavior. Each option includes its type, default value, and valid
 * range where applicable. The options include:
 * - Hash: Transposition table size in MB
 * - OwnBook: Enable/disable opening book usage
 * - BookFile: Path to the opening book file
 * - SyzygyPath: Tablebase directory (default empty = disabled)
 */
void UCIInterface::send_options() {
    std::cout << "option name Hash type spin default 64 min 1 max 4096" << std::endl;
    // #56: Threads and Ponder are NOT advertised — the engine is
    // single-threaded and does not ponder; advertising them promised
    // support that did not exist. Re-add when implemented.
    std::cout << "option name OwnBook type check default false" << std::endl;
    std::cout << "option name BookFile type string default src/performance.bin" << std::endl;
    // #56: tablebases default to DISABLED — no hard-coded c:\TB\ auto-probe.
    // `<empty>` is the UCI convention for an empty string default.
    std::cout << "option name SyzygyPath type string default <empty>" << std::endl;
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

    // BACKLOG #54: the whole command is transactional. Build the new root in
    // a scratch position — FEN (or startpos), structural validation, and the
    // complete move list — and commit to the live root only when EVERYTHING
    // succeeded. Any failure (bad FEN, illegal position, first bad move)
    // rejects the command and leaves the current root exactly as it was; the
    // old code committed a valid prefix, silently desyncing engine and GUI.
    Position new_position;
    size_t move_index = 0;

    if (tokens[1] == "startpos") {
        new_position.set_startpos();
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

        if (!new_position.set_from_fen(fen)) {
            if (debug_mode) std::cout << "info string Invalid FEN: " << fen << std::endl;
            return;
        }

        // Syntactically valid FEN, but is the position one the engine can
        // search safely? (kings, piece counts, idle-side check, castle/EP
        // coherence — see validate_uci_position)
        std::string why;
        if (!validate_uci_position(new_position, &why)) {
            if (debug_mode) std::cout << "info string Illegal position (" << why << "): " << fen << std::endl;
            return;
        }

        move_index = i; // Points to "moves" or end of tokens
    }
    else {
        if (debug_mode) std::cout << "info string Unknown position type: " << tokens[1] << std::endl;
        return;
    }

    // Process moves if present. Stop at the FIRST invalid/illegal move and
    // reject the entire command — never commit a valid prefix.
    if (move_index < tokens.size() && tokens[move_index] == "moves") {
        for (size_t i = move_index + 1; i < tokens.size(); ++i) {
            S_MOVE move = parse_uci_move(tokens[i], new_position);
            if (move.move == 0 || new_position.MakeMove(move) != 1) {
                if (debug_mode) std::cout << "info string Rejecting position command, bad move: " << tokens[i] << std::endl;
                return;
            }
        }
    }
    else if (move_index < tokens.size()) {
        if (debug_mode) std::cout << "info string Rejecting position command, unexpected token: " << tokens[move_index] << std::endl;
        return;
    }

    position = std::move(new_position);

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

    Huginn::MinimalLimits limits;
    limits.infinite = false;
    limits.max_time_ms = 0;  // Default to no time limit
    limits.max_depth = 25;   // Default search depth

    if (debug_mode) std::cout << "info string Debug: Parsing go command with " << tokens.size() << " tokens" << std::endl;

    // #56 (part 3): strict numeric `go` parameters — whole-token parse with an
    // overflow guard (saturates at 1e9 ≈ 11.6 days, which also bounds all the
    // downstream 64-bit arithmetic), clamped to a sane range. Junk leaves the
    // parameter unset instead of std::stoi's prefix-parse ("12junk" -> 12) or
    // exception; negative clocks (GUIs send them when a side is flagging)
    // clamp to 0 and get the emergency budget.
    bool depth_specified = false;
    bool infinite_requested = false;  // the literal `go infinite` token (#56)
    constexpr long long GO_TIME_MAX_MS = 1000000000LL;
    long long winc = 0, binc = 0, movestogo = 0, wtime = -1, btime = -1;
    long long movetime = -1;

    // Consume the value token after `tokens[i]` into @p out (clamped to
    // [lo, hi]); on junk the parameter stays unset and the token is skipped.
    auto parse_go_number = [&](size_t& i, const char* name, long long lo,
                               long long hi, long long& out) -> bool {
        if (i + 1 >= tokens.size()) return false;
        ++i;
        long long v = 0;
        if (!parse_spin_clamped(tokens[i], lo, hi, v)) {
            if (debug_mode) std::cout << "info string Invalid " << name << " value: " << tokens[i] << std::endl;
            return false;
        }
        out = v;
        if (debug_mode) std::cout << "info string Parsed " << name << ": " << v << std::endl;
        return true;
    };

    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth") {
            long long depth = 0;
            if (parse_go_number(i, "depth", 1, Huginn::MAX_DEPTH, depth)) {
                limits.max_depth = static_cast<int>(depth);
                depth_specified = true;
            }
        }
        else if (tokens[i] == "movetime")  parse_go_number(i, "movetime", 1, GO_TIME_MAX_MS, movetime);
        else if (tokens[i] == "wtime")     parse_go_number(i, "wtime", 0, GO_TIME_MAX_MS, wtime);
        else if (tokens[i] == "btime")     parse_go_number(i, "btime", 0, GO_TIME_MAX_MS, btime);
        else if (tokens[i] == "winc")      parse_go_number(i, "winc", 0, GO_TIME_MAX_MS, winc);
        else if (tokens[i] == "binc")      parse_go_number(i, "binc", 0, GO_TIME_MAX_MS, binc);
        else if (tokens[i] == "movestogo") parse_go_number(i, "movestogo", 1, 500, movestogo);
        else if (tokens[i] == "infinite") {
            limits.infinite = true;
            infinite_requested = true;
            limits.max_time_ms = 0;
            if (debug_mode) std::cout << "info string Parsed infinite search mode" << std::endl;
        }
        else if (debug_mode) {
            std::cout << "info string Unknown go parameter: " << tokens[i] << std::endl;
        }
    }

    // #56: `go infinite` must run until `stop` — the old default depth cap of
    // 25 let it return a bestmove on its own, which the protocol forbids. Give
    // it the engine's full depth range (every ply-indexed structure is guarded
    // at 64); if the loop still completes (mate/stalemate root), search_best_move
    // parks in wait_for_stop() instead of emitting bestmove.
    if (infinite_requested && !depth_specified) {
        limits.max_depth = Huginn::MAX_DEPTH;
    }

    // VICE Part 69: Set search mode based on parsed parameters
    if (depth_specified) {
        limits.infinite = true;  // Depth-only search ignores time
        limits.max_time_ms = 0;
        if (debug_mode) std::cout << "info string Using depth-only search: " << limits.max_depth << std::endl;
    }
    else if (movetime > 0) {
        // Fixed time per move (parse already clamped to [1, GO_TIME_MAX_MS])
        limits.max_time_ms = static_cast<int>(movetime);
        if (debug_mode) std::cout << "info string Using fixed movetime: " << movetime << "ms" << std::endl;
    }
    else if (!limits.infinite && (wtime >= 0 || btime >= 0)) {
        // Clock-based allocation — pure, 64-bit, boundary-safe (#56 part 3;
        // strategy itself unchanged from the gauntleted #47 tuning). A clock
        // at/below the reserve yields a 1 ms emergency budget: an instant
        // legal move beats the old 50 ms floor's overdraft time forfeit.
        const bool white_to_move = (position.side_to_move == Color::White);
        const long long side_time = white_to_move ? wtime : btime;  // -1 = our clock not sent
        const long long side_inc = white_to_move ? winc : binc;

        limits.max_time_ms = static_cast<int>(compute_time_budget_ms(side_time, side_inc, movestogo));
        if (debug_mode) {
            std::cout << "info string Time allocation: " << limits.max_time_ms
                      << "ms (clock " << side_time << "ms, inc " << side_inc
                      << "ms, movestogo " << movestogo << ")" << std::endl;
        }
    }
    else if (!limits.infinite) {
        // Bare `go` with no limits at all: a defined default budget. The old
        // code left max_time_ms at 0, so the first checkup stopped the search
        // ~2048 nodes in and the depth-1 move went out — accidental behaviour,
        // not a contract.
        limits.max_time_ms = 5000;
        if (debug_mode) std::cout << "info string No limits given, using default 5000ms" << std::endl;
    }

    if (debug_mode) {
        std::cout << "info string Final search parameters: depth=" << limits.max_depth
                  << " time=" << limits.max_time_ms << "ms infinite=" << (limits.infinite ? "true" : "false") << std::endl;
    }

    search_best_move(limits, infinite_requested);
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
// #56: the strict numeric parser (parse_spin_clamped) lives in uci_utils.cpp —
// shared between setoption spin values and `go` parameters since part 3.

void UCIInterface::handle_setoption(const std::vector<std::string>& tokens) {
    // #56: full `setoption name <name...> [value <value...>]` grammar —
    // option names and values may contain spaces (BookFile, SyzygyPath
    // paths), and the value clause may be absent or empty. The old parser
    // took exactly one token for each, truncating paths at the first space.
    if (tokens.size() < 3 || tokens[1] != "name") return;

    std::string option_name;
    std::string option_value;
    size_t ti = 2;
    for (; ti < tokens.size() && tokens[ti] != "value"; ++ti) {
        if (!option_name.empty()) option_name += ' ';
        option_name += tokens[ti];
    }
    if (ti < tokens.size()) {  // consume "value" and join the rest
        for (++ti; ti < tokens.size(); ++ti) {
            if (!option_value.empty()) option_value += ' ';
            option_value += tokens[ti];
        }
    }
    if (option_name.empty()) return;

    {
        if (option_name == "Hash") {
            // Resize the transposition table to the requested size in MB,
            // clamped to the advertised range (1..4096 MB); junk is rejected.
            long long hash_mb = 0;
            if (parse_spin_clamped(option_value, 1, 4096, hash_mb)) {
                if (search_engine) {
                    search_engine->tt_table.resize_mb(static_cast<size_t>(hash_mb));
                }
                if (debug_mode) {
                    std::cout << "info string Hash set to " << hash_mb << " MB ("
                              << (search_engine ? search_engine->tt_table.get_size() : 0)
                              << " entries)" << std::endl;
                }
            } else if (debug_mode) {
                std::cout << "info string Hash value invalid: " << option_value << std::endl;
            }
        }
        // #56: Threads / Ponder handlers removed with their advertisements —
        // unknown or unadvertised options are ignored per protocol.
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
        else if (option_name == "SyzygyPath") {
            if (tablebase) {
                // #56: `info string` feedback is unconditional — it is a
                // protocol-safe response to an explicit configuration action,
                // and a silent failure here means playing without TBs the
                // user thinks are on.
                if (option_value.empty() || option_value == "<empty>") {
                    tablebase->shutdown();
                    std::cout << "info string Syzygy tablebases disabled" << std::endl;
                } else if (tablebase->initialize(option_value)) {
                    std::cout << "info string " << tablebase->get_info() << std::endl;
                } else {
                    std::cout << "info string Failed to load Syzygy tablebases from: " << option_value << std::endl;
                }
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
void UCIInterface::search_best_move(const Huginn::MinimalLimits& limits, bool hold_for_stop) {
    // Reset the search engine
    search_engine->reset();

    // Engine uses a different search interface - searchPosition
    Huginn::SearchInfo info;
    info.max_depth = limits.max_depth;
    info.stopped = false;
    info.infinite = limits.infinite;

    // #56: install the mid-search command pump. checkup() calls it when stdin
    // has pending input: `isready` is answered without stopping, `stop`/`quit`
    // flag this SearchInfo (on this thread — no cross-thread writes), and
    // other commands queue for the run() loop to replay after bestmove.
    info.on_input = [this](Huginn::SearchInfo& i) { pump_search_input(i); };

    // Convert time limits
    auto search_start = std::chrono::steady_clock::now();
    info.start_time = search_start;
    info.stop_time = search_start + std::chrono::milliseconds(limits.max_time_ms);

    // Perform the search using Engine interface
    S_MOVE best_move;
    // Snapshot the pre-search root position. searchPosition() runs on
    // `position` by reference; a rare, time-dependent make/unmake imbalance can
    // leave the engine's internal board desynced from the real position, so the
    // returned move may be illegal in the *actual* root position (fastchess then
    // forfeits the game on "Illegal move" — BACKLOG #12-followup). We validate
    // the chosen move against this clean snapshot before emitting bestmove.
    auto pre_search = position;
    try {
        best_move = search_engine->searchPosition(position, info);  // Engine method
    } catch (const std::exception& e) {
        std::cout << "info string Search threw exception: " << e.what() << std::endl;
        std::cout.flush();
        best_move.move = 0; // Set to invalid move to trigger fallback
    } catch (...) {
        std::cout << "info string Search threw unknown exception" << std::endl;
        std::cout.flush();
        best_move.move = 0; // Set to invalid move to trigger fallback
    }

    // Illegal-move / board-desync guard. Validate the search's chosen move
    // against a freshly generated *legal* move list for the clean pre-search
    // position. This catches both "no move returned" (best_move == 0) and the
    // rare board-desync case where the search returns a move that is illegal in
    // the real position. On a miss we substitute a legal move so the engine
    // never forfeits on an illegal bestmove, and we emit a diagnostic capturing
    // the exact FEN + offending move so the underlying corruption can be
    // reproduced and root-caused.
    S_MOVELIST legal_moves;
    generate_legal_moves(pre_search, legal_moves);

    bool best_is_legal = false;
    if (best_move.move != 0) {
        for (int i = 0; i < legal_moves.count; ++i) {
            if (legal_moves.moves[i].move == best_move.move) {
                best_is_legal = true;
                break;
            }
        }
    }

    if (!best_is_legal) {
        if (best_move.move != 0) {
            // Non-zero but illegal: a real board-desync escaped the search.
            // Log loudly so a gauntlet run captures a deterministic repro.
            std::cout << "info string board-desync guard: search returned illegal move "
                      << search_engine->move_to_uci(best_move)
                      << " for fen " << pre_search.to_fen()
                      << " — substituting a legal move" << std::endl;
            std::cout.flush();
        }
        best_move.move = 0;
        if (legal_moves.count > 0) {
            best_move = legal_moves.moves[0]; // legal fallback (never forfeit)
        }
        // Drop the corrupted internal board; the next `position` command
        // rebuilds it, but restoring now keeps any follow-up consistent.
        position = pre_search;
    }
    
    // #56: `go infinite` contract — bestmove must not be sent until the GUI
    // says stop. If the search completed on its own (mate/stalemate root, or
    // the full depth range exhausted) rather than being stopped, park here:
    // keep answering `isready` and queueing commands until stop/quit arrives.
    if (hold_for_stop && !info.stopped && !info.quit && !quit_received) {
        wait_for_stop(info);
    }
    if (info.quit) quit_received = true;

    // Engine already outputs complete UCI info during search
    // No need for additional summary output here

    // Send the best move - use Engine's move_to_uci
    if (best_move.move != 0) {
        std::string uci_move = search_engine->move_to_uci(best_move);  // Engine method
        std::cout << "bestmove " << uci_move << std::endl;
        std::cout.flush(); // Ensure immediate output
    } else {
        // Last resort fallback (no legal moves at root: mate or stalemate)
        std::cout << "bestmove 0000" << std::endl;
        std::cout.flush(); // Ensure immediate output
    }
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
 * @brief Signals the running search to stop, safely from any thread.
 *
 * #56: sets ONLY the engine's atomic stop flag. checkup() (and wait_for_stop)
 * poll it on the searching thread and translate it into the local SearchInfo,
 * so the stack SearchInfo has a single writer. The old implementation
 * published a raw pointer to the stack SearchInfo and wrote its non-atomic
 * stopped/stop_time fields from this thread — a data race and a lifetime
 * hazard if the search unwound while the write was in flight.
 */
void UCIInterface::signal_stop() {
    if (search_engine) search_engine->stop();
}

/**
 * @brief #56 mid-search pump: handle one command line that arrived during a
 *        search. See uci.hpp for the per-command contract.
 */
bool UCIInterface::handle_search_input_line(const std::string& line, Huginn::SearchInfo& info) {
    auto tokens = split_string(line);
    if (tokens.empty()) return true;
    const std::string& command = tokens[0];

    if (command == "isready") {
        // The spec is explicit: isready sent while calculating must be
        // answered immediately WITHOUT stopping the search.
        std::cout << "readyok" << std::endl;
        std::cout.flush();
    }
    else if (command == "stop") {
        info.stopped = true;
        return false;
    }
    else if (command == "quit") {
        info.stopped = true;
        info.quit = true;
        quit_received = true;
        return false;
    }
    else if (command == "debug") {
        if (tokens.size() > 1) debug_mode = (tokens[1] == "on");
    }
    else if (command == "ponderhit") {
        // No pondering support (not advertised) — nothing to switch.
        if (debug_mode) std::cout << "info string ponderhit ignored (no ponder)" << std::endl;
    }
    else {
        // position / go / setoption / ucinewgame / ... — NOT a reason to stop.
        // Queue for the run() loop to replay in order after bestmove.
        pending_commands.push_back(line);
        if (debug_mode) std::cout << "info string queued during search: " << line << std::endl;
    }
    return true;
}

/**
 * @brief Drain every line currently waiting on stdin through the pump.
 *
 * Installed as SearchInfo::on_input; called from checkup() on the searching
 * thread whenever input_is_waiting(). Stops draining once stop/quit is seen —
 * any lines still in the stdin buffer are read by the run() loop after
 * bestmove, which drains pending_commands first, preserving arrival order.
 * EOF (GUI closed the pipe) is treated as quit.
 */
void UCIInterface::pump_search_input(Huginn::SearchInfo& info) {
    while (Huginn::input_is_waiting()) {
        std::string line;
        if (!std::getline(std::cin, line)) {
            info.stopped = true;
            info.quit = true;
            quit_received = true;
            return;
        }
        if (!handle_search_input_line(line, info)) return;
    }
}

/**
 * @brief Park after a naturally-completed `go infinite` search (#56).
 *
 * The UCI infinite contract forbids sending bestmove before `stop`. Poll both
 * stdin (isready still answered, commands still queued) and the engine's
 * atomic stop flag (signal_stop from tests / another thread) until released.
 */
void UCIInterface::wait_for_stop(Huginn::SearchInfo& info) {
    while (!info.stopped && !info.quit && !quit_received) {
        if (search_engine->should_stop.load(std::memory_order_relaxed)) break;
        if (Huginn::input_is_waiting()) {
            pump_search_input(info);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}
