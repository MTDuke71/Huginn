# PowerShell script to run mirror evaluation tests
# Usage: .\run_mirror_test.ps1

param(
    [switch]$Clean,
    [switch]$Verbose
)

Write-Host "==== Huginn Mirror Evaluation Test Runner ====" -ForegroundColor Cyan
Write-Host ""

# Set error handling
$ErrorActionPreference = "Stop"

try {
    # Check if we're in the right directory (should be in mirror_test subdirectory)
    if (!(Test-Path "../CMakeLists.txt")) {
        throw "Error: Run this script from the mirror_test directory (as a subdirectory of Huginn root)"
    }

    # Clean previous results if requested
    if ($Clean) {
        Write-Host "🧹 Cleaning previous results..." -ForegroundColor Yellow
        if (Test-Path "mirror_eval_results.txt") {
            Remove-Item "mirror_eval_results.txt"
            Write-Host "   Removed old results file"
        }
    }

    # Build the mirror test executable
    Write-Host "🔨 Building mirror evaluation test..." -ForegroundColor Green
    if ($Verbose) {
        & cmake --build "../build/msvc-x64-release" --config Release --target mirror_eval_test --parallel 24
    } else {
        & cmake --build "../build/msvc-x64-release" --config Release --target mirror_eval_test --parallel 24 2>&1 | Out-Null
    }
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE. Try running 'cmake --build ../build/msvc-x64-release --config Release' first to ensure all dependencies are built."
    }
    Write-Host "   ✅ Build successful"

    # Check if test data exists
    if (!(Test-Path "../test/mirror.epd")) {
        throw "Error: mirror.epd test file not found in test/ directory"
    }
    Write-Host "   ✅ Found test data: ../test/mirror.epd"

    # Run the mirror test
    Write-Host ""
    Write-Host "🧪 Running mirror evaluation test..." -ForegroundColor Green
    Write-Host "   Testing all positions in mirror.epd..."
    
    $startTime = Get-Date
    
    # Change to parent directory to run the executable (since it expects to find test/mirror.epd)
    Push-Location ".."
    try {
        if ($Verbose) {
            & ".\build\msvc-x64-release\bin\Release\mirror_eval_test.exe"
        } else {
            $output = & ".\build\msvc-x64-release\bin\Release\mirror_eval_test.exe" 2>&1
            # Extract just the summary info
            $output | Where-Object { $_ -match "Total positions|Symmetric|Asymmetric|Results written" } | ForEach-Object {
                Write-Host "   $_" -ForegroundColor White
            }
        }
        
        if ($LASTEXITCODE -ne 0) {
            throw "Mirror test failed with exit code $LASTEXITCODE"
        }
    } finally {
        Pop-Location
    }
    
    $endTime = Get-Date
    $duration = $endTime - $startTime
    
    # Check results
    if (Test-Path "mirror_eval_results.txt") {
        Write-Host ""
        Write-Host "📊 Test Results:" -ForegroundColor Cyan
        
        # Parse results from the output file
        $resultsContent = Get-Content "mirror_eval_results.txt"
        $summary = $resultsContent | Where-Object { $_ -match "Total positions tested|Symmetric \(PASS\)|Asymmetric \(FAIL\)|Success rate" }
        
        foreach ($line in $summary) {
            if ($line -match "Success rate: 100%") {
                Write-Host "   $line" -ForegroundColor Green
            } elseif ($line -match "Asymmetric.*: 0") {
                Write-Host "   $line" -ForegroundColor Green  
            } else {
                Write-Host "   $line" -ForegroundColor White
            }
        }
        
        Write-Host ""
        Write-Host "   📝 Full results: mirror_eval_results.txt" -ForegroundColor Gray
        Write-Host "   ⏱️  Duration: $($duration.TotalSeconds.ToString('F1')) seconds" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "✅ Mirror evaluation test completed successfully!" -ForegroundColor Green

} catch {
    Write-Host ""
    Write-Host "❌ Error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Options:" -ForegroundColor Gray
Write-Host "  -Clean   : Remove previous results before running" -ForegroundColor Gray
Write-Host "  -Verbose : Show detailed build and test output" -ForegroundColor Gray
Write-Host ""
Write-Host "Examples:" -ForegroundColor Gray
Write-Host "  .\run_mirror_test.ps1" -ForegroundColor Gray
Write-Host "  .\run_mirror_test.ps1 -Clean" -ForegroundColor Gray
Write-Host "  .\run_mirror_test.ps1 -Verbose" -ForegroundColor Gray
