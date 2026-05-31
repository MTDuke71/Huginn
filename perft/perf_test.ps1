# Performance Testing Script for Huginn Chess Engine
# Configures with Fathom enabled (matching the gauntlet bats), builds the
# perft_suite target, runs the quick test, and appends a row to
# performance_tracking.txt with: Date, Commit, CPU, Description, Time.
#
# The CPU column lets results from different machines (e.g., the AMD
# 7800X3D and the Intel 13700K gauntlet boxes) be distinguished in the
# shared log without manual annotation.

param(
    [string]$Description = "Performance test"
)

Write-Host "=== Huginn Performance Test ===" -ForegroundColor Green

# Detect CPU model for the log entry (normalize whitespace, trim trailing
# vendor noise like "(R)" "(TM)" "  Processor")
$cpu = (Get-CimInstance Win32_Processor | Select-Object -First 1).Name
$cpu = ($cpu -replace '\(R\)', '' -replace '\(TM\)', '' -replace '\s+Processor\s*$', '' -replace '\s+', ' ').Trim()

Write-Host "CPU: $cpu" -ForegroundColor Cyan
Write-Host "Description: $Description"
Write-Host "Starting perft quick test..." -ForegroundColor Yellow

# Configure with Fathom (Syzygy TB) enabled — required because
# src/syzygy_tablebase.hpp currently includes <tbprobe.h> unconditionally.
# Matches the convention used by test_huginn_vs_t*.bat.
Write-Host "Configuring project (msvc-x64-release, ENABLE_FATHOM=ON)..." -ForegroundColor Cyan
cmake --preset msvc-x64-release -DENABLE_FATHOM=ON
if ($LASTEXITCODE -ne 0) {
    Write-Host "Configure failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Building perft_suite..." -ForegroundColor Cyan
cmake --build build/msvc-x64-release --config Release --target perft_suite
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Run the quick test (option 1 on the menu) and capture output
$output = Write-Output "1" | .\build\msvc-x64-release\bin\Release\perft_suite.exe
$totalTimeMatch = $output | Select-String "Total time: (\d+)ms"
if ($totalTimeMatch) {
    $totalTime = $totalTimeMatch.Matches[0].Groups[1].Value
    Write-Host "Test completed in: ${totalTime}ms on $cpu" -ForegroundColor Green

    $date = Get-Date -Format "yyyy-MM-dd"
    $commit = git rev-parse --short HEAD

    # Format: Date, Commit, CPU, Description, Time(ms)
    # (Older rows in performance_tracking.txt are 4-column without CPU
    # — this is the new format from the 5-column extension.)
    $logEntry = "$date, $commit, $cpu, $Description, $totalTime"
    Add-Content -Path ".\perft\performance_tracking.txt" -Value $logEntry

    Write-Host "Performance logged to performance_tracking.txt" -ForegroundColor Cyan
    Write-Host "Entry: $logEntry" -ForegroundColor White
} else {
    Write-Host "Could not extract timing from perft output!" -ForegroundColor Red
    Write-Host "Output was:" -ForegroundColor Yellow
    Write-Host $output
}
