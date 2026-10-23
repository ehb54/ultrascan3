#!/usr/bin/env python3
"""Regression tests for us_mpi_analysis.

Each case builds an HPC job archive (synthetic data written by
us_mpi_testdata plus an hpcrequest XML in the format the LIMS writes), runs
us_mpi_analysis under mpirun and reduces the result archive to a JSON summary
of every fitted model (RMSD and solute distribution) and noise vector.  The
summary is compared with the stored baseline in baseline/<case>.json.

Input cases (INPUT_CASES) run a regression case with a modified job --
defective or incomplete input, other command-line arguments -- and check how
us_mpi_analysis handles it: an orderly abort with a given message, values in
job_statistics.xml, or a result equal to that of another case.

Commands:
  check  <case>...   run cases and compare with the baselines
  update <case>...   run cases and (re)write the baselines
  list               print the case names

A case with a "baseline" entry has no baseline of its own:  it must give
the result of the named case (the same fit with another layout or input),
and "update" checks it instead of writing a file.

Use "all" as the case name to select every case, "input" for the input
cases.
"""

import argparse
import json
import math
import os
import re
import shutil
import signal
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

GA_PARAMS = {
    "population": "30", "generations": "15", "crossover": "50",
    "mutation": "50", "plague": "4", "elitism": "2",
    "migration": "3", "regularization": "5",
    "conc_threshold": "0.000001", "s_grid": "100", "k_grid": "100",
    "mutate_sigma": "2", "p_mutate_s": "20", "p_mutate_k": "20",
    "p_mutate_sk": "20", "minimize_opt": "0",
    "mc_iterations": "1", "tinoise_option": "0", "rinoise_option": "0",
    "debug_level": "0", "seed": "4711",
}

# The GA mutation parameters and the defaults us_mpi_analysis documents for
# them (us_mpi_analysis.cpp, "Set some defaults")
GA_MUTATION_KEYS = ("mutate_sigma", "p_mutate_s", "p_mutate_k", "p_mutate_sk")
GA_MUTATION_DEFAULTS = {"mutate_sigma": "2.0", "p_mutate_s": "20",
                        "p_mutate_k": "20", "p_mutate_sk": "20"}


def without(params, keys):
    return {k: v for k, v in params.items() if k not in keys}


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
    "2dsa_timestate": {
        # Timestate file given (as in every LIMS job), at 1-second intervals
        "method": "2DSA", "datasets": [DS_A], "np": 2,
        "params": dict(GRID_2DSA), "timestate": "given",
    },
    "2dsa_composite_timestate": {
        "method": "2DSA", "datasets": [DS_A, DS_B], "np": 2,
        "params": dict(GRID_2DSA), "timestate": "given",
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
        "params": dict(GA_PARAMS),
        "buckets": GA_BUCKETS,
    },
    "ga_np2": {
        "method": "GA", "datasets": [DS_A], "np": 2,
        "params": dict(GA_PARAMS),
        "buckets": GA_BUCKETS,
    },
    "dmga_basic": {
        "method": "DMGA", "datasets": [DS_A], "np": 2,
        "params": dict(GA_PARAMS, DC_model="regression.dmga.model.xml"),
        "dmga": "regression.dmga.model.xml",
    },
}

# ---------------------------------------------------------------------------
# Input cases
# ---------------------------------------------------------------------------

HPC_XML = "hpcrequest-local-regression-1.xml"


def no_hpc_xml(inputs):
    # Rename the hpcrequest file so that it does not match hpc*.xml
    (inputs / HPC_XML).rename(inputs / ("job-" + HPC_XML))


def second_hpc_xml(inputs):
    # Another hpcrequest file; it would abort (unknown method) if read
    text = (inputs / HPC_XML).read_text()
    (inputs / "hpcrequest-local-other-2.xml").write_text(
        re.sub(r'method="[^"]*"', 'method="2DXA"', text, count=1))


