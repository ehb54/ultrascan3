#!/usr/bin/env python3
"""Recompute the anhydrous molar volume (bead volume) of somo.residue's prosthetic groups.

WHY
`somo.residue`'s amino-acid entries come from the crystallographic literature (Perkins 1986 and
relatives) and are void-INCLUSIVE Voronoi-style volumes. The prosthetic-group entries were
instead produced with 3V at a **0 A probe**, which is the bare van der Waals volume. Confirmed
exactly -- 3V at probe 0 with 3V's own default radii reproduces the stored numbers:
    HEM  536.0 vs stored 536.13      NAD  517.1 vs stored 517.75
The two halves of the table are therefore on different bases, and the ligand entries run ~30-50%
low relative to the convention the amino acids use.

THE CORRECTION (Mattia's procedure, plus one factor)
Run 3V again on each group at probe 0 (audit: should return the stored value) and at probe 1.4,
then scale onto the amino-acid convention:

    new_molvol = CONVENTION_FACTOR * V_isolated(probe 1.4)

The factor is needed because going 0 -> 1.4 A on an *isolated* molecule only fills its own
surface crevices; it cannot see the interstitial packing void a group carries when surrounded by
protein, which is exactly what the Voronoi-based amino-acid values include. Measured over all 20
amino acids, 6 instances each, 4 proteins (stored / V_isolated@1.4):

    large compact (Ile Leu Met Phe Tyr Trp)   1.204  sd 0.009
    small (< 160 A^3)                         1.131  sd 0.032
    large flexible (Lys 1.077, Arg 1.147)     lower -- flexible chains carry less

So the net correction is about 1.31x the stored value, not the 1.09x that a bare 0->1.4 rescale
would give.

CAVEAT -- CONFORMATION DEPENDENCE (read before trusting any number here)
The isolated volume depends on the conformer. CCD "ideal" coordinates reproduce the stored value
for NAD (518.0 vs 517.75) but NOT for HEM (585.6 vs 536.13, +9%), because heme's propionates are
extended in the idealised conformer and folded when bound. Prefer `--from-pdb` with a
representative complex for flexible groups; the probe-0 audit column tells you whether the
conformer you used matches the one the stored value came from.

An independent in-context measurement (V(complex) - V(complex minus ligand), see
calibrate_3v_context.py) gives ~795 A^3 for both HEM and NAD, about 15% above what this script
produces. The two routes bracket the truth; both are far above the stored values.

Usage
    python3 tools/recalc_ligand_volumes.py                     # all non-standard entries, CCD
    python3 tools/recalc_ligand_volumes.py HEM NAD ATP
    python3 tools/recalc_ligand_volumes.py --from-pdb HEM=1MBO.pdb NAD=1LDM.pdb:A
Requires Volume.exe from https://github.com/vosslab/vossvolvox.
"""
import os, re, shutil, subprocess, sys, urllib.request

RESFILE = os.environ.get("SOMO_RESIDUE", "../../etc/somo.residue.new")
# Volume.exe from https://github.com/vosslab/vossvolvox. Taken from $VOLUME_EXE, else PATH.
VOLEXE = os.environ.get("VOLUME_EXE") or shutil.which("Volume.exe") or "Volume.exe"
CACHE = os.environ.get("CCD_CACHE", os.path.join(os.environ.get("TMPDIR", "/tmp"), "ccd_cache"))
GRID = 0.15
FACTOR_LARGE, FACTOR_SMALL, SIZE_SPLIT = 1.204, 1.131, 160.0
AUDIT_TOL = 0.03          # |iso@0 / stored - 1| above this = conformer/provenance mismatch

STANDARD = set(("ALA ARG ASN ASP CYS CYH GLN GLU GLY HIS HSD HSE ILE LEU LYS MET MSE PHE PRO "
                "HYP SER THR TRP TYR VAL A C G U DA DC DG DT N1 N1- OXT WAT").split())


