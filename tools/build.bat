@echo off
setlocal enabledelayedexpansion

:: Huginn Cross-Platform Build Script for Windows
:: This script helps configure and build with different toolchains

echo ====================================
echo Huginn Cross-Platform Build Script
echo ====================================
echo.

:: Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    pause
    exit /b 1
)

:: Show available options
echo Available build configurations:
echo 1. MSVC Debug
echo 2. MSVC Release  
echo 3. GCC Debug (requires GCC in PATH)
echo 4. GCC Release (requires GCC in PATH)
echo 5. MinGW Debug (requires MinGW in PATH)
echo 6. MinGW Release (requires MinGW in PATH)
echo 7. Clean all build directories
echo 8. Exit
echo.

set /p choice="Enter your choice (1-8): "

if "%choice%"=="1" (
    set preset=msvc-x64-debug
    set action=build
) else if "%choice%"=="2" (
    set preset=msvc-x64-release
    set action=build
) else if "%choice%"=="3" (
    set preset=gcc-x64-debug
    set action=build
) else if "%choice%"=="4" (
    set preset=gcc-x64-release
    set action=build
) else if "%choice%"=="5" (
    set preset=mingw-x64-debug
    set action=build
) else if "%choice%"=="6" (
    set preset=mingw-x64-release
    set action=build
) else if "%choice%"=="7" (
    set action=clean
) else if "%choice%"=="8" (
    echo Exiting...
    exit /b 0
) else (
    echo Invalid choice. Exiting...
    pause
    exit /b 1
)

if "%action%"=="clean" (
    echo Cleaning all build directories...
    if exist "build" (
        rmdir /s /q build
        echo Build directories cleaned.
    ) else (
        echo No build directories found.
    )
    pause
    exit /b 0
)

echo.
echo Configuring with preset: %preset%
echo.

:: Configure
cmake --preset %preset%
if !errorlevel! neq 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

echo.
echo Building...
echo.

:: Build
cmake --build --preset %preset%
if !errorlevel! neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.

:: Ask if user wants to run tests
set /p run_tests="Do you want to run tests? (y/n): "
if /i "%run_tests%"=="y" (
    echo Running tests...
    ctest --preset %preset%
)

echo.
echo Done!
pause