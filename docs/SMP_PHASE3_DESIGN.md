# Lazy SMP phase 3 — threading design (BACKLOG #40)

**Status:** DRAFT for review. No code written yet.
**Prereqs:** phase 1 (lockless TT, `3ea44d2`) and phase 2 (shared TT/book,
`50439da`) are both landed and signature-gated against `baseline-t34`.

Phases 1 and 2 were semantics-preserving and could lean on the fixed-depth
signature. Phase 3 cannot: it is the change that makes the search
non-deterministic. Everything below is written around that fact.

---

## 1. What `searchPosition` does that a helper thread must not

Reading [search.cpp:3064-3567](../src/search.cpp) closely, the current root
driver interleaves five responsibilities. Four of them are main-thread-only,
so "spawn N copies of `searchPosition`" is not a viable shortcut — it would be
wrong in four separate ways:

| Responsibility | Line | Why a helper must not do it |
|---|---|---|
| Opening-book + tablebase root probe | 3069-3101 | Must resolve **once**, before any thread spawns. N helpers probing is wasted work, and a book hit must short-circuit the whole search, not one worker. |
| `clearForSearch` → `tt_table.new_search()` | 3104 / 1919 | **The sharpest trap.** The TT search date is global (`#42`). Calling it once per worker bumps the date N times per search, so entries stored by worker 0 look stale to worker 1 *within the same search* and become evictable by any depth. It would quietly gut aging. |
| `info` line emission to stdout | 3516-3530 | Interleaved UCI output from N threads is malformed protocol. Only thread 0 may write. |
| Iteration-start time gate | 3138-3146 | Helpers have no time budget of their own — they run until the group stops. Applying the `elapsed*2 > budget` gate per worker would make helpers quit early for no reason. |
| PV display walk (`MakeMove`/`TakeMove` on `pos`) | 3412-3508 | Mutates the position in place. Each worker needs its own `Position` copy regardless; only thread 0 needs to build a display PV at all. |

**Consequence:** `searchPosition` splits into a *driver* (thread 0: probe,
prepare, spawn, search-with-output, join, select) and a *worker loop*
(helpers: iterative deepening, no output, no time gate, no probes).

---

## 2. Structure

```
UCIInterface
 └── SearchPool                       // owns everything shared, lives across searches
      ├── TranspositionTable          // ONE, lockless-safe since phase 1
      ├── PolyglotBook                // ONE, immutable during search
      ├── SyzygyTablebase*            // borrowed
      ├── std::atomic<bool> stop_all  // the group cancellation channel
      ├── std::atomic<uint64_t> nodes[MAX_THREADS]
      └── vector<unique_ptr<Engine>>  // one per thread, built via the phase-2 ctor
```

`SearchPool` owning the TT/book is what phase 2 was for — the workers are
already constructible with `Engine(shared_tt, shared_book, tb)`.

The pool persists across searches so the TT survives (only `ucinewgame` clears
it, per `#46`) and threads aren't respawned per move. Worker Engines are reused;
their per-thread tables get the usual `clear_search_tables()` aging per search.

---

## 3. Stop propagation

Phase 2's test suite pins that `should_stop` is **per-Engine** — deliberately,
because a worker must be individually stoppable. But a UCI `stop` has to reach
every worker at once, so per-Engine flags alone would mean the driver polls,
notices, then fans out at its own ~2048-node checkup cadence.

Better: one `stop_all` atomic in the pool that every worker's `checkup()` also
consults. `Engine` gains an optional `std::atomic<bool>* group_stop` (null in
the single-threaded path, so that path is bit-identical):

```cpp
void Engine::checkup(SearchInfo& info) {
    if (info.quit || info.stopped) return;

    if (should_stop.load(std::memory_order_relaxed) ||
        (group_stop && group_stop->load(std::memory_order_relaxed))) {
        info.stopped = true;
        return;
    }
    ...
```

`stop_all` is set by: the UCI pump on `stop`/`quit`, the driver's own time
expiry, and the driver completing its final iteration. Workers only ever *read*
it — `SearchInfo` keeps its single-writer property from `#56`.

---

## 4. The stdin trap

`checkup()` currently ends its input block with:

```cpp
if (info.on_input) info.on_input(info);
else read_input(info);          // <-- fallback
```

