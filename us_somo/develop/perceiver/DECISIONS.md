# SOMO residue/hybrid perceiver — decision & progress log

Running log for review. Autonomous session started 2026-07-19.
Worked in an isolated local clone of this repository.
Prototype dir: `us_somo/develop/perceiver/` (self-contained, Qt-free, builds standalone).

## Goal
Given only element + 3D coordinates (arbitrary PDB/mmCIF, typically heavy-atom-only X-ray),
perceive each atom's SOMO **hybrid type** (`C4H3`, `N3H1`, `O1H0`, ...), from which vdW radius,
electron count and mass follow by lookup in `somo.hybrid.new`. Emit `somo.residue` + any new
`somo.hybrid` entries "on the fly" for residues SOMO doesn't already know.

Validation strategy (eb's idea): take the residues SOMO *does* know (`somo.residue.new`),
pretend they're unknown, run the perceiver on real coordinates from the demo PDBs, and check we
reproduce the curated hybrid types. Heavy unit + regression testing.

## Key findings that shaped the design
1. **The "type-keyed radius/electron table" already exists** = `somo.hybrid.new` (50 types).
   We only build the *perceiver* that outputs its keys; radius/electrons/mass are lookups.
2. **Hybrid-type digit = total sigma-bond count = (#heavy neighbors + #implicit H).**
   - C: digit = hybridization (sp3=4, sp2=3, sp=2); nH = digit - heavy.
   - O: digit 1 = carbonyl/=O (1 sigma); digit 2 = hydroxyl/ether/water.
   - N: digit = sigma count; the physically meaningful call is **nH** (see #3).
3. **Several types are physical aliases** (identical mw/radius/electrons), so exact-label match
   is stricter than needed. Most important: `N2H0` ≡ `N3H0` (both mw 14.01, r 1.64, 7 e−) —
   SOMO uses them interchangeably for no-H ring N (adenine N3 = N2H0, guanine N3 = N3H0).
   => **We score primarily on physics-equivalence (mw/radius/electrons), and secondarily on exact label.**
4. Resolvable-by-geometry distinctions confirmed from ground truth:
   - Disulfide vs free thiol: falls out of bond perception automatically (Cys SG with 1 heavy
     neighbor = free thiol `S2H1`; bonded to another S = 2 heavy = `S2H0`). No special case.
   - Lys NZ (`N4H3+`) vs Arg/Asn terminal NH2 (`N3H2`): distinguished by whether the attached
     carbon is sp3 (aliphatic amine, protonated) or sp2 (amide/guanidinium). Perceivable.
   - Phosphodiester bridging O (`O2H0B`, r 1.62) vs ester/ether O (`O2H0`, r 1.5): the bridging
     O has a P neighbor. Perceivable.

## Inherent ambiguities (heavy-atom geometry cannot fully resolve — logged, not solved by v1)
- **Ring-N protonation / tautomer** (His HSD vs HSE; 2-connected aromatic ring N with H vs lone
  pair). SOMO's `somo.residue.new` bakes in a fixed default (e.g. HIS = doubly protonated). Pure
  geometry can't see the H. Mitigations: aromatic bond-order/valence model (partial), or the
  CCD-first architecture (exact for known component codes). v1 uses a heuristic + flags ambiguity.
- **Carboxylic acid protonation** (Asp/Glu stored protonated: one O = `O1H0`, one = `O2H1`).
  A deprotonated crystallographic carboxylate has ~equal C–O lengths; assigning which O is the
  =O is a convention. v1: bond-length split, protonated default, flag when ~symmetric.
- **Formal-charge suffix placement** on a specific atom (SOMO distributes residue net charge to a
  chosen atom, e.g. Arg NH2 = `N3H2+` but NH1 = `N3H2`). Physically identical group; we assign
  charge by chemistry, not to match the exact atom SOMO picked. Does not affect radius; affects
  net-charge bookkeeping only.

## Decisions
- **Standalone, Qt-free class** so it unit-tests fast and drops into SOMO later (takes plain
  vectors of {element,x,y,z}; SOMO passes its parsed atoms in). No external deps; tiny in-repo
  test harness (`tinytest.h`) instead of vendoring a framework.
- **Metals/monatomic ions treated as non-bonding** during perception (identified by element →
  `ELEMENT+charge`) so metal–ligand contacts don't inflate coordinating-atom neighbor counts.
- Covalent radii: Cordero 2008; bond if dist < r_i + r_j + 0.45 Å (with a clash lower bound).
- Bondi vdW radii used only as fallback for elements absent from `somo.hybrid.new`.

## REQUIREMENTS from Mattia (2026-07-20) — these supersede my earlier open questions
1. `somo.residue` is the **master** table. Perceiver runs only for **non-coded** residues.
2. Non-coded residue → produce a **tentative entry** in somo.residue format, show it to the user
   with a RasMol (SOMO) / JSmol (SAF) visualization, and ask for confirmation.
3. Later: prompt for **hydration numbers** (hydrodynamics, possibly P(r)). Not perceived.
4. TBD whether a confirmed new residue is written back into somo.residue.
5. `somo.atom` is **irrelevant** here and should eventually become superfluous — it only existed to
   link atom names → somo.hybrid / somo.saxs_atoms for *manual* definition, and is hugely redundant
   (one hybridization shared by hundreds of differently-named PDB atoms). Geometric perception is
   exactly what makes it unnecessary. (I loaded it into data/ but never used it — confirmed correct.)
6. **Bottleneck: an atom with no hybridization AND/OR no SAXS coefficient in the current tables.**
7. **No pH-dependent handling for non-coded residues for now.**

### What changed in the code because of this
- **#7 closes my Q2** — dropped the protonation/pH policy layer from scope. (Also moots the His and
  guanine tautomer debate: His is a coded residue, so the master table handles it.)
- **#3 closes part of Q1** — hydration is user-entered; my stub was right.
- **#2** — `emit_residue()` now prefixes the tentative entry with a `# REVIEW` block listing every
  atom whose perception was uncertain (ambiguous protonation, novel type), so the confirming user
  knows exactly what to check.
- **#6** — added `saxs_table.h` + `tests/coverage.cpp` (`make coverage`): audits hybrid→SAXS
  resolution, reports element support, and scans structures for atoms that would fail.
- Added a **vocabulary guard**: for C/N/O/S/P the table is authoritative, so the perceiver never
  proposes a type absent from `somo.hybrid.new` (corrupt geometry previously yielded e.g. `C2H1`);
  it falls back to the saturated form and flags it. Plus a bond-plausibility floor (<1.15 Å is not
  a real bond). A user-facing proposal must never contain a nonexistent type.

## COVERAGE AUDIT RESULTS (Mattia's #6, quantified — these are pre-existing table issues)
- **`CL-1` has NO SAXS entry**: chlorine is entirely absent from `somo.saxs_atoms`, yet `CL-1` is a
  valid hybrid type and chloride is common in structures. SAXS breaks for any chloride.
- **`CO+3` does not resolve**: `somo.hybrid.new` spells it `CO+3`, `somo.saxs_atoms` spells it
  **`CO3+`**. Note the table already carries BOTH `O-1`/`O1-` and `O-2`/`O2-` — the same charge-
  spelling collision was hit for oxygen and worked around by duplicating entries, but missed for Co.
  (`saxs_table.h::resolve()` now tries the transposed spelling, so this is tolerated at runtime.)
- **21 common PDB elements are fully unsupported** (no hybrid type AND no SAXS coefficients):
  `AL SI NI BR AG I W PT AU HG AS LI SR BA CS RB CR V TI SN PB`. Notably **NI** (His-tags, urease),
  **BR/I** (halide phasing), **PT/AU/HG** (classic heavy-atom derivatives), **AS**, **W**.
- Scan of all 8 demo structures (41k+ atoms): **0 bottleneck atoms** after the guard. The only hit
  was LEU 367B in 3GUT, which has a **corrupt CG–CD2 bond of 1.131 Å** (impossible for C–C) — a
  defect in that PDB, now handled gracefully.

**I did NOT invent Cromer-Mann coefficients for the missing elements.** Filling them needs an
authoritative source (Waasmaier & Kirfel 1995, or International Tables Vol. C) — fabricating them
from memory would silently corrupt SAXS. Say the word and I'll wire in values you supply/approve.

## #4 ANSWERED (eb, 2026-07-20): make storing the confirmed residue a USER OPTION
`somo.residue` lives in the user's own directory (`~/ultrascan/etc/`), so a stored entry is reusable
for them in later sessions. Design: after the user confirms a tentative entry, offer "store in your
somo.residue?" — yes = append (persist, reusable), no = session-only. `emit_residue()` already just
returns the block, so persistence is purely a caller/UI concern; no core change needed.

## GENERATED: missing somo.saxs_atoms rows -> `data/ref/somo.saxs_atoms.generated`
Tool: `tools/gen_saxs_entries.py` (`--validate` / `--missing`). Sources downloaded to `data/ref/`
(f0_WaasKirf.dat 42 KB / 211 entries; cctbx it1992.cpp 32 KB / 212 symbols).
26 symbols emitted in BOTH forms (52 rows): CL-1 CO+3 NI+2 BR-1 I-1 PT+2 AU+1 HG+2 AS W AL+3 SI
AG+1 LI+1 SR+2 BA+2 CS+1 RB+1 CR+3 V TI SN PB+2 NI BR I.
**Excluded-volume column emitted as 0 (UNSET)** — it is SOMO-specific, not in either source. The
existing values are the Fraser/MacRae/Suzuki set (H 5.15, C 16.44, N 2.49, O 9.13, S 19.86, P 5.73)
and `somo.atom.new` shows the per-hybrid rule vol = vol(element) + nH*5.15 (C4H3 = 16.44+3*5.15
= 31.89). For the new elements that value has to come from eb. (F and SE already carry 0.)

### Converter validated against SOMO's own file
`--validate` regenerates rows for symbols SOMO already has and diffs them:
- **4-Gaussian: 17 match exactly** (all the IT1992-sourced light elements) — proves the
  grouped->interleaved transposition is right.
- 11 "mismatches" are NOT errors: they match `somo.cromer68.saxs_atoms` **exactly**, i.e.
  `somo.saxs_atoms` is a deliberate **mixed-provenance** table — light elements from IT1992
  (Vol. C, 1992), ions/metals from **Cromer 1968**. Worth knowing if you ever standardize sources.
- **5-Gaussian: 0 of 25 match** WaasKirf — confirms the b-shift bug below affects EVERY 5-Gaussian
  row, not just the C and N I originally spot-checked.

## SOURCES for the missing coefficients (verified 2026-07-20)
How SOMO parses `somo.saxs_atoms` (`US_Hydrodyn_Saxs::select_saxs_file`, us_hydrodyn_saxs.cpp:5883):
- **11 tokens** = `name a1 b1 a2 b2 a3 b3 a4 b4 c volume`  → 4-Gaussian, **interleaved**
- **13 tokens** = `name a1 b1 a2 b2 a3 b3 a4 b4 a5 b5 c volume` → 5-Gaussian, **interleaved**

- **4-Gaussian (Cromer-Mann / IT1992) — this is what production uses.** Source: International Tables
  for Crystallography Vol. C, Table 6.1.1.4. Machine-readable open copy: cctbx `eltbx/xray_scattering`
  (`it1992`), https://cci.lbl.gov/cctbx_sources/cctbx/eltbx/xray_scattering.h
- **5-Gaussian (Waasmaier & Kirfel 1995, Acta Cryst A51:416-431), valid to sin(th)/lambda = 6 A^-1.**
  Machine-readable: DABAX `f0_WaasKirf.dat` (211 element/ion entries — contains **every** symbol we
  are missing: Cl, Cl1-, Ni, Br, I, Pt, Au, Hg, As, W, Al, Si, Ag ...).
  http://ftp.esrf.fr/pub/scisoft/xop/DabaxFiles/f0_WaasKirf.dat
  mirror: https://raw.githubusercontent.com/diffpy/libdiffpy/main/src/runtime/f0_WaasKirf.dat
  **DABAX order is GROUPED** (`a1 a2 a3 a4 a5 c b1 b2 b3 b4 b5`) and must be transposed into SOMO's
  interleaved layout. The `volume` column is SOMO-specific (excluded volume) and is NOT in DABAX.

## LATENT BUG: the 5-Gaussian rows in somo.saxs_atoms have shifted b coefficients
Verified against DABAX for C and N (identical pattern in both):
- DABAX C: a = 2.657506 1.078079 1.490909 -4.241070 0.713791 ; c = 4.297983 ;
           b = 14.780758 0.776775 42.086842 -0.000294 0.239535
- SOMO's row parses to b = 0.713791, 14.780758, 0.776775, 42.086842, -0.000294
  i.e. **a5 sits in the b1 slot, b1..b4 are pushed down one, and the true b5 is dropped.**
  (N is the same: a5=0.912985 in the b1 slot, b5=0.217287 missing.) The a's and c are correct.
- **Severity: latent, not live.** The only `US_FFD(5)` construction is inside `#if defined(FFD_TEST)`
  in us_hydrodyn.cpp:272 and that define is commented out, so the 5-term path is not exercised.
  Production uses the 4-Gaussian rows, which I verified ARE correct (carbon matches IT1992 exactly).
  It is a landmine for whoever enables the 5-term path. Because f(0)=sum(a)+c is unaffected, forward
  scattering would look right while the q-dependence would be wrong — easy to miss.

## Mattia's "fanciness note" — IMPLEMENTED
Real PDB files name non-standard residues in the header (`HETNAM`). `read_hetnam()` parses those
(with continuation lines) and `emit_residue()` uses the name as the entry's comment line.
End-to-end check on ACE in 6LDH1: header gives "ACETYL GROUP"; the perceived types (C3H0, O1H0,
C4H3) **match the hand-curated somo.residue ACE entry exactly**, and the comment reproduces
"Acetyl group". mmCIF equivalent for later: `_chem_comp.name`.

## CAVEAT surfaced by that comparison — molvol/vbar estimates run high
Perceived ACE molvol 57.3 vs curated 40.40 (~42% high): summing vdW spheres ignores bond overlap,
and vbar (0.80 vs 0.565) inherits the error.

**RESOLVED 2026-07-20 — the estimate is now removed, not merely warned about.** Fitting a
group-contribution model over the ~100 polyatomic residues somo.residue codes and scoring it
leave-one-out (`tools/psv_model.py`) shows the sum-of-vdW-sphere estimate is the WORST option
available - MAE 0.193 in vbar, worse than simply assuming the global mean (0.110). A hybrid
group-contribution model reaches 0.054 overall and 0.036 for organic residues, but degrades to
0.222 on metal-containing ones (worse than the residue-type group average, 0.122), which are
exactly the residues most likely to be non-coded.

So `emit_residue()` now leaves molvol and vbar at 0 = unset and says why. Emitting a number that
is worse than guessing would be actively misleading, and vbar enters hydrodynamics through the
buoyancy term (1 - vbar*rho). The tiered replacement (group-contribution for organic, group
average for out-of-domain, always user-overridable) is tracked on ticket #980. The entry now also
reports the residue mass, since what actually matters is the residue's mass fraction of the model:
molecular vbar is mass-weighted, so a 616 Da heme in a 50 kDa protein moves it by ~0.5% even with
a 0.34 vbar error.

## Earlier open questions (superseded above, kept for context)
- (Q1) For unknown-residue emission, is per-atom hybrid type + radius/electrons enough, or must the
  prototype also produce bead assignments / vbar / pKa lines to be a *complete* `somo.residue`
  entry? (Those are SOMO modeling choices, not chemistry — I'm treating them as out of scope for
  the perceiver core and stubbing them.)
- (Q2) Default protonation policy: match `somo.residue.new`'s baked-in defaults (His doubly-prot,
  Asp/Glu protonated), or target physiological pH 7 (His neutral, Asp/Glu/­termini charged)?
  v1 matches the file defaults so regression is clean; real "on-the-fly" use probably wants pH 7.
- (Q3) Should the CCD (`components.cif`) lookup be part of this prototype, or a later layer? Not
  present locally; would need a download. v1 is geometry-only; CCD would resolve the ring-N cases.

## INTEGRATION (ticket #978, branch ehb54-issue-978) — phases A/B

**Layout.** The core moved into the SOMO tree and is compiled into `libus_somo`:
`include/us_hydrodyn_perceive{,_elements,_hybrid,_saxs}.h` + `src/us_hydrodyn_perceive.cpp`.
It stays **Qt-free**, so this directory's harness still builds the very same sources without Qt —
that is what keeps the unit tests and the 41k-atom regression runnable and meaningful.

**Adapter.** `us_hydrodyn_perceive_somo.{h,cpp}` is the only Qt-aware layer: converts
`PDB_model`/`PDB_chain`/`PDB_atom` (coords via `point.axis[3]`, note `resSeq` is a QString) into
perceiver input, reads CONECT and HETNAM from the source file, and returns `Tentative` entries.
It converts the WHOLE model, not one residue, because bond perception needs surrounding context
(peptide links, disulfides, metal coordination) to get coordination numbers and implicit-H right.
Non-primary altLocs are dropped, else a duplicated position perceives as a bonded neighbour.

**Headless command.** `perceive <pdbfile>` added to `us_hydrodyn_script.cpp`. It loads via SOMO's
own `screen_pdb()`, builds the "coded" set from `multi_residue_map` (somo.residue is the master, so
only names absent from it are perceived), and prints each tentative entry plus any new somo.hybrid
rows. Run with `us3_somo _pad -I -g <script>`.

## Progress
- [x] Studied somo.hybrid.new (vocab) + somo.residue.new (ground truth), derived conventions.
- [x] Isolated clone + branch + workspace + demo data copied.
- [x] Core perceiver (bonds → rings/aromaticity → Kekulé → classify) + tables + PDB reader.
- [x] Unit tests (18 synthetic-geometry cases, all pass).
- [x] Regression harness (8 demo PDBs vs oracle), iterated to convergence.
- [x] Emit: generate a somo.residue block + synthesized somo.hybrid lines for a residue.
- [ ] (future) CCD-first layer; protonation/pH policy; SOMO integration + -g gui_script driver.

## RESULTS (2026-07-19, 41,219 scored atoms across 8 demo structures)
- exact hybrid-label match: **97.5%**
- physics match (radius/electrons/mass identical): **98.4%**
- **geometric perception, excl. oracle-template + protonation policy: 99.83%**
- genuine perception errors: **0.167% (69 atoms)** — carboxylate/C-terminus conventions + a few
  crystallographic outliers (near-planar CA, long peptide C–N at chain breaks).

### Key iterations (what moved the needle)
1. Bond-length π cutoffs for sp2/sp3 carbon → **broke on low-res structures** with short bonds.
   Replaced with **ring-planarity aromaticity** (whole-ring test, immune to individual bad bonds).
2. **Explicit-H handling**: 2AAS ships hydrogens → they inflated heavy-neighbor counts (N→N3H0,
   O→O2H0). Fix: exclude H from bond perception → H-agnostic (77.7%→97.6% on 2AAS).
3. **Kekulé bond-order** → pyridine (N2H0) vs pyrrole (N3H1) ring N; fixed adenine/cytosine/
   thymine/uracil cleanly.
4. **O carbonyl-vs-hydroxyl by neighbor-carbon hybridization** (not C=O length) → fixed Asn OD1 and
   long-drawn backbone carbonyls.

### FINDINGS worth eb's attention
- **Bug in `somo.residue.new`:** deoxynucleotide C2' is labeled `C4H1` in DA/DC/DG/DT, but
  deoxyribose C2' is a CH2 (no O2') → should be **`C4H2`**. It's the ribose value carried over.
  The perceiver gets it right. This mis-models every deoxynucleotide C2' (1 H / ~1 e- / ~1 Da off).
  Worth a one-line fix in the residue file if confirmed. (144 atoms in the demo set.)
