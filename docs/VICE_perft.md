# EngineX Perft6 Compilation and Results (MSYS2 GCC)

## Compilation Command
```
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH; gcc -O3 -march=native EngineX/src/perft6_main.c EngineX/src/board.c EngineX/src/init.c EngineX/src/hashkeys.c EngineX/src/movegen.c EngineX/src/perft.c EngineX/src/makemove.c EngineX/src/search.c EngineX/src/validate.c EngineX/src/evaluate.c EngineX/src/misc.c EngineX/src/io.c EngineX/src/polybook.c EngineX/src/polykeys.c EngineX/src/pvtable.c EngineX/src/bitboards.c EngineX/src/data.c EngineX/src/attack.c -o EngineX/bin/perft6_main.exe -IEngineX/src
```

## Run Command
```
EngineX\bin\perft6_main.exe
```

## Results
```
Test Complete : 119060324 nodes visited in 3813ms
```

- Compiler: GCC (MSYS2, ucrt64)
- Optimization: -O3 -march=native
- Position: Standard chess starting position
- Depth: 6
- Nodes: 119060324
- Time: 3813 ms