For a helper thread **both branches are wrong**. `input_is_waiting()` and
`read_input()` are documented main-thread-only
([input_checking.hpp:30](../src/input_checking.hpp#L30), `:54`) — they touch
global stdin state. A helper reaching either would race the UCI thread for
console input and could *consume a command the GUI sent*.

`SearchInfo` needs an explicit opt-out, checked before the poll:

```cpp
bool polls_input = true;   // false on every helper's SearchInfo
```

Making it opt-*out* rather than opt-in keeps every existing construction site
(tests, bench, the UCI driver) behaving exactly as today.

---

## 5. Node counting

UCI `nodes`/`nps` must report the sum across threads, but `info.nodes` is a
plain `uint64_t` written by its own thread — the driver reading it directly is
a data race.

Each worker publishes to `pool.nodes[i]` (relaxed atomic) from inside
`checkup()`, i.e. every ~2048 nodes, not per node. The driver sums the array
when it emits an `info` line. Cost is one relaxed store per checkup; the
reported figure lags by <2048 nodes per thread, which is irrelevant at the
millions-of-nodes scale these lines report.

---

## 6. Search diversification

Pure Lazy SMP relies on threads desynchronising so they explore different
subtrees and cross-pollinate through the TT. Two cheap, well-established
levers, and nothing more for v1:

- **Depth skew** — worker `i` begins its iterative deepening at
  `1 + (i % 2)`, so odd workers run a ply ahead.
- **Aspiration skew** — workers use a slightly wider initial delta than the
  driver's `ASPIRATION_DELTA = 50`, so they fail differently and search
  different windows.

Deliberately *not* in v1: per-thread pruning-parameter variation, move-ordering
randomisation, split-point work sharing. Each is an independent Elo experiment
and would confound the "does SMP work at all" measurement.

---

## 7. Which move gets played

Two options, and I recommend the conservative one for v1:

- **(A) Driver's result — recommended.** Helpers contribute *only* through the
  shared TT. `bestmove` is whatever thread 0's iterative deepening concluded.
  This is textbook Lazy SMP, and it keeps `bestmove` a pure function of thread
  0's search given the TT contents — one fewer moving part while the threading
  itself is unproven.
- **(B) Best across threads** by `(completed_depth, score)`. Typically worth a
  little Elo, but adds a cross-thread result-selection race and makes a bad
  move harder to attribute. Queue as a follow-up experiment once (A) is
  measured and stable.

---

## 8. Verification contract

This is the part `#40`'s original writeup called the real blocker.

**`Threads=1` must remain bit-identical to `baseline-t34`.** With one thread
the pool spawns nothing, `group_stop` is null, `polls_input` is true, and the
driver *is* today's `searchPosition`. Both existing gates keep applying
verbatim:

| Gate | Expected |
|---|---|
| startpos d14 | `3,481,582 / cp 31 / e2e4` |
| Kiwipete d13 | `1,958,182 / cp −85 / e2a6` |

If either moves at `Threads=1`, the refactor is wrong — this stays a hard,
cheap, deterministic gate for everything except the threading itself.

**`Threads=N` is measured only by time-controlled SPRT at gauntlet
concurrency 1.** `cc=4` would oversubscribe the box and make `10+0.1`
wall-clock meaningless — roughly a 4× throughput hit per box, which is the
real cost of this phase. Fixed-depth node counts are *not* a comparable unit
under SMP: the SF18 measurement in
[SF18_GAP_STUDY.md](SF18_GAP_STUDY.md) found 8 threads gave ~6.4× node
throughput but **2.5× longer** wall-clock to the same nominal depth.

**New non-signature checks phase 3 needs:**

1. `Threads=1` bit-identity (above) — the discipline carrier.
2. Thread-sanitizer or stress run: N=8, repeated `go`/`stop` cycles, asserting
   no crash, no illegal `bestmove`, no lost `stop`.
3. `stop` latency under N=8 — bestmove must follow `stop` promptly at every
   thread count.
4. TT-shared-progress test: with N=2 the second worker demonstrably probes
   entries the first stored (phase 2 already pins the mechanism; this pins it
   through a real threaded search).
5. `ucinewgame` / `Hash` resize while workers exist — must not race. Both are
   between-search operations, so the pool must assert all workers are joined.

---

## 9. Open questions for review

1. ~~**Fathom thread-safety.**~~ **RESOLVED — safe.** The vendored copy
   guards its probe path with real mutexes
   ([tbprobe.c:79-102](../fathom/src/tbprobe.c#L79-L102)), and the opt-out
   `TB_NO_THREADS` is *not* defined anywhere in our build — it is commented out
   in [tbconfig.h:61](../fathom/src/tbconfig.h#L61) and appears only in the
   standalone `src/apps/Makefile`, which we don't use. `tb_probe_wdl` is
   thread-safe as built. No action needed.
2. **`#37` interaction.** The board-desync guard at the UCI boundary validates
   `bestmove` against a clean pre-search snapshot. Under option (A) that logic
   is unchanged and still covers us. Worth re-reading once the driver is split.
3. **Default `Threads` value.** Proposal: default **1**, so the shipped default
   binary stays deterministic and every existing test/tool is unaffected. The
   gauntlet opts in explicitly.
