@echo off
REM Regression match: current Huginn vs huginn_t5.exe — the frozen baseline
REM that includes every search/eval ship through BACKLOG #24 (real magic
REM bitboards):
REM
REM   baseline-t5 = 3eab266  (BACKLOG #24, 2026-05-16)
REM     = baseline-t4 stack
REM       + #1  P1a (LMR-exempt-check, +6 Elo pooled 600g)
REM       + #23 TT bound classification fix (+24 Elo pooled 400g)
REM       + #24 real magic-bitboard slider attacks (+52% NPS / +50 Elo single 200g)
REM
REM Cumulative t5 over t4: ~+75 Elo single Intel 200g, LOS 99.99%.
REM
REM Use this for every incremental search/eval change going forward,
REM superseding test_huginn_vs_t4.bat (kept around for historical
REM regression checks).
REM
REM Configures with -DENABLE_FATHOM=ON to keep Syzygy TB available;
REM the t5 baseline binary was built with TB so this stays apples-to-
REM apples for in-search probe behaviour.
REM
REM Concurrency 4: validated on multiple 200g gauntlets at ~25%% CPU
REM on the 13700K.
REM
REM Usage: test_huginn_vs_t5.bat [rounds]   (default: 100 rounds = 200 games)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=100

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Documents\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

if not exist "%FC%\huginn_t5.exe" (
    echo ERROR: %FC%\huginn_t5.exe not found.
    echo Build it once from commit 3eab266 ^(git tag baseline-t5^) and copy it into the fastchess folder.
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
REM for the combined 400g estimate (see gauntlet\README.md).
set RESULTS=%HUGINN_REPO%\gauntlet
if not exist "%RESULTS%" mkdir "%RESULTS%"

REM Each run is its own experiment: wipe this machine's prior PGN+log
REM so games never accumulate across runs (paired with append=false).
if exist "%RESULTS%\huginn_vs_t5_intel.pgn" del /q "%RESULTS%\huginn_vs_t5_intel.pgn"
if exist "%RESULTS%\fastchess_t5_intel.log" del /q "%RESULTS%\fastchess_t5_intel.log"

echo.
echo [Intel 13700K] Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs huginn_t5 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t5.exe" name=Huginn_t5 option.OwnBook=false ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%RESULTS%\huginn_vs_t5_intel.pgn" notation=san append=false ^
  -log file="%RESULTS%\fastchess_t5_intel.log" level=warn

pause
