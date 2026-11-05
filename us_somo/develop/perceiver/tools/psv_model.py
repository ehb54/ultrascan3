#!/usr/bin/env python3
"""Can a residue's psv (vbar) be predicted from its perceived hybridization composition?

Mattia's suggestion for non-coded residues: derive a rough psv from similarity between the
hybridizations we perceive and those already present in somo.residue. The classical form of that
idea is an additive group-contribution model (Cohn & Edsall; Durchschlag & Zipper).

Test: fit  V = sum_t n_t * v_t   over the residues somo.residue already codes, where n_t is the
count of hybrid type t and V = vbar * MW is the residue's molar volume. Then see how well the
fitted per-type contributions reproduce the tabulated vbar. Compare against the cruder fallback
of averaging vbar within a residue-type group.

Honest evaluation: leave-one-out, so each residue is predicted by a model that never saw it.
No numpy here (not installed) - normal equations with ridge, solved by Gaussian elimination.
"""
import sys, math, re
from collections import defaultdict

RESFILE = sys.argv[1] if len(sys.argv) > 1 else "../../etc/somo.residue.new"
HYBFILE = sys.argv[2] if len(sys.argv) > 2 else "../../etc/somo.hybrid.new"

TYPE_NAME = {0: "amino acid", 1: "sugar", 2: "nucleotide", 3: "lipid",
             4: "detergent", 5: "other"}

def load_hybrid_mass(path):
    mw = {}
    for line in open(path):
        p = line.split()
        if len(p) >= 7:
            mw[p[1]] = float(p[2])
    return mw

def load_residues(path):
    """-> list of dicts: name, rtype, vbar, molvol, counts{hybrid: n}, natoms"""
    lines = open(path).read().splitlines()
    out, i = [], 0
    while i < len(lines):
        i += 1                                   # comment line
        if i >= len(lines):
            break
        h = lines[i].split()
        if len(h) < 7:
            continue
        i += 1
        name, rtype = h[0], int(h[1])
        molvol, natoms, nbeads = float(h[2]), int(h[4]), int(h[5])
        vbar = float(h[6])
        counts = defaultdict(int)
        for _ in range(natoms):
            if i >= len(lines):
                break
            p = lines[i].split(); i += 1
            if len(p) >= 2:
                counts[p[1]] += 1
        i += nbeads
        out.append(dict(name=name, rtype=rtype, vbar=vbar, molvol=molvol,
                        counts=dict(counts), natoms=natoms))
    return out

def solve(A, b, ridge):
    """Solve (A + ridge*I) x = b by Gaussian elimination with partial pivoting."""
    n = len(b)
    M = [row[:] + [b[k]] for k, row in enumerate(A)]
    for k in range(n):
        M[k][k] += ridge
    for c in range(n):
        piv = max(range(c, n), key=lambda r: abs(M[r][c]))
        if abs(M[piv][c]) < 1e-12:
            continue
        M[c], M[piv] = M[piv], M[c]
        pv = M[c][c]
        for r in range(n):
            if r == c:
                continue
            f = M[r][c] / pv
            if f:
                for cc in range(c, n + 1):
                    M[r][cc] -= f * M[c][cc]
    return [ (M[k][n] / M[k][k] if abs(M[k][k]) > 1e-12 else 0.0) for k in range(n) ]

def fit(rows, types, ridge=1e-3):
    """Least-squares per-type volume contributions from residues in 'rows'."""
    n = len(types)
    idx = {t: k for k, t in enumerate(types)}
    AtA = [[0.0] * n for _ in range(n)]
    Atb = [0.0] * n
    for r in rows:
        x = [0.0] * n
        for t, c in r["counts"].items():
            if t in idx:
                x[idx[t]] = c
        y = r["V"]
        for a in range(n):
            if not x[a]:
                continue
            Atb[a] += x[a] * y
            for bq in range(n):
                if x[bq]:
                    AtA[a][bq] += x[a] * x[bq]
    return solve(AtA, Atb, ridge), idx

def predict(r, w, idx):
    v = 0.0
    for t, c in r["counts"].items():
        if t in idx:
            v += c * w[idx[t]]
    return v

def main():
    mwt = load_hybrid_mass(HYBFILE)
    res = load_residues(RESFILE)

    # keep residues we can weigh and that carry a real vbar
    rows = []
    for r in res:
        mw = sum(mwt.get(t, 0.0) * c for t, c in r["counts"].items())
        missing = [t for t in r["counts"] if t not in mwt]
        if mw <= 0 or r["vbar"] <= 0 or missing:
            continue
        r["MW"] = mw
        r["V"] = r["vbar"] * mw          # cm^3/mol
        rows.append(r)

    types = sorted({t for r in rows for t in r["counts"]})
    print(f"residues usable: {len(rows)} of {len(res)}   distinct hybrid types: {len(types)}")
    print(f"vbar range: {min(r['vbar'] for r in rows):.3f} - {max(r['vbar'] for r in rows):.3f}")

    # ---- baseline 1: single global mean vbar
    gm = sum(r["vbar"] for r in rows) / len(rows)
    e_glob = [abs(r["vbar"] - gm) for r in rows]

    # ---- baseline 2: mean vbar within the residue's own type group (Mattia's option 3)
    by_type = defaultdict(list)
    for r in rows:
        by_type[r["rtype"]].append(r["vbar"])
    e_grp = []
    for r in rows:
        peers = [v for v in by_type[r["rtype"]] if v is not r["vbar"]] or by_type[r["rtype"]]
        e_grp.append(abs(r["vbar"] - sum(peers) / len(peers)))

    # ---- group-contribution model, leave-one-out
    e_gc = []
    for k, r in enumerate(rows):
        train = rows[:k] + rows[k + 1:]
        w, idx = fit(train, types)
        Vp = predict(r, w, idx)
        e_gc.append(abs(r["vbar"] - Vp / r["MW"]))

    def stats(e, label):
        e = sorted(e)
        mae = sum(e) / len(e)
        rmse = math.sqrt(sum(x * x for x in e) / len(e))
        p90 = e[int(0.9 * (len(e) - 1))]
        print(f"  {label:34s} MAE {mae:.4f}   RMSE {rmse:.4f}   p90 {p90:.4f}   max {e[-1]:.4f}")

    print("\nabsolute error in vbar (cm^3/g), leave-one-out where applicable:")
    stats(e_glob, "global mean vbar")
    stats(e_grp,  "mean of same residue-type group")
    stats(e_gc,   "hybrid group-contribution model")

    print("\nper residue-type group:")
    for t in sorted(by_type):
        v = by_type[t]
        spread = max(v) - min(v)
        print(f"  {TYPE_NAME.get(t,'type '+str(t)):12s} n={len(v):3d}  "
              f"vbar {min(v):.3f}-{max(v):.3f}  mean {sum(v)/len(v):.3f}  spread {spread:.3f}")

    # worst cases for the group-contribution model
    worst = sorted(zip(e_gc, rows), key=lambda x: -x[0])[:8]
    print("\nworst group-contribution predictions:")
    for e, r in worst:
        w, idx = fit([q for q in rows if q is not r], types)
        pv = predict(r, w, idx) / r["MW"]
        print(f"  {r['name']:6s} ({TYPE_NAME.get(r['rtype'],'?'):10s}) "
              f"tabulated {r['vbar']:.3f}  predicted {pv:.3f}  err {e:.3f}")

if __name__ == "__main__":
    main()
