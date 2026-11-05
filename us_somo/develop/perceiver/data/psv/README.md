# Structures for the protein-level psv comparison

`../*.pdb` is gitignored because those files are copies of `us_somo/somo/demo/*.pdb`.
These two are not copies of anything else in the tree, so they are tracked here — the same
reasoning as `data/non-coded/`.

| file | protein | measured psv (20 C) | n |
|------|---------|--------------------|---|
| 1BEB.pdb | b-lactoglobulin (bovine milk) | 0.7444 | 14 |
| 2CGA.pdb | chymotrypsinogen A (bovine pancreas) | 0.7314 | 9 |

Measured values are the mean of the CORRECTED columns (phi, phi(0), v(0)) of Durchschlag's
Table 5. The apparent columns (phi', phi'(0)) are NOT usable here: each pH point in Table 5 sits
in a different cosolvent, and at constant pH the apparent value spreads 5.7% mean / 7.0% max on
cosolvent alone, against 0.74% for the corrected value.

See `data/psv_measured.txt` for the full set and `tests/protein_psv.cpp` for the comparison.
