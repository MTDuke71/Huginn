// Texel tuner for Huginn (#9).
//
// Reads labeled quiet positions ("<result> <FEN>" per line, result = game
// outcome from White's POV in {1.0, 0.5, 0.0}) and tunes the evaluation's
// material + piece-square tables to minimize the mean-squared error of
// sigmoid(K * white_eval) against the result.
//
// MUST be built with -DHUGINN_TUNING so the eval parameter tables are mutable
// inline globals (see chess_types.hpp EVAL_PARAM). The release engine keeps
// them constexpr (zero cost).
//
// Pipeline: tools/texel/extract_fens.py  ->  fens.txt  ->  this tuner  ->
// paste the printed tables back into chess_types.hpp / evaluation.hpp, rebuild,
// SPRT vs baseline-t10.
//
// Usage:
//   huginn_tuner fens.txt [--positions N] [--k K] [--max-sweeps S]

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "init.hpp"
#include "position.hpp"
#include "search.hpp"
#include "chess_types.hpp"
#include "evaluation.hpp"

#ifndef HUGINN_TUNING
#error "tuner must be built with -DHUGINN_TUNING (eval tables would be constexpr/immutable otherwise)"
#endif

namespace {

struct Sample {
    Position pos;
    float result;   // White POV: 1.0 win / 0.5 draw / 0.0 loss
};

double g_K = 1.0;

inline double white_eval(Huginn::Engine& eng, const Position& pos) {
    int e = eng.evaluate(pos);
    return (pos.side_to_move == Color::White) ? double(e) : double(-e);
}

inline double sigmoid(double s) {
    return 1.0 / (1.0 + std::pow(10.0, -g_K * s / 400.0));
}

double mse(Huginn::Engine& eng, const std::vector<Sample>& s) {
    // evaluate() is pure (reads position + global eval tables, writes no Engine
    // state), so all threads safely share one Engine for concurrent reads.
    unsigned nt = std::max(1u, std::thread::hardware_concurrency());
    if (s.size() < 40000) nt = 1;  // not worth the thread overhead
    std::vector<double> partial(nt, 0.0);
    std::vector<std::thread> th;
    const size_t chunk = (s.size() + nt - 1) / nt;
    for (unsigned t = 0; t < nt; ++t) {
        const size_t a = size_t(t) * chunk;
        const size_t b = std::min(s.size(), a + chunk);
        if (a >= b) break;
        th.emplace_back([&, t, a, b] {
            double sum = 0.0;
            for (size_t i = a; i < b; ++i) {
                double d = double(s[i].result) - sigmoid(white_eval(eng, s[i].pos));
                sum += d * d;
            }
            partial[t] = sum;
        });
    }
    for (auto& x : th) x.join();
    double sum = 0.0;
    for (double p : partial) sum += p;
    return sum / double(s.size());
}

// Scan K to minimize MSE at the current parameters.
double fit_k(Huginn::Engine& eng, const std::vector<Sample>& s) {
    double bestK = g_K, bestE = 1e18;
    for (double K = 0.20; K <= 2.0001; K += 0.02) {
        g_K = K;
        double e = mse(eng, s);
        if (e < bestE) { bestE = e; bestK = K; }
    }
    g_K = bestK;
    return bestK;
}

// Collect pointers to every tunable eval entry (mutable under HUGINN_TUNING).
std::vector<int*> collect_params() {
    std::vector<int*> p;
    // Material Pawn..Queen, MG + EG (skip None=0 and King=sentinel).
    for (int pt = int(PieceType::Pawn); pt <= int(PieceType::Queen); ++pt) {
        p.push_back(&PIECE_VALUES_MG[size_t(pt)]);
        p.push_back(&PIECE_VALUES_EG[size_t(pt)]);
    }
    auto add = [&](std::array<int, 64>& t) { for (int i = 0; i < 64; ++i) p.push_back(&t[i]); };
    add(Huginn::EvalParams::PAWN_TABLE);
    add(Huginn::EvalParams::KNIGHT_TABLE);
    add(Huginn::EvalParams::BISHOP_TABLE);
    add(Huginn::EvalParams::ROOK_TABLE);
    add(Huginn::EvalParams::QUEEN_TABLE);
    add(Huginn::EvalParams::KING_TABLE);
    add(Huginn::EvalParams::KING_TABLE_ENDGAME);
    return p;
}

// Per-parameter line search: step in the improving direction until it stops.
double optimize(Huginn::Engine& eng, const std::vector<Sample>& s,
                std::vector<int*>& params, int max_sweeps) {
    double cur = mse(eng, s);
    std::printf("start MSE = %.6f (K=%.3f), %zu params\n", cur, g_K, params.size());
    for (int sweep = 1; sweep <= max_sweeps; ++sweep) {
        double before = cur;
        int changed = 0;
        for (int* p : params) {
            const int orig = *p;
            *p = orig + 1;
            double e = mse(eng, s);
            int dir = 0;
            if (e < cur) { cur = e; dir = +1; }
            else {
                *p = orig - 1;
                e = mse(eng, s);
                if (e < cur) { cur = e; dir = -1; }
                else { *p = orig; }
            }
            if (dir != 0) {
                ++changed;
                while (true) {                 // keep going while improving
                    int v = *p; *p = v + dir;
                    double e2 = mse(eng, s);
                    if (e2 < cur) cur = e2;
                    else { *p = v; break; }
                }
            }
        }
        std::printf("sweep %2d: MSE %.6f -> %.6f  (%d params moved)\n",
                    sweep, before, cur, changed);
        std::fflush(stdout);
        if (changed == 0) { std::printf("converged.\n"); break; }
    }
    return cur;
}

void print_array(const char* name, const std::array<int, 64>& t) {
    std::printf("%s = {\n", name);
    for (int r = 0; r < 8; ++r) {
        std::printf("    ");
        for (int f = 0; f < 8; ++f) std::printf("%4d,", t[size_t(r * 8 + f)]);
        std::printf("\n");
    }
    std::printf("};\n");
}

void dump_results() {
    std::printf("\n==================== TUNED VALUES ====================\n");
    std::printf("PIECE_VALUES_MG = { 0,");
    for (int pt = int(PieceType::Pawn); pt <= int(PieceType::Queen); ++pt)
        std::printf(" %d,", PIECE_VALUES_MG[size_t(pt)]);
    std::printf(" 20000 };\n");
    std::printf("PIECE_VALUES_EG = { 0,");
    for (int pt = int(PieceType::Pawn); pt <= int(PieceType::Queen); ++pt)
        std::printf(" %d,", PIECE_VALUES_EG[size_t(pt)]);
    std::printf(" 20000 };\n\n");
    print_array("PAWN_TABLE", Huginn::EvalParams::PAWN_TABLE);
    print_array("KNIGHT_TABLE", Huginn::EvalParams::KNIGHT_TABLE);
    print_array("BISHOP_TABLE", Huginn::EvalParams::BISHOP_TABLE);
    print_array("ROOK_TABLE", Huginn::EvalParams::ROOK_TABLE);
    print_array("QUEEN_TABLE", Huginn::EvalParams::QUEEN_TABLE);
    print_array("KING_TABLE", Huginn::EvalParams::KING_TABLE);
    print_array("KING_TABLE_ENDGAME", Huginn::EvalParams::KING_TABLE_ENDGAME);
    std::printf("=====================================================\n");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s fens.txt [--positions N] [--k K] [--max-sweeps S]\n", argv[0]);
        return 1;
    }
    std::string fens_path = argv[1];
    long max_positions = 0;       // 0 = all
    double fixed_k = 0.0;         // 0 = auto-fit
    int max_sweeps = 30;
    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--positions" && i + 1 < argc) max_positions = std::stol(argv[++i]);
        else if (a == "--k" && i + 1 < argc)    fixed_k = std::stod(argv[++i]);
        else if (a == "--max-sweeps" && i + 1 < argc) max_sweeps = std::stoi(argv[++i]);
    }

    Huginn::init();
    Huginn::Engine engine;

    std::printf("loading %s ...\n", fens_path.c_str());
    std::ifstream in(fens_path);
    if (!in) { std::fprintf(stderr, "cannot open %s\n", fens_path.c_str()); return 1; }

    std::vector<Sample> samples;
    std::string line;
    long bad = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        double res;
        if (!(ss >> res)) { ++bad; continue; }
        std::string fen;
        std::getline(ss, fen);
        if (!fen.empty() && fen[0] == ' ') fen.erase(0, 1);
        Sample s;
        if (!s.pos.set_from_fen(fen)) { ++bad; continue; }
        s.result = float(res);
        samples.push_back(std::move(s));
        if (max_positions && long(samples.size()) >= max_positions) break;
    }
    std::printf("loaded %zu positions (%ld skipped)\n", samples.size(), bad);
    if (samples.empty()) return 1;

    if (fixed_k > 0.0) { g_K = fixed_k; std::printf("using fixed K=%.3f\n", g_K); }
    else { std::printf("fitting K ...\n"); double k = fit_k(engine, samples); std::printf("K=%.3f\n", k); }

    auto params = collect_params();
    optimize(engine, samples, params, max_sweeps);
    dump_results();
    return 0;
}
