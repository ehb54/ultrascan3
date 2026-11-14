# Extinction-profile XML behavior coverage

`test_us_eprofile_xml.cpp` exercises the existing `US_ExtProfile` string API
without a DB, filesystem access, or a new production seam. It uses `QtTestBase`
for the suite's standard state isolation.

## Covered behavior

Ten cases check fixed reader inputs and independent writer expectations:

- Profile metadata, replacement of an existing spectrum, and empty profiles.
- Analyte XML's `frequency` alias and preservation of caller-supplied metadata.
- Frequency precedence, the missing-wavelength default of 280, duplicate-key
  last-value behavior, and scientific numeric notation.
- Independently optional metadata pointers, escaped text, namespaces and unknown
  wrappers; unrecognized well-formed documents return false.
- Writer document type, envelope/version, metadata escaping, sorted wavelength
  attributes, numeric values, unchanged inputs and empty-profile output.

Reader fixtures are fixed XML strings. Writer assertions use `QXmlStreamReader`,
not the production reader as an oracle. These synthetic examples characterize
current accepted forms; they do not establish compatibility with historical files.

## Separate production defects

Exploratory probes exposed related issues in `utils/us_eprofile.cpp`:

- A truncated document containing a profile marker returns success with partial
  spectrum data because parsing does not check the XML error state.
- Default numeric formatting reduces precision: wavelengths 280.00001 and
  280.00002 both become 280, losing one point when subsequently read.
- A nonempty output string retains its previous contents before the generated
  document, yielding invalid XML.

These failures are grouped in [#1239](https://github.com/ehb54/ultrascan-tickets/issues/1239),
linked from the Phase 1 enhancement (#1228).
No production fix or passing assertion that requires these defects is included.
A future fix should add explicit regressions for malformed input and output
publication, distinct close wavelengths/full-precision values, and reused output
strings. Reader failure-state policy should be decided explicitly before changing
it; permissive legacy element handling need not be tightened.

## Validation

Run `--gtest_filter='*ExtProfileXml*'` in both DB and NO_DB offline unit binaries.
Use CTest's `gtest-unit-utils` label with `-R ExtProfileXml -j4` for isolated
parallel execution and three shuffled repeats for a bounded ordering check.
Full offline unit suites cover interaction with the shared fixtures. Validation
logs and local planning notes are not committed.