- **Free cysteines:** oracle name "CYS" = disulfide template (`S2H0`); the perceiver returns `S2H1`
  for a Cys with no S-S partner — which is what SOMO's runtime CYS/CYH selection actually does.
  Not an error, just a name-based-oracle artifact in the harness.
- **His / guanine:** protonation state (His) and lactam tautomer (guanine N1/N3) are not resolvable
  from heavy-atom geometry. Total ring H is right; only the per-atom placement differs (negligible
  for hydrodynamics). These are the poster-child cases for the CCD-first design.

## PSV (vbar) from published increments — 2026-07-21

Follow-up to Mattia's non-coded-residue problem. Two tools now sit side by side in `tools/`:
`psv_model.py` fits increments to `somo.residue` itself (leave-one-out MAE 0.115 cm^3/g —
barely better than a group average, so the similarity idea is weak on its own);
`psv_durchschlag.py` uses Durchschlag & Zipper's *published* increments with no fitting.

The published scheme is ~65 numbers and reproduces its own papers exactly (8/8 free amino
acids, 7/7 group increments). Applied to the coded residues it gives ~1.4% mean absolute
error, and vs Perkins 1986 (which is calibrated to experimental protein vbar at +/-0.005
ml/g) ~1.6-1.7% excluding Arg. A scheme that never saw `somo.residue` reproducing it that
closely is the argument for trusting it on ligands the table does *not* cover.

