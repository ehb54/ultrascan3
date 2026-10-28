# `grpy/` — shell reduction, and the boundary to the GRPY program

What SOMO keeps of the GRPY work. The solver itself is **not here**: it is a separate
GPLv3 program, [ehb54/grpy-cpp](https://github.com/ehb54/grpy-cpp), which SOMO runs rather
than links. See ehb54/ultrascan-tickets#1012 for why.

| file | what it is |
|---|---|
| `grpy_types.hpp` | the value types exchanged with the solver, and `SolveFn`, the injection point |
| `grpy_exposure.hpp` | per-bead solvent exposure (Shrake–Rupley) and the exposure ranking |
| `grpy_shell.hpp` | the self-validating shell reduction: a ladder of increasing bead counts that reports its own error bar |
| `grpy_process.hpp` | `ProcessSolver` — runs the GRPY program and parses its report, presented as a `SolveFn` |

All original work, all header-only. No Eigen, no QtConcurrent: the solver that needed them
is the part that moved out.

## How it fits together

```cpp
grpy::Input in = grpy::read_grpy_input( grpy_file );      // the .grpy SOMO wrote

grpy::ProcessSolver::Config cfg;
cfg.program    = ...;                                     // bin/GRPY_<platform>
cfg.full_input = grpy_file;                               // an unreduced rung reuses it
cfg.full_beads = (int) in.beads.size();
grpy::ProcessSolver psolver( cfg, [ & ]{ return stopFlag; } );

grpy::ShellOptions sopt;                                  // enabled = false => one solve
grpy::ShellReport  srep;
grpy::ShellSolver  solver( psolver.fn(), sopt );
grpy::Results r = solver.run( in.beads, in.params, srep, progress );
```

With shell reduction off — the default — the ladder is a pass-through: one solve, on the
`.grpy` file the user already has, with the report parsed exactly as it always was.

## The contract with the program

Unchanged from what SOMO used before the in-process port, which is what makes the program
a drop-in for the Fortran GRPY it replaces:

- run it as `GRPY -e <file>`, working directory the SOMO directory;
- the report arrives on stdout, and is what SOMO parses and writes to `.grpy_res`;
- progress arrives interleaved as carriage-return-separated `NN% TASK:` records;
- a non-zero exit is a failure, and its stderr is the message.

Everything added since the Fortran is an environment variable — `GRPY_SINGLE`, `GRPY_OOC`,
`GRPY_THREADS`, `GRPY_HP` — so the command line stays the published one.

## Tests

```
./tests/run.sh            # add QTDIR (or source qt5env) to include the process test
```

`test_shell` drives the ladder with an analytic model whose exact answer is known, so the
reported error bar can be checked for actually bounding the true error. `test_process`
drives `ProcessSolver` with a fake GRPY program that replays a real golden report and the
real progress banner, covering parsing, progress, a non-zero exit, a missing program and a
mid-solve stop.

The solver's own numerical validation — golden comparison against the original Fortran —
lives with the solver, in `grpy-cpp`.

## Note on precision

The ladder differences successive rungs, and the report carries four significant figures.
For tolerances near 0.5% that quantization is small but has not been measured against the
error estimator; `GRPY_HP=1` makes the program report in extended precision if it turns out
to matter. This does not affect a run with shell reduction off, which does no differencing.
