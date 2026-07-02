# SPRT Queue Test Plan — the 11 experiment branches off baseline-t22

> **Purpose:** self-contained run-sheet for gauntleting the experiment branches
> created 2026-07-02. Written so it can be followed with no chat context.
> Companion state lives in [BACKLOG.md](BACKLOG.md) ("SPRT queue" section +
> per-item sections); this file is the *procedure*.

## Ground rules (apply to every arm)

- **Baseline:** `baseline-t22` (= tag `b0d7780` content). Reference opponent
  binary: `huginn_t22.exe` in the fastchess dir — already snapshotted on the
  AMD box; build it per-machine from the tag on any box that lacks it (never
  copy binaries between boxes).
- **Run command (each box):**
  ```
  git fetch origin
  git checkout <branch>
  test_huginn_gauntlet.bat t22
  ```
  The bat builds the current checkout (the branch's flag default IS the test
  arm), auto-detects vendor, runs SPRT [0,10] at 10+0.1, 1t, 64MB,
  noob_3moves.epd, cc=4, and tags artifacts `_intel`/`_amd`.
- **Ship bar** (unchanged): same-sign two-machine H1-accept, or tight
  cross-machine agreement for a small effect. Sign-splits park by default.
- **Record every result** in BACKLOG under the branch's item: Elo ± CI, LOS,
  W/L/D, Ptnml, LLR, PGN path (`gauntlet/<arm>_vs_t22_<machine>.pgn` — rename
  the bat's generic output so runs don't clobber each other).
- **After each run:** winners queue for folding into `baseline-t23`; losers
  get the branch parked with the result logged. If MULTIPLE arms win, fold
  them one at a time (re-gauntlet the second on top of the first if both touch
  search) — pruning arms interact.
- **Sanity anchors:** t22 startpos `go depth 14` = 12,035,479 nodes / cp 27 /
  bestmove e2e4 (d15 = 18,223,597). Any branch's OFF arm must reproduce this;
  already verified for all 11, use only if rebuilding from scratch and unsure.

## The queue, in recommended order

Run top to bottom — ordered by expected value. Each entry: what it is, the
flag (branch default = test arm), what to expect, and the decision.

### 1. `experiment/fix-nondet-50` — Zobrist black-king OOB fix (#50) — PRIORITY
- **What:** `PIECE_NB` 12→13. t22's black king read past the key table: one
  square (f4) got a per-process ASLR value (the Kiwipete nondeterminism);
  squares b1/c1/d1/g4/h4 got constant 0 → positions differing only in that
  black-king placement **hash identically** (real TT collisions in every
  baseline ever). Flag `ENABLE_ZOBRIST_BLACK_KING_ROW` (0 = buggy t22 arm).
