@echo off
REM Regression match: current Huginn vs huginn_t1.exe — the frozen Tier-1
REM partial baseline (commit 4f0ff0c == git tag baseline-t1):
REM   Phase 4 final + tempo + doubled-pawns + RFP + razoring
REM
REM Use this for every incremental search/eval change going forward.
REM Goal: measure per-feature Elo delta on top of the t1 baseline.
REM
REM Usage: test_huginn_vs_t1.bat [rounds]   (default: 50 rounds = 100 games)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=50

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

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
echo Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs huginn_t1 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t1.exe" name=Huginn_t1 option.OwnBook=false ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 1 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\huginn_vs_t1.pgn" notation=uci append=true ^
  -log file="%FC%\fastchess_t1.log" level=warn

pause