def load_table(path):
    """-> molvol[res], natoms[res], desc[res], order

    natoms is the identity guard: many somo.residue codes are SOMO-specific (PBR-*, OXN =
    'Triton X-100 with 5 C2H4O', BF4 = '2-(BF4)2', PEG = 'PEG repeating unit') and collide with
    unrelated PDB chemical-component codes. A matching volume is NOT proof of a matching
    molecule -- compare the heavy-atom count too.
    """
    lines = open(path).read().splitlines()
    molvol, natoms, desc, formula, order, i = {}, {}, {}, {}, [], 0
    while i < len(lines):
        comment = lines[i] if i < len(lines) else ""
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
        if nm not in molvol:
            f = {}
            for j in range(i, min(i + na, len(lines))):
                p = lines[j].split()
                if len(p) >= 2:
                    el = re.match(r"([A-Za-z]+)", p[1])
                    if el:
                        e = el.group(1).upper()
                        f[e] = f.get(e, 0) + 1
            molvol[nm] = mv
            natoms[nm] = na
            desc[nm] = comment.strip()
            formula[nm] = f
            order.append(nm)
        i += na + max(nb, 0)
    return molvol, natoms, desc, formula, order


def fmt_formula(f):
    return "".join("%s%d" % (e, n) for e, n in sorted(f.items()))


def fetch_ccd(code):
    """CCD ideal coordinates -> heavy-atom PDB text, or None if the code is not a PDB component."""
    os.makedirs(CACHE, exist_ok=True)
    sdf = os.path.join(CACHE, code + "_ideal.sdf")
    if not os.path.exists(sdf):
        url = "https://files.rcsb.org/ligands/download/%s_ideal.sdf" % code
        try:
            with urllib.request.urlopen(url, timeout=30) as r:
                data = r.read().decode("utf-8", "replace")
        except Exception:
            return None
        if "V2000" not in data and "V3000" not in data:
            return None
        open(sdf, "w").write(data)
    L = open(sdf).read().splitlines()
    if len(L) < 4:
        return None
    try:
        na = int(L[3][0:3])
    except ValueError:
        return None
    out, n = [], 0
    for i in range(4, 4 + na):
        if i >= len(L):
            break
        try:
            x, y, z = float(L[i][0:10]), float(L[i][10:20]), float(L[i][20:30])
        except ValueError:
            return None
        el = L[i][31:34].strip().upper()
        if el == "H":
            continue
        n += 1
        out.append("HETATM%5d %-4s %3s A   1    %8.3f%8.3f%8.3f  1.00  0.00          %2s\n"
                   % (n, (el + str(n))[:4], code[:3], x, y, z, el))
    return "".join(out) if out else None


def from_pdb(spec, code):
    path, _, chain = spec.partition(":")
    out = []
    for ln in open(path):
        if not (ln.startswith("ATOM") or ln.startswith("HETATM")):
            continue
        if ln[17:20].strip() != code:
            continue
        if chain and ln[21] != chain:
            continue
        if ln[16] not in (" ", "A"):
            continue
        el = (ln[76:78].strip().upper() or re.sub(r"[^A-Z]", "", ln[12:16].strip().upper())[:1])
        if el == "H":
            continue
        out.append(ln)
    return "".join(out) if out else None


def volume(path, probe):
    """Volume.exe data line is probe, grid, volume, ... -- parse by POSITION (field 3)."""
    r = subprocess.run([VOLEXE, "-i", path, "-p", str(probe), "-g", str(GRID)],
                       capture_output=True, text=True)
    for line in r.stdout.splitlines():
        f = line.split()
        if len(f) >= 3:
            try:
                if abs(float(f[0]) - probe) < 1e-6:
                    return float(f[2])
            except ValueError:
                pass
    return None


