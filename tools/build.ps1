# Huginn Build Script for Windows PowerShell
# This script ensures a clean build environment and proper toolchain separation

param(
    [Parameter()]
    [ValidateSet("msvc-debug", "msvc-release", "gcc-debug", "gcc-release", "mingw-debug", "mingw-release", "clang-debug", "clang-release", "clean", "help")]
    [string]$Config = "",
    
    [Parameter()]
    [switch]$UseVcpkg = $false,
    
    [Parameter()]
    [switch]$RunTests = $false,
    
    [Parameter()]
    [switch]$Verbose = $false
)

function Show-Help {
    Write-Host "Huginn Cross-Platform Build Script" -ForegroundColor Green
    Write-Host "=================================="
    Write-Host ""
    Write-Host "Usage: .\build.ps1 [config] [options]"
    Write-Host ""
    Write-Host "Configurations:"
    Write-Host "  msvc-debug     - Debug build with MSVC"
    Write-Host "  msvc-release   - Release build with MSVC"
    Write-Host "  gcc-debug      - Debug build with GCC"
    Write-Host "  gcc-release    - Release build with GCC"
    Write-Host "  mingw-debug    - Debug build with MinGW"
    Write-Host "  mingw-release  - Release build with MinGW"
    Write-Host "  clang-debug    - Debug build with Clang"
    Write-Host "  clang-release  - Release build with Clang"
    Write-Host "  clean          - Clean all build directories"
    Write-Host "  help           - Show this help"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -UseVcpkg      - Use vcpkg for dependencies (MSVC only)"
    Write-Host "  -RunTests      - Run tests after building"
    Write-Host "  -Verbose       - Show verbose output"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build.ps1 msvc-debug -UseVcpkg -RunTests"
    Write-Host "  .\build.ps1 gcc-release -Verbose"
    Write-Host "  .\build.ps1 clean"
}

function Test-Prerequisites {
    param([string]$CompilerType)
    
    $missing = @()
    
    # Check for CMake
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        $missing += "CMake"
    }
    
    # Check for Ninja
    if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
        $missing += "Ninja"
    }
    
    # Check compiler-specific prerequisites
    switch ($CompilerType) {
        "msvc" {
            if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
                $missing += "MSVC compiler (run from Developer Command Prompt or install Visual Studio)"
            }
        }
        "gcc" {
            if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
                $missing += "GCC compiler"
            }
        }
        "mingw" {
            if (-not (Get-Command x86_64-w64-mingw32-gcc -ErrorAction SilentlyContinue)) {
                $missing += "MinGW compiler (install via MSYS2)"
            }
        }
        "clang" {
            if (-not (Get-Command clang -ErrorAction SilentlyContinue)) {
                $missing += "Clang compiler"
            }
        }
    }
    
    if ($missing.Count -gt 0) {
        Write-Host "Missing prerequisites:" -ForegroundColor Red
        $missing | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
        return $false
    }
    
    return $true
}

function Clean-Environment {
    # Remove MSYS2 paths from environment to prevent header conflicts
    $env:PATH = ($env:PATH -split ';' | Where-Object { $_ -notlike '*msys64*' }) -join ';'
    $env:PKG_CONFIG_PATH = ""
    $env:CMAKE_PREFIX_PATH = ""
    $env:MSYSTEM = ""
    $env:MSYS2_PATH_TYPE = ""
    
    if ($Verbose) {
        Write-Host "Environment cleaned of MSYS2 paths" -ForegroundColor Yellow
    }
}

function Get-PresetName {
    param([string]$Config, [bool]$UseVcpkg)
    
    $preset = switch ($Config) {
        "msvc-debug" { if ($UseVcpkg) { "msvc-x64-debug-vcpkg" } else { "msvc-x64-debug" } }
        "msvc-release" { if ($UseVcpkg) { "msvc-x64-release-vcpkg" } else { "msvc-x64-release" } }
        "gcc-debug" { "gcc-x64-debug" }
        "gcc-release" { "gcc-x64-release" }
        "mingw-debug" { "mingw-x64-debug" }
        "mingw-release" { "mingw-x64-release" }
        "clang-debug" { "clang-x64-debug" }
        "clang-release" { "clang-x64-release" }
        default { $null }
    }
    
    return $preset
}

function Build-Project {
    param([string]$Preset)
    
    Write-Host "Configuring with preset: $Preset" -ForegroundColor Green
    
    $configArgs = @("--preset", $Preset)
    if ($Verbose) {
        $configArgs += "--debug-output"
    }
    
    try {
        & cmake @configArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Configuration failed"
        }
        
        Write-Host "Building..." -ForegroundColor Green
        
        $buildArgs = @("--build", "--preset", $Preset)
        if ($Verbose) {
            $buildArgs += "--verbose"
        }
        
        & cmake @buildArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-Host "Build completed successfully!" -ForegroundColor Green
        
        if ($RunTests) {
            Write-Host "Running tests..." -ForegroundColor Green
            & ctest --preset $Preset
            if ($LASTEXITCODE -ne 0) {
                Write-Warning "Some tests failed"
            } else {
                Write-Host "All tests passed!" -ForegroundColor Green
            }
        }
        
    } catch {
        Write-Host "Error: $_" -ForegroundColor Red
        exit 1
    }
}

# Main script logic
if ($Config -eq "help" -or $Config -eq "") {
    Show-Help
    exit 0
}

if ($Config -eq "clean") {
    Write-Host "Cleaning build directories..." -ForegroundColor Yellow
    if (Test-Path "build") {
        Remove-Item "build" -Recurse -Force
        Write-Host "Build directories cleaned" -ForegroundColor Green
    } else {
        Write-Host "No build directories found" -ForegroundColor Yellow
    }
    exit 0
}

# Check if we're in the right directory
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Host "Error: CMakeLists.txt not found. Please run this script from the project root." -ForegroundColor Red
    exit 1
}

# Extract compiler type from config
$compilerType = ($Config -split '-')[0]

# Check prerequisites
if (-not (Test-Prerequisites $compilerType)) {
    exit 1
}

# Clean environment for MSVC builds
if ($compilerType -eq "msvc") {
    Clean-Environment
}

# Validate vcpkg usage
if ($UseVcpkg) {
    if ($compilerType -ne "msvc") {
        Write-Host "Warning: -UseVcpkg is only supported with MSVC builds" -ForegroundColor Yellow
        $UseVcpkg = $false
    } elseif (-not $env:VCPKG_ROOT) {
        Write-Host "Warning: VCPKG_ROOT environment variable not set, ignoring -UseVcpkg" -ForegroundColor Yellow
        $UseVcpkg = $false
    }
}

# Get the preset name
$preset = Get-PresetName $Config $UseVcpkg
if (-not $preset) {
    Write-Host "Error: Invalid configuration '$Config'" -ForegroundColor Red
    Show-Help
    exit 1
}

# Display configuration
Write-Host "=== Build Configuration ===" -ForegroundColor Cyan
Write-Host "Configuration: $Config"
Write-Host "Preset: $preset"
Write-Host "Use vcpkg: $UseVcpkg"
Write-Host "Run tests: $RunTests"
Write-Host "Verbose: $Verbose"
Write-Host "=========================="

# Build the project
Build-Project $preset

Write-Host "Done!" -ForegroundColor Green