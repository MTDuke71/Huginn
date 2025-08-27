@echo off
setlocal EnableDelayedExpansion

if "%1"=="" (
    echo Usage: show_asm_function.bat ^<function_name^> [file_name]
    echo.
    echo Examples:
    echo   show_asm_function.bat setBit
    echo   show_asm_function.bat generate_legal_moves movegen_enhanced
    echo   show_asm_function.bat evaluate_position evaluation
    echo.
    echo Available files: bitboard, movegen_enhanced, attack_detection,
    echo                  position, evaluation, search, hybrid_evaluation, simple_search
    exit /b 1
)

set "FUNCTION=%1"
set "FILE=%2"

if "%FILE%"=="" (
    echo Searching for function "%FUNCTION%" in all assembly files...
    echo.
    
    for %%f in (bitboard movegen_enhanced attack_detection position evaluation search hybrid_evaluation simple_search) do (
        if exist "asm_output\asm\%%f.asm" (
            findstr /C:"%FUNCTION%" "asm_output\asm\%%f.asm" >nul 2>&1
            if !errorlevel! == 0 (
                echo Found in %%f.asm:
                echo ================================
                findstr /N /C:"%FUNCTION%" "asm_output\asm\%%f.asm" | head -10
                echo ================================
                echo.
            )
        )
    )
) else (
    if exist "asm_output\asm\%FILE%.asm" (
        echo Searching for function "%FUNCTION%" in %FILE%.asm...
        echo.
        findstr /C:"%FUNCTION%" "asm_output\asm\%FILE%.asm" >nul 2>&1
        if !errorlevel! == 0 (
            echo Found:
            echo ================================
            findstr /N /A:1 /B:5 /C:"%FUNCTION%" "asm_output\asm\%FILE%.asm" | head -20
            echo ================================
        ) else (
            echo Function "%FUNCTION%" not found in %FILE%.asm
        )
    ) else (
        echo File asm_output\asm\%FILE%.asm not found
    )
)

echo.
echo To view the full context, open: asm_output\asm\%FILE%.asm
echo And search for: %FUNCTION%
