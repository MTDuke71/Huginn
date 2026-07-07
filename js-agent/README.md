# js-agent

Single-file JavaScript chess agent for a sandboxed stdin/stdout competition
(5s/move, 256MB, 0.5 CPU, no `fs`/`child_process`/network, Deno running in
Node-compat mode). See `agent.js`'s header comment for the protocol and the
two design lessons ported from Huginn (this repo's C++ engine): move-level
(not node-level) futility pruning, and root-level caution around repeating a
position while clearly winning.

`agent.js` is fully self-contained — it is the entire submission. Nothing
else in this directory is required to run it.

## Running it

```
node agent.js
```

Then feed it lines on stdin, one per move — a FEN, optionally followed by
`" moves <uci-move-list>"` — and it prints one UCI move (`e2e4`, `e7e8q`) per
line to stdout. The process is meant to stay alive for a whole game (the
transposition table, killers, and history heuristic persist across moves
within one process — but the board itself is always rebuilt fresh from the
FEN+history on every line, per spec).

**Protocol detail confirmed against a real arbiter-shaped input line**: when
`moves` is present, the leading FEN is the *current* position (it already
reflects every move in the list) — `moves` is the full game history from the
true start, needed for repetition detection, not a list to replay on top of
the given FEN. `buildPositionFromLine` reconstructs the position by replaying
`moves` from the standard start position, ignoring the leading FEN entirely
whenever `moves` is present (falling back to trusting the leading FEN
directly only if that replay fails, e.g. a non-standard game start).

## Dev-only self-tests

Both are gated behind CLI args so they never run during the real
stdin/stdout competition loop.

**Perft (movegen/make-unmake correctness):**
```
node agent.js --perft
```
Runs 5 reference positions (startpos to depth 4 and 5, the Kiwipete
castling/en-passant/promotion stress position to depth 3 and 4, plus one
more tactical position) against published exact node counts. All 5 must
print `OK`.

**Eval sign/orientation sanity check:**
```
node agent.js --eval "<FEN>"
```
Prints `evaluate()`'s score from the side-to-move's point of view (positive
= good for whoever is to move). Useful for catching an inverted eval term —
the classic first-build bug.

## Testing notes

- Verified via a persistent-process protocol smoke test: two `agent.js`
  processes played a full 60-ply game against each other over stdin/stdout,
  exactly as a real arbiter would drive it (one process per side, kept
  alive for the whole game, FEN+full-move-history on every line). All moves
  were legal and correctly formatted; per-move latency was a consistent
  ~4.3s, safely under the 5s hard cap.
- If you have other single-file agents around for sparring, the protocol is
  simple enough to script: send the same `<startFEN> moves <...>` line to
  whichever engine's turn it is, append its reply, repeat. Watch out for
  agents that don't actually honor the `moves` history — a couple of
  legacy ones tested here just replayed their fixed opening move regardless
  of position, which is a real (illegal-move) bug on their end.
