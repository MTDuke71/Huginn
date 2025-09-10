#!/usr/bin/env python3
"""
Huginn Chess Engine Performance Benchmark Suite
Tracks engine improvements over time with standardized test positions
"""

import subprocess
import time
import json
import os
from datetime import datetime

class HuginnBenchmark:
    def __init__(self, engine_path="build/bin/Release/huginn.exe"):
        self.engine_path = engine_path
        self.test_positions = [
            # Standard opening position
            {
                "name": "Starting Position",
                "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                "depth": 5
            },
            # Tactical position (WAC.1)
            {
                "name": "WAC.001 - White to Move",
                "fen": "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1",
                "depth": 6
            },
            # Middlegame position
            {
                "name": "Middlegame Position",
                "fen": "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
                "depth": 5
            },
            # Endgame position
            {
                "name": "King and Pawn Endgame",
                "fen": "8/8/8/8/8/8/k1K5/8 w - - 0 1",
                "depth": 8
            }
        ]
    
    def run_engine_test(self, fen, depth):
        """Run engine on a position and collect performance metrics"""
        commands = f"uci\nsetoption name OwnBook value false\nposition fen {fen}\ngo depth {depth}\nquit\n"
        
        start_time = time.time()
        try:
            result = subprocess.run(
                [self.engine_path],
                input=commands,
                text=True,
                capture_output=True,
                timeout=30
            )
            end_time = time.time()
            
            # Parse engine output
            lines = result.stdout.split('\n')
            nodes = 0
            time_ms = 0
            score = None
            bestmove = None
            
            for line in lines:
                if 'info depth' in line and f'depth {depth}' in line:
                    parts = line.split()
                    for i, part in enumerate(parts):
                        if part == 'nodes' and i + 1 < len(parts):
                            nodes = int(parts[i + 1])
                        elif part == 'time' and i + 1 < len(parts):
                            time_ms = int(parts[i + 1])
                        elif part == 'score' and i + 2 < len(parts):
                            if parts[i + 1] == 'cp':
                                score = int(parts[i + 2])
                elif line.startswith('bestmove'):
                    bestmove = line.split()[1] if len(line.split()) > 1 else None
            
            nps = nodes / (time_ms / 1000.0) if time_ms > 0 else 0
            wall_time = end_time - start_time
            
            return {
                "success": True,
                "nodes": nodes,
                "time_ms": time_ms,
                "wall_time_s": wall_time,
                "nps": nps,
                "score": score,
                "bestmove": bestmove
            }
            
        except subprocess.TimeoutExpired:
            return {"success": False, "error": "timeout"}
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def run_full_benchmark(self):
        """Run complete benchmark suite"""
        print("🚀 Starting Huginn Performance Benchmark")
        print("=" * 50)
        
        results = {
            "timestamp": datetime.now().isoformat(),
            "engine_path": self.engine_path,
            "positions": []
        }
        
        total_nodes = 0
        total_time = 0
        
        for pos in self.test_positions:
            print(f"\n📍 Testing: {pos['name']}")
            print(f"   FEN: {pos['fen']}")
            print(f"   Depth: {pos['depth']}")
            
            result = self.run_engine_test(pos['fen'], pos['depth'])
            
            if result['success']:
                print(f"   ✅ Nodes: {result['nodes']:,}")
                print(f"   ⏱️  Time: {result['time_ms']}ms")
                print(f"   🚀 NPS: {result['nps']:,.0f}")
                print(f"   🎯 Score: {result['score']}")
                print(f"   🏁 Best: {result['bestmove']}")
                
                total_nodes += result['nodes']
                total_time += result['time_ms']
            else:
                print(f"   ❌ Failed: {result.get('error', 'unknown')}")
            
            pos_result = {**pos, **result}
            results["positions"].append(pos_result)
        
        # Overall statistics
        overall_nps = total_nodes / (total_time / 1000.0) if total_time > 0 else 0
        results["summary"] = {
            "total_nodes": total_nodes,
            "total_time_ms": total_time,
            "average_nps": overall_nps
        }
        
        print("\n" + "=" * 50)
        print("📊 BENCHMARK SUMMARY")
        print("=" * 50)
        print(f"Total Nodes Searched: {total_nodes:,}")
        print(f"Total Time: {total_time/1000:.2f}s")
        print(f"Average NPS: {overall_nps:,.0f}")
        
        return results
    
    def save_results(self, results, filename=None):
        """Save benchmark results to file"""
        if filename is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"benchmark_results_{timestamp}.json"
        
        with open(filename, 'w') as f:
            json.dump(results, f, indent=2)
        
        print(f"\n💾 Results saved to: {filename}")
        return filename

if __name__ == "__main__":
    benchmark = HuginnBenchmark()
    results = benchmark.run_full_benchmark()
    benchmark.save_results(results)
