#!/usr/bin/env python3
"""Huginn NNUE trainer (BACKLOG #39).

Architecture: (768 -> H) x 2 perspectives, shared feature weights, clipped
ReLU [0,1], linear 2H -> 1 producing centipawns from the side-to-move's point
of view. Loss: MSE in sigmoid space (the Texel loss, scale K=400) plus a
centipawn anchor term.

Data: "<score_cp_white_pov> <fen>" lines produced by `huginn datagen`.

Export: flat little-endian binary consumed by src/nnue.cpp --
    w1 : int16[768][H] feature-major
    b1 : int16[H]
    w2 : int16[2H]
    b2 : int32
Quantisation: A = 256 (feature/accumulator), B = 256 (output), and the net is
trained to emit cp/16, so the engine computes
    cp = (sum(clamp(acc, 0, 256) * w2) + b2) / 4096      (4096 = A*B/16)
These constants are duplicated in src/nnue.hpp; change them in both or the net
silently mis-scales.

Three failure modes worth knowing about before you spend arena time on a net
(all three cost FableR's lab a night, reported in the shared correspondence log,
and this script is written to make them visible):
  1. A clean loss curve is not a working net. Clamping the output layer too
     tightly caps the net's reachable range, and the loss still looks fine
     because most training positions are near-equal. `probe` mode evaluates
     known material imbalances directly -- run it before the arena, every time.
  2. Sigmoid-space loss saturates in the tails, so won positions get compressed
     (+900 reads as +90). The anchor term keeps centipawn calibration honest.
  3. int64 feature tensors at tens of millions of positions eat >10 GB. Feature
     indices are stored int16 and upcast per batch.

Usage:
    python tools/train_nnue.py preprocess nnue_data/gen --npz nnue_data/train.npz
    python tools/train_nnue.py train --npz nnue_data/train.npz --out nnue/huginn.nnue
    python tools/train_nnue.py probe --net nnue/huginn.nnue
"""

import argparse
import glob
import os
import struct
import sys
import time

import numpy as np

# Feature layout. Must match feature_index() in src/nnue.hpp exactly:
#     (rel_colour * 6 + piece_type0) * 64 + rel_square
# where the black perspective flips the colour bit and mirrors the square
# vertically (sq ^ 56). Piece order P N B R Q K matches PieceType 1..6.
PIECES = {c: i for i, c in enumerate("PNBRQKpnbrqk")}
PAD = 768   # padding feature index -> the embedding's zero row
MAXF = 32   # max pieces on a legal board


def fen_features(board, stm):
    """Return (white_perspective_features, black_perspective_features, white_to_move)."""
    wf, bf = [], []
    rank, file = 7, 0
    for ch in board:
        if ch == "/":
            rank -= 1
            file = 0
        elif ch.isdigit():
            file += int(ch)
        else:
            p = PIECES[ch]
            colour = 0 if p < 6 else 1
            pt = p % 6
            sq = rank * 8 + file
            wf.append((colour * 6 + pt) * 64 + sq)
            bf.append(((1 - colour) * 6 + pt) * 64 + (sq ^ 56))
            file += 1
    return wf, bf, stm == "w"


def preprocess(prefix, out_path):
    """Two passes: count, then fill preallocated arrays.

    Building Python lists-of-lists first (the obvious way) costs ~300 bytes per
    row and OOMs somewhere north of 20M positions. Counting first is one extra
    cheap pass and keeps peak memory at the size of the final arrays.
    """
    files = sorted(glob.glob(prefix + ".*.txt"))
    files = [f for f in files if not f.endswith(".manifest.txt")]
    if not files:
        sys.exit("no files matching %s.*.txt" % prefix)

    t0 = time.time()
    total = 0
    for path in files:
        with open(path, "rb") as f:
            total += sum(1 for _ in f)
    print("%d files, %d raw lines (%.0fs)" % (len(files), total, time.time() - t0))

    stm = np.full((total, MAXF), PAD, dtype=np.int16)
    nstm = np.full((total, MAXF), PAD, dtype=np.int16)
    labels = np.zeros(total, dtype=np.float32)

    n = 0
    t0 = time.time()
    truncated = False
    for path in files:
        if truncated:
            break
        with open(path) as f:
            for line in f:
                # The counting pass and this pass can disagree if the corpus is
                # still being appended to (reading a live datagen run). Stop at
                # the allocated size rather than growing the array mid-fill.
                if n >= total:
                    truncated = True
                    break
                parts = line.split(" ", 2)
                if len(parts) < 3:
                    continue
                try:
                    cp = int(parts[0])
                except ValueError:
                    continue
                wf, bf, white_to_move = fen_features(parts[1], parts[2][0])
                if len(wf) > MAXF:
                    continue
                if white_to_move:
                    s_f, n_f, s_cp = wf, bf, cp
                else:
                    s_f, n_f, s_cp = bf, wf, -cp
                stm[n, : len(s_f)] = s_f
                nstm[n, : len(n_f)] = n_f
                labels[n] = s_cp
                n += 1
                if n % 2_000_000 == 0:
                    print("  %.1fM positions, %.0fs" % (n / 1e6, time.time() - t0), flush=True)

    if truncated:
        print("note: input grew during preprocessing; kept the first %d positions" % total)
    print("kept %d positions in %.0fs" % (n, time.time() - t0))
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    np.savez(out_path, stm=stm[:n], nstm=nstm[:n], labels=labels[:n])
    print("saved %s" % out_path)


