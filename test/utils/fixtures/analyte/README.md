# External corpus analyte fixtures

Unmodified XML extracted from public, lab-owned synthetic datasets in
a local `ehb54/aucsol_test_data` checkout at revision
`6e99725d953ec3afda918460e1ab19c6dce79625` (no tag at this revision).
The generated directory is **untracked in that checkout**, so this revision alone
does not reproduce these archives. Source directory: `datasets/generated/base/`.
Both archive SHA-256 values were verified against its `checksums.sha256`:

- Netrin archive: `c163fe7f90b4c3a135ba3a6d720402889a66e9d4e3f775a73881b2b28458891e`.
- DNA archive: `df368b87d531127c61c2e66e0e9d18ffaa4b3bc517cfdcf41176423a00508dd3`.

Its README attributes generation to `ehb54/uslims_stack` commit `945d18d`,
2026-11-04. The data repository README states no customer data is present.

| Fixture | Archive / member | Extracted SHA-256 |
|---|---|---|
| netrin1.xml | `ra/netrin1-ra_synthetic.zip` / `A0000001.xml` | `d5ee010635d1079a86bad431e8c9e7910462212331502e0da8eacecb64dc71e4` |
| dna-208bp.xml | `ri/dna-monomer-208bp-ri_synthetic.zip` / `A0000003.xml` | `75c570fe71c7bb3bdb4836c37bfd32bcd1017e57fc0e79535ff564c80bd78261` |

These are pre-existing generated corpus fixtures, **not historical lab files**.
The LIMS generator uses UltraScan tools; this is not an independent implementation
of the file format. The expected reader values in tests are specified separately.
Do not regenerate them with US_Analyte or reinterpret them as known-correct
scientific inputs. Both have empty sequences. Protein retains stored mw=49535;
DNA now retains stored mw=128396 when the sequence is empty. Initial
characterization reproduced the old result 189.74 (two phosphate termini); the
regression test now requires the authorized empty-sequence mass fix. RNA and
carbohydrate remain covered by synthetic hand-authored tests, not this corpus.

Qt resources embed these bytes read-only so tests need no LIMS checkout, network,
source-tree path or runtime fixture generation.
