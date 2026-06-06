@echo off
REM ===========================================================================
REM Self-configuring two-machine regression gauntlet: current Huginn vs
REM huginn_t10.exe (frozen baseline-t10).
REM
REM baseline-t10 = baseline-t9 + tapered-eval foundation (BACKLOG #35, engine
REM   commit 476d33c). Replaces the hard is_endgame boolean (material<=1150
REM   flips king-PST + mobility weight at a discontinuity) with a smooth
REM   game_phase_256() blend; mg/eg sums diverge only on the king table, so the
REM   flag-off path is byte-identical to t9. No new tuned values. Pooled 1448g
REM   vs t9: ~+39.5 Elo (AMD +45.86 @602g / Intel +35.03 @846g, both LOS 100%,
REM   both SPRT H1-accept). t10 is the new regression baseline: future
REM   experiments measured vs t10 isolate the new change (e.g. #35 Experiment 2
REM   tapered material values, Experiment 3 king safety).
REM
REM ONE script for BOTH boxes -- auto-detects the machine:
REM   - Repo path from %~dp0 (the bat lives in the repo root): Intel
REM     (...\Documents\Repos\Huginn) and AMD (...\Repos\Huginn) both work.
REM   - CPU vendor (PROCESSOR_IDENTIFIER) picks the result suffix:
REM       GenuineIntel -> _intel,  AuthenticAMD -> _amd
REM     mirroring perft\perf_test.ps1.
REM
REM Two-machine workflow for 2000 games per experiment:
REM   - Run this bat on EACH box -> 1000 games -> huginn_vs_t10_<machine>.pgn
REM   - Pool both PGNs (same engine names, same frozen t10 baseline, same tc,
REM     same concurrency). Pair pentanomials by [Round] tag, not file order.
REM
REM Concurrency 4 on BOTH machines keeps the pooled sample homogeneous; do
REM NOT raise it on the box with spare cores (perturbs tc=10+0.1 timing).
REM
REM Configures current Huginn with -DENABLE_FATHOM=ON so in-search Syzygy
REM probe behaviour matches the TB-enabled t10 baseline binary.
REM
REM Usage: test_huginn_vs_t10.bat [rounds_cap]
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

if not exist "%FC%\huginn_t10.exe" (
    echo ERROR: %FC%\huginn_t10.exe not found.
    echo Build it once from git tag baseline-t10 and copy it into the fastchess folder.
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
if exist "%RESULTS%\huginn_vs_t10_%MACHINE%.pgn" del /q "%RESULTS%\huginn_vs_t10_%MACHINE%.pgn"
if exist "%RESULTS%\fastchess_t10_%MACHINE%.log" del /q "%RESULTS%\fastchess_t10_%MACHINE%.log"

echo.
echo [%CPU_NAME%] Running SPRT (elo0=0, elo1=10, alpha=0.05, beta=0.05)
echo rounds cap: %ROUNDS% (= up to %ROUNDS%*2 games): Huginn current vs huginn_t10 tc=10+0.1
echo results -^> gauntlet\huginn_vs_t10_%MACHINE%.pgn
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t10.exe" name=Huginn_t10 option.OwnBook=false ^
  -each tc=10+0.1 ^
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%RESULTS%\huginn_vs_t10_%MACHINE%.pgn" notation=san append=false ^
  -log file="%RESULTS%\fastchess_t10_%MACHINE%.log" level=warn

pause
endlocal
