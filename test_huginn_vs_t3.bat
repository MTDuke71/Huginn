@echo off
REM Regression match: current Huginn vs huginn_t3.exe — the frozen Tier-2
REM mid-stack baseline (commit 2e97066 == git tag baseline-t3):
REM   huginn_t2 stack + ply-tracked TT-mate adjustment (#13 closure, 2c variant)
REM   = +104 +/- 62 Elo over baseline-t2 (LOS 99.98% / 100 games)
REM
REM Use this for every incremental search/eval change going forward, replacing
REM the now-superseded test_huginn_vs_t2.bat.
REM
REM Current Huginn is built with -DENABLE_FATHOM=ON (Syzygy 5-piece TB at
REM c:\TB\, BACKLOG #10 closure d79900a). The t3 baseline binary was built
REM before the TB integration so it has no TB — the matchup measures both
REM cumulative search progress AND the TB contribution.
REM
REM Concurrency 2: validated on 2026-05-07 t-chain round-robin
REM (200g per engine) with no instability and tighter per-game wall clock.
REM
REM Usage: test_huginn_vs_t3.bat [rounds]   (default: 50 rounds = 100 games)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=50

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

echo.
echo Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs huginn_t3 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t3.exe" name=Huginn_t3 option.OwnBook=false ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 2 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\huginn_vs_t3.pgn" notation=uci append=true ^
  -log file="%FC%\fastchess_t3.log" level=warn

pause
