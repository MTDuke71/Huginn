@echo off
REM ===========================================================================
REM AMD-machine variant of test_huginn_vs_t7.bat
REM
REM This machine: AMD Ryzen 7 7800X3D (8C/16T), repo at C:\Users\m_lad\Repos\Huginn
REM Other machine: Intel 13700K,                 repo at C:\Users\m_lad\Documents\Repos\Huginn
REM
REM Two-machine workflow for 2000 games per experiment:
REM   - Run THIS bat on the AMD box -> 1000 games -> huginn_vs_t7_amd.pgn
REM   - Run test_huginn_vs_t7.bat on Intel -> 1000 games -> huginn_vs_t7_intel.pgn
REM   - Pool both PGNs (same engine names "Huginn_current" / "Huginn_t7",
REM     same frozen t7 baseline binary, same tc, same concurrency) for a
REM     combined Elo estimate. Pair pentanomials by [Round] tag, not file
REM     order (fastchess writes games in completion order under concurrency).
REM
REM Regression target: huginn_t7.exe == git tag baseline-t7 (commit 304f2b7):
REM t6 + #28 Part 1 (halfmove-clock repetition lookback) + #28 Part 2
REM (TT-safe Zarkov single-rep draw gated on winning eval). Pooled vs t6:
REM W509 / L465 / D1026, score 51.10%, +7.6 +/- ~10.5 Elo.
REM
REM Concurrency 4 deliberately matches the Intel bat: identical per-game CPU
REM contention keeps the pooled sample homogeneous (do NOT raise it
REM just because this box has spare cores -- that would perturb tc=10+0.1
REM timing and make the two halves non-comparable).
REM
REM Configures current Huginn with -DENABLE_FATHOM=ON so in-search Syzygy
REM probe behaviour matches the TB-enabled t7 baseline binary.
REM
REM Usage: test_huginn_vs_t7_amd.bat [rounds_cap]
REM   default rounds_cap=500 (SPRT early-stop; cap is a safety limit)
REM ===========================================================================

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=500

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

if not exist "%FC%\huginn_t7.exe" (
    echo ERROR: %FC%\huginn_t7.exe not found.
    echo Build it once from git tag baseline-t7 and copy it into the fastchess folder.
    pause
    exit /b 1
)

echo Configuring current Huginn with Fathom (Syzygy) enabled...
cd /d %HUGINN_REPO%
%CMAKE% --preset msvc-x64-release -DENABLE_FATHOM=ON
if errorlevel 1 (
    echo Configure failed!
    pause
    exit /b 1
)

echo Building current Huginn (msvc-x64-release)...
%CMAKE% --build build/msvc-x64-release --config Release --target huginn
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

copy /Y build\msvc-x64-release\bin\Release\huginn.exe "%FC%\huginn.exe"
if not exist "%FC%\src" mkdir "%FC%\src"
copy /Y src\performance.bin "%FC%\src\performance.bin"

REM Results go INTO the repo (gauntlet\) so git shuttles them between
REM machines. Machine-tagged filenames -> no cross-machine git conflict.
set RESULTS=%HUGINN_REPO%\gauntlet
if not exist "%RESULTS%" mkdir "%RESULTS%"

REM Each run is its own experiment: wipe this machine's prior PGN+log
REM so games never accumulate across runs (paired with append=false).
if exist "%RESULTS%\huginn_vs_t7_amd.pgn" del /q "%RESULTS%\huginn_vs_t7_amd.pgn"
if exist "%RESULTS%\fastchess_t7_amd.log" del /q "%RESULTS%\fastchess_t7_amd.log"

echo.
echo [AMD 7800X3D] Running SPRT (elo0=0, elo1=10, alpha=0.05, beta=0.05)
echo rounds cap: %ROUNDS% (= up to %ROUNDS%*2 games): Huginn current vs huginn_t7 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t7.exe" name=Huginn_t7 option.OwnBook=false ^
  -each tc=10+0.1 ^
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%RESULTS%\huginn_vs_t7_amd.pgn" notation=san append=false ^
  -log file="%RESULTS%\fastchess_t7_amd.log" level=warn

pause