def unreadable_hpc_xml(inputs):
    # A directory matches the hpc*.xml pattern but cannot be opened as a file
    path = inputs / HPC_XML
    path.unlink()
    path.mkdir()


def jobxml_file(truncate=False, **entries):
    """A jobxmlfile.xml with the given Header entries (task_parse format).

    With truncate, the file ends inside the Header."""
    def prepare(inputs):
        root = ET.Element("Message")
        header = ET.SubElement(root, "Header")
        for key, value in entries.items():
            ET.SubElement(header, key).text = str(value)
        ET.SubElement(root, "Body")
        ET.indent(root)
        text = ET.tostring(root, encoding="unicode")
        if truncate:
            text = text[:text.index("</Header>")]
        (inputs / "jobxmlfile.xml").write_text(text)
    return prepare


def empty_cgrid_model(inputs):
    # A custom grid model without solutes (no analyte elements)
    path = inputs / CASES["2dsa_customgrid"]["cgrid"]
    tree = ET.parse(path)
    for parent in tree.iter():
        for child in list(parent):
            if child.tag == "analyte":
                parent.remove(child)
    tree.write(path, encoding="unicode")


def set_method(method):
    def edit(text):
        return re.sub(r'method="[^"]*"', 'method="%s"' % method, text, count=1)
    return edit


def truncate_after_first_dataset(text):
    # As a file cut off in transfer: the second dataset is missing entirely
    end = text.index("</dataset>") + len("</dataset>")
    return text[:end] + "\n"


def input_case(base, **changes):
    case = dict(CASES[base], base=base)
    if "params" in changes and callable(changes["params"]):
        changes["params"] = changes["params"](case["params"])
    case.update(changes)
    return case


