"""
Summarize the cc_analysis.csv produced by analyze_played_vs_engines.py into a
weakness report: where does Huginn 2.1 lose the most cp vs Stockfish?

    python summarize_cc.py [tools/cc_analysis.csv]

Reports overall match rates + mean cp-loss, broken down by game phase and by
tactical-vs-quiet, both on all positions and on "fair fights" (roughly balanced,
non-decided positions where a cp-loss is most meaningful), then the worst N.
"""
import sys, csv, statistics as st

def num(x):
    try: return int(x)
    except Exception: return 0

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "tools/cc_analysis.csv"
    rows = list(csv.DictReader(open(path, newline="")))
    for r in rows:
        r["hug_cploss"]=num(r["hug_cploss"]); r["material_cp"]=num(r["material_cp"])
        r["sf_eval_cp"]=num(r["sf_eval_cp"]); r["hug_eq_sf"]=num(r["hug_eq_sf"])
        r["played_eq_sf"]=num(r["played_eq_sf"])
        r["tactical"] = (r["sf_best_is_capture"]=="1" or r["sf_best_is_check"]=="1")
    n=len(rows)
    def stats(rs):
        if not rs: return (0,0,0,0)
        l=[x["hug_cploss"] for x in rs]
        return (len(rs), sum(x["hug_eq_sf"] for x in rs)/len(rs)*100,
                sum(l)/len(rs), st.median(l))
    print(f"positions: {n}")
    N,eq,mean,med = stats(rows)
    print(f"Huginn==SF: {eq:.0f}%   played==SF: {sum(r['played_eq_sf'] for r in rows)/n*100:.0f}%")
    print(f"Huginn cp-loss  mean {mean:.1f}  median {med:.0f}  max {max(r['hug_cploss'] for r in rows)}")

    print("\n-- mean cp-loss by phase --")
    for ph in ("opening","middlegame","endgame"):
        N,eq,mean,med = stats([r for r in rows if r["phase"]==ph])
        print(f"  {ph:11} n={N:5} Huginn==SF {eq:3.0f}%  mean cp-loss {mean:5.1f}")
    print("\n-- tactical vs quiet (SF best is capture/check?) --")
    for lab,flag in (("tactical",True),("quiet",False)):
        N,eq,mean,med = stats([r for r in rows if r["tactical"]==flag])
        print(f"  {lab:8} n={N:5} Huginn==SF {eq:3.0f}%  mean cp-loss {mean:5.1f}")

    fair=[r for r in rows if abs(r["material_cp"])<=200 and abs(r["sf_eval_cp"])<=300]
    print(f"\n-- FAIR FIGHTS only (|material|<=200, |SF eval|<=300): n={len(fair)} --")
    for ph in ("opening","middlegame","endgame"):
        N,eq,mean,med = stats([r for r in fair if r["phase"]==ph])
        if N: print(f"  {ph:11} n={N:5} Huginn==SF {eq:3.0f}%  mean cp-loss {mean:5.1f}")

    print("\n-- worst 20 Huginn cp-losses (fair fights) --")
    for r in sorted(fair, key=lambda r:-r["hug_cploss"])[:20]:
        print(f"  -{r['hug_cploss']:4}cp {r['phase'][:3]} {r['side']} HUG {r['hug_best']:6} vs SF {r['sf_best']:6} "
              f"played={r['played']:6} | {r['fen']}")

if __name__ == "__main__":
    main()
