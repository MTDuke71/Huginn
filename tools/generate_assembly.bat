@echo off
echo Huginn Chess Engine - Assembly Generation
echo.

set "CONFIG=%1"
if "%CONFIG%"=="" set "CONFIG=msvc-x64-release-asm"

echo Configuration: %CONFIG%
echo.

:: Configure with assembly generation
echo Configuring with assembly generation...
cmake --preset %CONFIG%

if %ERRORLEVEL% neq 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

:: Build the project
echo.
echo Building project...
cmake --build --preset %CONFIG%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

:: Generate assembly files
echo.
echo Generating assembly files...
cmake --build build/%CONFIG% --target generate_assembly

if %ERRORLEVEL% neq 0 (
    echo Assembly generation failed!
    pause
    exit /b 1
)

:: Create organized directory structure for comparison
echo.
echo Organizing files for side-by-side comparison...

set "BUILD_DIR=build\%CONFIG%"
set "ANALYSIS_DIR=%BUILD_DIR%\analysis"

if not exist "%ANALYSIS_DIR%" mkdir "%ANALYSIS_DIR%"
if not exist "%ANALYSIS_DIR%\cpp" mkdir "%ANALYSIS_DIR%\cpp"
if not exist "%ANALYSIS_DIR%\asm" mkdir "%ANALYSIS_DIR%\asm"

:: Copy relevant C++ source files
echo Copying C++ source files...
copy "src\bitboard.cpp" "%ANALYSIS_DIR%\cpp\" >nul
copy "src\movegen_enhanced.cpp" "%ANALYSIS_DIR%\cpp\" >nul
copy "src\attack_detection.cpp" "%ANALYSIS_DIR%\cpp\" >nul
copy "src\position.cpp" "%ANALYSIS_DIR%\cpp\" >nul
copy "src\search.cpp" "%ANALYSIS_DIR%\cpp\" >nul
copy "src\hybrid_evaluation.cpp" "%ANALYSIS_DIR%\cpp\" >nul
copy "src\simple_search.cpp" "%ANALYSIS_DIR%\cpp\" >nul

:: Copy assembly files
echo Copying assembly files...
if exist "%BUILD_DIR%\asm\*.asm" (
    copy "%BUILD_DIR%\asm\*.asm" "%ANALYSIS_DIR%\asm\" >nul
    echo MSVC assembly files (.asm) copied.
)
if exist "%BUILD_DIR%\asm\*.s" (
    copy "%BUILD_DIR%\asm\*.s" "%ANALYSIS_DIR%\asm\" >nul
    echo GCC assembly files (.s) copied.
)

echo.
echo ========================================
echo Assembly generation completed!
echo.
echo Files organized in: %ANALYSIS_DIR%
echo.
echo C++ source files: %ANALYSIS_DIR%\cpp\
echo Assembly files:   %ANALYSIS_DIR%\asm\
echo.
echo Key files for performance analysis:
echo - bitboard.cpp/.asm        (Bit manipulation)
echo - movegen_enhanced.cpp/.asm (Move generation)
echo - attack_detection.cpp/.asm (Attack calculations)
echo - position.cpp/.asm        (Position management)
echo - search.cpp/.asm          (Search algorithm)
echo - hybrid_evaluation.cpp/.asm (Advanced evaluation)
echo.
echo You can now open both .cpp and .asm files side by side
echo in your editor for performance analysis.
echo ========================================
echo.

:: List generated assembly files
echo Generated assembly files:
dir /b "%ANALYSIS_DIR%\asm\*.*" 2>nul

echo.
pause
