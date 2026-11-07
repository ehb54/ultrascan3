#!/usr/bin/env python3
"""Time us_mpi_analysis with and without the node-shared simulation cache.

Builds a 2DSA job on the regression test data with a larger grid, runs it
once with debug text "SimCache=off" and once with the cache, and prints the
wall times, the speedup, the fit RMSDs (which must agree) and the cache
reuse each node reports.

Example:
  python3 us_mpi_benchmark.py --program build/bin/us_mpi_analysis \\
      --testdata build/bin/us_mpi_testdata --np 8 --grid 64 --refine 5
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import us_mpi_regression as R  # noqa: E402


def run(name, case, args, debug_text):
    casedir = Path(args.workdir) / name
    if casedir.exists():
        shutil.rmtree(casedir)
    casedir.mkdir(parents=True)
    case = dict(case, params=dict(case["params"], debug_text=debug_text))
    archive = R.build_archive(name, case, args.testdata, casedir)

    env = dict(os.environ)
    env.setdefault("OMPI_ALLOW_RUN_AS_ROOT", "1")
    env.setdefault("OMPI_ALLOW_RUN_AS_ROOT_CONFIRM", "1")
    env.setdefault("OMPI_MCA_rmaps_base_oversubscribe", "1")
    env["QT_LOGGING_RULES"] = "*.debug=true"   # for the cache report

    cmd = R.mpirun_command(args.mpirun, args.np) + [
        args.program, "-walltime", "600", "-mgroupcount", str(args.mgroupcount),
        archive.name]
    log = casedir / "run.log"
    start = time.time()
    with open(log, "w") as out:
        proc = subprocess.run(cmd, cwd=casedir, env=env, stdout=out,
                              stderr=subprocess.STDOUT)
    seconds = time.time() - start
    results = casedir / "output" / "analysis-results.tar"
    if proc.returncode != 0 or not results.exists():
        sys.exit("%s: us_mpi_analysis failed, see %s" % (name, log))

    summary = R.summarize(results)
    rmsds = [m["rmsd"] for ms in summary["models"].values() for m in ms]
    report = [line.strip() for line in log.read_text(errors="replace").splitlines()
              if re.match(r"\s*Simulation cache \(", line)]
    return seconds, rmsds, report


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--program", required=True)
    parser.add_argument("--testdata", required=True)
    parser.add_argument("--mpirun", default="mpirun")
    parser.add_argument("--workdir", default="mpi_benchmark_work")
    parser.add_argument("--np", type=int, default=4, help="MPI ranks")
    parser.add_argument("--mgroupcount", type=int, default=1)
    parser.add_argument("--grid", type=int, default=64,
                        help="s and f/f0 grid points (grid x grid solutes)")
    parser.add_argument("--subgrids", type=int, default=8,
                        help="uniform_grid: subgrids per dimension")
    parser.add_argument("--refine", type=int, default=5,
                        help="refinement iterations")
    parser.add_argument("--mc", type=int, default=1, help="Monte Carlo iterations")
    args = parser.parse_args()

    args.program = str(Path(args.program).resolve())
    args.testdata = str(Path(args.testdata).resolve())
    args.workdir = str(Path(args.workdir).resolve())

    base = R.CASES["2dsa_basic"]
    params = dict(base["params"], s_grid_points=str(args.grid),
                  ff0_grid_points=str(args.grid), uniform_grid=str(args.subgrids),
                  max_iterations=str(args.refine), mc_iterations=str(args.mc))
    case = dict(base, np=args.np, params=params)

    print("2DSA %d x %d grid, %d refinement iterations, %d Monte Carlo"
          " iterations, %d ranks, %d master groups"
          % (args.grid, args.grid, args.refine, args.mc, args.np, args.mgroupcount))

    t_off, rms_off, _ = run("cache_off", case, args, "SimCache=off")
    t_on, rms_on, report = run("cache_on", case, args, "")

    print("without cache: %8.1f s" % t_off)
    print("with cache:    %8.1f s   (speedup %.2f)" % (t_on, t_off / t_on))
    for line in report:
        print("   " + line)
    same = len(rms_off) == len(rms_on) and all(
        abs(a - b) <= 1e-9 * max(abs(a), 1e-30) for a, b in zip(rms_off, rms_on))
    print("RMSDs %s: %s" % ("identical" if same else "DIFFER",
                            " ".join("%.7f" % r for r in rms_on)))
    return 0 if same else 1


if __name__ == "__main__":
    sys.exit(main())