def main():
    args = [a for a in sys.argv[1:]]
    pdbmap = {}
    if "--from-pdb" in args:
        args.remove("--from-pdb")
        for a in list(args):
            if "=" in a:
                k, _, v = a.partition("=")
                pdbmap[k] = v
                args.remove(a)
    molvol, natoms, desc, formula, order = load_table(RESFILE)
    targets = args or [n for n in order if n not in STANDARD]

    tmp = os.path.join(os.environ.get("TMPDIR", "/tmp"), "_lig.pdb")
    print("factor: %.3f if V@1.4 >= %.0f A^3 else %.3f   (amino-acid convention)"
          % (FACTOR_LARGE, SIZE_SPLIT, FACTOR_SMALL))
    print("\n%-7s %9s %6s %9s %8s %9s %10s  %s" %
          ("res", "stored", "atoms", "V@0", "audit", "V@1.4", "PROPOSED", "verdict"))
    accept, review, miss = [], [], []
    for code in targets:
        stored = molvol.get(code)
        if stored is None:
            continue
        txt = from_pdb(pdbmap[code], code) if code in pdbmap else fetch_ccd(code)
        src = "pdb" if code in pdbmap else "ccd"
        if not txt:
            miss.append(code)
            print("%-7s %9.2f %6d %9s %8s %9s %10s  NO COORDS (%s)"
                  % (code, stored, natoms[code], "-", "-", "-", "-", src))
            continue
        gotf = {}
        for l in txt.splitlines():
            if not l.strip():
                continue
            e = (l[76:78].strip().upper()
                 or re.sub(r"[^A-Z]", "", l[12:16].strip().upper())[:1])
            gotf[e] = gotf.get(e, 0) + 1
        got = sum(gotf.values())
        open(tmp, "w").write(txt)
        v0, v14 = volume(tmp, 0), volume(tmp, 1.4)
        if v0 is None or v14 is None or v14 <= 0:
            miss.append(code)
            print("%-7s %9.2f %6d %9s %8s %9s %10s  3V FAILED (too few atoms?)"
                  % (code, stored, natoms[code], "-", "-", "-", "-"))
            continue
        factor = FACTOR_LARGE if v14 >= SIZE_SPLIT else FACTOR_SMALL
        new = factor * v14
        audit = v0 / stored if stored else float("nan")
        # identity guard first: a matching volume on the WRONG molecule is the dangerous case
        if gotf != formula[code]:
            # exactly one extra O in the CCD entry = the free monomer vs the polymer residue
            # (a glycosidic/peptide bond sheds H2O). Not a collision -- and such entries are
            # usually already on the literature/Voronoi basis, so they need no correction.
            delta = {e: gotf.get(e, 0) - formula[code].get(e, 0)
                     for e in set(gotf) | set(formula[code])}
            if {e: d for e, d in delta.items() if d} == {"O": 1}:
                verdict = "RESIDUE FORM: %s is the free monomer (+H2O); entry is a polymer " \
                          "residue - likely already literature-basis, do not correct" % src.upper()
            else:
                verdict = "WRONG MOLECULE: %s=%s vs somo.residue=%s" % (
                    src.upper(), fmt_formula(gotf), fmt_formula(formula[code]))
            review.append(code)
        elif abs(audit - 1.0) > AUDIT_TOL:
            verdict = "conformer differs (V@0 %+.0f%%) - use --from-pdb" % (100 * (audit - 1))
            review.append(code)
        else:
            verdict = "ACCEPT  (x%.2f)" % (new / stored)
            accept.append((code, stored, new))
        print("%-7s %9.2f %6d %9.1f %8.3f %9.1f %10.1f  %s"
              % (code, stored, natoms[code], v0, audit, v14, new, verdict))

    print("\n%d ACCEPT, %d need review, %d no usable coordinates" %
          (len(accept), len(review), len(miss)))
    if accept:
        print("\n=== proposed somo.residue molvol replacements (identity + audit both clean) ===")
        for code, old, new in accept:
            print("  %-7s %9.2f -> %9.1f   (x%.2f)   %s" % (code, old, new, new / old, desc[code][:44]))
    if review:
        print("\nneed review: " + " ".join(review))
    if miss:
        print("no coordinates: " + " ".join(miss))
    print("\nGuards: heavy-atom count must match the somo.residue entry (catches code collisions,")
    print("e.g. SOMO's OXN = 'Triton X-100 with 5 C2H4O' is not the PDB component OXN), AND")
    print("V@0 must reproduce the stored value (confirms the conformer/provenance).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