On the two SOMO ligands with measured values it beats the shipped numbers outright:
glycerol exp 0.767 / SOMO 0.793 / calc 0.760; NAD+ exp 0.62 / SOMO 0.599 / calc 0.617.

### FINDINGS worth eb's attention
- **`somo.residue` vbar = Cohn & Edsall (1943) minus exactly 0.002 cm^3/g.** 17 of 19 match
  to the last digit; Asp and Glu resolve exactly using the unrounded C-E values (0.605,
  0.665). The offset is a 25→20 C correction: 5 K x 4.25e-4 = 0.002125, which is precisely
  the constant commented out at `us_hydrodyn_load.cpp:925`.
  NOTE: an earlier draft of these notes also claimed SOMO's Traube covolume was 0 / unused.
  That was WRONG (Mattia caught it): gparams["covolume"] defaults to 12.4
  (us_hydrodyn_settings.cpp:3347), is persisted in all 99 configs here, and is added once per
  structure in calc_vbar_updated (us_hydrodyn_load.cpp:841) -- the single-covolume-per-structure
  model. It only falls back to 0 if a pre-covolume config JSON is loaded; none exists here.
  So `somo.residue` is a **20 C
  table** — which is correct by design, not a defect: `US_Hydrodyn::tc_vbar()`
  (`us_hydrodyn_load.cpp:1899`) corrects 20 C -> the working `hydro.temperature` at run time
  with the same 4.25e-4 coefficient, and for a user-entered vbar it first backs out the
  measurement temperature. It is applied in the GRPY, ZENO, SupC and reporting paths, and the
  commented-out line at :925 would have double-applied it. Temperature handling is fine.
  (An earlier draft of this section claimed otherwise — that was my error, caught by Mattia.)
  The one thing worth knowing for *new* PSV work: 4.25e-4 is applied uniformly regardless of
  composition, whereas D-Z quote 2-10e-4 across compounds and recommend 5e-4 generally. Fine
  for protein residues; a non-coded ligand could sit further from that single value.
