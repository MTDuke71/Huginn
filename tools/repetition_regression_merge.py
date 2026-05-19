"""Build the authoritative BACKLOG #28 Part 2 regression fixture as the
union of every confirmed REAL_BUG case across data sources, tagged with
provenance and deduped by pre_fen.

Sources:
  - t5_vs_t5    : the original committed fixture (repetition_regression_t5
                  .json), mined from the baseline-t6 *vs t5* gauntlet,
                  pre-Part-1 code.
  - t6_post_fix : repetition_research_results_t6.json + the matching
                  repetition_candidates_t6.json, mined from the *vs t6*
                  gauntlet whose Huginn_current already carries the
                  Part 1 fix (`a21a037`). These are residual bugs that
                  survive Part 1 in real play -- the highest-signal
                  Part 2 targets.

Output: tools/repetition_regression.json  (the master Part 2 fixture)
"""
import json
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
T = REPO / "tools"


def t6_real_rows():
    results = json.load(open(T / "repetition_research_results_t6.json"))
    cands = {f"{r['source']}-R{r['round']}": r
             for r in json.load(open(T / "repetition_candidates_t6.json"))}
    out = []
    for r in results:
        if r["verdict"] != "REAL_BUG":
            continue
        c = cands[r["id"]]
        out.append({
            "id": r["id"],
            "source_set": "t6_post_fix",
            "subclass": ("history_dependent"
                         if r["sf_bestmove"] == r["clincher"]
                         else "alt_exists"),
            "start_fen": c["start_fen"],
            "moves_uci": c["moves_uci"],
            "pre_fen": r["pre_fen"],
            "clincher": r["clincher"],
            "sf_best": r["sf_bestmove"],
            "sf_cp_clincher_pov": r["sf_cp_clincher_pov"],
            "pgn_reported_cp": r["pgn_reported_cp"],
        })
    return out


def t5_rows():
    rows = json.load(open(T / "repetition_regression_t5.json"))
    for r in rows:
        r.setdefault("source_set", "t5_vs_t5")
    return rows


def main():
    merged = {}
    # t5 first, then t6 — t6 (post-fix, higher signal) wins on dup pre_fen.
    for r in t5_rows() + t6_real_rows():
        merged[r["pre_fen"]] = r
    fixture = sorted(merged.values(),
                     key=lambda x: (x["source_set"], x["subclass"], x["id"]))

    (T / "repetition_regression.json").write_text(
        json.dumps(fixture, indent=2), encoding="utf-8")

    by_set = {}
    for r in fixture:
        by_set.setdefault(r["source_set"], []).append(r)
    print(f"master Part 2 fixture: {len(fixture)} cases "
          f"(deduped by pre_fen)")
    for s, rs in sorted(by_set.items()):
        sub = {}
        for r in rs:
            sub[r["subclass"]] = sub.get(r["subclass"], 0) + 1
        print(f"  {s:<13} {len(rs):>2}  {dict(sub)}")
    print("wrote tools/repetition_regression.json")


if __name__ == "__main__":
    main()
