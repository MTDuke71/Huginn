# Huginn lint pass - surface unused functions across translation units.
#
# Uses cppcheck (winget install Cppcheck.Cppcheck). Targets the cross-TU
# dead-code question specifically: "function defined, called from nowhere"
# - which neither MSVC warnings nor grep can reliably answer for symbols
# with external linkage.
#
# Prefers compile_commands.json from the Ninja release build (more
# accurate include / define resolution); falls back to explicit -I paths
# so it always works without a Ninja configure step.
#
# Usage:
#   .\lint.ps1                          # run, print findings, write report, exit 0 (advisory)
#   .\lint.ps1 -Strict                  # exit 1 if any unusedFunction warnings (CI gate)
#   .\lint.ps1 -Output path\report.txt  # custom report path (default: build\lint_report.txt)
#
# What it catches (real, has bitten us recently):
#   - Functions defined but called from nowhere (verify_attack_tables,
#     SqAttacked wrapper, generate_king_moves_{lookup,hybrid}).
#
# What it CANNOT catch - needs human judgment:
#   - Duplicate data with separate callers (KingLookupTables::KING_ATTACKS
#     vs attack_tables::king_attacks - both had real callers).
#   - Dead CMake targets (perft_benchmark - no source references, only a
#     CMakeLists.txt entry).
#   - Write-only fields (the Priority 7 undo backups - symbol used, value
#     never read; needs dataflow analysis).
#   - Net-negative-Elo features (gauntlet measurement, not static analysis).

param(
    [switch]$Strict,
    [string]$Output = "build\lint_report.txt"
)

$ErrorActionPreference = "Continue"

# Locate cppcheck. Try PATH first; fall back to default winget install path.
$cppcheck = Get-Command cppcheck.exe -ErrorAction SilentlyContinue
if (-not $cppcheck) {
    $fallback = "C:\Program Files\Cppcheck\cppcheck.exe"
    if (Test-Path $fallback) {
        $cppcheckPath = $fallback
    } else {
        Write-Host "cppcheck.exe not found on PATH or at default install location." -ForegroundColor Red
        Write-Host "Install with:  winget install --id Cppcheck.Cppcheck --silent" -ForegroundColor Yellow
        exit 2
    }
} else {
    $cppcheckPath = $cppcheck.Source
}

Write-Host "=== Huginn lint (cppcheck unusedFunction) ===" -ForegroundColor Green
Write-Host "Using: $cppcheckPath" -ForegroundColor DarkGray

# Prefer compile_commands.json when the Ninja release build exists - gives
# cppcheck exact include paths and defines (e.g. ENABLE_FATHOM). Otherwise
# scan src/ with manual -I paths.
$compileCommands = "build\msvc-x64-release-ninja\compile_commands.json"
$cppcheckArgs = @(
    "--enable=unusedFunction"
    "--quiet"
    "--inline-suppr"
    "--suppress=missingIncludeSystem"
    "--suppress=unmatchedSuppression"
    "--error-exitcode=0"
    "--platform=win64"
    "--std=c++17"
)

if (Test-Path $compileCommands) {
    Write-Host "Mode: compile_commands.json (accurate)" -ForegroundColor DarkGray
    $cppcheckArgs += "--project=$compileCommands"
} else {
    Write-Host "Mode: explicit -I (no Ninja build present; for tighter analysis run cmake --preset msvc-x64-release-ninja once)" -ForegroundColor DarkGray
    # Scan src/ AND test/ together so test-only callers don't show up as
    # false-positive unusedFunction (e.g. is_initialized, get_*_bitboard
    # accessors only exercised from the GoogleTest suite).
    $cppcheckArgs += @(
        "-I", "src"
        "-I", "fathom\src"
        "--suppress=*:fathom/src/*"
        "src"
        "test"
        "perft"
        "mirror_test"
    )
}

Write-Host "Scanning..." -ForegroundColor Cyan

# PS 5.1 wraps native-exe stderr-via-2>&1 as NativeCommandError objects. Use
# file-redirect for stderr instead - OS-level redirect, no wrapping.
$stderrFile = [System.IO.Path]::GetTempFileName()
try {
    $stdout = & $cppcheckPath @cppcheckArgs 2>$stderrFile
    $stderr = Get-Content $stderrFile -ErrorAction SilentlyContinue
} finally {
    Remove-Item $stderrFile -ErrorAction SilentlyContinue
}

# cppcheck reports findings on stderr by default; merge both streams to filter.
$allOutput = @($stdout) + @($stderr)
$unused = $allOutput | Select-String -Pattern "unusedFunction"

Write-Host ""

# Build the plain-text report. Same content the console shows, minus colour.
$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$commit = (& git rev-parse --short HEAD 2>$null)
if (-not $commit) { $commit = "(no git)" }
$mode = if (Test-Path $compileCommands) { "compile_commands.json" } else { "explicit -I" }

$reportLines = @(
    "Huginn lint report"
    "Generated: $timestamp"
    "Commit:    $commit"
    "Mode:      $mode"
    "cppcheck:  $cppcheckPath"
    ""
    "Unused functions (cppcheck --enable=unusedFunction)"
    "===================================================="
)

if ($unused) {
    Write-Host "Unused functions found:" -ForegroundColor Yellow
    foreach ($line in $unused) {
        Write-Host "  $line" -ForegroundColor Yellow
        $reportLines += $line.ToString()
    }
    Write-Host ""
    Write-Host "Total: $($unused.Count)" -ForegroundColor Yellow
    $reportLines += ""
    $reportLines += "Total: $($unused.Count)"
} else {
    Write-Host "No unused functions found." -ForegroundColor Green
    $reportLines += "(none)"
}

# Write the report - ensure parent dir exists. build\ is gitignored so the
# default path doesn't pollute git status.
$outDir = Split-Path -Path $Output -Parent
if ($outDir -and -not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}
$reportLines | Set-Content -Path $Output -Encoding utf8
Write-Host ""
Write-Host "Report written to: $Output" -ForegroundColor Cyan

if ($unused -and $Strict) {
    Write-Host "Strict mode - exiting 1." -ForegroundColor Red
    exit 1
}
if ($unused) {
    Write-Host "Advisory mode - exiting 0. Use -Strict to gate." -ForegroundColor DarkGray
}
exit 0
