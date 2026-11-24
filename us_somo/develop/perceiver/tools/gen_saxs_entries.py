#!/usr/bin/env python3
"""Generate somo.saxs_atoms rows from authoritative sources.

Sources (downloaded into data/ref/):
  * it1992.cpp        - cctbx transcription of International Tables Vol. C, Table 6.1.1.4
                        (Cromer-Mann 4-Gaussian). THIS is the form SOMO production uses.
  * f0_WaasKirf.dat   - DABAX transcription of Waasmaier & Kirfel (1995) Acta Cryst A51:416-431
                        (5-Gaussian, valid to sin(th)/lambda = 6 A^-1).

SOMO's parser (US_Hydrodyn_Saxs::select_saxs_file) wants INTERLEAVED coefficients:
  11 tokens: name a1 b1 a2 b2 a3 b3 a4 b4 c volume
  13 tokens: name a1 b1 a2 b2 a3 b3 a4 b4 a5 b5 c volume
Both sources store a's and b's GROUPED, so they must be transposed.

Modes:
  --validate   regenerate rows for symbols somo.saxs_atoms already has and diff (correctness check)
  --missing    emit rows for symbols SOMO lacks
"""
import re, sys, math, argparse

REF = "data/ref"
SOMO_SAXS = "../../etc/somo.saxs_atoms.new"

# ---------------------------------------------------------------- source parsers
def parse_it1992(path):
    """-> {symbol: (a[4], b[4], c)}   entries look like:
       { "C",  { a1, a2, a3, a4 }, { b1, b2, b3, b4 }, c },"""
    txt = open(path).read()
    # Some entries carry inline C comments between the coefficient blocks
    # (e.g. { "Si", {...}, /* "Siv" */ {...}, c }) which would break the pattern below.
    txt = re.sub(r'/\*.*?\*/', ' ', txt, flags=re.S)
    out = {}
    pat = re.compile(
        r'\{\s*"([A-Za-z0-9+\-]+)"\s*,\s*'
        r'\{([^}]*)\}\s*,\s*'
        r'\{([^}]*)\}\s*,\s*'
        r'(-?[0-9.eE+\-]+)\s*\}', re.S)
    for m in pat.finditer(txt):
        sym = m.group(1)
        a = [float(x) for x in m.group(2).replace('\n',' ').split(',') if x.strip()]
        b = [float(x) for x in m.group(3).replace('\n',' ').split(',') if x.strip()]
        c = float(m.group(4))
        if len(a) == 4 and len(b) == 4:
            out[sym] = (a, b, c)
    return out

def parse_waaskirf(path):
    """-> {symbol: (a[5], c, b[5])}  blocks: '#S Z Sym' then a data line of 11 numbers
       ordered a1 a2 a3 a4 a5 c b1 b2 b3 b4 b5."""
    out, sym = {}, None
    for line in open(path):
        s = line.strip()
        if s.startswith('#S'):
            parts = s.split()
            sym = parts[2] if len(parts) >= 3 else None
        elif s.startswith('#'):
            continue
        elif sym and s:
            v = [float(x) for x in s.split()]
            if len(v) == 11:
                out[sym] = (v[0:5], v[5], v[6:11])
            sym = None
    return out

# ---------------------------------------------------------------- symbol mapping
def somo_to_source(somo_sym):
    """SOMO writes 'CL-1','FE+2','NA+1' (and inconsistently 'CO3+'); the tables write
       'Cl1-','Fe2+','Na1+'. Return the source-style symbol."""
    m = re.match(r'^([A-Za-z]+)(?:([+-])(\d+)|(\d+)([+-]))?$', somo_sym)
    if not m: return None
    el = m.group(1).capitalize()
    if m.group(2):   sign, mag = m.group(2), m.group(3)      # XX+n
    elif m.group(5): sign, mag = m.group(5), m.group(4)      # XXn+
    else:            return el
    return f"{el}{mag}{sign}"

# ---------------------------------------------------------------- formatting
def fmt(v):
    return f"{v:g}"

def row_4g(name, a, b, c, volume):
    cols = []
    for i in range(4):
        cols += [fmt(a[i]), fmt(b[i])]
    cols += [fmt(c), fmt(volume)]
    return name + "\t" + "\t".join(cols)

