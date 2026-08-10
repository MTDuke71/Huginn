/**
 * @file datagen.cpp
 * @brief NNUE training-data generation (#39): multithreaded self-play at fixed
 *        depth, every recorded position labelled with its own search score.
 *
 * Invoked as `huginn datagen <out_prefix> [options]`. Each worker owns one
 * output file (`<prefix>.<tid>.txt`) and appends to it after every game, so a
 * kill loses at most one game per thread rather than the whole session — the
 * append-only + per-thread-file requirement from docs/BACKLOG.md #39.
 *
 * ## Line format
 * `<score_cp_white_pov> <fen>` — one position per line.
 *
 * The backlog asks for positions and labels to be *separately joinable* so an
 * eval change costs a re-labelling pass rather than a regeneration. A two-column
 * text file satisfies that literally: column 2 is the reusable position corpus
 * and column 1 is the disposable label. Re-labelling reads the FENs, discards
 * column 1, and re-searches — a pass over the file, not 40 hours of self-play.
 *
 * ## What gets recorded, and why
 * Only quiet, non-extreme, past-the-opening positions are written:
 *  - **not in check / not a capture / not a promotion** — the leaf eval is what
 *    the net has to learn, and tactical positions are the search's job, not the
 *    eval's. Labelling them teaches the net to duplicate tactics it cannot see.
 *  - **|score| < 1500** — decided positions carry no eval gradient and their
 *    labels are dominated by mate-distance and TB scores (#66).
 *  - **ply >= 10** — the first plies are book, shared across thousands of games,
 *    and would be massively over-represented.
 *
 * Every worker uses a distinct, recorded seed (a session salt mixed with the
 * thread id), so repeated overnight sessions produce independent data instead of
 * one night's data N times — the single most expensive mistake available here.
 *
 * @see tools/train_nnue.py which consumes this output.
 */
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "init.hpp"
#include "movegen.hpp"
#include "position.hpp"
#include "nnue.hpp"
#include "search.hpp"

#include <cstring>

namespace {

struct Options {
    std::string prefix;
    std::string book = "C:\\Users\\m_lad\\Documents\\fastchess-windows-x86-64\\noob_3moves.epd";
    uint64_t games = 100000;
    int depth = 8;
    int threads = 14;
    double max_hours = 0.0;  // 0 = unlimited
    uint64_t seed = 0;       // 0 = derive from the clock
};

/// xorshift64 — cheap, and reproducible from a recorded seed.
inline uint64_t xorshift(uint64_t& s) {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return s;
}

std::vector<std::string> load_book(const std::string& path) {
    std::vector<std::string> out;
    std::ifstream in(path);
    if (!in) return out;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string board, stm, castle, ep;
        if (!(ss >> board >> stm >> castle >> ep)) continue;
        // EPD carries no clocks; append the two FEN counters.
        out.push_back(board + " " + stm + " " + castle + " " + ep + " 0 1");
    }
    return out;
}

std::atomic<uint64_t> g_games_started{0};
std::atomic<uint64_t> g_games_done{0};
std::atomic<uint64_t> g_positions{0};
std::atomic<bool> g_time_up{false};

