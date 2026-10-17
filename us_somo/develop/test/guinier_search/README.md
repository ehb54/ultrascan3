# guinier_search validation harness

Synthetic curves with analytically known Rg plus a comparison table for the
SOMO automatic Guinier range search (`us_saxs_cmds_t guinier_search`).

```
perl gen_synthetic.pl synth                      # 22 curves + synth/truth.csv
us_saxs_cmds_t guinier_search --json synth/*.dat > somo.jsonl
perl compare.pl synth/truth.csv somo.jsonl [atsas.csv]   # SOMO vs truth [vs ATSAS autorg -f csv]
```

Cases: spheres at several noise levels and grids, mis-scaled SD columns,
prolate ellipsoid, cylinder, Debye chain, aggregation and repulsion
structure factors, over-subtracted tail, zeroed SDs, nm^-1 units, CSV and
multi-line headers, a rising curve and a six-point curve (both must fail).

Real-data validation used the SASBDB round-robin entries SASDUA4..SASDUE4
(five proteins with nominal Rg); `compare.pl` takes any truth.csv with the
columns name, rg_true, i0_true, notes.
