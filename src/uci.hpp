/**
 * @file uci.hpp
 * @brief Universal Chess Interface (UCI) protocol implementation
 * 
 * Implements the UCI protocol for communication between the Huginn chess engine
 * and UCI-compatible chess GUIs. The UCI protocol is the standard interface used
 * by most modern chess programs and provides a clean separation between the
 * engine's chess logic and the user interface.
 * 
 * ## UCI Protocol Features
 * - **Standard Commands**: uci, isready, position, go, stop, quit
 * - **Engine Options**: Hash table size, threads, opening book settings
 * - **Search Control**: Time management, depth limits, node limits
 * - **Move Communication**: Algebraic notation move parsing and output
 * 
 * ## Implementation Details
 * - **Synchronous Search + Command Pump**: searches run on the command loop's
 *   thread; mid-search stdin is drained by a pump (#56) that answers `isready`,
 *   applies `stop`/`quit`, and queues other commands for in-order replay
 * - **Thread Safety**: cancellation is a single atomic on the Engine;
 *   SearchInfo is only ever written by the searching thread
 * - **Command Parsing**: Robust input parsing with error handling
 * - **Debug Mode**: Enhanced logging for debugging and analysis
 * 
 * ## Search Integration
 * - Engine integration for search operations
 * - SearchInfo structure for search state management
 * - Time management and iteration control
 * - Principal variation and score reporting
 * 
 * @author MTDuke71
 * @version 1.2
 * @see search.hpp for search engine interface
 * @see https://www.shredderchess.com/chess-info/features/uci-universal-chess-interface.html
 */

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <deque>
#include <memory>
#include "position.hpp"
#include "movegen.hpp"
#include "search.hpp"  // Changed from search.hpp
#include "syzygy_tablebase.hpp"

/**
 * @brief UCI protocol front-end: owns the engine + root position, parses GUI
 *        commands from stdin, and drives searches (async, stoppable).
 */
class UCIInterface {
private:
    Position position;                                       ///< Current root position (set by the `position` command).
    std::unique_ptr<Huginn::Engine> search_engine;          ///< The search engine (owns the transposition table).
    std::unique_ptr<Huginn::SyzygyTablebase> tablebase;     ///< Syzygy probe wrapper (disabled until SyzygyPath is set).
    bool debug_mode = false;                                ///< Extra `info string` logging when true.
    bool quit_received = false;                             ///< `quit` seen (possibly mid-search); run() exits after unwinding.
    std::deque<std::string> pending_commands;               ///< Commands received mid-search, replayed in order after `bestmove` (#56).

    // Opening book settings
    bool own_book = false;  ///< Opening book OFF by default (set OwnBook=true to enable).
                            ///< Prevents ad-hoc UCI/analysis runs from silently playing book
                            ///< moves instead of searching; gauntlets pass OwnBook explicitly,
                            ///< so this default does not affect measured strength.
    std::string book_file = "src/performance.bin";          ///< Polyglot book path (UCI `BookFile` option).

public:
    /// @brief Split @p str into whitespace-separated tokens.
    std::vector<std::string> split_string(const std::string& str);
    /// @brief Handle `position [startpos | fen <fen>] [moves ...]` — set the root position.
    void handle_position(const std::vector<std::string>& tokens);
    /// @brief Handle `go ...` — parse limits / time controls and launch the search.
    void handle_go(const std::vector<std::string>& tokens);
    /// @brief Handle `setoption name <id> value <v>` (Hash, OwnBook, BookFile, SyzygyPath).
    void handle_setoption(const std::vector<std::string>& tokens);
    /// @brief Run a search under @p limits and emit `info` lines + the final `bestmove`.
    ///        With @p hold_for_stop (`go infinite`), a search that completes on its
    ///        own parks in wait_for_stop() — `bestmove` is not sent until `stop`/`quit`.
    void search_best_move(const Huginn::MinimalLimits& limits, bool hold_for_stop = false);
    /// @brief (Re)load the Polyglot opening book from book_file (no-op unless own_book).
    void load_opening_book();
    /// @brief Drain every line currently waiting on stdin through
    ///        handle_search_input_line() (installed as SearchInfo::on_input).
    void pump_search_input(Huginn::SearchInfo& info);
    /// @brief Park after a naturally-completed `go infinite` search: keep
    ///        answering `isready`/queueing commands until `stop`/`quit` (or
    ///        signal_stop()), per the UCI infinite contract (#56).
    void wait_for_stop(Huginn::SearchInfo& info);

public:
    UCIInterface();

    /// @brief Main UCI loop — read and dispatch commands from stdin until `quit`/EOF.
    void run();
    /// @brief Dispatch one full command line (the body of run()'s loop; also used
    ///        to replay commands queued mid-search). @return false on `quit`.
    bool dispatch_command(const std::string& line);
    /// @brief #56 pump: handle ONE line that arrived while a search is running.
    ///        `isready` → immediate `readyok` (search keeps going); `stop`/`quit` →
    ///        flag @p info; `debug` applies; `ponderhit` is a no-op (no pondering);
    ///        anything else is queued for dispatch after `bestmove`.
    ///        @return false when pumping should cease (stop/quit seen).
    bool handle_search_input_line(const std::string& line, Huginn::SearchInfo& info);
    /// @brief Commands queued by the mid-search pump (test observability).
    const std::deque<std::string>& pending() const { return pending_commands; }
    /// @brief Read-only view of the current root position (tests: BACKLOG #54
    ///        transactionality — a rejected `position` command must not move it).
    const Position& current_position() const { return position; }
    /// @brief Signal stop to the running search from any thread (tests; mirrors
    ///        the UCI `stop` command). Sets only the engine's atomic — the search
    ///        thread translates it into its own SearchInfo (#56, race-free).
    void signal_stop();
    /// @brief Emit the `id name` / `id author` lines.
    void send_id();
    /// @brief Emit the `option name ...` advertisement lines.
    void send_options();
};
