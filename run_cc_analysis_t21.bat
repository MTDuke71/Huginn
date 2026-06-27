@echo off
REM ===========================================================================
REM #41 played-game re-diagnosis vs t21 (post #45 move-level-futility + #46/#47
REM TT-clear/time-management fixes -- the current engine).
REM Re-runs the Stockfish-vs-Huginn cp-loss / move-match / optimism study on the
REM SAME CC_Games.pgn, reusing the 2026-06-14 (t15) Stockfish ground truth by FEN
REM so only Huginn's moves + SF's verdict on them are recomputed (~halves runtime,
REM ~2h, and keeps the ground truth identical for a clean t15-vs-t21 compare).
REM
REM QUIET-BOX JOB: Huginn runs at a fixed 1.5s/move, so a loaded box changes its
REM moves. Do NOT run this alongside a gauntlet or daytime PC use. Resumable
REM (re-running continues from the CSV), so a stop/restart is safe.
REM
REM PREREQ: huginn_t21.exe in the fastchess dir (snapshotted by the t21 ship).
REM ===========================================================================
cd /d %~dp0

set T21=C:\Users\m_lad\Documents\fastchess-windows-x86-64\huginn_t21.exe
set BASE=tools\cc_analysis20260614.csv
set OUT=tools\cc_analysis_t21.csv

if not exist "%T21%" (
    echo Missing: %T21%
    echo   Ship/snapshot baseline-t21 first ^(huginn_t21.exe^), then re-run.
    exit /b 1
)
if not exist "%BASE%" (
    echo Missing baseline CSV: %BASE%
    exit /b 1
)

echo Running #41 re-diagnosis vs t21 (reusing t15 SF ground truth)...
python tools\analyze_played_vs_engines.py --hug "%T21%" --reuse-sf "%BASE%" --out "%OUT%"
if errorlevel 1 (
    echo analyze step failed.
    exit /b 1
)

echo.
echo === t15 vs t21 comparison ===
python tools\compare_cc.py "%BASE%" "%OUT%"
