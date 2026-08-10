/**
 * @file main.cpp
 * @brief Program entry point — constructs the UCI interface and runs its command loop.
 *
 * With no arguments this is the UCI engine, exactly as before. The single
 * subcommand `datagen` runs NNUE training-data generation (#39) instead; it is a
 * developer tool and deliberately not reachable over the UCI protocol.
 */
#include <cstring>
#include <iostream>
#include <string>

#include "nnue.hpp"
#include "uci.hpp"

int run_datagen(int argc, char* argv[]);
int run_nnuecheck(int argc, char* argv[]);

namespace {

/// @brief Directory containing the running executable, with trailing separator.
std::string exe_dir(const char* argv0) {
    std::string p = argv0 ? argv0 : "";
    const size_t cut = p.find_last_of("/\\");
    return cut == std::string::npos ? std::string() : p.substr(0, cut + 1);
}

/**
 * @brief Auto-load `huginn.nnue` from the executable's own directory.
 *
 * The ladder tests binaries by copying them into the fastchess folder, so the
 * net has to travel with the exe rather than depend on the working directory.
 * Absent net file => nothing loads, NNUE stays off, and the binary is the
 * hand-crafted-eval engine it has always been.
 */
void autoload_net(const char* argv0) {
    const std::string path = exe_dir(argv0) + "huginn.nnue";
    if (Huginn::NNUE::load(path)) {
        Huginn::NNUE::set_enabled(true);
        std::cout << "info string NNUE loaded: " << Huginn::NNUE::description() << std::endl;
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::strcmp(argv[1], "datagen") == 0) {
        return run_datagen(argc, argv);
    }

    autoload_net(argc > 0 ? argv[0] : nullptr);

    if (argc >= 2 && std::strcmp(argv[1], "nnuecheck") == 0) {
        return run_nnuecheck(argc, argv);
    }

    UCIInterface uci;
    uci.run();
    return 0;
}
