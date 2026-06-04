#!/usr/bin/env python3
"""
WAC-201 (Win At Chess, reduced) Test Suite for Huginn Chess Engine
Tests engine tactical ability against the *sound* WAC subset (wac201.epd).

The classic WAC-300 derives from the 1958 "Win at Chess" book; over the
decades many of its positions were found to be busted (dual solutions,
flat-out wrong "best" moves, or trivial by modern engine standards). This
runs the curated wac201.epd set (200 positions, the dubious ones removed),
a more honest tactical-resolution signal. Same engine harness and output
format as wac_test.py; only the default EPD file + position caps differ.
"""

import subprocess
import time
import re
import os
import sys
from datetime import datetime
from typing import List, Tuple, Dict, Optional

class WACPosition:
    def __init__(self, line: str):
        """Parse EPD line into position data"""
        parts = line.strip().split()
        # FEN is first 4 parts in EPD format (missing halfmove and fullmove)
        fen_parts = parts[:4]
        # Add default halfmove and fullmove counters
        fen_parts.extend(['0', '1'])
        self.fen = ' '.join(fen_parts)
        # Side to move ('w' or 'b') — needed to expand castling SAN to UCI.
        self.side_to_move = parts[1] if len(parts) > 1 else 'w'

        # Find best move
        bm_match = re.search(r'bm\s+([^;]+)', line)
        self.best_moves = []
        if bm_match:
            # Handle multiple best moves separated by space or comma
            moves_str = bm_match.group(1).strip()
            self.best_moves = [move.strip() for move in re.split(r'[,\s]+', moves_str) if move.strip()]

        # Find ID
        id_match = re.search(r'id\s+"([^"]+)"', line)
        self.id = id_match.group(1) if id_match else "Unknown"
        
    def __str__(self):
        return f"{self.id}: {self.fen} (best: {', '.join(self.best_moves)})"

