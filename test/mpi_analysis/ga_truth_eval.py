#!/usr/bin/env python3
"""Known-truth evaluation of the us_mpi_analysis genetic algorithm (GA).

Each case simulates a sample of known composition with us_mpi_testdata,
fits it with the GA in us_mpi_analysis for several random seeds and scores
the fitted model against the truth.  Several us_mpi_analysis builds can be
compared in one call (for example the code before and after a change).

Unlike us_mpi_regression.py this is not a pass/fail test:  the GA is
stochastic, so the result is a table of statistics over the seeds.

Per run the score holds
  rmsd/sigma   fit RMSD relative to the simulated noise (1.0 is ideal)
  n_sig        solutes with at least 1 % of the total signal
  decoy        fraction of the signal in buckets that hold no true species
  ds, dk, dc   relative errors of s, f/f0 and concentration of each true
               species (signal-weighted over its bucket), and dM for the
               molar mass derived from s and f/f0
  ok           all species within |ds| <= 2 %, |dk| <= 5 %, |dc| <= 5 %
               and decoy <= 2 %
For Monte Carlo cases the spread of the iterations is reported as well.

Usage:
  ga_truth_eval.py --variant NAME=path/to/us_mpi_analysis [...]
                   --testdata path/to/us_mpi_testdata
                   [--cases a,b] [--seeds 1,2,3] [--workdir DIR] [--out FILE]
"""

import argparse
import json
import math
import statistics
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import us_mpi_regression as reg  # noqa: E402

# Default sample of us_mpi_testdata (s in S at 20,W, f/f0, concentration)
BASE3 = [(2.5, 1.25, 0.25), (4.5, 1.6, 0.40), (7.0, 2.1, 0.30)]
BASE3_BUCKETS = [(1.5, 3.5, 1.0, 1.6), (3.5, 5.5, 1.3, 2.0),
                 (5.5, 8.5, 1.7, 2.6)]
# Buckets that hold no species, placed next to the true ones
DECOYS = [(1.5, 3.5, 1.7, 3.0), (3.5, 5.5, 2.1, 3.0), (8.6, 11.0, 1.0, 3.0)]

GA_PARAMS = {
    "population": "60", "generations": "50", "crossover": "50",
    "mutation": "50", "plague": "4", "elitism": "2", "migration": "3",
    "regularization": "0", "conc_threshold": "0.000001",
    "s_grid": "100", "k_grid": "100", "mutate_sigma": "2",
    "p_mutate_s": "20", "p_mutate_k": "20", "p_mutate_sk": "20",
    "minimize_opt": "0", "mc_iterations": "1",
    "tinoise_option": "0", "rinoise_option": "0", "debug_level": "0",
}

CASES = {
    # The regression sample in tight buckets
    "base3": {"species": BASE3, "buckets": BASE3_BUCKETS},
    # The same with three empty buckets and the parsimony penalty
    "decoys": {"species": BASE3, "buckets": BASE3_BUCKETS + DECOYS,
               "params": {"regularization": "0.05"}},
    # The same at 2.5 times the noise
    "decoys_noisy": {"species": BASE3, "buckets": BASE3_BUCKETS + DECOYS,
                     "noise": 0.01, "params": {"regularization": "0.05"}},
    # Two species with the same s, different shape
    "same_s": {"species": [(4.0, 1.25, 0.4), (4.0, 2.0, 0.4),
                           (7.0, 1.6, 0.2)],
               "buckets": [(3.0, 5.0, 1.0, 1.6), (3.0, 5.0, 1.6, 2.6),
                           (6.0, 8.0, 1.2, 2.0)]},
    # Five species over a wide s range
    "five": {"species": [(2.0, 1.2, 0.2), (3.5, 1.4, 0.2), (5.0, 1.6, 0.2),
                         (7.0, 1.9, 0.2), (9.5, 2.3, 0.2)],
             "buckets": [(1.4, 2.7, 1.0, 1.6), (2.9, 4.2, 1.1, 1.8),
                         (4.3, 5.8, 1.2, 2.0), (6.1, 8.0, 1.4, 2.4),
                         (8.3, 11.0, 1.7, 3.0)]},
    # The regression sample in wide buckets (coarser s,k grid)
    "wide": {"species": BASE3,
             "buckets": [(1.0, 3.8, 1.0, 2.5), (3.8, 5.8, 1.0, 3.0),
                         (5.8, 10.0, 1.0, 3.5)]},
    # Monte Carlo:  spread of the iterations
    "mc": {"species": BASE3, "buckets": BASE3_BUCKETS,
           "params": {"mc_iterations": "6"}},
}

NOISE = 0.004             # us_mpi_testdata default sigma
SIG_FRACTION = 0.01       # n_sig threshold (fraction of total signal)
OK_LIMITS = {"ds": 0.02, "dk": 0.05, "dc": 0.05, "decoy": 0.02}

# Molar mass from s and f/f0 at 20,W (cgs units)
AVOGADRO = 6.02214076e23
ETA20W = 0.01002
RHO20W = 0.998234
VBAR = 0.72


