#!/usr/bin/env python3
"""Rename chosen residues in a PDB so somo.residue no longer codes them.

The point is a GROUND TRUTH test. The atoms are untouched -- only the residue name in
columns 18-20 changes -- so whatever the perceiver derives for the renamed residue can be
compared directly against the coded entry for the residue it really is. That is the closest
thing to a known answer this pipeline has: for a free ligand there is nothing to check against.

us_somo/somo/demo/8RATNC.pdb is the same trick done by hand (8RAT with LYS 31 -> LYD), which is
what suggested generating the rest.

Two shapes are worth having beyond a single renamed residue, because they exercise the chain
handling rather than the chemistry:

  consecutive   several non-coded residues in a row -- the bead builder and the peptide bond
                rule see an unbroken stretch of unknown
  gapped        non-coded residues separated by coded ones -- alternating known/unknown, which
                is where residue-to-residue bookkeeping is most likely to slip

Usage:
    make_noncoded_variant.py IN.pdb OUT.pdb A:31=LYZ A:32=SEZ ...

Each spec is CHAIN:RESSEQ=NEWNAME. The new name must not exist in somo.residue -- replacing the
third letter with Z is a convention that keeps the original readable (LYS -> LYZ) and does not
collide with anything in the table today. Prints what it changed, and fails if a spec matched
nothing, since a silently-unmodified file would quietly pass any test built on it.
"""
import sys


def main(argv):
    if len(argv) < 4:
        sys.exit(__doc__)
    src, dst, specs = argv[1], argv[2], argv[3:]

    want = {}
    for s in specs:
        try:
            where, new = s.split("=")
            chain, resseq = where.split(":")
        except ValueError:
            sys.exit("bad spec %r, want CHAIN:RESSEQ=NEWNAME" % s)
        if len(new) > 3:
            sys.exit("residue name %r does not fit columns 18-20" % new)
        want[(chain, resseq.strip())] = new.strip().rjust(3)

    seen = {}
    out = []
    for line in open(src):
        if line.startswith(("ATOM  ", "HETATM")) and len(line) > 26:
            chain = line[21]
            resseq = line[22:26].strip()
            key = (chain, resseq)
            if key in want:
                old = line[17:20]
                seen.setdefault(key, (old, 0))
                seen[key] = (old, seen[key][1] + 1)
                line = line[:17] + want[key] + line[20:]
        out.append(line)

    missed = set(want) - set(seen)
    if missed:
        sys.exit("no atoms matched: %s" % ", ".join("%s:%s" % m for m in sorted(missed)))

    with open(dst, "w") as f:
        f.writelines(out)

    for key in sorted(seen, key=lambda k: (k[0], int(k[1]))):
        old, n = seen[key]
        print("%s:%-4s %s -> %s  (%d atoms)" % (key[0], key[1], old, want[key].strip(), n))
    print("wrote %s" % dst)


if __name__ == "__main__":
    main(sys.argv)
