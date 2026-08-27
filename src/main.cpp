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

/**
 * @brief Net file paired with this executable: `<exe path minus extension>.nnue`.
 *
 * Deliberately NOT a fixed `huginn.nnue`. The ladder keeps every historical
 * binary in one fastchess folder, so a fixed name would mean dropping one net
 * file there silently converts `huginn_2.4.exe` — and every other baseline in
 * the folder — into an NNUE engine, corrupting any future A/B that used them.
 * Naming the net after its exe makes each binary's evaluation self-contained.
 */
std::string paired_net_path(const char* argv0) {
    std::string p = argv0 ? argv0 : "";
    const size_t slash = p.find_last_of("/\\");
    const size_t dot = p.find_last_of('.');
    if (dot != std::string::npos && (slash == std::string::npos || dot > slash)) {
        p = p.substr(0, dot);
    }
    return p + ".nnue";
}

/**
 * @brief Auto-load the paired net, if one is sitting next to the executable.
 *
 * Absent net file => nothing loads, NNUE stays off, and the binary is the
 * hand-crafted-eval engine it has always been. This is what keeps a build with
 * the NNUE code compiled in bit-identical to 2.4 unless a net is deliberately
 * shipped alongside it.
 *
 * SILENT by design (#72): UCI forbids unsolicited stdout before the GUI
 * speaks (the same contract #56 pinned with SilentUntilUciThenHonestOptions,
 * which a startup print breaks the moment a net is deployed next to the exe).
 * The `uci` handler announces the net instead; the dev subcommands announce
 * it themselves in main().
 */
bool autoload_net(const char* argv0) {
    const std::string path = paired_net_path(argv0);
    if (Huginn::NNUE::load(path)) {
        Huginn::NNUE::set_enabled(true);
        return true;
    }
    return false;
}

}  // namespace

int main(int argc, char* argv[]) {
    // Autoload FIRST, before any subcommand dispatch. datagen in particular
    // must see the net: the iterative training loop is net -> better labels ->
    // better net, and generating a second-round corpus with the paired net
    // silently unloaded would relabel with the HCE and produce a round of data
    // no stronger than the first.
    const bool net_loaded = autoload_net(argc > 0 ? argv[0] : nullptr);

    if (argc >= 2 && (std::strcmp(argv[1], "datagen") == 0 ||
                      std::strcmp(argv[1], "nnuecheck") == 0)) {
        // Dev consoles, not UCI: say which eval is labelling, up front.
        // Datagen in particular must not silently relabel with the HCE (#39).
        if (net_loaded) {
            std::cout << "info string NNUE loaded: " << Huginn::NNUE::description() << std::endl;
        } else {
            std::cout << "info string no paired net; HCE evaluation" << std::endl;
        }
        return std::strcmp(argv[1], "datagen") == 0 ? run_datagen(argc, argv)
                                                    : run_nnuecheck(argc, argv);
    }

    UCIInterface uci;
    uci.run();
    return 0;
}
