#!/usr/bin/env python3
"""
Benchmark Comparison Analysis
Compare performance between different versions
"""

import json

# Load previous and current results
with open('benchmark_results_20250912_214636.json', 'r') as f:
    previous = json.load(f)

with open('benchmark_results_20250913_185642.json', 'r') as f:
    current = json.load(f)

print("HUGINN ENGINE PERFORMANCE COMPARISON")
print("=" * 60)
print(f"Previous: {previous['timestamp']}")
print(f"Current:  {current['timestamp']}")
print("")

# Overall comparison
prev_nps = previous['summary']['average_nps']
curr_nps = current['summary']['average_nps']
nps_improvement = ((curr_nps - prev_nps) / prev_nps) * 100

prev_nodes = previous['summary']['total_nodes']
curr_nodes = current['summary']['total_nodes']
nodes_change = ((curr_nodes - prev_nodes) / prev_nodes) * 100

print("OVERALL PERFORMANCE")
print("-" * 30)
print(f"Previous NPS: {prev_nps:,.0f}")
print(f"Current NPS:  {curr_nps:,.0f}")
print(f"NPS Change:   {nps_improvement:+.2f}%")
print("")
print(f"Previous Nodes: {prev_nodes:,}")
print(f"Current Nodes:  {curr_nodes:,}")
print(f"Nodes Change:   {nodes_change:+.2f}%")
print("")

# Position by position analysis
print("POSITION-BY-POSITION ANALYSIS")
print("-" * 40)

for i, pos in enumerate(current['positions']):
    prev_pos = previous['positions'][i]
    
    print(f"\n{pos['name']}:")
    print(f"  Previous: {prev_pos['nodes']:,} nodes, {prev_pos['nps']:,.0f} NPS")
    print(f"  Current:  {pos['nodes']:,} nodes, {pos['nps']:,.0f} NPS")
    
    nps_change = ((pos['nps'] - prev_pos['nps']) / prev_pos['nps']) * 100
    nodes_change = ((pos['nodes'] - prev_pos['nodes']) / prev_pos['nodes']) * 100
    
    print(f"  NPS Change:   {nps_change:+.2f}%")
    print(f"  Nodes Change: {nodes_change:+.2f}%")

print("\n" + "=" * 60)
print("ANALYSIS SUMMARY")
print("=" * 60)

if nps_improvement > 0:
    print(f"✅ Performance improved by {nps_improvement:.2f}%")
else:
    print(f"❌ Performance decreased by {abs(nps_improvement):.2f}%")

if nodes_change > 0:
    print(f"📈 Engine searches {abs(nodes_change):.2f}% more nodes (better search?)")
else:
    print(f"📉 Engine searches {abs(nodes_change):.2f}% fewer nodes (more efficient?)")

print("\nWith enhanced history heuristic:")
print("- Aging mechanism preserves useful historical information")
print("- Negative scoring penalizes poor moves")
print("- Periodic decay prevents score inflation")
print("- Better move ordering should improve search efficiency")