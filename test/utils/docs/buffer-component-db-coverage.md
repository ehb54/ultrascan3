# Buffer-component DB caller coverage

`test_us_buffer_component_db.cpp` replaces three permissive component DB tests
with five strict real-caller cases. It uses the existing `IUS_DB2` seam and
`QtTestBase`; no production or fixture changes are needed.

- Exact read procedure and component ID, one successful cursor advance, and
  the six expected columns; unexpected DB calls fail the test.
- All twelve density/viscosity coefficients, including negative, zero and
  scientific notation values; deliberately distinct arrays detect swapped columns.
- Explicit range precedence over the legacy density suffix, fallback to that
  suffix, and replacement of stale state (including a true gradient flag).
- Write procedure, argument count, metadata order, both gradient encodings,
  successful inserted-ID publication and query/status/ID ordering.
- Failed write preserves the previous component ID and forbids insert-ID lookup.

The writer payload check deliberately stops short of asserting the current broken
coefficient serialization: the implementation inserts a space before coefficient
zero and none between subsequent coefficients. These findings are grouped in [#1241](https://github.com/ehb54/ultrascan-tickets/issues/1241).
This is a production defect, not
an accepted payload contract. Full six-token numeric assertions belong with its
separate fix.

Inspection also found `getInfoFromDB` ignores cursor/query failure and indexes
six tokens without validating their count. Empty/error/malformed-row regression
contracts remain pending a separately selected fix; this suite does not bless
those unsafe paths. `getAllFromDB` creates its own live connection and is excluded
from offline unit execution.

Validation uses both DB/NO_DB builds, focused CTest parallel execution and three
shuffled repeats. The mock checks interactions and returned-value mapping; it
cannot establish stored-procedure compatibility or real database behavior.