class WACTester:
    def __init__(self, engine_path: str = "../build/msvc-x64-release/bin/Release/huginn.exe",
                 epd_file: str = "wac201.epd", max_positions: Optional[int] = None,
                 failed_positions_file: Optional[str] = None,
                 threads: int = 1):
        self.engine_path = engine_path
        self.epd_file = epd_file
        self.max_positions = max_positions
        self.failed_positions_file = failed_positions_file
        self.threads = threads
        self.results = []
        self.log_file = None
        
    def load_positions(self) -> List[WACPosition]:
        """Load positions from EPD file"""
        positions = []
        try:
            with open(self.epd_file, 'r') as f:
                for line_num, line in enumerate(f, 1):
                    if line.strip() and not line.startswith('#'):
                        try:
                            pos = WACPosition(line)
                            positions.append(pos)
                            if self.max_positions and len(positions) >= self.max_positions:
                                break
                        except Exception as e:
                            print(f"Warning: Could not parse line {line_num}: {e}")
        except FileNotFoundError:
            print(f"Error: Could not find EPD file {self.epd_file}")
            sys.exit(1)
        
        return positions
    
    def load_failed_positions(self, failed_file: str) -> List[WACPosition]:
        """Load specific positions from a failed positions file"""
        positions = []
        try:
            # Read the failed position numbers
            with open(failed_file, 'r') as f:
                failed_numbers = []
                for line in f:
                    line = line.strip()
                    if line and line.isdigit():
                        failed_numbers.append(int(line))
            
            # Load all positions and filter by failed numbers
            with open(self.epd_file, 'r') as f:
                for line_num, line in enumerate(f, 1):
                    if line.strip() and not line.startswith('#'):
                        if line_num in failed_numbers:
                            try:
                                pos = WACPosition(line)
                                positions.append(pos)
                            except Exception as e:
                                print(f"Error parsing line {line_num}: {e}")
                                
        except FileNotFoundError:
            print(f"Failed positions file not found: {failed_file}")
        except Exception as e:
            print(f"Error loading failed positions: {e}")
            
        return positions
    
    def normalize_move(self, move: str) -> str:
        """Normalize move notation for comparison"""
        # Remove common suffixes and normalize
        move = move.strip().rstrip('+#!?')
        
        # Handle promotion moves - ensure lowercase promotion piece
        if len(move) >= 5 and move[4] in 'QRBN':
            move = move[:4] + move[4].lower()
        elif len(move) >= 5 and move[4] == '=':
            if len(move) >= 6 and move[5] in 'QRBN':
                move = move[:4] + move[5].lower()
        
        return move.lower()
    
    def _expand_castling(self, san: str, side_to_move: str):
        """Convert 'O-O' / 'O-O-O' (or '0-0' / '0-0-0') to its UCI form
        based on whose turn it is. Returns None if not a castling move."""
        m = san.strip().rstrip('+#!?').replace('0', 'O')
        if m == 'O-O':
            return 'e1g1' if side_to_move == 'w' else 'e8g8'
        if m == 'O-O-O':
            return 'e1c1' if side_to_move == 'w' else 'e8c8'
        return None

    def moves_match(self, engine_move: str, expected_moves: List[str],
                    position: 'WACPosition' = None) -> bool:
        """Check if engine move matches any expected move.

        `position` is used only to expand castling SAN (O-O / O-O-O) to its
        side-specific UCI form. Without it, castling expectations would never
        match the engine's UCI castling output (e1g1 etc).
        """
        if not engine_move:
            return False

        # Normalize engine move
        normalized_engine = self.normalize_move(engine_move)

        # Check against all expected moves
        for expected in expected_moves:
            # Castling: expand to UCI based on side-to-move and compare directly.
            if position is not None:
                castling_uci = self._expand_castling(expected, position.side_to_move)
                if castling_uci is not None:
                    if normalized_engine == castling_uci:
                        return True
                    continue  # don't fall through to SAN heuristics

            normalized_expected = self.normalize_move(expected)

            # Direct match
            if normalized_engine == normalized_expected:
                return True

            # Handle different notation formats
            if len(normalized_engine) == 4:  # Engine move like "g5g6"
                engine_from = normalized_engine[:2]
                engine_to = normalized_engine[2:4]

                # Case 1: Expected move is just destination (like "g6" for pawn move)
                if len(normalized_expected) == 2 and normalized_expected == engine_to:
                    return True

                # Case 2: Expected move is piece + destination (like "qg6", "kg1")
                if len(normalized_expected) == 3:
                    expected_piece = normalized_expected[0]
                    expected_dest = normalized_expected[1:3]
                    if expected_dest == engine_to:
                        return True

                # Case 3: Expected move includes source file (like "rxb2", "nf3")
                if len(normalized_expected) >= 3:
                    expected_dest = normalized_expected[-2:]
                    if expected_dest == engine_to:
                        return True

            # Handle case where expected is also in long algebraic (4 chars)
            elif len(normalized_expected) == 4:
                if normalized_engine == normalized_expected:
                    return True

        return False
    
    def test_position(self, position: WACPosition, search_time: int = 5) -> Dict:
        """Test engine on a single position"""
        print(f"\nTesting {position.id}")
        print(f"FEN: {position.fen}")
        print(f"Expected: {', '.join(position.best_moves)}")
        
        # Log to file
        self.log_file.write(f"\n{'='*60}\n")
        self.log_file.write(f"Testing {position.id}\n")
        self.log_file.write(f"FEN: {position.fen}\n")
        self.log_file.write(f"Expected best moves: {', '.join(position.best_moves)}\n")
        self.log_file.write(f"{'='*60}\n")
        
        # Drive UCI interactively with Popen instead of batching the whole
        # script to subprocess.run.
        #
        # Why: subprocess.run(input=...) writes all input then closes stdin.
        # Engines that poll stdin during search (e.g. Stockfish 18) treat
        # stdin-EOF as "abort and exit ASAP" and return a depth-0 move
        # before producing any real search output. The old code masked
        # this because Huginn only reads stdin between checkup intervals,
        # so the buffered EOF never reached its decision loop during the
        # actual search. With proper interactive UCI we write commands,
        # wait for `bestmove`, THEN send `quit` and close stdin.
        #
        # OwnBook is explicitly disabled so a book hit can't replace the
        # engine's actual tactical search on a WAC position. Threads is
        # set so the harness can drive multi-threaded engines for honest
        # cross-engine comparison; default is 1 (matches single-thread
        # gauntlet convention).
        engine_output = []
        best_move = None
        proc = None
        try:
            proc = subprocess.Popen(
                [self.engine_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,  # line buffered
                cwd=os.path.dirname(os.path.abspath(self.engine_path))
            )

            def send(line: str) -> None:
                proc.stdin.write(line + "\n")
                proc.stdin.flush()

            send("uci")
            send("setoption name OwnBook value false")
            if self.threads and self.threads > 1:
                send(f"setoption name Threads value {self.threads}")
            send("ucinewgame")
            send("isready")
            send(f"position fen {position.fen}")
            send(f"go movetime {search_time * 1000}")

            # Read until `bestmove ...` or hard deadline. The deadline is a
            # safety net for engines that hang; honest `go movetime N` ends
            # within N ms plus a small overhead.
            import time as _time
            deadline = _time.monotonic() + search_time + 15
            while _time.monotonic() < deadline:
                line = proc.stdout.readline()
                if not line:
                    # Engine closed stdout — process likely exited.
                    break
                line = line.rstrip()
                if line:
                    engine_output.append(line)
                    self.log_file.write(f"<<< {line}\n")
                if line.startswith("bestmove"):
                    m = re.search(r'bestmove\s+(\S+)', line)
                    if m:
                        best_move = m.group(1)
                    break

            # Clean shutdown: send quit only AFTER we have a bestmove, so
            # the search has fully completed. If we never saw bestmove
            # (deadline / EOF) we still try quit to give the engine a
            # graceful exit before fallback kill.
            try:
                send("quit")
            except (BrokenPipeError, OSError):
                pass
            try:
                proc.stdin.close()
            except (BrokenPipeError, OSError):
                pass

            try:
                proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=2)

        except Exception as e:
            # Defensive: a hung engine or pipe error shouldn't kill the
            # whole sweep. Log and return a clean failure for this position.
            if proc is not None:
                try:
                    proc.kill()
                    proc.wait(timeout=2)
                except Exception:
                    pass
            print(f"Error analysing {position.id}: {e}")
            self.log_file.write(f"ERROR: {e}\n")
            return {
                'position': position,
                'engine_move': None,
                'expected_moves': position.best_moves,
                'success': False,
                'error': str(e),
                'engine_output': engine_output
            }

        # Echo to console (replaces the old per-line print loop above).
        for line in engine_output:
            print(f"Engine: {line}")

        # Analyze result
        success = False
        if best_move:
            success = self.moves_match(best_move, position.best_moves, position)

        # Log result
        status = "PASS" if success else "FAIL"
        print(f"Engine played: {best_move}")
        print(f"Result: {status}")

        self.log_file.write(f"\nRESULT: {status}\n")
        self.log_file.write(f"Engine move: {best_move}\n")
        self.log_file.write(f"Expected: {', '.join(position.best_moves)}\n")

        return {
            'position': position,
            'engine_move': best_move,
            'expected_moves': position.best_moves,
            'success': success,
            'engine_output': engine_output
        }
    
    def run_test_suite(self, search_time: int = 5):
        """Run the complete test suite"""
        if self.failed_positions_file:
            positions = self.load_failed_positions(self.failed_positions_file)
        else:
            positions = self.load_positions()
        
        if not positions:
            print("No positions loaded!")
            return
        
        print(f"Loaded {len(positions)} positions")
        if not self.failed_positions_file and self.max_positions:
            print(f"Testing first {min(len(positions), self.max_positions)} positions")
        
        # Create log file
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_filename = f"wac201_test_log_{timestamp}.txt"
        
        with open(log_filename, 'w') as self.log_file:
            self.log_file.write(f"WAC Test Suite - Huginn Chess Engine\n")
            self.log_file.write(f"Timestamp: {datetime.now()}\n")
            self.log_file.write(f"Engine: {self.engine_path}\n")
            self.log_file.write(f"Search time per position: {search_time} seconds\n")
            self.log_file.write(f"Testing {len(positions)} positions\n")
            self.log_file.write("\n")
            
            start_time = time.time()
            
            for i, position in enumerate(positions, 1):
                print(f"\n{'='*60}")
                print(f"Position {i}/{len(positions)}")
                
                result = self.test_position(position, search_time)
                self.results.append(result)
                
                # Progress update
                elapsed = time.time() - start_time
                remaining = len(positions) - i
                eta = (elapsed / i) * remaining if i > 0 else 0
                print(f"Elapsed: {elapsed:.1f}s, ETA: {eta:.1f}s")
            
            # Generate summary
            self.generate_summary(log_filename)
            
            # Offer interactive retest mode if there are failures
            if any(not r['success'] for r in self.results):
                print(f"\n{'='*60}")
                try:
                    offer_retest = input("Some positions failed. Enter interactive retest mode? (y/n): ").strip().lower()
                except EOFError:
                    # No stdin (background / piped run) — skip retest, results
                    # are already saved to the log + failed-positions file.
                    offer_retest = 'n'
                if offer_retest in ['y', 'yes']:
                    self.interactive_retest()
    
    def generate_summary(self, log_filename: str):
        """Generate test summary"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r['success'])
        failed = total - passed
        
        print(f"\n{'='*60}")
        print("WAC TEST SUITE SUMMARY")
        print(f"{'='*60}")
        print(f"Total positions: {total}")
        print(f"Passed: {passed}")
        print(f"Failed: {failed}")
        print(f"Success rate: {(passed/total*100):.1f}%")
        
        self.failed_positions = []
        if failed > 0:
            print(f"\nFAILED POSITIONS:")
            print("-" * 40)
            for i, result in enumerate(self.results):
                if not result['success']:
                    pos = result['position']
                    engine_move = result.get('engine_move', 'None')
                    self.failed_positions.append((i, result))
                    print(f"{len(self.failed_positions)}. {pos.id}: Expected {', '.join(pos.best_moves)}, got {engine_move}")
        
        # Write summary to log file
        self.log_file.write(f"\n{'='*60}\n")
        self.log_file.write("FINAL SUMMARY\n")
        self.log_file.write(f"{'='*60}\n")
        self.log_file.write(f"Total positions: {total}\n")
        self.log_file.write(f"Passed: {passed}\n")
        self.log_file.write(f"Failed: {failed}\n")
        self.log_file.write(f"Success rate: {(passed/total*100):.1f}%\n")
        
        if failed > 0:
            self.log_file.write(f"\nFAILED POSITIONS:\n")
            for result in self.results:
                if not result['success']:
                    pos = result['position']
                    engine_move = result.get('engine_move', 'None')
                    self.log_file.write(f"{pos.id}: Expected {', '.join(pos.best_moves)}, got {engine_move}\n")
                    
            # Save failed position numbers to a separate file for easy rerun
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            failed_file = f"wac201_failed_positions_{timestamp}.txt"
            with open(failed_file, 'w') as f:
                f.write("# Failed WAC position line numbers\n")
                f.write("# Use: python wac201_test.py --failed-file wac201_failed_positions_XXXXXX.txt\n")
                f.write("# to retest only these positions\n\n")
                
                for i, result in enumerate(self.results, 1):
                    if not result['success']:
                        # Find the original line number in the EPD file
                        pos = result['position']
                        f.write(f"{self.get_position_line_number(pos)}\n")
                        
            print(f"Failed positions saved to: {failed_file}")
            self.log_file.write(f"\nFailed positions saved to: {failed_file}\n")
        
        print(f"\nDetailed log saved to: {log_filename}")
    
    def get_position_line_number(self, target_position: WACPosition) -> int:
        """Find the line number of a position in the EPD file"""
        try:
            with open(self.epd_file, 'r') as f:
                for line_num, line in enumerate(f, 1):
                    if line.strip() and not line.startswith('#'):
                        try:
                            pos = WACPosition(line)
                            # Match by ID or FEN
                            if pos.id == target_position.id or pos.fen == target_position.fen:
                                return line_num
                        except:
                            continue
        except:
            pass
        return 0  # Return 0 if not found
    
    def show_interactive_help(self):
        """Show detailed help for interactive mode"""
        print(f"\n{'='*60}")
        print("INTERACTIVE RETEST MODE - HELP")
        print(f"{'='*60}")
        print("This mode allows you to retest failed positions with custom time controls.")
        print()
        print("Commands:")
        print("  1-N    : Select a specific failed position to retest")
        print("  all    : Retest all failed positions with the same time control")
        print("  help/h : Show this help message")
        print("  quit/q : Exit interactive mode")
        print()
        print("When retesting:")
        print("  - You can specify any search time (in seconds)")
        print("  - Longer times may help the engine find the correct move")
        print("  - Each retest creates a detailed log file")
        print("  - If a position is solved, you can update the main results")
        print()
        print("Tips:")
        print("  - Try 10-30 seconds for tactical positions")
        print("  - Some positions may require 60+ seconds for complex tactics")
        print("  - The engine shows its thinking process in real-time")
        print("  - Check the PV (principal variation) to understand the engine's plan")
        print(f"{'='*60}")
    
    def interactive_retest(self):
        """Interactive mode to retest failed positions"""
        if not hasattr(self, 'failed_positions') or not self.failed_positions:
            print("No failed positions to retest!")
            return
        
        print(f"\n{'='*60}")
        print("INTERACTIVE RETEST MODE")
        print(f"{'='*60}")
        
        while True:
            print(f"\nFailed positions available for retesting:")
            for i, (orig_idx, result) in enumerate(self.failed_positions, 1):
                pos = result['position']
                engine_move = result.get('engine_move', 'None')
                print(f"{i}. {pos.id}: Expected {', '.join(pos.best_moves)}, got {engine_move}")
            
            print(f"\nOptions:")
            print(f"  1-{len(self.failed_positions)}: Select position to retest")
            print(f"  'all': Retest all failed positions")
            print(f"  'help' or 'h': Show detailed help")
            print(f"  'quit' or 'q': Exit interactive mode")
            
            choice = input("\nEnter your choice: ").strip().lower()
            
            if choice in ['quit', 'q', 'exit']:
                break
            elif choice == 'all':
                self.retest_all_failed()
            elif choice in ['help', 'h']:
                self.show_interactive_help()
            else:
                try:
                    pos_num = int(choice)
                    if 1 <= pos_num <= len(self.failed_positions):
                        self.retest_single_position(pos_num - 1)
                    else:
                        print(f"Invalid position number. Please enter 1-{len(self.failed_positions)}")
                except ValueError:
                    print("Invalid input. Please enter a number, 'all', or 'quit'")
    
    def retest_single_position(self, failed_idx: int):
        """Retest a single failed position with custom time"""
        orig_idx, orig_result = self.failed_positions[failed_idx]
        position = orig_result['position']
        
        print(f"\n{'='*50}")
        print(f"RETESTING: {position.id}")
        print(f"{'='*50}")
        print(f"FEN: {position.fen}")
        print(f"Expected: {', '.join(position.best_moves)}")
        print(f"Previous result: {orig_result.get('engine_move', 'None')}")
        
        while True:
            time_input = input(f"\nEnter search time in seconds (default 10, or 'back' to return): ").strip()
            
            if time_input.lower() in ['back', 'b']:
                return
            
            if time_input == '':
                search_time = 10
                break
            
            try:
                search_time = int(time_input)
                if search_time > 0:
                    break
                else:
                    print("Please enter a positive number")
            except ValueError:
                print("Please enter a valid number")
        
        print(f"\nRetesting {position.id} with {search_time} seconds...")
        
        # Create temporary log file for this retest
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        retest_log = f"wac201_retest_{position.id.replace('.', '_')}_{timestamp}.txt"
        
        with open(retest_log, 'w') as self.log_file:
            self.log_file.write(f"WAC Retest - {position.id}\n")
            self.log_file.write(f"Timestamp: {datetime.now()}\n")
            self.log_file.write(f"Search time: {search_time} seconds\n\n")
            
            result = self.test_position(position, search_time)
            
            # Show result
            if result['success']:
                print(f"\n🎉 SUCCESS! Engine found: {result['engine_move']}")
                
                # Ask if user wants to update the main results
                update = input("Update main test results with this success? (y/n): ").strip().lower()
                if update in ['y', 'yes']:
                    # Update the original result
                    self.results[orig_idx] = result
                    # Remove from failed positions
                    self.failed_positions.pop(failed_idx)
                    print("Main results updated!")
            else:
                print(f"\n❌ Still failed. Engine found: {result.get('engine_move', 'None')}")
            
            print(f"\nDetailed log saved to: {retest_log}")
    
    def retest_all_failed(self):
        """Retest all failed positions with custom time"""
        if not self.failed_positions:
            print("No failed positions to retest!")
            return
        
        while True:
            time_input = input(f"\nEnter search time in seconds for all positions (default 15): ").strip()
            
            if time_input == '':
                search_time = 15
                break
            
            try:
                search_time = int(time_input)
                if search_time > 0:
                    break
                else:
                    print("Please enter a positive number")
            except ValueError:
                print("Please enter a valid number")
        
        print(f"\nRetesting {len(self.failed_positions)} failed positions with {search_time} seconds each...")
        estimated_time = len(self.failed_positions) * search_time / 60
        print(f"Estimated time: {estimated_time:.1f} minutes")
        
        confirm = input("Continue? (y/n): ").strip().lower()
        if confirm not in ['y', 'yes']:
            return
        
        # Create log file for batch retest
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        retest_log = f"wac201_retest_all_{timestamp}.txt"
        
        updated_count = 0
        
        with open(retest_log, 'w') as self.log_file:
            self.log_file.write(f"WAC Batch Retest\n")
            self.log_file.write(f"Timestamp: {datetime.now()}\n")
            self.log_file.write(f"Search time: {search_time} seconds per position\n")
            self.log_file.write(f"Retesting {len(self.failed_positions)} positions\n\n")
            
            # Process in reverse order so indices don't shift when removing items
            for i in range(len(self.failed_positions) - 1, -1, -1):
                orig_idx, orig_result = self.failed_positions[i]
                position = orig_result['position']
                
                print(f"\nRetesting {position.id} ({len(self.failed_positions) - i}/{len(self.failed_positions)})...")
                
                result = self.test_position(position, search_time)
                
                if result['success']:
                    print(f"✅ SUCCESS! {position.id} now solved!")
                    # Update main results
                    self.results[orig_idx] = result
                    # Remove from failed positions
                    self.failed_positions.pop(i)
                    updated_count += 1
                else:
                    print(f"❌ {position.id} still failed")
        
        print(f"\n{'='*60}")
        print("RETEST SUMMARY")
        print(f"{'='*60}")
        print(f"Positions retested: {len(self.failed_positions) + updated_count}")
        print(f"New successes: {updated_count}")
        print(f"Still failing: {len(self.failed_positions)}")
        
        if updated_count > 0:
            total = len(self.results)
            passed = sum(1 for r in self.results if r['success'])
            print(f"\nUpdated overall success rate: {(passed/total*100):.1f}% ({passed}/{total})")
        
        print(f"\nDetailed log saved to: {retest_log}")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(
        description='WAC-201 (reduced, sound) Test Suite for Huginn Chess Engine',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python wac201_test.py            # Full reduced set (200 positions), 5 seconds each
  python wac201_test.py 50         # First 50 positions, 5 seconds each
  python wac201_test.py 200 -t 10  # Full set, 10 seconds each
  python wac201_test.py --failed-file wac_failed_positions_20250911_123456.txt
                                  # Retest only previously failed positions

Runs wac201.epd (the curated, more-sound subset). The script automatically
offers interactive retest mode for failed positions; failed positions are
saved to wac_failed_positions_TIMESTAMP.txt for easy re-testing.
        """)
    parser.add_argument('positions', nargs='?', type=int, default=200,
                       help='Number of positions to test (default: 200 = full reduced set, max: 200)')
    parser.add_argument('-t', '--time', type=int, default=5,
                       help='Search time per position in seconds (default: 5)')
    parser.add_argument('--failed-file', type=str,
                       help='File containing failed position line numbers to retest')
    parser.add_argument('--engine', type=str, default=None,
                       help='Path to UCI engine binary (default: ../build/msvc-x64-release/bin/Release/huginn.exe). '
                            'Any UCI-compliant engine works.')
    parser.add_argument('--threads', type=int, default=1,
                       help='UCI "Threads" option value (default: 1). '
                            'Huginn is single-threaded; bump for multi-threaded engines '
                            'like Stockfish when comparing tactical resolution at fixed time.')

    args = parser.parse_args()

    # Resolve the engine path. We honour --engine if given; otherwise the
    # WACTester default applies (huginn from this repo's release build).
    engine_kwargs = {'threads': args.threads}
    if args.engine:
        engine_kwargs['engine_path'] = args.engine

    if args.failed_file:
        # Retest mode for failed positions
        if not os.path.exists(args.failed_file):
            print(f"Error: Failed positions file not found: {args.failed_file}")
            return

        print(f"WAC Test Suite - Retesting failed positions from {args.failed_file}")
        print(f"Each position will be searched for {args.time} seconds")

        tester = WACTester(failed_positions_file=args.failed_file, **engine_kwargs)
        tester.run_test_suite(search_time=args.time)
        
    else:
        # Normal mode
        if args.positions <= 0 or args.positions > 200:
            print("Error: Number of positions must be between 1 and 200")
            return
        
        if args.time <= 0:
            print("Error: Search time must be positive")
            return
        
        print(f"WAC Test Suite - Testing first {args.positions} positions")
        print(f"Each position will be searched for {args.time} seconds")
        print(f"Estimated time: {args.positions * args.time / 60:.1f} minutes")
        
        # Show a tip for first-time users
        if args.positions < 200 and args.time == 5:
            print("\n💡 Tip: pass no count to run the full reduced set.")
            print("   'python wac201_test.py' runs all 200 sound positions.")
        
        tester = WACTester(max_positions=args.positions, **engine_kwargs)
        tester.run_test_suite(search_time=args.time)

if __name__ == "__main__":
    main()
