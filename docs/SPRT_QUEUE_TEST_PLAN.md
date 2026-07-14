# SPRT Queue Test Plan — TT aging LTC re-test (#42) candidate off baseline-t33 (OPEN)

> **Run (custom — the gauntlet bat is 10+0.1 only):** `git checkout
> candidate/tt-aging-ltc`, build (branch default ON), then fastchess with
> **tc=60+0.6**, SPRT [0,10], 1t, 64MB, noob_3moves.epd, cc=4, 250-round cap,
> `option.SyzygyPath=c:\TB\` both sides, vs per-machine `huginn_t33.exe`.
> Flag `ENABLE_TT_AGING` (unstick ON THE BRANCH: `cmake -UENABLE_TT_AGING`).
> **⚠ Signature caveat (like #53):** first-search node counts are IDENTICAL on
> both arms BY DESIGN (startpos d14 = 3,481,582 = t33 exact — from a fresh
> table every entry carries the current date, so the age clause can't fire;
> aging acts only from the second search of a process on). Do NOT use startpos
> nodes to verify the arm — the discriminators are the 8 gated TT tests
> (`test_transposition_table.cpp`, cross-search eviction cases) and the
> "TT aging enabled" configure line. 282/283 green on the branch.
> **What:** #42 idea 1, date-based aging (Fruit/Toga): 6-bit search date packed
> in the node_type byte; `new_search()` bumps it per search; stale-dated
> entries evictable regardless of depth; probe hits re-date. Idea-1 blitz
> verdict at t23 was inconclusive (AMD +0.69 flat / Intel +11.12 lean, 1000g
> each) — aging's value should concentrate in long games (more searches per
> game accumulate more staleness), so this LTC leg is the final park/ship
> call (road-to-2.3 item 2).
> **Result:** Intel LTC leg running (2026-07-13) →
> `gauntlet/huginn_vs_t33_ttaging_ltc_intel.pgn`.
> **Decision:** positive → ship as t34 (and queue clusters/idea 2 as a quick
> follow-up); flat/negative at LTC too → definitive park (both TCs agree).

# SPRT Queue Test Plan — history-modulated LMR (#63) candidate off baseline-t32 (CLOSED)

> **QUEUE CLOSED, `baseline-t33` SHIPPED (2026-07-13)** — two-machine SPRT vs
> t32, both legs positive (AMD +8.69 / Intel +18.43), pooled **+13.63 ±
> 10.72, LOS ≈ 99.4%, 2000g**. `ENABLE_HISTORY_LMR` default flipped ON
> (source + CMake); ship build reproduces d14 = 3,481,582 / e2e4. Fourth
> straight selectivity ship (t30→t33 ≈ +60 pooled self-play in four rounds).
> Writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md).

> **Run:** `git checkout candidate/history-lmr` + `test_huginn_gauntlet.bat t32`.
> Flag `ENABLE_HISTORY_LMR` (unstick: `cmake -UENABLE_HISTORY_LMR` — ON THE
> BRANCH, per the #17-r2 AMD-leg warning).
> **t32 baseline (OFF):** startpos d14 = **5,669,691** / cp 33 / e2e4;
> Kiwipete d13 = **2,768,609** / cp −88 / e2a6.
> **#63 arm (ON):** startpos d14 = **3,481,582** / cp 31 / e2e4 (−38.6% —
> ⚠ root move does NOT discriminate, node count does); Kiwipete d13 =
> **1,958,182** / cp −85 / e2a6 (−29.3%; the SPRT decides the fixed-time
> trade).
> **What:** history-modulated LMR (road-to-2.3 item 1; third leg of the
> SF18-study selectivity program after #62 + #17-r2). At the LMR site the
> mover's butterfly-history score adjusts the static table reduction ±1 ply
> (grain ±4096; `[1, depth−2]` clamps still apply): proven-good quiets are
> reduced less, history-hated quiets more. Fixture:
> `info.history_lmr_adjusts > 0` in a d12 Kiwipete search (test
> `HistoryLmr.ModulationFiresInDeepSearch` tracks the built arm); counter
> stone-dead on the baseline arm. Both arms 274/275 green (1 by-design skip).
> **Result — BOTH LEGS DONE, same-sign positive, pooled clears the bar:**
> - **AMD (2026-07-13):** **+8.69 ± 15.27** (nElo 12.26), LOS 86.78%, 1000g
>   round cap (LLR 0.60, no bound), 51.25% (W269/L244/D487), DrawRatio
>   39.00%, PairsRatio 1.15, Ptnml [31,111,195,128,35]. Clean run (zero
>   forfeits/illegal/disconnects, 1h53m). Arm verified pre-run (test d14 =
>   3,481,582 exact + configure line, unstick run on the branch). Softest
>   AMD leg of the selectivity series.
>   PGN `gauntlet/huginn_vs_t32_historylmr_amd.pgn`.
> - **Intel (2026-07-13):** **+18.43 ± 15.05** (nElo 26.43), **LOS 99.19%**,
>   1000g round cap (LLR 1.76, no bound), 52.65% (W278/L225/D497), DrawRatio
>   37.20%, PairsRatio 1.34, Ptnml [26,108,186,147,33]. Clean run, 1h51m.
>   PGN `gauntlet/huginn_vs_t32_histlmr_intel.pgn`.
> - **Pooled (inverse-variance): +13.63 ± 10.72, LOS ≈ 99.4%, 51.95% / 2000g**
>   (W547/L469/D984, Ptnml [57,219,381,275,68]). Same ship profile as the
>   whole selectivity series — threats-r2 +17.0 / 99.9%, #62 +14.90 / 99.7%,
>   #17-r2 +14.46 / 99.6% — with the widest per-leg spread so far (AMD 86.78%
>   / Intel 99.19%, both positive; #58-precedent agreement holds).
> **Verdict: SHIPPED as `baseline-t33` (2026-07-13)** — standard two-machine
> ship bar cleared; `ENABLE_HISTORY_LMR` default flipped ON on `main` (source
> `#ifndef` + CMake option — both), tagged, `huginn_t33.exe` snapshotted.

