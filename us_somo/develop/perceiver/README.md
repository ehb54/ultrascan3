# SOMO residue/hybrid perceiver (prototype)

Perceives each atom's SOMO **hybrid type** (`C4H3`, `N3H1`, `O1H0`, ...) from only **element +
3D coordinates** (any PDB/mmCIF, typically heavy-atom-only X-ray). From the hybrid type, the vdW
radius, electron count and mass follow by lookup in `somo.hybrid.new`. This is the chemical-
perception core needed to generate a `somo.residue` entry "on the fly" for residues SOMO does not
already know, for hydrodynamic calculations.

Isolated and **Qt-free**: the class takes a flat list of `{element, x, y, z}` and returns per-atom
types. SOMO would feed its own parsed atoms in. Builds standalone with a C++17 compiler; no deps.

## Layout

The perception core lives in the SOMO tree proper and is compiled into `libus_somo`; this directory
holds the standalone test harness that builds the same sources **without Qt**.

In-tree (built by `libus_somo.pro`):
- `../include/us_hydrodyn_perceive.h` + `../src/us_hydrodyn_perceive.cpp` — the `Perceiver` class
  (bond/ring/Kekule perception + classification + emit). **Qt-free.**
- `../include/us_hydrodyn_perceive_hybrid.h` — loads `somo.hybrid`; physics-equivalence classes.
- `../include/us_hydrodyn_perceive_elements.h` — covalent (Cordero) + vdW (Bondi) radii, Z, ion set.
- `../include/us_hydrodyn_perceive_saxs.h` — `somo.saxs_atoms` key resolution (coverage auditing).
- `../include/us_hydrodyn_perceive_somo.h` + `../src/us_hydrodyn_perceive_somo.cpp` — the **only**
  Qt-aware layer: converts SOMO's `PDB_model`/`PDB_atom` into perceiver input, reads CONECT/HETNAM,
  and returns tentative `somo.residue` entries for residues `somo.residue` does not code.

Test-side (this directory, not part of the SOMO build):
- `pdb_lite.h` — minimal PDB reader (tests only; SOMO has its own parser).
- `residue_oracle.h` — parses `somo.residue.new` into the regression ground truth.
- `tinytest.h` — zero-dependency test harness.
- `tests/` — `tests_unit.cpp` (synthetic geometries), `regression.cpp` (demo PDB vs oracle),
  `emit_residue.cpp` (generate a somo.residue entry), `coverage.cpp` (table-gap audit).
