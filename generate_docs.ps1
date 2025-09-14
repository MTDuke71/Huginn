# Generate Huginn Chess Engine Documentation
# PowerShell script for generating Doxygen documentation

Write-Host "Generating Huginn Chess Engine Documentation..." -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green

# Check if Doxygen is available
try {
    $doxygenVersion = & doxygen --version 2>$null
    Write-Host "Found Doxygen version: $doxygenVersion" -ForegroundColor Yellow
} catch {
    Write-Host "ERROR: Doxygen not found in PATH" -ForegroundColor Red
    Write-Host "Please install Doxygen or add it to your PATH" -ForegroundColor Red
    Write-Host "Download from: https://www.doxygen.nl/download.html" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Clean previous documentation
if (Test-Path "docs\html") {
    Write-Host "Cleaning previous documentation..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "docs\html"
}

# Generate documentation
Write-Host "Running Doxygen..." -ForegroundColor Yellow
try {
    & doxygen Doxyfile.huginn
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "================================================" -ForegroundColor Green
        Write-Host "Documentation generated successfully!" -ForegroundColor Green
        Write-Host "Location: docs\html\index.html" -ForegroundColor Green
        Write-Host "================================================" -ForegroundColor Green
        
        # Ask if user wants to open the documentation
        $openDocs = Read-Host "Open documentation in browser? (y/n)"
        if ($openDocs -eq "y" -or $openDocs -eq "Y") {
            Start-Process "docs\html\index.html"
        }
    } else {
        Write-Host "ERROR: Documentation generation failed" -ForegroundColor Red
        Read-Host "Press Enter to exit"
        exit 1
    }
} catch {
    Write-Host "ERROR: Failed to run Doxygen" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Read-Host "Press Enter to exit"