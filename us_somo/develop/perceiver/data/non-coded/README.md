# Test structures containing non-coded residues

The demo structures in the parent directory (`data/*.pdb`) are **fully coded** — every residue in
them already has a `somo.residue` entry, so the perceiver correctly reports nothing to do and they
exercise only the regression path (`make regress`, `perceive compare`).

The structures here each contain at least one residue `somo.residue` does **not** code, so they
exercise the path that matters: *Lookup Tables → Perceive Non-Coded Residues…* in the GUI, and
`perceive <pdb>` in a gui_script.

Unlike `data/*.pdb` these are **tracked in git** — they are not copies of anything else in the
tree, and without them there is no non-coded test case at all. Each is the unmodified RCSB
deposition, `https://files.rcsb.org/download/<ID>.pdb`.

## The set

| structure | non-coded | atoms | flagged | what it exercises |
|---|---|---|---|---|
| `1MBO.pdb` | SO4 | 5 | 0 | the trivial case, and **selectivity** |
| `2CMD.pdb` | CIT | 13 | 3 | a real ligand; carboxylates, no rings |
| `3PTB.pdb` | BEN | 9 | 0 | an **aromatic ring** and an amidinium |
| `5PTI.pdb` | 18 types | — | many | the **pathological** case |

Verified against the build of 2026-08-09 (branch `ehb54-issue-980`):

```
1MBO  1267 atoms, 3 chains,  1 instance,   1 type   SO4  vbar 0.390  molvol  61.75  hydr  0.0
2CMD  2291 atoms, 2 chains,  1 instance,   1 type   CIT  vbar 0.542  molvol 186.56  hydr 16.0
3PTB  1639 atoms, 2 chains,  1 instance,   1 type   BEN  vbar 0.801  molvol 146.13  hydr  2.0
5PTI   539 atoms, 2 chains, 50 instances, 18 types
```

### 1MBO — oxymyoglobin
Contains HEM, OXY and SO4; only **SO4** comes back non-coded, because heme and bound dioxygen are
already in `somo.residue`. That is the useful part: it shows the scan proposing entries for what is
genuinely missing rather than for every HETATM it sees.

### 2CMD — malate dehydrogenase + citrate
One citrate, and the best first test. 3 of its 13 atoms are flagged for review, so the REVIEW block
and the dialog's flagging are both non-empty without being overwhelming. There is exactly **one**
CIT in the file (chain A, 313) — a scan reporting two is the cross-structure state leak fixed in
`41f898a6`, and is worth reporting as a regression.

### 3PTB — trypsin + benzamidine
Benzamidine has a benzene ring, so this is the only case in the set that reaches the SSSR and
ring-decrement code, and its amidinium reaches the electrostriction term. The structure also holds
a Ca²⁺, which **is** coded (`somo.residue` line 700, vbar −0.636) and so is correctly left alone.

### 5PTI — BPTI, neutron structure
Deposited with heavy water and explicit deuteriums. Its residues therefore carry atoms the tables
have no hybrid for (`ARG` with 15 atoms rather than 11), SOMO's residue matching fails on every one
of them, and the scan reports **50 instances across 18 types** — 18 ordinary amino acids plus UNX.

This is a stress case, not a chemistry case. Its purpose is to check that the review loop can be
escaped: the dialog's **Skip all remaining** must abandon the run and report the unreviewed count
rather than marching through eighteen modal dialogs. The entries it proposes are not meaningful and
should not be accepted.

Note that the perceiver itself handles deuterated structures fine — it excludes hydrogens by
design. What fails here is SOMO's residue *matching*, on atom count. Making that H-agnostic would
remove this failure mode, and is a separate change.

## Running the scan headlessly

```
perceive /abs/path/to/perceiver/data/non-coded/2CMD.pdb
```

in a gui_script (see `../../examples/perceive.somo` for the invocation, which differs between
macOS and Linux). The path must be **absolute** — SOMO changes directory during startup, so one
relative to your shell fails with `ERROR: file does not exist`. Nothing is written to
`somo.residue` on this path; entries are printed only.
