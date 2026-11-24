#!/usr/bin/env python3
"""Partial specific volume from *published* atomic volume increments (Durchschlag & Zipper).

Companion to psv_model.py. Both answer "where does a non-coded residue's vbar come from?",
but from opposite directions:

  psv_model.py       fits per-hybrid-type increments to somo.residue itself (empirical,
                     leave-one-out). That is Mattia's similarity idea, done honestly.
  psv_durchschlag.py uses the increments Durchschlag & Zipper published, with no fitting at
                     all, and asks whether they reproduce the tables we already trust.

The second is the interesting one: if a scheme that never saw somo.residue can reproduce it,
that scheme can be trusted on the ligands somo.residue does *not* cover -- which is the whole
non-coded-residue problem.

Sources:
  [DZ94] Durchschlag & Zipper (1994) Prog Colloid Polym Sci 94:20-39.  Tables 1-5.
  [DZ97] Durchschlag & Zipper (1997) J Appl Cryst 30:803-807 + SUP84592.
  [P86]  Perkins (1986) Eur J Biochem 157:169-180.  Table 1, six residue-volume sets.
  [D86]  Durchschlag (1986) in Hinz (ed), Thermodynamic Data for Biochem & Biotech, 45-128.

    V_c = SUM(V_i) + V_CV - SUM(V_RF) - SUM(V_ES)            [DZ94 eq 4]

For a *polymer* residue (a monomeric unit) the covolume and end groups are dropped [DZ94 2.3].

WHAT IS HAND-SUPPLIED HERE, AND WHY IT MATTERS
The increments are keyed on chemical environment: which of six classes a nitrogen is in, which
of five an oxygen is in, ring sizes, formal charges. Deriving those from coordinates is exactly
what the C++ perceiver does. This script does NOT re-derive them -- element and H-count come
free from the hybrid string, but the N/O classes, rings and charges are the hand-written
PERCEPTION table below. So treat that table as the *specification* the perceiver must satisfy
if this ever moves into C++: everything in it is a question the perceiver already answers, or
must learn to.

Usage:  python3 tools/psv_durchschlag.py            # from the perceiver/ directory
"""
import re, sys, math

# <element><sigma>H<nH>[charge-magnitude][+/-], with an optional trailing 'B' (bridging O).
# nH is a single digit, so any further digits before the sign are the charge magnitude.
HYBRID_RE = re.compile(r"^([A-Za-z]+)(\d+)H(\d)(\d*)B?([+-]?)$")

RESFILE = sys.argv[1] if len(sys.argv) > 1 else "../../etc/somo.residue.new"

# --------------------------------------------------------------------------------------
# Durchschlag & Zipper increments, cm^3/mol.  [DZ94 Tables 1-3], [DZ97 Table 1]
# --------------------------------------------------------------------------------------
V_CV = 12.4                                              # covolume
V_ES_POS, V_ES_NEG = 6.8, 6.7                            # electrostriction per charge
V_RF = {3: 2.1, 4: 4.1, 5: 6.1, 6: 8.1, 7: 10.1, 8: 12.1, 9: 14.1}   # ring formation

V_ELEM = {"C": 9.9, "H": 3.1, "S": 15.5, "F": 5.0, "CL": 15.0, "BR": 19.7, "I": 32.1}
V_P = {3: 17.0, 5: 28.5}

# oxygen: five environments
V_O = {"carbonyl":  5.5,   # =O, except a 2nd =O at a ring
       "ether":     5.5,   # -O- in ether/ester/anhydride, O in a ring, amine oxide, nitro
       "oh1":       2.3,   # 1st OH; O- in a phenolate
       "oh2":       0.4,   # 2nd or further neighbouring OH
       "acid":      0.4,   # OH or O- in carboxyl, phosphate, sulfate, sulfonate
       "ring_2dO":  0.9}   # 2nd =O at a ring where tautomerism is possible

# nitrogen: six environments.  NB the backbone amide value of 2.0 is not stated directly --
# back it out of the peptide-group increment, see selftest_group_increments().
V_N = {"diamide_1":   0.5,
       "amide_1":     2.0,   # primary N in monoamides; alpha/omega amino in aa and peptides
       "backbone":    2.0,   # peptide backbone -NH-  (implied by [DZ94] Table 4: 33.5)
       "amine":       4.0,   # primary/secondary/tertiary monoamines; 2nd N in amides; 2nd ammonium
       "aromatic":    7.0,   # N or NH in aromatic rings; nitro; nitrile
       "guanidinium": 8.0,   # per N; in Arg only the two terminal N
       "ammonium_1": 12.2}   # 1st ammonium N; N in betaines

V_METAL = {"MG": 14.0, "CA": 26.0, "MN": 7.4, "FE": 7.1, "CO": 6.6, "NI": 6.6,
           "CU": 7.1, "ZN": 9.2, "MO": 9.4, "HG": 14.8, "SC": 16.5}   # [DZ94 Table 2]
