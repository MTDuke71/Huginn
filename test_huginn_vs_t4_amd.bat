@echo off
REM ===========================================================================
REM AMD-machine variant of test_huginn_vs_t4.bat
REM
REM This machine: AMD Ryzen 7 7800X3D (8C/16T), repo at C:\Users\m_lad\Repos\Huginn
REM Other machine: Intel 13700K,                 repo at C:\Users\m_lad\Documents\Repos\Huginn
REM
REM Two-machine workflow for 400 games per experiment:
REM   - Run THIS bat on the AMD box  -> 200 games -> huginn_vs_t4_amd.pgn
REM   - Run test_huginn_vs_t4.bat on the Intel box -> 200 games -> huginn_vs_t4.pgn
REM   - Pool both PGNs (same engine names "Huginn_current" / "Huginn_t4",
REM     same frozen t4 baseline binary, same tc, same concurrency) for a
REM     combined 400-game Elo/SPRT estimate.
REM
REM Regression target: huginn_t4.exe == commit 6e3a761 (contempt ship,
REM "git tag baseline-t4"): huginn_t3 stack + Syzygy TB (#10) + contempt=25
REM (#16) = +40 +/- 41 Elo over baseline-t3 (LOS 97.5%% / 200 games).
REM
REM Concurrency 4 deliberately matches the Intel bat: identical per-game CPU
REM contention keeps the pooled 400-game sample homogeneous (do NOT raise it
REM just because this box has spare cores -- that would perturb tc=10+0.1
REM timing and make the two halves non-comparable).
REM
REM Configures current Huginn with -DENABLE_FATHOM=ON so in-search Syzygy
REM probe behaviour matches the TB-enabled t4 baseline binary.
REM
REM Usage: test_huginn_vs_t4_amd.bat [rounds]   (default: 100 rounds = 200 games)
REM ===========================================================================

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=100

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

if not exist "%FC%\huginn_t4.exe" (
    echo ERROR: %FC%\huginn_t4.exe not found.
    echo Build it once from commit 6e3a761 and copy it into the fastchess folder.
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

echo.
echo [AMD 7800X3D] Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs huginn_t4 tc=10+0.1
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
  -pgnout file="%FC%\huginn_vs_t4_amd.pgn" notation=san append=true ^
  -log file="%FC%\fastchess_t4_amd.log" level=warn

pause
