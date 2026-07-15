@echo off
REM ===========================================================================
REM BACKLOG #31: Hash-size sweep — three-engine LTC round-robin.
REM
REM   test_huginn_hash_sweep.bat [tN] [rounds]
REM
REM One identical baseline binary (huginn_tN.exe, default t34) entered three
REM times with option.Hash = 64 / 256 / 1024; fastchess plays the full round
REM robin at tc=60+0.6 (the only TC where the table fills — a blitz d14 search
REM reaches ~43 permille hashfull, so a blitz sweep measures nothing).
REM
REM Run on BOTH boxes (vendor auto-detected, artifacts machine-tagged):
REM architecture matters here — the AMD 7800X3D's 96MB L3 V-Cache holds a
REM meaningful fraction of a small TT, the Intel box's L3 does not, so the
REM optimal size may differ per machine. Per-machine binaries as always.
REM
REM [rounds]: default 100 -> 3 pairings x 100 x 2 = 600 games (~6-7h at cc=4).
REM Memory worst case at cc=4: 4 x (256+1024)MB = 5GB — fine on 32GB boxes.
REM No SPRT (pairwise-only tool); read the final Elo table.
REM ===========================================================================

setlocal

set BASE=%1
if "%BASE%"=="" set BASE=t34
set ROUNDS=%2
if "%ROUNDS%"=="" set ROUNDS=100

set "HUGINN_REPO=%~dp0"
if "%HUGINN_REPO:~-1%"=="\" set "HUGINN_REPO=%HUGINN_REPO:~0,-1%"

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set ENGINE=%FC%\huginn_%BASE%.exe

REM --- Detect the machine from the CPU vendor (mirrors test_huginn_gauntlet) -
set "MACHINE="
echo %PROCESSOR_IDENTIFIER% | find /i "GenuineIntel" >nul && set "MACHINE=intel"
echo %PROCESSOR_IDENTIFIER% | find /i "AuthenticAMD" >nul && set "MACHINE=amd"
if not defined MACHINE (
    echo ERROR: could not classify CPU vendor "%PROCESSOR_IDENTIFIER%".
    exit /b 1
)

if not exist "%ENGINE%" (
    echo Missing: %ENGINE%
    echo   Build it from git tag baseline-%BASE% and copy it in as huginn_%BASE%.exe.
    exit /b 1
)

set RESULTS=%HUGINN_REPO%\gauntlet
if not exist "%RESULTS%" mkdir "%RESULTS%"
set "PGN=%RESULTS%\hash_sweep_%BASE%_ltc_%MACHINE%.pgn"
set "LOG=%RESULTS%\fastchess_hash_sweep_%BASE%_ltc_%MACHINE%.log"
if exist "%PGN%" del /q "%PGN%"
if exist "%LOG%" del /q "%LOG%"

echo.
echo [%MACHINE%] #31 hash sweep: %BASE% @ Hash 64/256/1024, tc=60+0.6, rounds=%ROUNDS% (games = 6x rounds), cc=4
echo   results -^> %PGN%
echo.

"%FASTCHESS%" ^
  -engine cmd="%ENGINE%" name=Huginn_h64   option.OwnBook=false option.SyzygyPath=c:\TB\ option.Hash=64 ^
  -engine cmd="%ENGINE%" name=Huginn_h256  option.OwnBook=false option.SyzygyPath=c:\TB\ option.Hash=256 ^
  -engine cmd="%ENGINE%" name=Huginn_h1024 option.OwnBook=false option.SyzygyPath=c:\TB\ option.Hash=1024 ^
  -each tc=60+0.6 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 4 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%PGN%" notation=san append=false ^
  -log file="%LOG%" level=warn

pause
endlocal
