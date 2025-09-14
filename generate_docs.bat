@echo off
REM Generate Huginn Chess Engine Documentation
REM This script regenerates the Doxygen documentation for the Huginn project

echo Generating Huginn Chess Engine Documentation...
echo ================================================

REM Ensure Doxygen is available
set DOXYGEN_PATH="C:\Program Files\doxygen\bin\doxygen.exe"
if not exist %DOXYGEN_PATH% (
    where doxygen >nul 2>&1
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Doxygen not found in PATH or standard location
        echo Please install Doxygen or add it to your PATH
        echo Download from: https://www.doxygen.nl/download.html
        pause
        exit /b 1
    ) else (
        set DOXYGEN_PATH=doxygen
    )
)

REM Clean previous documentation
if exist docs\html (
    echo Cleaning previous documentation...
    rmdir /s /q docs\html
)

REM Generate documentation
echo Running Doxygen...
%DOXYGEN_PATH% Doxyfile.huginn

if %ERRORLEVEL% equ 0 (
    echo ================================================
    echo Documentation generated successfully!
    echo Location: docs\html\index.html
    echo ================================================
    
    REM Ask if user wants to open the documentation
    set /p OPEN_DOCS="Open documentation in browser? (y/n): "
    if /i "%OPEN_DOCS%"=="y" (
        start docs\html\index.html
    )
) else (
    echo ERROR: Documentation generation failed
    pause
    exit /b 1
)

pause