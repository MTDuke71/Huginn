@echo off
echo Huginn Chess Engine - Simple Assembly Generation
echo.

:: Set up Visual Studio x64 environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Create assembly output directory
if not exist "asm_output" mkdir "asm_output"
if not exist "asm_output\cpp" mkdir "asm_output\cpp"
if not exist "asm_output\asm" mkdir "asm_output\asm"

echo Generating assembly for performance-critical files...
echo.

:: Define files to process
set FILES=bitboard movegen_enhanced attack_detection position evaluation search

:: Copy include paths from successful build
set INCLUDES=/I"src" /I"src"
set DEFINES=/D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_WINDOWS /DDEBUG
set FLAGS=/std:c++17 /EHsc /O2 /W3

for %%f in (%FILES%) do (
    echo Processing %%f.cpp...
    
    :: Copy source file for comparison
    copy "src\%%f.cpp" "asm_output\cpp\" >nul 2>&1
    
    :: Generate assembly 
    cl %FLAGS% %DEFINES% %INCLUDES% /FA /Fa"asm_output\asm\%%f.asm" /c "src\%%f.cpp" >nul 2>&1
    
    if exist "asm_output\asm\%%f.asm" (
        echo   ? %%f.asm generated successfully
    ) else (
        echo   ? Failed to generate %%f.asm
    )
)

:: Process evaluation and search files
echo.
echo Processing evaluation and search files...
for %%f in (evaluation search) do (
    echo Processing %%f.cpp...
    
    :: Copy source file for comparison
    copy "src\%%f.cpp" "asm_output\cpp\" >nul 2>&1
    
    :: Generate assembly 
    cl %FLAGS% %DEFINES% %INCLUDES% /FA /Fa"asm_output\asm\%%f.asm" /c "src\%%f.cpp" >nul 2>&1
    
    if exist "asm_output\asm\%%f.asm" (
        echo   ? %%f.asm generated successfully
    ) else (
        echo   ? Failed to generate %%f.asm
    )
)

echo.
echo ========================================
echo Assembly generation completed!
echo.
echo Files organized in: asm_output\
echo.
echo C++ source files: asm_output\cpp\
echo Assembly files:   asm_output\asm\
echo.
echo Key files for performance analysis:
echo - bitboard.cpp/.asm        (Bit manipulation)
echo - movegen_enhanced.cpp/.asm (Move generation)
echo - attack_detection.cpp/.asm (Attack calculations)
echo - position.cpp/.asm        (Position management)
echo - evaluation.cpp/.asm      (Position evaluation)
echo - search.cpp/.asm          (Search algorithm)
echo - hybrid_evaluation.cpp/.asm (Advanced evaluation)
echo.
echo You can now open both .cpp and .asm files side by side
echo in your editor for performance analysis.
echo ========================================
echo.

echo Generated assembly files:
dir /b "asm_output\asm\*.asm" 2>nul

echo.
echo Assembly generation log:
echo Date: %date% %time% > asm_output\generation_log.txt
echo Compiler: MSVC x64 Release >> asm_output\generation_log.txt
echo Flags: %FLAGS% %DEFINES% >> asm_output\generation_log.txt
echo Files processed: %FILES% hybrid_evaluation simple_search >> asm_output\generation_log.txt

pause
