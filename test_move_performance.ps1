# Move Decoding Performance Test Script

param(
    [switch]$Optimized = $false
)

Write-Host "=== Move Decoding Performance Test ===" -ForegroundColor Green

if ($Optimized) {
    Write-Host "Building optimized version..." -ForegroundColor Yellow
    $cmd = "g++ -Isrc -O3 -DNDEBUG -std=gnu++17 test_move_decoding.cpp src/move.cpp src/chess_types.cpp -o build/test_move_decoding_optimized.exe"
    Invoke-Expression $cmd
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Running optimized benchmark..." -ForegroundColor Cyan
        & ".\build\test_move_decoding_optimized.exe"
    } else {
        Write-Host "Build failed!" -ForegroundColor Red
    }
} else {
    Write-Host "Building debug version..." -ForegroundColor Yellow
    Set-Location build
    cmake --build . --target test_move_decoding --config Debug
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Running debug benchmark..." -ForegroundColor Cyan
        & ".\test_move_decoding.exe"
    } else {
        Write-Host "Build failed!" -ForegroundColor Red
    }
    Set-Location ..
}

Write-Host ""
Write-Host "Analysis complete! See MOVE_DECODING_ANALYSIS.md for detailed results." -ForegroundColor Green