- **Class:** latent correctness bug (#44/#45 family) — could be worth real
  Elo; even if ~neutral it ships on soundness (like #43 MDP) because the
  collision class is strictly harmful and the fix costs nothing.
- **Decision:** positive or neutral-with-lean → ship into t23 regardless of
  magnitude (document the reasoning); clearly negative would be shocking —
  re-verify the run before believing it.
- **Follow-up hook:** if it ships, revisit #37 (illegal bestmove) — zobrist
  collisions corrupt TT-move sourcing, one of few mechanisms that fits #37.

### 2. `experiment/see-ordering` — SEE good/bad capture split (#6)
- **What:** in `pick_next_move`, captures with SEE < 0 drop below every quiet
  (−10M + MVV-LVA); SEE ≥ 0 captures stay above killers. Promotions + en
  passant exempt. Flag `ENABLE_SEE_ORDER_SPLIT`.
- **Expect:** startpos d14 tree −52% at fixed depth. SEE cost is paid once per
  node in the scoring pass — the SPRT decides if the node savings beat it at
  fixed time. Prior #6 ("lazy SEE") was ~neutral; this is the full split.
- **Decision:** standard two-machine bar.

### 3. `experiment/razoring-off` — pruning-stack audit probe (#45-audit)
- **What:** razoring (depth 2–4, eval+400<alpha ⇒ depth−1) fully removed.
  Flag `ENABLE_RAZORING` — **INVERTED: default 0 = razoring OFF is the test
  arm; =1 is the t22 arm.** The plain branch build is correct for the bat.
- **Expect:** −12% nodes... at fixed depth (fewer razor-reduced re-searches).
- **Decision:** if OFF wins or is neutral → razoring is dead weight, remove it
  in t23 (fewer, better, sound). If OFF clearly loses → razoring earns its
  keep; park and optionally test a PV-guarded variant later.

### 4. `experiment/rfp-pv-guard` — reverse futility PV guard (#45-audit)
- **What:** RFP fires only at null-window nodes (`beta - alpha == 1`), so it
  can no longer prune an interior PV node. Flag `ENABLE_RFP_PV_GUARD`.
- **Expect:** startpos d14 −34% nodes (sounder PV values → more reusable EXACT
  TT entries shrank the tree — plausible but flattering; trust only the SPRT).
- **Decision:** standard bar. Same #45 leak-class rationale as futility's fix.

### 5. `experiment/futility-depth2` — futility envelope ≤3 → ≤2 (#45 knob a)
- **What:** move-level futility now only at depth ≤ 2 (Fruit trusts only
  depth 1). Flag `ENABLE_FUTILITY_DEPTH2`.
- **Expect:** +20% nodes at fixed depth (prunes less). The bet: the removed
  depth-3 errors are worth more than the lost speed. #45 warned these knobs
  "may have already saturated" — a clean neutral is a fine outcome (park).
- **Decision:** standard bar.

### 6. `experiment/futility-pv-guard` — futility PV guard (#45 knob b)
- **What:** futility exempts ALL PV nodes (`beta - alpha == 1` added), Fruit's
  exact recipe. Flag `ENABLE_FUTILITY_PV_GUARD`. Expect +11% nodes.
- **Decision:** standard bar. Independent of knob (a) — if BOTH win, fold one,
  re-run the other on top before folding it too (they overlap).

### 7. `experiment/tt-aging` — date-based TT aging (#42 idea 1)
- **What:** 6-bit search date packed into `node_type`'s upper bits;
  `new_search()` bumps it per `go`; stale-dated entries evictable regardless
  of depth; probe hits re-date. Flag `ENABLE_TT_AGING`.
- **Expect:** pays most in LONG games (many searches per game) — blitz SPRT
  may understate it. If blitz is neutral-positive, consider one LTC leg
  (60+0.6 vs Stash 17, like the t21 validation) before deciding.
- **Decision:** standard bar, with the LTC caveat above.

### 8. `experiment/drawishness-scaling` — endgame draw scaling (roadmap)
- **What:** final white-positive eval scaled: pure OCB ×64/128; pawnless
  favoured side ahead ≤ a minor ×16/128. Flag `ENABLE_DRAWISHNESS_SCALING`.
  Targets the #5 conversion weakness (draw-heavy vs weaker engines).
- **Expect:** most visible vs WEAKER opponents (conversion), so self-play
  vs t22 may understate it; if neutral, a 200g check vs Stash 11/12 (the
  draw-prone pairing) is the better read.
- **Decision:** standard bar (+ optional external check).

### 9. `experiment/root-twofold-avoid` — root 2-fold avoidance (#44 follow-up)
- **What:** the root winning-repetition clamp now triggers on a SINGLE
  repetition (key already in game history), not just rule-threefold, so a won
  engine routes around the shuffle a move earlier. Inert without game history.
  Flag `ENABLE_ROOT_TWOFOLD_AVOID`.
- **Expect:** small; fires only in won-and-shuffling games. Draw-rate vs t22
  should tick down.
- **Decision:** standard bar; neutral-positive is shippable on soundness
  grounds if the drawn-won-games mechanism is visible in the PGNs.

### 10. `experiment/trapped-bishop` — CPW trapped-bishop locks (#20)
- **What:** six square+pawn locks per side (a7/h7/b8/g8 full = seeds 100/120;
  a6/h6 lighter = 50/60), tapered, tuner-wired (`P_BISHOP_TRAPPED_*`). Flag
  `ENABLE_TRAPPED_BISHOP`.
- **Expect:** SF-class term ≈ 1–3 Elo — likely inconclusive at 1000g. Fine to
  run LAST or batch with a future eval round; a full Texel re-tune with these
  params exposed is the better long-term path.
- **Decision:** standard bar; treat "inconclusive lean-positive" as
  keep-parked-for-tune, not ship.

### 11. `experiment/pext` — BMI2 PEXT sliders (#32) — NO GAUNTLET NEEDED
- **What:** `_pext_u64` table indexing replaces magic multiply; init fills
  tables with pext ordering; `verify_or_die()` validates at startup. Flag
  `ENABLE_PEXT`. **Verified bit-identical** to t22 (both arms 12,035,479).
- **Procedure instead:** on EACH box, build the branch and run a quiet-machine
  interleaved nps A/B vs t22 (e.g. 5× `go depth 15` alternating). If pext is
  faster on both, batch it with the next speed ship (like #48+#49) — it does
  not need its own SPRT slot since behavior is identical.

## Combining the winners → t23 (screen → combine → validate)

1. **Screen** (the queue above): every arm independently vs t22.
2. **Combine:** merge ALL winning branches into one candidate branch
   (`candidate/t23`). The flags make this mechanical — the candidate is just
   "all winning flags ON"; resolve the (textual) flag-block merge conflicts,
   re-run the suite + the startpos signature sanity of each OFF arm.
3. **Validate:** candidate vs t22, two machines, standard bar. **The combined
   measured Elo — not the sum of the individual runs — is t23's delta.**
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
6. Tag `baseline-t23` on the folded main (flip flag defaults / delete losing
   paths per the aggressive-cleanup convention), build + snapshot
   `huginn_t23.exe` per machine, push tag.
7. Update BACKLOG (per-item results + close/park), BASELINE_LADDER.md (t23
   section), CLAUDE.md (baseline bullet). Park losing branches (do NOT
   delete — their commit messages carry the negative-result documentation).
8. If `fix-nondet-50` shipped: re-run the #37 thread with fresh eyes
   (collision-corrupted TT moves are a candidate mechanism), and re-establish
   the Kiwipete d14 signature as a SECOND deterministic verification anchor
   alongside startpos.
