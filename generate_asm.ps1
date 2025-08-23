# Assembly Generation Script for Huginn Chess Engine
# Generates assembly files for key source files to analyze optimizations

param(
    [string]$OutputDir = "asm_output"
)

Write-Host "=== Huginn Assembly Generation ===" -ForegroundColor Green

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
    Write-Host "Created directory: $OutputDir" -ForegroundColor Cyan
}

# Key source files to generate assembly for
$sourceFiles = @(
    @{File="src/movegen_enhanced.cpp"; Desc="Move generation with IS_PLAYABLE macro"},
    @{File="src/attack_detection.cpp"; Desc="Square attack detection"},
    @{File="src/board120.cpp"; Desc="Board representation and lookup tables"},
    @{File="src/position.cpp"; Desc="Position management"},
    @{File="src/move.cpp"; Desc="Move encoding/decoding"}
)

Write-Host "Generating assembly files..." -ForegroundColor Yellow

foreach ($src in $sourceFiles) {
    $srcPath = $src.File
    $asmFile = $src.File -replace "src/", "" -replace "\.cpp$", ".s"
    $outputPath = Join-Path $OutputDir $asmFile
    
    Write-Host "  Processing: $($src.File)" -ForegroundColor White
    Write-Host "    Description: $($src.Desc)" -ForegroundColor Gray
    
    # Generate assembly with optimizations
    $cmd = "g++ -Isrc -O3 -DNDEBUG -std=gnu++17 -S `"$srcPath`" -o `"$outputPath`""
    Invoke-Expression $cmd
    
    if (Test-Path $outputPath) {
        $size = (Get-Item $outputPath).Length
        Write-Host "    Generated: $outputPath ($size bytes)" -ForegroundColor Green
    } else {
        Write-Host "    FAILED: Could not generate $outputPath" -ForegroundColor Red
    }
}

# Generate a summary report
$reportFile = Join-Path $OutputDir "assembly_analysis.md"
$report = @"
# Assembly Analysis for Huginn Chess Engine

Generated on: $(Get-Date)
Compiler: g++ with -O3 -DNDEBUG optimization
Target: x64 architecture

## Generated Files

"@

foreach ($src in $sourceFiles) {
    $asmFile = $src.File -replace "src/", "" -replace "\.cpp$", ".s"
    $outputPath = Join-Path $OutputDir $asmFile
    
    if (Test-Path $outputPath) {
        $size = (Get-Item $outputPath).Length
        $lines = (Get-Content $outputPath).Count
        $report += @"

### $asmFile
- **Source**: $($src.File)
- **Description**: $($src.Desc)
- **Size**: $size bytes
- **Lines**: $lines

"@
    }
}

$report += @"

## Key Optimizations to Analyze

1. **IS_PLAYABLE Macro** (in movegen_enhanced.s)
   - Look for: `cmpl `$119, %reg` (bounds check)
   - Look for: `cmpb `$0, (%rsi,%rbx)` (lookup table access)
   - Location: Around FILE_RANK_LOOKUPS+240 references

2. **Lookup Table Usage** (in board120.s)
   - FILE_RANK_LOOKUPS table definitions
   - Static data organization

3. **Move Generation Loops**
   - Sliding piece move generation (most critical for performance)
   - Piece list iteration optimization

## Analysis Commands

```powershell
# Search for specific patterns
Select-String "FILE_RANK_LOOKUPS" *.s
Select-String "cmpl.*119" *.s  # Bounds checks
Select-String "cmpb.*0" *.s    # Playable checks

# Count function calls vs inline code
Select-String "call.*" *.s | Measure-Object  # Function calls
Select-String "cmpb.*0" *.s | Measure-Object # Direct comparisons
```
"@

Set-Content -Path $reportFile -Value $report
Write-Host "Analysis report generated: $reportFile" -ForegroundColor Cyan

Write-Host ""
Write-Host "=== Assembly Generation Complete ===" -ForegroundColor Green
Write-Host "Files generated in: $OutputDir" -ForegroundColor Yellow
Write-Host "Review the assembly to see optimization effects!" -ForegroundColor White
