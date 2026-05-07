@echo off
REM External calibration: current Huginn vs MTLChess v0.3 (MTLChess_v03_before.exe).
REM
REM MTLChess v0.3 implied rating ~1984 (transitive from MORA = 2191).
REM Use this to track absolute strength as Huginn improves. Last run
REM 2026-04-30 (post-mobility): 2W/17L/1D — Huginn at ~-340 Elo vs MTL v0.3.
REM Cadence: re-run after each shipped feature expected to be >= +10 Elo.
REM
REM Usage: test_huginn_vs_mtlchess_v03.bat [rounds]   (default: 50 rounds = 100 games)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=50

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Documents\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"
REM Use mtlchess003.exe (same binary as test_huginn_calibration.bat mtl03)
REM so this script's results are comparable to BACKLOG #5 calibration history.
set OPPONENT=mtlchess003.exe
set OPPONENT_NAME=MTLChess_v0.3

if not exist "%FC%\%OPPONENT%" (
    echo Missing: %FC%\%OPPONENT%
    exit /b 1
)

echo Building current Huginn (msvc-x64-release)...
cd /d %HUGINN_REPO%
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
echo Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs %OPPONENT_NAME% tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\%OPPONENT%" name=%OPPONENT_NAME% ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 1 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\huginn_vs_mtlchess_v03.pgn" notation=uci append=true ^
  -log file="%FC%\fastchess_mtlchess_v03.log" level=warn

pause
