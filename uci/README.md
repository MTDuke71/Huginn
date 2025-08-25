# UCI Test Files

This directory contains test files for the UCI (Universal Chess Interface) implementation.

## Files

- **`test_uci_commands.txt`** - Basic UCI command sequence for testing
- **`test_uci_debug.txt`** - UCI commands for debugging purposes  
- **`test_uci_move.txt`** - UCI move-related test commands
- **`test_uci_positions.txt`** - UCI position setup test commands

## Usage

These files can be used to test the UCI implementation by piping their contents to the chess engine:

```powershell
# Test basic UCI commands
Get-Content uci\test_uci_commands.txt | .\build\huginn.exe

# Test position commands
Get-Content uci\test_uci_positions.txt | .\build\huginn.exe
```

See `docs\UCI_IMPLEMENTATION.md` for detailed documentation on the UCI interface implementation.