def build_net(torch, nn, H):
    class Net(nn.Module):
        def __init__(self):
            super().__init__()
            self.ft = nn.Embedding(769, H, padding_idx=PAD)
            self.ft_bias = nn.Parameter(torch.zeros(H))
            self.out = nn.Linear(2 * H, 1)
            nn.init.uniform_(self.ft.weight, -0.05, 0.05)
            with torch.no_grad():
                self.ft.weight[PAD].zero_()

        def forward(self, s_rows, ns_rows):
            a = torch.clamp(self.ft(s_rows).sum(dim=1) + self.ft_bias, 0, 1)
            b = torch.clamp(self.ft(ns_rows).sum(dim=1) + self.ft_bias, 0, 1)
            # OUT_SCALE = 16: the head emits cp/16 so the quantised output
            # weights stay small while the reachable centipawn range stays wide.
            return self.out(torch.cat([a, b], dim=1)).squeeze(1) * 16.0

    return Net()


def train(npz_path, epochs, batch, out_path, lr, H, val_frac):
    import torch
    import torch.nn as nn

    dev = "cuda" if torch.cuda.is_available() else "cpu"
    print("device: %s" % dev)
    if dev == "cuda":
        print("gpu: %s" % torch.cuda.get_device_name(0))

    d = np.load(npz_path)
    stm = torch.from_numpy(d["stm"])
    nstm = torch.from_numpy(d["nstm"])
    labels = torch.from_numpy(d["labels"])
    n = labels.shape[0]

    # Held-out split, taken from the tail so it is a contiguous block of games
    # rather than positions interleaved with their own neighbours -- successive
    # plies of one game are near-duplicates, and a random split would leak them
    # across the boundary and report a flatteringly low validation loss.
    n_val = int(n * val_frac)
    n_train = n - n_val
    print("%d positions: %d train / %d val" % (n, n_train, n_val))

    net = build_net(torch, nn, H).to(dev)
    opt = torch.optim.Adam(net.parameters(), lr=lr)
    scale = 400.0
    tgt_sig = torch.sigmoid(labels / scale)

    def loss_for(idx):
        y = net(stm[idx].to(dev).long(), nstm[idx].to(dev).long())
        t = labels[idx].to(dev)
        sig = torch.mean((torch.sigmoid(y / scale) - tgt_sig[idx].to(dev)) ** 2)
        anchor = torch.mean(((y - t.clamp(-1200, 1200)) / 800) ** 2)
        return sig + 0.15 * anchor, sig

    steps = (n_train + batch - 1) // batch
    for ep in range(1, epochs + 1):
        perm = torch.randperm(n_train)
        t0 = time.time()
        tot = 0.0
        for s in range(steps):
            idx = perm[s * batch : (s + 1) * batch]
            loss, _ = loss_for(idx)
            opt.zero_grad()
            loss.backward()
            opt.step()
            with torch.no_grad():
                # Clamp to the quantisation range: A=256 means a weight of 0.5
                # is the largest that survives int16 accumulation across 32
                # pieces without risking overflow.
                net.ft.weight.clamp_(-0.49, 0.49)
                net.ft.weight[PAD].zero_()
                net.ft_bias.clamp_(-0.49, 0.49)
                net.out.weight.clamp_(-4.0, 4.0)
            tot += loss.item()

        msg = "epoch %d: train %.6f" % (ep, tot / steps)
        if n_val:
            with torch.no_grad():
                vs = []
                for s in range(0, n_val, batch):
                    idx = torch.arange(n_train + s, min(n_train + s + batch, n))
                    _, sig = loss_for(idx)
                    vs.append(sig.item())
                msg += "  val-sig %.6f" % (sum(vs) / len(vs))
        print("%s  (%.0fs)" % (msg, time.time() - t0), flush=True)
        export(net, out_path)
        print("  checkpoint -> %s" % out_path, flush=True)


