#!/usr/bin/env python3
"""Calibrate the 3V "in-context difference" method for measuring a bound ligand's volume.

THE PROBLEM
`somo.residue`'s anhydrous molar volume field defines the bead volume. For a novel ligand we
must measure it. Measuring the ligand in isolation is wrong: the tabulated residue volumes are
a Voronoi-style partition of a *packed* protein, so each entry carries a share of the
interstitial void that only exists in context.

THE METHOD (Mattia's proposal)
Measure the ligand bound:   V_ligand = V(complex) - V(complex minus ligand)
This works only because 3V's Volume.exe leaves interior cavities UNCOUNTED, so the pocket
vacated by the deleted ligand does not contribute to V(apo). Verified with a synthetic sealed
shell: Volume.exe returns the shell-only volume, VolumeNoCav.exe returns the filled ball.
(Use Volume.exe. VolumeNoCav.exe would fill the pocket and the difference would collapse to 0.)

THE CATCH, AND WHAT THIS SCRIPT MEASURES
The difference is size-dependent. Deleting a *small* residue leaves a gap too narrow to admit a
1.4 A probe centre, so the gap stays counted in V(apo) and the difference underestimates.
Deleting a *large* one opens a cavity that also swallows neighbouring interstitial space, so it
overestimates. This script calibrates that bias against residues whose "right answer" we already
know -- the tabulated somo.residue volumes -- so a ligand measurement can be corrected.

    ratio = [V(all) - V(all minus residue)] / tabulated_molvol

Reports ratio vs residue size and vs burial, over many residues in several proteins.

Usage:  python3 tools/calibrate_3v_context.py [pdb ...]      (run from perceiver/)
Requires: Volume.exe from https://github.com/vosslab/vossvolvox on PATH or at ../../../vossvolvox/bin.
"""
import os, re, shutil, subprocess, sys, math, statistics
from collections import defaultdict

RESFILE = os.environ.get("SOMO_RESIDUE",
                         "../../etc/somo.residue.new")
# Volume.exe from https://github.com/vosslab/vossvolvox. Taken from $VOLUME_EXE, else PATH.
VOLEXE = os.environ.get("VOLUME_EXE") or shutil.which("Volume.exe") or "Volume.exe"
PROBE = 1.4
GRID = 0.25
MIN_BURIAL = 85            # neighbour atoms within 8 A of the residue centroid
ELEM_R = {"C": 1.70, "N": 1.55, "O": 1.52, "S": 1.80, "FE": 1.80, "P": 1.80, "SE": 1.90}
SKIP_RES = {"HOH", "SO4", "OXM", "CIT", "OXY", "PO4", "GOL", "EDO", "ACT", "MPD"}
STANDARD = ("ALA ARG ASN ASP CYS GLN GLU GLY HIS ILE LEU LYS "
            "MET PHE PRO SER THR TRP TYR VAL").split()


def load_table(path):
    """-> (radius[(res,atom)], molvol[res], natoms[res])"""
    lines = open(path).read().splitlines()
    rad, molvol, natoms, i = {}, {}, {}, 0
    while i < len(lines):
        i += 1
        if i >= len(lines):
            break
        h = lines[i].split()
        if len(h) < 7:
            continue
        i += 1
        try:
            nm, mv, na, nb = h[0], float(h[2]), int(h[4]), int(h[5])
        except ValueError:
            continue
        molvol.setdefault(nm, mv)
        natoms.setdefault(nm, na)
        for j in range(i, i + na):
            p = lines[j].split()
            if len(p) >= 4:
                try:
                    rad.setdefault((nm, p[0]), float(p[3]))
                except ValueError:
                    pass
        i += na + max(nb, 0)
    return rad, molvol, natoms


def read_atoms(pdb, rad, chain=None):
    out = []
    for ln in open(pdb):
        if not (ln.startswith("ATOM") or ln.startswith("HETATM")):
            continue
        rn, an, ch = ln[17:20].strip(), ln[12:16].strip(), ln[21]
        if rn in SKIP_RES:
            continue
        if chain and ch != chain:
            continue
        if ln[16] not in (" ", "A"):          # altloc: keep the first only
            continue
        el = (ln[76:78].strip().upper() or re.sub(r"[^A-Z]", "", an.upper())[:1])
        if el == "H":
            continue
        out.append(dict(rn=rn, an=an, key=ln[21:27],
                        x=float(ln[30:38]), y=float(ln[38:46]), z=float(ln[46:54]),
                        r=rad.get((rn, an), ELEM_R.get(el, 1.70))))
    return out


def write_xyzr(path, atoms):
    with open(path, "w") as f:
        for a in atoms:
            f.write("%.3f %.3f %.3f %.3f\n" % (a["x"], a["y"], a["z"], a["r"]))


def volume(path, probe=PROBE, grid=GRID):
    """Volume.exe prints one tab-separated data line: probe, grid, volume, surf_area, natoms, file.

    Parse by POSITION. (A 'first number bigger than 1' heuristic silently returns the probe
    radius itself for probe=1.4, which makes every difference come out exactly 0.)
    """
    r = subprocess.run([VOLEXE, "-i", path, "-p", str(probe), "-g", str(grid)],
                       capture_output=True, text=True)
    for line in r.stdout.splitlines():
        f = line.split()
        if len(f) < 3:
            continue
        try:
            got_probe, _grid, vol = float(f[0]), float(f[1]), float(f[2])
        except ValueError:
            continue
        if abs(got_probe - probe) > 1e-6:      # not the data line we asked for
            continue
        return vol
    return None


