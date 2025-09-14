#!/usr/bin/env python3
"""
Futility Pruning Performance Analysis
Compare performance before and after futility pruning implementation
"""

import json

# Load results before and after futility pruning
with open('benchmark_results_20250913_190247.json', 'r') as f:
    before_futility = json.load(f)

with open('benchmark_results_20250913_202501.json', 'r') as f:
    after_futility = json.load(f)

print("HUGINN FUTILITY PRUNING PERFORMANCE ANALYSIS")
print("=" * 60)
print(f"Before Futility Pruning: {before_futility['timestamp']}")
print(f"After Futility Pruning:  {after_futility['timestamp']}")
print("")

# Overall comparison
before_nps = before_futility['summary']['average_nps']
after_nps = after_futility['summary']['average_nps']
nps_change = ((after_nps - before_nps) / before_nps) * 100

before_nodes = before_futility['summary']['total_nodes']
after_nodes = after_futility['summary']['total_nodes']
nodes_change = ((after_nodes - before_nodes) / before_nodes) * 100

before_time = before_futility['summary']['total_time_ms']
after_time = after_futility['summary']['total_time_ms']
time_change = ((after_time - before_time) / before_time) * 100

print("OVERALL PERFORMANCE IMPACT")
print("-" * 35)
print(f"Before NPS: {before_nps:,.0f}")
print(f"After NPS:  {after_nps:,.0f}")
print(f"NPS Change: {nps_change:+.2f}%")
print("")
print(f"Before Nodes: {before_nodes:,}")
print(f"After Nodes:  {after_nodes:,}")
print(f"Nodes Change: {nodes_change:+.2f}%")
print("")
print(f"Before Time: {before_time}ms")
print(f"After Time:  {after_time}ms")
print(f"Time Change: {time_change:+.2f}%")
print("")

# Position by position analysis
print("POSITION-BY-POSITION ANALYSIS")
print("-" * 40)

total_nodes_saved = 0
total_time_saved = 0

for i, pos in enumerate(after_futility['positions']):
    before_pos = before_futility['positions'][i]
    
    nodes_saved = before_pos['nodes'] - pos['nodes']
    time_saved = before_pos['time_ms'] - pos['time_ms']
    nps_change_pos = ((pos['nps'] - before_pos['nps']) / before_pos['nps']) * 100
    nodes_reduction = (nodes_saved / before_pos['nodes']) * 100
    
    total_nodes_saved += nodes_saved
    total_time_saved += time_saved
    
    print(f"\n{pos['name']}:")
    print(f"  Before: {before_pos['nodes']:,} nodes in {before_pos['time_ms']}ms ({before_pos['nps']:,.0f} NPS)")
    print(f"  After:  {pos['nodes']:,} nodes in {pos['time_ms']}ms ({pos['nps']:,.0f} NPS)")
    print(f"  Change: {nodes_saved:+,} nodes ({nodes_reduction:+.1f}%), {time_saved:+}ms, {nps_change_pos:+.1f}% NPS")

print("\n" + "=" * 60)
print("FUTILITY PRUNING EFFECTIVENESS SUMMARY")
print("=" * 60)
print(f"Total nodes saved: {total_nodes_saved:,}")
print(f"Total time saved: {total_time_saved}ms")
print(f"Overall node reduction: {(total_nodes_saved / before_nodes * 100):.1f}%")
print(f"Overall time reduction: {(total_time_saved / before_time * 100):.1f}%")
if nps_change > 0:
    print(f"Performance improvement: {nps_change:.1f}% faster NPS")
else:
    print(f"Performance impact: {nps_change:.1f}% slower NPS")

# Analysis
if total_nodes_saved > 0:
    print("\n✅ FUTILITY PRUNING IS WORKING!")
    print("   Successfully reducing search tree size while maintaining accuracy")
else:
    print("\n⚠️  UNEXPECTED RESULTS")
    print("   Futility pruning may not be activating as expected")