# Each input case documents one defect of the input handling.  "expect":
#   abort       regex the ***ABORTED*** message must match (orderly abort)
#   statistics  attribute values required in job_statistics.xml
#   same_as     case whose result this run must reproduce exactly
#   finite      all fitted values must be finite numbers
#   loose_vs    case whose RMSD this run must not exceed by more than 15 %
#   log         regex the run log must match
#   log_absent  regex the run log must not match
# A case with "pending" documents a defect whose fix is still to come: it is
# reported as XFAIL while it fails and as an error once it passes, so that
# the marker gets removed.
INPUT_CASES = {
    # A9: an abort before the inputs are read archives and removes the
    #     files of the work directory (the job's input files)
    "input_hpcxml_missing": input_case(
        "2dsa_basic", prepare=no_hpc_xml,
        expect={"abort": r"Could not find unique hpc input file"}),
    # A1: unreadable hpcrequest file -> MPI_Finalize, then execution goes on
    "input_hpcxml_unreadable": input_case(
        "2dsa_basic", prepare=unreadable_hpc_xml,
        expect={"abort": r"Cannot open .*hpcrequest"}),
    # A2: a jobxmlfile.xml without walltime resets -walltime to 1440
    "input_walltime_jobxml_without": input_case(
        "2dsa_basic", prepare=jobxml_file(queue="normal"),
        expect={"statistics": {"maxwalltime": "60"}}),
    # A2: the command line takes precedence over the job xml
    "input_walltime_cmdline_precedence": input_case(
        "2dsa_basic", prepare=jobxml_file(walltime="30"),
        expect={"statistics": {"maxwalltime": "60"}}),
    # A3/A6: GA mutation defaults are never applied
    "input_ga_mutation_defaults": input_case(
        "ga_np2", params=lambda p: without(p, GA_MUTATION_KEYS),
        expect={"same_as": "ga_np2"}),
    # A3: DMGA reads mutate_sigma before its default is set
    "input_dmga_sigma_default": input_case(
        "dmga_basic", params=lambda p: without(p, ("mutate_sigma",)),
        expect={"same_as": "dmga_basic"}),
    # A4: a flotation (negative s) GA bucket gives log10 of a negative value
    "input_ga_negative_bucket": input_case(
        "ga_np2", buckets=GA_BUCKETS + [(-3.0, -1.0, 1.0, 2.0)],
        expect={"finite": True, "loose_vs": "ga_np2"}),
    # A5: rotor_stretch with one value is indexed past its end
    "input_rotor_stretch_short": input_case(
        "2dsa_basic", rotor_stretch="0",
        expect={"abort": r"rotor_stretch"}),
    # A8: an XML file cut off after the first of two datasets is run as a
    #     single-dataset fit
    "input_xml_truncated": input_case(
        "2dsa_global", edit_xml=truncate_after_first_dataset,
        expect={"abort": r"XML"}),
    # A8: a job xml file cut off inside its Header
    "input_jobxml_truncated": input_case(
        "2dsa_basic", prepare=jobxml_file(truncate=True, walltime="30"),
        expect={"abort": r"XML"}),

    # A10: a timestate file listed but not in the archive was not created:
    #      ranks ran with different speed profiles
    "input_timestate_missing": input_case(
        "2dsa_timestate", timestate="missing", debug_log=True,
        expect={"same_as": "2dsa_timestate",
                "log_absent": r"timestate file does not exist"}),

    # Two hpcrequest files: not unique without -hpcrequest, the one named
    #  with it
    "input_hpcxml_two": input_case(
        "2dsa_basic", prepare=second_hpc_xml,
        expect={"abort": r"Could not find unique hpc input file"}),
    "input_hpcxml_named": input_case(
        "2dsa_basic", prepare=second_hpc_xml,
        args=["-walltime", "60", "-mgroupcount", "1", "-hpcrequest", HPC_XML],
        expect={"same_as": "2dsa_basic"}),

    # D2: an abort on the master alone, during the run: abort() waited in a
    #     barrier the workers never reach.  Only the master reads the
    #     custom grid model.
    "input_master_abort": input_case(
        "2dsa_customgrid", prepare=empty_cgrid_model, timeout=90,
        expect={"abort": r"no solutes"}),
    # One process: the master waited for workers that do not exist
    "input_single_rank": input_case(
        "2dsa_basic", np=1, timeout=60,
        expect={"abort": r"at least 2 processes"}),
    # D2: a data read failing on one worker only (test hook in debug_text)
    "input_load_fails_on_worker": input_case(
        "2dsa_basic", params=lambda p: dict(p, debug_text="fail_load_rank=1"),
        timeout=90, expect={"abort": r"Bad data file.*rank 1"}),

    # Validation of job parameters (US_MPI_JobConfig::validate)
    "input_invalid_number": input_case(
        "2dsa_basic", params=lambda p: dict(p, s_max="ten"),
        expect={"abort": r"s_max"}),
    "input_missing_required": input_case(
        "2dsa_basic", params=lambda p: without(p, ("s_max",)),
        expect={"abort": r"s_max"}),
    "input_unknown_method": input_case(
        "2dsa_basic", edit_xml=set_method("2DXA"),
        expect={"abort": r"2DXA"}),
    "input_meniscus_points_negative": input_case(
        "2dsa_meniscus", params=lambda p: dict(p, meniscus_points="-3"),
        expect={"abort": r"meniscus_points"}),
    "input_bucket_inverted": input_case(
        "ga_np2", buckets=GA_BUCKETS + [(9.5, 8.6, 1.0, 2.0)],
        expect={"abort": r"[Bb]ucket"}),
    "input_centerpiece_bottom_default": input_case(
        "2dsa_basic", centerpiece_bottom="7.3",
        expect={"abort": r"invalid default value of 7\.3"}),
    # An unknown parameter is reported, the run goes on
    "input_unknown_key": input_case(
        "2dsa_basic", params=lambda p: dict(p, s_mx="10.0"),
        expect={"same_as": "2dsa_basic", "log": r"[Uu]nknown parameter.*s_mx"}),
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

INPUT_TIMEOUT = 180   # A broken input must not run for long


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
        if key in ("CG_model", "DC_model"):
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
        if case.get("timestate"):
            # As in LIMS jobs; "missing": listed, but not in the archive
            sub(files, "timestate", filename="%s.time_state.tmst" % run_id)
        solution = sub(dset, "solution")
        sub(solution, "buffer", density="0.998234", viscosity="1.00194",
            compress="0", manual="0")
        sub(solution, "analyte", mw="50000", vbar20="0.72", amount="1",
            type="Protein")
        # Without band_volume the default (0.015) makes a band-forming run
        sub(dset, "band_volume", value="0.0")
        sub(dset, "rotor_stretch", value=case.get("rotor_stretch", "0 1e-12"))
        sub(dset, "centerpiece_bottom", value=case.get("centerpiece_bottom", "7.2"))
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
        extra += ["tmst"] if case.get("timestate") == "given" else []
        subprocess.run([testdata, "data", str(inputs), run_id, str(cell),
                        str(seed), str(scale)] + extra, check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for kind in ("cgrid", "dmga"):
        if kind in case:
            subprocess.run([testdata, kind, str(inputs), case[kind]],
                           check=True, stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)
    text = job_xml(name, case)
    if "edit_xml" in case:
        text = case["edit_xml"](text)
    (inputs / "hpcrequest-local-regression-1.xml").write_text(text)
    if "prepare" in case:
        case["prepare"](inputs)

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


def kill_process_tree(proc):
    """Stop mpirun and every process it started (the ranks, and on Windows
    the smpd of MS-MPI), so that no rank keeps running or holds the
    program file open."""
    if sys.platform == "win32":
        # No process groups to signal:  taskkill /T follows the parent ids
        subprocess.run(["taskkill", "/F", "/T", "/PID", str(proc.pid)],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    else:
        os.killpg(proc.pid, signal.SIGKILL)
    proc.wait()


def run_job(name, case, args):
    """Build the archive and run us_mpi_analysis.

    Returns (exit status, log text, path of analysis-results.tar)."""
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

    if case.get("debug_log"):
        env["QT_LOGGING_RULES"] = "*.debug=true"

    cmd = mpirun_command(args.mpirun, case["np"]) + [args.program]
    cmd += case.get("args", ["-walltime", "60",
                             "-mgroupcount", str(case.get("mgroupcount", 1))])
    cmd += [archive.name]
    # The log is kept outside the work directory, which a defect may clear
    log = casedir.parent / ("%s.log" % name)
    with open(log, "w") as out:
        # Own process group, so that a hanging run is stopped completely
        if sys.platform == "win32":
            group = {"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP}
        else:
            group = {"start_new_session": True}
        proc = subprocess.Popen(cmd, cwd=casedir, env=env, stdout=out,
                                stderr=subprocess.STDOUT, **group)
        try:
            status = proc.wait(timeout=case.get("timeout", args.timeout))
        except subprocess.TimeoutExpired:
            kill_process_tree(proc)
            status = "timeout"
    results = casedir / "output" / "analysis-results.tar"
    return status, log.read_text(errors="replace"), results


def lost_inputs(name, case, args):
    """Input files that are no longer in the work directory."""
    casedir = Path(args.workdir) / name
    expected = [casedir / ("%s.tar" % name)]
    expected += [casedir / p.name for p in (casedir / "inputs").iterdir()]
    return [p.name for p in expected if not p.exists()]


def run_case(name, case, args):
    status, text, results = run_job(name, case, args)
    if status != 0 or not results.exists():
        tail = text.splitlines()[-30:]
        raise RuntimeError("%s: us_mpi_analysis failed (exit %s)\n%s"
                           % (name, status, "\n".join(tail)))
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


def job_statistics(results):
    """Attributes of job_statistics.xml (statistics and id elements)."""
    with tarfile.open(results) as tar:
        for member in tar.getmembers():
            if os.path.basename(member.name) == "job_statistics.xml":
                root = ET.fromstring(tar.extractfile(member).read())
                stats = {}
                for elem in root:
                    stats.update(elem.attrib)
                return stats
    return {}


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


def all_finite(summary):
    for models in summary["models"].values():
        for model in models:
            values = [model["rmsd"]] + list(model["moments"].values())
            values += [v for sol in model["solutes"] for v in sol]
            if not all(math.isfinite(v) for v in values):
                return False
    return all(math.isfinite(v) for vals in summary["noises"].values()
               for v in vals)


def check_input_case(name, case, args, reference):
    """Run an input case; return a list of errors (empty if it passed)."""
    case = dict(case, timeout=min(args.timeout, case.get("timeout", INPUT_TIMEOUT)))
    expect = case["expect"]
    status, text, results = run_job(name, case, args)
    tail = "\n      ".join(text.splitlines()[-12:])

    if "abort" in expect:
        aborts = re.findall(r"\*\*\*ABORTED\*\*\*:\s*(.*)", text)
        if status == 0 or status == "timeout":
            return ["expected an abort, got exit %s\n      %s" % (status, tail)]
        if not any(re.search(expect["abort"], msg) for msg in aborts):
            return ["expected abort message /%s/, got %s (exit %s)\n      %s"
                    % (expect["abort"], aborts or "no abort message",
                       status, tail)]
        # An abort must leave the job's input files in place
        lost = lost_inputs(name, case, args)
        if lost:
            return ["abort removed input files from the work directory: %s"
                    % ", ".join(sorted(lost))]
        return []

    if status != 0 or not results.exists():
        return ["us_mpi_analysis failed (exit %s)\n      %s" % (status, tail)]

    errors = []
    if "log" in expect and not re.search(expect["log"], text):
        errors.append("log does not match /%s/" % expect["log"])
    if "log_absent" in expect:
        found = re.search(r".*%s.*" % expect["log_absent"], text)
        if found:
            errors.append("log matches /%s/: %s"
                          % (expect["log_absent"], found.group(0).strip()))
    summary = summarize(results)
    for key, value in expect.get("statistics", {}).items():
        got = job_statistics(results).get(key)
        if got != value:
            errors.append("job_statistics %s=%s, expected %s" % (key, got, value))
    if expect.get("finite") and not all_finite(summary):
        errors.append("result contains non-finite values")
    if "same_as" in expect:
        errors += compare(name, reference(expect["same_as"]), summary,
                          DEFAULT_TOL)
    if "loose_vs" in expect:
        errors += compare(name, reference(expect["loose_vs"]), summary,
                          dict(DEFAULT_TOL, **LOOSE))
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
        print("\n".join(list(CASES) + list(INPUT_CASES)))
        return 0
    if not args.program or not args.testdata:
        parser.error("--program and --testdata are required")

    args.program = str(Path(args.program).resolve())
    args.testdata = str(Path(args.testdata).resolve())
    args.workdir = str(Path(args.workdir).resolve())

    if args.cases in ([], ["all"]):
        names = list(CASES) + list(INPUT_CASES)
    elif args.cases == ["input"]:
        names = list(INPUT_CASES)
    else:
        names = args.cases
    unknown = [n for n in names if n not in CASES and n not in INPUT_CASES]
    if unknown:
        parser.error("unknown case(s): %s" % ", ".join(unknown))

    references = {}

    def reference(ref):
        # Reference result for an input case, run once
        if ref not in references:
            references[ref] = run_case(ref + "_ref", CASES[ref], args)
        return references[ref]

    failed = 0
    for name in names:
        if name in INPUT_CASES:
            if args.command == "update":
                continue    # Input cases have no baseline
            case = INPUT_CASES[name]
            errors = check_input_case(name, case, args, reference)
            pending = case.get("pending")
            if errors and pending:
                print("%s: XFAIL (pending %s)" % (name, pending))
                for err in errors[:5]:
                    print("   " + err)
            elif errors:
                failed += 1
                print("%s: FAIL" % name)
                for err in errors[:40]:
                    print("   " + err)
            elif pending:
                failed += 1
                print("%s: XPASS -- remove the pending marker (%s)"
                      % (name, pending))
            else:
                print("%s: OK" % name)
            continue
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
