# Performance Testing Script for Huginn Chess Engine
# Run perft quick test and log results

param(
    [string]$Description = "Performance test"
)

Write-Host "=== Huginn Performance Test ===" -ForegroundColor Green
Write-Host "Description: $Description"
Write-Host "Starting perft quick test..." -ForegroundColor Yellow

# Build the project
Write-Host "Building project..." -ForegroundColor Cyan
Set-Location ..\build\msvc-x64-release
cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Run perft test and capture output
Set-Location ..
$output = Write-Output "1" | .\build\msvc-x64-release\bin\Release\perft_suite_demo.exe
$totalTimeMatch = $output | Select-String "Total time: (\d+)ms"
if ($totalTimeMatch) {
    $totalTime = $totalTimeMatch.Matches[0].Groups[1].Value
    Write-Host "Test completed in: ${totalTime}ms" -ForegroundColor Green
    
    # Get current date and git commit
    $date = Get-Date -Format "yyyy-MM-dd"
    $commit = git rev-parse --short HEAD
    
    # Append to performance tracking file
    $logEntry = "$date, $commit, $Description, $totalTime"
    Add-Content -Path ".\perft\performance_tracking.txt" -Value $logEntry
    
    Write-Host "Performance logged to performance_tracking.txt" -ForegroundColor Cyan
    Write-Host "Entry: $logEntry" -ForegroundColor White
} else {
    Write-Host "Could not extract timing from perft output!" -ForegroundColor Red
    Write-Host "Output was:" -ForegroundColor Yellow
    Write-Host $output
}