def export(net, out_path):
    import torch

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    with torch.no_grad():
        w1 = net.ft.weight.cpu().numpy()[:768]
        b1 = net.ft_bias.cpu().numpy()
        w2 = net.out.weight.cpu().numpy().ravel()
        b2 = float(net.out.bias.cpu().numpy()[0])
    A, B = 256.0, 256.0
    w1q = np.clip(np.round(w1 * A), -32767, 32767).astype("<i2")
    b1q = np.clip(np.round(b1 * A), -32767, 32767).astype("<i2")
    w2q = np.clip(np.round(w2 * B), -32767, 32767).astype("<i2")
    b2q = np.int32(round(b2 * A * B))
    with open(out_path, "wb") as f:
        f.write(w1q.tobytes())
        f.write(b1q.tobytes())
        f.write(w2q.tobytes())
        f.write(struct.pack("<i", b2q))


# --------------------------------------------------------------------------
# probe: evaluate known positions straight out of the quantised file, using the
# same integer arithmetic src/nnue.cpp uses. This is the "probe material
# directly" check -- it catches a silently range-capped net that a loss curve
# will happily hide.
# --------------------------------------------------------------------------

PROBE_POSITIONS = [
    ("startpos, equal",            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -",   0,   60),
    ("white up a queen",           "4k3/8/8/8/8/8/8/3QK3 w - -",                            700, 2000),
    ("white up a rook",            "4k3/8/8/8/8/8/8/3RK3 w - -",                            350, 1200),
    ("white up a knight",          "4k3/8/8/8/8/8/8/3NK3 w - -",                             -1,  400),
    ("KB vs K (dead draw)",        "4k3/8/8/8/8/8/8/3BK3 w - -",                           -100,  100),
    ("KP vs K (white winning)",    "4k3/8/8/8/8/4P3/8/4K3 w - -",                             0, 1200),
    ("black up a queen",           "3qk3/8/8/8/8/8/8/4K3 w - -",                         -2000, -700),
]


def probe(net_path, H):
    size = os.path.getsize(net_path)
    expect = 2 * (768 * H + H + 2 * H) + 4
    if size != expect:
        sys.exit("net is %d bytes, expected %d for H=%d" % (size, expect, H))
    raw = open(net_path, "rb").read()
    w1 = np.frombuffer(raw, "<i2", 768 * H, 0).reshape(768, H).astype(np.int32)
    b1 = np.frombuffer(raw, "<i2", H, 2 * 768 * H).astype(np.int32)
    w2 = np.frombuffer(raw, "<i2", 2 * H, 2 * (768 * H + H)).astype(np.int32)
    b2 = int(np.frombuffer(raw, "<i4", 1, 2 * (768 * H + H + 2 * H))[0])

    print("net %s  (768->%d)x2   w1 range [%d, %d]  w2 range [%d, %d]  b2 %d"
          % (net_path, H, w1.min(), w1.max(), w2.min(), w2.max(), b2))
    print()

    failures = 0
    for name, fen, lo, hi in PROBE_POSITIONS:
        board, stm = fen.split(" ")[0], fen.split(" ")[1]
        wf, bf, white_to_move = fen_features(board, stm)
        s_f, n_f = (wf, bf) if white_to_move else (bf, wf)
        acc_s = b1 + w1[s_f].sum(axis=0)
        acc_n = b1 + w1[n_f].sum(axis=0)
        total = int((np.clip(acc_s, 0, 256) * w2[:H]).sum()
                    + (np.clip(acc_n, 0, 256) * w2[H:]).sum())
        cp_stm = (total + b2) // 4096
        cp_white = cp_stm if white_to_move else -cp_stm
        ok = lo <= cp_white <= hi
        failures += not ok
        print("  %-28s %+6d cp (white POV)   expect [%+d, %+d]  %s"
              % (name, cp_white, lo, hi, "ok" if ok else "*** FAIL ***"))

    print()
    if failures:
        print("%d/%d probes FAILED -- do not spend arena time on this net."
              % (failures, len(PROBE_POSITIONS)))
        return 1
    print("all %d probes passed" % len(PROBE_POSITIONS))
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["preprocess", "train", "probe"])
    ap.add_argument("prefix", nargs="?", default="nnue_data/gen")
    ap.add_argument("--npz", default="nnue_data/train.npz")
    ap.add_argument("--epochs", type=int, default=12)
    ap.add_argument("--batch", type=int, default=16384)
    ap.add_argument("--lr", type=float, default=1e-3)
    ap.add_argument("--out", default="nnue/huginn.nnue")
    ap.add_argument("--net", default="nnue/huginn.nnue")
    ap.add_argument("--hidden", type=int, default=256)
    ap.add_argument("--val-frac", type=float, default=0.02)
    a = ap.parse_args()

    if a.mode == "preprocess":
        preprocess(a.prefix, a.npz)
    elif a.mode == "train":
        train(a.npz, a.epochs, a.batch, a.out, a.lr, a.hidden, a.val_frac)
    else:
        sys.exit(probe(a.net, a.hidden))


if __name__ == "__main__":
    main()
