# CSV behavior coverage

`test_us_csv_data.cpp` tests the real `US_CSV_Data` API. There were no dedicated
CSV utility tests before this batch. No production code or seam was added.

Covered contracts:

- Rectangular in-memory columns, headers, file paths and invalid column indexes.
- Rejection of mismatched header counts, ragged columns and empty columns.
- Replacement/clear semantics, including recovery from a prior error.
- Explicit comma, semicolon, tab and space input; numeric first lines generate
  headers and remain data; scientific and signed numbers retain numeric values.
- CRLF, blank lines, trimmed whitespace and the accepted trailing separator.
- Tab auto-detection; missing, empty, one-line, ragged and nonnumeric inputs.
- Independent writer bytes/row order, truncation of previous output, missing
  arguments/data and deterministic open failure using a nonexistent parent.

Reader fixtures are checked fixed bytes; writer assertions inspect bytes directly
rather than calling the production reader. `Us3FileTestBase` supplies private paths
and settings restoration. There are no permissions-based faults, sleeps, live
services, application resources or external corpus dependencies.

Known gap: automatic comma/semicolon/space detection fails even for valid simple
numeric tables. A separate standalone probe reproduces this against production;
the production correction and its auto-detection regression are tracked in
ehb54/ultrascan-tickets#1238, separately from this coverage batch. Full quoted CSV
semantics, nonfinite-number policy and mid-stream writer error injection are not
claimed by these tests.

Run `*CsvData*` in the unit binary or `ctest -L gtest-unit-utils -R CsvData`.
