#!/usr/bin/env python3
"""Regression tests for us_mpi_analysis.

Each case builds an HPC job archive (synthetic data written by
us_mpi_testdata plus an hpcrequest XML in the format the LIMS writes), runs
us_mpi_analysis under mpirun and reduces the result archive to a JSON summary
of every fitted model (RMSD and solute distribution) and noise vector.  The
summary is compared with the stored baseline in baseline/<case>.json.

Commands:
  check  <case>...   run cases and compare with the baselines
  update <case>...   run cases and (re)write the baselines
  list               print the case names

A case with a "baseline" entry has no baseline of its own:  it must give
the result of the named case (the same fit with another layout or input),
and "update" checks it instead of writing a file.

Use "all" as the case name to select every case.
"""

import argparse
import json
import math
import os
import re
import shutil
import subprocess
import sys
import tarfile
import xml.etree.ElementTree as ET
from pathlib import Path

HERE = Path(__file__).resolve().parent
BASELINE_DIR = HERE / "baseline"

# Dataset descriptors: (runID, cell, noise seed, concentration scale) and
# optionally a dict of us_mpi_testdata options ("scans": scan count,
# "nobottom": edit without <bottom>).  The experiment constants live in
# us_mpi_testdata.cpp.
DS_A = ("regA", 1, 11, 1.0)
DS_B = ("regA", 2, 23, 0.6)
# 31 x 185 points:  an odd number of Monte Carlo deviates per iteration
DS_ODD = ("regA", 1, 11, 1.0, {"scans": 31})
DS_NOBOTTOM = ("regA", 1, 11, 1.0, {"nobottom": True})

GRID_2DSA = {
    "s_min": "1.0", "s_max": "10.0",
    "ff0_min": "1.0", "ff0_max": "3.0",
    "s_grid_points": "36", "ff0_grid_points": "12",
    "uniform_grid": "3",
    "max_iterations": "1", "mc_iterations": "1",
    "tinoise_option": "0", "rinoise_option": "0",
    "meniscus_points": "1", "meniscus_range": "0.0", "fit_mb_select": "0",
    "debug_level": "0", "seed": "4711",
}

# GA buckets bracket the three simulated species (s in S, f/f0).
GA_BUCKETS = [(1.5, 3.5, 1.0, 1.6), (3.5, 5.5, 1.3, 2.0), (5.5, 8.5, 1.7, 2.6)]

