// BACKLOG #56 (part 2): search-control contract at the UCI boundary.
//
// Covers the mid-search command pump (isready answered without stopping,
// stop/quit applied, other commands queued in order), the race-free
// cancellation channel (signal_stop -> engine atomic only; SearchInfo has a
// single writer), the `go infinite` bestmove lifetime (no bestmove until
// stop), silent startup (no unsolicited stdout before `uci`), and the
// Syzygy default-disabled contract. The end-to-end pipe behaviour (real
// stdin) is exercised by test_uci_transcript.cpp against the huginn binary.

#include "gtest/gtest.h"
#include "../src/uci.hpp"
#include "../src/init.hpp"
#include "../src/syzygy_tablebase.hpp"
#include <atomic>
#include <chrono>
#include <sstream>
#include <thread>

namespace {

// RAII stdout capture (same pattern as the existing UCI stop tests).
class CaptureCout {
public:
    CaptureCout() : old_buf(std::cout.rdbuf(oss.rdbuf())) {}
    ~CaptureCout() { std::cout.rdbuf(old_buf); }
    std::string str() const { return oss.str(); }
private:
    std::ostringstream oss;
    std::streambuf* old_buf;
};

// Black to move and checkmated (back rank): the root search finds no legal
// moves and completes instantly — the fastest natural completion available,
// used to expose the `go infinite` hold-until-stop contract.
const std::vector<std::string> MATED_ROOT_POSITION =
    {"position", "fen", "R5k1/5ppp/8/8/8/8/8/K7", "b", "-", "-", "0", "1"};

} // namespace

// --- Mid-search pump: per-line contract -------------------------------------

TEST(UciSearchControl, IsreadyDuringSearchAnswersWithoutStopping) {
    Huginn::init();
    UCIInterface uci;
    Huginn::SearchInfo info;

    CaptureCout cap;
    bool keep_pumping = uci.handle_search_input_line("isready", info);

    EXPECT_TRUE(keep_pumping);
    EXPECT_FALSE(info.stopped) << "isready must not stop the search";
    EXPECT_FALSE(info.quit);
    EXPECT_NE(cap.str().find("readyok"), std::string::npos);
    EXPECT_TRUE(uci.pending().empty()) << "isready is answered, not queued";
}

TEST(UciSearchControl, StopLineStopsSearchOnly) {
    Huginn::init();
    UCIInterface uci;
    Huginn::SearchInfo info;

    CaptureCout cap;
    bool keep_pumping = uci.handle_search_input_line("stop", info);

    EXPECT_FALSE(keep_pumping);
    EXPECT_TRUE(info.stopped);
    EXPECT_FALSE(info.quit) << "stop is not quit";
    EXPECT_TRUE(uci.pending().empty());
}

TEST(UciSearchControl, QuitLineStopsAndQuits) {
    Huginn::init();
    UCIInterface uci;
    Huginn::SearchInfo info;

    CaptureCout cap;
    bool keep_pumping = uci.handle_search_input_line("quit", info);

    EXPECT_FALSE(keep_pumping);
    EXPECT_TRUE(info.stopped);
    EXPECT_TRUE(info.quit);
}

TEST(UciSearchControl, NonControlCommandsQueueInOrderWithoutStopping) {
    Huginn::init();
    UCIInterface uci;
    Huginn::SearchInfo info;

    CaptureCout cap;
    EXPECT_TRUE(uci.handle_search_input_line("position startpos moves e2e4", info));
    EXPECT_TRUE(uci.handle_search_input_line("setoption name Hash value 32", info));
    EXPECT_TRUE(uci.handle_search_input_line("go movetime 100", info));

    EXPECT_FALSE(info.stopped)
        << "queued commands must not stop the search (#56: the old poll "
           "classified ANY pending line as a stop)";
    ASSERT_EQ(uci.pending().size(), 3u);
    EXPECT_EQ(uci.pending()[0], "position startpos moves e2e4");
    EXPECT_EQ(uci.pending()[1], "setoption name Hash value 32");
    EXPECT_EQ(uci.pending()[2], "go movetime 100");
}

TEST(UciSearchControl, PonderhitAndEmptyLinesIgnored) {
    Huginn::init();
    UCIInterface uci;
    Huginn::SearchInfo info;

    CaptureCout cap;
    EXPECT_TRUE(uci.handle_search_input_line("ponderhit", info));
    EXPECT_TRUE(uci.handle_search_input_line("", info));
    EXPECT_TRUE(uci.handle_search_input_line("   ", info));

    EXPECT_FALSE(info.stopped);
    EXPECT_TRUE(uci.pending().empty());
}

// --- dispatch_command: run()-loop semantics ----------------------------------

