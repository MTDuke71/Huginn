#!/usr/bin/env pwsh
# Huginn Chess Engine - Preferred Build Script
# Uses msvc-x64-release preset for consistent build paths

Write-Host "=== Huginn Release Build (msvc-x64-release) ===" -ForegroundColor Green

# Configure using preset
Write-Host "Configuring project..." -ForegroundColor Yellow
cmake --preset msvc-x64-release

if ($LASTEXITCODE -ne 0) {
    Write-Host "Configuration failed!" -ForegroundColor Red
    exit 1
}

# Build using preset
Write-Host "Building project..." -ForegroundColor Yellow
cmake --build --preset msvc-x64-release --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable location: build\msvc-x64-release\bin\Release\huginn.exe" -ForegroundColor Cyan
Write-Host "Test executable: build\msvc-x64-release\bin\Release\huginn_tests.exe" -ForegroundColor Cyan