@echo off
REM Regression match: current Huginn vs huginn_t2.exe — the frozen Tier-1
REM mid-stack baseline (commit 66685f3 == git tag baseline-t2):
REM   huginn_t1 stack + step (a) root PVS + SEE qsearch + LMR table
REM   = ~+52.5 Elo over baseline-t1 (LOS 95.68% / 100 games)
REM
REM Use this for every incremental search/eval change going forward.
REM
REM Usage: test_huginn_vs_t2.bat [rounds]   (default: 50 rounds = 100 games)

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
echo Running %ROUNDS% rounds (= %ROUNDS%*2 games): Huginn current vs huginn_t2 tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t2.exe" name=Huginn_t2 option.OwnBook=false ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 1 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\huginn_vs_t2.pgn" notation=uci append=true ^
  -log file="%FC%\fastchess_t2.log" level=warn

pause