# NB "SC" is how the scan reads; scandium is odd company for this list and selenium would fit
# (SOMO carries MSE, selenomethionine). Unverified -- do not rely on it.

V_ION = {"H+": -3.8, "LI+": -4.7, "NA+": -5.0, "K+": 5.2, "CS+": 17.5, "NH4+": 14.6,
         "MG2+": -28.8, "CA2+": -25.4, "MN2+": -25.3, "FE2+": -32.3, "CU2+": -31.8,
         "ZN2+": -29.2, "FE3+": -55.1, "OH-": -0.2, "F-": 2.6, "CL-": 21.6, "BR-": 28.5,
         "I-": 40.0, "HCO3-": 27.2, "NO3-": 32.8, "H2PO4-": 32.9, "CO3-2": 3.3,
         "HPO4-2": 15.3, "SO4-2": 21.6}          # [DZ94 Table 3]; include ionization already

DVDT = 5.0e-4       # cm^3 g^-1 K^-1 [DZ94 sec 2.3]; increments are for 25 C
T_TABLE = 20.0      # somo.residue is a 20 C table -- see provenance note in main()

# --------------------------------------------------------------------------------------
# PERCEPTION TABLE -- the part a real implementation must derive from coordinates.
# --------------------------------------------------------------------------------------
RINGS = {"PRO": [5], "HYP": [5], "HIS": [5], "HSD": [5], "HSE": [5],
         "PHE": [6], "TYR": [6], "TRP": [5, 6]}

N_CLASS = {                                   # residue -> {atom name: N environment}
    "ARG": {"NH1": "guanidinium", "NH2": "guanidinium", "NE": "amine"},
    "LYS": {"NZ": "ammonium_1"},
    "HIS": {"ND1": "aromatic", "NE2": "aromatic"},
    "HSD": {"ND1": "aromatic", "NE2": "aromatic"},
    "HSE": {"ND1": "aromatic", "NE2": "aromatic"},
    "TRP": {"NE1": "aromatic"},
    "ASN": {"ND2": "amide_1"},
    "GLN": {"NE2": "amide_1"},
    "PRO": {"N": "amine"}, "HYP": {"N": "amine"},
}

# Formal charge is NOT listed here: it is read off the hybrid names (N4H3+, O1H0-, O2H02-),
# so the stored protonation state drives electrostriction directly. Asp/Glu are stored
# protonated and His doubly protonated -- see DECISIONS.md.

# carboxyl/phosphate oxygens -- "acid" class rather than a plain hydroxyl
ACID_O_PREFIX = ("OD", "OE", "OXT", "OP")

AA20 = ("ALA ARG ASN ASP CYS GLN GLU GLY HIS ILE LEU LYS "
        "MET PHE PRO SER THR TRP TYR VAL").split()
CHARGE_SENSITIVE = {"LYS", "ARG", "HIS", "HSD", "HSE", "ASP", "GLU"}


def parse_hybrid(hyb):
    """'C4H3' -> ('C', 4, 3, 0);  'N4H3+' -> ('N', 4, 3, +1);  'O2H02-' -> ('O', 2, 0, -2).

    somo.residue encodes formal charge as a trailing +/-/2-/2+ on the hybrid name, so the
    electrostriction term comes straight from the table rather than from a hand-written
    charge list. A trailing 'B' (O2H0B, bridging phosphodiester O) is a bonding annotation,
    not a charge. Returns None if the name does not look like a hybrid.
    """
    m = HYBRID_RE.match(hyb)
    if not m:
        return None
    el, sigma, nh, mag, sign = m.groups()
    charge = 0
    if sign:
        charge = (1 if sign == "+" else -1) * (int(mag) if mag else 1)
    return el.upper(), int(sigma), int(nh), charge


