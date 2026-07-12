// BACKLOG #56 (part 2): subprocess transcript tests.
//
// These drive the real huginn binary over real pipes — the exact transport a
// GUI uses — so they exercise the PeekNamedPipe input poll, the mid-search
// command pump, and the bestmove lifetimes end-to-end, which the in-process
// tests (test_uci_search_control.cpp) cannot. Windows-only: the pipe harness
// is Win32, and Huginn's development platform is MSVC/Windows.

#include "gtest/gtest.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

namespace {

using Clock = std::chrono::steady_clock;

long long ms_since(Clock::time_point t0) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count();
}

std::string engine_exe_path() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string p(buf, n);
    size_t slash = p.find_last_of("\\/");
    return p.substr(0, slash + 1) + "huginn.exe";  // built into the same bin dir
}

// Minimal two-pipe child-process harness for UCI transcripts.
class EngineProc {
public:
    ~EngineProc() { kill(); }

    bool start() {
        SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        HANDLE out_w = nullptr, in_r = nullptr;
        if (!CreatePipe(&stdout_r, &out_w, &sa, 0)) return false;
        SetHandleInformation(stdout_r, HANDLE_FLAG_INHERIT, 0);
        if (!CreatePipe(&in_r, &stdin_w, &sa, 0)) return false;
        SetHandleInformation(stdin_w, HANDLE_FLAG_INHERIT, 0);

        // stderr -> NUL: diagnostics are allowed there at any time and must
        // not pollute the stdout transcript the assertions read.
        HANDLE nul = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE, &sa,
                                 OPEN_EXISTING, 0, nullptr);

        STARTUPINFOA si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = in_r;
        si.hStdOutput = out_w;
        si.hStdError = nul;

        std::string cmd = engine_exe_path();
        PROCESS_INFORMATION pi{};
        BOOL ok = CreateProcessA(nullptr, &cmd[0], nullptr, nullptr, TRUE,
                                 CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
        CloseHandle(in_r);
        CloseHandle(out_w);
        if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
        if (!ok) return false;
        process = pi.hProcess;
        CloseHandle(pi.hThread);
        return true;
    }

    void send(const std::string& line) {
        std::string data = line + "\n";
        DWORD written = 0;
        WriteFile(stdin_w, data.data(), static_cast<DWORD>(data.size()), &written, nullptr);
    }

    // Pull whatever the engine has written so far into the transcript.
    void drain() {
        DWORD avail = 0;
        while (PeekNamedPipe(stdout_r, nullptr, 0, nullptr, &avail, nullptr) && avail > 0) {
            char buf[4096];
            DWORD got = 0;
            if (!ReadFile(stdout_r, buf, std::min<DWORD>(avail, sizeof(buf)), &got, nullptr) || got == 0)
                break;
            transcript_.append(buf, got);
        }
    }

