"""Capture a fixed-depth signature from a Huginn UCI binary.

Keeps stdin open until `bestmove` arrives, so the #56 mid-search input pump
never sees a buffered `quit` and cuts the search short.

usage: python sig.py <exe> [depth] [fen-or-'startpos'] [setoption-lines...]
"""
import subprocess
import sys

exe = sys.argv[1]
depth = sys.argv[2] if len(sys.argv) > 2 else "14"
pos = sys.argv[3] if len(sys.argv) > 3 else "startpos"
extra = sys.argv[4:]

posline = "position startpos" if pos == "startpos" else f"position fen {pos}"

p = subprocess.Popen(exe, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                     text=True, bufsize=1)
p.stdin.write("uci\n")
for line in extra:
    p.stdin.write(line + "\n")
p.stdin.write("isready\n")
p.stdin.write(f"{posline}\ngo depth {depth}\n")
p.stdin.flush()

last = ""
best = ""
while True:
    line = p.stdout.readline()
    if not line:
        break
    line = line.strip()
    if line.startswith("info depth"):
        last = line
    if line.startswith("bestmove"):
        best = line
        break

p.stdin.write("quit\n")
p.stdin.flush()
try:
    p.wait(timeout=5)
except subprocess.TimeoutExpired:
    p.kill()

fields = last.split()


def field(name):
    return fields[fields.index(name) + 1] if name in fields else "?"


print(f"depth={field('depth')} seldepth={field('seldepth')} "
      f"score={' '.join(fields[fields.index('score') + 1:fields.index('score') + 3]) if 'score' in fields else '?'} "
      f"nodes={int(field('nodes')):,} time={field('time')}ms nps={int(field('nps')):,} {best}")
