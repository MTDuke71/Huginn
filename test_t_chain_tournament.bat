@echo off
REM Three-way round-robin: huginn_t1 vs huginn_t2 vs huginn_t3.
REM
REM All three baselines are frozen pre-built binaries in the fastchess
REM folder. No build step — this script just runs the tournament.
REM
REM Output: 3 pairings (t1<->t2, t1<->t3, t2<->t3) x %ROUNDS%*2 games
REM each = total %ROUNDS%*6 games. Default 50 rounds = 300 games.
REM
REM Sanity-check the chain:
REM   t2 should beat t1 by ~+52 Elo (per baseline-t2 closing run)
REM   t3 should beat t2 by ~+104 Elo (per baseline-t3 closing run / #13)
REM   t3 should beat t1 by ~+156 Elo (additive prediction)
REM Significant deviation from the additive prediction would suggest
REM non-transitive interaction worth investigating.
REM
REM Usage: test_t_chain_tournament.bat [rounds]   (default: 50)

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=50

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe

if not exist "%FC%\huginn_t1.exe" (
    echo Missing: %FC%\huginn_t1.exe
    exit /b 1
)
if not exist "%FC%\huginn_t2.exe" (
    echo Missing: %FC%\huginn_t2.exe
    exit /b 1
)
if not exist "%FC%\huginn_t3.exe" (
    echo Missing: %FC%\huginn_t3.exe
    exit /b 1
)

echo.
echo Running round-robin: t1 vs t2 vs t3 at tc=10+0.1, concurrency 2
echo Rounds: %ROUNDS% (= %ROUNDS%*6 = total games)
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn_t1.exe" name=Huginn_t1 option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t2.exe" name=Huginn_t2 option.OwnBook=false ^
  -engine cmd="%FC%\huginn_t3.exe" name=Huginn_t3 option.OwnBook=false ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 2 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\t_chain_tournament.pgn" notation=uci append=true ^
  -log file="%FC%\fastchess_t_chain.log" level=warn ^
  -tournament roundrobin

pause
