# BACKLOG (Huginn 2.1+)

> **Lean working backlog** — open and in-progress items only, plus a deferred-
> ideas list. Full closed-item history lives in two archives:
> [BACKLOG-archive-2.0.md](BACKLOG-archive-2.0.md) (2.0-era, issues #1–#37) and
> [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) (2.1/2.2-era, t16→t24: the
> #41 calibration study, #43 NMP round, #44 repetition fix, #45 futility fix +
> pruning-stack audit, #46–#50, and the 2026-07 SPRT queue that produced
> `baseline-t24`). Issue numbers are preserved so cross-references into either
> archive still resolve.
>
> **Current baseline + strength:** see [CLAUDE.md](CLAUDE.md) (top-level
> status) and [BASELINE_LADDER.md](BASELINE_LADDER.md) (full per-baseline
> writeup, newest first) — not duplicated here.
>
> **Independent code audit (2026-07-09, `348a6ff`):** issues #52–#61 below
> were found by reviewing the position/movegen, search/eval, UCI, and build/test
> paths. The release suite discovered 205 tests (204 passed, 1 skipped), but the
> audit also confirmed that the documented `check` target currently runs **zero**
> tests and exits successfully (#60). Issue #51 is already assigned to the
> `baseline-t25` history-index fix in [BASELINE_LADDER.md](BASELINE_LADDER.md),
> so new IDs start at #52.

## Road to v2.3 (release checklist — runnable from either box, no chat context)

> **Version roadmap (agreed 2026-07-13):** **v2.3** = the recalibrated
> classical-engine peak (this checklist); **v2.4** = Lazy SMP (infrastructure,
> same eval — note SMP breaks fixed-depth signature ship-discipline, plan a
> new verification method); **v3.0** = NNUE (identity change: trained net
> ships with the binary — deliberately major-versioned). Don't fold SMP or
> NNUE work into 2.3.

**Done (items 1–4, 2026-07-13 → 07-15):**
1. ✅ History-modulated LMR → **`baseline-t33`** (pooled +13.63, LOS ≈ 99.4%).
2. ✅ TT aging → **`baseline-t34`** (LTC-verdict ship, +15.99 @ 60+0.6);
   clusters follow-up parked two-TC-flat — **#42 closed**.
3. ✅ LMP re-test → **re-parked** on a two-machine sign-split (pooled −0.87 ±
   10.33) — **#7 closed** with the two-era verdict.
4. ✅ Hash sweep → **flat on both boxes**, default stays 64MB — **#31 closed**.

**Remaining:**

5. **Hygiene (no gauntlet needed):**
   - **Tactical sweep on the t34 build**: WAC300 + LCT2 @ 5s/position
     ([test/WAC300.epd](../test/WAC300.epd), [test/lct2.epd](../test/lct2.epd)).
     Reference: **270/300 WAC @ 5s on t9** — 25 baselines stale; record the
     new number in [SEARCH_AND_EVAL.md](SEARCH_AND_EVAL.md) as the fresh
     tactical reference (expect a large jump; investigate any WAC position
     that REGRESSED vs t9 before shrugging).
   - **#60 leftovers**: parser-purity test refactor + randomized make/unmake
     invariants (see the #60 section below).
   - **CHANGELOG_2.3.md**: the t22 → t34 ladder (v2.2 stopped at t21) — one
     row per baseline with the shipped feature + pooled result; mirror
     [CHANGELOG_2.2.md](CHANGELOG_2.2.md)'s format. Sources:
     [BASELINE_LADDER.md](BASELINE_LADDER.md).
   - #37 (board-desync) stays open — guarded at the UCI boundary, does NOT
     gate the release.
6. **#5 recalibration → tag `v2.3`:**
   - Build the release binary from the final baseline tag (t34 unless
     something ships meanwhile) per-machine as usual.
   - Calibration matches, mirroring the t26 measurement: `stash19` + `stash20`
     via `test_huginn_gauntlet.bat` (cc=1, 500g per anchor, 10+0.1, no book);
     inverse-variance pool the two pins. t26 measured **~2571 ± 19**; the
     t27→t34 ladder adds ≈ +105 self-play (× ~0.86 external compression ≈
     +90) → **expect ~2650–2670**.
   - ⚠ If Huginn scores > ~65% vs Stash 20.0.1 (2509), the anchor is
     saturating — add a stronger rung (Stash 21+ or another CCRL ~2650–2750
     UCI-clean engine) to the fastchess dir + the bat's calibration table
     before trusting the pin.
   - Then: tag `v2.3`, update [CLAUDE.md](CLAUDE.md) strength line +
     [BASELINE_LADDER.md](BASELINE_LADDER.md) + CHANGELOG_2.3, and snapshot
     the release build.

## Open / in-progress

| # | Title | Status | Type | Priority |
|---|-------|--------|------|----------|
| 52 | Check-aware qsearch + horizon terminal states | **SHIPPED (2026-07-10, `baseline-t26`)** — two-machine H1-ACCEPT (Intel +40.11 / AMD +44.67, pooled ≈ +42 over 1306g); flag default ON | bug/search | critical |
| 53 | Rule-50-aware TT eligibility (#29 follow-up) | **SHIPPED (2026-07-10, `baseline-t26`)** — on correctness+tests (#50/#51 precedent); blitz SPRT sign-split ≈ −6 Elo pooled, accepted; flag default ON | bug/search | critical |
| 54 | Transactional, bounded FEN / `position` input | **CLOSED (2026-07-09)** — unconditional, regression-tested | bug/input | critical |
| 55 | Bound every fixed-capacity move-list write | **CLOSED (2026-07-09)** — unconditional, regression-tested | bug/memory-safety | critical |
| 56 | UCI parser, options, timing, and search-control contract | **CLOSED (2026-07-12, parts 1–3)** — part 1: full setoption grammar, strict spin parser, honest adverts; part 2: mid-search command pump, race-free atomic-only cancellation, go-infinite bestmove lifetime, Syzygy default disabled + silent startup, subprocess transcript tests; part 3: pure 64-bit `compute_time_budget_ms` (50 ms floor capped by safely-usable remainder — tiny clocks get a 1 ms emergency budget instead of the forfeit-bait overdraft), strict `go` numerics, bare-`go` 5 s default, 0/1/10/49/50/100 ms boundary tests | bug/UCI | high |
| 57 | Use legal-move ordinal for PVS / LMR | **SHIPPED (2026-07-11, `baseline-t27`)** — AMD-only H1-ACCEPT (+29.98 ± 15.53, LOS 99.99%, user call per #51 precedent); flag default ON | bug/search | high |
| 58 | Make SEE sound before using it for hard pruning | **SHIPPED (2026-07-11, `baseline-t28`)** — pin-aware first recapture behind `ENABLE_SEE_LEGALITY`, default ON; two-machine same-sign positive (AMD +5.56 / Intel +8.69, pooled ≈ +7.2 ± 10.5, LOS ≈ 91%, 2000g), shipped on cross-machine agreement + correctness (user call); ship sig d14 = 7,128,502 / cp 30 / e2e4 | bug/search | high |
| 59 | En-passant key semantics (repetition + Polyglot) | **FIXED on main (2026-07-11)** — EP right normalized at source (MakeMove + set_from_fen, X-FEN convention); Polyglot wrong-rank check replaced, spec anchor keys pass; **SHIPPED (2026-07-11, `baseline-t29`)** — unconditional; AMD regression gate clean (+8.34 ± 15.32, LOS 85.73%, 1000g); Polyglot spec anchors pass | bug/rules/book | high |
| 60 | Make CMake / CTest / CI a trustworthy safety net | **CORE CLOSED (2026-07-11)** — `check` runs the real suite (fails on empty discovery), quick perft registered, BUILD_TESTING=OFF engine-only, real sanitizer flags, CI matrix incl. Windows; REMAINING: parser-purity test refactor + randomized invariants (see section) | build/test | medium |
| 61 | Repair or remove divergent public helper APIs | **CLOSED (2026-07-11)** — all four contracts fixed/removed + focused regressions (`test_audit_helpers.cpp`); d14 signature byte-identical | maintenance | low |
| 62 | Singular extensions (SF18-study EBF lever) | **SHIPPED (2026-07-13, `baseline-t31`)** — `ENABLE_SINGULAR_EXT` default ON; two-machine same-sign positive (AMD +12.17 / Intel +17.39, pooled **+14.90 ± 10.62, LOS ≈ 99.7%, 2000g**); first search-shape ship since t27; ship sig d14 = 6,583,846 / cp 24 / e2e4 | feature/search | high |
| 17-r2 | Aspiration windows at the root (re-test) | **SHIPPED (2026-07-13, `baseline-t32`)** — two-machine same-sign positive (AMD +12.51 / Intel +16.34, pooled +14.46 ± 10.61, LOS ≈ 99.6%); flag default ON; t15 attempt-1 rejection formally superseded | feature/search | high |
| 63 | History-modulated LMR (road-to-2.3 item 1) | **SHIPPED (2026-07-13, `baseline-t33`)** — `ENABLE_HISTORY_LMR` default ON; two-machine same-sign positive (AMD +8.69 / Intel +18.43, pooled **+13.63 ± 10.72, LOS ≈ 99.4%, 2000g**); fourth straight selectivity ship; ship sig d14 = 3,481,582 / cp 31 / e2e4 | feature/search | high |
| 7 | Late move pruning re-test (road-to-2.3 item 3) | **RE-PARKED (2026-07-14)** — Intel blitz leg negative lean (−6.60 ± 15.09, LOS 19.54%, 1000g; no AMD leg, cannot pool to the bar). Two-era verdict: −254→−56 vs t1, −6.6 vs t34 — five ordering ships moved LMP toward zero but never across; Huginn's shallow tree is already thin (RFP/null/razoring/futility/history-LMR). In-tree behind `ENABLE_LMP` (default OFF); revisit needs a NEW ordering signal, not dialing | feature/search | medium |
| 9 / 35 | Texel eval program + tapered eval | **IN-PROGRESS** — t10→t19 shipped (see archive); **threats round 2 SHIPPED `baseline-t30` (2026-07-12)**, pooled +17.0 two-machine; roadmap continues below | feature/eval | high |
| 37 | Board-desync illegal bestmove | **GUARDED + INSTRUMENTED**; root cause OPEN (needs repro) | bug | high |
| 42 | TT aging + clusters (Fruit/Toga design) | **IDEA 1 SHIPPED (2026-07-14, `baseline-t34`)** — LTC verdict positive (+15.99 ± 17.00, LOS 96.77%, 500g @ 60+0.6; blitz-flat/LTC-positive matches the staleness hypothesis); flag default ON. Idea 2 ("#42b") 4-way clusters: r1 (always-store) PARKED (AMD −9.38); **r2 (drop gate) PARKED (2026-07-14) — two-TC flat: AMD blitz +0.69 (LOS 53.58%, 1000g) / Intel LTC −2.78 (LOS 37.95%, 500g @ 60+0.6). The drop gate recovered r1's ~10-Elo regression but 4-way geometry adds nothing on top of aging; kept in-tree behind `ENABLE_TT_CLUSTERS` (default OFF) + 10 gated tests for a post-NNUE revisit. ITEM #42 CLOSED** (idea 1 shipped t34, idea 2 parked) — run-sheet in [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md) | feature/search | medium |
| 5 | Recalibrate vs external opponents (CCRL scale) | **MEASURED @t26 (2026-07-11)** — ~2571 ± 19 CCRL-blitz (Stash 19/20 anchors, [BASELINE_LADDER.md](BASELINE_LADDER.md)); re-run near ~2650 with a new rung | maintenance | medium |
| 31 | TT-size (`Hash`) sweep (road-to-2.3 item 4) | **BOTH LEGS DONE (2026-07-15) — FLAT on both boxes, ITEM CLOSED.** `huginn_t34.exe` × Hash 64/256/1024, 60+0.6, 600g/leg. AMD (7800X3D, 400g/engine, 6h27m): h256 +2.61 ± 21.49 / h1024 0.00 (anchor) / h64 −2.61 ± 21.89. Intel (600g, 6h25m): h1024 +6.95 ± 21.95 / h64 +4.34 ± 21.49 / h256 −11.30 ± 22.78. Every arm sits inside its own CI on both boxes, and the two boxes don't even agree on a direction (AMD ranks h256>h1024>h64, Intel ranks h1024>h64>h256) — that's noise, not a real per-architecture split. No clear LTC ordering on either machine ⇒ per the pre-registered rule, UCI release default stays at 64MB ([uci.cpp:250](../src/uci.cpp#L250)); gauntlet standard unchanged | tuning | medium |
| 34 | Pin/blocker-aware legal movegen | **OPEN** | speed/research | low |
| 39 | NNUE evaluation | **DEFERRED** (HCE first) — big lever | feature/eval | — |
| 40 | Lazy SMP / multithreading | **DEFERRED** (HCE first) — big lever | feature/speed | — |

## 2026-07-09 audit queue

These are code-backed defects, not speculative Elo ideas. Correctness and
memory-safety items should be fixed and regression-tested before resuming eval
expansion. Search-shape changes still need the normal fixed-depth / fixed-time
comparison and SPRT after their correctness tests pass.

**Status 2026-07-10: all four criticals CLOSED.** #54 + #55 fixed
unconditionally on 2026-07-09 (input/memory boundary, no search-shape
change); #52 + #53 went through the standard SPRT queue behind candidate
flags and **shipped together as `baseline-t26` (2026-07-10)** — both flags
(`ENABLE_QSEARCH_CHECK_EVASIONS`, `ENABLE_RULE50_TT_GUARD`) now default ON;
`-DENABLE_<X>=0` rebuilds a pre-t26 arm. Per-item resolution notes below;
ship writeup in [BASELINE_LADDER.md](BASELINE_LADDER.md).

### #52: Check-aware qsearch + horizon terminal states (critical)

**Finding.** `AlphaBeta()` enters quiescence at `depth == 0` before its check
test / check extension (`search.cpp:1900–1914`). `quiescence()` then accepts a
stand-pat score and generates captures only (`search.cpp:2505–2522`), even if
the side to move is in check. It therefore misses quiet king escapes/blocks,
checkmate, stalemate, and non-capturing promotions at the horizon. Its recursive
call also does not advance `info.ply`, despite the `seldepth` contract.

**Deterministic repro:** from
`7k/8/5KQ1/8/8/8/8/8 w - - 0 1`, `go depth 1` chooses `g6g5` at roughly
`cp 1277`; `g6g7#` is mate in one and is only found at depth 2.

**Acceptance criteria:**

- Detect check / terminal state before stand pat. In check, disable stand-pat,
  delta, and SEE pruning; search every legal evasion and return `-MATE + ply`
  when none exists.
- Detect non-check positions with no legal move as stalemate, and include quiet
  promotions in the tactical frontier.
- Advance `info.ply` through qsearch and report the true qsearch `seldepth`.
- Add direct regressions for the FEN above, a quiet-only block/king escape,
  checkmate, horizon stalemate, and a non-capture promotion. Then run tactical,
  fixed-depth, fixed-time, and SPRT comparisons behind an ablation flag.

**Resolution (2026-07-09): CANDIDATE behind `ENABLE_QSEARCH_CHECK_EVASIONS`
(default OFF, flag-off byte-identical — startpos d12 = 2,300,322 nodes both).**
Flag ON: qsearch detects check at entry; in check it skips stand-pat/delta/SEE,
searches every pseudo-legal evasion via `generate_all_moves`, and returns
`-MATE + ply` on no legal reply; out of check the frontier is
`generate_tactical_pseudo` (captures + quiet promotions); `info.ply` advances
through the recursion (true mate distances + seldepth); in-check nodes may run
past `MAX_QUIESCENCE_DEPTH` up to a hard 2× bound. Verified: the repro FEN now
returns `mate 1 / g6g7` at depth 1 (was `cp 1277 / g6g5`); regressions in
[test_audit_criticals.cpp](../test/test_audit_criticals.cpp) cover mate-in-1
at depth 1, horizon-checkmate encoding, quiet-only evasion, quiet promotion,
and seldepth (217/217 both arms). **Deliberately NOT implemented:**
horizon-stalemate detection in non-check qsearch nodes — it would need full
movegen + legality at every quiet leaf (huge cost, and standard engines skip
it too); stand-pat remains the approximation there. **Next:** two-machine
SPRT via branch `candidate/qsearch-check-evasions` (flag default flipped ON
there so the gauntlet bat builds the test arm from a plain checkout) — full
run-sheet incl. per-arm signatures and instant discriminators in
[SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md). Test-arm startpos d14 =
6,634,033 nodes (−21% fixed-depth vs t25's 8,406,631, at lower nps — the
SPRT decides the fixed-time trade). **Intel leg (2026-07-09): H1 ACCEPT
+40.11 ± 18.18, LOS 100%, 696g, 55.75% (W226/L146/D324), LLR 2.95,
Ptnml [12,69,126,109,32]** — the fixed-time trade pays. **AMD leg
(2026-07-10): H1 ACCEPT +44.67 ± 18.94, LOS 100%, 610g, 56.39%
(W188/L110/D312), LLR 2.97, Ptnml [7,66,97,112,23].** PGNs
`gauntlet/huginn_vs_t25_qsearch_{intel,amd}.pgn`. **SHIPPED in
`baseline-t26` (2026-07-10)** — same-sign two-machine H1-accept, pooled
56.05% / 1306g ≈ +42 Elo; flag default flipped ON (source + CMake option +
test mirror), ship build reproduces the test-arm signature (d14 =
6,634,033 + `mate 1 g6g7` at depth 1), 216/217 tests. Writeup:
[BASELINE_LADDER.md](BASELINE_LADDER.md).

### #53: Rule-50-aware TT eligibility — finish #29 (critical)

**Finding.** The direct `halfmove_clock >= 100` return precedes the TT, but a
clock-98/99 node can derive its score from a child that reaches 100 and then
store that propagated result under the position-only key. A later search of the
same placement with a low clock can reuse the draw-bound entry. Returning only
the terminal draw node before TT access does **not** keep its ancestors
path-independent, contrary to the current `INVARIANTS.md` wording.

**Deterministic repro:** warm one engine by searching
`k7/8/8/8/8/8/7Q/7K w - - 98 1` to depth 2, then search the same placement
with clock 0. The warm search returns about `cp 25`, `h2c7`; a fresh engine at
clock 0 returns about `cp 1211`, `h2d6`.

**Acceptance criteria:**

- Qualify TT probes/stores with enough rule-50 context, or bypass them whenever
  the searched horizon can reach the 100-ply boundary. Keep the repetition key
  semantically separate if a rule-50-qualified TT key is introduced.
- Add warm/cold, both-order tests across clocks 0/98/99/100; the clock-0 result
  after a warm near-draw search must match a fresh-engine oracle.
- Update `INVARIANTS.md` and the #29 write-up to cover propagated draw bounds,
  not only direct terminal returns.

**Resolution (2026-07-09): CANDIDATE behind `ENABLE_RULE50_TT_GUARD` (default
OFF, flag-off byte-identical).** Flag ON: when `halfmove_clock + depth >= 100`
(the subtree can reach the boundary) AlphaBeta takes no TT cutoff and stores
no entry; the store-side guard is re-evaluated with the final check-extended
depth; the TT best move stays usable for ordering (always sound); no key
change, so the repetition key is untouched. Verified at the UCI level: warm
clock-98 search then clock-0 search now returns the fresh-engine `cp 1211 /
h2d6` (guard OFF reproduces the poisoned `cp 25 / h2c7`); warm/cold both-order
regression in [test_audit_criticals.cpp](../test/test_audit_criticals.cpp)
asserts exact fresh-oracle equality. `INVARIANTS.md` TT bullet now covers
propagated draw bounds. **Documented residual:** check extensions can stretch
a subtree a few plies past nominal depth — a fixed guard cannot contain that
adversarial corner; precise containment needs taint propagation (follow-up if
ever observed). **Next:** SPRT via branch `candidate/rule50-tt-guard` (run-
sheet in [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md)) — expected
~neutral at 10+0.1 (the value is long shuffle endgames; consider an LTC leg
like #42's, or ship on correctness per the #50/#51 precedent). ⚠ This arm is
startpos-signature-IDENTICAL to baseline by design — verify with the
clock-98/clock-0 discriminator in the run-sheet, not startpos nodes.
**Intel leg (2026-07-10): mild REGRESSION, not neutral — −18.08 ± 15.32,
LOS 1.02%, 1000g (SPRT hit the round cap, LLR −2.51, leans H0/reject),
47.40% (W217/L269/D514), Ptnml [39,139,181,117,24], PairsRatio 0.79.**
Arms verified pre-run via the clock discriminator (test → `cp 1211 / h2d6`,
baseline → poisoned `cp 25 / h2c7`). The guard disables TT cutoffs near the
50-move boundary that blitz self-play does benefit from, so it costs a little
where it can't pay back — the correctness value lives in long shuffle endgames
this TC rarely reaches. PGN `gauntlet/huginn_vs_t25_rule50_intel.pgn`.
**AMD leg (2026-07-10): inconclusive / ~neutral — +5.21 ± 14.37, LOS 76.16%,
1000g (round cap, LLR 0.23, no bound crossed), 50.75% (W250/L235/D515),
Ptnml [23,119,205,126,27], PairsRatio 1.08.** Arms verified pre-run (clock
discriminator + d14 = 8,406,631 both arms by design + mate-in-1 probe clean).
PGN `gauntlet/huginn_vs_t25_rule50_amd.pgn`. **Two-leg verdict: machines
disagree in sign (Intel −18.08 / AMD +5.21); pooled 49.08% over 2000g
(≈ −6.4 Elo) — neutral-to-slightly-negative at blitz.** **SHIPPED in
`baseline-t26` (2026-07-10) on correctness+tests (explicit user call,
#50/#51 precedent)** — the small blitz cost is accepted as the price of
path-independent scores in long shuffle endgames (a #5-class conversion
concern this TC never exercises); NOT counted as an Elo winner. Flag
default flipped ON (source + CMake option + test mirror); ship build
passes the warm/fresh oracle-equality check (`cp 1211 / h2d6` both ways).
Writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md).

### #54: Transactional, bounded FEN / `position` input (critical)

**Finding.** `Position::set_from_fen()` calls `reset()` before validation and
mutates the live object while parsing (`position.cpp:180–256`). A rejected FEN
therefore leaves an empty or partial position; `UCIInterface::handle_position()`
passes its live root directly to it. Rank/file are not range-checked before
`set_sq64()` performs `1ULL << square`, digits outside `1..8` are accepted during
the scan, and `stoi` accepts suffix junk / negatives before narrowing into
`uint16_t`. Examples observed: `-1` becomes 65535 and `12junk` becomes 12.
The move suffix is also applied incrementally and processing continues after an
invalid move, leaving a valid prefix rather than rejecting the command.

**Acceptance criteria:**

- Parse a FEN and its optional UCI move list into a temporary; commit only after
  the complete command succeeds. Any failure must preserve FEN, key, clocks,
  side, and history exactly, and processing must stop at the first bad move.
- Require exactly eight ranks of eight files, digits `1..8`, checked square
  bounds before shifting, fully consumed numeric fields, non-negative halfmove,
  and `fullmove >= 1` with explicit storage-range handling.
- Give the UCI boundary a structural/legal-position validator (kings, side,
  castling/EP coherence) without preventing focused unit tests from constructing
  partial positions through explicit test helpers.
- Add malformed/adversarial corpus tests and run a FEN fuzz target under
  ASan/UBSan. Harden `MakeMove()` special-move preconditions so bad EP/castling
  flags fail without partially changing the position.

**Resolution (2026-07-09): CLOSED (unconditional — input boundary only, no
search-shape change).** `set_from_fen` now parses into a scratch `Position`
and commits only on full success (rejects leave `*this` untouched); strict
placement (digits `1..8`, file bounds checked before any shift, exactly 8×8),
duplicate-free castling set, EP square must be rank-3/6 AND side-coherent,
digits-only clock parse with explicit ranges (halfmove 0–65535, fullmove
1–65535). `handle_position` builds the whole command (FEN + every move) in a
scratch position, stops at the first bad move, and commits all-or-nothing.
New `validate_uci_position()` (uci_utils) gates the UCI boundary: exactly one
king/side, ≤8 pawns / ≤16 units per side, no pawns on ranks 1/8, idle side
not in check, castling rights match king/rook home squares, EP target backed
by the pushed pawn — while `set_from_fen` itself stays permissive for unit
tests. Malformed corpus + transactionality regressions in
[test_audit_criticals.cpp](../test/test_audit_criticals.cpp); the one
incoherent test fixture this exposed (`test_legal_moves.cpp` EP-pin FEN with
the wrong side to move) was rewritten into a real EP-pin assertion.
**Deliberate scope cuts:** `MakeMove()` preconditions NOT hardened (hot path —
bad EP/castling flags are now unreachable through the validated boundary);
the ASan/UBSan FEN fuzz target is deferred into #60, whose finding is that
`ENABLE_SANITIZERS` currently adds no flags at all.

### #55: Bound every fixed-capacity move-list write (critical)

**Finding.** `S_MOVELIST` has 256 entries, but all five append helpers rely only
on `__assume(count < MAX_POSITION_MOVES)` before writing (`movegen.hpp:21–112`).
Accepted composed/adversarial positions can exceed it. For example
`QQQQQQQk/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1` generates 279
pseudo-legal moves, so entries 257–279 overwrite adjacent stack memory; on GCC
violating the assumption is explicit undefined behaviour.

**Acceptance criteria:**

- Route every append through one checked primitive. Use a dynamically sized
  container or a capacity justified by a proven bound; in all cases fail closed
  and assert diagnostically rather than writing out of bounds.
- Add the 279-move position as a regression and exercise overflow handling in
  Release as well as Debug/sanitizer builds.
- Keep strict UCI position validation as a second boundary, not as the only
  protection for this core container.

**Resolution (2026-07-09): CLOSED (unconditional — unreachable from legal
chess, so no search-shape change).** All five `S_MOVELIST` append helpers now
route through one checked `full()` primitive: release builds fail closed
(drop the move, list stays valid), debug builds `DEBUG_ASSERT`; the
`__assume(count < MAX_POSITION_MOVES)` optimizer promises are gone. The
caps/tactical filter passes copy through a new checked `add_scored_move`.
The 279-move board is a Release regression (truncates to exactly 256, all
entries in-bounds) in
[test_audit_criticals.cpp](../test/test_audit_criticals.cpp), and the same
board is rejected at the UCI boundary by #54's piece-count gate (≤16 units
per side) — the deliberate second boundary.

### #56: UCI parser, options, timing, and search-control contract (high)

Several independently reproduced failures share the UCI boundary:

- `setoption` assumes one-token names/values (`uci.cpp:504–512`), truncating
  `BookFile` / `SyzygyPath` at the first space. `Threads` uses uncaught `stoi`;
  `setoption name Threads value nope` terminates the process.
- Construction auto-probes hard-coded `c:\TB\` and the tablebase layer writes a
  raw status line to stdout before the GUI sends `uci`.
- Search is synchronous; the input poll classifies **any** pending line as a
  stop. A queued `isready` ends a 10-second search in a few milliseconds instead
  of replying without stopping it. The alternative `signal_stop()` path exposes
  a stack `SearchInfo*` and writes its non-atomic fields from another thread.
- `go infinite` still has `max_depth = 25`, so it can return without `stop`.
  `Threads` (1–64) and `Ponder` are advertised but not implemented.
- The final `max(50, alloc)` can budget 50 ms with only 1–10 ms on the clock,
  defeating the preceding reserve cap. Limit arithmetic uses unchecked `int`.

These contradict the bundled UCI spec: option names/values may contain spaces,
`isready` must work while calculating, advertising `Ponder` promises support,
and `go infinite` must run until `stop`.

**Acceptance criteria:**

- Extract pure, checked option/limit parsers implementing the full
  `name ... [value ...]` grammar, including spaces/empty strings, strict numeric
  consumption, overflow checks, supported ranges, and no escaping exceptions.
- Use one lifetime-safe, race-free cancellation channel and a command pump that
  distinguishes `stop`/`quit` from `isready`/queued commands. No raw pointer to a
  stack `SearchInfo`; pass a ThreadSanitizer stop-stress test.
- `isready` answers promptly without a `bestmove`; `stop` produces exactly one
  prompt `bestmove`; queued `stop`, `position`, `go` executes in order; infinite
  and ponder modes obey their protocol lifetimes.
- Remove `Threads`/`Ponder` until implemented, emit no unsolicited startup
  stdout, default Syzygy to disabled/empty unless intentionally configured, and
  route diagnostics through protocol-safe `info string` or stderr.
- Use checked 64-bit clock math; never allocate beyond safely usable remaining
  time. Add boundary cases at 0/1/10/49/50/100 ms and subprocess transcript tests.

**Resolution (part 1, 2026-07-11): setoption robustness — shipped
unconditionally** (protocol-only, d14 byte-identical). Full
`name ... [value ...]` grammar with multi-token names/values,
`parse_spin_clamped` (whole-token numeric, overflow guard, range clamp — the
old naked `stoi` terminated the process on junk), Threads/Ponder adverts and
handlers removed.

**Resolution (part 2, 2026-07-12): search-control contract — shipped
unconditionally** (protocol-only; startpos d14 = 5,485,978 / cp 26 / e2e4
byte-identical to t29). What changed:

- **Mid-search command pump.** `SearchInfo::on_input` hook: checkup() hands
  pending stdin lines to the UCI layer instead of the old classify-anything-
  as-stop poll. `isready` mid-search → immediate `readyok`, search continues;
  `stop`/`quit` flag the SearchInfo on the searching thread; everything else
  (`position`, `go`, `setoption`, …) queues and replays in arrival order after
  `bestmove` (run() drains the queue before reading stdin again, so global
  order is preserved). Bare-Engine use keeps the conservative `read_input`
  fallback.
- **One race-free cancellation channel.** `Engine::should_stop` is atomic and
  is the only cross-thread signal (`signal_stop()` sets just that); checkup()
  translates it into `info.stopped` on the searching thread. The
  `running_info` published-raw-pointer path (cross-thread writes into a stack
  `SearchInfo`'s non-atomic fields) is deleted. MSVC has no TSan; the race is
  gone by construction (single writer) and a 12-round stop-stress test guards
  the observable contract.
- **`go infinite` lifetime.** Depth cap raised from the silent 25 to
  `MAX_DEPTH` (64 — every ply-indexed structure is bounds-guarded), and a
  naturally-completed infinite search parks in `wait_for_stop()` — still
  answering `isready`, still queueing — until `stop`/`quit` releases exactly
  one `bestmove`.
- **Silent startup + Syzygy default disabled.** Constructor no longer
  auto-probes `c:\TB\` (advertised default is now `<empty>`); the tablebase
  layer's status lines moved stdout→stderr; `initialize("")` means disabled,
  not a hard-coded path; SyzygyPath setoption feedback is an unconditional
  protocol-safe `info string`.
- **Tests.** In-process: per-line pump contract, infinite-hold (mated root —
  instant natural completion), stop-stress, startup silence, honest adverts
  (`test_uci_search_control.cpp`). Subprocess transcripts over real Win32
  pipes against the huginn binary: silent-until-uci, isready-during-search
  (readyok before bestmove, search survives), ChessBase-style batched
  `stop`+`position`+`go` (two bestmoves, in order), infinite-holds-until-stop,
  quit-mid-search exits (`test_uci_transcript.cpp`). 248 pass + 1 by-design
  skip.

**Resolution (part 3, 2026-07-12): clock math — shipped unconditionally**
(startpos d14 = 5,485,978 / cp 26 / e2e4 byte-identical to t29; the #47
allocation strategy is numerically unchanged on normal clocks — the house
gauntlet TC allocation `60000/100/0 → 3050 ms` is pinned by test). What
changed:

- **Pure 64-bit allocator.** `compute_time_budget_ms(time, inc, movestogo)`
  in uci_utils replaces the inline int math in `handle_go`. Same formulas
  (classical `time/mtg + inc/2`, sudden-death `time/20 + inc/2`, reserve
  `clamp(time/10, 50, 1000)`, 60% cap) with the floors fixed: `safe_max`
  lost its bogus 50 ms floor, and the 50 ms quality floor is now capped by
  the safely usable remainder — the old final `max(50, alloc)` budgeted
  50 ms with 1–10 ms on the clock, overdrawing the reserve it had just
  computed. A clock at/below the reserve now yields a 1 ms emergency budget
  (instant depth-1 move beats a time forfeit). Unknown own-clock falls back
  to `max(50, inc/4)`, or 5 s with no increment.
- **Strict `go` numerics.** `parse_spin_clamped` promoted from uci.cpp's
  anonymous namespace into uci_utils and used for depth/movetime/wtime/
  btime/winc/binc/movestogo with sane ranges (depth 1–64, movestogo 1–500,
  times 0–1e9). Junk is rejected whole-token (the old per-token `stoi`
  prefix-parsed `12junk` as 12); negative clocks (flagging GUIs send them)
  clamp to 0 → emergency budget; the 1e9 saturation guard bounds all
  downstream 64-bit arithmetic.
- **Bare `go` defined.** No limits at all → 5000 ms budget (previously
  `max_time_ms` stayed 0 and the first checkup killed the search ~2048
  nodes in — accidental, not a contract).
- **Tests** (`test_uci_time_allocation.cpp`): the acceptance boundary cases
  0/1/10/49/50/100 ms, a grid invariant (budget ≥ 1 and ≤ safely-usable
  remainder for all time/inc/movestogo combinations), movestogo=1 not
  spending the clock, hostile-magnitude overflow, parser junk/clamp/
  saturation, and end-to-end tiny-clock/junk-token `handle_go` liveness;
  plus a real-pipe transcript test (10 ms clock → instant bestmove, engine
  alive after). 258 pass + 1 by-design skip.

### #57: Use legal-move ordinal for PVS / LMR (high)

**Finding.** The alpha-beta loop increments `i` for pseudo-legal moves and skips
illegal entries, yet `i` decides the first full-window move, the LMR late-move
threshold, and fail-high-first telemetry (`search.cpp:2129–2299`). Several
ordered pinned moves can therefore make the **first legal move** look late and
receive a reduced null-window search. Separately, the current
`i == 0 || alpha == best_score` condition makes later moves full-window after a
normal alpha improvement, defeating most intended PVS savings.

**Acceptance criteria:**

- Maintain a legal/searched-move ordinal incremented only after successful
  `MakeMove()` and use it consistently for first-move PVS, LMR thresholds, and
  `fhf`; the first legal move must always receive a full-depth/full-window search.
- Add a pinned position whose ordered pseudo-list starts with multiple illegal
  moves, plus instrumentation tests asserting the expected full/null/re-search
  sequence and equivalence to a legal-list reference search.
- Gate the correction for fixed-depth score/PV/node comparison, then measure the
  search-shape change at fixed time and by SPRT.

**Resolution (2026-07-11): CANDIDATE behind `ENABLE_LEGAL_MOVE_ORDINAL`
(default OFF, flag-off byte-identical — startpos d14 = 6,634,033 unchanged).**
Flag ON: a searched-move ordinal incremented only after successful
`MakeMove()` drives the LMR lateness threshold + reduction row, PVS
first-move treatment, and fhf; the PVS condition is textbook (first legal
move full-window, all others null-window + fail-high re-search) — the
historical `i == 0 || alpha == best_score` gave every move after a normal
alpha improvement a full window, so null-window PVS only engaged in
failing-low nodes. Test-arm signatures: startpos d14 = 7,484,807 / cp 23 /
**d2d4** (root choice changes — instant arm discriminator); Kiwipete d13 =
1,930,694 / cp −63 / e2a6. Pinned-position fixture
(`AuditHelpersTest.PinnedPositionSearchReturnsLegalMove`) exercises the
illegal-moves-first shape on both arms; 226/226 both arms. The
full/null/re-search instrumentation sub-bullet is intentionally not
implemented — arm verification runs through the signature discriminators and
the SPRT per house process. **Next:** two-machine SPRT via branch
`candidate/legal-move-ordinal` — run-sheet in
[SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md). **AMD leg (2026-07-11):
H1 ACCEPT +29.98 ± 15.53, LOS 99.99%, 976g (early-stop, LLR 2.97), 54.30%
(W288/L204/D484), Ptnml [22,99,184,139,44].** PGN
`gauntlet/huginn_vs_t26_ordinal_amd.pgn`. **SHIPPED in `baseline-t27`
(2026-07-11) on AMD-only accept** (explicit user call given the clear
CI/LOS margin — #51/#25 precedent; no Intel leg). Flag default flipped ON
(source + CMake option); ship build reproduces the test-arm signature
(d14 = 7,484,807 / d2d4); 226/226 tests. Writeup:
[BASELINE_LADDER.md](BASELINE_LADDER.md).

### #58: Make SEE sound before using it for hard pruning (high)

**Finding.** SEE discovers exchanges geometrically and does not reject pinned
attackers or illegal king recaptures (`see.cpp:36–75, 148–157`). Quiescence then
hard-prunes every non-promotion capture with negative SEE
(`search.cpp:2562–2569`). From
`4k3/4n3/8/5p2/6Q1/8/8/4R1K1 w - - 0 1`, current SEE scores `Qxf5` as about
`-800` by counting `Ne7xf5`; that recapture is illegal because the knight is
pinned to the e8 king, so the real material result is +100.

**Acceptance criteria:**

- Either make SEE legality-aware for pinned pieces, king recaptures, EP x-rays,
  and promotions, or restrict imperfect SEE to ordering and remove the hard
  prune until it is sound.
- Add pinned-defender, illegal-king-recapture, en-passant x-ray, and promotion
  exchange tests, plus a qsearch regression proving `Qxf5` is searched.
- Re-run WAC/LCT2, fixed-depth signatures, node counts, and SPRT; this may trade
  nodes for tactics, so correctness tests precede the performance call.

**Resolution (2026-07-11): SHIPPED in `baseline-t28`.** Took the "make SEE
legality-aware where it matters most" line: the FIRST recapture is filtered
for pinned defenders (pin-line captures still count; deeper swap plies stay
geometric — the first recapture decides the hard-prune sign in the vast
majority of exchanges). Behind `ENABLE_SEE_LEGALITY`, default ON; flag-off
byte-identical. The audit's `Qxf5` fixture (pinned-knight recapture) plus
pin/EP/promotion exchange tests landed with the flag. Two-machine SPRT vs
t27, both legs same-sign positive — AMD +5.56 ± 15.11 (LOS 76%) / Intel
+8.69 ± 14.65 (LOS 88%), **pooled ≈ +7.2 ± 10.5, LOS ≈ 91% over 2000g** —
shipped on cross-machine agreement + correctness (user call, #15/#19
precedent). Ship signature: startpos d14 = 7,128,502 / cp 30 / e2e4.
Full writeup: [BASELINE_LADDER.md](BASELINE_LADDER.md).

### #59: En-passant key semantics — repetition + Polyglot (high)

**Finding.** Internal full and incremental Zobrist hashing include every stored
EP file, while every double push sets `ep_square` even when the opponent has no
pawn that can capture (`zobrist.cpp:35–38`, `position.cpp:44–56, 431–439`). That
distinguishes positions whose legal move sets are identical and can miss a
threefold. Independently, Polyglot checks pawns beside the EP **target rank**;
capturing pawns are one rank behind it (`polyglot_book.cpp:329–356`), so standard
book keys are wrong in capturable-EP positions.

**Repetition fixture:** from
`4k1n1/p7/8/8/8/8/8/4K1N1 b - - 0 1`, play `a7a5`, then
`g1f3 g8f6 f3g1 f6g8` twice. No White pawn can capture `a6`, so all three
post-cycle positions are equivalent; the first currently has a different key.

**Acceptance criteria:**

- Centralize the definition of a hashable EP right (at minimum a side-to-move
  pawn attacks the target; document whether full legal availability is used) and
  apply it consistently to full/incremental keys, consistency checking, and the
  Polyglot implementation.
- Preserve incremental-vs-full key equality across double pushes and EP expiry,
  and make the fixture above register the correct repetition count.
- Add canonical Polyglot vectors for startpos, castling, capturable EP for both
  colors, uncapturable EP, and promotion moves.

### #60: Make CMake / CTest / CI a trustworthy safety net (high)

**Finding.** The `check` target runs CTest from `${binaryDir}/bin`, which has no
`CTestTestfile.cmake`; it prints “No tests were found” and exits 0. The README's
`ctest --config Release` is also invalid (use `-C Release` or a preset). The
`ENABLE_SANITIZERS` option adds no sanitizer flags, the current CI covers Linux
Release only (Windows is explicitly disabled), and the comprehensive
`perft_suite` is built but not registered as a test. Parser tests largely assert
only `EXPECT_NO_THROW` while launching real searches, rather than checking the
parsed state.

Build paths also need a clean-config pass: compiler-ID logic runs before
`project()`, only CXX is enabled while non-MSVC Fathom sources are forced to C,
the advertised GCC “Linux” preset hard-codes `C:/msys64/...`, GoogleTest is
fetched even for production builds, and duplicate source lists / conflicting
C++17-vs-C++20 settings obscure the actual build contract.

**Acceptance criteria:**

- Run CTest from the build root (or `--test-dir`) and fail if discovery is empty.
  Register a bounded quick perft suite; keep full perft/tactical/benchmark jobs
  separately labelled. Convert the standalone futility test into real GTests.
- Make `BUILD_TESTING=OFF` build the engine offline without GoogleTest. Implement
  verified ASan+UBSan flags for GCC/Clang Debug and a Windows MSVC + Linux
  GCC/Clang CI matrix; exercise Fathom ON/OFF and restore a primary-platform
  Windows smoke test.
- Enable the languages actually compiled, make presets host-correct, choose one
  target-local C++ standard, deduplicate sources, and smoke-test every advertised
  preset from a clean directory.
- Factor pure FEN/UCI parsers so tests assert exact results/errors. Add randomized
  legal make/unmake/null-move invariants and exact eval-mirror properties. Fix
  documented commands and avoid hard-coded test counts that immediately drift.

**Resolution (2026-07-11): CORE CLOSED.** Everything except the last bullet:
`check` now runs CTest from the build root with `-C $<CONFIG>
--no-tests=error` (was: ran in `bin/`, found zero tests, exited 0 — every
"`--target check` passed" before this date verified nothing); `perft_quick`
(startpos + Kiwipete to d5, ~7s) registered in the default set with the full
EPD suite behind `-DHUGINN_HEAVY_TESTS=ON` (label `heavy`);
`BUILD_TESTING=OFF` builds the engine with no GoogleTest fetch;
`ENABLE_SANITIZERS` applies real ASan+UBSan flags on GCC/Clang Debug (ASan
on MSVC) instead of printing a warning; the never-executed pre-`project()`
MSYS2 block was deleted (CMAKE_CXX_COMPILER_ID is empty before `project()` —
it had printed its else-branch on every MSVC configure since it was written);
`project()` now enables C for non-MSVC Fathom; source lists deduplicated;
single global C++20 (per-target `cxx_std_17` minimums dropped); the
"gcc-x64-release" preset uses PATH compilers instead of a hard-coded
`C:/msys64`; docs' `ctest --config` → `ctest -C`; dead never-built
`test/test_futility_pruning.cpp` deleted; CI rewritten
(`.github/workflows/ci.yml`): Windows MSVC + Linux GCC/Clang, Fathom ON/OFF,
a Debug ASan+UBSan leg, and a BUILD_TESTING=OFF engine-only + UCI smoke job.
Verified: 226/226 CTest entries pass locally, startpos d14 = 6,634,033
byte-identical. **REMAINING (this item stays open at medium):** the
parser-purity refactor (exact-result FEN/UCI parser assertions instead of
EXPECT_NO_THROW), randomized make/unmake/null-move invariants, and exact
eval-mirror properties.

### #61: Repair or remove divergent public helper APIs (low)

Small, verified inconsistencies that are not on the current production search
path, but are traps for tests and future callers:

- `Position::perft()` calls the member `generate_all_moves()`, whose body only
  clears the list, so `pos.perft(depth > 0)` returns 0. Existing tests use a
  separate local perft and never exercise the public method.
- `generate_legal_moves()` re-scores captures after making them, when the source
  square is empty, and resets promotion scores despite claiming to preserve the
  generator's scores.
- `S_MOVE::to_string()` indexes a zero-based piece-character table with the
  one-based `PieceType` value and emits the wrong/uppercase promotion suffix.
- `clear_piece_sq64()` subtracts king value although rebuild/add deliberately
  exclude kings; malformed king capture/removal therefore desynchronizes
  material.

**Acceptance criteria:** delegate the member generator/perft to the production
free generator or remove the duplicate API; copy already-scored legal moves;
make promotion text share the UCI formatter; make king material updates
symmetric and assert kings are never captured. Add focused tests for all four
contracts and update stale mailbox-120 / undo-backup architecture docs.

**Resolution (2026-07-11): CLOSED.** `Position::perft()` + the member
`generate_all_moves()` stub REMOVED (zero callers anywhere — the compiler now
enforces the contract); `generate_legal_moves()` copies surviving moves
verbatim via `add_scored_move` (generator scores preserved; its three
production call sites — book-move validation, bestmove-legality guard, UCI
move parsing — only match by move identity, so this is search-neutral);
`S_MOVE::to_string()` fixed (lowercase UCI promotion suffix, correct piece,
"0000" null-move/bounds guards) and `Engine::move_to_uci` now delegates to it
— one formatter; `clear_piece_sq64()` excludes king material symmetrically
with `add_piece_sq64` and DEBUG-asserts kings are never cleared. Focused
regressions in [test_audit_helpers.cpp](../test/test_audit_helpers.cpp)
(8 tests, all four contracts). Verified: 226/226, d14 = 6,634,033
byte-identical.

### #62: Singular extensions — the SF18-study EBF lever (high)

**Motivation ([SF18_GAP_STUDY.md](SF18_GAP_STUDY.md), 2026-07-12).** The gap
study measured Huginn's effective branching factor at **~1.90/ply vs SF18's
~1.37** — at equal depth SF's tree is 112× smaller by d16, and that compounding
is most of the non-eval strength gap. Extensions concentrated on forced lines
are the classic EBF attack, and the #57 family already proved selectivity work
converts on this codebase (+30 solo).

**What (2026-07-12): CANDIDATE behind `ENABLE_SINGULAR_EXT` (default OFF,
flag-off byte-identical — startpos d14 = 8,298,375 unchanged).** At a non-root,
non-check node with `depth >= 8` whose TT entry has a LOWER_BOUND/EXACT
non-mate score at `tt_depth >= depth − 3` and a best move, run a reduced-depth
(`(depth−1)/2`) **exclusion search** of every *other* move at a null window
just below `tt_score − 2·depth`; if it fails low, nothing else comes close —
the TT move is *singular* and is searched one ply deeper. Plumbing: a defaulted
`excluded_move` parameter through `AlphaBeta`; at exclusion nodes there is no
TT cutoff and no TT store (the entry describes the full move set), no
null-move (`doNull=false` — meaningless in a "position minus one move"), no
PV-table write, and a no-other-legal-move result fails low at the exclusion
window (maximally singular), not mate. `info.singular_exts` counts fires.

**Verified:** OFF arm byte-identical to t30 (d14 = 8,298,375 / cp 26 / e2e4
exact); ON arm startpos d14 = **6,583,846** / cp 24 / e2e4 (−20.7% — sharper
tree), Kiwipete d13 = **3,442,234** / cp −75 / e2a6 (+86%, seldepth 30 vs 25 —
tactical lines verified much deeper); 4 regressions in
[test_singular_ext.cpp](../test/test_singular_ext.cpp) (deep-search mate
integrity + determinism both arms, extensions-fire on the ON arm,
counter-dead on the baseline arm); full suite 268/269 green both arms (1
by-design skip). **SPRT — BOTH LEGS DONE (2026-07-12), same-sign positive:**
AMD +12.17 ± 15.39 (LOS 93.97%) / Intel +17.39 ± 14.67 (LOS 99.01%), both
1000g caps, clean runs; **pooled +14.90 ± 10.62, LOS ≈ 99.7%, 2000g** —
clears the two-machine bar (threats-r2 precedent). **SHIPPED as
`baseline-t31` (2026-07-13)** — flag default ON (source + CMake), first
search-shape ship since t27; full leg detail in
[SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md).

### #17-r2: Aspiration windows at the root — RE-TEST off t31 (high)

**Motivation (2026-07-13).** Attempt 1 (`228817b`, MTLChess-guided) was
H0-rejected at t15 (−33.8 ± 18.0 AMD, LOS 0.01%) and reverted (`df4ccdb`),
falsifying the then-current "stronger eval stabilizes scores" hypothesis. But
that verdict predates **every** search-soundness fix that followed: #44
repetition blindness (t17), #50 Zobrist OOB corrupting TT scores (t23), #52
check-blind qsearch (t26), #57 the broken PVS re-search condition (t27), #58
SEE pin legality (t28). Aspiration's failure mode is inter-iteration score
instability → fail-low/high re-search storms, and those five bugs were all
direct sources of it — a contaminated verdict (#45 precedent: falsified
diagnoses hide behind since-fixed bugs). The SF18 gap study's EBF thesis
(1.90 → toward 1.37) makes it the on-thesis follow-up to #62. Measured on t31
before implementing: startpos swings ≤ 9cp between iterations, Kiwipete
≤ 31cp from depth 6 on — a ±50cp window holds where it didn't at t15.

**What (2026-07-13): CANDIDATE behind `ENABLE_ASPIRATION` (default OFF,
flag-off byte-identical — startpos d14 = 6,583,846 unchanged).** From depth
≥ 6 (attempt 1 opened at 4, inside the measured noisy zone) the root searches
`[prev−50, prev+50]` around the previous depth's score; a fail widens the
failed side ×2 around `best_score`; delta > 800 or a mate-range centre
(|score| ≥ 27000) snaps to the full window. Interrupted passes are discarded
by the existing stopped-check (bestmove never publishes from a failed pass).
`info.aspiration_researches` counts window fails.

**Verified:** OFF arm byte-identical to t31 (d14 = 6,583,846 / cp 24 / e2e4
exact); ON arm startpos d14 = **5,669,691** / cp 33 / e2e4 (−13.9%), Kiwipete
d13 = **2,768,609** / cp −88 / e2a6 (−19.6% — same best moves, cheaper tree
at fixed depth); 3 regressions in
[test_aspiration.cpp](../test/test_aspiration.cpp) (deep-search mate
integrity + determinism both arms, window-fails-fire on the ON arm,
counter-dead on the baseline arm); full suite 271/272 green both arms (1
by-design skip). **SPRT — both legs positive (2026-07-13):** AMD +12.51 ±
15.12 (LOS 94.78%, LLR 1.07) / Intel +16.34 ± 14.88 (LOS 98.45%, LLR 1.53),
each 1000g cap, both clean ~1h52m runs. **Pooled +14.46 ± 10.61, LOS ≈ 99.6%,
52.08% / 2000g** (W534/L451/D1015, Ptnml [47,235,375,274,69]) — clears the
standard two-machine bar; same ship profile as #62 (+14.90) and threats-r2
(+17.0). The t15 attempt-1 rejection is formally superseded — with the
soundness stack fixed, aspiration converts exactly as the contaminated-verdict
hypothesis predicted. **SHIPPED `baseline-t32` (2026-07-13)** —
`ENABLE_ASPIRATION` default flipped ON (source `#ifndef` + CMake option); ship
build verified d14 = 5,669,691 / cp 33 / e2e4 exact from a clean no-override
configure; 271/272 green (1 by-design skip); huginn_t32 snapshotted. Run-sheet
in [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md); writeup in
[BASELINE_LADDER.md](BASELINE_LADDER.md).

### #63: History-modulated LMR — road-to-2.3 item 1 (high)

**Motivation.** The LMR table is static `log·log` over (depth, move-ordinal):
every quiet at the same slot gets the same reduction regardless of how that
move has actually performed. Modulating reductions by move history is the
standard next EBF cut after extensions (#62) and aspiration (#17-r2) — the
third leg of the SF18-study selectivity program — and it needs no
continuation history (falsified, #3): the butterfly table
(`search_history[13][64]`, ±depth² updates, aged ÷4 across searches) is
already there.

**What (2026-07-13): CANDIDATE behind `ENABLE_HISTORY_LMR` (default OFF,
flag-off byte-identical — startpos d14 = 5,669,691 unchanged).** At the LMR
site, after the table lookup and before the `[1, depth−2]` clamps, the
mover's butterfly-history score adjusts the reduction by ±1 ply:
`hist >= +4096` (proven-good quiet) ⇒ reduce one ply less;
`hist <= −4096` (history-hated quiet) ⇒ reduce one ply more
(`HISTORY_LMR_GRAIN = 4096`). Read post-MakeMove from the TO square
(promotions are LMR-exempt, so `at_sq64(to)` is always the mover).
`info.history_lmr_adjusts` counts modulations.

**Verified:** OFF arm byte-identical to t32 (d14 = 5,669,691 / cp 33 / e2e4
exact); ON arm startpos d14 = **3,481,582** / cp 31 / e2e4 (−38.6% — the
biggest fixed-depth cut of the selectivity series), Kiwipete d13 =
**1,958,182** / cp −85 / e2a6 (−29.3%; same best moves — node count
discriminates, not the root move); 4 regressions in
[test_history_lmr.cpp](../test/test_history_lmr.cpp) (deep-search mate
integrity + determinism both arms, modulation-fires on the ON arm,
counter-dead on the baseline arm); full suite 274/275 green both arms (1
by-design skip). **SPRT — BOTH LEGS DONE (2026-07-13), same-sign positive:**
AMD +8.69 ± 15.27 (LOS 86.78%) / Intel +18.43 ± 15.05 (LOS 99.19%), both
1000g caps, clean runs; **pooled +13.63 ± 10.72, LOS ≈ 99.4%, 2000g** —
clears the two-machine bar (widest per-leg spread of the selectivity
series, both positive). **SHIPPED as `baseline-t33` (2026-07-13)** — flag
default ON (source + CMake), fourth straight selectivity ship; full leg
detail in [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md).

### #9 / #35: Eval program — Texel tuning + tapered eval (IN-PROGRESS, paused)

The main strength thread. A `game_phase_256` tapered eval foundation (#35) plus
a Texel tuner (`tools/texel/`, `bake.py` rewrites headers from the tuner dump,
verified exact) drive successive full-vector re-tunes / new-feature rounds on
the 725k Zurichess quiet-labeled corpus. Shipped ladder (full round-by-round
detail in [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md)):

- t10: tapered-eval foundation · t11: material+PSTs+king-EG · t12: tapered EG
  PSTs + tunable mobility · t13: connected+backward pawns · t14: rook-on-7th
  (sign-split exception) · t15: threats (largest eval-term ship) · t16: king
  safety (first KS ship) · t19: safe mobility (cross-machine-agreement ship)

Method note (learned across rounds): **new-feature MSE converts to Elo better
than re-fit MSE**; SPRT decides every round. King safety (t16) is the
exception that proves a corollary: quiet-corpus MSE UNDER-states terms that
pay in sharp positions.

**Paused since 2026-06-27:** the #45 pruning-stack audit (now closed, see
archive) redirected the active thread from eval to search-soundness, which
dwarfed any eval term in the same period (#45 +510, #47 +127 vs the ~+30 from
t16–t19 combined). Round 7+ HCE roadmap remains, priority order set by the
#41 calibration study (king safety → threats round 2 → safe mobility →
outposts; king safety, safe mobility, and outposts are done — see archive):

- **Threats round 2** — extend the +54 t15 cluster: hanging pieces (attacked
  *and* undefended), pawn-push threats, threat-by-king. Same machinery,
  proven. **CANDIDATE (2026-07-12) behind `ENABLE_THREATS_R2`** (default OFF,
  flag-off byte-identical to t29 — startpos d14 = 5,485,978 unchanged).
  Three classes layered on t15: hanging units (attacked + undefended), safe
  pawn-push threats, hanging units in the king ring. Params Texel-fitted
  `--only-new` (new tuner mode: the six new params fit while the rest of the
  841-param vector stays frozen — clean SPRT attribution): HANGING 9/18,
  PAWN_PUSH 10/4, BY_KING −5/34; **new-feature ΔMSE 0.056857 → 0.056459
  (−0.000398, ~1.6× the t16 KS round)**. Bake-verified exact. Test-arm sigs:
  startpos d14 = 8,298,375 / cp 26 / e2e4 (+51% — big tree reshape, fixed
  time is the SPRT's call); Kiwipete d13 = 1,846,915 / cp −83 / e2a6.
  7 regression tests (`test_eval_threats_r2.cpp`): mirror symmetry both
  arms, term behaviour + unsafe-push filter on the ON arm. Both arms full
  suite green. **Two-machine SPRT vs t29 — both legs positive:** AMD +9.73 ±
  14.88 (LOS 90.02%, 1000g cap, LLR 0.75) / Intel +24.01 ± 14.55 (**LOS
  99.94%**, 1000g cap, LLR 2.54), both clean 1h48–49m runs. **Pooled ≈ +17.0 ±
  10.4 Elo, LOS ≈ 99.9%, 2000g, 52.43%** (W552/L455/D993, Ptnml
  [44,218,406,261,71]) — neither leg crossed the LLR bound before the cap but
  the pool clears the strict 95% bar decisively. **SHIPPED `baseline-t30`
  (2026-07-12)** — largest eval-term ship since t15; `ENABLE_THREATS_R2` default
  flipped ON (source `#ifndef` + CMake option). Ship build verified startpos
  d14 = 8,298,375 / cp 26 / e2e4; 265 tests green (1 by-design skip); huginn_t30
  snapshotted. Run-sheet in [SPRT_QUEUE_TEST_PLAN.md](SPRT_QUEUE_TEST_PLAN.md);
  writeup in [BASELINE_LADDER.md](BASELINE_LADDER.md).
- **Passed-pawn refinements** — king distance to the passer (own + enemy),
  blockade, rook-behind-passer. **Deprioritized**: #41 shows balanced-endgame
  play is already solid (fair-fight cp-loss 13.3). Not yet attempted.
- **Lower priority:** doubled-rooks / blind-pig follow-up to t14's
  rook-on-7th; space (safe squares behind own centre pawns); rook-on-king-file;
  specific endgame recognizers (KPK, KRKP).

### #37: Board-desync illegal bestmove — GUARDED + INSTRUMENTED, root cause OPEN

A rare, **time-dependent** make/unmake imbalance can leave the engine's internal
board desynced from the real root position, so the search can return a move that
is illegal in the actual position (observed as `b2b4` in round-6 vs t14 on
Intel; fastchess forfeits on it). Guarded (UCI boundary validates + substitutes,
never forfeits) and instrumented (`ENABLE_SEARCH_INTEGRITY_ASSERTS` diagnostics)
— see [BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) for that work's detail.

- **Root cause still open.** Not reproducible cold / warm-TT / faithful replay —
  only under real time pressure. Audited every make/unmake site + the primitives;
  all balanced on inspection. **Next:** run an instrumented self-play / fastchess
  repro until it trips, then bisect from the captured context.
- **Follow-up hook (2026-07-04):** the #50 fix (Zobrist black-king-row OOB,
  shipped `baseline-t23`) removed a real TT-key-collision source. Zobrist
  collisions corrupting TT-move sourcing was one of the few mechanisms that
  could explain a rare illegal bestmove — worth re-testing whether #37 still
  repros post-#50 before investing further debugging time here.

### #42: TT aging + clusters (Fruit/Toga design) — INCONCLUSIVE (2026-07-03)

Huginn's TT (`transposition_table.hpp`) is single-entry-per-index,
depth-preferred replacement, no aging. It persists across searches
(`clearForSearch` clears the PV table, not the TT), so with depth-preferred-only
replacement a deep entry stored early in a game can squat in its slot
indefinitely — new shallow stores can't evict it (`depth >=` fails), crowding
out fresh results. Two portable ideas from the Fruit/Toga TT:

1. **Date-based aging** *(the main win)* — each entry carries a `date`; a
   global date bumps once per search; `age` = searches-elapsed. Replace by
   value `age*256 - depth` (prefer old + shallow). **Tested 2026-07-03**
   (`copilot/fix50-for-tt-aging`, `ENABLE_TT_AGING`): two-machine SPRT vs t23
   (10+0.1, 1t, 64MB, noob_3moves.epd, 1000g each): AMD **+0.69 ± 15.24**
   (LOS 53.56%, dead flat); Intel **+11.12 ± 15.01** (LOS 92.71%, real lean,
   undecided). Same-sign positive, but AMD too flat for clean cross-machine
   agreement (cf. t19's both-legs-above-+5 precedent). Behavior verified
   genuinely live (211/211 tests incl. 8 aging-specific cases; a direct
   multi-search-in-one-process A/B with a small hash showed the arms diverging
   from the second search onward). **Reading:** aging's value should
   concentrate in LONG games (many searches per game accumulate more staleness
   for it to fix) — blitz may genuinely understate it. **LTC re-test
   (road-to-2.3 item 2): ported to `candidate/tt-aging-ltc` off t33
   (cherry-pick `4c54c96`, clean — zero TT-header drift since t22; 282/283
   tests green incl. the 8 aging cases; first-search d14 = 3,481,582 matches
   t33 exactly BY DESIGN — the arm discriminator is the gated TT tests, not
   node counts). Intel LTC leg (60+0.6, SPRT [0,10], 500g cap):
   +15.99 ± 17.00, LOS 96.77% — POSITIVE, and the TC profile (blitz AMD flat
   / blitz Intel +11 lean / LTC +16) matches the staleness hypothesis
   exactly. SHIPPED `baseline-t34` (2026-07-14)** per the pre-registered
   decision rule; `ENABLE_TT_AGING` default ON on `main`. PGN
   `gauntlet/huginn_vs_t33_ttaging_ltc_intel.pgn`.
2. **Clusters ("#42b")** — each index addresses an N-way contiguous cluster
   (probe scans for a lock match; store replaces the least-valuable slot).
   Fewer collision evictions → higher hit rate, at the cost of a small
   per-probe scan. **CANDIDATE (2026-07-14) behind `ENABLE_TT_CLUSTERS`**
   (default OFF; flag-off byte-identical to t34 — startpos d14 = 3,481,582 /
   Kiwipete d13 = 1,958,182 both reproduce exactly, verified again after the
   cache unstick). Design: the index unit becomes a **4-entry cluster**
   (4 × 16B = one 64-byte cache line, `alignas(64)` — a whole-cluster scan
   costs the same memory traffic as a single-entry probe); probe scans for a
   full-key match; store refreshes a same-key slot, else fills an empty slot,
   else ALWAYS replaces the least-valuable resident by Fruit's
   `age_dist*256 − depth` (stale-dated first, then shallowest — composes with
   idea 1's shipped aging; with aging off it collapses to min-depth). The
   single-slot drop-shallower-store rule is deliberately gone on this arm:
   deep residents are protected by victim choice, not write rejection. Same
   total entries at any Hash size (the index just loses 2 bits). Test-arm
   sigs: startpos d14 = **3,367,661** / cp 34 / e2e4 (−3.3%); Kiwipete d13 =
   **1,956,522** / cp −85 / e2a6 (−0.08%) — unlike idea 1, the arms diverge
   from the FIRST search (replacement changes within a single search), so
   startpos node counts DO discriminate the arms. 6 cluster-gated tests
   (capacity, shallowest-victim, same-key refresh, stale-beats-depth,
   probe-touch protection, age-wrap fallback); ON arm 286/286, OFF arm
   284/284 (each incl. the 1 by-design skip). ⚠ TC caveat: a single fresh
   d14 search fills 64MB to only ~43‰ hashfull — collision pressure (the
   thing clusters fix) builds over a GAME as the persistent table fills, so
   like idea 1 blitz may understate; idea-1 precedent (blitz flat → LTC +16)
   makes an LTC leg the natural follow-up on a flat-positive blitz read.
   **r1 AMD blitz leg (2026-07-14): NEGATIVE-LEAN — r1 PARKED.** **−9.38 ±
   14.81** (nElo −13.66 ± 21.53), LOS 10.69%, 1000g cap (LLR −1.55, leaning
   H0, no bound), 48.65% (W222/L249/D529), DrawRatio 38.40%, PairsRatio
   0.90, Ptnml [33,129,192,124,22]. PGN
   `gauntlet/huginn_vs_t34_ttclusters_amd.pgn`. No Intel leg — parked on
   the mechanistic read (user call). Diagnosis: always-store is the one
   semantic delta vs baseline, and it admits EVERY depth-1/2 store (the
   exponential bulk of all stores), each displacing the cluster's
   4th-deepest resident — continuously washing out the mid-depth entries
   that feed TT cutoffs, IID, and singular-extension eligibility
   (`tt_depth >= depth−3`) on re-visits. The baseline arm dropped those
   shallow colliding stores outright. Fruit's design, but Huginn's stack is
   tuned to Huginn's dynamics (#53 lesson: design-faithful ≠ Elo-positive).
   **r2 QUEUED: same clusters + the baseline drop gate applied to the
   weakest resident** — a store lands only on a same-key / empty /
   stale-dated victim, or when `depth >= victim.depth`. Strictly fewer
   dropped stores than baseline (the weakest-of-4 is ≤ the single random
   occupant the baseline compares against) and never a shallow-over-deep
   eviction within a search; single-knob delta from r1 for clean
   attribution. If r2 recovers to ≥ 0 the always-store hypothesis is
   confirmed; if it still loses, the geometry itself is implicated and
   #42b parks for good.
   **r2 AMD blitz leg (2026-07-14): DEAD-FLAT, hypothesis supported.**
   **+0.69 ± 15.18** (nElo 0.99 ± 21.53), LOS 53.58%, 1000g cap (LLR
   −0.33), 50.10% (W230/L228/D542), DrawRatio 38.60%, PairsRatio 1.05,
   Ptnml [34,116,193,128,29]. PGN
   `gauntlet/huginn_vs_t34_ttclusters_r2_amd.pgn`. The single-knob drop
   gate recovered the full ~10-Elo r1 regression on the same box/baseline
   (−9.38 → +0.69; directionally decisive, CIs overlap) — always-store
   was the r1 problem. r2's blitz profile now exactly mirrors idea 1's
   (aging AMD blitz was also +0.69 dead-flat; LTC +15.99 shipped it), and
   the mechanism predicts the same TC-dependence: clusters pay only where
   the table actually fills (a blitz search runs at ~43‰ hashfull; 60+0.6
   searches are ~10× deeper into the fill curve).
   **r2 Intel LTC leg (2026-07-14): DEAD-FLAT — PARKED, #42b closed.**
   **−2.78 ± 17.77** (nElo −4.77 ± 30.45), LOS 37.95%, 500g cap (LLR −0.40),
   49.60% (W85/L89/D326), DrawRatio 48.80%, Ptnml [6,61,122,53,8]. Clean,
   5h25m; arm verified pre-run (d14 = 3,367,661 exact + the r2 drop-gate
   discriminator test). An early −40 @ 120g converged to flat — noise. PGN
   `gauntlet/huginn_vs_t34_ttclusters_r2_ltc_intel.pgn`. The idea-1 analogy
   broke exactly where the mechanisms differ: aging CLEANS a filling table
   (LTC +16), clusters only re-shape collisions in it — and with aging
   already shipped, 4-way geometry adds nothing at either TC (blitz +0.69 /
   LTC −2.78). Pre-registered rule applied: **park**; code stays in-tree
   behind `ENABLE_TT_CLUSTERS` (default OFF, byte-identical off, 10 gated
   tests) for a post-NNUE revisit. **Item #42 CLOSED** — idea 1 shipped
   (t34), idea 2 parked with a complete two-TC verdict.

Pairs naturally with #31's Hash sweep (test aging + size together).

### #5: Recalibrate vs external opponents (OPEN)

Re-anchor the CCRL-Blitz-scale Elo estimate. Scripts: `test_huginn_gauntlet.bat
<snowy|cdrill|mtl03|mora|mtl05|mtl06>`; the 2.0 archive has the t11 3-anchor
MLE (~1818). Full round-robin + Stash-ladder calibration detail (round-robins,
findings, the score-compression/conversion-weakness lead) is in
[BACKLOG-archive-2.1.md](BACKLOG-archive-2.1.md) under this item's original
write-up — key finding carried forward:

- **Score compression / monotonic anchor drift (2026-06-15):** the per-anchor
  estimate *climbs* with opponent strength (Huginn over-performs a flat Elo
  model vs strong opponents, under-performs vs weak ones — 99 draws vs Stash
  11.0). Reads as a **conversion weakness** — solid vs stronger engines but
  doesn't convert wins vs weaker ones. Points at drawishness-scaling /
  50-move-conversion items (drawishness-scaling now tested+parked, see the
  Deferred/parked list below — a weaker-anchor retest is the natural next step
  for that item, which would also feed back into this one).
- **Next:** re-run the Stash-anchored round-robin to full 2400g (prior run
  stalled at 2049g on a sleep interrupt) with sleep disabled (`powercfg /change
  standby-timeout-ac 0`) to tighten CIs; `stash14` (2060) optional for
  headroom. Then fold the refreshed estimate into CLAUDE.md.

### #31 / #34: Speed / research (OPEN, low)

- **#31** — SPRT sweep of `Hash` (64 vs 128 vs 256 MB) at the current
  strength. Pairs with #42's TT-aging test (test aging + size together).
- **#34** — SF-style `blockersForKing` pin/blocker-aware legal movegen (drop
  the per-move `MakeMove` legality filter for pinned-piece fast paths).

## Big levers (deferred — HCE first)

The two architectural jumps that dwarf any single HCE term. Both deliberately
deferred (2026-06-13) to exhaust pure-HCE headroom first — but recorded here so
the strategic picture is explicit.

### #39: NNUE evaluation (deferred)

- The per-node *quality* lever — hundreds of Elo, and the honest path to closing
  the gap to Fruit/Stockfish-class strength. Replaces (or augments) the HCE
  `evaluate()` with a small efficiently-updatable network.
- **Measured 2026-07-12** ([SF18_GAP_STUDY.md](SF18_GAP_STUDY.md)): on
  Kiwipete, SF18's depth-1 NNUE eval is closer to the converged truth than
  Huginn's depth-16 HCE result, and Huginn's static eval carries the wrong
  sign until depth 6. Same study: Huginn is ~2× faster in raw nps but its
  tree is 112× larger at equal depth (EBF ~1.90 vs ~1.37) — the two levers
  in one number each.
- **Why it fits this project:** preserves single-thread determinism (the
  fixed-depth / cross-machine-bit-identical test methodology survives), and it
  improves the thing we actually care about — move quality per node.
- **Cost:** a multi-session build — training-data generation (self-play at depth,
  labelled), the net + quantisation, and incremental-update integration into
  make/unmake. Highest conceptual lift of the options; lowest methodology cost.
- **Hardware fit (2026-06-26):** this is also where the AMD box's **AVX-512** earns
  its keep — accumulator updates + int8/int16 dot products map to `AVX512-VNNI`
  (`VPDPBUSD`) for a big inference speedup. The current HCE engine doesn't vectorize
  (see the AVX-512 note under #32 in the archive), so NNUE is the project that
  justifies an AVX-512 code path. (Intel consumer 12–14th gen lacks AVX-512 →
  the fast path would be a build-gated AMD variant with an AVX2 fallback.)

### #40: Lazy SMP / multithreading (deferred)

- The biggest *Elo-at-tournament-hardware* lever short of NNUE: ~**+100–200 Elo**
  on the 8-core gauntlet boxes (currently idle — a cc=4 gauntlet just runs 4
  separate single-thread games, never one strong parallel search). Lazy SMP is a
  near-boilerplate algorithm, so lowest *conceptual* risk.
- **But three real catches** (the reason it's deferred, not done):
  1. **Buys Elo with hardware, not smarts** — per-node quality is unchanged.
     Legitimate for CCRL placement and GUI use, but orthogonal to the eval gap.
  2. **Breaks the determinism the test methodology leans on** — Lazy SMP is
     non-deterministic by construction, killing the bit-identical-at-fixed-depth
     ship checks and the cross-machine WAC determinism (#33-class).
     **Measured on SF18 (2026-07-12,
     [SF18_GAP_STUDY.md](SF18_GAP_STUDY.md)):** 8 threads = ~6.4× node
     throughput but 2.5× *longer* wall-clock to the same nominal depth (the
     d32 tree grows 14×) — fixed-depth is not a comparable unit under SMP.
  3. **Highest engineering risk** — needs a race-tolerant (lockless XOR-key) TT,
     per-thread history/killers, a clean multi-thread stop, and forces
     **concurrency-1 gauntlets** (cc=4 would oversubscribe the CPU and make
     10+0.1 wall-clock meaningless) → ~4× throughput hit per box. Concurrency
     bugs are the worst class to debug — and #37 (a *single*-threaded desync) is
     still open.
- **Decision:** revisit after the HCE roadmap is mined out (or alongside NNUE).

## Deferred / parked ideas

One-liners; full detail + evidence in the archives
([2.0](BACKLOG-archive-2.0.md) / [2.1](BACKLOG-archive-2.1.md)).

- **#2 — King safety on top of mobility.** Shipped (t16) — see archive.
- **#3 — Continuation history.** 1-ply additive falsified (w16 neutral, w64 −9);
  flag off, kept in-tree (`experiment/continuation-history`).
- **#7 — LMP (late move pruning).** Deferred 2026-04, re-tested and
  **RE-PARKED 2026-07-14** (`candidate/lmp-r2` vs t34, two-machine pooled
  **−0.87 ± 10.33, LOS ≈ 46.6%** — AMD +4.17 ± 14.16 / Intel −6.60 ± 15.09, a
  sign-split that pools to dead-flat): the two-era verdict (−254→−56 vs t1;
  ≈0 vs t34 with the full t24–t33 ordering stack + the never-tested non-PV
  gate) shows ordering improvements converge LMP essentially to zero —
  Huginn's shallow tree is already thin without it. In-tree behind
  `ENABLE_LMP` (default OFF); a future revisit needs a genuinely new ordering
  signal (e.g. a working conthist redesign), not more threshold dialing.
- **#8 — Aspiration step (b), narrow-window search.** Deferred; folds into #17
  (t15 rejection — see archive; **re-opened 2026-07-13 as #17-r2**, the t15
  verdict predates the soundness-fix stack — see the live table).
- **#20 — Trapped-bishop eval pattern.** Tested 2026-07-03 (CPW locks,
  tuner-wired seeds 100/120 + 50/60) — **PARKED, clean two-machine neutral**
  (AMD +2.08/LOS 60.8%, Intel −5.21/LOS 24.7%, both noise-level, exactly the
  "~1–3 Elo, needs large N" outcome expected). Park-for-Texel-retune: a full
  re-tune with these params exposed is the better long-term path if revisited.
- **#26 — `board64[64]` piece-on-square cache.** Deferred (a prior sign-split;
  perf/cache changes carry more downside risk than gated eval terms).
- **#27 — Unorthodox early-queen PV** (d1d3 / d8d6). Deferred (evaluation).
- **#29 — Fifty-move-rule search blindness.** Direct terminal detection was
  kept on correctness; audit #53 reopens the propagated-score / TT-contamination
  gap. P2 (clock-scaled eval) remains deferred (near-inert at 10+0.1).
- **#32 — PEXT slider attacks.** Tested 2026-07-03/04 — **PARKED, 3–5%
  SLOWER on the AMD 7800X3D/Zen4 box** (not the expected win; memory-latency-
  bound hot path, not ALU-bound). Bit-identical node counts confirm
  correctness. Infrastructure (build-gated PEXT + verified magic fallback)
  kept behind the flag for a future revisit on different hardware.
- **Drawishness scaling (`mul[]`)** — opposite-coloured bishops →½,
  pawnless-minor-up→⅛ (KNNK etc. left to the existing `MaterialDraw()`
  short-circuit). Tested 2026-07-03 — **PARKED, two-machine flat neutral vs
  t23** (AMD +1.04/LOS 55.4%, Intel +4.52/LOS 71.9%), exactly as predicted:
  self-play vs an equal-strength opponent is the least sensitive test for a
  #5 conversion-weakness fix. **Revisit vs a weaker external anchor** (Stash
  11/12, the #5 draw-prone pairing) rather than self-play if pursued further.

## Conventions

- **Ship bar:** same-sign two-machine SPRT H1-accept (AMD + Intel), or tight
  cross-machine agreement for a small effect. Sign-splits are parked by default
  (t14 rook-on-7th was a logged exception, not the rule).
- **Complexity gate (fixed-depth vs fixed-time + ablation) — run BEFORE each new
  eval/search round.** Guards against over-engineering: eval terms that don't
  earn their noise, and pruning that trims real tactics ("trim the tree in
  vain"). *Context:* Huginn is NOT depth-starved by eval — it runs ~3.55 Mnps vs
  MTLChess's ~2.33 and prunes harder (R=4 + futility/razoring/IID vs their R=2/3,
  none), yet loses. So MTLChess's simpler-but-stronger eval is **correlation, not
  causation**: the gap is term *quality*/search *shape*, not eval *cost*. The
  failure modes that ARE real here: (a) mis-tuned/correlated terms add eval noise
  that also degrades move ordering + LMR/futility margins; (b) over-aggressive
  pruning reaches more depth on a tree missing the move that mattered. Diagnose,
  don't guess:
  1. **Fixed-depth vs fixed-time SPRT — the key test.** Gains at fixed-depth but
     loses at fixed-time → costs more speed than it returns (true over-engineering).
     Loses at both → the term is just wrong/noisy. Neutral at fixed-depth → adds
     nothing but risk. Buckets every change cleanly.
  2. **Periodic ablation sweep** — before adding term N+1, SPRT-disable an existing
     cluster (mobility, all pawn-structure). MTLChess proves these *can* be
     near-neutral; if removing one doesn't hurt, it's dead weight adding noise.
  3. **nps-per-term regression + average-depth telemetry** at the test TC — if
     breadth creeps up and depth drifts down with no Elo, the line is crossed.
  - **Pruning-soundness audit** (the "trim in vain" lever): periodically test a
    sound-lean config (R=3, lighter razoring) at fixed time. A gain means the
    tree-trimming is over-aggressive — likely worth more than any new eval term.
    (The 2026-07 pruning-stack audit — razoring, RFP, the #45 knobs — ran this
    exact play against t23; all four checked out sound or neutral. See archive.)
  - **Principle: fewer, better, sound > many, noisy, fighting.** "Demote breadth"
    (#41 + the MTLChess comparison, archive) is this gate applied.
- **Two-machine gauntlet workflow (#19, established):**
  `test_huginn_gauntlet.bat <tN|calib> [rounds] [cc]` auto-detects CPU vendor,
  tags PGNs `_intel`/`_amd`, runs SPRT [0,10] for baselines. Keep cc=4 on both
  boxes for poolable runs. Pentanomial pooling pairs games by `[Round]` tag.
- **Commit directly to `main`** (no feature branch unless asked); push only when
  asked. Gauntlet results: `gauntlet/*_<machine>.{log,pgn}` + a BACKLOG/commit
  summary carrying Elo/LOS/Ptnml.
- **Baselines are built per-machine** from the `baseline-tN` tag — never copy one
  box's binary to the other.