def residue_volume(name, atoms):
    """Molar volume (cm^3/mol) of a residue as a polymer monomeric unit: no covolume.

    atoms: list of (atom_name, hybrid, mw).  Returns (V, None) or (None, reason).
    """
    v = 0.0
    n_hydroxyl = 0
    npos = nneg = 0
    nclass = N_CLASS.get(name, {})
    for aname, hyb, _mw in atoms:
        p = parse_hybrid(hyb)
        if p is None:
            return None, "unparsed hybrid %s" % hyb
        el, sigma, nh, q = p
        npos += max(q, 0)
        nneg += max(-q, 0)
        v += nh * V_ELEM["H"]
        if el in V_ELEM and el != "H":
            v += V_ELEM[el]
        elif el == "P":
            v += V_P[5]
        elif el == "O":
            # "acid" (0.4) is the OH or O- of a carboxyl/phosphate/sulfate/sulfonate. A
            # neutral =O is a carbonyl (5.5) even in those groups -- so test protonation
            # and charge first, and only then the group the atom name implies.
            if q < 0:
                v += V_O["acid"]
            elif nh >= 1:
                v += V_O["acid"] if aname.startswith(ACID_O_PREFIX) else (
                    V_O["oh1"] if not n_hydroxyl else V_O["oh2"])
                if not aname.startswith(ACID_O_PREFIX):
                    n_hydroxyl += 1
            else:
                v += V_O["carbonyl"] if sigma == 1 else V_O["ether"]
        elif el == "N":
            v += V_N[nclass.get(aname, "backbone")]
        elif el in V_METAL:
            v += V_METAL[el]
        else:
            return None, "no increment for element %s" % el
    for r in RINGS.get(name, []):
        v -= V_RF[r]
    # electrostriction: charges come from the hybrid names, not a hand-written table
    v -= npos * V_ES_POS + nneg * V_ES_NEG
    return v, None


# --------------------------------------------------------------------------------------
# self-tests: the scheme must reproduce the papers before it is allowed to say anything
# --------------------------------------------------------------------------------------
def selftest_free_amino_acids():
    """[DZ97] Table 2 free amino acids (zwitterions: one +, one -), covolume included."""
    C, H = V_ELEM["C"], V_ELEM["H"]

    def aa(nC, nH, extra=0.0, rings=(), n=V_N["amide_1"]):
        v = nC * C + nH * H + n + V_O["acid"] + V_O["carbonyl"] + extra + V_CV
        for r in rings:
            v -= V_RF[r]
        return v - V_ES_POS - V_ES_NEG

    cases = [("Glycine", aa(2, 5), 42.1), ("L-Alanine", aa(3, 7), 58.2),
             ("L-Valine", aa(5, 11), 90.4), ("L-Leucine", aa(6, 13), 106.5),
             ("L-Serine", aa(3, 7, extra=V_O["oh1"]), 60.5),
             ("L-Cysteine", aa(3, 7, extra=V_ELEM["S"]), 73.7),
             ("L-Phenylalanine", aa(9, 11, rings=(6,)), 121.9),
             ("L-Proline", aa(5, 9, rings=(5,), n=V_N["amine"]), 80.1)]
    bad = [(n, got, want) for n, got, want in cases if abs(got - want) > 0.05]
    return cases, bad


def selftest_group_increments():
    """[DZ94] Table 4 group increments must decompose exactly from the Table 1 atoms."""
    C, H, O = V_ELEM["C"], V_ELEM["H"], V_O["carbonyl"]
    cases = [("peptide -N(H)-C(H)-C(=O)-", V_N["backbone"] + 2 * C + 2 * H + O, 33.5),
             ("guanidinium -C(=NH2)NH2", C + 2 * V_N["guanidinium"] + 4 * H, 38.3),
             ("-SH", V_ELEM["S"] + H, 18.6),
             ("-C(=O)OH", C + O + V_O["acid"] + H, 18.9),
             ("-CH2-", C + 2 * H, 16.1),
             ("-CH3", C + 3 * H, 19.2),
             ("-OH (1st)", V_O["oh1"] + H, 5.4)]
    bad = [(n, got, want) for n, got, want in cases if abs(got - want) > 0.05]
    return cases, bad


# --------------------------------------------------------------------------------------
def load_residues(path):
    """somo.residue -> [{name, rtype, vbar, molvol, atoms:[(name,hybrid,mw)]}] (first entry wins)"""
    lines = open(path).read().splitlines()
    out, seen, i = [], set(), 0
    while i < len(lines):
        i += 1                                       # descriptive comment line
        if i >= len(lines):
            break
        h = lines[i].split()
        if len(h) < 7:
            continue
        i += 1
        try:
            name, rtype = h[0], int(h[1])
            molvol, natoms, nbeads, vbar = float(h[2]), int(h[4]), int(h[5]), float(h[6])
        except ValueError:
            continue
        atoms = []
        for _ in range(natoms):
            if i >= len(lines):
                break
            p = lines[i].split(); i += 1
            if len(p) >= 3:
                try:
                    atoms.append((p[0], p[1], float(p[2])))
                except ValueError:
                    pass
        i += nbeads
        if name not in seen:
            seen.add(name)
            out.append(dict(name=name, rtype=rtype, vbar=vbar, molvol=molvol, atoms=atoms))
    return out