- `examples/perceive.somo` — ready-to-run gui_script for the in-SOMO commands.
- `tools/gen_saxs_entries.py` — converts published scattering-factor tables into SOMO's row order.
- `tools/psv_model.py` — can a residue's `vbar` be predicted from its perceived hybrid composition?
  Fits per-type increments to `somo.residue` itself, leave-one-out (Mattia's similarity idea).
- `tools/psv_durchschlag.py` — the same question from published increments instead: Durchschlag &
  Zipper's atomic volume increments, **no fitting**, checked against `somo.residue` and Perkins 1986.
  Self-tests against the papers' own worked values first; run it from this directory.
- `tools/calibrate_3v_context.py` — calibrates the 3V "in-context difference" method used to
  measure a **bound ligand's** anhydrous volume: `V = V(complex) - V(complex minus ligand)`,
  scored against residues whose tabulated volume we already know. Needs `Volume.exe` from
  [vossvolvox](https://github.com/vosslab/vossvolvox) (the 3vee.molmovdb.org site is down).
  See DECISIONS.md for the resulting correction factors.
- `tools/recalc_ligand_volumes.py` — recomputes the anhydrous volume of `somo.residue`'s
  prosthetic groups (3V at probe 0 as a provenance audit, probe 1.4 scaled onto the amino-acid
  convention). Guards on elemental formula *and* on reproducing the stored value, because many
  SOMO codes collide with unrelated PDB components. `--from-pdb RES=file.pdb[:chain]` for
  flexible groups whose CCD idealised conformer differs from the bound one.
- `tools/hydration_table.py` — derives a hybrid-type → waters lookup from `somo.residue` for
  proposing a new residue's per-atom hydration (`--cpp` emits a ready-to-paste map). 36 of 48
  types are unanimous, 7 are weak and must be flagged for user review rather than defaulted.
- `data/` — demo PDBs (gitignored; copy them from `us_somo/somo/demo`) and, under `data/ref/`,
  the published scattering-factor sources plus generated `somo.saxs_atoms` rows. The `somo.*`
  parameter tables are **not** copied here — the tests read `us_somo/etc` directly so they cannot
  drift from the real tables.
- `data/non-coded/` — structures that actually contain residues `somo.residue` does **not** code,
  which the demo PDBs (all fully coded) do not provide. These *are* tracked, since they are copies
  of nothing else in the tree. `2CMD` (citrate) is the clean first test; `3PTB` (benzamidine) the
  only ring-bearing one; `5PTI` (neutron/deuterated) the stress case for the review loop.
  See `data/non-coded/README.md` for what each exercises and the expected output.

## Build & run
```
make unit       # synthetic-geometry unit tests
make regress    # score perception vs somo.residue.new on the demo structures
make coverage   # audit the "no hybridization and/or no SAXS coefficient" bottleneck
make emit PDB=data/1HEL.pdb RES=TRP        # emit a somo.residue entry for one residue
```

Inside SOMO the same perception is reachable headlessly through a gui_script command — see
`examples/perceive.somo`:

```
us3_somo _pad -I -g examples/perceive.somo
```

Two sub-commands:

| command | what it does |
|---|---|
| `perceive <pdb>` | loads the structure with SOMO's own parser and prints a tentative `somo.residue` entry (plus any new `somo.hybrid` rows) for every residue `somo.residue` does **not** code — the residues SOMO would otherwise model as a generic ABB average bead |
| `perceive apply <pdb>` | the same, then **puts the entries into effect**: they go into a session overlay of the residue table, SOMO re-reads it and re-reads the structure, and the non-coded count is reported before and after. The headless equivalent of accepting every entry in the GUI dialog |
| `perceive build [method] <pdb>` | `apply`, then **builds the bead model** and fails the script if it does not. The headless equivalent of the whole GUI session: load, perceive, accept, build. Needed because a clean load never reaches the code that resolves atoms against `somo.atom` — defects that only bite at bead-build time are invisible to `apply` |

`method` is one of `somo` (default, overlaps removed), `somo_o` (overlaps kept), `vdw`, or
`grid`/`a2b` — the SOMO, SoMo-overlap, vdW-beads and AtoB buttons respectively. They cover
different parts of the model builder, so an entry that satisfies one can still break another;
`vdw` is the sharpest test of a perceived residue because it relies on **atomic** hydration, which
the perceiver only proposes.

**Put `somo overwrite` at the top of any script that uses `perceive build`.** Without the
overwrite flag the bead-model write calls `setSomoGridFile()`, which raises a modal dialog and
hangs a headless run (batch mode suppresses those prompts, a gui_script does not). There is no
equivalent flag for the hydrodynamic calculations, so a build can still reach a prompt if the
config has automatic hydrodynamics enabled.
| `perceive compare <pdb>` | **hand-testing aid**: runs perception over the residues `somo.residue` **does** code and diffs the result against the curated types, reporting exact-match and physics-match rates plus every difference |

`perceive apply` never writes your `somo.residue`: accepted entries go to `<table>.perceived`
alongside it, which becomes the active table for the rest of the session (the GUI shows this in
the residue-table label). Its closing line is the assertion that matters —
`0 residue type(s) still non-coded` means the bead builder will use the perceived entries rather
than a generic averaged bead.

`perceive compare` is the interactive equivalent of `make regress`: same check, but inside SOMO
against *your installed* `~/ultrascan/etc` tables, so it also tells you whether an installed table
is out of date.

Invocation notes: on macOS the throwaway `_pad` first argument is required and on Linux it must be
omitted; `-I` avoids blocking config dialogs; and the first run after rebuilding `libus_somo` only
re-installs configs and exits, so run it twice.

## How it works
1. **Bond perception** — spatial-grid neighbor search; bonded if `dist < rcov_i + rcov_j + 0.45 A`.
   Hydrogens and monatomic ions are excluded so heavy-atom coordination (hence inferred implicit-H)
   is identical with or without explicit H, and metal-ligand contacts don't inflate counts.
2. **Ring + aromaticity** — finds 5/6-rings; a ring is aromatic if it is flat (<0.10 A out-of-plane)
   and conjugated (mean ring bond <1.43 A) — robustly separates aromatic rings from puckered
   furanose sugars and saturated proline, independent of individual (often distorted) bond lengths.
3. **Kekule** — assigns ring double bonds so every aromatic carbon gets exactly one; a 2-coordinate
   aromatic N with a ring double bond is pyridine-type (no H, `N2H0`), one left single is pyrrole-
   type (H, `N3H1`).
4. **Classification** — per element, hybrid type = element + sigma-count digit + implicit-H count,
   with resolvable special cases (Lys `N4H3+` vs amide `N3H2` by the attached carbon's hybridization;
   carbonyl vs hydroxyl O by the neighbor carbon's hybridization; carboxylate O by symmetry/order;
   disulfide vs free thiol automatically from bond perception).
5. **Emit** — formats a `somo.residue` block and synthesizes `somo.hybrid` lines for any novel type
   (electrons = Z + nH − charge; radius from Bondi if the element is new). Modeling fields
   (bead assignment, ASA) are stubbed; molvol/vbar are estimated — see open question Q1 in DECISIONS.

## Validation (see DECISIONS.md for the full story)
Scored against `somo.residue.new` on 8 demo structures (proteins, protein/DNA; with and without
explicit H), 41,219 atoms:

| metric | result |
|---|---|
| exact hybrid-label match | 97.5% |
| physics match (radius/electrons/mass identical) | 98.4% |
| **geometric perception** (excl. oracle-template + protonation policy) | **99.83%** |
| genuine perception errors | 0.167% (69 atoms) |

The residual is dominated by (a) cases where the perceiver is chemically correct and the residue
template is not (deoxyribose C2' mislabeled `C4H1` in `somo.residue.new`; free cysteines) and
(b) protonation/tautomer policy that heavy-atom geometry cannot resolve (His protonation state,
guanine lactam tautomer) — exactly the cases a Chemical Component Dictionary lookup resolves,
which is why the intended production design is CCD-first with this perceiver as the fallback.