void worker(int tid, const Options& opt, const std::vector<std::string>& book,
            std::chrono::steady_clock::time_point start) {
    uint64_t rng = opt.seed ^ (uint64_t(tid + 1) * 0x9E3779B97F4A7C15ULL);
    xorshift(rng);

    std::ofstream out(opt.prefix + "." + std::to_string(tid) + ".txt", std::ios::app);
    if (!out) {
        std::cerr << "datagen: cannot open output for thread " << tid << "\n";
        return;
    }

    Huginn::Engine engine;

    Position pos;
    std::vector<std::string> lines;

    while (true) {
        if (g_time_up.load(std::memory_order_relaxed)) break;
        if (g_games_started.fetch_add(1, std::memory_order_relaxed) >= opt.games) break;

        pos.set_from_fen(book[xorshift(rng) % book.size()]);
        engine.reset();
        lines.clear();

        for (int ply = 1; ply <= 320; ++ply) {
            if (pos.halfmove_clock >= 100) break;

            S_MOVELIST legal;
            generate_legal_moves(pos, legal);
            if (legal.count == 0) break;  // mate or stalemate

            S_MOVE chosen;
            bool have_label = false;
            int label = 0;

            // Exploration noise: a random legal move 8% of the time before ply
            // 20. This is the on-policy escape valve the backlog calls for —
            // without it the corpus only ever covers positions Huginn already
            // chooses to enter, which is precisely where its eval is already
            // right.
            if (ply < 20 && (xorshift(rng) % 100) < 8) {
                chosen = legal.moves[xorshift(rng) % size_t(legal.count)];
            } else {
                Huginn::SearchInfo info;
                info.max_depth = opt.depth;
                info.depth_only = true;
                info.polls_input = false;
                info.infinite = false;
                // is_helper suppresses all UCI output and the per-iteration PV
                // display walk. Search behaviour is unchanged; it is the
                // engine's existing "search but say nothing" role (#40 phase 3).
                info.is_helper = true;
                chosen = engine.searchPosition(pos, info);
                if (chosen.move == 0) break;
                label = engine.last_root_score;
                have_label = true;
            }

            if (have_label && ply >= 10 && std::abs(label) < 1500 && !in_check(pos) &&
                !chosen.is_capture() && !chosen.is_promotion()) {
                const int white_pov = pos.side_to_move == Color::White ? label : -label;
                lines.push_back(std::to_string(white_pov) + " " + pos.to_fen());
            }

            if (pos.MakeMove(chosen) != 1) break;  // should not happen on a legal move
        }

        for (const std::string& l : lines) out << l << '\n';
        out.flush();

        g_positions.fetch_add(lines.size(), std::memory_order_relaxed);
        const uint64_t done = g_games_done.fetch_add(1, std::memory_order_relaxed) + 1;

        if (tid == 0) {
            const auto now = std::chrono::steady_clock::now();
            const auto secs =
                std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            if (opt.max_hours > 0.0 && double(secs) >= opt.max_hours * 3600.0) {
                g_time_up.store(true, std::memory_order_relaxed);
            }
            if (done % 100 < uint64_t(opt.threads)) {
                const uint64_t n = g_positions.load(std::memory_order_relaxed);
                std::cout << "games " << done << "  positions " << n << "  ("
                          << (n / uint64_t(secs > 0 ? secs : 1)) << "/s)  elapsed "
                          << (secs / 60) << "m" << std::endl;
            }
        }
    }
}

}  // namespace

