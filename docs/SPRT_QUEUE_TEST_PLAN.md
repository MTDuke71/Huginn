# SPRT Queue Test Plan — 10 branches off baseline-t23

> **QUEUE CLOSED 2026-07-04.** All 11 items (the 10 branches below + #50)
> have final two-machine verdicts. **Winners for `baseline-t24`:** #50
> (already in `main`/t23), `see-ordering` (shipped), `root-twofold-avoid`
> (shipped). Everything else parked/rejected — see the per-item status below
> and BACKLOG.md for full numbers.
>
> **`candidate/t24` built, verified, AMD leg H1-ACCEPTED (2026-07-04).**
> Cherry-picked `see-ordering`'s + `root-twofold-avoid`'s feature commits onto
> `main` (both flags default ON). Verified: both-OFF reproduces `baseline-t23`
> exactly (14,306,844 nodes); both-ON matches `see-ordering`'s standalone
> signature (8,461,833 / d2d4); 204/205 tests pass incl. the positive
> repetition-routing test. **AMD SPRT vs t23: H1 ACCEPTED @580g — +48.84 ±
> 20.36, LOS 100%** (56.98%, W186/L105/D289, Ptnml [8,59,99,92,32], LLR 2.98,
> resolved inside the cap). **Undershoot guard: clean** — combined sits
> essentially at `see-ordering`'s solo number (+49.54), not below it; the gap
> to the naive sum (+62.05) is normal Elo non-additivity, not a negative
> interaction. No leave-one-out isolation needed. **Intel leg is the last
> thing needed before tagging `baseline-t24`** — run
> `test_huginn_gauntlet.bat t23` on `candidate/t24` there, then tag, snapshot
> `huginn_t24.exe` per machine, update BACKLOG/BASELINE_LADDER/CLAUDE.md,
> merge `candidate/t24` → `main`.

> **Purpose:** self-contained run-sheet for gauntleting the experiment branches
> created 2026-07-02. Written so it can be followed with no chat context.
> Companion state lives in [BACKLOG.md](BACKLOG.md) ("SPRT queue" section +
> per-item sections); this file is the *procedure*.
>
> **History (2026-07-02):** #50 (a Zobrist table out-of-bounds read, see below)
> was a correctness bug, not a feature, so it shipped directly to `main`
> ahead of the queue rather than going through the flag/branch/SPRT process —
> `main` was tagged **`baseline-t23`** immediately after. The other 10 items
> were originally built as `experiment/*` branches off the OLD `baseline-t22`
> tag (pre-#50); since #50 changes the Zobrist RNG stream (see the signature
> note below), every one of those branches was **rebased onto t23** by GitHub
> Copilot as `copilot/fix50-for-*` branches. Diff-verified byte-identical
> feature content to the originals (`git diff experiment/X copilot/fix50-for-X
> -- . ":(exclude)src/zobrist.hpp"` → empty for all 10) — **use the
> `copilot/fix50-for-*` branches for gauntlets, not the stale `experiment/*`
> ones**, which lack the #50 fix and are kept only as historical reference.

## ⚠ Known trap: verify the signature before EVERY gauntlet, every box

**2026-07-03 finding.** `copilot/fix50-for-futility-pv-guard`'s plain build
(`git checkout` + `cmake --build`, no explicit `-D`) silently compiled the
flag as **OFF** in a build directory that had been reconfigured across many
prior branch checkouts in the same session — even though the branch's
intended default is ON and the CMakeLists.txt registration looked correct.
Root cause: that branch used `set(VAR "" CACHE STRING ...)` +
`if(NOT VAR STREQUAL "")` to decide whether to forward the flag; once a
build directory's `CMakeCache.txt` had that variable cached as an empty
`STRING` (from that branch's very first configure), later relying on the
source's `#ifndef/#define` fallback did NOT reliably kick back in — CMake
cache entries are sticky and don't reset just because the guard logic
changes. The Intel gauntlet box hit the exact same thing independently and
had to force `-DENABLE_FUTILITY_PV_GUARD=1` to get the real test arm; on
this AMD-session box it was caught before any games were run by re-checking
the node-count signature against t23's own baseline and finding it matched
*exactly* (byte-for-byte) instead of differing as every other arm in the
queue did.
- **Fixed:** `futility-pv-guard`, `tt-aging`, `drawishness-scaling`, and
  `root-twofold-avoid` were all switched to the same pattern already used
  safely by `see-ordering`/`rfp-pv-guard`/`futility-depth2`/`trapped-bishop`/
  `pext`: `option(VAR "..." ON)` + an **unconditional** `if(VAR)/else()` that
  ALWAYS explicitly forwards `=1` or `=0` — no path silently no-ops.
- **Still verify anyway, on every box, every branch:** before trusting a
  gauntlet result, run `go depth 14` from startpos on the freshly-built exe
  and confirm the node count is NOT byte-identical to t23's own baseline
  (14,306,844) — the OFF arm should match, the test arm should differ. If a
  "test arm" build reproduces the baseline exactly, the flag silently didn't
  take (rebuild with an explicit `-DENABLE_<X>=1`, or `cmake -UENABLE_<X>`
  then reconfigure, to unstick a poisoned cache entry).
- `rfp-pv-guard` and `futility-depth2`'s already-reported results were
  independently re-verified via a fully fresh build directory after this was
  found — both stand as reported.

## Ground rules (apply to every arm)

- **Baseline:** `baseline-t23` (= t22 + the #50 Zobrist fix, `PIECE_NB` 12→13,
  unconditional — no flag). Reference opponent binary: build `huginn_t23.exe`
  per-machine from the tag (never copy binaries between boxes).
- **Run command (each box):**
  ```
  git fetch origin
  git checkout copilot/fix50-for-<name>
  test_huginn_gauntlet.bat t23
  ```
  The bat builds the current checkout (the branch's flag default IS the test
  arm), auto-detects vendor, runs SPRT [0,10] at 10+0.1, 1t, 64MB,
  noob_3moves.epd, cc=4, and tags artifacts `_intel`/`_amd`.
- **Ship bar** (unchanged): same-sign two-machine H1-accept, or tight
  cross-machine agreement for a small effect. Sign-splits park by default.
- **Record every result** in BACKLOG under the branch's item: Elo ± CI, LOS,
  W/L/D, Ptnml, LLR, PGN path (`gauntlet/<arm>_vs_t23_<machine>.pgn` — rename
  the bat's generic output so runs don't clobber each other).
- **After each run:** winners queue for combining into the next baseline
  (`t24`); losers get the branch parked with the result logged.
- **Sanity anchors (t23 — NOT the same numbers as t22!):** the #50 fix
  inserts a whole extra row into the `init_zobrist()` RNG draw sequence
  (`PIECE_NB` 12→13), which shifts every subsequently-drawn key (Side,
  Castle[], EpFile[], and the correctly-allocated black-king row) — so even
  positions that never touch a black-king edge case get different TT
  interactions and different fixed-depth node counts. This is expected and
  harmless (the search logic is unchanged; only the hash constants moved).
  **t23 reference signature** (1 thread, `OwnBook=false`, fresh `ucinewgame`,
  default 64MB hash):
  - startpos `go depth 14`: **nodes = 14,306,844**, score cp 24, bestmove e2e4
    (t22 was 12,035,479 / cp 27 — different constants, not a bug).
  - startpos `go depth 15`: **nodes = 21,844,725**, score cp 25, bestmove e2e4.
  - Kiwipete `go depth 13`: **nodes = 1,639,166**, score cp −88, bestmove e2a6
    — **and it is now deterministic** (3/3 runs identical; this position was
    BACKLOG #50's original nondeterminism repro, and the fix eliminated it,
    not just papered over it. Kiwipete is now usable as a second signature
    anchor alongside startpos going forward).
  - Any branch's OFF arm must reproduce the startpos numbers above exactly.

## The queue, in recommended order

Run top to bottom — ordered by expected value. Each entry: what it is, the
flag (branch default = test arm), what to expect, and the decision. Branch
names below are the `copilot/fix50-for-*` rebases (t23-based); flags and
feature content are unchanged from the original `experiment/*` design.

### 1. `copilot/fix50-for-see-ordering` — SEE good/bad capture split (#6) — ✅ SHIPPED
- **What:** in `pick_next_move`, captures with SEE < 0 drop below every quiet
  (−10M + MVV-LVA); SEE ≥ 0 captures stay above killers. Promotions + en
  passant exempt. Flag `ENABLE_SEE_ORDER_SPLIT`.
- **Expect:** large fixed-depth node reduction (was −52% vs t22's baseline;
  re-measure vs t23's own signature since the reference numbers moved). SEE
  cost is paid once per node in the scoring pass — the SPRT decides if the
  node savings beat it at fixed time. Prior #6 ("lazy SEE") was ~neutral;
  this is the full split.
- **Decision:** standard two-machine bar.

### 2. `copilot/fix50-for-razoring-off` — pruning-stack audit probe (#45-audit) — ⏸ PARKED (sign-split)
- **What:** razoring (depth 2–4, eval+400<alpha ⇒ depth−1) fully removed.
  Flag `ENABLE_RAZORING` — **INVERTED: default 0 = razoring OFF is the test
  arm; =1 is the t23-equivalent arm.** The plain branch build is correct for
  the bat.
- **Expect (verified 2026-07-02, t23-based build):** counterintuitively
  **FEWER** nodes at fixed depth with razoring OFF — 10,316,958 vs the ON
  arm's 14,306,844 (which reproduces the t23 signature byte-for-byte, as
  required). Razoring's `depth--` doesn't just shrink the local subtree; it
  also caches that subtree's results in the TT at the REDUCED depth, so a
  later transposition into the same position at the node's true (higher)
  depth gets a TT-depth-miss and re-searches — the same TT-reuse mechanism
  documented for `rfp-pv-guard` below. Net effect at fixed depth: OFF (no
  premature shallow TT stores) ends up smaller here. Trust the SPRT for Elo,
  not this node-count direction — it doesn't predict search quality either
  way.
- **Decision:** if OFF wins or is neutral → razoring is dead weight, remove it
  in the next baseline (fewer, better, sound). If OFF clearly loses →
  razoring earns its keep; park and optionally test a PV-guarded variant.

### 3. `copilot/fix50-for-rfp-pv-guard` — reverse futility PV guard (#45-audit) — ❌ REJECTED
- **What:** RFP fires only at null-window nodes (`beta - alpha == 1`), so it
  can no longer prune an interior PV node. Flag `ENABLE_RFP_PV_GUARD`.
- **Expect:** meaningfully more nodes at fixed depth (was −34% *smaller* than
  t22's baseline tree, i.e. t22-era RFP was pruning a lot; the PV guard
  removes that pruning at PV nodes specifically — trust only the SPRT for the
  Elo read).
- **Decision:** standard bar. Same #45 leak-class rationale as futility's fix.

### 4. `copilot/fix50-for-futility-depth2` — futility envelope ≤3→≤2 (#45 knob a) — ⏸ PARKED (flat neutral)
- **What:** move-level futility now only at depth ≤ 2 (Fruit trusts only
  depth 1). Flag `ENABLE_FUTILITY_DEPTH2`.
- **Expect:** more nodes at fixed depth (prunes less, was +20% vs the t22
  baseline). The bet: the removed depth-3 errors are worth more than the lost
  speed. #45 warned these knobs "may have already saturated" — a clean
  neutral is a fine outcome (park).
- **Decision:** standard bar.

### 5. `copilot/fix50-for-futility-pv-guard` — futility PV guard (#45 knob b) — ❌ REJECTED
- **What:** futility exempts ALL PV nodes (`beta - alpha == 1` added), Fruit's
  exact recipe. Flag `ENABLE_FUTILITY_PV_GUARD`.
- **⚠ CMake fix required first (see the trap note above) — rebuild before
  gauntleting.** Verified test-arm signature post-fix: startpos d14 =
  38,332,470 nodes (cp 27, e2e4) — a big jump, MUCH larger than the
  pre-rebase estimate (+11% vs the old t22 baseline was measured on a build
  that likely ALSO silently had the guard off, given how easy this was to
  trigger — treat that old number as unreliable and use 38,332,470 as the
  real reference for this arm going forward).
- **Decision:** standard bar. Independent of knob (a) — if BOTH win, fold one,
  re-run the other on top before folding it too (they overlap).

### 6. `copilot/fix50-for-tt-aging` — date-based TT aging (#42 idea 1) — ❓ INCONCLUSIVE (LTC check recommended)
- **What:** 6-bit search date packed into `node_type`'s upper bits;
  `new_search()` bumps it per `go`; stale-dated entries evictable regardless
  of depth; probe hits re-date. Flag `ENABLE_TT_AGING`.
- **Expect:** pays most in LONG games (many searches per game) — blitz SPRT
  may understate it. If blitz is neutral-positive, consider one LTC leg
  (60+0.6 vs Stash 17, like the t21 validation) before deciding.
- **Decision:** standard bar, with the LTC caveat above.

### 7. `copilot/fix50-for-drawishness-scaling` — endgame draw scaling (roadmap) — ⏸ PARKED (flat, needs weaker anchor)
- **What:** final white-positive eval scaled: pure OCB ×64/128; pawnless
  favoured side ahead ≤ a minor ×16/128. Flag `ENABLE_DRAWISHNESS_SCALING`.
  Targets the #5 conversion weakness (draw-heavy vs weaker engines).
- **Expect:** most visible vs WEAKER opponents (conversion), so self-play
  vs t23 may understate it; if neutral, a 200g check vs Stash 11/12 (the
  draw-prone pairing) is the better read.
- **Decision:** standard bar (+ optional external check).

### 8. `copilot/fix50-for-root-twofold-avoid` — root 2-fold avoidance (#44 f/u) — ✅ SHIPPED
- **What:** the root winning-repetition clamp now triggers on a SINGLE
  repetition (key already in game history), not just rule-threefold, so a won
  engine routes around the shuffle a move earlier. Inert without game history.
  Flag `ENABLE_ROOT_TWOFOLD_AVOID`.
- **Expect:** small; fires only in won-and-shuffling games. Draw-rate vs t23
  should tick down.
- **Decision:** standard bar; neutral-positive is shippable on soundness
  grounds if the drawn-won-games mechanism is visible in the PGNs.

### 9. `copilot/fix50-for-trapped-bishop` — CPW trapped-bishop locks (#20) — ⏸ PARKED (flat neutral)
- **What:** six square+pawn locks per side (a7/h7/b8/g8 full = seeds 100/120;
  a6/h6 lighter = 50/60), tapered, tuner-wired (`P_BISHOP_TRAPPED_*`). Flag
  `ENABLE_TRAPPED_BISHOP`.
- **Expect:** SF-class term ≈ 1–3 Elo — likely inconclusive at 1000g. Fine to
  run LAST or batch with a future eval round; a full Texel re-tune with these
  params exposed is the better long-term path.
- **Decision:** standard bar; treat "inconclusive lean-positive" as
  keep-parked-for-tune, not ship.

### 10. `copilot/fix50-for-pext` — BMI2 PEXT sliders (#32) — ⏸ PARKED (3–5% slower on this box, not faster)
- **What:** `_pext_u64` table indexing replaces magic multiply; init fills
  tables with pext ordering; `verify_or_die()` validates at startup. Flag
  `ENABLE_PEXT`. **Verified bit-identical** to its OFF arm (both t23-equal).
- **Procedure instead:** on EACH box, build the branch and run a quiet-machine
  interleaved nps A/B vs t23 (e.g. 5× `go depth 15` alternating). If pext is
  faster on both, batch it with the next speed ship (like #48+#49) — it does
  not need its own SPRT slot since behavior is identical.

## #50 — already shipped, no gauntlet slot needed

`PIECE_NB` 12→13 (`src/zobrist.hpp`), unconditional, no flag — landed directly
on `main` as a correctness fix ahead of the queue (see history note at top).
**AMD SPRT vs t22: H1 ACCEPTED @872g, +33.97 ± 16.60, LOS 100%** (54.87%,
W267/L182/D423, Ptnml [20,86,160,129,41], LLR 3.01). Intel leg still pending
for the two-machine bar, but it is already folded into `baseline-t23` — every
branch above is built on top of it.

## Combining the winners → t24 (screen → combine → validate)

1. **Screen** (the queue above): every arm independently vs t23.
2. **Combine:** merge ALL winning `copilot/fix50-for-*` branches into one
   candidate branch (`candidate/t24`), built on top of `baseline-t23` (which
   already has #50). The flags make this mechanical — the candidate is just
   "all winning flags ON"; resolve the (textual) flag-block merge conflicts,
   re-run the suite + the t23 startpos signature sanity of each OFF arm.
3. **Validate:** candidate vs t23, two machines, standard bar. **The combined
   measured Elo — not the sum of the individual runs — is t24's delta.**
4. **Undershoot guard (the t16 lesson):** compare combined vs the naive sum.
   Mild under-additivity is normal (ship the measured number). But if the
   combined result lands clearly BELOW the best single arm, a winner has
   turned negative in company (precedent: the #44+NMP bundle measured +62
   while NMP-verify silently dragged −15 — only leave-one-out isolation
   exposed it). Drop the most suspect interacting arm and re-validate;
   suspect order: the pruning pair-ups (futility knobs × razoring ×
   rfp-guard), then see-ordering (it changes what every pruning heuristic
   sees), then tt-aging (TT dynamics).
5. **Orthogonal-evidence exception:** if tt-aging only proves out at LTC, or
   drawishness only vs weaker external anchors, fold them on that evidence
   but keep them OUT of the blitz validation run's attribution reasoning —
   they're near-orthogonal to the search arms.
6. Tag `baseline-t24` on the folded main (flip flag defaults / delete losing
   paths per the aggressive-cleanup convention), build + snapshot
   `huginn_t24.exe` per machine, push tag.
7. Update BACKLOG (per-item results + close/park), BASELINE_LADDER.md (t24
   section), CLAUDE.md (baseline bullet). Park losing branches (do NOT
   delete — their commit messages carry the negative-result documentation).
8. Re-run the #37 thread with fresh eyes (collision-corrupted TT moves were a
   candidate mechanism for the illegal-bestmove bug, and #50 removed that
   specific collision source — worth re-testing whether #37 still repros).