# SPRT Queue Test Plan — aspiration windows re-test (#17-r2) candidate off baseline-t31 (CLOSED)

> **QUEUE CLOSED, `baseline-t32` SHIPPED (2026-07-13)** — two-machine SPRT vs
> t31, both legs positive (AMD +12.51 / Intel +16.34), pooled **+14.46 ±
> 10.61, LOS ≈ 99.6%, 2000g**. `ENABLE_ASPIRATION` default flipped ON (source +
> CMake); ship build reproduces d14 = 5,669,691 / e2e4; 271/272 green. The t15
> attempt-1 rejection is formally superseded (contaminated verdict, #45
> precedent). Writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md).

> **Run:** `git checkout candidate/aspiration-r2` + `test_huginn_gauntlet.bat t31`.
> Flag `ENABLE_ASPIRATION` (unstick: `cmake -UENABLE_ASPIRATION`).
> **t31 baseline (OFF):** startpos d14 = **6,583,846** / cp 24 / e2e4;
> Kiwipete d13 = **3,442,234** / cp −75 / e2a6.
> **#17-r2 arm (ON):** startpos d14 = **5,669,691** / cp 33 / e2e4 (−13.9% —
> ⚠ root move does NOT discriminate, node count does); Kiwipete d13 =
> **2,768,609** / cp −88 / e2a6 (−19.6% — same best move, cheaper tree at
> fixed depth; the SPRT decides the fixed-time trade).
> **What:** aspiration windows at the root — RE-TEST. Attempt 1 (`228817b`)
> was H0-rejected at t15 (−33.8 ± 18.0 AMD, LOS 0.01%) and reverted
> (`df4ccdb`) — but that verdict predates every soundness fix that followed:
> #44 repetition blindness (t17), #50 Zobrist OOB corrupting TT scores (t23),
> #52 check-blind qsearch (t26), #57 broken PVS re-search condition (t27),
> #58 SEE pin legality (t28) — all direct sources of the inter-iteration
> score instability that makes aspiration thrash (contaminated-verdict
> re-test, #45 precedent). Measured on t31 pre-implementation: startpos
> swings ≤ 9cp/iteration, Kiwipete ≤ 31cp from d6 on — a ±50cp window holds.
> Design: from depth ≥ 6 (attempt 1 opened at 4, inside the measured noisy
> zone), window `[prev−50, prev+50]`; a fail-low/high widens that side ×2
> around `best_score`; delta > 800 or a mate-range centre (|score| ≥ 27000)
> snaps to the full window. `info.aspiration_researches` counts window fails.
> Fixture: counter > 0 in a d8 Kiwipete search (the measured d5→d6 −57cp
> swing must fail the first window) — test
> `Aspiration.WindowFailuresFireInDeepSearch` tracks the built arm; counter
> stone-dead on the baseline arm. Both arms 271/272 green (1 by-design skip).
> **Result — BOTH LEGS DONE, same-sign positive, pooled clears the bar:**
> - **AMD (2026-07-13):** **+12.51 ± 15.12** (nElo 17.84), LOS 94.78%, 1000g
>   round cap (LLR 1.07, no bound), 51.80% (W261/L225/D514), Ptnml
>   [22,123,192,123,40]. Clean run, 1h51m. Arm verified pre-run (test d14 =
>   5,669,691 exact — NOTE: run the cache unstick ON THE BRANCH; an off-branch
>   `-U` bakes the main default OFF into the cache and silently builds a
>   baseline-vs-baseline match). PGN `gauntlet/huginn_vs_t31_aspiration_amd.pgn`.
> - **Intel (2026-07-13):** **+16.34 ± 14.88** (nElo 23.70), **LOS 98.45%**,
>   1000g round cap (LLR 1.53, no bound), 52.35% (W273/L226/D501), DrawRatio
>   36.60%, PairsRatio 1.31, Ptnml [25,112,183,151,29]. Clean run, 1h52m.
>   PGN `gauntlet/huginn_vs_t31_aspiration_intel.pgn`.
> - **Pooled (inverse-variance): +14.46 ± 10.61, LOS ≈ 99.6%, 52.08% / 2000g**
>   (W534/L451/D1015, Ptnml [47,235,375,274,69]). Same ship profile as #62
>   (+14.90 / 99.7%) and threats-r2 (+17.0 / 99.9%).
> **Verdict: clears the standard two-machine ship bar — SHIP as
> `baseline-t32`** (flip `ENABLE_ASPIRATION` default ON, source `#ifndef` +
> CMake option). The 2026-04 attempt-1 rejection is formally superseded: with
> the soundness stack fixed, aspiration converts exactly as the
> contaminated-verdict hypothesis predicted. User's call to pull the trigger.

# SPRT Queue Test Plan — singular extensions (#62) candidate off baseline-t30 (CLOSED)

> **QUEUE CLOSED, `baseline-t31` SHIPPED (2026-07-13)** — two-machine SPRT vs
> t30, both legs positive (AMD +12.17 / Intel +17.39), pooled **+14.90 ± 10.62,
> LOS ≈ 99.7%, 2000g**. `ENABLE_SINGULAR_EXT` default flipped ON (source +
> CMake); ship build reproduces d14 = 6,583,846 / e2e4. First search-shape ship
> since t27. Writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md).