int run_datagen(int argc, char* argv[]) {
    Options opt;
    for (int i = 2; i < argc; ++i) {
        const std::string a = argv[i];
        auto next = [&](const char* def) -> std::string {
            return (i + 1 < argc) ? argv[++i] : def;
        };
        if (a == "--games") opt.games = std::strtoull(next("100000").c_str(), nullptr, 10);
        else if (a == "--depth") opt.depth = std::atoi(next("8").c_str());
        else if (a == "--threads") opt.threads = std::atoi(next("14").c_str());
        else if (a == "--book") opt.book = next("");
        else if (a == "--max-hours") opt.max_hours = std::atof(next("0").c_str());
        else if (a == "--seed") opt.seed = std::strtoull(next("0").c_str(), nullptr, 10);
        else opt.prefix = a;
    }

    if (opt.prefix.empty()) {
        std::cerr << "usage: huginn datagen <out_prefix> [--games N] [--depth D] "
                     "[--threads T] [--book path] [--max-hours H] [--seed S]\n";
        return 1;
    }
    if (opt.seed == 0) {
        opt.seed = uint64_t(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    Huginn::init();

    const std::vector<std::string> book = load_book(opt.book);
    if (book.empty()) {
        std::cerr << "datagen: empty or missing book at " << opt.book << "\n";
        return 1;
    }

    // The manifest is append-only and records the seed: without it, seven
    // nights of generation produce one night of unique data seven times.
    {
        std::ofstream mf(opt.prefix + ".manifest.txt", std::ios::app);
        mf << "seed=" << opt.seed << " games=" << opt.games << " depth=" << opt.depth
           << " threads=" << opt.threads
           << " teacher=" << (Huginn::NNUE::enabled() ? "NNUE" : "HCE")
           << " book=" << opt.book << "\n";
    }

    std::cout << "datagen: " << book.size() << " openings, " << opt.games << " games, depth "
              << opt.depth << ", " << opt.threads << " threads, seed " << opt.seed
              << ", teacher " << (Huginn::NNUE::enabled() ? "NNUE" : "HCE") << " -> "
              << opt.prefix << ".N.txt" << std::endl;

    const auto start = std::chrono::steady_clock::now();
    std::vector<std::thread> pool;
    pool.reserve(size_t(opt.threads));
    for (int t = 0; t < opt.threads; ++t) pool.emplace_back(worker, t, std::cref(opt),
                                                           std::cref(book), start);
    for (std::thread& t : pool) t.join();

    const auto secs = std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();
    std::cout << "done: " << g_positions.load() << " positions from " << g_games_done.load()
              << " games in " << (secs / 60) << "m" << std::endl;
    return 0;
}

/**
 * @brief `huginn nnuecheck <fen|startpos> <depth>` — accumulator self-test (#39).
 *
 * Walks the full legal move tree to `depth` and, at every node, compares the
 * incrementally-maintained accumulator against a from-scratch refresh. The
 * incremental path is only correct if `TakeMove` replays the exact inverse of
 * every piece operation `MakeMove` performed, which is an assumption about
 * castling, en passant, and promotion-with-capture that is far cheaper to test
 * than to debug through a bad training run.
 *
 * A mismatch is reported with the offending FEN and the walk aborts.
 */
int run_nnuecheck(int argc, char* argv[]) {
    Huginn::init();

    const std::string fen = argc > 2 ? argv[2] : "startpos";
    const int depth = argc > 3 ? std::atoi(argv[3]) : 4;

    if (!Huginn::NNUE::loaded()) {
        std::cerr << "nnuecheck: no net loaded (needs huginn.nnue next to the exe)\n";
        return 1;
    }
    Huginn::NNUE::set_enabled(true);

    Position pos;
    if (fen == "startpos") pos.set_startpos();
    else if (!pos.set_from_fen(fen)) {
        std::cerr << "nnuecheck: bad FEN\n";
        return 1;
    }

    uint64_t nodes = 0, mismatches = 0;

    // Recursive walk. `probe` holds a from-scratch rebuild so the live
    // accumulator is never disturbed by the check itself.
    struct Walker {
        uint64_t& nodes;
        uint64_t& mismatches;
        void go(Position& p, int d) {
            ++nodes;
            Position probe = p;
            Huginn::NNUE::refresh(probe);
            if (std::memcmp(probe.acc.v, p.acc.v, sizeof(p.acc.v)) != 0) {
                if (mismatches < 5) {
                    std::cerr << "MISMATCH at " << p.to_fen() << "\n";
                }
                ++mismatches;
            }
            if (d == 0) return;
            S_MOVELIST list;
            generate_all_moves(p, list);
            for (int i = 0; i < list.count; ++i) {
                if (p.MakeMove(list.moves[i]) != 1) continue;
                go(p, d - 1);
                p.TakeMove();
            }
        }
    } w{nodes, mismatches};

    w.go(pos, depth);

    std::cout << "nnuecheck: " << nodes << " nodes, " << mismatches << " mismatches"
              << std::endl;
    return mismatches == 0 ? 0 : 1;
}
