@echo off
REM ===========================================================================
REM Unified Huginn gauntlet runner — replaces the per-baseline clones
REM (test_huginn_vs_t9/t10/...) AND test_huginn_calibration.bat.
REM
REM   test_huginn_gauntlet.bat <opponent> [rounds] [concurrency]
REM
REM <opponent>:
REM   tN  (t9, t10, t11, ...) -> internal baseline regression vs huginn_tN.exe
REM                              SPRT [0,10], OwnBook off both sides, concurrency
REM                              4, results -> gauntlet\huginn_vs_tN_<machine>.pgn.
REM                              Any tN works automatically — no new script ever.
REM   mtl03 | mora | mtl05    -> external calibration vs a known-rated engine
REM                              (~1984 / ~2191 / ~2314). No SPRT, concurrency 1.
REM
REM [rounds]: cap (default 500 for baselines = SPRT early-stops; 50 for calib).
REM           Pass a small number (e.g. 20) for a quick eyeball.
REM [concurrency]: games in parallel (default 4 baseline / 1 calib). KEEP 4 on
REM           BOTH boxes for poolable two-machine baseline runs — mixing it
REM           perturbs tc=10+0.1 timing. Override only for calib / local eyeballs.
REM
REM Two-machine workflow (baselines): run on EACH box; auto-detects the CPU
REM vendor and tags the PGN (_intel / _amd) so the two pool without conflict.
REM Concurrency 4 on BOTH boxes keeps the pooled sample homogeneous.
REM ===========================================================================

setlocal

set OPP=%1
set ROUNDS=%2
set CC_ARG=%3
if "%OPP%"=="" (
    echo usage: test_huginn_gauntlet.bat ^<tN ^| mtl03 ^| mora ^| mtl05^> [rounds] [concurrency]
    exit /b 1
)

REM --- Repo root = this script's own folder (works on both boxes) ---
set "HUGINN_REPO=%~dp0"
if "%HUGINN_REPO:~-1%"=="\" set "HUGINN_REPO=%HUGINN_REPO:~0,-1%"

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

REM --- Detect the machine from the CPU vendor (mirrors perf_test.ps1) ---
set "MACHINE="
echo %PROCESSOR_IDENTIFIER% | find /i "GenuineIntel" >nul && set "MACHINE=intel"
echo %PROCESSOR_IDENTIFIER% | find /i "AuthenticAMD" >nul && set "MACHINE=amd"
if not defined MACHINE (
    echo ERROR: could not classify CPU vendor "%PROCESSOR_IDENTIFIER%".
    exit /b 1
)

REM --- Resolve opponent + mode ------------------------------------------------
REM Internal baseline?  tN where N is one or more digits.
set "MODE="
echo %OPP%| findstr /R "^t[0-9][0-9]*$" >nul && set "MODE=baseline"
if not defined MODE set "MODE=calib"

set "OPPONENT="
set "OPP_NAME="
set "OPP_OPT="

if "%MODE%"=="baseline" set OPPONENT=huginn_%OPP%.exe
if "%MODE%"=="baseline" set OPP_NAME=Huginn_%OPP%
if "%MODE%"=="baseline" set OPP_OPT=option.OwnBook=false

REM External calibration table (extend here when adding a rated opponent).
if /I "%OPP%"=="mtl03" set OPPONENT=mtlchessV3.exe
if /I "%OPP%"=="mtl03" set OPP_NAME=MTLChess_v0.3
if /I "%OPP%"=="mora"  set OPPONENT=MORA110.exe
if /I "%OPP%"=="mora"  set OPP_NAME=MORA
if /I "%OPP%"=="mtl05" set OPPONENT=mtlchess_v05.exe
if /I "%OPP%"=="mtl05" set OPP_NAME=MTLChess_v0.5

if "%OPPONENT%"=="" (
    echo Unknown opponent "%OPP%". Use tN, or add it to the calibration table.
    exit /b 1
)
if not exist "%FC%\%OPPONENT%" (
    echo Missing: %FC%\%OPPONENT%
    if "%MODE%"=="baseline" echo   Build it once from git tag baseline-%OPP% and copy it in as %OPPONENT%.
    exit /b 1
)

REM --- Per-mode defaults ------------------------------------------------------
if "%ROUNDS%"=="" if "%MODE%"=="baseline" set ROUNDS=500
if "%ROUNDS%"=="" set ROUNDS=50

set "SPRT="
set CC=1
if "%MODE%"=="baseline" set SPRT=-sprt elo0=0 elo1=10 alpha=0.05 beta=0.05
if "%MODE%"=="baseline" set CC=4
REM Optional concurrency override (arg 3). WARNING: for poolable two-machine
REM baseline runs keep cc=4 on BOTH boxes — mixing concurrency perturbs
REM tc=10+0.1 timing and breaks pentanomial pooling. Safe to vary for
REM calibration or a quick local eyeball.
if not "%CC_ARG%"=="" set CC=%CC_ARG%

REM --- Build current Huginn (Fathom on, to match TB-enabled baselines) --------
echo Configuring + building current Huginn (msvc-x64-release, Fathom ON)...
cd /d "%HUGINN_REPO%"
%CMAKE% --preset msvc-x64-release -DENABLE_FATHOM=ON
if errorlevel 1 ( echo Configure failed! & exit /b 1 )
%CMAKE% --build build/msvc-x64-release --config Release --target huginn
if errorlevel 1 ( echo Build failed! & exit /b 1 )

copy /Y build\msvc-x64-release\bin\Release\huginn.exe "%FC%\huginn.exe"
if not exist "%FC%\src" mkdir "%FC%\src"
copy /Y src\performance.bin "%FC%\src\performance.bin"

REM --- Results into the repo (gauntlet\), machine-tagged ----------------------
set RESULTS=%HUGINN_REPO%\gauntlet
if not exist "%RESULTS%" mkdir "%RESULTS%"
set "PGN=%RESULTS%\huginn_vs_%OPP%_%MACHINE%.pgn"
set "LOG=%RESULTS%\fastchess_%OPP%_%MACHINE%.log"
if exist "%PGN%" del /q "%PGN%"
if exist "%LOG%" del /q "%LOG%"

echo.
echo [%MACHINE%] %MODE%: Huginn_current vs %OPP_NAME%  tc=10+0.1  rounds-cap=%ROUNDS%  cc=%CC%
if defined SPRT echo   SPRT elo0=0 elo1=10 alpha=0.05 beta=0.05
echo   results -^> %PGN%
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\%OPPONENT%" name=%OPP_NAME% %OPP_OPT% ^
  -each tc=10+0.1 ^
  %SPRT% ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency %CC% ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%PGN%" notation=san append=false ^
  -log file="%LOG%" level=warn

pause
endlocal