> **Run:** `git checkout candidate/singular-ext` + `test_huginn_gauntlet.bat t30`.
> Flag `ENABLE_SINGULAR_EXT` (unstick: `cmake -UENABLE_SINGULAR_EXT`).
> **t30 baseline (OFF):** startpos d14 = **8,298,375** / cp 26 / e2e4;
> Kiwipete d13 = **1,846,915** / cp −83 / e2a6.
> **#62 arm (ON):** startpos d14 = **6,583,846** / cp 24 / e2e4 (−20.7% —
> ⚠ root move does NOT discriminate, node count does); Kiwipete d13 =
> **3,442,234** / cp −75 / e2a6 (+86%, seldepth 30 vs 25 — forced lines
> verified deeper; the SPRT decides the fixed-time trade).
> **What:** singular extensions (the SF18-gap-study EBF lever, EBF 1.90 →
> toward 1.37): at depth ≥ 8, non-root, non-check, with a LOWER_BOUND/EXACT
> non-mate TT score at depth ≥ depth−3, a reduced-depth `(depth−1)/2`
> exclusion search of every OTHER move at a null window below
> `tt_score − 2·depth`; fail-low ⇒ the TT move is singular ⇒ +1 ply. At
> exclusion nodes: no TT cut/store, no null-move, no PV write; only-legal-move
> ⇒ fail low (not mate). Fixture: `info.singular_exts > 0` in a d10 Kiwipete
> search (test `SingularExt.ExtensionsFireInDeepSearch` tracks the built arm).
> **Result — BOTH LEGS DONE, same-sign positive, pooled clears the bar:**
> - **AMD (2026-07-12):** **+12.17 ± 15.39** (nElo 17.05), LOS 93.97%, 1000g
>   round cap (LLR 0.99, no bound), 51.75% (W290/L255/D455), DrawRatio
>   39.20%, PairsRatio 1.20, Ptnml [31,107,196,128,38]. Clean run (zero
>   forfeits/illegal/disconnects, 1h50m).
>   PGN `gauntlet/huginn_vs_t30_singular_amd.pgn`.
> - **Intel (2026-07-12):** **+17.39 ± 14.67** (nElo 25.59), **LOS 99.01%**,
>   1000g round cap (LLR 1.70, no bound), 52.50% (W266/L216/D518), DrawRatio
>   38.00%, PairsRatio 1.25, Ptnml [18,120,190,138,34]. Clean run, 1h49m.
>   PGN `gauntlet/huginn_vs_t30_singular_intel.pgn`.
> - **Pooled (inverse-variance): +14.90 ± 10.62 Elo, LOS ≈ 99.7%, 2000g,
>   52.13%** (W556/L471/D973, Ptnml [49,227,386,266,72]). Both legs positive;
>   neither crossed the SPRT LLR bound before the 1000g cap, but the pooled
>   two-machine estimate clears the strict 95% LOS bar decisively — same
>   shape as the threats-r2 ship (pooled +17.0 ± 10.4).
> Arms verified pre-run on each box (test d14 = 6,583,846; baseline
> huginn_t30.exe d14 = 8,298,375; AMD also MD5-matched huginn_t30.exe to
> the t30 release build).
> **Verdict: SHIPPED as `baseline-t31` (2026-07-13)** — standard two-machine
> ship bar cleared; `ENABLE_SINGULAR_EXT` default flipped ON on `main` (source
> `#ifndef` + CMake option — both), tagged, `huginn_t31.exe` snapshotted.

