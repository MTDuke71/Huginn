@echo off
setlocal
REM ===========================================================================
REM Fixed, repeatable SEARCH workload for profiling (attach a profiler to it,
REM e.g. Very Sleepy or AMD uProf -- or launch it under AMDuProfCLI).
REM
REM Runs Huginn on the Kiwipete middlegame for a fixed movetime, holding stdin
REM OPEN for the whole search. This matters: a closed pipe / EOF makes Huginn's
REM input-poll stop the search early (we verified ~8ms / depth-6 stops). The
REM trailing ping keeps the pipe's write end open until the search finishes.
REM
REM Usage:  profile_workload.bat [movetime_ms]      (default 40000 = 40s)
REM ===========================================================================
set "MT=%~1"
if "%MT%"=="" set "MT=40000"
set /a "SEC=%MT%/1000 + 6"

REM Prefer the symbol-bearing profiling build; fall back to the plain release.
set "EXE=%~dp0..\build\msvc-profile\bin\Release\huginn.exe"
if not exist "%EXE%" set "EXE=%~dp0..\build\msvc-x64-release\bin\Release\huginn.exe"
if not exist "%EXE%" ( echo Engine not found. Build build/msvc-profile first. & exit /b 1 )

set "FEN=r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

echo Engine : %EXE%
echo Search : Kiwipete, movetime %MT% ms  (stdin held open ~%SEC%s)
(
  echo setoption name OwnBook value false
  echo position fen %FEN%
  echo go movetime %MT%
  ping -n %SEC% 127.0.0.1 >nul
) | "%EXE%"
echo Done.
endlocal
