"""
Diff two cc_analysis CSVs (e.g. t15 baseline vs t20) on the #41 metrics, to test
whether the #45 search fix changed the diagnosis ("gap is eval, not search/depth").

    python tools/compare_cc.py <baseline.csv> <new.csv>
    e.g. python tools/compare_cc.py tools/cc_analysis20260614.csv tools/cc_analysis_t20.csv

Prints, side by side with deltas: Huginn==SF match% (overall, by phase, tactical
vs quiet), mean cp-loss (fair fights), the over-optimism fingerprint
(hug_eval - SF's eval of Huginn's move) by phase, and mean search depth reached.

Fair fight = |material_cp| <= 200 and |sf_eval_cp| <= 300 (same as summarize_cc.py)
-- the subset where a cp-loss / optimism number is meaningful.
"""
import sys, csv

def num(x):
    try: return int(x)
    except Exception:
        try: return float(x)
        except Exception: return 0

def load(path):
    rows = list(csv.DictReader(open(path, newline="")))
    for r in rows:
        for k in ("hug_cploss","material_cp","sf_eval_cp","sf_eval_after_hug_cp",
                  "hug_eval_cp","hug_eq_sf","played_eq_sf","hug_depth"):
            r[k] = num(r.get(k, 0))
        r["tactical"] = (r.get("sf_best_is_capture")=="1" or r.get("sf_best_is_check")=="1")
        r["fair"] = abs(r["material_cp"])<=200 and abs(r["sf_eval_cp"])<=300
        r["optimism"] = r["hug_eval_cp"] - r["sf_eval_after_hug_cp"]
    return rows

def mean(xs):
    xs = list(xs)
    return sum(xs)/len(xs) if xs else 0.0

def metrics(rows):
    fair = [r for r in rows if r["fair"]]
    m = {}
    m["n"] = len(rows)
    m["n_fair"] = len(fair)
    m["match"] = mean(r["hug_eq_sf"] for r in rows)*100
    m["played_match"] = mean(r["played_eq_sf"] for r in rows)*100
    m["cploss_fair"] = mean(r["hug_cploss"] for r in fair)
    m["depth"] = mean(r["hug_depth"] for r in rows)
    for ph in ("opening","middlegame","endgame"):
        ph_all = [r for r in rows if r["phase"]==ph]
        ph_fair = [r for r in fair if r["phase"]==ph]
        m[f"match_{ph}"] = mean(r["hug_eq_sf"] for r in ph_all)*100
        m[f"opt_{ph}"]   = mean(r["optimism"] for r in ph_fair)
    for lab in ("tactical","quiet"):
        flag = (lab=="tactical")
        sub = [r for r in rows if r["tactical"]==flag]
        m[f"match_{lab}"] = mean(r["hug_eq_sf"] for r in sub)*100
    return m

def main():
    if len(sys.argv) < 3:
        print("usage: python tools/compare_cc.py <baseline.csv> <new.csv>"); return
    a_path, b_path = sys.argv[1], sys.argv[2]
    A, B = metrics(load(a_path)), metrics(load(b_path))
    la = a_path.split("\\")[-1].split("/")[-1]
    lb = b_path.split("\\")[-1].split("/")[-1]

    def row(label, key, pct=False, signed=False):
        av, bv = A[key], B[key]
        d = bv - av
        u = "%" if pct else "cp" if "opt" in key or "cploss" in key else ""
        ds = f"{d:+.1f}{u}"
        print(f"  {label:28} {av:8.1f}{u:2}  {bv:8.1f}{u:2}   {ds:>10}")

    print(f"\n# {la}  (A)  vs  {lb}  (B)")
    print(f"  positions A={A['n']} (fair {A['n_fair']})   B={B['n']} (fair {B['n_fair']})")
    print(f"\n  {'metric':28} {'A(t15)':>10}  {'B(t20)':>10}   {'delta B-A':>10}")
    print("  " + "-"*64)
    row("Huginn==SF  (all)",        "match", pct=True)
    row("  middlegame match",       "match_middlegame", pct=True)
    row("  endgame match",          "match_endgame", pct=True)
    row("  opening match",          "match_opening", pct=True)
    row("  TACTICAL match",         "match_tactical", pct=True)
    row("  QUIET match",            "match_quiet", pct=True)
    print("  " + "-"*64)
    row("cp-loss (fair, lower=better)", "cploss_fair")
    print("  " + "-"*64)
    print("  over-optimism by phase (hug_eval - SF-truth-of-hug-move; lower=better):")
    row("  opening optimism",       "opt_opening")
    row("  middlegame optimism",    "opt_middlegame")
    row("  endgame optimism",       "opt_endgame")
    print("  " + "-"*64)
    row("mean search depth",        "depth")
    print()
    print("  READ: #45 was a SEARCH fix. If the #41 'gap is eval, not search' story")
    print("  still holds, match% and optimism barely move. If they improve markedly")
    print("  (esp. quiet match up, middlegame optimism down), the diagnosis flips and")
    print("  the roadmap should weight search over eval.")

if __name__ == "__main__":
    main()