# SPRT Queue Test Plan — threats round 2 (#9) candidate off baseline-t29 (CLOSED)

> **QUEUE CLOSED, `baseline-t30` SHIPPED (2026-07-12)** — two-machine SPRT vs
> t29, both legs positive (AMD +9.73 / Intel +24.01), pooled **+17.0 ± 10.4,
> LOS ≈ 99.9%, 2000g**. `ENABLE_THREATS_R2` default flipped ON (source + CMake);
> ship build reproduces d14 = 8,298,375 / e2e4; 265 tests green. Largest
> eval-term ship since t15. Writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md).

> **Run:** `git checkout candidate/threats-r2` + `test_huginn_gauntlet.bat t29`.
> Flag `ENABLE_THREATS_R2` (unstick: `cmake -UENABLE_THREATS_R2`).
> **t29 baseline (OFF):** startpos d14 = **5,485,978** / cp 26 / e2e4;
> Kiwipete d13 = **1,902,821** / cp −63 / e2a6.
> **Threats-R2 arm (ON):** startpos d14 = **8,298,375** / cp 26 / e2e4
> (+51% fixed-depth nodes — the new terms reshape the tree, PV switches to a
> French line; nps −3.5% from the extra attack unions; the SPRT decides the
> fixed-time trade, cf. #57's +12.8%); Kiwipete d13 = **1,846,915** /
> cp −83 / e2a6 (−2.9%).
> **What:** three eval threat classes layered on t15 — hanging units
> (attacked + undefended), safe pawn-push threats, hanging units in our
> king's ring. Params Texel-fitted `--only-new` on the 725k quiet corpus
> (K=1.520, rest of the vector frozen so the OFF arm stays byte-identical
> to t29): HANGING 9/18, PAWN_PUSH 10/4, BY_KING −5/34 (MG/EG).
> **New-feature ΔMSE = 0.056857 → 0.056459 (−0.000398)** — ~1.6× the t16
> king-safety round's delta; house note: new-feature MSE converts to Elo
> better than re-fit MSE, and quiet-corpus MSE *under*-states terms that pay
> in sharp positions (t16 corollary). SPRT decides.
> **Result — BOTH LEGS DONE, same-sign positive, pooled clears the bar:**
> - **AMD (2026-07-12):** **+9.73 ± 14.88** (nElo 14.09), LOS 90.02%, 1000g
>   round cap (LLR 0.75, no bound), 51.40% (W268/L240/D492), Ptnml
>   [29,105,208,125,33]. Clean run (zero forfeits/illegal/disconnects, 1h48m).
>   PGN `gauntlet/huginn_vs_t29_threatsr2_amd.pgn`.
> - **Intel (2026-07-12):** **+24.01 ± 14.55** (nElo 35.68), **LOS 99.94%**,
>   1000g round cap (LLR 2.54, just short of +2.94), 53.45% (W284/L215/D501),
>   DrawRatio 39.60%, PairsRatio 1.36, Ptnml [15,113,198,136,38]. Clean run,
>   1h49m. PGN `gauntlet/huginn_vs_t29_threatsr2_intel.pgn`.
> - **Pooled (inverse-variance): +17.03 ± 10.40 Elo, LOS ≈ 99.9%, 2000g,
>   52.43%** (W552/L455/D993, Ptnml [44,218,406,261,71]). Both legs positive;
>   neither crossed the SPRT LLR bound before the 1000g cap, but the pooled
>   two-machine estimate clears even the strict 95% LOS bar decisively.
> Both runs used the TB-parity fix (explicit `option.SyzygyPath` both sides —
> #56 removed the c:\TB auto-probe). Arms verified pre-run on each box
> (test d14 = 8,298,375; baseline huginn_t29.exe d14 = 5,485,978).
> **Verdict: SHIP as `baseline-t30`** (largest eval-term ship since t15's
> threats round 1) — flip `ENABLE_THREATS_R2` default ON on `main` (source +
> CMake), merge, tag, snapshot `huginn_t30.exe`. User's call to pull the trigger.

---

# SPRT Queue Test Plan — #58 candidate off baseline-t27 (CLOSED)

> **QUEUE CLOSED, `baseline-t28` SHIPPED (2026-07-11)** on cross-machine
> agreement (AMD +5.56 / Intel +8.69, pooled +7.2 ± 10.5, LOS ≈ 91%) plus
> correctness (#53 precedent) — user call. Flag default ON on `main`.

> **Run:** `git checkout candidate/see-legality` + `test_huginn_gauntlet.bat t27`.
> Flag `ENABLE_SEE_LEGALITY` (unstick: `cmake -UENABLE_SEE_LEGALITY`).
> **t27 baseline (OFF):** startpos d14 = **7,484,807** / cp 23 / d2d4.
> **#58 arm (ON):** startpos d14 = **7,128,502** / cp 30 / **e2e4** (root move
> discriminates). What: SEE's FIRST recapture skips defenders absolutely
> pinned to their own king (pin-line captures still count); deeper plies stay
> geometric. Fixture: see(Qg4xf5) >= 0 in 4k3/4n3/8/5p2/6Q1/8/8/4R1K1 w on
> the test arm (~-800 baseline) — test SeePinnedDefenderRecapture tracks the
> built arm.
> **Result — BOTH LEGS DONE, same-sign small positive, tight agreement:**
> - **AMD (2026-07-11):** **+5.56 ± 15.11** (nElo 7.93), LOS 76.48%, 1000g round
>   cap (LLR 0.24, no bound), 50.80% (W257/L241/D502), Ptnml [29,118,194,126,33].
>   PGN `gauntlet/huginn_vs_t27_see_amd.pgn`.
> - **Intel (2026-07-11):** **+8.69 ± 14.65** (nElo 12.78), LOS 87.77%, 1000g
>   round cap (LLR 0.64, no bound), 51.25% (W257/L232/D511), Ptnml
>   [22,125,187,138,28]. PGN `gauntlet/huginn_vs_t27_see_intel.pgn`.
> - **Pooled (inverse-variance): ≈ +7.2 ± 10.5 Elo, LOS ≈ 91%, 2000g, 51.03%**
>   (W514/L473/D1013, Ptnml [51,243,381,264,61]). Same profile as #53: a real
>   tactical-soundness fix that blitz self-play prices near zero — but the sign
>   is positive on BOTH boxes.
> **Decision (user's call):** neither leg crossed the SPRT bound, but the two
> agree in sign and pool to LOS ≈ 91% — clears the cross-machine-agreement ship
> bar used for small effects (#15 precedent, shipped at 91%), and it's a
> correctness fix besides (#53 precedent). Reasonable to ship as `baseline-t28`
> (flip `ENABLE_SEE_LEGALITY` default ON, source + CMake) or park; not a
> park-for-regression case since the sign is positive.

# SPRT Queue Test Plan — #57 candidate off baseline-t26 (CLOSED)

> **QUEUE CLOSED, `baseline-t27` SHIPPED (2026-07-11)** on the AMD leg's
> H1-accept (+29.98 ± 15.53, LOS 99.99%, 976g) — AMD-only ship by explicit
> user call (#51 precedent, no Intel leg). Flag default ON on `main`;
> writeup in [BASELINE_LADDER.md](BASELINE_LADDER.md).

> **Purpose:** run-sheet for the BACKLOG #57 candidate branch created
> 2026-07-11 (fix on `main` behind `ENABLE_LEGAL_MOVE_ORDINAL`, default OFF,
> flag-off byte-identical to `baseline-t26`; the branch flips the default ON
> so `test_huginn_gauntlet.bat` builds the test arm from a plain checkout).

## Ground rules

- **Baseline:** `baseline-t26` (== `main` with the flag OFF). Opponent binary
  `huginn_t26.exe` built per-machine from the tag as usual (already
  snapshotted on the AMD box).
- **Run command (each box):**
  ```
  git fetch origin
  git checkout candidate/legal-move-ordinal
  test_huginn_gauntlet.bat t26
  ```
  Standard SPRT [0,10], 10+0.1, 1t, 64MB, noob_3moves.epd, cc=4, artifacts
  tagged `_intel`/`_amd` (rename to `..._ordinal_<machine>` after the run).
- **⚠ Cache trap as always:** `ENABLE_LEGAL_MOVE_ORDINAL` is a CMake
  `option()` — verify the arm before EVERY gauntlet with the signatures
  below; unstick with `cmake -UENABLE_LEGAL_MOVE_ORDINAL` + reconfigure.

## Reference signatures (1 thread, OwnBook=false, fresh process, 64MB hash)

**t26 baseline (flag OFF) — any OFF arm must reproduce these exactly:**

- startpos `go depth 14`: **nodes = 6,634,033**, score cp 27, bestmove e2e4
- Kiwipete `go depth 13`: **nodes = 2,047,460**, score cp −63, bestmove e2a6

**#57 test arm (flag ON):**

- startpos `go depth 14`: **nodes = 7,484,807**, score cp 23, bestmove
  **d2d4** (+12.8% fixed-depth nodes AND a different root choice — proper
  PVS re-searches cost nodes when ordering is imperfect; the SPRT decides
  the fixed-time trade)
- Kiwipete `go depth 13`: **nodes = 1,930,694**, score cp −63, bestmove e2a6
  (−5.7%)
- **Instant discriminator:** the startpos d14 bestmove alone separates the
  arms (test = d2d4, baseline = e2e4).

### `candidate/legal-move-ordinal` — legal-move ordinal PVS/LMR (#57)

- **What:** flag `ENABLE_LEGAL_MOVE_ORDINAL`. The move loop's pseudo-legal
  index `i` decided PVS first-move treatment, LMR lateness/row, and fhf —
  illegal (pinned) entries inflate it, so the first LEGAL move could get a
  null-window/reduced search. Separately the historical PVS condition
  `i == 0 || alpha == best_score` made every move after a normal alpha
  improvement full-window (null-window PVS only engaged in failing-low
  nodes). Flag ON: a searched-move ordinal (incremented only after a
  successful MakeMove) drives all three, and PVS is textbook — first legal
  move full-window, everything else null-window + fail-high re-search.
- **Expect:** genuine search-shape change (same soundness family as
  #44/#45/#52). Fixed-depth nodes moved both directions (startpos up,
  Kiwipete down); fixed-time strength is the SPRT's call.
- **Result (AMD, 2026-07-11) — H1 ACCEPT:** **+29.98 ± 15.53** (nElo 42.31 ±
  21.80), LOS 99.99%, 976g (early-stop, LLR 2.97 crossed +2.94), 54.30%
  (W288/L204/D484), Ptnml(0-2) [22,99,184,139,44], PairsRatio 1.51,
  DrawRatio 37.70%. Arms verified pre-run (test d14 = 7,484,807 / d2d4;
  baseline huginn_t26.exe d14 = 6,634,033 / e2e4). PGN
  `gauntlet/huginn_vs_t26_ordinal_amd.pgn`. **Intel leg pending — do not
  ship on this leg alone.**
- **Decision:** standard two-machine ship bar. If it ships, flip the flag
  default ON on `main` (source + CMake option — both).

---

# Historical: SPRT Queue — 2026-07-09 audit candidates off baseline-t25 (CLOSED)

> **QUEUE CLOSED, `baseline-t26` SHIPPED (2026-07-10).** Both candidates have
> final two-machine verdicts and both shipped in `baseline-t26`: **#52** on a
> same-sign two-machine H1-accept (Intel +40.11 / AMD +44.67, pooled ≈ +42
> Elo / 1306g); **#53** on correctness+tests by explicit user call (#50/#51
> precedent) despite a ~neutral-to-slightly-negative blitz sign-split (Intel
> −18.08 / AMD +5.21, pooled ≈ −6 Elo / 2000g). Both flags now default ON on
> `main`; ship verification + writeup in
> [BASELINE_LADDER.md](BASELINE_LADDER.md). Per-leg numbers below retained as
> the procedure record.

> **Purpose:** self-contained run-sheet for the two BACKLOG #52/#53 candidate
> branches created 2026-07-09 (commit `c9f190a` on `main` + one flag-flip
> commit per branch). Written so either gauntlet box can run it with no chat
> context. Both fixes are already IN `main` behind flags (default OFF,
> flag-off byte-identical to `baseline-t25`); the branches only flip the flag
> default so `test_huginn_gauntlet.bat` builds the test arm from a plain
> checkout. SPRT each branch SEPARATELY; winners combine per the t24-queue
> methodology (combined candidate + undershoot guard) below.

## Ground rules

- **Baseline:** `baseline-t25` (== `main`@`c9f190a` with both flags OFF).
  Opponent binary `huginn_t25.exe` built per-machine from the tag as usual.
- **Run command (each box):**
  ```
  git fetch origin
  git checkout candidate/qsearch-check-evasions   (or candidate/rule50-tt-guard)
  test_huginn_gauntlet.bat t25
  ```
  The branch's flag default IS the test arm. Standard SPRT [0,10], 10+0.1,
  1t, 64MB, noob_3moves.epd, cc=4, artifacts tagged `_intel`/`_amd`.
- **⚠ Cache trap (see the t23-queue section below for the war story):** both
  flags are CMake `option()`s, so a build directory configured on a candidate
  branch keeps the flag **ON in `CMakeCache.txt` after switching back to
  `main`** (and vice versa). Verify the arm before EVERY gauntlet with the
  signatures below; unstick a poisoned cache with
  `cmake -UENABLE_QSEARCH_CHECK_EVASIONS -UENABLE_RULE50_TT_GUARD` +
  reconfigure, or delete the build dir.

## Reference signatures (1 thread, OwnBook=false, fresh process, 64MB hash)

**t25 baseline (both flags OFF) — any OFF arm must reproduce these exactly:**

- startpos `go depth 14`: **nodes = 8,406,631**, score cp 22, bestmove d2d4
- startpos `go depth 12`: **nodes = 2,300,322**, score cp 21, bestmove d2d4
- Kiwipete `go depth 13`: **nodes = 1,868,991**, score cp −88, bestmove e2a6

## The queue, in recommended order

### 1. `candidate/qsearch-check-evasions` — check-aware qsearch (#52)

- **What:** flag `ENABLE_QSEARCH_CHECK_EVASIONS`. Qsearch detects check at
  entry; in check it skips stand-pat/delta/SEE and searches every evasion,
  returning `-MATE + ply` when none is legal; out of check the frontier gains
  quiet promotions; `info.ply` advances through qsearch (true mate distance +
  seldepth). Same soundness class as #44/#45 (stop being blind at the
  horizon), which is why it runs first.
- **Test-arm signature:** startpos d14 **nodes = 6,634,033**, cp 27, bestmove
  e2e4 (−21% fixed-depth nodes vs baseline, but nps is lower — evasion
  movegen + no stand-pat cutoff in check; the SPRT decides the fixed-time
  trade). Kiwipete d13 **nodes = 2,047,460**, cp −63, bestmove e2a6.
- **Instant discriminator:** `position fen 7k/8/5KQ1/8/8/8/8/8 w - - 0 1` +
  `go depth 1` → test arm answers **`score mate 1 ... pv g6g7`**; baseline
  arm answers `cp 1277 / g6g5`.
- **Result (Intel, 2026-07-09) — H1 ACCEPT:** **+40.11 ± 18.18** (nElo 57.51 ±
  25.81), LOS 100%, 696g, W226/L146/D324 = 55.75%, LLR 2.95 (crossed +2.94).
  Ptnml(0-2) [12, 69, 126, 109, 32], PairsRatio 1.74, DrawRatio 36.21%. Arms
  verified pre-run (baseline d14 = 8,406,631 / test d14 = 6,634,033 / mate-in-1
  discriminator `mate 1 g6g7`). PGN
  `gauntlet/huginn_vs_t25_qsearch_intel.pgn`.
- **Result (AMD, 2026-07-10) — H1 ACCEPT:** **+44.67 ± 18.94** (nElo 65.80 ±
  27.57), LOS 100%, 610g, W188/L110/D312 = 56.39%, LLR 2.97 (crossed +2.94).
  Ptnml(0-2) [7, 66, 97, 112, 23], PairsRatio 1.85, DrawRatio 31.80%. Arms
  verified pre-run (test d14 = 6,634,033 / cp 27 / e2e4; mate-in-1
  discriminator `mate 1 g6g7`; baseline t25 d14 = 8,406,631 re-verified same
  session). PGN `gauntlet/huginn_vs_t25_qsearch_amd.pgn`.
- **Two-leg verdict (final, 2026-07-10): SHIP BAR MET** — same-sign
  two-machine H1-accept (Intel +40.11 / AMD +44.67, remarkably consistent).
  Pooled W414/L256/D636 = 56.05% over 1306g ≈ +42 Elo.
- **Decision:** two-machine ship bar met. To ship: flip the flag default ON
  on `main` (source + CMake option + test mirror — all three), verify the
  flag-ON build reproduces the test-arm signature (d14 = 6,634,033 +
  `mate 1 g6g7`), then tag per the baseline ladder process.

### 2. `candidate/rule50-tt-guard` — rule-50-aware TT eligibility (#53)

- **What:** flag `ENABLE_RULE50_TT_GUARD`. No TT cutoff and no TT store when
  `halfmove_clock + depth >= 100` (subtree can reach the fifty-move boundary,
  so scores are path-dependent); TT move still used for ordering.
- **Expect:** ~neutral at 10+0.1 — the guard fires only near the boundary,
  which blitz self-play rarely reaches. The value is correctness in long
  shuffle endgames (a #5-class conversion concern). If blitz is flat,
  consider one LTC leg (60+0.6, like #42's plan) or shipping on
  correctness+tests alone (#50/#51 precedent) — user's call.
- **⚠ Signature caveat:** the test arm is startpos-signature-IDENTICAL to
  baseline **by design** (clock 0 + depth ≤ 14 never reaches the boundary,
  so d14 = 8,406,631 on BOTH arms). Do NOT use startpos nodes to verify this
  arm.
- **Instant discriminator (same process, two searches):**
  `position fen k7/8/8/8/8/8/7Q/7K w - - 98 1` + `go depth 2` (both arms:
  ~cp 25, h2c7), then `position fen k7/8/8/8/8/8/7Q/7K w - - 0 1` +
  `go depth 2` → test arm answers **`cp 1211 / h2d6`** (fresh-oracle value);
  baseline arm answers the TT-poisoned `cp 25 / h2c7`.
- **Result (Intel, 2026-07-10) — mild REGRESSION (not neutral):** **−18.08 ±
  15.32** (nElo −25.48 ± 21.53), LOS 1.02%, 1000g (SPRT hit the round cap,
  LLR −2.51 — leans H0/reject, no bound crossed), 47.40% (W217/L269/D514),
  Ptnml(0-2) [39,139,181,117,24], PairsRatio 0.79. Arms verified pre-run via
  the clock discriminator (test → `cp 1211 / h2d6`; baseline → `cp 25 / h2c7`).
  The guard trades away TT cutoffs near the boundary that blitz benefits from;
  the correctness payoff is in long shuffle endgames this TC rarely reaches.
  PGN `gauntlet/huginn_vs_t25_rule50_intel.pgn`.
- **Result (AMD, 2026-07-10) — inconclusive / ~neutral:** **+5.21 ± 14.37**
  (nElo 7.82 ± 21.53), LOS 76.16%, 1000g (round cap, LLR 0.23 — no bound
  crossed), 50.75% (W250/L235/D515), Ptnml(0-2) [23,119,205,126,27],
  PairsRatio 1.08, DrawRatio 41.00%. Arms verified pre-run: clock
  discriminator (test → `cp 1211 / h2d6`; baseline → `cp 25 / h2c7`), both
  arms startpos d14 = 8,406,631 (identical by design), and the mate-in-1
  probe answered `cp 1277 / g6g5` on both (no qsearch-flag contamination).
  PGN `gauntlet/huginn_vs_t25_rule50_amd.pgn`.
- **Two-leg picture (final, 2026-07-10):** Intel −18.08 / AMD +5.21 — the
  machines disagree in sign; pooled W467/L504/D1029 = 49.08% over 2000g
  (≈ −6.4 Elo). Net blitz read: neutral-to-slightly-negative. NOT an Elo
  win — do not combine as a baseline winner.
- **Decision:** blitz legs complete on both machines; verdict recorded in
  BACKLOG #53. Open options (user's call): park at the blitz cost, run an
  LTC leg (60+0.6) where the boundary actually gets hit, or ship on
  correctness+tests alone (#50/#51 precedent) accepting the small blitz cost.

---

# Historical: SPRT Queue — 10 branches off baseline-t23 (CLOSED)

> **QUEUE CLOSED, `baseline-t24` SHIPPED (2026-07-04).** All 11 items (the 10
> branches below + #50) have final two-machine verdicts. **Winners:** #50
> (already in `main`/t23), `see-ordering` (#6), `root-twofold-avoid` (#44 f/u).
> Everything else parked/rejected — see the per-item status below and
> [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) for full numbers (this
> whole queue is now closed history — see the lean [BACKLOG.md](BACKLOG.md)
> for what's currently open).
>
> **Combined-candidate two-machine SPRT vs t23 — both legs H1-ACCEPT:** AMD
> **+48.84 ± 20.36** (LOS 100%, 580g, LLR 2.98); Intel **+66.33 ± 23.61** (LOS
> 100%, 440g, LLR 2.97). Pooled W345/L181/D494 = **58.04% / 1020g**.
> Undershoot guard clean on both legs — AMD sits at `see-ordering`'s solo
> number (healthy sub-additivity), Intel exceeds it (apparent positive
> synergy); neither pattern is a red flag. `candidate/t24` was merged into
> `main`, tagged `baseline-t24`, and `huginn_t24.exe` snapshotted. Full
> writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md). This file is retained as
> the historical procedure record for how the queue was run.

> **Purpose:** self-contained run-sheet for gauntleting the experiment branches
> created 2026-07-02. Written so it can be followed with no chat context.
> Companion state now lives in [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md)
> ("SPRT queue" section + per-item sections, since the queue is closed); this
> file is the *procedure*.
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