CASES = {
    "2dsa_basic": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA),
    },
    "2dsa_refine": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, max_iterations="3"),
    },
    "2dsa_mc": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, mc_iterations="3"),
    },
    "2dsa_mc_pmasters": {
        "method": "2DSA", "datasets": [DS_A], "np": 6, "mgroupcount": 2,
        "params": dict(GRID_2DSA, mc_iterations="4"),
    },
    "2dsa_mc_pmasters3": {
        # Iterations 2 and 3 are the last ones of their groups
        "method": "2DSA", "datasets": [DS_A], "np": 9, "mgroupcount": 3,
        "params": dict(GRID_2DSA, mc_iterations="4"),
    },
    "2dsa_mc_pmasters_idle": {
        # More master groups than Monte Carlo iterations
        "method": "2DSA", "datasets": [DS_A], "np": 9, "mgroupcount": 3,
        "params": dict(GRID_2DSA, mc_iterations="2"),
    },
    "2dsa_mc_pmasters_uneven": {
        # 10 ranks do not split into 3 groups:  runs as 2 groups of 5
        "method": "2DSA", "datasets": [DS_A], "np": 10, "mgroupcount": 3,
        "params": dict(GRID_2DSA, mc_iterations="3"), "baseline": "2dsa_mc",
    },
    "2dsa_mc_odd": {
        "method": "2DSA", "datasets": [DS_ODD], "np": 2,
        "params": dict(GRID_2DSA, mc_iterations="3"),
    },
    "2dsa_mc_odd_pmasters": {
        # Each iteration's data must not depend on the deviates the group
        #  master drew before
        "method": "2DSA", "datasets": [DS_ODD], "np": 6, "mgroupcount": 2,
        "params": dict(GRID_2DSA, mc_iterations="3"),
        "baseline": "2dsa_mc_odd",
    },
    "2dsa_meniscus": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, meniscus_points="3", meniscus_range="0.04",
                       fit_mb_select="1"),
    },
    "2dsa_bottom": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, meniscus_points="3", meniscus_range="0.04",
                       fit_mb_select="2"),
    },
    "2dsa_meniscus_nobottom": {
        # Edit without <bottom>:  the stretched centerpiece bottom is used
        "method": "2DSA", "datasets": [DS_NOBOTTOM], "np": 2,
        "params": dict(GRID_2DSA, meniscus_points="3", meniscus_range="0.04",
                       fit_mb_select="1"),
        "baseline": "2dsa_meniscus",
    },
    "2dsa_menbot": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, meniscus_points="3", meniscus_range="0.04",
                       fit_mb_select="3"),
    },
    "2dsa_composite_menbot": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA, meniscus_points="2", meniscus_range="0.04",
                       fit_mb_select="3"),
    },
    "2dsa_composite_menbot_pmasters": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 6, "mgroupcount": 2,
        "params": dict(GRID_2DSA, meniscus_points="2", meniscus_range="0.04",
                       fit_mb_select="3"),
    },
    "2dsa_noise": {
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, tinoise_option="1", rinoise_option="1"),
    },
    "2dsa_customgrid": {
        "method": "2DSA-CG", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA, CG_model="regression.cgrid.model.xml"),
        "cgrid": "regression.cgrid.model.xml",
    },
    "2dsa_global": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA), "global_fit": True,
    },
    "2dsa_global_mc": {
        # Monte Carlo residuals span all datasets after the global write
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA, mc_iterations="3"), "global_fit": True,
    },
    "2dsa_composite": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA),
    },
    "2dsa_composite_pmasters": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 6, "mgroupcount": 2,
        "params": dict(GRID_2DSA),
    },
    "2dsa_composite_pmasters_np4": {
        # 4 ranks leave no worker for group 0 with 2 groups:  one master
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 4, "mgroupcount": 3,
        "params": dict(GRID_2DSA), "baseline": "2dsa_composite",
    },
    "2dsa_composite_mc": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA, mc_iterations="3"),
    },
    "2dsa_composite_mc_pmasters": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 6, "mgroupcount": 2,
        "params": dict(GRID_2DSA, mc_iterations="3"),
        "baseline": "2dsa_composite_mc",
    },
    "2dsa_composite_meniscus": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA, meniscus_points="3", meniscus_range="0.04",
                       fit_mb_select="1"),
    },
    "2dsa_composite_meniscus_pmasters": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 6, "mgroupcount": 2,
        "params": dict(GRID_2DSA, meniscus_points="3", meniscus_range="0.04",
                       fit_mb_select="1"),
    },
    "2dsa_refine_np4": {
        "method": "2DSA", "datasets": [DS_A], "np": 4,
        "params": dict(GRID_2DSA, max_iterations="3"),
    },
    "2dsa_mc_np4": {
        "method": "2DSA", "datasets": [DS_A], "np": 4,
        "params": dict(GRID_2DSA, mc_iterations="3"),
    },
    "pcsa_sl": {
        "method": "PCSA", "datasets": [DS_A], "np": 2,
        "params": {
            "curve_type": "SL", "solute_type": "013.skv",
            "x_min": "1.0", "x_max": "10.0", "y_min": "1.0", "y_max": "3.0",
            "z_value": "0.0", "vars_count": "6", "curves_points": "10",
            "gfit_iterations": "1", "thr_deltr_ratio": "0.0001",
            "max_iterations": "1", "mc_iterations": "1",
            "tikreg_option": "0", "tikreg_alpha": "0.0",
            "tinoise_option": "0", "rinoise_option": "0",
            "debug_level": "0", "seed": "4711",
        },
    },
    "ga_basic": {
        "method": "GA", "datasets": [DS_A], "np": 4,
        "params": {
            "population": "30", "generations": "15", "crossover": "50",
            "mutation": "50", "plague": "4", "elitism": "2",
            "migration": "3", "regularization": "5",
            "conc_threshold": "0.000001", "s_grid": "100", "k_grid": "100",
            "mutate_sigma": "2", "p_mutate_s": "20", "p_mutate_k": "20",
            "p_mutate_sk": "20", "minimize_opt": "0",
            "mc_iterations": "1", "tinoise_option": "0", "rinoise_option": "0",
            "debug_level": "0", "seed": "4711",
        },
        "buckets": GA_BUCKETS,
    },
}

