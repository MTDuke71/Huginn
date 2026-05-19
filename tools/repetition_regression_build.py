"""Step 6 of BACKLOG #28: turn confirmed REAL_BUG cases into a targeted,
history-aware regression set.

The bug is history-dependent (current Huginn does NOT repeat from the
bare FEN; it only repeats with the full game move stack replayed), so a
plain `bm` EPD cannot reproduce it. The regression fixture therefore
stores start_fen + the full UCI move list to replay, plus:

  - clincher : the repetition-clinching move Huginn must STOP playing
  - sf_best  : Stockfish's move from the bare position (oracle)
  - sf_cp    : Stockfish score, clincher-side POV (objective "won by")
  - subclass : 'alt_exists'       SF best != clincher -> a clearly
                                   winning non-repeating move exists;
                                   regression criterion = Huginn must
                                   not output `clincher`.
               'history_dependent' SF best == clincher -> the move is
                                   objectively best from the bare
                                   position but completes a 3-fold given
                                   game history; needs explicit root
                                   repetition detection, not a bm test.

Outputs:
  tools/repetition_regression.json   (machine-readable fixture)
  tools/repetition_findings.md       (human summary for BACKLOG #28)
"""
import json
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
RESULTS = json.load(open(REPO / "tools" / "repetition_research_results.json"))
CANDS = {f"{r['source']}-R{r['round']}": r
         for r in json.load(open(REPO / "tools"
                                  / "repetition_candidates.json"))}

real = [r for r in RESULTS if r["verdict"] == "REAL_BUG"]
fixed = [r for r in RESULTS if r["verdict"] == "FIXED_BY_T6"]
artifact = [r for r in RESULTS if r["verdict"] == "ARTIFACT"]

fixture = []
for r in real:
    c = CANDS[r["id"]]
    subclass = ("history_dependent"
                if r["sf_bestmove"] == r["clincher"] else "alt_exists")
    fixture.append({
        "id": r["id"],
        "subclass": subclass,
        "start_fen": c["start_fen"],
        "moves_uci": c["moves_uci"],
        "pre_fen": r["pre_fen"],
        "clincher": r["clincher"],
        "sf_best": r["sf_bestmove"],
        "sf_cp_clincher_pov": r["sf_cp_clincher_pov"],
        "pgn_reported_cp": r["pgn_reported_cp"],
    })

(REPO / "tools" / "repetition_regression.json").write_text(
    json.dumps(fixture, indent=2), encoding="utf-8")

alt = [f for f in fixture if f["subclass"] == "alt_exists"]
hist = [f for f in fixture if f["subclass"] == "history_dependent"]

lines = []
lines.append("# BACKLOG #28 — PGN repetition conversion findings\n")
lines.append("Source: 400 games across `huginn_vs_t5_intel.pgn` + "
             "`huginn_vs_t5_amd.pgn` (baseline-t6 gauntlet).\n")
lines.append("## Pipeline\n")
lines.append("- 238 games ended in 3-fold repetition.")
lines.append("- 126 of those: Huginn_current played the clinching move.")
lines.append("- 20 of *those* had Huginn's own reported eval >= +300cp "
             "**and** a legal non-repeating alternative (candidates).")
lines.append("- Each candidate re-searched: current Huginn with full game "
             "history replayed (300ms) + Stockfish 18 depth-24 oracle "
             "on the bare pre-repetition position.\n")
lines.append("## Verdict tally (20 candidates)\n")
lines.append(f"- **REAL_BUG: {len(real)}** — Huginn still clinches the "
             "repetition AND Stockfish confirms the position is won.")
lines.append(f"- **FIXED_BY_T6: {len(fixed)}** — Stockfish confirms won, "
             "but current Huginn no longer repeats (t6 root-demotion "
             "now steers away). Regression-prevention guards.")
lines.append(f"- **ARTIFACT: {len(artifact)}** — Stockfish says the "
             "position is ~0/drawn; Huginn's large eval was inflated. "
             "Not a thrown win; discard.\n")
lines.append("## Key correction\n")
lines.append("An early harness bug (sending `quit` before `bestmove`, "
             "aborting the background `go`) made Huginn *look* like it "
             "still repeated everywhere. With a streaming UCI driver, "
             "most candidates are FIXED_BY_T6 or ARTIFACT. The genuine "
             f"residual bug is **{len(real)}/20**.\n")
lines.append("## Regression set (REAL_BUG)\n")
lines.append(f"### alt_exists ({len(alt)}) — a clearly winning "
             "non-repeating move exists; Huginn must stop emitting the "
             "clincher\n")
lines.append("| id | clincher | SF best | SF cp | PGN cp |")
lines.append("|---|---|---|---|---|")
for f in alt:
    lines.append(f"| {f['id']} | {f['clincher']} | {f['sf_best']} | "
                 f"{f['sf_cp_clincher_pov']} | {f['pgn_reported_cp']} |")
lines.append("")
lines.append(f"### history_dependent ({len(hist)}) — SF's own best move "
             "IS the clincher; objectively best from the bare position "
             "but a 3-fold given game history. Needs explicit root "
             "repetition detection, not a bm test.\n")
lines.append("| id | clincher (=SF best) | SF cp | PGN cp |")
lines.append("|---|---|---|---|")
for f in hist:
    lines.append(f"| {f['id']} | {f['clincher']} | "
                 f"{f['sf_cp_clincher_pov']} | {f['pgn_reported_cp']} |")
lines.append("")
lines.append("## Recommended next action\n")
lines.append("The `alt_exists` subset is the clean, fixable bug class: "
             "in won positions with a winning non-repeating move, Huginn "
             "still selects the repetition. The t6 root-demotion fires "
             "only at >= +300cp *and* only when the repetition is the "
             "immediate root move; these cases either dip under threshold "
             "during the shuffle or repeat one ply deeper. Candidate "
             "fix: lower/condition the demotion and detect claimable "
             "repetition at the root explicitly (BACKLOG #28 candidate "
             "fixes 2-3), validated against this fixture + a t6 gauntlet.")

(REPO / "tools" / "repetition_findings.md").write_text(
    "\n".join(lines) + "\n", encoding="utf-8")

print(f"REAL_BUG={len(real)} (alt_exists={len(alt)}, "
      f"history_dependent={len(hist)})  "
      f"FIXED_BY_T6={len(fixed)}  ARTIFACT={len(artifact)}")
print("wrote tools/repetition_regression.json, tools/repetition_findings.md")
