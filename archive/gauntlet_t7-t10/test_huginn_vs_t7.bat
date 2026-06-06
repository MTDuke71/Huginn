@echo off
REM ===========================================================================
REM Self-configuring two-machine regression gauntlet: current Huginn vs
REM huginn_t7.exe (frozen baseline-t7 = commit 304f2b7).
REM
REM ONE script for BOTH boxes -- it auto-detects the machine so there is no
REM longer a separate _amd variant:
REM   - Repo path comes from %~dp0 (the bat lives in the repo root), so the
REM     Intel box (...\Documents\Repos\Huginn) and the AMD box
REM     (...\Repos\Huginn) both just work with no hard-coded path.
REM   - CPU vendor (PROCESSOR_IDENTIFIER) picks the result suffix:
REM       GenuineIntel -> _intel,  AuthenticAMD -> _amd
REM     mirroring the CPU detection in perft\perf_test.ps1.
REM
REM Two-machine workflow for 2000 games per experiment:
REM   - Run this bat on EACH box -> 1000 games -> huginn_vs_t7_<machine>.pgn
REM   - Pool both PGNs (same engine names "Huginn_current" / "Huginn_t7",
REM     same frozen t7 baseline, same tc, same concurrency) for a combined
REM     Elo estimate. Pair pentanomials by [Round] tag, not file order
REM     (fastchess writes games in completion order under concurrency).
REM
REM baseline-t7 = t6 + #28 Part 1 (halfmove-clock repetition lookback)
REM   + #28 Part 2 (TT-safe Zarkov single-rep draw gated on winning eval).
REM   Pooled vs t6: W509 / L465 / D1026, score 51.10%%, +7.6 +/- ~10.5 Elo.
REM
REM Concurrency 4 on BOTH machines: identical per-game CPU contention keeps
REM the pooled sample homogeneous. Do NOT raise it on the box with spare
REM cores -- that perturbs tc=10+0.1 timing and makes the halves
REM non-comparable.
REM
REM Configures current Huginn with -DENABLE_FATHOM=ON so in-search Syzygy
REM probe behaviour matches the TB-enabled t7 baseline binary.
REM
REM Usage: test_huginn_vs_t7.bat [rounds_cap]
REM   default rounds_cap=500 (SPRT early-stop; cap is a safety limit)
REM ===========================================================================

setlocal

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=500

REM --- Repo root = this script's own folder (strip trailing backslash) ---
set "HUGINN_REPO=%~dp0"
if "%HUGINN_REPO:~-1%"=="\" set "HUGINN_REPO=%HUGINN_REPO:~0,-1%"

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

REM --- Detect the machine from the CPU vendor (mirrors perf_test.ps1) ---
set "MACHINE="
echo %PROCESSOR_IDENTIFIER% | find /i "GenuineIntel" >nul && set "MACHINE=intel"
echo %PROCESSOR_IDENTIFIER% | find /i "AuthenticAMD" >nul && set "MACHINE=amd"
if not defined MACHINE (
    echo ERROR: Could not classify CPU vendor from PROCESSOR_IDENTIFIER:
    echo   "%PROCESSOR_IDENTIFIER%"
    echo Edit this script to force MACHINE=intel or MACHINE=amd.
    pause
    exit /b 1
)

REM Friendly CPU name for the banner (same source as perf_test.ps1)
for /f "usebackq delims=" %%C in (`powershell -NoProfile -Command "(Get-CimInstance Win32_Processor ^| Select-Object -First 1).Name.Trim()"`) do set "CPU_NAME=%%C"
if not defined CPU_NAME set "CPU_NAME=%MACHINE%"

if not exist "%FC%\huginn_t7.exe" (
    echo ERROR: %FC%\huginn_t7.exe not found.
    echo Build it once from git tag baseline-t7 and copy it into the fastchess folder.
    pause
    exit /b 1
)

echo Configuring current Huginn with Fathom (Syzygy) enabled...
cd /d "%HUGINN_REPO%"
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
if exist "%RESULTS%\huginn_vs_t7_%MACHINE%.pgn" del /q "%RESULTS%\huginn_vs_t7_%MACHINE%.pgn"
if exist "%RESULTS%\fastchess_t7_%MACHINE%.log" del /q "%RESULTS%\fastchess_t7_%MACHINE%.log"

echo.
echo [%CPU_NAME%] Running SPRT (elo0=0, elo1=10, alpha=0.05, beta=0.05)
echo rounds cap: %ROUNDS% (= up to %ROUNDS%*2 games): Huginn current vs huginn_t7 tc=10+0.1
echo results -^> gauntlet\huginn_vs_t7_%MACHINE%.pgn
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
  -pgnout file="%RESULTS%\huginn_vs_t7_%MACHINE%.pgn" notation=san append=false ^
  -log file="%RESULTS%\fastchess_t7_%MACHINE%.log" level=warn

pause
endlocal
