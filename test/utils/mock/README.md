# Utility database mocks

Use `IUS_DB2` and GoogleMock to test a production caller. `MockBuilder` is the
single implementation of connected success defaults. Defaults are not
expectations or a database emulator.
Successful connection/transaction defaults clear reference error outputs.

For each behavior, specify the query arguments and expected call count. Script
finite rows (`true` per row, then `false`) and the columns consumed. Set status
and error text together for failures, and forbid subsequent work when appropriate.
Use `InSequence` where order is part of the caller's contract. Prefer strict mocks
for small protocols; use NiceMock only when irrelevant calls are intentionally
outside the assertion. Do not share a mock between cases or configure it while
workers are running.

Examples to extend:

- `test_us_buffer.cpp`: exact spectrum query, finite multiple/empty rows and columns.
- `test_us_convertio_editeddata.cpp`: missing rows versus query failure, exact lookup
  arguments, and error propagation through the real caller.
- `test_us_convertio_editeddata_read.cpp`: blob copies in owned QTemporaryDirs,
  download failures, and no publication of partial output.
- `test_us_convertio_writeraw.cpp`: existing per-procedure scripted DB and cleanup
  after write failures. Error state follows the most recent operation.

The utility writer does not own transactions; its cleanup is deliberately tested
for nontransactional callers. No utils caller of begin/commit/rollback was found
outside US_DB2 itself. Do not invent rollback expectations here. Actual DB
transaction behavior remains in the separately selected live-DB tests.
