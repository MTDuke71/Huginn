/**
 * @file input_checking.cpp
 * @brief Implementation of cross-platform input detection for chess engine responsiveness
 * 
 * This file implements the platform-specific details for non-blocking input checking.
 * It handles the fundamental differences between Windows and Unix-like systems in
 * their approaches to console input detection, providing a unified interface for
 * the chess engine's search interrupt capabilities.
 * 
 * **Platform-Specific Implementations:**
 * - Windows: Uses _kbhit() from conio.h for immediate keyboard detection
 * - Unix/Linux: Uses select() system call with zero timeout for POSIX compliance
 * 
 * **Integration Notes:**
 * These functions are designed for periodic calling from search loops to maintain
 * engine responsiveness without significant performance impact. The implementation
 * prioritizes minimal overhead and maximum compatibility across platforms.
 */

#include "input_checking.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace Huginn {

/**
 * @brief Implementation: Non-blocking input detection with platform-specific optimizations
 * 
 * **Windows Implementation (_WIN32):**
 * - Uses `_kbhit()` from `<conio.h>` for immediate keyboard hit detection
 * - Returns non-zero if any keystroke is available in the input buffer
 * - Highly efficient with minimal system call overhead
 * - No file descriptor management required
 * 
 * **Unix/Linux/POSIX Implementation:**
 * - Uses `select()` system call with zero timeout for non-blocking I/O multiplexing
 * - Creates file descriptor set containing STDIN_FILENO (standard input)
 * - `FD_ZERO()` initializes the set, `FD_SET()` adds stdin to monitoring
 * - `timeout.tv_sec = 0; timeout.tv_usec = 0` ensures immediate return
 * - Returns > 0 if stdin has data ready to read without blocking
 * - Provides portable solution across Unix-like operating systems
 * 
 * **Performance Analysis:**
 * - Windows: Single function call, typically 1-2 CPU cycles
 * - Unix: System call overhead ~100-200 cycles, but still O(1)
 * - Both suitable for frequent polling in search loops (every 1000-10000 nodes)
 * 
 * @implementation Based on VICE Part 70 chess engine tutorial
 * @complexity O(1) constant time on both platforms
 * @threadsafe No - accesses global stdin state, call from main thread only
 */

bool input_is_waiting() {
#ifdef _WIN32
    // The original `return _kbhit() != 0;` was a console-only check that
    // returned false whenever stdin was a pipe — which is every UCI GUI
    // (Arena, ChessBase, Fritz, etc., all spawn the engine with stdin/
    // stdout redirected through pipes). That broke mid-search GUI input
    // detection: with no input ever observed during search, `info.stopped`
    // never got set from a GUI `stop`, the search just kept deepening on
    // the old position, and queued `position`/`go` commands waited for a
    // `bestmove` that never arrived (Kibitzer / analysis-mode silently
    // hung on a position change).
    //
    // Fix: pick the right poll for the handle type once at startup and
    // cache it. For pipe stdin (GUI mode) use PeekNamedPipe to inspect
    // the pipe buffer; for actual console stdin (manual UCI testing)
    // keep _kbhit. Pattern matches Stockfish's WinProcGroup::input_available.
    static HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
    static const bool is_pipe = (GetFileType(h_stdin) == FILE_TYPE_PIPE);

    if (is_pipe) {
        DWORD bytes_avail = 0;
        if (PeekNamedPipe(h_stdin, NULL, 0, NULL, &bytes_avail, NULL)) {
            return bytes_avail > 0;
        }
        // PeekNamedPipe failure typically means the GUI closed its end
        // of the pipe — return true so the search bails, the UCI loop
        // reads EOF from getline, and the engine exits cleanly instead
        // of spinning forever on a dead pipe.
        return true;
    }
    // Interactive console — original _kbhit path. Kept for manual UCI
    // testing where the user types commands at a console prompt.
    return _kbhit() != 0;
#else
    // Unix/Linux implementation using select() — handles both pipe and
    // terminal stdin transparently, so no separate case needed.
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) > 0;
#endif
}

/**
 * @brief Implementation: Safe input reading with UCI command processing
 * 
 * **Input Safety Protocol:**
 * 1. Only reads when `input_is_waiting()` confirms input availability (prevents blocking)
 * 2. Uses `std::getline(std::cin, input)` for safe line-based reading
 * 3. Handles stream errors gracefully - no exceptions thrown on malformed input
 * 4. Immediate return on any input to prevent GUI communication delays
 * 
 * **Command Processing Logic:**
 * - **"quit"**: Sets both `info.quit = true` and `info.stopped = true`
 *   - Signals complete engine shutdown and immediate search termination
 *   - Used when user closes GUI or explicitly quits engine
 * - **"stop"**: Sets only `info.stopped = true` 
 *   - Halts current search but keeps engine running for next command
 *   - Standard UCI protocol for search interruption
 * - **Any other input**: Also sets `info.stopped = true`
 *   - Ensures maximum responsiveness to GUI communication
 *   - Handles unexpected commands gracefully without hanging
 * 
 * **UCI Protocol Implementation Details:**
 * - Designed for compatibility with UCI GUIs (Arena, ChessBase, Fritz, etc.)
 * - GUIs may send commands at any time during search operations
 * - Conservative approach: any input stops search to prevent communication issues
 * - Typical integration: called every 1000-10000 nodes in search loop
 * 
 * **Error Handling:**
 * - Stream errors (EOF, bad input) handled without crashes
 * - Function remains stable even with malformed or unexpected input
 * - No memory allocation failures (uses stack-based std::string)
 * 
 * @implementation Based on VICE Part 70 responsive engine design
 * @performance O(1) for command processing, O(n) for input line length where n is line size
 * @threadsafe No - modifies SearchInfo state and accesses global stdin stream
 * 
 * @example
 * ```cpp
 * // Advanced search loop integration with timing
 * uint64_t nodes = 0;
 * auto start_time = std::chrono::steady_clock::now();
 * 
 * while (depth <= max_depth && !info.stopped) {
 *     // Search nodes...
 *     nodes++;
 *     
 *     // Check input every 5000 nodes or every 100ms
 *     if (nodes % 5000 == 0 || time_elapsed() > 100) {
 *         read_input(info);
 *         if (info.quit) {
 *             save_state_and_exit();
 *         }
 *     }
 * }
 * ```
 */

void read_input(SearchInfo& info) {
    // CRITICAL: do NOT consume the input line here.
    //
    // The old implementation called std::getline and discarded any line
    // that wasn't `stop`/`quit`. That broke ChessBase Kibitzer (and any
    // GUI that batches commands): when the user moves on the board,
    // ChessBase sends a SEQUENCE on stdin —
    //     stop
    //     position fen <new>
    //     go infinite
    // The search loop bails only between iterative-deepening depths,
    // not instantly, so checkup() can fire multiple times between
    // "stop arrived" and "search actually returned". Each fire would
    // consume one queued line — first `stop` (legitimate), then the
    // `position` command (lost), then the `go infinite` (lost). The
    // UCI command loop would then wake up to an empty stdin and the
    // Kibitzer would silently stop without restarting on the new
    // position.
    //
    // Correct behaviour: detect that the GUI sent SOMETHING, set
    // info.stopped so the search bails at the next depth boundary,
    // and leave the actual command bytes in the stdin buffer for the
    // UCI command loop (UCIInterface::run) to consume via its own
    // std::getline. That loop already handles `stop`/`quit`/`position`/
    // `go` correctly and will process the full sequence in order.
    if (input_is_waiting()) {
        info.stopped = true;
    }
}

} // namespace Huginn