def molar_mass(s_sv, ff0):
    s = s_sv * 1.0e-13
    m23 = (s * AVOGADRO * 6.0 * math.pi * ETA20W * ff0
           * (3.0 * VBAR / (4.0 * math.pi * AVOGADRO)) ** (1.0 / 3.0)
           / (1.0 - VBAR * RHO20W))
    return m23 ** 1.5


def regression_case(case, seed):
    opts = {"species": case["species"]}
    if "noise" in case:
        opts["noise"] = case["noise"]
    params = dict(GA_PARAMS, seed=str(seed), **case.get("params", {}))
    return {"method": "GA", "np": 4, "params": params,
            "datasets": [("regA", 1, 11, 1.0, opts)],
            "buckets": case["buckets"]}


def in_bucket(sol, bucket):
    s_sv, ff0 = sol[0] * 1.0e13, sol[1]
    xmin, xmax, ymin, ymax = bucket
    eps = 1.0e-6
    return (xmin - eps <= s_sv <= xmax + eps
            and ymin - eps <= ff0 <= ymax + eps)


def score_model(model, case):
    """Score one fitted model against the truth of the case."""
    sigma = case.get("noise", NOISE)
    sols = model["solutes"]              # [s, f/f0, vbar, signal]
    total = sum(sol[3] for sol in sols) or 1.0
    truth = case["species"]
    # The bucket of each true species (the one containing it)
    true_bucket = []
    for (s_sv, ff0, _c) in truth:
        hits = [ii for ii, bb in enumerate(case["buckets"])
                if in_bucket((s_sv * 1.0e-13, ff0), bb)]
        true_bucket.append(hits[0] if hits else None)

    species = []
    for (s_sv, ff0, conc), bidx in zip(truth, true_bucket):
        members = [sol for sol in sols
                   if bidx is not None and in_bucket(sol, case["buckets"][bidx])]
        csum = sum(sol[3] for sol in members)
        if csum > 0:
            s_fit = sum(sol[0] * sol[3] for sol in members) / csum * 1.0e13
            k_fit = sum(sol[1] * sol[3] for sol in members) / csum
            species.append({
                "ds": s_fit / s_sv - 1.0, "dk": k_fit / ff0 - 1.0,
                "dc": csum / conc - 1.0,
                "dM": molar_mass(s_fit, k_fit) / molar_mass(s_sv, ff0) - 1.0,
                "s": s_fit, "k": k_fit, "c": csum})
        else:
            species.append({"ds": None, "dk": None, "dc": -1.0, "dM": None,
                            "s": None, "k": None, "c": 0.0})

    used = set(b for b in true_bucket if b is not None)
    decoy = sum(sol[3] for sol in sols
                if not any(in_bucket(sol, case["buckets"][b]) for b in used))
    found = all(sp["ds"] is not None for sp in species)
    worst = {key: (max(abs(sp[key]) for sp in species) if found else None)
             for key in ("ds", "dk", "dc", "dM")}
    ok = (found and worst["ds"] <= OK_LIMITS["ds"]
          and worst["dk"] <= OK_LIMITS["dk"]
          and worst["dc"] <= OK_LIMITS["dc"]
          and decoy / total <= OK_LIMITS["decoy"])
    return {
        "rmsd": model["rmsd"], "rmsd_sigma": model["rmsd"] / sigma,
        "n_sig": sum(1 for sol in sols if sol[3] >= SIG_FRACTION * total),
        "decoy": decoy / total, "worst": worst, "ok": ok,
        "species": species,
    }


def run_one(variant, program, name, case, seed, args):
    rargs = argparse.Namespace(
        program=program, testdata=args.testdata, mpirun=args.mpirun,
        workdir=str(Path(args.workdir) / variant / ("seed%d" % seed)),
        timeout=args.timeout)
    t0 = time.time()
    summary = reg.run_case(name, regression_case(case, seed), rargs)
    wall = time.time() - t0
    # One model file; an MC run holds one model per iteration
    (models,) = summary["models"].values()
    scores = [score_model(m, case) for m in models]
    return {"variant": variant, "case": name, "seed": seed, "wall": wall,
            "scores": scores}


def mean_sd(values):
    values = [v for v in values if v is not None]
    if not values:
        return "n/a"
    if len(values) == 1:
        return "%.4g" % values[0]
    return "%.4g ± %.2g" % (statistics.mean(values), statistics.stdev(values))


def pct(values):
    values = [v for v in values if v is not None]
    if not values:
        return "n/a"
    if len(values) == 1:
        return "%.2f" % (100.0 * values[0])
    return "%.2f ± %.2f" % (100.0 * statistics.mean(values),
                            100.0 * statistics.stdev(values))