def row_5g(name, a5, c5, b5, volume):
    cols = []
    for i in range(5):
        cols += [fmt(a5[i]), fmt(b5[i])]
    cols += [fmt(c5), fmt(volume)]
    return name + "\t" + "\t".join(cols)

# ---------------------------------------------------------------- somo file
def load_somo(path):
    """-> {name: {'4g': [floats], '5g': [floats], 'vol4':x, 'vol5':y}}"""
    out = {}
    for line in open(path):
        p = line.split()
        if len(p) not in (11, 13): continue
        name = p[0]; nums = [float(x) for x in p[1:]]
        e = out.setdefault(name, {})
        if len(p) == 11: e['4g'] = nums[:-1]; e['vol4'] = nums[-1]
        else:            e['5g'] = nums[:-1]; e['vol5'] = nums[-1]
    return out

def approx(x, y, tol=2e-4):
    return abs(x - y) <= tol * max(1.0, abs(x), abs(y))

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--validate', action='store_true')
    ap.add_argument('--missing',  action='store_true')
    args = ap.parse_args()

    it = parse_it1992(f"{REF}/it1992.cpp")
    wk = parse_waaskirf(f"{REF}/f0_WaasKirf.dat")
    somo = load_somo(SOMO_SAXS)
    print(f"# parsed IT1992: {len(it)} symbols; WaasKirf: {len(wk)} symbols; "
          f"somo.saxs_atoms: {len(somo)} names", file=sys.stderr)

    if args.validate:
        print("==== 4-Gaussian (Cromer-Mann / IT1992) vs somo.saxs_atoms ====")
        ok = bad = skip = 0
        for name, e in sorted(somo.items()):
            if '4g' not in e: continue
            src = somo_to_source(name)
            if src not in it: skip += 1; continue
            a, b, c = it[src]
            want = []
            for i in range(4): want += [a[i], b[i]]
            want += [c]
            got = e['4g']
            if all(approx(x, y) for x, y in zip(want, got)): ok += 1
            else:
                bad += 1
                print(f"  MISMATCH {name} (source {src})")
                print(f"    somo : {['%g'%v for v in got]}")
                print(f"    IT1992:{['%g'%v for v in want]}")
        print(f"  => {ok} match, {bad} mismatch, {skip} not in IT1992")

        print("\n==== 5-Gaussian (Waasmaier-Kirfel) vs somo.saxs_atoms ====")
        ok = bad = skip = 0
        for name, e in sorted(somo.items()):
            if '5g' not in e: continue
            src = somo_to_source(name)
            if src not in wk: skip += 1; continue
            a5, c5, b5 = wk[src]
            want = []
            for i in range(5): want += [a5[i], b5[i]]
            want += [c5]
            got = e['5g']
            if all(approx(x, y) for x, y in zip(want, got)): ok += 1
            else:
                bad += 1
                if bad <= 3:
                    print(f"  MISMATCH {name} (source {src})")
                    print(f"    somo: {['%g'%v for v in got]}")
                    print(f"    WK  : {['%g'%v for v in want]}")
        print(f"  => {ok} match, {bad} mismatch, {skip} not in WaasKirf")

    if args.missing:
        # Elements SOMO's hybrid table can reference but saxs_atoms lacks, plus common PDB elements.
        wanted = ["CL-1","CO+3","NI+2","BR-1","I-1","PT+2","AU+1","HG+2","AS","W","AL+3","SI",
                  "AG+1","LI+1","SR+2","BA+2","CS+1","RB+1","CR+3","V","TI","SN","PB+2","NI","BR","I"]
        print("# Generated somo.saxs_atoms rows (interleaved, SOMO parser order).")
        print("# Coefficients: IT1992 Table 6.1.1.4 (4-Gaussian) / Waasmaier-Kirfel 1995 (5-Gaussian).")
        print("# !! LAST COLUMN (excluded volume) IS A PLACEHOLDER - see note at end.")
        for name in wanted:
            if name in somo: continue
            src = somo_to_source(name)
            if src in it:
                a, b, c = it[src]
                print(row_4g(name, a, b, c, 0))
            if src in wk:
                a5, c5, b5 = wk[src]
                print(row_5g(name, a5, c5, b5, 0))
            if src not in it and src not in wk:
                print(f"# {name}: symbol '{src}' not found in either source", file=sys.stderr)

if __name__ == '__main__':
    main()
