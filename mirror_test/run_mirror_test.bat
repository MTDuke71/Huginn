@echo off
REM Batch file to run mirror evaluation test
REM This bypasses PowerShell execution policy issues

echo Running Huginn Mirror Evaluation Test...
echo.

powershell.exe -ExecutionPolicy Bypass -File "%~dp0run_mirror_test.ps1" %*

echo.
pause
