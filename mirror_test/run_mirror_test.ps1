param([switch]$Clean, [switch]$Verbose)

Write-Host "==== Huginn Mirror Evaluation Test Runner ====" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"

try {
    if (!(Test-Path "../CMakeLists.txt")) {
        throw "Error: Run this script from the mirror_test directory"
    }

    if ($Clean -and (Test-Path "mirror_eval_results.txt")) {
        Remove-Item "mirror_eval_results.txt"
        Write-Host "Cleaned previous results" -ForegroundColor Yellow
    }

    Write-Host "Building mirror evaluation test..." -ForegroundColor Green
    if ($Verbose) {
        & cmake --build "../build/msvc-x64-release" --config Release --target mirror_eval_test --parallel 24
    } else {
        & cmake --build "../build/msvc-x64-release" --config Release --target mirror_eval_test --parallel 24 2>&1 | Out-Null
    }
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
    Write-Host "Build successful" -ForegroundColor Green

    if (!(Test-Path "../test/mirror.epd")) {
        throw "Error: mirror.epd test file not found"
    }
    Write-Host "Found test data: ../test/mirror.epd" -ForegroundColor Green

    Write-Host ""
    Write-Host "Running mirror evaluation test..." -ForegroundColor Green
    
    $startTime = Get-Date
    Push-Location ".."
    try {
        if ($Verbose) {
            & ".\build\msvc-x64-release\bin\Release\mirror_eval_test.exe"
        } else {
            $output = & ".\build\msvc-x64-release\bin\Release\mirror_eval_test.exe" 2>&1
            $output | Where-Object { $_ -match "Total positions|Symmetric|Asymmetric|Results written" } | ForEach-Object {
                Write-Host "$_" -ForegroundColor White
            }
        }
        if ($LASTEXITCODE -ne 0) { 
            throw "Mirror test failed" 
        }
    } finally {
        Pop-Location
    }
    
    $endTime = Get-Date
    $duration = $endTime - $startTime
    
    if (Test-Path "mirror_eval_results.txt") {
        Write-Host ""
        Write-Host "Test Results:" -ForegroundColor Cyan
        $resultsContent = Get-Content "mirror_eval_results.txt"
        $summary = $resultsContent | Where-Object { $_ -match "Total positions tested|Symmetric|Asymmetric|Success rate" }
        
        foreach ($line in $summary) {
            if ($line -match "Success rate: 100%") {
                Write-Host "$line" -ForegroundColor Green
            } else {
                Write-Host "$line" -ForegroundColor White
            }
        }
        Write-Host ""
        Write-Host "Full results: mirror_eval_results.txt" -ForegroundColor Gray
        Write-Host "Duration: $($duration.TotalSeconds.ToString('F1')) seconds" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "Mirror evaluation test completed successfully!" -ForegroundColor Green

} catch {
    Write-Host ""
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Usage: .\run_mirror_test.ps1 [-Clean] [-Verbose]"
if ($Host.Name -eq "ConsoleHost") {
    Read-Host "Press Enter to exit"
}
