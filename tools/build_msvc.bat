@echo off
:: Huginn Build Script for Windows with MSVC
:: Ensures clean environment and proper toolchain usage

setlocal enabledelayedexpansion

echo === Huginn MSVC Build Script ===

:: Check if we're already in a VS Developer environment
if not defined VCINSTALLDIR (
    echo.
    echo WARNING: Not in Visual Studio Developer Command Prompt
    echo For best results, run this from "Developer Command Prompt for VS 2022"
    echo.
    pause
    goto :eof
)

:: Clean MSYS2 paths from environment to prevent header conflicts
echo Cleaning environment of MSYS2 paths...
set "NEW_PATH="
for %%i in ("%PATH:;=" "%") do (
    set "CURRENT_PATH=%%~i"
    echo !CURRENT_PATH! | findstr /i "msys64" >nul
    if errorlevel 1 (
        if defined NEW_PATH (
            set "NEW_PATH=!NEW_PATH!;!CURRENT_PATH!"
        ) else (
            set "NEW_PATH=!CURRENT_PATH!"
        )
    )
)
set "PATH=%NEW_PATH%"

:: Clear other potentially conflicting environment variables
set PKG_CONFIG_PATH=
set CMAKE_PREFIX_PATH=
set MSYSTEM=
set MSYS2_PATH_TYPE=

:: Display configuration
echo.
echo Build Configuration:
echo - Compiler: %CC% (MSVC)
echo - Build Type: Debug
echo - Generator: Ninja
echo - Target: Huginn Chess Engine
echo.

:: Check if CMake and Ninja are available
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found in PATH
    exit /b 1
)

ninja --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Ninja not found in PATH
    echo Please install Ninja or use Visual Studio generator
    exit /b 1
)

:: Create build directory
if not exist "build\msvc-debug" (
    mkdir "build\msvc-debug"
)

:: Configure with CMake
echo Configuring with CMake...
pushd "build\msvc-debug"

cmake ..\.. ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_C_COMPILER=cl ^
    -DCMAKE_CXX_COMPILER=cl

if errorlevel 1 (
    echo ERROR: CMake configuration failed
    popd
    exit /b 1
)

:: Build
echo.
echo Building...
ninja

if errorlevel 1 (
    echo ERROR: Build failed
    popd
    exit /b 1
)

popd

echo.
echo === Build completed successfully! ===
echo.
echo Executables are in: build\msvc-debug\bin\
echo.
echo Main targets:
echo   - huginn.exe          : Huginn 1.1 (main engine)
echo   - huginn_legacy.exe   : Original Huginn engine  
echo   - huginn_uci.exe      : UCI interface
echo   - huginn_demo.exe     : Position display demo
echo   - huginn_tests.exe    : Test suite (if GTest found)
echo.
echo To run tests: cd build\msvc-debug ^&^& ctest --output-on-failure
echo.
pause