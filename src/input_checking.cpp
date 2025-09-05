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

// VICE Part 70: Check if input is waiting on stdin without blocking (1:57)
bool input_is_waiting() {
#ifdef _WIN32
    // Windows implementation using _kbhit()
    return _kbhit() != 0;
#else
    // Unix/Linux implementation using select()
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) > 0;
#endif
}

// VICE Part 70: Read input if available and process commands (2:32)
void read_input(SearchInfo& info) {
    // Only try to read if input is actually waiting
    if (!input_is_waiting()) {
        return;
    }
    
    std::string input;
    
    // Read the input line
    if (std::getline(std::cin, input)) {
        // Check for quit command (3:01)
        if (input == "quit") {
            info.quit = true;
            info.stopped = true;
            return;
        }
        
        // Check for stop command 
        if (input == "stop") {
            info.stopped = true;
            return;
        }
        
        // Any other input also stops the search (GUI might send other commands)
        // This ensures the engine is responsive to GUI communication
        info.stopped = true;
    }
}

} // namespace Huginn