def bbox(atoms):
    return tuple(round(f(a[c] + s * a["r"] for a in atoms), 2)
                 for c in "xyz" for f, s in ((min, -1), (max, 1)))


def calibrate(pdb, rad, molvol, natoms, tmpdir, chain=None):
    atoms = read_atoms(pdb, rad, chain)
    if not atoms:
        return []
    byres = defaultdict(list)
    for a in atoms:
        byres[a["key"]].append(a)
    cent = {k: (sum(a["x"] for a in v) / len(v), sum(a["y"] for a in v) / len(v),
                sum(a["z"] for a in v) / len(v)) for k, v in byres.items()}
    burial = {}
    for k, (cx, cy, cz) in cent.items():
        burial[k] = sum(1 for a in atoms
                        if (a["x"] - cx) ** 2 + (a["y"] - cy) ** 2 + (a["z"] - cz) ** 2 < 64)

    base = os.path.join(tmpdir, "all.xyzr")
    write_xyzr(base, atoms)
    full = volume(base)
    if full is None:
        return []
    full_box = bbox(atoms)

    rows = []
    for k, res in sorted(byres.items(), key=lambda kv: -burial[kv[0]]):
        rn = res[0]["rn"]
        if rn not in STANDARD or rn not in molvol:
            continue
        if burial[k] < MIN_BURIAL:
            continue
        # composition must match the tabulated entry exactly: incomplete side chains or
        # terminal residues (extra OXT) would silently bias the measured volume.
        if len(res) != natoms.get(rn):
            continue
        sub = [a for a in atoms if a["key"] != k]
        if bbox(sub) != full_box:        # deletion moved the grid -> difference is not clean
            continue
        p = os.path.join(tmpdir, "minus.xyzr")
        write_xyzr(p, sub)
        v = volume(p)
        if v is None:
            continue
        d = full - v
        rows.append(dict(pdb=os.path.basename(pdb), res=rn, key=k.strip(),
                         burial=burial[k], tab=molvol[rn], diff=d, ratio=d / molvol[rn]))
    return rows


def report(rows):
    if not rows:
        print("no measurements")
        return
    print("\n%-8s %-5s %-8s %7s %9s %9s %7s" %
          ("pdb", "res", "id", "burial", "diff", "tabulated", "ratio"))
    for r in sorted(rows, key=lambda r: r["tab"]):
        print("%-8s %-5s %-8s %7d %9.1f %9.2f %7.3f" %
              (r["pdb"], r["res"], r["key"], r["burial"], r["diff"], r["tab"], r["ratio"]))

    print("\n=== by residue size (tabulated molvol) ===")
    print("%-18s %5s %8s %8s %8s" % ("size band", "n", "mean", "median", "sd"))
    bands = [("< 100", 0, 100), ("100-140", 100, 140), ("140-180", 140, 180),
             ("180-220", 180, 220), (">= 220", 220, 1e9)]
    fits = []
    for name, lo, hi in bands:
        v = [r["ratio"] for r in rows if lo <= r["tab"] < hi]
        if len(v) >= 2:
            print("%-18s %5d %8.3f %8.3f %8.3f" %
                  (name, len(v), statistics.mean(v), statistics.median(v), statistics.pstdev(v)))
            fits.append((name, len(v), statistics.mean(v)))
        elif v:
            print("%-18s %5d %8.3f %8s %8s" % (name, len(v), v[0], "-", "-"))

    print("\n=== by residue type ===")
    byt = defaultdict(list)
    for r in rows:
        byt[r["res"]].append(r["ratio"])
    for t in sorted(byt, key=lambda t: -len(byt[t])):
        v = byt[t]
        print("  %-5s n=%2d  mean %6.3f  sd %6.3f" %
              (t, len(v), statistics.mean(v), statistics.pstdev(v) if len(v) > 1 else 0.0))

    big = [r["ratio"] for r in rows if r["tab"] >= 180]
    if big:
        print("\nLARGE-RESIDUE CORRECTION FACTOR (tabulated >= 180 A^3, the ligand analogues):")
        print("  n=%d  mean %.3f  median %.3f  sd %.3f  sem %.3f" %
              (len(big), statistics.mean(big), statistics.median(big),
               statistics.pstdev(big), statistics.pstdev(big) / math.sqrt(len(big))))
        print("  -> corrected ligand volume = in-context difference / %.3f" % statistics.mean(big))

    print("\n=== burial dependence (large residues only) ===")
    for lo, hi in ((85, 100), (100, 115), (115, 1000)):
        v = [r["ratio"] for r in rows if r["tab"] >= 180 and lo <= r["burial"] < hi]
        if len(v) >= 2:
            print("  burial %3d-%-4d n=%2d  mean %.3f  sd %.3f" %
                  (lo, hi, len(v), statistics.mean(v), statistics.pstdev(v)))


def main():
    pdbs = sys.argv[1:]
    if not pdbs:
        print(__doc__)
        return 2
    rad, molvol, natoms = load_table(RESFILE)
    tmpdir = os.environ.get("TMPDIR", "/tmp")
    allrows = []
    for spec in pdbs:
        pdb, _, chain = spec.partition(":")
        rows = calibrate(pdb, rad, molvol, natoms, tmpdir, chain or None)
        print("%s%s: %d measurements" % (os.path.basename(pdb),
                                         (" chain " + chain) if chain else "", len(rows)))
        allrows += rows
    report(allrows)
    return 0


if __name__ == "__main__":
    sys.exit(main())
