# Lazy SMP measurement runbook (BACKLOG #40)

How to build, verify, and measure a multithreaded Huginn build — written so a
second machine can reproduce the AMD-box numbers without re-deriving the
method. Every command is copy-pasteable.

> **The one-line version:** verify `Threads=1` is bit-identical *before*
> spending hours on a match, run SMP matches at `concurrency 1`, and never add
> the multicore Elo to the CCRL rating.

---

## 1. Build

SMP lives on branch **`smp-phase1-lockless-tt`**. Build at **`429cefd` or
later** — that commit is the diversification fix, and anything earlier
measures the *bug*, not the feature (helpers duplicated each other's work; see
[BACKLOG.md](BACKLOG.md) #40).

```powershell
git checkout smp-phase1-lockless-tt
cmake --preset msvc-x64-release -DENABLE_FATHOM=ON
cmake --build build/msvc-x64-release --config Release --target huginn
```

Fathom **ON** — the baselines it is measured against have tablebase support,
and the match passes `SyzygyPath` to both sides.

## 2. Verify before you spend hours

The go/no-go check. `Threads=1` must be **bit-identical to `baseline-t34`**;
if it is not, the build is wrong and the match is worthless.

```powershell
python tools/smp/signature.py .\build\msvc-x64-release\bin\Release\huginn.exe 14 startpos
python tools/smp/signature.py .\build\msvc-x64-release\bin\Release\huginn.exe 13 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
```

Expected, exactly:

| position | nodes | score | move |
|---|---|---|---|
| startpos d14 | **3,481,582** | cp 31 | e2e4 |
| Kiwipete d13 | **1,958,182** | cp −85 | e2a6 |

`signature.py` exists because piping `go depth N` straight from `printf`
**silently truncates the search** — the `quit` sits in the stdin buffer and the
mid-search input pump (#56) consumes it mid-iteration, so you get a bestmove
several plies early. The script holds stdin open until `bestmove` arrives.

Then the suite: `ctest -C Release` from `build/msvc-x64-release`, expect
**310/310** (1 by-design skip).

## 3. Concurrency rules — the easiest thing to get wrong

`-concurrency` interacts with `Threads`, and getting it wrong invalidates the
result rather than just slowing it down.

| Match | Threads | `-concurrency` | Why |
|---|---|---|---|
| SMP arm (t2/t4/t8) vs anything | >1 | **1** | `cc>1` oversubscribes the CPU, so `10+0.1` wall-clock stops meaning anything. This is the ~4× throughput cost #40 always predicted. |
| Default arm (`Threads=1`) vs a baseline | 1 | 4 | Both sides single-threaded — same as every prior baseline gauntlet, and poolable with them. |

Fixed-*depth* matches are timing-insensitive and could tolerate `cc>1`, but
they are also the wrong instrument for SMP: more threads does not raise a
fixed-depth result, and [SF18_GAP_STUDY.md](SF18_GAP_STUDY.md) measured
fixed depth to be actively misleading under SMP.

## 4. The Intel leg

```
fastchess.exe ^
  -engine cmd=huginn_smp_t8.exe name=Huginn_smp_t8 option.OwnBook=false option.SyzygyPath=c:\TB\ option.Threads=8 ^
  -engine cmd=huginn_2_3.exe   name=Huginn_2.3    option.OwnBook=false option.SyzygyPath=c:\TB\ ^
  -each tc=10+0.1 ^
  -rounds 200 -repeat -concurrency 1 -recover ^
  -openings file=noob_3moves.epd format=epd order=random ^
  -pgnout file=<repo>\gauntlet\smp_t8_vs_23_intel.pgn notation=san append=false nodes=true seldepth=true nps=true ^
  -log    file=<repo>\gauntlet\fastchess_smp_t8_vs_23_intel.log level=warn
```

Keep **`nodes=true seldepth=true`** — those annotations are what the per-move
depth analysis is built from, and they cannot be recovered afterwards.

`SyzygyPath` goes to **both** sides deliberately: baselines up to t29
auto-probed `c:\TB\` at startup, so omitting it makes the arms silently
TB-asymmetric on a box that has tablebases.

AMD reference to compare against: **+199.05 ± 25.63**, LOS 100%, 75.88%,
400g ([gauntlet/smp_t8_vs_23_amd.pgn](../gauntlet/smp_t8_vs_23_amd.pgn)).

## 5. Analysis

```powershell
# Per-move depth, paired by game and move number, both engines
python tools/smp/pgn_depth_compare.py gauntlet\smp_t8_vs_23_intel.pgn

# Time-to-depth scaling, one position, several thread counts
python tools/smp/time_to_depth.py <exe> 17 "<fen>" 3 1 2 4 8

# Controlled depth A/B: identical positions, fixed movetime, Threads varies
python tools/smp/depth_ab.py <exe> tools\smp\cc_positions.epd 1000 3 1 2 4 8
```

`cc_positions.epd` is 40 positions evenly sampled from `CC_Games.pgn`'s 1929
eligible (regenerate with `extract_positions.py`). Requires `python-chess`
only for regeneration, not for the analysis scripts.

**Which method to trust.** `depth_ab.py` is confound-free: identical position,
identical clock, only `Threads` differs. `pgn_depth_compare.py` pairs White's
move N against Black's move N, which are *different positions* — a quiet
position where the engine really searches deep versus a forced recapture. In
principle that is a confound. In practice, at 24k pairs with colour-balanced
openings the two methods agreed to **0.07 ply** (+1.85 vs +1.92), so the PGN
method is usable and comes free with any gauntlet. Prefer `depth_ab.py` for
small samples.

## 6. Interpreting the result

**Report the two boxes separately; do not pool them.** Every prior baseline
inverse-variance-pooled its AMD and Intel legs because both ran the *same*
single-threaded engine. That does not hold here: `Threads=8` is physically a
different workload on different silicon (core counts, P/E-core mixes, cache
size — and 8 threads sharing one TT is memory-bound). Cross-machine agreement
in **sign and rough magnitude** is the signal; a pooled point estimate would
claim precision the hardware difference cannot support. On a P+E-core Intel
part, also consider running `Threads=<physical P-core count>`.

**Never add the multicore Elo to the CCRL rating.** CCRL's Blitz and 40/15
lists run engines single-core, so a `Threads=8` gain does not apply there.
Huginn stays **~2625 CCRL-blitz, single-core**, and SMP is reported as a
separate scaling claim ("on 8 cores vs 1 core: +199 ± 26"). It is also the
maximum-possible SMP delta by construction — at equal thread counts on both
sides the difference is zero.

**A null result is the expected outcome for the default arm.** `Threads=1` is
bit-identical at fixed depth, so a `Threads=1` gauntlet is testing only whether
the pool wrapper and group-stop plumbing cost anything under a real clock. Use
a fixed round count and read the CI bound; an early-stopping SPRT returns a
loose CI on a true null.

## 7. Known gotchas

- **A pre-`429cefd` build measures the r1 bug.** r1 diversified only by
  `thread_id % 2` — two distinct schedules regardless of thread count, so t8
  performed like t2 while burning 3× the nodes. It passed all 310 tests.
- **Scaling is invisible to the test suite.** Every correctness property held
  through the r1 bug. Scaling must be measured explicitly.
- **One unreproduced test flake** exists — a single `ctest` run immediately
  after a build showed 1 failure; 9 subsequent runs were clean. If you see a
  failure, re-run before believing it, and record which test it was (that
  information was lost the first time).
