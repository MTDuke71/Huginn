#include "uci.hpp"

/**
 * @brief Entry point of the application.
 *
 * Initializes the UCI (Universal Chess Interface) and starts its main loop.
 * 
 * @return int Returns 0 upon successful execution.
 */

int main() {
    UCIInterface uci;
    uci.run();
    return 0;
}
