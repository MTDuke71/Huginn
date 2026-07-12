# Stockfish 18 gap study (2026-07-12)

Where, exactly, does Huginn lose to a top engine — openings, speed, search
shape, or evaluation? Three experiments on the AMD 7800X3D box, all against
**Stockfish 18** (bmi2 build, 1 thread / 64 MB unless stated). Huginn is the
**threats-r2 candidate arm** (startpos d14 = 8,298,375 — the build in SPRT at
the time; the conclusions are baseline-generation-independent).

Artifacts: `gauntlet/exp_*` (PGNs with per-move evals, raw per-depth
captures, console transcripts).

## 1. Match + same-seed control: the openings are innocent

40 games Huginn vs SF18 (10+0.1, `noob_3moves.epd`, `-srand 20260712`), then
40 games **SF18 vs itself on the identical opening sequence** (same seed →
same 20 openings, both colors).

| Match | W | L | D | Score |
|---|--:|--:|--:|--:|
| Huginn vs SF18 | 0 | 40 | 0 | 0.0% |
| SF18 vs SF18 (control) | 1 | 0 | 39 | 51.25% |

- Every loss by mate; **zero** time losses / illegal moves / protocol issues.
- The control's one decisive game was Round 11 — the only opening whose
  out-of-book eval was materially unbalanced (+0.67 for White); even
  SF-as-Black failed to hold it once. Every other opening: dead draw
  between equals. **The 0–40 is pure play-strength differential.**

## 2. Where the games are lost: moves ~6–14

From the PGN eval comments: the move at which SF's own eval first reached
**+1.00** (its POV), per game. Openings exit book around move 4–6.

**All 40 games crossed +1.00, at median move 14 (mean 14.0, earliest 5,
latest 26).** SF's first out-of-book eval was near-equal every game (worst
±0.76) — so Huginn concedes a winning advantage within ~8–10 moves of
leaving book, every time, via steady ~0.1–0.3 cp/move bleed rather than
single blunders. Mate was announced around move 38 and delivered by ~45 on
average. (Full 40-row crossing table: reproduce with
`scratchpad sf_cross_table.py` logic over `exp_huginn_vs_sf18.pgn`; the
per-game numbers are in the conversation-of-record and the PGN itself.)

Against ~2500-rated peers this early-middlegame bleed is survivable and
often reversed later; against SF18 a −1.00 at move 14 converts with zero
variance.

## 3. Kiwipete depth study: speed is solved, tree shape and eval are not

`go depth N` from Kiwipete, fresh process, 1t/64MB. Per-depth
nodes/time/score (side-to-move POV, White to move — negative = White worse):

| depth | Huginn nodes | ms | score | SF18 nodes | ms | score |
|------:|-------------:|---:|:------|-----------:|---:|:------|
| 1 | 843 | 0 | +68 | 104 | 1 | −128 |
| 6 | 34,107 | 17 | −53 | 451 | 1 | −128 |
| 10 | 293,922 | 145 | −53 | 10,775 | 12 | −169 |
| 13 | 1,846,915 | 883 | −83 | 36,077 | 35 | −171 |
| 16 | 13,750,128 | 6,442 | −84 | 122,426 | 112 | −189 |
| 24 | — | — | — | 1,841,823 | 1,651 | −206 |
| 32 | — | — | — | 19,286,090 | 16,582 | −222 |

(Full tables: `exp_kiwipete_huginn.txt`, `exp_kiwipete_sf18.txt`.)

- **Raw speed: Huginn is ~2× faster.** ~2.1 Mnps vs SF's ~1.2 Mnps (NNUE
  inference is expensive per node). Speed-side parity is done — confirmed
  again.
- **Tree shape: 112× at equal depth.** Huginn d16 = 13.75M nodes; SF d16 =
  122K. Effective branching factor **~1.90/ply (Huginn) vs ~1.37/ply (SF)**
  — that compounding gap is why SF's d32 costs what Huginn's d16 does.
- **Eval fidelity: SF's depth-1 eval (−128) is closer to the truth (−222)
  than Huginn's depth-16 eval (−84).** Huginn's static eval carries the
  wrong *sign* until depth 6 (+68 at d1). NNUE's depth-1 ≈ HCE's depth-16.
- **PV agreement:** both settle on the two known tries (Bxa6 / dxe6);
  Huginn's d16 PV matches SF's d8 skeleton nearly move for move.

## 4. SF18 at 8 threads: why fixed-depth breaks under SMP

Same Kiwipete run, `Threads=8`:

| depth | SF 1t nodes | ms | SF 8t nodes | ms | wall speedup |
|------:|------------:|---:|------------:|---:|-------------:|
| 16 | 122,426 | 112 | 504,695 | 69 | 1.6× |
| 20 | 493,560 | 462 | 9,022,332 | 1,143 | 0.4× |
| 24 | 1,841,823 | 1,651 | 32,182,226 | 4,142 | 0.4× |
| 32 | 19,286,090 | 16,582 | 276,973,705 | 37,313 | 0.4× |

- Throughput scales well: **~7.4 Mnps wall (≈6.4× effective from 8 cores)**.
- But the same *nominal depth* takes **2.5× longer**: Lazy SMP threads
  search wide/redundant — d32 costs 277M nodes vs 19M (14× fatter), and the
  resulting "d32" embeds more verification (eval −230 vs −222). Depth stops
  being a comparable unit across thread counts. (64 MB hash under-sizes a
  277M-node search; a bigger TT tightens the tail but not the shape.)
- **This is BACKLOG #40's catch, measured:** SMP buys strength at fixed
  *time*, never fixed *depth*, and it destroys the byte-identical
  fixed-depth signatures the ship discipline depends on.

## Conclusions (feeding the backlog)

1. **Openings and protocol robustness contribute nothing to the gap** — the
   #54/#55/#56 hardening held up perfectly under 40 straight losses.
2. The gap decomposes into exactly the two "big levers":
   **search selectivity** (EBF 1.90 → toward 1.37: better ordering, more
   aggressive LMR, singular extensions — the #57-family of work) and
   **evaluation fidelity** (NNUE d1 ≈ HCE d16 — BACKLOG #39). Raw speed is
   a solved problem and further nps work is near-worthless by comparison.
3. The losses concentrate in the first ~10 out-of-book moves — the exact
   territory eval terms (threats-r2 in SPRT now, king safety, mobility)
   operate in.