TEST(UciSearchControl, DispatchCommandQuitReturnsFalse) {
    Huginn::init();
    UCIInterface uci;

    CaptureCout cap;
    EXPECT_TRUE(uci.dispatch_command("isready"));
    EXPECT_TRUE(uci.dispatch_command("position startpos moves e2e4"));
    EXPECT_TRUE(uci.dispatch_command("stop"));  // stale stop: ignored, loop continues
    EXPECT_FALSE(uci.dispatch_command("quit"));
}

// --- go infinite: bestmove lifetime ------------------------------------------

TEST(UciSearchControl, InfiniteHoldsBestmoveUntilStopOnNaturalCompletion) {
    Huginn::init();
    UCIInterface uci;
    uci.handle_position(MATED_ROOT_POSITION);

    CaptureCout cap;
    // The mated-root search itself finishes in well under a millisecond; if
    // handle_go returns before signal_stop() fires, the hold is broken.
    std::atomic<long long> search_ms{-1};
    std::thread search([&uci, &search_ms]() {
        auto t0 = std::chrono::steady_clock::now();
        uci.handle_go({"go", "infinite"});
        search_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    uci.signal_stop();
    search.join();

    EXPECT_GE(search_ms.load(), 450)
        << "go infinite emitted bestmove before stop (search returned in "
        << search_ms.load() << "ms)";
    EXPECT_NE(cap.str().find("bestmove"), std::string::npos)
        << "stop must release exactly one bestmove";
}

TEST(UciSearchControl, StopReleasesInfiniteSearchPromptly) {
    Huginn::init();
    UCIInterface uci;
    uci.handle_position({"position", "startpos"});

    CaptureCout cap;
    std::atomic<long long> search_ms{-1};
    std::thread search([&uci, &search_ms]() {
        auto t0 = std::chrono::steady_clock::now();
        uci.handle_go({"go", "infinite"});
        search_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    uci.signal_stop();
    search.join();

    EXPECT_LT(search_ms.load(), 1500) << "stop did not release the infinite search promptly";
    EXPECT_NE(cap.str().find("bestmove"), std::string::npos);
}

// --- Cancellation channel: stress the atomic stop path ------------------------
//
// The #56 rework removed the cross-thread writes into the stack SearchInfo
// (running_info raw pointer); signal_stop() now only raises the engine's
// atomic, which checkup() translates on the searching thread. MSVC has no
// ThreadSanitizer, so beyond the by-construction argument this stress loop
// guards the observable contract: every stop lands promptly, every search
// produces exactly one bestmove.

TEST(UciSearchControl, RepeatedStopStressAlwaysReleasesWithOneBestmove) {
    Huginn::init();
    UCIInterface uci;
    uci.handle_position({"position", "startpos"});

    for (int i = 0; i < 12; ++i) {
        CaptureCout cap;
        std::thread search([&uci]() {
            uci.handle_go({"go", "movetime", "5000"});
        });
        // Vary the stop timing across iterations to catch different search phases.
        std::this_thread::sleep_for(std::chrono::milliseconds(5 + 10 * (i % 4)));
        uci.signal_stop();
        search.join();

        const std::string out = cap.str();
        const size_t first = out.find("bestmove");
        ASSERT_NE(first, std::string::npos) << "iteration " << i << ": no bestmove";
        EXPECT_EQ(out.find("bestmove", first + 1), std::string::npos)
            << "iteration " << i << ": more than one bestmove";
    }
}

// --- Startup silence + honest options -----------------------------------------

TEST(UciSearchControl, ConstructionEmitsNoStdout) {
    Huginn::init();
    CaptureCout cap;
    UCIInterface uci;
    EXPECT_TRUE(cap.str().empty())
        << "engine wrote to stdout before the GUI sent anything: " << cap.str();
}

TEST(UciSearchControl, OptionsAdvertiseSyzygyDisabledAndNoFalsePromises) {
    Huginn::init();
    UCIInterface uci;

    CaptureCout cap;
    uci.send_options();
    const std::string out = cap.str();

    EXPECT_NE(out.find("option name SyzygyPath type string default <empty>"), std::string::npos)
        << "SyzygyPath must default to disabled, not a hard-coded path";
    EXPECT_EQ(out.find("Threads"), std::string::npos);
    EXPECT_EQ(out.find("Ponder"), std::string::npos);
}

TEST(UciSearchControl, SyzygyEmptyPathMeansDisabled) {
    Huginn::SyzygyTablebase tb;
    EXPECT_FALSE(tb.initialize("")) << "empty path must mean disabled, not c:\\TB\\";
    EXPECT_FALSE(tb.is_available());
}
