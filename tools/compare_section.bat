@echo off
setlocal EnableDelayedExpansion

if "%1"=="" (
    echo Usage: compare_section.bat ^<function_or_pattern^> [file_name]
    echo.
    echo Examples:
    echo   compare_section.bat rank_file_dirs attack_detection
    echo   compare_section.bat "constexpr int" attack_detection
    echo   compare_section.bat SqAttacked attack_detection
    echo.
    exit /b 1
)

set "PATTERN=%1"
set "FILE=%2"

if "%FILE%"=="" set "FILE=attack_detection"

echo ========================================
echo Comparing C++ and Assembly for: %PATTERN%
echo File: %FILE%
echo ========================================
echo.

echo === C++ CODE ===
if exist "asm_output\cpp\%FILE%.cpp" (
    findstr /N /C:"%PATTERN%" "asm_output\cpp\%FILE%.cpp"
    echo.
    echo Context (�3 lines):
    for /f "tokens=1 delims=:" %%i in ('findstr /N /C:"%PATTERN%" "asm_output\cpp\%FILE%.cpp"') do (
        echo Line %%i context:
        powershell -command "Get-Content 'asm_output\cpp\%FILE%.cpp' | Select-Object -Skip (%%i-4) -First 7 | ForEach-Object -Begin { $line = %%i-3 } -Process { Write-Host ('{0,3}: {1}' -f $line, $_); $line++ }"
        echo.
    )
) else (
    echo File asm_output\cpp\%FILE%.cpp not found
)

echo.
echo === ASSEMBLY CODE ===
if exist "asm_output\asm\%FILE%.asm" (
    echo Looking for related patterns in assembly...
    
    REM Look for the exact pattern
    findstr /N /C:"%PATTERN%" "asm_output\asm\%FILE%.asm" 2>nul
    
    REM Look for constants if it's a numeric pattern
    echo.
    echo Constants and data definitions:
    findstr /N "DD\|DQ\|CONST" "asm_output\asm\%FILE%.asm" | findstr /I "%PATTERN%\|10\|-10\|1\|-1" | head -10
    
    REM Look for function boundaries
    echo.
    echo Function boundaries:
    findstr /N "PROC\|ENDP" "asm_output\asm\%FILE%.asm" | head -5
    
) else (
    echo File asm_output\asm\%FILE%.asm not found
)

echo.
echo ========================================
echo Analysis complete. 
echo.
echo To view files side by side:
echo   code asm_output\cpp\%FILE%.cpp asm_output\asm\%FILE%.asm
echo.
echo To search for specific assembly instructions:
echo   findstr "mov\|lea\|add\|cmp" asm_output\asm\%FILE%.asm ^| head -20
echo ========================================