    bool wait_for(const std::string& token, int timeout_ms) {
        auto t0 = Clock::now();
        while (ms_since(t0) < timeout_ms) {
            drain();
            if (transcript_.find(token) != std::string::npos) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        drain();
        return transcript_.find(token) != std::string::npos;
    }

    // True if @p token did NOT appear within the window.
    bool absent_for(const std::string& token, int window_ms) {
        auto t0 = Clock::now();
        while (ms_since(t0) < window_ms) {
            drain();
            if (transcript_.find(token) != std::string::npos) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return true;
    }

    int count(const std::string& token) {
        drain();
        int n = 0;
        for (size_t pos = transcript_.find(token); pos != std::string::npos;
             pos = transcript_.find(token, pos + token.size()))
            ++n;
        return n;
    }

    bool wait_exit(int timeout_ms) {
        return WaitForSingleObject(process, timeout_ms) == WAIT_OBJECT_0;
    }

    void kill() {
        if (process) {
            if (WaitForSingleObject(process, 0) != WAIT_OBJECT_0) TerminateProcess(process, 1);
            CloseHandle(process);
            process = nullptr;
        }
        if (stdin_w)  { CloseHandle(stdin_w);  stdin_w = nullptr; }
        if (stdout_r) { CloseHandle(stdout_r); stdout_r = nullptr; }
    }

    const std::string& transcript() { drain(); return transcript_; }

private:
    HANDLE process = nullptr;
    HANDLE stdin_w = nullptr;
    HANDLE stdout_r = nullptr;
    std::string transcript_;
};

// Start the engine or skip (partial builds where only huginn_tests exists).
#define START_ENGINE_OR_SKIP(eng)                                                   \
    do {                                                                            \
        if (GetFileAttributesA(engine_exe_path().c_str()) == INVALID_FILE_ATTRIBUTES) \
            GTEST_SKIP() << "huginn.exe not built next to huginn_tests: "           \
                         << engine_exe_path();                                      \
        ASSERT_TRUE((eng).start()) << "failed to launch " << engine_exe_path();     \
    } while (0)

} // namespace

TEST(UciTranscript, SilentUntilUciThenHonestOptions) {
    EngineProc eng;
    START_ENGINE_OR_SKIP(eng);

    // #56: nothing may appear on stdout before the GUI speaks (the old build
    // auto-probed c:\TB\ and printed a raw status line at construction).
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    eng.drain();
    EXPECT_TRUE(eng.transcript().empty())
        << "unsolicited startup stdout:\n" << eng.transcript();

    eng.send("uci");
    ASSERT_TRUE(eng.wait_for("uciok", 3000)) << eng.transcript();
    const std::string& t = eng.transcript();
    EXPECT_NE(t.find("id name Huginn"), std::string::npos);
    EXPECT_NE(t.find("option name SyzygyPath type string default <empty>"), std::string::npos);
    EXPECT_EQ(t.find("option name Threads"), std::string::npos);
    EXPECT_EQ(t.find("option name Ponder"), std::string::npos);

    eng.send("quit");
    EXPECT_TRUE(eng.wait_exit(2000));
}

TEST(UciTranscript, IsreadyDuringSearchAnswersWithoutKillingIt) {
    EngineProc eng;
    START_ENGINE_OR_SKIP(eng);
    eng.send("uci");
    ASSERT_TRUE(eng.wait_for("uciok", 3000));

    eng.send("position startpos");
    auto go_sent = Clock::now();
    eng.send("go movetime 2000");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // The ONLY isready of this session, sent mid-search.
    eng.send("isready");
    ASSERT_TRUE(eng.wait_for("readyok", 1000))
        << "isready during search not answered:\n" << eng.transcript();
    EXPECT_EQ(eng.transcript().find("bestmove"), std::string::npos)
        << "isready killed the search (#56: old poll treated any input as stop)";

    ASSERT_TRUE(eng.wait_for("bestmove", 4000)) << eng.transcript();
    long long search_ms = ms_since(go_sent);
    EXPECT_GE(search_ms, 800)
        << "bestmove after " << search_ms << "ms — movetime 2000 search died early";

    const std::string& t = eng.transcript();
    EXPECT_LT(t.find("readyok"), t.find("bestmove")) << "readyok must precede bestmove";

    eng.send("quit");
    EXPECT_TRUE(eng.wait_exit(2000));
}

TEST(UciTranscript, BatchedStopPositionGoExecutesInOrder) {
    EngineProc eng;
    START_ENGINE_OR_SKIP(eng);
    eng.send("uci");
    ASSERT_TRUE(eng.wait_for("uciok", 3000));

    eng.send("position startpos");
    eng.send("go movetime 5000");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // ChessBase-Kibitzer-style batch: all three lines land in the pipe at once.
    auto stop_sent = Clock::now();
    eng.send("stop\nposition startpos moves e2e4\ngo movetime 300");

    ASSERT_TRUE(eng.wait_for("bestmove", 1500))
        << "stop did not produce a prompt bestmove:\n" << eng.transcript();
    long long stop_ms = ms_since(stop_sent);
    EXPECT_LT(stop_ms, 1200) << "first bestmove took " << stop_ms << "ms after stop";

    // The queued position + go must then run: a second bestmove appears.
    auto t0 = Clock::now();
    while (eng.count("bestmove") < 2 && ms_since(t0) < 3000)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(eng.count("bestmove"), 2)
        << "queued position/go after stop were lost:\n" << eng.transcript();

    eng.send("quit");
    EXPECT_TRUE(eng.wait_exit(2000));
}

TEST(UciTranscript, GoInfiniteHoldsBestmoveUntilStop) {
    EngineProc eng;
    START_ENGINE_OR_SKIP(eng);
    eng.send("uci");
    ASSERT_TRUE(eng.wait_for("uciok", 3000));

    // Mated root: the search itself completes instantly, so only the #56
    // hold-until-stop keeps bestmove back. (Old build: max_depth=25 default
    // meant even normal infinite searches could emit bestmove on their own.)
    eng.send("position fen R5k1/5ppp/8/8/8/8/8/K7 b - - 0 1");
    eng.send("go infinite");
    EXPECT_TRUE(eng.absent_for("bestmove", 600))
        << "go infinite emitted bestmove without stop:\n" << eng.transcript();

    // Still responsive while parked.
    eng.send("isready");
    ASSERT_TRUE(eng.wait_for("readyok", 1000)) << eng.transcript();
    EXPECT_EQ(eng.transcript().find("bestmove"), std::string::npos);

    eng.send("stop");
    ASSERT_TRUE(eng.wait_for("bestmove", 1500))
        << "stop did not release the held bestmove:\n" << eng.transcript();

    eng.send("quit");
    EXPECT_TRUE(eng.wait_exit(2000));
}

TEST(UciTranscript, QuitDuringSearchExitsPromptly) {
    EngineProc eng;
    START_ENGINE_OR_SKIP(eng);
    eng.send("uci");
    ASSERT_TRUE(eng.wait_for("uciok", 3000));

    eng.send("position startpos");
    eng.send("go movetime 10000");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    eng.send("quit");
    EXPECT_TRUE(eng.wait_exit(3000))
        << "quit during search did not terminate the process";
}

#else  // !_WIN32

TEST(UciTranscript, SkippedOnNonWindows) {
    GTEST_SKIP() << "Win32 pipe harness; transcript tests run on Windows only";
}

#endif
