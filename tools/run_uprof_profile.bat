@echo off
setlocal
REM ===========================================================================
REM One-shot AMD uProf time-based profile of a Huginn search -> CSV report.
REM Collects with time-based sampling (the self-time/hotspot view) while running
REM the fixed Kiwipete workload, then generates a CSV report INCLUDING inlined
REM functions (--inline; off by default, and our build is LTCG-inlined).
REM
REM Usage:  run_uprof_profile.bat [movetime_ms]      (default 40000)
REM
REM NOTE: adjust CLI / flags to your uProf version. If huginn.exe samples are
REM missing (uProf profiled the launching cmd, not the child), use ATTACH instead
REM (more reliable, and how the Very Sleepy run was done):
REM     1) run  tools\profile_workload.bat 60000   in one window
REM     2) in uProf, attach CPU profiling to huginn.exe while it searches
REM ===========================================================================
set "CLI=C:\Program Files\AMD\AMDuProf\bin\AMDuProfCLI.exe"
set "OUT=%~dp0..\benchmark\uprof_session"
set "WORKLOAD=%~dp0profile_workload.bat"
set "MT=%~1"
if "%MT%"=="" set "MT=40000"

if not exist "%CLI%" (
  echo AMDuProfCLI not found at "%CLI%".
  echo   Edit CLI in this script to your uProf install bin folder.
  exit /b 1
)

echo === Collect (time-based sampling, ~%MT%ms search) ===
"%CLI%" collect --config tbp -o "%OUT%" -- cmd /c ""%WORKLOAD%" %MT%"
if errorlevel 1 ( echo collect step failed -- try ATTACH mode (see header). & exit /b 1 )

echo === Report (CSV, inlined functions included) ===
"%CLI%" report --input-dir "%OUT%" --inline

echo.
echo CSV report is under: "%OUT%"
echo Copy the function-hotspot CSV into benchmark\ to keep/diff it across versions.
endlocal