# Default comparison tolerances.  RMSD differences are relative; solute
# distribution moments are compared relative to their scale.
DEFAULT_TOL = {"rmsd_rel": 1e-6, "moment_rel": 1e-4, "noise_abs": 1e-6}

# GA migrates genes asynchronously, so its result depends on message timing.
# Only require a fit that is not markedly worse.
LOOSE = {"rmsd_rel": 0.15, "moment_rel": 0.0, "noise_abs": 0.0, "loose": True}
CASE_TOL = {
    "ga_basic": LOOSE,
}


# ---------------------------------------------------------------------------
# Job archive
# ---------------------------------------------------------------------------

def sub(parent, tag, **attrs):
    return ET.SubElement(parent, tag, {k: str(v) for k, v in attrs.items()})


def job_xml(name, case):
    root = ET.Element("US_JobSubmit", method=case["method"], version="1.0")
    job = sub(root, "job")
    sub(job, "cluster", name="local", shortname="local")
    sub(job, "name", value="us3_regression")
    sub(job, "udp", server="127.0.0.1", port="12233")
    sub(job, "global_fit", value="1" if case.get("global_fit") else "0")
    sub(job, "request", id="1", guid="00000000-0000-4000-8000-000000000001")
    params = sub(job, "jobParameters")
    for key, value in case["params"].items():
        if key == "CG_model":
            sub(params, key, filename=value)
        else:
            sub(params, key, value=value)
    for (xmin, xmax, ymin, ymax) in case.get("buckets", []):
        sub(params, "bucket", s_min=xmin, s_max=xmax, ff0_min=ymin, ff0_max=ymax)

    for (run_id, cell, _seed, scale, *_opts) in case["datasets"]:
        triple = "RI.%d.A.260" % cell
        dset = sub(root, "dataset")
        files = sub(dset, "files")
        sub(files, "auc", filename="%s.%s.auc" % (run_id, triple))
        sub(files, "edit", filename="%s.e1.%s.xml" % (run_id, triple))
        solution = sub(dset, "solution")
        sub(solution, "buffer", density="0.998234", viscosity="1.00194",
            compress="0", manual="0")
        sub(solution, "analyte", mw="50000", vbar20="0.72", amount="1",
            type="Protein")
        # Without band_volume the default (0.015) makes a band-forming run
        sub(dset, "band_volume", value="0.0")
        sub(dset, "rotor_stretch", value="0 1e-12")
        sub(dset, "centerpiece_bottom", value="7.2")
        sub(dset, "centerpiece_shape", value="standard")
        sub(dset, "centerpiece_angle", value="2.5")
        sub(dset, "centerpiece_pathlength", value="1.2")
        sub(dset, "centerpiece_width", value="0")
        sub(dset, "total_concentration", value="%.4f" % (0.95 * scale))

    if hasattr(ET, "indent"):  # Python 3.9+
        ET.indent(root)
    return ET.tostring(root, encoding="unicode")


