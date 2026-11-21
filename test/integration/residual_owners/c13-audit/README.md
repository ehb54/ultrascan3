# C13 production-reader probe

Production revision: `2ddf75d6e`. No production code is patched.

`run.py` recompiles the complete current `utils/us_dataIO.cpp` with Clang 18,
UBSan and `-ftrivial-auto-var-init=pattern`, then with `=zero`. It uses the
existing macOS baseline's compile-command configuration and static utils library
for supporting functions, with Qt 6.10.1. The script is intentionally tied to
the recorded local build paths; this is an audit artifact, not a portable CI test.

The driver supplies fully value-initialized input data, writes a real AUC file
with the production writer, reads it with the production reader, and copies the
returned raw scan's plateau object representation into an integer using memcpy.
It also writes a real edit XML and calls the production loadData path.

Results in the paired run:

- Pattern mode: raw plateau bits `ffffffffffffffff`.
- Zero mode: raw plateau bits `0000000000000000`.
- Both: raw readings `0.2,0.250001,0.3`; edited plateau `0.250001`, plateau radius
  `5.81`, floatingData false, and the same edited readings. Exit 0.

The raw plateau follows automatic-storage initialization rather than a reader
assignment. This confirms the missing initialization in readRawData; raw AUC
files do not encode an edited plateau. The audit also found copyRange omits
plateau before loadData appends the scan and subsequently assigns its plateau.
UBSan emits no diagnostic for these double representations, so this is **not**
a claimed UBSan uninitialized-memory report. Pattern/zero instrumentation and
byte inspection establish the missing write.

One earlier exploratory driver without the additional value prints returned
NaN for the edited plateau in pattern mode; changing the diagnostic prints made
the paired output finite. That observation has not been causally isolated and
must not be represented as proven report corruption or ignored as a passing
production-path check. Preserve it as a follow-up investigation in the separate
reader-initialization ticket. No report baseline is based on this synthetic probe.

Scope: raw reader/scan initialization predates the residual dependency changes.
The final edited plateau is explicitly assigned by loadData, and there is no
established residual/report regression caused by raw plateau. Track the reader
repair and the unexplained exploratory result separately from #628; do not add
a blanket constructor change without a regression and an explicit field policy.

Run from the repository root:

```sh
python3 test/integration/residual_owners/c13-audit/run.py
```

Generated objects/executables, compile commands and logs are under
`build/c13-audit/`. Copies of the paired result logs are stored here.
