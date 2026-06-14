@echo off
REM ===========================================================================
REM Analyze CC_Games.pgn: played move vs Stockfish 17.1 vs Huginn 2.1, every
REM position in the full variation tree. Writes tools\cc_analysis.csv.
REM
REM ~6,837 unique positions, SF depth 22, Huginn 1.5 s/move -> est ~4-4.5 hours.
REM RESUMABLE: if it stops/crashes, just run this .bat again and it continues
REM from where the CSV left off. To start completely over, delete
REM tools\cc_analysis.csv first.
REM
REM If the engine/PGN paths differ on this machine, edit the constants at the
REM top of tools\analyze_played_vs_engines.py.
REM ===========================================================================
setlocal
cd /d "%~dp0"

echo Huginn 2.1 vs Stockfish 17.1 analysis of CC_Games.pgn
echo   ~6837 positions, SF depth 22, Huginn 1.5s/move (est ~4-4.5 hours)
echo   Output: tools\cc_analysis.csv  (resumable)
echo   Started: %DATE% %TIME%
echo.

python tools\analyze_played_vs_engines.py --sf-depth 22 --hug-time 1.5 --out tools\cc_analysis.csv

echo.
echo Finished: %DATE% %TIME%
echo Results: tools\cc_analysis.csv
echo For the weakness summary, run:  python tools\summarize_cc.py
echo.
pause
