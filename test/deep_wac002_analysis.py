#!/usr/bin/env python3
"""
Deep analysis of WAC.002 with different time controls
"""

import subprocess
import time
import os

def test_position_at_time(search_time):
    """Test the WAC.002 position with a specific time control"""
    print(f"\n{'='*60}")
    print(f"Testing WAC.002 with {search_time} seconds search time")
    print(f"{'='*60}")
    
    engine_path = "../build/bin/Release/huginn.exe"
    if not os.path.exists(engine_path):
        print(f"Engine not found at {engine_path}")
        return
    
    fen = "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1"
    
    try:
        # Start engine process
        engine = subprocess.Popen(
            engine_path,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        
        # Initialize UCI
        engine.stdin.write("uci\n")
        engine.stdin.flush()
        
        # Wait for uciok
        while True:
            line = engine.stdout.readline().strip()
            print(f"Engine: {line}")
            if line == "uciok":
                break
        
        # Set position
        engine.stdin.write(f"position fen {fen}\n")
        engine.stdin.flush()
        
        # Start search
        engine.stdin.write(f"go movetime {search_time * 1000}\n")
        engine.stdin.flush()
        
        best_move = None
        best_score = None
        max_depth = 0
        
        # Read search info
        while True:
            line = engine.stdout.readline().strip()
            print(f"Engine: {line}")
            
            if line.startswith("info"):
                if "depth" in line and "score" in line and "pv" in line:
                    parts = line.split()
                    try:
                        depth_idx = parts.index("depth")
                        depth = int(parts[depth_idx + 1])
                        if depth > max_depth:
                            max_depth = depth
                            
                        score_idx = parts.index("score")
                        score_type = parts[score_idx + 1]
                        score_value = parts[score_idx + 2]
                        best_score = f"{score_type} {score_value}"
                    except (ValueError, IndexError):
                        pass
            
            elif line.startswith("bestmove"):
                best_move = line.split()[1]
                break
        
        # Quit engine
        engine.stdin.write("quit\n")
        engine.stdin.flush()
        engine.wait()
        
        print(f"\nResults for {search_time}s search:")
        print(f"Best move: {best_move}")
        print(f"Max depth: {max_depth}")
        print(f"Final score: {best_score}")
        
        return best_move, max_depth, best_score
        
    except Exception as e:
        print(f"Error: {e}")
        if engine:
            engine.terminate()
        return None, None, None

def main():
    print("WAC.002 Deep Analysis")
    print("Position: 8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -")
    print("Expected: Rxb2")
    print("Stockfish prefers: Rb7") 
    print("Let's see what Huginn thinks at different time controls...")
    
    time_controls = [3, 10, 30, 60]
    results = []
    
    for time_control in time_controls:
        move, depth, score = test_position_at_time(time_control)
        results.append((time_control, move, depth, score))
        time.sleep(1)  # Brief pause between tests
    
    print(f"\n{'='*80}")
    print("SUMMARY OF RESULTS")
    print(f"{'='*80}")
    print(f"{'Time':<8} {'Move':<8} {'Depth':<8} {'Score':<15}")
    print("-" * 50)
    
    for time_control, move, depth, score in results:
        print(f"{time_control}s{'':<5} {move or 'N/A':<8} {depth or 'N/A':<8} {score or 'N/A':<15}")
    
    print(f"\nAnalysis:")
    print("- Does longer thinking time find Rxb2?")
    print("- Is c4c3 consistent across time controls?")
    print("- What depth is needed to see the tactical point?")

if __name__ == "__main__":
    main()
