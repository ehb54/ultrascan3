# Analyte XML compatibility

Scope: `US_Analyte` disk/device reader and writer. Regression tests are in
`test_us_analyte_xml.cpp`. Synthetic and external generated fixtures establish
current behavior; they do not establish coverage of all historical lab files.

## Current contract

| Area | Reader | Writer / consequence |
|---|---|---|
| Envelope | Accepts alternate wrappers, namespaces, unknown fields and known descendants within unknown wrappers | Emits `US_Analyte` DTD and `AnalyteData version="1.0"` |
| Selection | Reads requested GUID, or first analyte if the device API receives no GUID; ignores other/nested analytes and fields outside the selected element | Emits one analyte |
| Types | Exact PROTEIN/DNA/RNA/CARBOHYDRATE; unknown/missing type retains existing type | Emits uppercase supported type |
| Missing fields | Missing/invalid numbers become zero; absent description/GUID become empty; unmentioned sequence/maps can survive | Writes description/GUID/vbar; mw only when positive |
| Sequence | Preserves text/CDATA for protein/carbohydrate; lowercases populated nucleotide sequences | Adds leading/trailing newline and wraps at 80 characters; exact multiples include an extra final newline |
| Protein | Sequence computes E280 and computes mw only when zero; subsequent explicit E280 overrides computed E280 | Computes vbar below 0.01, mutating object |
| DNA/RNA | Populated sequence recalculates mass; empty/whitespace-only sequence retains explicit mass (or zero if absent); attributes supply flags/counterions when sequence is present | Emits flags/counterions; missing sequence and empty sequence remain distinct for state/defaulting |
| Booleans | Only `1` and `T` mean true | Emits `1`/`0` |
| Spectra | Merges with existing maps; repeated frequency last wins; missing/invalid numeric attributes become zero | Sorted keys; frequencies use `max_digits10` significant digits to preserve distinct finite double keys; value precision unchanged |
| XML errors | Malformed/truncated XML or device read failure returns DBERROR without publishing partial data | Device short-write/error returns DBERROR; output may be partial |
| No match | Device API returns NO_ANALYTE when a well-formed document has no selected analyte; filename search returns NO_ANALYTE when no matching file is found | No matching data is published |
| Diagnostics | Failed reads change only message; successful selected reads clear stale message | Successful device writes clear stale message |

Filename discovery scans A*.xml files for a matching analyte and then validates
that selected file. Unrelated files that cannot be opened or contain no discovered
match do not prevent finding a valid later file. This directory-search behavior
is distinct from validating a supplied device/document.

## Corrections from initial characterization

The user authorized fixing the identified defects after characterization:

- GUID selection previously allowed later elements to overwrite identity and mix
  spectra. Both disk lookup and device parsing now select one analyte. Disk lookup
  can also find the requested GUID after an unrelated first analyte in a file.
- One-decimal wavelength output previously collapsed distinct keys. Output now
  preserves even adjacent representable double wavelengths. Existing readers
  already accept these numeric strings; tests inspect decoded numeric values,
  not a forced textual spelling. Spectrum **values** and other scalar formatting
  have not changed.
- An empty-sequence corpus DNA file previously replaced stored mass 128396 with
  terminal-group mass 189.74. It now retains 128396. Populated-sequence calculation
  remains unchanged; an empty sequence without explicit mass remains zero.
- XML/device errors previously returned success with partial state. Reads now use
  a candidate copy and publish only on successful validation. Unknown fields,
  alternate wrappers, namespaces, CDATA and Unicode remain accepted; malformed
  documents are intentionally no longer treated as successful loads.

The parser consumes the complete document to check trailing syntax/read errors,
including errors after the selected element. A small internal read adapter records
actual negative device reads because the XML reader can treat these as EOF.

## Device and file ownership

`read_xml(QIODevice&, guid)` and `write_xml(QIODevice&)` borrow an already-open
synchronous device at its current position. They never open/close the supplied
device. Closed/wrong-mode devices return DBERROR. The filename wrapper owns its
QFile and checks flush errors after writing. Device writes may leave partial output;
file replacement is not atomic and flush is not a durability guarantee. No global
filesystem service, dependency-injection framework or timing-dependent fault test
is introduced.

## Corpus evidence and remaining gap

The project and `etc/` contain no standalone legacy analyte examples. Four HPC job
XMLs contain solution summaries, which are a different format. The user supplied
LIMS `scripts/test-corpus/` (generator/model inputs) and `test-data/` (artifacts).
Two pre-existing protein/DNA XML files were extracted unchanged after archive
checksum verification; [fixture provenance](../fixtures/analyte/README.md) records
hashes and generator stamps. They are embedded as immutable Qt resources, so tests
need no LIMS checkout, network or runtime generation.

A follow-up ZIP inventory found 205 saved-analyte entries / 42 distinct files:
38 PROTEIN and four DNA, all with empty sequences. No RNA, carbohydrate or
populated-sequence examples were found. These are generated artifacts, not a
historical lab corpus; the external generated directory is untracked, so its Git
revision alone does not reproduce it. The standard lab DNA archive contains raw
AUC and time-state XML, not saved analytes.

Still needed: approved historical saved A*.xml examples with provenance, especially
RNA/carbohydrate and populated sequences. Do not claim broad historical coverage
from hand-authored or generated fixtures alone.

## Verification

Reader tests use fixed XML; writer tests inspect structure/values through Qt XML,
not through US_Analyte. Tests cover both filename and device paths plus controlled
read errors, short writes, immediate write errors, ownership and current position.
Use `*AnalyteXml*` with the unit binary, or
`ctest -L gtest-unit-utils -R AnalyteXml --output-on-failure` in the build directory.
Linux/Qt5 validation passed: 947 DB / 946 NO_DB unit tests, including 49 focused
analyte cases per variant. Focused four-worker CTest and three shuffled repeats
passed with no skips or new process sandbox leaks. Qt6/native results are not
established by those runs.
