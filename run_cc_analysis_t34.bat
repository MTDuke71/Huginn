@echo off
REM ===========================================================================
REM #41 played-game re-diagnosis vs t34 (road-to-v2.3 hygiene: how far has the
REM t21->t34 selectivity/soundness stack moved the needle since the v2.2
REM baseline?).
REM Re-runs the Stockfish-vs-Huginn cp-loss / move-match / optimism study on
REM the SAME CC_Games.pgn, reusing the t21 Stockfish ground truth by FEN (which
REM itself carried forward the 2026-06-14 t15 ground truth unchanged) so only
REM Huginn's moves + SF's verdict on them are recomputed (~halves runtime vs a
REM cold run, ~2h, and keeps the ground truth identical across the t15/t21/t34
REM lineage for a clean compare).
REM
REM QUIET-BOX JOB: Huginn runs at a fixed 1.5s/move, so a loaded box changes its
REM moves. Do NOT run this alongside a gauntlet or daytime PC use. Resumable
REM (re-running continues from the CSV), so a stop/restart is safe.
REM
REM PREREQ: huginn_t34.exe in the fastchess dir (snapshotted by the t34 ship).
REM ===========================================================================
cd /d %~dp0

set T34=C:\Users\m_lad\Documents\fastchess-windows-x86-64\huginn_t34.exe
set BASE=tools\cc_analysis_t21.csv
set OUT=tools\cc_analysis_t34.csv

if not exist "%T34%" (
    echo Missing: %T34%
    echo   Ship/snapshot baseline-t34 first ^(huginn_t34.exe^), then re-run.
    exit /b 1
)
if not exist "%BASE%" (
    echo Missing baseline CSV: %BASE%
    exit /b 1
)

echo Running #41 re-diagnosis vs t34 (reusing t21/t15 SF ground truth)...
python tools\analyze_played_vs_engines.py --hug "%T34%" --reuse-sf "%BASE%" --out "%OUT%"
if errorlevel 1 (
    echo analyze step failed.
    exit /b 1
)

echo.
echo === t21 (v2.2 baseline) vs t34 comparison ===
python tools\compare_cc.py "%BASE%" "%OUT%"
