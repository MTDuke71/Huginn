# Huginn domain glossary

Names for concepts the code relies on. Architecture reviews and refactors use
these terms; keep them current when a concept is added or sharpened.

- **Driver**: the search thread whose move is played and whose `bestmove`/`info`
  output the GUI reads. Thread 0. Sole writer to stdout; owns the time policy
  (soft bound, iteration gate). Its Threads=1 schedule is what the bit-identity
  contract pins.
- **Helper**: a Lazy SMP worker (#40) spawned alongside the driver. No output,
  no time budget of its own (released by the group stop), diversified so its TT
  entries are worth something to the others.
- **Quiet fixed-depth search**: a search used as an oracle, not a player —
  datagen labels (#39), tests, bench. Fixed depth, no stdin polling, no UCI
  output. Built by `SearchInfo::for_quiet_fixed_depth`.
- **Diversification**: the mechanism keeping helpers off each other's trees:
  iteration skipping (per-helper size/phase schedules) and aspiration skew.
  Keyed on `thread_id`; exactly zero for thread 0, which is what keeps the
  driver's signature untouched.
- **Signature gate**: the verification protocol for refactors that claim
  bit-identical Threads=1 behaviour — fixed-depth `info` line + `bestmove`
  captured by `tools/smp/signature.py` must match the baseline binary exactly.
