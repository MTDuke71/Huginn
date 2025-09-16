#!/usr/bin/env python3
"""
Compare our depth 4 results with expected results to find discrepancies
"""

# Expected results from the user
expected = {
    "a1b1": 83348, "a1c1": 83263, "a1d1": 79695, "a2a3": 94405, "a2a4": 90978,
    "b2b3": 81066, "c3a4": 91447, "c3b1": 84773, "c3b5": 81498, "c3d1": 84782,
    "d2c1": 83037, "d2e3": 90274, "d2f4": 84869, "d2g5": 87951, "d2h6": 82323,
    "d5d6": 79551, "d5e6": 97464, "e1c1": 79803, "e1d1": 79989, "e1f1": 77887,
    "e1g1": 86975, "e2a6": 69334, "e2b5": 79739, "e2c4": 84835, "e2d1": 74963,
    "e2d3": 85119, "e2f1": 88728, "e5c4": 77752, "e5c6": 83885, "e5d3": 77431,
    "e5d7": 93913, "e5f7": 88799, "e5g4": 79912, "e5g6": 83866, "f3d3": 83727,
    "f3e3": 92505, "f3f4": 90488, "f3f5": 104992, "f3f6": 77838, "f3g3": 94461,
    "f3g4": 92037, "f3h3": 98524, "f3h5": 95034, "g2g3": 77468, "g2g4": 75677,
    "g2h3": 82759, "h1f1": 81563, "h1g1": 84876
}

# Our actual results
actual = {
    "a2a3": 94405, "b2b3": 81066, "g2g3": 77468, "d5d6": 79551, "a2a4": 90978,
    "g2g4": 75677, "g2h3": 82759, "d5e6": 97464, "c3b1": 84773, "c3d1": 84782,
    "c3a4": 91447, "c3b5": 81498, "e5d3": 77431, "e5c4": 77752, "e5g4": 79912,
    "e5c6": 83885, "e5g6": 87080, "e5d7": 88586, "e5f7": 84310, "d2c1": 77350,
    "d2e3": 84572, "d2f4": 79185, "d2g5": 78963, "d2h6": 77123, "e2a6": 65035,
    "e2d1": 69159, "e2f1": 82855, "e2d3": 81353, "e2c4": 81125, "e2b5": 74757,
    "a1b1": 77662, "a1c1": 77577, "a1d1": 74023, "h1f1": 75884, "h1g1": 79207,
    "f3h3": 90881, "f3f6": 71478, "f3d3": 78139, "f3e3": 84878, "f3g3": 86821,
    "f3f4": 82881, "f3g4": 86266, "f3f5": 90274, "f3h5": 85698, "e1d1": 74444,
    "e1f1": 72498, "e1g1": 81287, "e1c1": 74131
}

print("=== MOVE-BY-MOVE COMPARISON ===\n")
print(f"{'Move':<8} {'Expected':<10} {'Actual':<10} {'Difference':<12} {'Status'}")
print("-" * 50)

total_diff = 0
problematic_moves = []

for move in sorted(expected.keys()):
    exp_val = expected[move]
    act_val = actual.get(move, 0)
    diff = act_val - exp_val
    total_diff += diff
    
    status = "✅ MATCH" if diff == 0 else "❌ DIFF"
    if diff != 0:
        problematic_moves.append((move, exp_val, act_val, diff))
    
    print(f"{move:<8} {exp_val:<10} {act_val:<10} {diff:+<12} {status}")

print(f"\nTotal expected: {sum(expected.values())}")
print(f"Total actual: {sum(actual.values())}")
print(f"Total difference: {total_diff:+}")

print(f"\n=== PROBLEMATIC MOVES ({len(problematic_moves)}) ===")
for move, exp, act, diff in sorted(problematic_moves, key=lambda x: abs(x[3]), reverse=True):
    print(f"{move}: {exp} → {act} ({diff:+})")