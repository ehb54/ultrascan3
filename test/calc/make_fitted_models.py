#!/usr/bin/env python3
"""Fit the calculation-library fixture samples with us_mpi_analysis.

The fixtures are fitted models of simulated samples whose composition is
known.  This script reuses the job building and running code of
../mpi_analysis/us_mpi_regression.py, keeps every model file the analysis
writes, and records how they were made in fixtures/models/MANIFEST.json.

The models are inputs to the calculation tests, so they are generated once and
then kept unchanged.  Regenerating them changes every expected value derived
from them; run calc_oracle.py afterwards.

Usage:
    make_fitted_models.py --program build/bin/us_mpi_analysis \
        --testdata build/bin/us_mpi_testdata [--mpirun mpirun] [cases...]
"""

import argparse
import datetime
import hashlib
import json
import re
import subprocess
import sys
import tarfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "mpi_analysis"))
import us_mpi_regression as reg  # noqa: E402

MODEL_DIR = HERE / "fixtures" / "models"

# Run-specific analysis date stamps ("_a" + yymmddHHMM) are replaced so that
# file names and descriptions do not change between regenerations.
STAMP = re.compile(r"_a\d{10}_")
FIXED_STAMP = "_a2611090000_"

# Samples:  (s in S, f/f0, signal concentration).  SAMPLE_A is the default
# sample of us_mpi_testdata (kSample in us_mpi_testdata.cpp).
SAMPLE_A = [(2.5, 1.25, 0.25), (4.5, 1.6, 0.40), (7.0, 2.1, 0.30)]
# Monomer, dimer and a 5 % trace aggregate.
SAMPLE_B = [(4.3, 1.25, 0.80), (6.2, 1.35, 0.15), (9.0, 1.5, 0.05)]
# A near-degenerate pair 0.3 S apart and a third species.
SAMPLE_C = [(5.0, 1.4, 0.35), (5.3, 1.45, 0.35), (8.0, 1.8, 0.30)]

SAMPLES = {"A": SAMPLE_A, "B": SAMPLE_B, "C": SAMPLE_C}


def dataset(run_id, cell, seed, sample):
    opts = {} if sample is SAMPLE_A else {"species": sample}
    return (run_id, cell, seed, 1.0, opts)


DS_A1 = dataset("fixA", 1, 11, SAMPLE_A)
DS_A2 = dataset("fixA", 2, 23, SAMPLE_A)
DS_A3 = dataset("fixA", 3, 37, SAMPLE_A)
DS_B = dataset("fixB", 1, 11, SAMPLE_B)
DS_C = dataset("fixC", 1, 11, SAMPLE_C)

IT = dict(reg.GRID_2DSA, max_iterations="3")
MC = dict(reg.GRID_2DSA, mc_iterations="3")

CASES = {
    "A_2dsa_it":       {"sample": "A", "method": "2DSA", "datasets": [DS_A1], "np": 2, "params": IT},
    "A_2dsa_mc":       {"sample": "A", "method": "2DSA", "datasets": [DS_A1], "np": 2, "params": MC},
    "A_pcsa_sl":       {"sample": "A", "method": "PCSA", "datasets": [DS_A1], "np": 2,
                        "params": reg.CASES["pcsa_sl"]["params"]},
    "A_ga":            {"sample": "A", "method": "GA", "datasets": [DS_A1], "np": 4,
                        "params": dict(reg.GA_PARAMS), "buckets": reg.GA_BUCKETS},
    "A_dmga":          {"sample": "A", "method": "DMGA", "datasets": [DS_A1], "np": 2,
                        "params": dict(reg.GA_PARAMS, DC_model="regression.dmga.model.xml"),
                        "dmga": "regression.dmga.model.xml"},
    "A_rep2_2dsa_it":  {"sample": "A", "method": "2DSA", "datasets": [DS_A2], "np": 2, "params": IT},
    "A_rep3_2dsa_it":  {"sample": "A", "method": "2DSA", "datasets": [DS_A3], "np": 2, "params": IT},
    "B_2dsa_it":       {"sample": "B", "method": "2DSA", "datasets": [DS_B], "np": 2, "params": IT},
    "B_2dsa_mc":       {"sample": "B", "method": "2DSA", "datasets": [DS_B], "np": 2, "params": MC},
    "C_2dsa_it":       {"sample": "C", "method": "2DSA", "datasets": [DS_C], "np": 2, "params": IT},
    "C_2dsa_mc":       {"sample": "C", "method": "2DSA", "datasets": [DS_C], "np": 2, "params": MC},
}


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def tool_version(cmd):
    try:
        out = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             universal_newlines=True, timeout=30).stdout
        return out.strip().splitlines()[0] if out.strip() else ""
    except (OSError, subprocess.TimeoutExpired):
        return ""