- **Bead volumes and vbar disagree for the non-standard entries.** They are consistent for
  amino acids and nucleotides (bead/psv = 1.02 +/- 0.04) but not for cofactors, detergents
  and ions: heme 0.60, SF4 0.57, CLF 0.59, NAD 0.79, B12 0.77, the whole detergent block
  0.64-0.71, PO4 1.42, K 5.76. Nothing in the code cross-checks the two.
  **The bead volumes are the wrong half**, not the vbars: SOMO's Triton X-100 (OXN) vbar of
  0.912 matches the measured micellar value 0.9125 (Durchschlag 1986 Table 2), and glycerol's
  bead volume is ~34% too small (77.9 A^3 vs the 117 A^3 its own vbar implies). Glycerol is
  common in PDB entries, so this is live, and it is independent of any new PSV work.
- **Arg (+9%) and Lys (+6%) are the only real residuals** and both are the ammonium/
  guanidinium N assignment. His was +8% until charge was read from the hybrid names rather
  than guessed — worth noting that the `+`/`-` markers in `somo.residue` are load-bearing and
  the perceiver must keep emitting them.
- **`SE2H0` exists in `somo.residue.new`** (selenomethionine). Durchschlag & Zipper 1994
  Table 2 lists a metal as "Sc 16.5"; scandium is odd company for Mg/Mn/Fe/Co/Ni/Cu/Zn/Mo/Hg
  and selenium would fit. Unverified against a clean original — do not rely on it.

## 3V probe radius for the anhydrous volume field — 2026-08-08

The Residue Definition Module doc says to fill "Residue anhydrous mol. vol." using 3V with a
**0 A probe radius**. Mattia's reasoning was that a buried ligand (HEME, NAD) is not water-
accessible, so water accessibility should not set its volume. Tested directly. 3V source is at
https://github.com/vosslab/vossvolvox (the 3vee.molmovdb.org site is long down); it builds with
one `make` on macOS, `bin/Volume.exe`.

### The tabulated amino-acid entries are on the SOLVENT-EXCLUDED (1.4 A) basis
Lysozyme 1HEL, 129 residues, SOMO's own per-atom radii:
- sum of tabulated somo.residue molvols  17374 A^3
- 3V whole protein, probe 1.4 A          16364 A^3   (ratio 0.94)
- 3V whole protein, probe 0 A            11663 A^3   (ratio 0.67)
The 0.67 is just the protein packing fraction -- the interstitial space water cannot enter,
which belongs inside the bead and which a 0 A probe discards.

### Measuring a BOUND ligand: the difference method works, but must be calibrated
`V_ligand = V(complex) - V(complex minus ligand)`. Valid only because **Volume.exe leaves
interior cavities UNCOUNTED**, so the vacated pocket does not contribute to V(apo). Proven on a
synthetic sealed shell (500 atoms r=2.0 on a sphere R=10):
- Volume.exe       5032 A^3  vs analytic shell-only 5094  -> cavity NOT counted  <- use this
- VolumeNoCav.exe  7166 A^3  vs analytic full ball  7238  -> cavity filled
(`volume.cpp` marks probe-fitting points accessible by a local test; `volume-fill_cavities.cpp`
flood-fills from outside first. Using the wrong binary collapses the difference to ~0.)

