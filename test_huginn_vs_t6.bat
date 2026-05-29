@echo off
REM Regression match: current Huginn vs huginn_t6.exe — the frozen baseline
REM that includes every search/eval ship through the winning-repetition
REM root-avoidance/PV-repetition guard change:
REM
REM   baseline-t6 = tag baseline-t6
REM     = baseline-t5 stack
REM       + full 120/mailbox cleanup into sq64-only representation
REM       + root-side winning repetition avoidance
REM       + PV reconstruction repetition guard
REM
REM t6 over t5 pooled 400g, two machines:
REM   W93 / L68 / D239, score 53.125%, ~+21.8 Elo
REM
REM Use this for every incremental search/eval change going forward,
REM superseding test_huginn_vs_t5.bat (kept around for historical
REM regression checks).
REM
REM Configures with -DENABLE_FATHOM=ON to keep Syzygy TB available;
REM the t6 baseline binary should be built with TB so this stays
REM apples-to-apples for in-search probe behaviour.
REM
REM Concurrency 4: validated on multiple 200g gauntlets at ~25%% CPU
REM on the 13700K.
REM
REM Usage: test_huginn_vs_t6.bat [rounds_cap]
REM   default rounds_cap=500 (SPRT early-stop; cap is a safety limit)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=500

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Documents\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

if not exist "%FC%\huginn_t6.exe" (
    echo ERROR: %FC%\huginn_t6.exe not found.
    echo Build it once from git tag baseline-t6 and copy it into the fastchess folder.
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
REM machines. _intel suffix pairs with the AMD box's _amd files; the
REM two never collide on a git merge (distinct paths). Pool both PGNs
REM for the combined 400-game estimate (see gauntlet\README.md).
set RESULTS=%HUGINN_REPO%\gauntlet
if not exist "%RESULTS%" mkdir "%RESULTS%"

REM Each run is its own experiment: wipe this machine's prior PGN+log
REM so games never accumulate across runs (paired with append=false).
if exist "%RESULTS%\huginn_vs_t6_intel.pgn" del /q "%RESULTS%\huginn_vs_t6_intel.pgn"
if exist "%RESULTS%\fastchess_t6_intel.log" del /q "%RESULTS%\fastchess_t6_intel.log"

echo.
echo [Intel 13700K] Running SPRT (elo0=0, elo1=10, alpha=0.05, beta=0.05)
echo rounds cap: %ROUNDS% (= up to %ROUNDS%*2 games): Huginn current vs huginn_t6 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t6.exe" name=Huginn_t6 option.OwnBook=false ^
  -each tc=10+0.1 ^
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%RESULTS%\huginn_vs_t6_intel.pgn" notation=san append=false ^
  -log file="%RESULTS%\fastchess_t6_intel.log" level=warn

pause
