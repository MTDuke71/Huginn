# Huginn 2.4 — Lazy SMP + soft/hard time bounds

Released 2026-08-06. Two features, both in the search's *control* layer rather
than its chess knowledge: multithreading (`#40`) and a per-move time allocator
that can finish what it starts (`#65`). The evaluation is unchanged from 2.3.

**Single-core strength is unchanged from 2.3 apart from `#65`.** Read the
multicore number as a scaling claim, not a rating — see the warning below.

---

## Strength

| Claim | Measurement |
|---|---|
| **Single-core rating** | **~2600–2680 CCRL-blitz** (pooled ~2625 ± 18), carried over from 2.3's three-anchor Stash calibration |
| **`#65` time bounds** (1 thread) | **+10.60 ± 10.57**, LOS 97.54%, 2000g vs its own predecessor |
| **8 cores vs 1 core** | **+199.05 ± 25.63** (AMD, 400g) · **+172.78 ± 25.51** (Intel, 400g) |

> ⚠ **The multicore figure is not a rating increase and must never be added to
> the ~2625.** CCRL's Blitz and 40/15 lists run engines single-core, so a
> `Threads=8` gain does not apply there. What was measured is *Huginn on 8
> cores vs Huginn on 1 core* — the maximum-possible SMP delta by construction,
> since at equal thread counts the difference is zero. The two boxes are
> reported **separately and deliberately not pooled**: `Threads=8` is
> physically a different workload on different silicon (core counts, cache,
> SMT), so cross-machine agreement in *sign and rough magnitude* is the signal,
> and a pooled point estimate would claim precision the hardware cannot support.

## What's new

### Lazy SMP (`#40`) — the `Threads` option

`Threads` (default 1, max 512). Helpers share one transposition table and
opening book; each runs its own iterative-deepening schedule so the trees
diverge and the shared TT is worth something to every thread.

Depth gained at fixed time, two independent methods that agree to **0.07 ply**:

| | t1 | t2 | t4 | t8 |
|---|---|---|---|---|
| mean depth (40 positions, 1000 ms) | 13.57 | 14.30 | 14.82 | **15.43** |
| vs t1 | — | +0.72 | +1.25 | **+1.85 ply** |

In real games at t8: **+1.92 ply**, deeper on 79% of moves, shallower on 6.4%;
5.6× the nodes at identical `s/move` (time management undisturbed).

**`Threads=1` is bit-identical to 2.3** at fixed depth (startpos d14 =
3,481,582 / cp 31 / e2e4; Kiwipete d13 = 1,958,182 / cp −85 / e2a6). The
shipped default is single-threaded, so the default binary is provably the
2.3 search.

### Soft/hard time bounds (`#65`)

The per-move deadline was previously a single wall: `time/20 + inc/2`, enforced
mid-search. An iteration overrunning it by any margin was killed and its work
discarded — at 10+0.1 Huginn never once exceeded **0.56 s** across 2455
measured moves and used only **83.2%** of its clock.

Now split. The **soft** bound gates *starting* a new iteration; the **hard**
bound (`min(3×soft, safe_max, cap60, time/2)`) aborts an in-flight one. An
iteration already begun runs to completion.

| at 10+0.1 | 2.3 | 2.4 |
|---|---|---|
| clock utilisation | 83.2% | **96.9%** |
| max / move | 0.56 s | **1.67 s** |
| median / move | 0.203 s | **0.157 s** |

Less time on easy moves, far more on hard ones. The anti-forfeit guarantees are
unchanged — `safe_max` and `cap60` still bind, plus a `time/2` clamp that
matters only under the new 3× allowance. Verified across 200 games at
**5+0.05 with `Threads=8`**: zero time forfeits, zero illegal moves, zero
disconnects.

## Configuration notes

- **`Hash` default stays 64 MB.** Measured, not assumed: `Hash=128` vs `Hash=64`
  at `Threads=8` came back **−18.08 ± 21.85** (LOS 5.18%, 500g). Larger tables
  cost throughput — 13.52 → 13.15 → 12.90 Mnps at 64 / 128 / 256 MB.
- ⚠ That result is **machine-specific**. It was measured on a Ryzen 7 7800X3D
  whose 96 MB L3 V-Cache largely holds a 64 MB table; a CPU with a conventional
  ~30 MB L3 is already out of cache at 64 MB and may prefer more. If you run
  many threads on non-X3D hardware, it is worth testing your own optimum.
- Tablebase and book behaviour are unchanged from 2.3.
- **Recommended `Threads`** = physical cores. On a P+E-core Intel part,
  consider the physical P-core count.

## Known issues

- **`#66`** — Syzygy WDL scores are ply-independent (`TB_WIN` → a flat 28000,
  shown as `+280.00`), so in a won tablebase ending the engine has no gradient
  preferring a faster win and can shuffle. It still converts; the exposure is
  the 50-move rule. Fix identified (make the score ply-sensitive), not yet
  shipped.
- **`#37`** — the rare board-desync illegal-bestmove report remains guarded at
  the UCI boundary (never forfeits) with its root cause open. With SMP landed, a
  future desync report can no longer be cleanly attributed between `#37` and a
  threading race.
- One unreproduced test flake (a single `ctest` failure immediately after a
  build; 9 subsequent runs clean).

## Verification

316/316 tests (1 by-design skip). `Threads=1` fixed-depth signatures identical
to `baseline-t34` on both standard positions. Ship gate for the threading work
was run against the **immediate predecessor** rather than a shared baseline
(+2.78 ± 14.80, 1000g), after a chain-through-2.3 comparison produced a false
negative — see `SMP_MEASUREMENT_RUNBOOK.md` §7.
