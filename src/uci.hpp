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
 * - **Asynchronous Search**: Multi-threaded search with stop capability
 * - **Thread Safety**: Atomic operations for search control variables
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
#include <memory>
#include "position.hpp"
#include "movegen.hpp"
#include "search.hpp"  // Changed from search.hpp
#include "syzygy_tablebase.hpp"

// UCI constants following VICE Part 67 recommendations
constexpr int UCI_INPUT_BUFFER_SIZE = 400 * 6; // Large buffer for GUI commands

/**
 * @brief UCI protocol front-end: owns the engine + root position, parses GUI
 *        commands from stdin, and drives searches (async, stoppable).
 */
class UCIInterface {
private:
    Position position;                                       ///< Current root position (set by the `position` command).
    std::unique_ptr<Huginn::Engine> search_engine;          ///< The search engine (owns the transposition table).
    std::unique_ptr<Huginn::SyzygyTablebase> tablebase;     ///< Syzygy probe wrapper (may be uninitialized).
    std::atomic<bool> is_searching{false};                  ///< True while a search thread is active.
    std::atomic<bool> should_stop{false};                   ///< Set by `stop`/`quit` to abort the search.
    std::atomic<Huginn::SearchInfo*> running_info{nullptr}; ///< Live SearchInfo so stop() can flag it safely across threads.
    bool debug_mode = false;                                ///< Extra `info string` logging when true.
    int threads = 1;                                        ///< UCI `Threads` option (the engine is single-threaded).

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
    /// @brief Handle `setoption name <id> value <v>` (Hash, Threads, OwnBook, BookFile, SyzygyPath).
    void handle_setoption(const std::vector<std::string>& tokens);
    /// @brief Run a search under @p limits and emit `info` lines + the final `bestmove`.
    void search_best_move(const Huginn::MinimalLimits& limits);
    /// @brief (Re)load the Polyglot opening book from book_file (no-op unless own_book).
    void load_opening_book();

public:
    UCIInterface();

    /// @brief Main UCI loop — read and dispatch commands from stdin until `quit`/EOF.
    void run();
    /// @brief Read-only view of the current root position (tests: BACKLOG #54
    ///        transactionality — a rejected `position` command must not move it).
    const Position& current_position() const { return position; }
    /// @brief Test helper: signal stop to the running search (mirrors the UCI `stop` command).
    void signal_stop();
    /// @brief Emit the `id name` / `id author` lines.
    void send_id();
    /// @brief Emit the `option name ...` advertisement lines.
    void send_options();
};
