@echo off
echo Detailed futility pruning analysis on WAC.001...
cd /d "%~dp0\..\build\bin\Release"

(
echo uci
echo setoption name OwnBook value false
echo position fen 2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1
echo go depth 6
echo quit
) | huginn.exe

echo.
echo Analysis complete. Check futility cut statistics above.