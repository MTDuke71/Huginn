@echo off
REM Regression match: current Huginn vs huginn_t4.exe — the frozen Tier-2+
REM mid-stack baseline (commit 6e3a761 == git tag baseline-t4):
REM   huginn_t3 stack + Syzygy TB integration (#10) + contempt = 25 (#16)
REM   = +40 +/- 41 Elo over baseline-t3 (LOS 97.5%% / 200 games)
REM
REM Cumulative position over baseline-t3:
REM   #10 TB integration: +15.65 +/- 42 Elo / 200g, LOS 77%%
REM   #16 contempt 25 cp: +40.13 +/- 41 Elo / 200g, LOS 97.5%%
REM   Marginal #16 contribution: ~+24 Elo (LOS jump 77%% -> 97.5%%)
REM
REM Use this for every incremental search/eval change going forward,
REM replacing the now-superseded test_huginn_vs_t3.bat.
REM
REM Configures with -DENABLE_FATHOM=ON to keep Syzygy TB available;
REM the t4 baseline binary was built with TB so this stays apples-to-
REM apples for in-search probe behaviour.
REM
REM Concurrency 4: validated on multiple 200g gauntlets at ~25%% CPU
REM on the 13700K.
REM
REM Usage: test_huginn_vs_t4.bat [rounds]   (default: 100 rounds = 200 games)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=100

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Documents\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

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
if exist "%RESULTS%\huginn_vs_t4_intel.pgn" del /q "%RESULTS%\huginn_vs_t4_intel.pgn"
if exist "%RESULTS%\fastchess_t4_intel.log" del /q "%RESULTS%\fastchess_t4_intel.log"

echo.
echo [Intel 13700K] Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs huginn_t4 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t4.exe" name=Huginn_t4 option.OwnBook=false ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%RESULTS%\huginn_vs_t4_intel.pgn" notation=san append=false ^
  -log file="%RESULTS%\fastchess_t4_intel.log" level=warn

pause