The difference is **size-dependent** -- deleting a small residue leaves a gap too narrow to admit
a 1.4 A probe centre, so the gap stays counted and the difference underestimates; deleting a
large one opens a cavity that swallows neighbouring interstitial space and overestimates.
`tools/calibrate_3v_context.py` measures this against residues whose answer we already know.

### Production calibration (386 measurements, 6 proteins: 1MBO 1HEL 2LZM 1UBQ 5PTI 1LDM:A)
`tools/calibrate_3v_context.py`, probe 1.4 A, grid 0.25, burial >= 85 neighbours within 8 A,
composition required to match the tabulated entry exactly (no partial side chains, no termini).

| tabulated size band | n | mean ratio | median | sd |
|---|---|---|---|---|
| < 100 A^3   |  78 | 0.705 | 0.735 | 0.274 |
| 100-140     | 102 | 0.895 | 0.881 | 0.226 |
| 140-180     | 147 | 1.043 | 1.029 | 0.136 |
| 180-220     |  44 | 1.011 | 0.985 | 0.142 |
| >= 220      |  15 | 1.101 | 1.116 | 0.160 |

**CORRECTION FACTOR for ligand-sized species (tabulated >= 180 A^3): 1.034, sd 0.152, sem 0.020
(n=59).** So `V_ligand = in-context difference / 1.034`. Burial above the threshold does not
matter (0.996 / 1.093 / 0.946 across burial bands -- no trend, all within scatter).
Two precisions, do not confuse them: the *factor* is known to ~2% (sem), but a *single* ligand
measurement carries the ~15% per-residue sd, because the bias depends on local packing.

Most reliable types (low sd): LYS 0.949+-0.054, TYR 1.016+-0.090, HIS 1.006+-0.103,
ARG 0.963+-0.119, ILE 1.015+-0.120. Worst: GLY 0.458+-0.326, CYS 0.677+-0.261, ASN 0.873+-0.302.

### Result: the ligand entries are ~33% low, on a vdW (0 A) basis
| ligand | in-context diff | grid-stable | calibrated (/1.034) | tabulated | tab/measured |
|---|---|---|---|---|---|
| HEM in myoglobin 1MBO | 825 A^3 | 823-828 @ g=0.20-0.25 | **798** | 536.13 | **0.67** |
| NAD in LDH 1LDM:A     | 817 A^3 | 816.5-817.4           | **790** | 517.75 | **0.66** |
Two ligands, two proteins, same ratio -- and it equals the vdW/solvent-excluded packing ratio
measured independently on lysozyme (0.67). Robust to the correction factor: anywhere in
1.0-1.2 puts the ligands at 688-825 A^3, i.e. tabulated = 0.65-0.78 of truth either way.

### Conclusion: 1.4 A is the correct probe
The decisive argument is consistency, not physics. A buried Trp is exactly as water-inaccessible
as a buried heme, yet its tabulated 228.2 A^3 is on the solvent-excluded basis. The packing void
around a buried group is real volume the amino-acid convention already includes, so ligands must
use the same basis or the two halves of a bead model are not comparable.
=> the ~31 ligand molvol entries need recomputing (~1.5x), and their SoMo bead volumes cascade.

### FINDINGS worth eb's attention
- **The difference method FAILS for small ligands** (tabulated < 130 A^3): ratio ~0.70 and
  erratic, because the vacated gap cannot admit a probe centre. GOL, PEG, ACE, PO3/PO4 and the
  other small entries need a different route (isolated 3V at 1.4 A plus a packing correction),
  which is NOT resolved.
- Heme's propionates are solvent-exposed in myoglobin, so 825 is probably a slight underestimate
  of the fully-buried case.
- Use `Volume.exe`, never `VolumeNoCav.exe`, for this measurement.
- **The correction factor is an extrapolation.** It is calibrated on amino acids, whose largest
  entry is TRP at 228 A^3; heme and NAD are ~800 A^3, i.e. 3.5x beyond the calibration range.
  The >= 220 band trends *upward* (1.101), so the true factor for a large ligand may exceed
  1.034. This does not change the conclusion (see the robustness note above) but it does mean
  the calibrated ligand volumes carry more than the quoted sem.
- **Parsing trap in Volume.exe output** (cost me a full silent run): the data line is
  `probe<TAB>grid<TAB>volume<TAB>surf_area<TAB>natoms<TAB>file`. Parse by POSITION (field 3).
  A "first number greater than 1" heuristic returns the *probe radius* 1.4 for every call, so
  every difference comes out exactly 0.000 and the run looks superficially fine.

## Recomputing the prosthetic-group volumes — 2026-08-08 (Mattia's procedure)

Mattia: "run 3V again on each prosthetic group and recalibrate proportionally the bead volumes.
For safety, run 3V with 0 (should get back the actually stored value) and 1.4."

### The safety check passes exactly -> provenance confirmed
3V at probe 0 on the isolated group, using **3V's own default radii** (NOT SOMO's radii, which
give HEM 498):
    HEM  536.0 vs stored 536.13 (0.02%)      NAD  517.1 vs stored 517.75 (0.13%)
The amino acids do not reproduce this way (ALA 3V@0 = 76.2 vs stored 90.0), confirming they came
from the crystallographic literature. The two halves of the table really are on different bases.

### But a bare 0->1.4 rescale UNDER-corrects
Going 0 -> 1.4 A on an isolated molecule only fills its own surface crevices; it cannot see the
interstitial packing void a group carries inside a protein, which is what the Voronoi-based
amino-acid values include. Measured over all 20 amino acids, 6 instances each, 4 proteins
(stored / V_isolated@1.4):
    large compact (Ile Leu Met Phe Tyr Trp)  **1.204**  sd 0.009
    small (< 160 A^3)                        **1.131**  sd 0.032
    large flexible (Lys 1.077, Arg 1.147)    lower - extended chains carry less void
So: **new_molvol = 1.204 (or 1.131 if small) x V_isolated@1.4**, i.e. ~1.31x stored, not the
1.09x a bare rescale gives. `tools/recalc_ligand_volumes.py` implements exactly this.