# Perkins 1986 Table 1, residue volumes in 1e-3 nm^3 (= A^3): Cohn-Edsall and consensus columns
PERKINS = {
    "ILE": (168.9, 166.1), "PHE": (187.9, 189.7), "VAL": (141.4, 138.8), "LEU": (168.9, 168.0),
    "TRP": (228.5, 227.9), "MET": (163.1, 165.2), "ALA": (87.2, 87.8), "GLY": (60.6, 59.9),
    "CYS": (106.7, 105.4), "TYR": (192.1, 191.2), "PRO": (122.4, 123.3), "THR": (117.4, 118.3),
    "SER": (91.0, 91.7), "HIS": (152.4, 156.3), "GLU": (141.4, 140.9), "ASN": (117.4, 120.1),
    "GLN": (142.4, 145.1), "ASP": (114.6, 115.4), "LYS": (174.3, 172.7), "ARG": (181.3, 188.2),
}
A3_PER_CM3MOL = 1.0 / 0.6022140761


def stats(errs, label):
    e = sorted(abs(x) for x in errs)
    mae = sum(e) / len(e)
    rmse = math.sqrt(sum(x * x for x in e) / len(e))
    print("  %-34s MAE %.4f   RMSE %.4f   p90 %.4f   max %.4f"
          % (label, mae, rmse, e[int(0.9 * (len(e) - 1))], e[-1]))


def main():
    print("=== self-test: reproduce the published worked values ===")
    ok = True
    for title, fn in (("free amino acids [DZ97 Table 2]", selftest_free_amino_acids),
                      ("group increments [DZ94 Table 4]", selftest_group_increments)):
        cases, bad = fn()
        print("  %-34s %d/%d exact" % (title, len(cases) - len(bad), len(cases)))
        for n, got, want in bad:
            ok = False
            print("      FAIL %-28s got %.2f want %.2f" % (n, got, want))
    if not ok:
        print("\nself-test failed -- increments are transcribed wrong, stopping.")
        return 1

    res = load_residues(RESFILE)
    by_name = {r["name"]: r for r in res}
    print("\n=== somo.residue: %s (%d entries) ===" % (RESFILE, len(res)))

    # ---- the 20 coded amino acids, where perception is hand-specified above
    print("\n%-6s %8s %8s %8s %8s   %s" % ("res", "MW", "DZ vbar", "table", "diff%", "note"))
    errs, errs_clean = [], []
    for name in AA20:
        r = by_name.get(name)
        if r is None:
            continue
        v, why = residue_volume(name, r["atoms"])
        if v is None:
            print("%-6s SKIP %s" % (name, why))
            continue
        mw = sum(a[2] for a in r["atoms"])
        # increments are 25 C; somo.residue is a 20 C table -> bring DZ down to 20 C
        vbar = v / mw - DVDT * (25.0 - T_TABLE)
        d = 100.0 * (vbar - r["vbar"]) / r["vbar"]
        errs.append(vbar - r["vbar"])
        note = "charge-state sensitive" if name in CHARGE_SENSITIVE else ""
        if not note:
            errs_clean.append(vbar - r["vbar"])
        print("%-6s %8.2f %8.3f %8.3f %8.1f   %s" % (name, mw, vbar, r["vbar"], d, note))

    print("\nabsolute error in vbar (cm^3/g) vs the tabulated value:")
    stats(errs, "all 20 coded amino acids")
    stats(errs_clean, "excluding charge-state sensitive")

    # ---- against Perkins, which is calibrated to experimental protein vbar
    print("\n=== vs Perkins 1986 Table 1 (A^3) ===")
    print("%-6s %9s %9s %7s %9s %7s" % ("res", "DZ", "Cohn-Ed", "d%", "consensus", "d%"))
    dce, dcons = [], []
    for name in AA20:
        r = by_name.get(name)
        if r is None or name not in PERKINS:
            continue
        v, why = residue_volume(name, r["atoms"])
        if v is None:
            continue
        a3 = v * A3_PER_CM3MOL          # both are 25 C quantities here, no T correction
        ce, cons = PERKINS[name]
        dce.append(100.0 * (a3 - ce) / ce)
        dcons.append(100.0 * (a3 - cons) / cons)
        print("%-6s %9.1f %9.1f %7.1f %9.1f %7.1f" % (name, a3, ce, dce[-1], cons, dcons[-1]))
    for tag, d in (("vs Cohn-Edsall", dce), ("vs Perkins consensus", dcons)):
        da = [abs(x) for x in d]
        print("  %-22s mean %+.2f%%  mean|d| %.2f%%  max|d| %.1f%%"
              % (tag, sum(d) / len(d), sum(da) / len(da), max(da)))
        noarg = [abs(x) for n, x in zip(AA20, d) if n != "ARG"] if len(d) == len(AA20) else None
        if noarg:
            print("  %-22s mean|d| %.2f%% excluding Arg" % ("", sum(noarg) / len(noarg)))

    print("\nNote: Arg is the one large outlier, but the six residue-volume sets in Perkins")
    print("Table 1 disagree with each other about Arg by 17% (and Glu/Ser/Gly by 18-21%),")
    print("so this sits inside the literature's own scatter rather than outside it.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