def report(results, variants):
    lines = []
    for name in dict.fromkeys(r["case"] for r in results):
        case = CASES[name]
        lines.append("\n### %s\n" % name)
        lines.append("Truth (s, f/f0, c): %s; %d buckets; noise %g; params %s\n"
                     % ("; ".join("%g, %g, %g" % sp for sp in case["species"]),
                        len(case["buckets"]), case.get("noise", NOISE),
                        case.get("params", {})))
        mc = int(case.get("params", {}).get("mc_iterations", "1")) > 1
        if not mc:
            lines.append("| variant | runs | rmsd/σ | n_sig | decoy % | "
                         "max\\|ds\\| % | max\\|dk\\| % | max\\|dc\\| % | "
                         "max\\|dM\\| % | ok | wall s |")
            lines.append("|---|---|---|---|---|---|---|---|---|---|---|")
        else:
            lines.append("| variant | runs | rmsd/σ (iter. 1) | species | "
                         "mean ds % | sd s % | mean dk % | sd k % | "
                         "mean dc % | sd c % |")
            lines.append("|---|---|---|---|---|---|---|---|---|---|")
        for variant in variants:
            runs = [r for r in results
                    if r["case"] == name and r["variant"] == variant]
            if not runs:
                continue
            if not mc:
                sc = [r["scores"][-1] for r in runs]
                lines.append("| %s | %d | %s | %s | %s | %s | %s | %s | %s | "
                             "%d/%d | %.0f |" % (
                                 variant, len(runs),
                                 mean_sd([s["rmsd_sigma"] for s in sc]),
                                 mean_sd([s["n_sig"] for s in sc]),
                                 pct([s["decoy"] for s in sc]),
                                 pct([s["worst"]["ds"] for s in sc]),
                                 pct([s["worst"]["dk"] for s in sc]),
                                 pct([s["worst"]["dc"] for s in sc]),
                                 pct([s["worst"]["dM"] for s in sc]),
                                 sum(s["ok"] for s in sc), len(sc),
                                 statistics.mean(r["wall"] for r in runs)))
                continue
            for ii in range(len(case["species"])):
                cols = []
                for key, val in (("ds", "s"), ("dk", "k"), ("dc", "c")):
                    means, sds = [], []
                    for r in runs:
                        vals = [s["species"][ii][val] for s in r["scores"]
                                if s["species"][ii][val] is not None]
                        errs = [s["species"][ii][key] for s in r["scores"]
                                if s["species"][ii][key] is not None]
                        if len(vals) > 1:
                            means.append(statistics.mean(errs))
                            sds.append(statistics.stdev(vals)
                                       / statistics.mean(vals))
                    cols += [pct(means), pct(sds)]
                lines.append("| %s | %d | %s | %d | %s |" % (
                    variant, len(runs),
                    mean_sd([r["scores"][0]["rmsd_sigma"] for r in runs]),
                    ii + 1, " | ".join(cols)))
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--variant", action="append", required=True,
                        help="NAME=us_mpi_analysis path (repeatable)")
    parser.add_argument("--testdata", required=True)
    parser.add_argument("--cases", default=",".join(CASES))
    parser.add_argument("--seeds", default="1,2,3,4,5")
    parser.add_argument("--mc-seeds", default="1,2,3",
                        help="seeds for Monte Carlo cases")
    parser.add_argument("--mpirun", default="mpirun")
    parser.add_argument("--workdir", default="ga_truth_work")
    parser.add_argument("--timeout", type=int, default=1800)
    parser.add_argument("--out", default="ga_truth_results.json")
    parser.add_argument("--report", help="re-print the report of --out")
    args = parser.parse_args()

    variants = dict(v.split("=", 1) for v in args.variant)
    if args.report:
        results = json.loads(Path(args.report).read_text())
        print(report(results, list(variants)))
        return 0

    args.testdata = str(Path(args.testdata).resolve())
    args.workdir = str(Path(args.workdir).resolve())
    out = Path(args.out)
    results = json.loads(out.read_text()) if out.exists() else []
    done = {(r["variant"], r["case"], r["seed"]) for r in results}

    for name in args.cases.split(","):
        case = CASES[name]
        mc = int(case.get("params", {}).get("mc_iterations", "1")) > 1
        seeds = [int(s) for s in (args.mc_seeds if mc else args.seeds).split(",")]
        for seed in seeds:
            for variant, program in variants.items():
                if (variant, name, seed) in done:
                    continue
                program = str(Path(program).resolve())
                try:
                    res = run_one(variant, program, name, case, seed, args)
                except Exception as exc:  # keep the other runs
                    print("%s %s seed %d: FAILED %s" % (variant, name, seed, exc),
                          flush=True)
                    continue
                results.append(res)
                out.write_text(json.dumps(results, indent=1))
                last = res["scores"][-1]
                print("%s %s seed %d: rmsd/sigma %.4f n_sig %d decoy %.4f "
                      "ok %s (%.0f s)" % (variant, name, seed,
                                          last["rmsd_sigma"], last["n_sig"],
                                          last["decoy"], last["ok"],
                                          res["wall"]), flush=True)

    print(report(results, list(variants)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