### Two guards, because a plausible number on the wrong molecule is the real danger
1. **Elemental formula must match** the somo.residue entry. Many SOMO codes collide with
   unrelated PDB chemical components. 13 genuine collisions found -- applying CCD blindly would
   have been destructive:
   `BF4` (SOMO = a C30FE1N6 iron complex, PDB = tetrafluoroborate), `MCA` (SOMO C10N1O1,
   PDB C25N7O19P3S1), `LIP`/`LP2`/`LP3` (POPC/DMPC/DMPG), `OX9`, `MEN`, `MO2`, `MO6`, `PEG`
   (C2O1 repeat unit), `SUC`, `SUL`, `XHY` (a 1-atom pseudo-entry).
2. **V@0 must reproduce the stored value** -- confirms the conformer matches. Isolated volume is
   conformation-dependent: CCD ideal reproduces NAD (518.0) but not HEM (585.6 vs 536.13, +9%)
   because heme's propionates are extended in the idealised conformer and folded when bound.
   Use `--from-pdb HEM=1MBO.pdb`, which then audits 1.000 and gives HEM 536.13 -> **701.0**.

### Result over all non-standard entries
**18 ACCEPT** (both guards clean), factors 1.16-1.51, mostly ~1.25-1.32:
ADP 296.6->392.7, AMP 258.9->339.8, ATF 348.6->456.9, ATP 333.9->441.7, B12 1212.6->1598.6,
13P 118.3->139.3, CFN 241.1->363.1, CLF 172.5->255.0, DMP 580.1->748.2, GOL 77.9->90.4,
HCA 147.5->178.5, LNC 591.8->764.3, NAD 517.8->668.8, NDP 560.8->720.4, SF4 96.4->132.2,
PO4 47.9->59.6, LMT 491.6->616.9, OXN 702.4->929.4.  Plus HEM 536.1->701.0 via --from-pdb.

**13 RESIDUE FORM - DO NOT CORRECT**: CGU ORN PCA SAC BMA FUC GAL MAN GLC NAG NDG NGA SIA.
CCD differs by exactly one O = free monomer vs polymer residue (bond formation sheds H2O).
These also had bead/vbar ratios ~1.0 in the original audit, i.e. they are **already on the
literature/Voronoi basis** like the amino acids. Correcting them would be wrong.

**Still open**: 5 conformer-only cases (PO3 OXM BOG ACE BEF) need a bound conformer; 13 collisions
need manual coordinates; 19 have none at all (the `PBR-*`/`OXT-P` pseudo-residues, and the metals
CA CD CL CU K MG MN NA PD ZN plus NH2 CMO OXY, where 3V fails on 1-3 atoms -- for a monatomic ion
the volume is just (4/3)pi r^3 and needs no 3V).

### FINDINGS worth eb's attention
- **The correction is NOT uniform** -- it ranges 1.16x (GOL) to 1.51x (CFN). A single global
  factor would be wrong; each entry needs its own measurement.
- The independent in-context route (calibrate_3v_context.py) gives ~795 A^3 for HEM and NAD vs
  ~700/669 here, i.e. **this route may still be ~15% low**. The two bracket the truth. Both are
  far above the stored values, so the direction and rough size of the correction are safe.
- Bead volumes must be rescaled in step with molvol -- for the 1-bead entries they are the same
  number, but multi-bead entries need the per-bead split rescaled proportionally.

## Hydration for a new residue (Mattia's step 3) — 2026-08-08

Mattia: "build a table of atom hybridizations from our somo.residue table linked to their
hydration numbers ... proposed atomic hydration are shown, to be interactively modified."
Built: `tools/hydration_table.py` (`--cpp` emits a ready-to-paste map).

### What somo.residue's hydration actually is
Stored **per atom** (field 8 of each atom line). Summed per residue it reproduces the classical
**Kuntz** per-residue numbers (Durchschlag 1986 Table 1, pH 6-8): 14 of 20 amino acids agree
within 0.5 water. The two large misses are Asp (SOMO 1 vs Kuntz 6) and Glu (1 vs 7.5) -- exactly
the residues SOMO stores PROTONATED, so they carry no charged-carboxylate water.
=> the per-atom values are a **hand distribution of a per-residue Kuntz total**, not an
independently derived per-atom rule. Same group, different entry, different answer:
ADP.O2'=1 but A.O2'=0; BMA.O2/O3/O6=1 but BMA.O4=0; MET.SD=1 but CYS.SG=0.

### The derived table
48 hybrid types: **36 unanimous, 5 majority, 7 WEAK**.
- unanimous and safe: every carbon type = 0, `O2H0B` = 0, `O2H02-` = 0, `S2H1` = 0, and the
  metals (MG+2 = 6, CU+2 = 6, CL-1 = 6, MN+2 = 4, CA+2/CD+2/ZN+2/PD+2 = 2, NA+1 = 1).
- **WEAK, must be flagged not silently defaulted**: `O2H1` (default 1, only 55% of 121 obs),
  `O2H0` (0, 75% of 115), `N3H2` (1, 59%), `N3H0` (0, 74%), plus `O1H0-`, `S2H0`, `N4H3+`
  (2-4 observations each).

### How to use it
Propose per-atom defaults from the table, flag the WEAK ones for review, and **also show the
residue TOTAL** -- the total is the quantity with literature backing (Kuntz), the per-atom split
is convention. This is a starting point for interactive editing, exactly as Mattia specified, and
must not be presented to the user as a physical prediction.

### FINDINGS worth eb's attention
- Hydration inherits the **same pH/ionization dependence as vbar**. Asp/Glu are the proof: their
  hydration is 1 protonated vs 6-7.5 charged. So the deferred pH layer affects hydration at least
  as strongly as it affects psv, and a charged novel ligand will be badly under-hydrated by any
  table built from SOMO's neutral-stored entries.

## Bead colours (Mattia's step 7) + gui_script defaults for pipeline use — 2026-08-08

Colour list is in `us_somo/somo/doc/manual/somo/somo_residue.html` (Panel 3). Verified against
the colour field of every bead line in `somo.residue.new` -- the doc and the table agree, and the
reserved indices are genuinely unused in the table.