def build_archive(name, case, testdata, casedir):
    inputs = casedir / "inputs"
    inputs.mkdir(parents=True)
    for (run_id, cell, seed, scale, *opts) in case["datasets"]:
        opts = opts[0] if opts else {}
        extra = ["scans=%d" % opts["scans"]] if "scans" in opts else []
        extra += ["nobottom"] if opts.get("nobottom") else []
        subprocess.run([testdata, "data", str(inputs), run_id, str(cell),
                        str(seed), str(scale)] + extra, check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if "cgrid" in case:
        subprocess.run([testdata, "cgrid", str(inputs), case["cgrid"]],
                       check=True, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
    (inputs / "hpcrequest-local-regression-1.xml").write_text(
        job_xml(name, case))

    archive = casedir / ("%s.tar" % name)
    with tarfile.open(archive, "w") as tar:
        for path in sorted(inputs.iterdir()):
            tar.add(path, arcname=path.name)
    return archive


# ---------------------------------------------------------------------------
# Running
# ---------------------------------------------------------------------------

def mpirun_command(mpirun, nprocs):
    cmd = [mpirun]
    version = subprocess.run([mpirun, "--version"], stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True).stdout
    if "Open MPI" in version or "OpenRTE" in version:
        cmd += ["--oversubscribe"]
    return cmd + ["-np", str(nprocs)]


def run_case(name, case, args):
    casedir = Path(args.workdir) / name
    if casedir.exists():
        shutil.rmtree(casedir)
    casedir.mkdir(parents=True)
    archive = build_archive(name, case, args.testdata, casedir)

    env = dict(os.environ)
    env.setdefault("OMPI_ALLOW_RUN_AS_ROOT", "1")
    env.setdefault("OMPI_ALLOW_RUN_AS_ROOT_CONFIRM", "1")
    env.setdefault("OMPI_MCA_rmaps_base_oversubscribe", "1")
    env["QT_LOGGING_RULES"] = "*.debug=false"

    cmd = mpirun_command(args.mpirun, case["np"]) + [
        args.program, "-walltime", "60",
        "-mgroupcount", str(case.get("mgroupcount", 1)), archive.name]
    log = casedir / "run.log"
    with open(log, "w") as out:
        proc = subprocess.run(cmd, cwd=casedir, env=env, stdout=out,
                              stderr=subprocess.STDOUT, timeout=args.timeout)
    results = casedir / "output" / "analysis-results.tar"
    if proc.returncode != 0 or not results.exists():
        tail = log.read_text(errors="replace").splitlines()[-30:]
        raise RuntimeError("%s: us_mpi_analysis failed (exit %d)\n%s"
                           % (name, proc.returncode, "\n".join(tail)))
    return summarize(results)


# ---------------------------------------------------------------------------
# Result summary
# ---------------------------------------------------------------------------

def model_key(fname):
    # Strip the run-specific date stamp so names are comparable across runs.
    return re.sub(r"_a\d{10}_", "_aDATE_", fname)


def summarize_model(root):
    models = []
    for model in root.iter("model"):
        comps = []
        for ana in model.iter("analyte"):
            comps.append({k: float(ana.get(k, "0"))
                          for k in ("s", "f_f0", "vbar20", "signal")})
        desc = model.get("description", "")
        variance = float(model.get("variance", "nan"))
        total = sum(c["signal"] for c in comps)
        moments = {"total": total, "count": len(comps)}
        if total > 0:
            for key in ("s", "f_f0", "vbar20"):
                mean = sum(c[key] * c["signal"] for c in comps) / total
                var = sum((c[key] - mean) ** 2 * c["signal"]
                          for c in comps) / total
                moments[key + "_mean"] = mean
                moments[key + "_sd"] = math.sqrt(max(var, 0.0))
        models.append({
            "description": re.sub(r"_a\d{10}_", "_aDATE_", desc),
            "rmsd": math.sqrt(variance) if variance >= 0 else variance,
            "moments": moments,
            "solutes": sorted([[c["s"], c["f_f0"], c["vbar20"], c["signal"]]
                               for c in comps]),
        })
    return models


def summarize_noise(root):
    noise = root.find("noise")
    key = "%s:%s" % (noise.get("type"),
                     re.sub(r"_a\d{10}_", "_aDATE_", noise.get("description", "")))
    return key, [float(d.get("v")) for d in noise.iter("d")]


def summarize(results):
    summary = {"models": {}, "noises": {}}
    with tarfile.open(results) as tar:
        for member in sorted(tar.getmembers(), key=lambda m: m.name):
            name = os.path.basename(member.name)
            data = tar.extractfile(member)
            if data is None:
                continue
            text = data.read().decode("utf-8", errors="replace")
            key = model_key(name)
            if name.endswith(".model.xml") or name.endswith(".mdl.tmp"):
                # MC files hold several concatenated model documents
                docs = re.findall(r"<ModelData.*?</ModelData>", text, re.S)
                models = []
                for doc in docs:
                    models += summarize_model(ET.fromstring(doc))
                summary["models"][key] = models
            elif re.match(r"(ti|ri)\.noise\.", name):
                # Noise files are named by a random GUID; key by description
                nkey, values = summarize_noise(ET.fromstring(text))
                summary["noises"][nkey] = values
    return summary


# ---------------------------------------------------------------------------
# Comparison
# ---------------------------------------------------------------------------

def rel_diff(a, b):
    scale = max(abs(a), abs(b), 1e-30)
    return abs(a - b) / scale


def compare(name, base, cur, tol):
    errors = []
    if sorted(base["models"]) != sorted(cur["models"]):
        errors.append("model files differ: %s vs %s"
                      % (sorted(base["models"]), sorted(cur["models"])))
    if sorted(base["noises"]) != sorted(cur["noises"]):
        errors.append("noise files differ: %s vs %s"
                      % (sorted(base["noises"]), sorted(cur["noises"])))

    for fname in sorted(set(base["models"]) & set(cur["models"])):
        bms, cms = base["models"][fname], cur["models"][fname]
        if len(bms) != len(cms):
            errors.append("%s: %d models vs %d" % (fname, len(bms), len(cms)))
            continue
        for idx, (bm, cm) in enumerate(zip(bms, cms)):
            where = "%s[%d]" % (fname, idx)
            if bm["description"] != cm["description"]:
                errors.append("%s: description %s vs baseline %s"
                              % (where, cm["description"], bm["description"]))
            if tol.get("loose"):
                # Only require a fit that is not worse than the baseline
                if cm["rmsd"] > bm["rmsd"] * (1.0 + tol["rmsd_rel"]):
                    errors.append("%s: rmsd %.6g worse than baseline %.6g"
                                  % (where, cm["rmsd"], bm["rmsd"]))
                continue
            if rel_diff(bm["rmsd"], cm["rmsd"]) > tol["rmsd_rel"]:
                errors.append("%s: rmsd %.10g vs baseline %.10g"
                              % (where, cm["rmsd"], bm["rmsd"]))
            for key, bval in bm["moments"].items():
                cval = cm["moments"].get(key)
                if cval is None:
                    errors.append("%s: moment %s missing" % (where, key))
                    continue
                if key == "count":
                    continue
                ref = bm["moments"].get(key.replace("_sd", "_mean"), bval)
                if abs(bval - cval) > tol["moment_rel"] * max(abs(ref), 1e-30):
                    errors.append("%s: %s %.8g vs baseline %.8g"
                                  % (where, key, cval, bval))

    if not tol.get("loose"):
        for fname in sorted(set(base["noises"]) & set(cur["noises"])):
            bn, cn = base["noises"][fname], cur["noises"][fname]
            if len(bn) != len(cn):
                errors.append("%s: %d values vs %d" % (fname, len(cn), len(bn)))
                continue
            worst = max((abs(b - c) for b, c in zip(bn, cn)), default=0.0)
            if worst > tol["noise_abs"]:
                errors.append("%s: max difference %.3g" % (fname, worst))
    return errors


# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check", "update", "list"])
    parser.add_argument("cases", nargs="*")
    parser.add_argument("--program", help="us_mpi_analysis executable")
    parser.add_argument("--testdata", help="us_mpi_testdata executable")
    parser.add_argument("--mpirun", default="mpirun")
    parser.add_argument("--workdir", default="mpi_regression_work")
    parser.add_argument("--timeout", type=int, default=540)
    args = parser.parse_args()

    if args.command == "list":
        print("\n".join(CASES))
        return 0
    if not args.program or not args.testdata:
        parser.error("--program and --testdata are required")

    args.program = str(Path(args.program).resolve())
    args.testdata = str(Path(args.testdata).resolve())
    args.workdir = str(Path(args.workdir).resolve())

    names = list(CASES) if args.cases in ([], ["all"]) else args.cases
    unknown = [n for n in names if n not in CASES]
    if unknown:
        parser.error("unknown case(s): %s" % ", ".join(unknown))

    failed = 0
    for name in names:
        summary = run_case(name, CASES[name], args)
        # A case with "baseline" must equal that case's result
        shared = CASES[name].get("baseline")
        path = BASELINE_DIR / ("%s.json" % (shared or name))
        if args.command == "update" and not shared:
            BASELINE_DIR.mkdir(exist_ok=True)
            path.write_text(json.dumps(summary, indent=1, sort_keys=True) + "\n")
            print("%s: baseline written (%d model files)"
                  % (name, len(summary["models"])))
            continue
        if not path.exists():
            print("%s: FAIL no baseline %s" % (name, path))
            failed += 1
            continue
        tol = dict(DEFAULT_TOL, **CASE_TOL.get(name, {}))
        errors = compare(name, json.loads(path.read_text()), summary, tol)
        if errors:
            failed += 1
            print("%s: FAIL" % name)
            for err in errors[:40]:
                print("   " + err)
        else:
            print("%s: OK" % name)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
