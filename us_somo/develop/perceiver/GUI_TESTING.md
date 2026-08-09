# Perceiving non-coded residues — GUI cheat sheet

For testing entirely from the SOMO window. Nothing here needs a command line.

## The loop

1. **Load a PDB** that contains a residue `somo.residue` does not code.
2. If a dialog about the unrecognized residue appears, **stop processing** (see below).
3. **Lookup Tables → Perceive Non-Coded Residues…**
4. Review each proposed entry, then **Accept** or **Skip**.
5. **Build the bead model** (SOMO / SoMo-overlap / vdW / AtoB).

Everything the perceiver does is echoed to the text window, including every reason it
declines to do something. If the menu item appears to do nothing, the text window will say
why — that is the first place to look.

## Structures to test with

In `us_somo/develop/perceiver/data/non-coded/` (they come with the branch):

| file | non-coded | what it exercises |
|---|---|---|
| `2CMD.pdb` | citrate, 13 atoms | **start here** — carboxylates, 3 atoms flagged for review |
| `3PTB.pdb` | benzamidine, 9 atoms | the only ring-bearing case; its Ca²⁺ *is* coded and is left alone |
| `1MBO.pdb` | sulfate, 5 atoms | the trivial case; its heme and O₂ are coded, so it also shows selectivity |
| `5PTI.pdb` | 18 types, 50 instances | the pathological case — a neutron structure whose deuteriums make every residue look non-coded. Use **Skip all remaining**. Its entries are not meaningful |

The demo structures shipped with SOMO are all fully coded, so the menu will correctly report
there is nothing to perceive.

## The unrecognized-residue dialog

SOMO puts up its own dialog when it meets a residue it cannot code. **For now: choose the
option that stops processing**, then go to Lookup Tables → Perceive. Perceive first, build
second — the perceiver reads the structure as loaded, and letting the bead builder run first
leaves the Automatic Bead Builder's `_NC` placeholders in the model.

Wiring the perceiver directly into that dialog is the obvious next step, so **please note the
exact wording and the exact buttons you see** — it will make the hook-up unambiguous.

## Accept vs. save — how long does an entry last?

This is the part worth being precise about.

| | where it goes | how long it lasts |
|---|---|---|
| **Accept** (box unticked) | `<home>/etc/somo.residue.perceived`, which becomes the active lookup table | **this SOMO session only** |
| **Accept** + *add to somo.residue* | also appended to your real `somo.residue` | permanent |

On Accept the entry takes effect immediately — the structure is re-read and the residue stops
being non-coded, so you can build straight away. The label next to **Select Lookup Table**
changes to `somo.residue.perceived` so you can always see which table is live.

**On restart the perceived entries are gone.** SOMO resets the active table to
`somo.residue` every time it starts; the `.perceived` file is still on disk but is not
loaded. Worse, the *next* session's first Accept rebuilds that file from `somo.residue`, so
anything accepted in an earlier session is overwritten at that point.

Two ways to keep entries:

- Tick **add to somo.residue** in the dialog — permanent, and what you want once an entry is
  reviewed and correct.
- Or, before accepting anything new in a later session, press **Select Lookup Table** and pick
  `somo.residue.perceived` by hand. That reloads the previous session's entries without
  touching your `somo.residue`.

Accepting several residues in one session accumulates them all into the one overlay.

## What to be sceptical of in a proposed entry

The dialog flags what it is unsure of; those flags are the point, not noise.

- **Hydration** is the weakest of the three computed numbers — proposed from pH 7 chemistry
  rules, and falls back to a coded-residue average for groups the rules do not recognise
  (all four sulfate oxygens, for example). The residue *total* is the quantity with
  literature backing; the per-atom split is convention. **vdW models rely on atomic
  hydration**, so that is where a weak proposal shows up as a real modelling problem.
- **psv** is good to ±2–3% for ordinary neutral organic chemistry, and poorer for charged
  groups, lipids and ionic detergents.
- **ASA** is a placeholder, not a measurement. Nothing computes with it — but it may not be
  zero, or the entry silently fails to load.
- **Everything is editable** before you accept: hydration per atom, volume, psv, bead colour.
  Atom names, hybrid types, masses and radii are perceived and locked, because editing them
  would desynchronise the entry from the geometry.

## Known rough edges

- A perceived residue is a **single bead** with all atoms assigned to it. Multi-bead residues
  are not proposed yet.
- An atom whose name is not in `somo.atom` (a sulfate's `S`, say) is resolved from its hybrid
  or element rather than dropped; the text window says so when it happens.
- The **sulfur radius of 0.43 Å** you will see on a sulfate comes from `somo.hybrid`, not from
  the perceiver. It is irrelevant to SOMO-mode beads, where the residue volume drives the
  bead, but it is wrong for vdW models and ASA — worth a decision.
