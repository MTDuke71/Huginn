/**
 * @file main.cpp
 * @brief Program entry point — constructs the UCI interface and runs its command loop.
 */
#include "uci.hpp"

int main(int /*argc*/, char* /*argv*/[]) {
    UCIInterface uci;
    uci.run();
    return 0;
}
