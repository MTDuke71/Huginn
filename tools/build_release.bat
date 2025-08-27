@echo off
echo Building Huginn Chess Engine - MSVC x64 Release
echo.

:: Set up Visual Studio x64 environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Create build directory
if not exist build\msvc-x64-release mkdir build\msvc-x64-release

:: Configure
echo Configuring...
cd build\msvc-x64-release
cmake ..\.. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl

if %ERRORLEVEL% neq 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

:: Build
echo.
echo Building...
ninja huginn huginn_uci huginn_legacy

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executables are in: build\msvc-x64-release\bin\
echo.
dir bin\*.exe

pause
