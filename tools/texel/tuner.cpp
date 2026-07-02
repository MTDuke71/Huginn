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
    // #9 round 2: endgame PSTs for the non-king pieces (tapered PSTs).
    add(Huginn::EvalParams::PAWN_TABLE_EG);
    add(Huginn::EvalParams::KNIGHT_TABLE_EG);
    add(Huginn::EvalParams::BISHOP_TABLE_EG);
    add(Huginn::EvalParams::ROOK_TABLE_EG);
    add(Huginn::EvalParams::QUEEN_TABLE_EG);
    // Mobility weights (mg / eg).
    p.push_back(&Huginn::EvalParams::MOBILITY_WEIGHT_DEFAULT);
    p.push_back(&Huginn::EvalParams::MOBILITY_WEIGHT_ENDGAME);
    // Safe-mobility per-piece weights (#9 round 9).
    p.push_back(&Huginn::EvalParams::KNIGHT_MOBILITY_MG);
    p.push_back(&Huginn::EvalParams::KNIGHT_MOBILITY_EG);
    p.push_back(&Huginn::EvalParams::BISHOP_MOBILITY_MG);
    p.push_back(&Huginn::EvalParams::BISHOP_MOBILITY_EG);
    p.push_back(&Huginn::EvalParams::ROOK_MOBILITY_MG);
    p.push_back(&Huginn::EvalParams::ROOK_MOBILITY_EG);
    p.push_back(&Huginn::EvalParams::QUEEN_MOBILITY_MG);
    p.push_back(&Huginn::EvalParams::QUEEN_MOBILITY_EG);
    // #9 round 3: positional scalars (well-constrained by quiet data) + the
    // passed-pawn rank bonus. KS is deliberately excluded — its non-linear
    // attacker term fires too rarely in quiet positions to tune reliably.
    for (int i = 0; i < 8; ++i) p.push_back(&Huginn::EvalParams::PASSED_PAWN_BONUS[i]);
    p.push_back(&Huginn::EvalParams::BISHOP_PAIR_BONUS);
    p.push_back(&Huginn::EvalParams::ROOK_OPEN_FILE_BONUS);
    p.push_back(&Huginn::EvalParams::ROOK_SEMI_OPEN_FILE_BONUS);
    p.push_back(&Huginn::EvalParams::QUEEN_OPEN_FILE_BONUS);
    p.push_back(&Huginn::EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS);
    p.push_back(&Huginn::EvalParams::ISOLATED_PAWN_PENALTY);
    p.push_back(&Huginn::EvalParams::DOUBLED_PAWN_PENALTY);
    p.push_back(&Huginn::EvalParams::TEMPO_BONUS);
    // #9 round 4: connected + backward pawn terms. Ranks 1/8 can't hold pawns,
    // so connected indices 0 and 7 stay pinned at 0 (excluded from tuning).
    for (int i = 1; i <= 6; ++i) {
        p.push_back(&Huginn::EvalParams::CONNECTED_PAWN_BONUS_MG[i]);
        p.push_back(&Huginn::EvalParams::CONNECTED_PAWN_BONUS_EG[i]);
    }
    p.push_back(&Huginn::EvalParams::BACKWARD_PAWN_PENALTY_MG);
    p.push_back(&Huginn::EvalParams::BACKWARD_PAWN_PENALTY_EG);
    // #9 round 5: rook on the relative 7th rank (tapered MG/EG).
    p.push_back(&Huginn::EvalParams::ROOK_ON_7TH_MG);
    p.push_back(&Huginn::EvalParams::ROOK_ON_7TH_EG);
    // #9 round 8 candidate: knight/bishop outposts (tapered MG/EG).
    p.push_back(&Huginn::EvalParams::KNIGHT_OUTPOST_BONUS_MG);
    p.push_back(&Huginn::EvalParams::KNIGHT_OUTPOST_BONUS_EG);
    p.push_back(&Huginn::EvalParams::BISHOP_OUTPOST_BONUS_MG);
    p.push_back(&Huginn::EvalParams::BISHOP_OUTPOST_BONUS_EG);
    // #9 round 6: threats (tapered MG/EG per attacker->target class).
    p.push_back(&Huginn::EvalParams::THREAT_PAWN_ON_MINOR_MG);
    p.push_back(&Huginn::EvalParams::THREAT_PAWN_ON_MINOR_EG);
    p.push_back(&Huginn::EvalParams::THREAT_PAWN_ON_ROOK_MG);
    p.push_back(&Huginn::EvalParams::THREAT_PAWN_ON_ROOK_EG);
    p.push_back(&Huginn::EvalParams::THREAT_PAWN_ON_QUEEN_MG);
    p.push_back(&Huginn::EvalParams::THREAT_PAWN_ON_QUEEN_EG);
    p.push_back(&Huginn::EvalParams::THREAT_MINOR_ON_ROOK_MG);
    p.push_back(&Huginn::EvalParams::THREAT_MINOR_ON_ROOK_EG);
    p.push_back(&Huginn::EvalParams::THREAT_MINOR_ON_QUEEN_MG);
    p.push_back(&Huginn::EvalParams::THREAT_MINOR_ON_QUEEN_EG);
    p.push_back(&Huginn::EvalParams::THREAT_ROOK_ON_QUEEN_MG);
    p.push_back(&Huginn::EvalParams::THREAT_ROOK_ON_QUEEN_EG);
    // #9 round 7: king-safety attacker weights (Knight..Queen) + open-file
    // shelter. Now tunable since the >=2-attacker gate was removed (the term
    // fires on quiet positions). MG-only in the eval, so no EG counterpart.
    for (int pt = int(PieceType::Knight); pt <= int(PieceType::Queen); ++pt)
        p.push_back(&Huginn::EvalParams::KS_ATTACK_WEIGHT[size_t(pt)]);
    p.push_back(&Huginn::EvalParams::KS_OPEN_FILE_PENALTY);
    // #20: trapped-bishop locks (tapered MG/EG, full a7-tier + lighter a6-tier).
    p.push_back(&Huginn::EvalParams::P_BISHOP_TRAPPED_A7_MG);
    p.push_back(&Huginn::EvalParams::P_BISHOP_TRAPPED_A7_EG);
    p.push_back(&Huginn::EvalParams::P_BISHOP_TRAPPED_A6_MG);
    p.push_back(&Huginn::EvalParams::P_BISHOP_TRAPPED_A6_EG);
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
    print_array("PAWN_TABLE_EG", Huginn::EvalParams::PAWN_TABLE_EG);
    print_array("KNIGHT_TABLE_EG", Huginn::EvalParams::KNIGHT_TABLE_EG);
    print_array("BISHOP_TABLE_EG", Huginn::EvalParams::BISHOP_TABLE_EG);
    print_array("ROOK_TABLE_EG", Huginn::EvalParams::ROOK_TABLE_EG);
    print_array("QUEEN_TABLE_EG", Huginn::EvalParams::QUEEN_TABLE_EG);
    std::printf("MOBILITY_WEIGHT_DEFAULT = %d;\n", Huginn::EvalParams::MOBILITY_WEIGHT_DEFAULT);
    std::printf("MOBILITY_WEIGHT_ENDGAME = %d;\n", Huginn::EvalParams::MOBILITY_WEIGHT_ENDGAME);
    std::printf("KNIGHT_MOBILITY_MG = %d;\n", Huginn::EvalParams::KNIGHT_MOBILITY_MG);
    std::printf("KNIGHT_MOBILITY_EG = %d;\n", Huginn::EvalParams::KNIGHT_MOBILITY_EG);
    std::printf("BISHOP_MOBILITY_MG = %d;\n", Huginn::EvalParams::BISHOP_MOBILITY_MG);
    std::printf("BISHOP_MOBILITY_EG = %d;\n", Huginn::EvalParams::BISHOP_MOBILITY_EG);
    std::printf("ROOK_MOBILITY_MG = %d;\n", Huginn::EvalParams::ROOK_MOBILITY_MG);
    std::printf("ROOK_MOBILITY_EG = %d;\n", Huginn::EvalParams::ROOK_MOBILITY_EG);
    std::printf("QUEEN_MOBILITY_MG = %d;\n", Huginn::EvalParams::QUEEN_MOBILITY_MG);
    std::printf("QUEEN_MOBILITY_EG = %d;\n", Huginn::EvalParams::QUEEN_MOBILITY_EG);
    std::printf("PASSED_PAWN_BONUS = {");
    for (int i = 0; i < 8; ++i) std::printf(" %d,", Huginn::EvalParams::PASSED_PAWN_BONUS[i]);
    std::printf(" };\n");
    std::printf("BISHOP_PAIR_BONUS = %d;\n", Huginn::EvalParams::BISHOP_PAIR_BONUS);
    std::printf("ROOK_OPEN_FILE_BONUS = %d;\n", Huginn::EvalParams::ROOK_OPEN_FILE_BONUS);
    std::printf("ROOK_SEMI_OPEN_FILE_BONUS = %d;\n", Huginn::EvalParams::ROOK_SEMI_OPEN_FILE_BONUS);
    std::printf("QUEEN_OPEN_FILE_BONUS = %d;\n", Huginn::EvalParams::QUEEN_OPEN_FILE_BONUS);
    std::printf("QUEEN_SEMI_OPEN_FILE_BONUS = %d;\n", Huginn::EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS);
    std::printf("ISOLATED_PAWN_PENALTY = %d;\n", Huginn::EvalParams::ISOLATED_PAWN_PENALTY);
    std::printf("DOUBLED_PAWN_PENALTY = %d;\n", Huginn::EvalParams::DOUBLED_PAWN_PENALTY);
    std::printf("TEMPO_BONUS = %d;\n", Huginn::EvalParams::TEMPO_BONUS);
    std::printf("CONNECTED_PAWN_BONUS_MG = {");
    for (int i = 0; i < 8; ++i) std::printf(" %d,", Huginn::EvalParams::CONNECTED_PAWN_BONUS_MG[i]);
    std::printf(" };\n");
    std::printf("CONNECTED_PAWN_BONUS_EG = {");
    for (int i = 0; i < 8; ++i) std::printf(" %d,", Huginn::EvalParams::CONNECTED_PAWN_BONUS_EG[i]);
    std::printf(" };\n");
    std::printf("BACKWARD_PAWN_PENALTY_MG = %d;\n", Huginn::EvalParams::BACKWARD_PAWN_PENALTY_MG);
    std::printf("BACKWARD_PAWN_PENALTY_EG = %d;\n", Huginn::EvalParams::BACKWARD_PAWN_PENALTY_EG);
    std::printf("ROOK_ON_7TH_MG = %d;\n", Huginn::EvalParams::ROOK_ON_7TH_MG);
    std::printf("ROOK_ON_7TH_EG = %d;\n", Huginn::EvalParams::ROOK_ON_7TH_EG);
    std::printf("KNIGHT_OUTPOST_BONUS_MG = %d;\n", Huginn::EvalParams::KNIGHT_OUTPOST_BONUS_MG);
    std::printf("KNIGHT_OUTPOST_BONUS_EG = %d;\n", Huginn::EvalParams::KNIGHT_OUTPOST_BONUS_EG);
    std::printf("BISHOP_OUTPOST_BONUS_MG = %d;\n", Huginn::EvalParams::BISHOP_OUTPOST_BONUS_MG);
    std::printf("BISHOP_OUTPOST_BONUS_EG = %d;\n", Huginn::EvalParams::BISHOP_OUTPOST_BONUS_EG);
    std::printf("THREAT_PAWN_ON_MINOR_MG = %d;\n",  Huginn::EvalParams::THREAT_PAWN_ON_MINOR_MG);
    std::printf("THREAT_PAWN_ON_MINOR_EG = %d;\n",  Huginn::EvalParams::THREAT_PAWN_ON_MINOR_EG);
    std::printf("THREAT_PAWN_ON_ROOK_MG = %d;\n",   Huginn::EvalParams::THREAT_PAWN_ON_ROOK_MG);
    std::printf("THREAT_PAWN_ON_ROOK_EG = %d;\n",   Huginn::EvalParams::THREAT_PAWN_ON_ROOK_EG);
    std::printf("THREAT_PAWN_ON_QUEEN_MG = %d;\n",  Huginn::EvalParams::THREAT_PAWN_ON_QUEEN_MG);
    std::printf("THREAT_PAWN_ON_QUEEN_EG = %d;\n",  Huginn::EvalParams::THREAT_PAWN_ON_QUEEN_EG);
    std::printf("THREAT_MINOR_ON_ROOK_MG = %d;\n",  Huginn::EvalParams::THREAT_MINOR_ON_ROOK_MG);
    std::printf("THREAT_MINOR_ON_ROOK_EG = %d;\n",  Huginn::EvalParams::THREAT_MINOR_ON_ROOK_EG);
    std::printf("THREAT_MINOR_ON_QUEEN_MG = %d;\n", Huginn::EvalParams::THREAT_MINOR_ON_QUEEN_MG);
    std::printf("THREAT_MINOR_ON_QUEEN_EG = %d;\n", Huginn::EvalParams::THREAT_MINOR_ON_QUEEN_EG);
    std::printf("THREAT_ROOK_ON_QUEEN_MG = %d;\n",  Huginn::EvalParams::THREAT_ROOK_ON_QUEEN_MG);
    std::printf("THREAT_ROOK_ON_QUEEN_EG = %d;\n",  Huginn::EvalParams::THREAT_ROOK_ON_QUEEN_EG);
    std::printf("KS_ATTACK_WEIGHT = { %d, %d, %d, %d, %d, %d, %d };\n",
        Huginn::EvalParams::KS_ATTACK_WEIGHT[0], Huginn::EvalParams::KS_ATTACK_WEIGHT[1],
        Huginn::EvalParams::KS_ATTACK_WEIGHT[2], Huginn::EvalParams::KS_ATTACK_WEIGHT[3],
        Huginn::EvalParams::KS_ATTACK_WEIGHT[4], Huginn::EvalParams::KS_ATTACK_WEIGHT[5],
        Huginn::EvalParams::KS_ATTACK_WEIGHT[6]);
    std::printf("KS_OPEN_FILE_PENALTY = %d;\n", Huginn::EvalParams::KS_OPEN_FILE_PENALTY);
    std::printf("P_BISHOP_TRAPPED_A7_MG = %d;\n", Huginn::EvalParams::P_BISHOP_TRAPPED_A7_MG);
    std::printf("P_BISHOP_TRAPPED_A7_EG = %d;\n", Huginn::EvalParams::P_BISHOP_TRAPPED_A7_EG);
    std::printf("P_BISHOP_TRAPPED_A6_MG = %d;\n", Huginn::EvalParams::P_BISHOP_TRAPPED_A6_MG);
    std::printf("P_BISHOP_TRAPPED_A6_EG = %d;\n", Huginn::EvalParams::P_BISHOP_TRAPPED_A6_EG);
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