| idx | colour | meaning | in table |
|----|--------|---------|---------|
| 0 | Black | **RESERVED** - auto-assigned to very small beads, always excluded from computation | unused |
| 1 | Blue | protein main-chain | 39 |
| 2 | Green | protein acidic side-chain (D,E) | 4 |
| 3 | Cyan | protein hydrophobic side-chain (A,V,L,I,F,W) | 8 |
| 4 | Red | protein polar side-chain (H,Y,S,T,N,Q) | 9 |
| 5 | Magenta | protein non-polar side-chain (C,M,P) | 7 |
| 6 | Orange/brown | **RESERVED** - buried beads, auto-assigned during model generation | unused |
| 7 | White | **USED** for fused beads | unused |
| 8 | Grey | **USED** for previously-buried beads found exposed on re-check | unused |
| 9 | Light Blue | lipid tails, carbon monoxide | 33 |
| 10 | Light Green | "USED by the Automatic Bead Builder for non-coded residues" | 5 |
| 11 | Light Cyan | bases in DNA/RNA, oxygen, 13P | 36 |
| 12 | Light Red | heme, NAD, cofactors, prosthetic groups, ions, PO2, lipid heads | 80 |
| 13 | Light Magenta | carbohydrates (incl. sugar rings in nucleotides), some ions | 38 |
| 14 | Yellow | protein basic side-chain (K,R) | 3 |
| 15 | Bright White | unassigned | 2 |

Offer the user 1,2,3,4,5,9,10,11,12,13,14,15. **Never offer 0, 6, 7, 8** -- SOMO already pops a
warning for 0 and 6; 7 and 8 are equally unsafe and the doc says so.
The colour is not cosmetic: it is how the bead is categorised (and 0/6 are how beads get excluded
from the hydrodynamics), so a wrong colour silently changes the model.

### OPEN QUESTION for Mattia -- which default for a non-coded prosthetic group?
The doc says **10 (Light Green)** is "used by the Automatic Bead Builder for non-coded residues",
which is literally our case. But in the actual table 10 is used by A/DA/DG/G/NH2 (nucleotides),
while **12 (Light Red)** is the one described as "heme, NAD, other co-factors, some prosthetic
groups and ions" and is the largest class (n=80). These point different ways; 12 looks right for a
cofactor-like ligand and 10 right for "machine-generated". Needs a ruling before pipeline mode
picks one silently.

## gui_script: non-interactive defaults for pipeline use (eb, 2026-08-08)
Requirement: the whole new-residue flow must run with no user intervention.
Existing commands (`us_hydrodyn_script.cpp:160`): `perceive <pdb>`, `perceive compare <pdb>`.
Proposed additions, following the same sub-option style:

    perceive auto <pdb> [outfile]      # full flow, all defaults, never blocks on a dialog
    perceive interactive <pdb>         # the guided 8-step flow (RasMol, prompts)

with per-step defaults as `gparams` so a pipeline can pin them (same pattern as `covolume`):

    perceive_default_color    12       # see OPEN QUESTION above
    perceive_default_beads     1       # Mattia: single bead for now
    perceive_hydration_mode    table   # table | zero
    perceive_volume_probe      1.4     # NOT 0 -- see the 3V section above
    perceive_volume_factor     auto    # 1.204 large / 1.131 small
    perceive_psv_mode          durchschlag
    perceive_write_residue     false   # writing back to somo.residue stays OPT-IN

Two rules that matter for batch safety: `perceive auto` must never open a dialog or a RasMol
window (it is the headless path), and `perceive_write_residue` must default to **false** so a
pipeline run cannot silently mutate the master table -- consistent with the earlier decision that
storing a confirmed residue is a user option.

## Durchschlag 1986 Table 2 mining — 2026-08-08

Mined `978-3-642-71114-5_3.pdf` Table 2 ("Specific volumes of small molecules") for measured
partial specific volumes, to (a) audit SOMO's stored vbar and (b) validate the D-Z increment
method on ligand-class chemistry rather than only on amino acids.
86 compounds parsed: **72 with genuinely measured values, 12 calculated-only.**

### TRAP: most of the interesting ligand entries are CALCULATED, not measured
The chapter's symbol list defines **v_c = "calculated partial specific volume"**, and the table
annotates such rows `(v_c)` -- which the OCR renders variously as `(vc) (v c) (ve) (v e) (vJ)`.
These are additivity OUTPUT, so they cannot validate an additivity method or audit SOMO.
Dropped on that basis: **ATP, Galactose, Mannose, Fucose, N-Acetylglucose/galactose, Acetyl-CoA,
CTP, carbamyl phosphate, PALA, succinate, sodium glyoxylate, sodium pyruvate.**
My first pass missed this and produced two spurious "outliers" -- ATP +17% and PO4 -40% -- both
of which are comparisons against calculated numbers. Always check the annotation.
Measured markers, by contrast, are bare values or `(v3)/(V3)` (= v-bar of component 3) and `(phi3)`.

### (a) SOMO's stored vbar is in good shape -- no correction needed
Measured values only, sugars converted to residue basis (-H2O, -12.5 cm^3/mol, Perkins 1986):

| entry | somo vbar | measured | diff |
|---|---|---|---|
| GOL | 0.793 | 0.770 | +3.0% |
| NAD | 0.599 | 0.620 | -3.4% |
| OXN / OX9 | 0.912 | 0.912 | -0.1% |
| SIA | 0.579 | 0.580 (free 0.587) | -0.2% |
| GLC | 0.605 | 0.615 (free 0.623) | -1.6% |

n=6, mean |d| **1.4%**, worst NAD -3.4%. **This is the clean separation the project needed:
`vbar` needs no correction, `molvol` did (30-50%).** The validation set is small only because
Durchschlag's ligand entries are largely calculated -- an inherent limit, not an oversight.

### (b) The D-Z method on measured, non-amino-acid chemistry
11 compounds, structures hand-encoded, measured values only:
urea -0.5%, glycerol -1.2%, glucose -1.6%, sucrose -1.9%, acetic acid -2.3%, propionic +0.5%,
butyric -1.8%, ethanol -2.3%, propylene glycol -1.8%, betaine +2.8%, TMAO +10.3%.
**mean +0.0%, mean|d| 2.5%, median|d| 1.8%, 10/11 within 3%** -- consistent with D-Z's own
claim (75% within 2%, 90% within 3%) and with the +-2-3% we quote for a novel residue.
The single failure, **trimethylamine N-oxide (+10.3%)**, is the semipolar N->O bond: Table 1 has
a dedicated "O in an amine oxide" increment and a `-N-O (amine oxide) 7.5` group in Table 4, so
this is my encoding, not the method. It is a good regression case for the C++ port.

### FINDINGS worth eb's attention
- The structures above are **hand-encoded by me** -- element/group counts, ring sizes, charges.
  That is precisely the perception step the C++ perceiver automates, so this validates the
  increments and arithmetic, NOT the end-to-end pipeline. The end-to-end number is still unmeasured.
- Sucrose is a name collision worth noting: SOMO's `SUC` is C7N2O3, **not** sucrose (C12O11).

## DECISIONS (eb, 2026-08-08)
- **D.2: implement the grid volume natively** in SOMO rather than shelling out to 3V's
  `Volume.exe`. Removes the only new external dependency. 3V stays as the *validation oracle*.