def git_state():
    root = HERE.parent.parent
    head = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                          stdout=subprocess.PIPE, universal_newlines=True).stdout.strip()
    dirty = subprocess.run(["git", "-C", str(root), "status", "--porcelain", "--untracked-files=no"],
                           stdout=subprocess.PIPE, universal_newlines=True).stdout.strip()
    return head, bool(dirty)


def extract_models(name, results):
    """Copy every model document of the results archive into MODEL_DIR/name."""
    outdir = MODEL_DIR / name
    if outdir.exists():
        for old in outdir.iterdir():
            old.unlink()
    outdir.mkdir(parents=True, exist_ok=True)
    written = []
    with tarfile.open(results) as tar:
        for member in sorted(tar.getmembers(), key=lambda m: m.name):
            base = Path(member.name).name
            if not base.endswith(".model.xml"):
                continue
            data = tar.extractfile(member)
            if data is None:
                continue
            text = STAMP.sub(FIXED_STAMP, data.read().decode("utf-8"))
            target = outdir / STAMP.sub(FIXED_STAMP, base)
            target.write_text(text, encoding="utf-8")
            written.append(target)
    if not written:
        raise RuntimeError("%s: the results archive holds no model files" % name)
    return written


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("cases", nargs="*", help="cases to fit (default: all)")
    parser.add_argument("--program", required=True, help="us_mpi_analysis executable")
    parser.add_argument("--testdata", required=True, help="us_mpi_testdata executable")
    parser.add_argument("--mpirun", default="mpirun")
    parser.add_argument("--workdir", default="calc_fixture_work")
    parser.add_argument("--timeout", type=int, default=900)
    args = parser.parse_args()
    args.debug_text = ""

    names = args.cases or list(CASES)
    unknown = [n for n in names if n not in CASES]
    if unknown:
        parser.error("unknown cases: %s" % ", ".join(unknown))

    manifest_path = MODEL_DIR / "MANIFEST.json"
    manifest = json.loads(manifest_path.read_text()) if manifest_path.exists() else {"cases": {}}

    head, dirty = git_state()
    for name in names:
        case = dict(CASES[name])
        sample = case.pop("sample")
        print("fitting %s ..." % name, flush=True)
        status, text, results = reg.run_job(name, case, args)
        if status != 0 or not results.exists():
            sys.exit("%s: us_mpi_analysis failed (exit %s)\n%s"
                     % (name, status, "\n".join(text.splitlines()[-30:])))
        files = extract_models(name, results)
        manifest["cases"][name] = {
            "sample": sample,
            "species_s_ff0_c": SAMPLES[sample],
            "datasets": [list(ds[:4]) for ds in case["datasets"]],
            "method": case["method"],
            "np": case["np"],
            "params": case["params"],
            "files": {f.name: sha256(f) for f in files},
        }
        print("  %d model file(s)" % len(files), flush=True)

    manifest["generator"] = {
        "script": "test/calc/make_fitted_models.py",
        "ultrascan3_commit": head,
        "worktree_modified": dirty,
        "us_mpi_analysis": str(Path(args.program).resolve().name),
        "mpirun": tool_version([args.mpirun, "--version"]),
        "generated_utc": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "date_stamp_normalized_to": FIXED_STAMP,
    }
    manifest_path.write_text(json.dumps(manifest, indent=1, sort_keys=True) + "\n")
    print("wrote %s" % manifest_path)


if __name__ == "__main__":
    main()
