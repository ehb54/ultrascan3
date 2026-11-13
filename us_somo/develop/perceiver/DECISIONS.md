# SOMO residue/hybrid perceiver — decision & progress log

Running log for **eb**'s review. Autonomous session started 2026-07-19.
Workspace: `~/claude/ultrascan3-perceiver` (isolated local clone), branch `ehb54-residue-perceiver`.
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