- **Default bead colour = 10 (Light Green)** for machine-generated non-coded residues, per the
  manual's "USED by the Automatic Bead Builder for non-coded residues". Must be a **named,
  single-point-of-definition constant** so the choice can be changed later, not a literal
  scattered through the code.

### D.2 implemented: native grid volume (no 3V dependency)
`include/us_hydrodyn_grid_volume.h` + `src/us_hydrodyn_grid_volume.cpp` (Qt-free),
test `tests/grid_volume.cpp` (`make gridvol DATA=<dir of .xyzr>`).

    accessible A = { v : |v - c_i| > r_i + probe for every atom i }   (probe-CENTRE positions)
    excluded   E = { v : v not in A, and dist(v, A) > probe }
    volume       = |E| * grid^3

probe 0 -> bare vdW union; probe 1.4 -> solvent-excluded. Interior cavities are deliberately NOT
counted (a sealed void that can host a probe centre is in A), which is what makes
V(complex) - V(apo) measure a bound ligand. Only the A/blocked *boundary* is dilated, since the
nearest A voxel to any blocked voxel is always a boundary one -- a surface not a volume, which is
what makes a 0.25 A grid affordable (~1 s for a 1300-atom protein).

Validated against 3V `Volume.exe`, the tool it replaces:
| case | native | 3V | diff |
|---|---|---|---|
| ALA residue, probe 0 | 68.0 | 68.2 | -0.22% |
| ALA residue, probe 1.4 | 71.9 | 71.9 | +0.04% |
| myoglobin holo, probe 1.4 | 20870.6 | 20889.1 | -0.09% |
| myoglobin apo, probe 1.4 | 20049.5 | 20061.5 | -0.06% |
| HEM in-context difference | 821.2 | 827.6 | -0.8% |
plus analytic checks: single sphere within 1%, coincident spheres give the union not the sum,
and the sealed hollow shell returns shell-only (5038 vs analytic 5094, full ball would be 7238).
10 checks, 0 failures. **3V is retained only as the validation oracle, not as a dependency.**

### BUG FOUND AND FIXED: emit_residue wrote the atom count into the bead COLOUR field
`emit_residue` emitted `"0\t" << atoms.size() << "\t0\t0\t0"`, but the bead line is
`hydration, colour, placing_method, chain, volume` (US_Hydrodyn::read_residue_file,
us_hydrodyn_load.cpp:509). So the colour was the atom count:
- a **6**-atom novel residue got colour 6 = RESERVED brown = *silently excluded from the
  hydrodynamic computation*; 7 and 8 atoms hit the other two reserved colours;
- anything above 15 atoms was simply out of range.
Now emits `DEFAULT_BEAD_COLOR` (= 10, Light Green) from a single point of definition in
`us_hydrodyn_perceive.h`, alongside `bead_color_is_reserved()` / `bead_color_is_selectable()`
helpers and the full documented colour list. Unit suite still 54 checks / 0 failures.

## Phase A: SSSR ring perception + bond graph exposure — 2026-08-08

Prerequisite for the Durchschlag & Zipper volume increments, which charge a ring-formation
decrement **once per ring** (Table 1: 3-ring 2.1 ... >=9-ring 14.1). The perceiver previously
exposed only an `aromatic` flag and never returned `Bonds` at all, so ring *sizes* and the
topology needed to classify an N or O environment were unavailable -- the Python prototype had
to be hand-fed both.

- `Bonds` gains `rings` = **SSSR** (smallest set of smallest rings), and `find_sssr()` is
  declared in the header so it can be unit-tested on hand-built adjacency without geometry.
- New `perceive(atoms, bonds_out, explicit_bonds)` overload returns the graph; the old
  signature delegates to it, so existing callers are untouched.
- SSSR is deliberately **separate from** the existing `find_rings()`, which enumerates every
  simple 5-/6-cycle and feeds aromaticity. That is the right input for "is this atom in a flat
  conjugated ring" but the wrong one for anything charged per ring: a fused bicyclic has 3
  simple cycles and circuit rank 2. Keeping them separate also avoids perturbing the validated
  99.833% perception -- confirmed, the regression figure is unchanged to the digit.
- Method: smallest cycle through each bond (BFS with that bond removed), then greedy
  smallest-first acceptance while a candidate covers an uncovered bond, until circuit rank
  |E| - |V| + |components|. Rings beyond `max_ring` (default 12) are not sought, since the
  consumer charges one "large ring" term for everything from 9 up.

### Tests
`tests/sssr.cpp` (`make sssr`) -- 35 checks on hand-built graphs: acyclic shapes, every ring
size 3..9, ring-plus-substituent, **fused bicyclics (naphthalene, indole, purine) giving 2 rings
not 3 cycles**, spiro, bridged/norbornane, three-fused, disconnected components, macrocycle
beyond max_ring, idempotency, and that every reported ring is a genuine cycle. Also asserts the
arithmetic the counts drive: one 6-ring decrement reproduces D-Z's published Phe 121.9, one
5-ring their Pro 80.1.

`tests/sssr_real.cpp` (`make sssrreal`) -- 18 checks on real coordinates through the whole
perception path, across all 8 demo structures: PHE {6} x171, TYR {6} x150, **TRP {5,6} x47**,
HIS {5} x138, PRO {5} x244, DA/DG {5,5,6} x72, DC/DT {5,6} x72, and Ala/Gly/Leu/Ser/Arg
ring-free. Every instance of a residue type gives an identical signature.

### FINDINGS worth eb's attention
- **Some demo structures contain physically impossible geometry**, which produces spurious small
  rings. `1AO6` models LYS536 and LEU583 on top of each other -- LYS536/NZ to LEU583/CG is
  **0.73 A**, shorter than any covalent bond (C-C is 1.54), B-factors 60-85. `6LYZ` (an early
  low-resolution lysozyme) has LYS97/O 1.22 A from ASP101/OD1, two oxygens at an impossible
  separation. `3GUT`'s corrupt LEU367 bond was already known. Across the demo set this yields 13
  cross-residue rings and 12 three-membered rings -- **none of them intra-residue**, so no
  residue's own ring count is affected.
- This is pre-existing bond perception on defective input, not an SSSR defect, and the tests now
  assert it as such: every cross-residue ring must be closed by an inter-residue bond that is
  neither a peptide/phosphodiester link nor a disulfide, and no 3-membered ring may lie inside a
  single residue.
- **Design consequence for the volume work**: a residue's ring decrements must count only rings
  whose atoms all lie in that residue. That is both correct chemistry (the increments are
  per-molecule) and immunity to this whole class of input defect.
