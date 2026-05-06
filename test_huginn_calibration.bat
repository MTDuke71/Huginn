@echo off
REM External calibration: huginn (current) vs known-rated opponent.
REM
REM Implied ratings (transitive from MORA = 2191):
REM   MTLChess_v0.3 ~1984
REM   MORA          2191
REM   MTLChess_v0.5 ~2314
REM
REM Usage:  test_huginn_calibration.bat mtl03  [rounds]   (~1984)
REM         test_huginn_calibration.bat mora   [rounds]   (~2191)
REM         test_huginn_calibration.bat mtl05  [rounds]   (~2314)

set WHICH=%1
set ROUNDS=%2
if "%ROUNDS%"=="" set ROUNDS=50

set FC=C:\Users\m_lad\Documents\fastchess-windows-x86-64
set FASTCHESS=%FC%\fastchess.exe
set HUGINN_REPO=C:\Users\m_lad\Documents\Repos\Huginn
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"

if /I "%WHICH%"=="mtl03" set OPPONENT=mtlchess003.exe
if /I "%WHICH%"=="mtl03" set OPPONENT_NAME=MTLChess_v0.3
if /I "%WHICH%"=="mora"  set OPPONENT=MORA110.exe
if /I "%WHICH%"=="mora"  set OPPONENT_NAME=MORA
if /I "%WHICH%"=="mtl05" set OPPONENT=mtlchess_v05.exe
if /I "%WHICH%"=="mtl05" set OPPONENT_NAME=MTLChess_v0.5

if "%OPPONENT%"=="" (
    echo Usage: test_huginn_calibration.bat ^<mtl03^|mora^|mtl05^> [rounds]
    exit /b 1
)

if not exist "%FC%\%OPPONENT%" (
    echo Missing: %FC%\%OPPONENT%
    exit /b 1
)

echo Building current Huginn (msvc-x64-release)...
cd /d %HUGINN_REPO%
%CMAKE% --build build/msvc-x64-release --config Release --target huginn
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

copy /Y build\msvc-x64-release\bin\Release\huginn.exe "%FC%\huginn.exe"
if not exist "%FC%\src" mkdir "%FC%\src"
copy /Y src\performance.bin "%FC%\src\performance.bin"

echo.
echo Running %ROUNDS% rounds (=%ROUNDS%*2 games): Huginn current vs %OPPONENT_NAME% tc=10+0.1
echo.

"%FASTCHESS%" ^
  -engine cmd="%FC%\huginn.exe" name=Huginn_current option.OwnBook=false ^
  -engine cmd="%FC%\%OPPONENT%" name=%OPPONENT_NAME% ^
  -each tc=10+0.1 ^
  -rounds %ROUNDS% ^
  -repeat ^
  -concurrency 1 ^
  -recover ^
  -openings file="%FC%\noob_3moves.epd" format=epd order=random ^
  -pgnout file="%FC%\huginn_vs_%WHICH%.pgn" notation=uci append=true ^
  -log file="%FC%\fastchess_calib_%WHICH%.log" level=warn

pause
