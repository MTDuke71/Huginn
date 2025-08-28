@echo off
REM Build script for GCC using MSYS2 environment

echo Setting up MSYS2 environment...
set MSYS2_PATH_TYPE=inherit
set PATH=C:\msys64\ucrt64\bin;%PATH%

echo Creating GCC build directory...
if not exist "build\gcc-release" mkdir "build\gcc-release"

cd build\gcc-release

echo Configuring with CMake...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc.exe -DCMAKE_CXX_COMPILER=g++.exe ..\..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

echo Building with GCC...
mingw32-make -j%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
cd ..\..

echo Running performance test...
echo 1 | build\gcc-release\bin\perft_suite_demo.exe
