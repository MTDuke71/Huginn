# Development Tools

This directory contains development and analysis tools for the Huginn chess engine.

## Files

### Assembly Analysis
- **`generate_asm.ps1`** - Generates assembly output for key source files to analyze compiler optimizations

## Usage

### Generating Assembly for Analysis

```powershell
# Generate assembly files for optimization analysis
.\tools\generate_asm.ps1

# Generate assembly to custom directory
.\tools\generate_asm.ps1 -OutputDir "custom_asm_output"
```

The assembly generation script processes key source files and outputs human-readable assembly code that can be analyzed to understand compiler optimizations and identify performance bottlenecks.

### Assembly Analysis Workflow

1. Make code changes
2. Run `.\tools\generate_asm.ps1` 
3. Compare generated assembly before/after changes
4. Analyze optimization effectiveness at the assembly level

See `docs\ASSEMBLY_ANALYSIS.md` for detailed information on assembly analysis methodology and findings.
