#!/usr/bin/env python3
"""GA benchmark:  main vs branch (all fixes + speedup), GA initialized from 2DSA-IT.

Stages (each resumable; results in WORK/):
  2dsa     2DSA-IT of every dataset (one build; the result is shared)
  buckets  GA buckets from each 2DSA-IT model:  one solute per peak
           (signal-weighted centroid), then a port of us_ga_init "Autoassign
           Solute Bins" (US_SoluteData::autoCalcBins:  fixed-size box around
           each, overlaps split)
  ga       GA runs:  every dataset x seed x build (LIMS default GA settings)
  report   scores against the known truth, timing, tables (JSON + Markdown)

usage:
  ga_2dsa_bench.py STAGE --repo BRANCH_CHECKOUT --work WORK --np N
                [--main MAIN_BINARY] [--branch BRANCH_BINARY] [--testdata BIN]
                [--dna DIR] [--seeds 1-10] [--datasets a,b] [--jobs J] [--small]
"""
import argparse
import json
import math
import os
import shutil
import statistics as st
import subprocess
import sys
import tarfile
import time
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

# ---------------------------------------------------------------------------
# Datasets and truth
# ---------------------------------------------------------------------------
BASE3 = [(2.5, 1.25, 0.25), (4.5, 1.6, 0.40), (7.0, 2.1, 0.30)]
SYNTH = {   # us_mpi_testdata samples (s in S at 20,W, f/f0, concentration)
    "base3":       {"species": BASE3, "noise": 0.004},
    "base3_noisy": {"species": BASE3, "noise": 0.010},
    "same_s":      {"species": [(4.0, 1.25, 0.4), (4.0, 2.0, 0.4), (7.0, 1.6, 0.2)], "noise": 0.004},
    "five":        {"species": [(2.0, 1.2, 0.2), (3.5, 1.4, 0.2), (5.0, 1.6, 0.2),
                                (7.0, 1.9, 0.2), (9.5, 2.3, 0.2)], "noise": 0.004},
}
DNA_TRUTH = [(5.94054, 3.02921, 0.2), (7.09909, 4.02518, 0.2), (7.75376, 4.82969, 0.2),
             (8.23625, 5.50832, 0.2), (8.55466, 6.15413, 0.2)]
DNA_RUNS = {"20k": ("5DNA-New-fragments20k-extracheck", "2610291512"),
            "40k": ("5DNA-New-fragments40k-extracheck", "2610291512"),
            "60k": ("5DNA-New-fragments60k-extracheck", "2610291513")}
DNA = {"dna20k": ["20k"], "dna40k": ["40k"], "dna60k": ["60k"], "dna_global": ["20k", "40k", "60k"]}
DATASETS = list(SYNTH) + list(DNA)
DNA_NOISE = 0.0025   # (fit RMSD of the truth model, for rmsd/sigma only)

# 2DSA-IT (standard):  64 x 64 points in 6 x 6 subgrids, 10 refinement iterations
TWODSA = {"ff0_min": "1.0", "ff0_max": "8.0", "s_grid_points": "64", "ff0_grid_points": "64",
          "uniform_grid": "6", "max_iterations": "10", "mc_iterations": "1",
          "tinoise_option": "0", "rinoise_option": "0", "meniscus_points": "1",
          "meniscus_range": "0.0", "fit_mb_select": "0", "debug_level": "0", "seed": "4711"}
S_RANGE = {"synth": (1.0, 10.0), "dna": (2.0, 12.0)}

# GA:  LIMS defaults
GA = {"population": "200", "generations": "100", "crossover": "50", "mutation": "50",
      "plague": "4", "elitism": "2", "migration": "3", "regularization": "0.05",
      "conc_threshold": "0.000001", "s_grid": "100", "k_grid": "100", "mutate_sigma": "2",
      "p_mutate_s": "20", "p_mutate_k": "20", "p_mutate_sk": "20", "minimize_opt": "0",
      "mc_iterations": "1", "tinoise_option": "0", "rinoise_option": "0", "debug_level": "0"}
BRANCH_EXTRA = {"ga_normal": "1"}   # speedup switch (branch only)
BUCKET_GRID_STEPS = 8               # autoassign box size in 2DSA grid steps
PEAK_MIN_FRACTION = 0.01            # peaks below 1% of the signal are not counted
PEAK_LINK_STEPS = 2.0               # solutes within this many grid steps form one peak

reg = None   # us_mpi_regression module (from the branch checkout)


def is_dna(name):
    return name.startswith("dna")


def truth(name):
    return DNA_TRUTH if is_dna(name) else SYNTH[name]["species"]


def sigma(name):
    return DNA_NOISE if is_dna(name) else SYNTH[name]["noise"]


# ---------------------------------------------------------------------------
# Job archives
# ---------------------------------------------------------------------------
def dna_job_xml(method, params, buckets, speeds, dna_dir):
    root = ET.Element("US_JobSubmit", method=method, version="1.0")
    job = reg.sub(root, "job")
    reg.sub(job, "cluster", name="local", shortname="local")
    reg.sub(job, "name", value="astf_bench")
    reg.sub(job, "udp", server="127.0.0.1", port="12233")
    reg.sub(job, "global_fit", value="1" if len(speeds) > 1 else "0")
    reg.sub(job, "request", id="1", guid="00000000-0000-4000-8000-000000000001")
    jp = reg.sub(job, "jobParameters")
    for key, value in params.items():
        reg.sub(jp, key, value=value)
    for (xmin, xmax, ymin, ymax) in buckets:
        reg.sub(jp, "bucket", s_min="%.6g" % xmin, s_max="%.6g" % xmax,
                ff0_min="%.6g" % ymin, ff0_max="%.6g" % ymax)
    for speed in speeds:
        run_id, edit = DNA_RUNS[speed]
        raw = ET.parse(Path(dna_dir) / run_id / ("%s.RA.xml" % run_id)).getroot()
        dset = reg.sub(root, "dataset")
        files = reg.sub(dset, "files")
        reg.sub(files, "auc", filename="%s.RA.1.S.0.auc" % run_id)
        reg.sub(files, "edit", filename="%s.%s.RA.1.S.0.xml" % (run_id, edit))
        reg.sub(files, "timestate", filename="%s.time_state.tmst" % run_id)
        sol = reg.sub(dset, "solution")
        reg.sub(sol, "buffer", density="1.007810", viscosity="1.03432", compress="0", manual="0")
        reg.sub(sol, "analyte", mw="363185", vbar20="0.51", amount="1", type="DNA")
        dset.append(raw.find(".//speedstep"))
        reg.sub(dset, "band_volume", value="0.0")
        reg.sub(dset, "rotor_stretch", value="0 0")
        reg.sub(dset, "centerpiece_bottom", value="7.2")
        reg.sub(dset, "centerpiece_shape", value="standard")
        reg.sub(dset, "centerpiece_angle", value="2.5")
        reg.sub(dset, "centerpiece_pathlength", value="1.2")
        reg.sub(dset, "centerpiece_width", value="0")
        reg.sub(dset, "total_concentration", value="1.0")
    if hasattr(ET, "indent"):
        ET.indent(root)
    return ET.tostring(root, encoding="unicode")


def build_archive(name, method, params, buckets, casedir, args):
    casedir.mkdir(parents=True)
    if is_dna(name):
        inputs = casedir / "inputs"
        inputs.mkdir()
        for speed in DNA[name]:
            run_id, edit = DNA_RUNS[speed]
            src = Path(args.dna) / run_id
            for fn in ("%s.RA.1.S.0.auc" % run_id, "%s.%s.RA.1.S.0.xml" % (run_id, edit),
                       "%s.time_state.tmst" % run_id, "%s.time_state.xml" % run_id):
                shutil.copy(src / fn, inputs / fn)
        (inputs / "hpcrequest-local-astf-1.xml").write_text(
            dna_job_xml(method, params, buckets, DNA[name], args.dna))
        archive = casedir / "job.tar"
        with tarfile.open(archive, "w") as tar:
            for path in sorted(inputs.iterdir()):
                tar.add(path, arcname=path.name)
        return archive
    sp = SYNTH[name]
    case = {"method": method, "params": params, "buckets": buckets,
            "datasets": [("regA", 1, 11, 1.0, {"species": sp["species"], "noise": sp["noise"]})]}
    return reg.build_archive("job", case, args.testdata, casedir)


def run_job(program, name, method, params, buckets, casedir, args, env_extra=None):
    """Run one us_mpi_analysis job; returns (models dict, wall, log text)."""
    if casedir.exists():
        shutil.rmtree(casedir)
    archive = build_archive(name, method, params, buckets, casedir, args)
    env = dict(os.environ, OMPI_ALLOW_RUN_AS_ROOT="1", OMPI_ALLOW_RUN_AS_ROOT_CONFIRM="1",
               OMPI_MCA_rmaps_base_oversubscribe="1", **(env_extra or {}))
    cmd = reg.mpirun_command(args.mpirun, args.np) + [program, "-walltime", "100000",
                                                      "-mgroupcount", "1", archive.name]
    t0 = time.time()
    proc = subprocess.run(cmd, cwd=casedir, env=env, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, universal_newlines=True)
    wall = time.time() - t0
    (casedir / "run.log").write_text(proc.stdout)
    res = casedir / "output" / "analysis-results.tar"
    if proc.returncode != 0 or not res.exists():
        raise RuntimeError("%s failed (rc %d):\n%s" % (casedir, proc.returncode,
                                                      "\n".join(proc.stdout.splitlines()[-30:])))
    return reg.summarize(res)["models"], wall, proc.stdout


# ---------------------------------------------------------------------------
# Stage 1:  2DSA-IT
# ---------------------------------------------------------------------------
def twodsa_params(name):
    smin, smax = S_RANGE["dna" if is_dna(name) else "synth"]
    return dict(TWODSA, s_min="%g" % smin, s_max="%g" % smax)


def final_model(models):
    """The final model of a job:  the last iteration's model of the first file
    (a global fit writes one model file per data set, with the same solutes)."""
    keys = sorted(models)
    mods = models[keys[0]]
    return mods[-1]


def stage_2dsa(args):
    out = Path(args.work) / "2dsa.json"
    res = json.loads(out.read_text()) if out.exists() else {}
    for name in args.datasets:
        if name in res:
            continue
        params = twodsa_params(name)
        models, wall, _log = run_job(args.branch, name, "2DSA", params, [],
                                     Path(args.work) / "2dsa" / name, args)
        m = final_model(models)
        res[name] = {"params": params, "wall": wall, "rmsd": m["rmsd"],
                     "solutes": m["solutes"], "files": sorted(models)}
        out.write_text(json.dumps(res, indent=1))
        print("2DSA-IT %-12s rmsd %.5f  %d solutes  %.0f s" % (name, m["rmsd"], len(m["solutes"]), wall),
              flush=True)


# ---------------------------------------------------------------------------
# Stage 2:  buckets (port of US_SoluteData::autoCalcBins, us_ga_init)
# ---------------------------------------------------------------------------
MIN_VHR = 0.01


class Buk:
    __slots__ = ("x", "x_min", "x_max", "y", "y_min", "y_max", "conc", "status")

    def __init__(self, **kw):
        for k, v in kw.items():
            setattr(self, k, v)

    def copy(self):
        return Buk(**{k: getattr(self, k) for k in self.__slots__})


def limit_bucket(b):
    """US_SoluteData::limitBucket for x = s, y = f/f0."""
    if b.x_min > 0.0:
        b.x_min = max(0.1, b.x_min)
        b.x_max = max(b.x_min + 0.0001, b.x_max)
    elif b.x_max <= 0.0:
        b.x_max = min(-0.1, b.x_max)
        b.x_min = min(b.x_max - 0.0001, b.x_min)
    elif b.x_min + b.x_max >= 0.0:
        b.x_min = 0.1
        b.x_max = max(0.2, b.x_max)
    else:
        b.x_min = min(-0.2, b.x_min)
        b.x_max = -0.1
    b.y_min = max(1.0, b.y_min)
    b.y_max = max(b.y_min + 0.0001, b.y_max)


def auto_calc_bins(distro, mxsols, wsbuck, hfbuck):
    """US_SoluteData::autoCalcBins:  a wsbuck x hfbuck box around each of the
    mxsols highest-concentration solutes, overlaps resolved by splitting."""
    wh, hh = wsbuck / 2.0, hfbuck / 2.0
    nisols = len(distro)
    ntsols = nisols if (mxsols < 2 or mxsols > nisols) else mxsols
    buks1 = []
    for (s, k, c) in distro:
        b = Buk(x=s, x_min=s - wh, x_max=s + wh, y=k, y_min=k - hh, y_max=k + hh, conc=c, status=2)
        limit_bucket(b)
        buks1.append(b)
    cvals = sorted(c for (_s, _k, c) in distro)
    cutlo = cvals[nisols - ntsols]
    buks1 = [b for b in buks1 if b.conc >= cutlo]
    nssols = len(buks1)

    def ok(b):
        return (b.x_max - b.x_min) / wsbuck > MIN_VHR and (b.y_max - b.y_min) / hfbuck > MIN_VHR

    overs, tstat = True, 1
    while overs:
        novls = 0
        buks2 = [buks1[0]]
        for jj in range(1, nssols):
            buk = buks1[jj].copy()
            if buk.status == tstat:
                buks2.append(buk)
                continue
            cx1, cx2, cy1, cy2 = buk.x_min, buk.x_max, buk.y_min, buk.y_max
            for kk in range(jj):
                p = buks1[kk]
                px1, px2, py1, py2 = p.x_min, p.x_max, p.y_min, p.y_max
                if cx1 < px2:
                    if cy1 < py2 and cy2 > py2:          # lower left
                        novls += 1
                        buk.status = 1
                        b2 = buk.copy()
                        horzr, vertr = (px2 - cx1) / wsbuck, (py2 - cy1) / hfbuck
                        if vertr < horzr:
                            b2.x_min, b2.x_max = min(cx1, px2), max(px2, cx1)
                            b2.y_min, b2.y_max = min(py2, cy2), max(cy2, py2)
                            if ok(b2): buks2.append(b2)
                            buk.x_min, buk.x_max = min(px2, cx2), max(cx2, px2)
                            buk.y_min, buk.y_max = cy1, cy2
                        else:
                            b2.x_min, b2.x_max = cx1, cx2
                            b2.y_min, b2.y_max = min(py2, cy2), max(cy2, py2)
                            if ok(b2): buks2.append(b2)
                            buk.x_min, buk.x_max = min(px2, cx2), max(cx2, px2)
                            buk.y_min, buk.y_max = min(cy1, py2), max(py2, cy1)
                        break
                    elif cy2 > py1 and cy1 < py1:        # upper left
                        novls += 1
                        buk.status = 1
                        b2 = buk.copy()
                        horzr, vertr = (px2 - cx1) / wsbuck, (cy2 - py1) / hfbuck
                        if vertr < horzr:
                            b2.x_min, b2.x_max = min(cx1, px2), max(px2, cx1)
                            b2.y_min, b2.y_max = min(cy1, py1), max(py1, cy1)
                            if ok(b2): buks2.append(b2)
                            buk.x_min, buk.x_max = min(px2, cx2), max(cx2, px2)
                            buk.y_min, buk.y_max = cy1, cy2
                        else:
                            b2.x_min, b2.x_max = min(px2, cx2), max(cx2, px2)
                            b2.y_min, b2.y_max = min(py1, cy2), max(cy2, py1)
                            if ok(b2): buks2.append(b2)
                            buk.x_min, buk.x_max = min(cx1, cx2), max(cx2, cx1)
                            buk.y_min, buk.y_max = min(cy1, py1), max(py1, cy1)
                        break
                    elif cy1 >= py1 and cy2 <= py2:      # middle left
                        novls += 1
                        buk.status = 1
                        buk.x_min, buk.x_max = min(px2, cx2), max(cx2, px2)
                        break
                    elif cy2 > py2 and cy1 < py1:        # upper and lower left
                        novls += 1
                        buk.status = 1
                        b2 = buk.copy()
                        horzr, vertr = (px2 - cx1) / wsbuck, (py2 - py1) / hfbuck
                        if vertr < horzr:
                            b2.x_min, b2.x_max = min(cx1, px2), max(px2, cx1)
                            b2.y_min, b2.y_max = min(py1, cy2), max(cy2, py1)
                            if ok(b2): buks2.append(b2.copy())
                            b2.y_min, b2.y_max = min(cy1, py1), max(py1, cy1)
                            if ok(b2): buks2.append(b2.copy())
                            buk.x_min, buk.x_max = min(px2, cx2), max(cx2, px2)
                            buk.y_min, buk.y_max = cy1, cy2
                        else:
                            b2.x_min, b2.x_max = min(cx1, cx2), max(cx2, cx1)
                            b2.y_min, b2.y_max = min(py2, cy2), max(cy2, py2)
                            if ok(b2): buks2.append(b2.copy())
                            b2.y_min, b2.y_max = min(cy1, py1), max(py1, cy1)
                            if ok(b2): buks2.append(b2.copy())
                            buk.x_min, buk.x_max = min(px2, cx2), max(cx2, px2)
                            buk.y_min, buk.y_max = min(py1, py2), max(py2, py1)
                        break
                elif cx1 == px1:
                    if cy1 < py1 and cy2 > py1:          # pure vertical, upper part
                        novls += 1
                        buk.status = 1
                        buk.y_min, buk.y_max = min(cy1, py1), max(py1, cy1)
                        break
            if ok(buk):
                buks2.append(buk)
        nssols = len(buks2)
        buks1 = buks2
        if novls > 0:
            buks1.sort(key=lambda b: (b.x_min, b.y_min))
        elif tstat == 1:
            tstat = 4
        else:
            overs = False
    buks1.sort(key=lambda b: (b.x, b.y))   # re-sort on the center point
    out = []
    for b in buks1:
        limit_bucket(b)
        out.append((b.x_min, b.x_max, b.y_min, b.y_max))
    return out


def peak_solutes(distro, ds, dk):
    """Peaks of a 2DSA-IT distribution:  clusters of solutes connected within
    PEAK_LINK_STEPS grid steps in s and f/f0, holding at least 1% of the signal.  Each peak
    is returned as one solute at its signal-weighted centroid with the peak's
    total signal (autoassign then puts one box around each peak)."""
    n = len(distro)
    parent = list(range(n))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    for i in range(n):
        for j in range(i + 1, n):
            if (abs(distro[i][0] - distro[j][0]) <= PEAK_LINK_STEPS * ds
                    and abs(distro[i][1] - distro[j][1]) <= PEAK_LINK_STEPS * dk):
                parent[find(i)] = find(j)
    total = sum(c for (_s, _k, c) in distro)
    cl = {}
    for i, (s, k, c) in enumerate(distro):
        a = cl.setdefault(find(i), [0.0, 0.0, 0.0])
        a[0] += s * c
        a[1] += k * c
        a[2] += c
    peaks = [(a[0] / a[2], a[1] / a[2], a[2]) for a in cl.values() if a[2] >= PEAK_MIN_FRACTION * total]
    return sorted(peaks)


def stage_buckets(args):
    dsa = json.loads((Path(args.work) / "2dsa.json").read_text())
    out = Path(args.work) / "buckets.json"
    res = json.loads(out.read_text()) if out.exists() else {}
    for name in args.datasets:
        p = dsa[name]["params"]
        ds = (float(p["s_max"]) - float(p["s_min"])) / (int(p["s_grid_points"]) - 1)
        dk = (float(p["ff0_max"]) - float(p["ff0_min"])) / (int(p["ff0_grid_points"]) - 1)
        # Distribution (s in S, f/f0, signal), duplicates summed (us_ga_init sort_distro)
        acc = {}
        for (s, k, _v, c) in dsa[name]["solutes"]:
            key = (round(s * 1e13, 9), round(k, 9))
            acc[key] = acc.get(key, 0.0) + c
        distro = sorted((s, k, c) for (s, k), c in acc.items())
        peaks = peak_solutes(distro, ds, dk)
        npk = len(peaks)
        bks = auto_calc_bins(peaks, npk, BUCKET_GRID_STEPS * ds, BUCKET_GRID_STEPS * dk)
        res[name] = {"peaks": peaks, "box": [BUCKET_GRID_STEPS * ds, BUCKET_GRID_STEPS * dk],
                     "buckets": bks, "n_2dsa_solutes": len(distro)}
        cover = [any(x0 <= t[0] <= x1 and y0 <= t[1] <= y1 for (x0, x1, y0, y1) in bks) for t in truth(name)]
        res[name]["truth_covered"] = cover
        print("buckets %-12s %d peaks -> %d buckets;  true species inside a bucket: %d/%d" %
              (name, npk, len(bks), sum(cover), len(cover)))
        for b in bks:
            print("    s %.3f-%.3f  f/f0 %.3f-%.3f" % b)
    out.write_text(json.dumps(res, indent=1))


# ---------------------------------------------------------------------------
# Stage 3:  GA runs
# ---------------------------------------------------------------------------
def parse_timing(log):
    import re
    ft = [(float(a), int(b), int(c)) for a, b, c in
          re.findall(r"fitness time ([\d.eE+-]+) s for (\d+) checks, (\d+) computed", log)]
    gens = [int(g) for g in re.findall(r"Finish signalled at deme generation (\d+)", log)]
    early = "Early Termination" in log
    return ft, gens, early


def stage_ga(args):
    bks = json.loads((Path(args.work) / "buckets.json").read_text())
    out = Path(args.work) / "ga.json"
    res = json.loads(out.read_text()) if out.exists() else []
    done = {(r["build"], r["dataset"], r["seed"]) for r in res}
    todo = [(b, n, s) for s in args.seeds for n in args.datasets for b in ("main", "branch")
            if (b, n, s) not in done]
    print("%d GA runs to do" % len(todo), flush=True)

    def one(item):
        build, name, seed = item
        params = dict(GA, seed=str(seed))
        if args.small:
            params.update(population="40", generations="20")
        prog = args.main if build == "main" else args.branch
        if build == "branch":
            params.update(BRANCH_EXTRA)
        casedir = Path(args.work) / "ga" / ("%s_%s_s%d" % (build, name, seed))
        models, wall, log = run_job(prog, name, "GA", params, bks[name]["buckets"], casedir, args)
        ft, gens, early = parse_timing(log)
        m = final_model(models)
        rmsds = [final_model({k: v})["rmsd"] for k, v in models.items()]
        shutil.rmtree(casedir / "inputs", ignore_errors=True)
        return {"build": build, "dataset": name, "seed": seed, "wall": wall, "rmsd": m["rmsd"],
                "rmsds": rmsds, "solutes": m["solutes"], "fit_timing": ft, "generations": gens,
                "early": early, "np": args.np}

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for row in pool.map(one, todo):
            res.append(row)
            out.write_text(json.dumps(res, indent=0))
            print("%-6s %-12s seed %2d: rmsd %.5f  %3d solutes  wall %6.1f s" %
                  (row["build"], row["dataset"], row["seed"], row["rmsd"], len(row["solutes"]), row["wall"]),
                  flush=True)


# ---------------------------------------------------------------------------
# Stage 4:  report
# ---------------------------------------------------------------------------
AVOGADRO, ETA20W, RHO20W = 6.02214076e23, 0.01002, 0.998234


def molar_mass(s_sv, ff0, vbar):
    s = s_sv * 1.0e-13
    m23 = (s * AVOGADRO * 6.0 * math.pi * ETA20W * ff0
           * (3.0 * vbar / (4.0 * math.pi * AVOGADRO)) ** (1.0 / 3.0) / (1.0 - vbar * RHO20W))
    return m23 ** 1.5


def score(row):
    """Each fitted solute is assigned to the nearest true species (distance in
    s relative to the truth's s range and f/f0 relative to its f/f0 range);
    signal-weighted s, f/f0 and the summed signal are compared with the truth."""
    tr = truth(row["dataset"])
    vbar = 0.51 if is_dna(row["dataset"]) else 0.72
    srange = max(t[0] for t in tr) - min(t[0] for t in tr) or 1.0
    krange = max(t[1] for t in tr) - min(t[1] for t in tr) or 1.0
    groups = [[] for _ in tr]
    for (s, k, _v, c) in row["solutes"]:
        s *= 1e13
        d = [((s - t[0]) / srange) ** 2 + ((k - t[1]) / krange) ** 2 for t in tr]
        groups[d.index(min(d))].append((s, k, c))
    total = sum(c for g in groups for (_s, _k, c) in g) or 1.0
    tsum = sum(t[2] for t in tr)
    sp = []
    for t, g in zip(tr, groups):
        csum = sum(c for (_s, _k, c) in g)
        if csum <= 0.01 * total:
            sp.append(None)
            continue
        s = sum(s * c for (s, _k, c) in g) / csum
        k = sum(k * c for (_s, k, c) in g) / csum
        sp.append({"ds": s / t[0] - 1, "dk": k / t[1] - 1,
                   "dc": (csum / total) / (t[2] / tsum) - 1,
                   "dM": molar_mass(s, k, vbar) / molar_mass(t[0], t[1], vbar) - 1})
    found = all(x is not None for x in sp)
    worst = {key: (max(abs(x[key]) for x in sp) if found else None) for key in ("ds", "dk", "dc", "dM")}
    return {"found": found, "missed": sum(x is None for x in sp), "worst": worst,
            "rmsd_sigma": row["rmsd"] / sigma(row["dataset"]), "species": sp}


def ranks(x):
    o = sorted(range(len(x)), key=lambda i: x[i])
    rk = [0.0] * len(x)
    i = 0
    while i < len(x):
        j = i
        while j + 1 < len(x) and x[o[j + 1]] == x[o[i]]:
            j += 1
        for q in range(i, j + 1):
            rk[o[q]] = (i + j) / 2 + 1
        i = j + 1
    return rk


def rank_effect(a, b, n=10000):
    """(mean rank of b - mean rank of a) / N, and a two-sided permutation p."""
    import random
    rng = random.Random(3)

    def stat(a, b):
        rk = ranks(a + b)
        return (sum(rk[len(a):]) / len(b) - sum(rk[:len(a)]) / len(a)) / (len(a) + len(b))
    obs = stat(a, b)
    pool = a + b
    hits = 0
    for _ in range(n):
        rng.shuffle(pool)
        hits += abs(stat(pool[:len(a)], pool[len(a):])) >= abs(obs) - 1e-12
    return obs, (hits + 1) / (n + 1)


def stage_report(args):
    res = json.loads((Path(args.work) / "ga.json").read_text())
    bks = json.loads((Path(args.work) / "buckets.json").read_text())
    for r in res:
        r["score"] = score(r)
    (Path(args.work) / "scored.json").write_text(json.dumps(res, indent=0))
    BIG = 9.99
    mets = [("RMSD/σ", lambda r: r["score"]["rmsd_sigma"]),
            ("max|Δs| %", lambda r: 100 * r["score"]["worst"]["ds"] if r["score"]["found"] else 100 * BIG),
            ("max|Δf/f0| %", lambda r: 100 * r["score"]["worst"]["dk"] if r["score"]["found"] else 100 * BIG),
            ("max|Δc| %", lambda r: 100 * r["score"]["worst"]["dc"] if r["score"]["found"] else 100 * BIG),
            ("max|ΔM| %", lambda r: 100 * r["score"]["worst"]["dM"] if r["score"]["found"] else 100 * BIG),
            ("wall s", lambda r: r["wall"])]
    lines = ["# GA main vs branch (all fixes + speedup), buckets from 2DSA-IT", "",
             "Medians over seeds; effect = rank effect of branch vs main (−0.5…+0.5, negative = smaller), "
             "p = permutation test.  Missed species count as 999 %.", ""]
    lines += ["| dataset | buckets | metric | main | branch | effect (p) |", "|---|---|---|---|---|---|"]
    for name in [d for d in DATASETS if any(r["dataset"] == d for r in res)]:
        a = [r for r in res if r["dataset"] == name and r["build"] == "main"]
        b = [r for r in res if r["dataset"] == name and r["build"] == "branch"]
        if not a or not b:
            continue
        for mname, fn in mets:
            xa, xb = [fn(r) for r in a], [fn(r) for r in b]
            eff, p = rank_effect(xa, xb)
            lines.append("| %s | %d | %s | %.3g | %.3g | %+.2f (%.3f) |" %
                         (name, len(bks[name]["buckets"]), mname, st.median(xa), st.median(xb), eff, p))
        ma = sum(r["score"]["missed"] > 0 for r in a)
        mb = sum(r["score"]["missed"] > 0 for r in b)
        lines.append("| %s | | runs with a missed species | %d/%d | %d/%d | |" % (name, ma, len(a), mb, len(b)))
        sp = st.median(r["wall"] for r in a) / st.median(r["wall"] for r in b)
        lines.append("| %s | | **wall-time speedup (median ratio)** | | **%.1f×** | |" % (name, sp))
    (Path(args.work) / "report.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


def main():
    global reg
    ap = argparse.ArgumentParser()
    ap.add_argument("stage", choices=["2dsa", "buckets", "ga", "report", "all"])
    ap.add_argument("--repo", required=True, help="branch checkout (for test/mpi_analysis)")
    ap.add_argument("--work", required=True)
    ap.add_argument("--main", help="us_mpi_analysis built from main")
    ap.add_argument("--branch", help="us_mpi_analysis built from the branch")
    ap.add_argument("--testdata", help="us_mpi_testdata")
    ap.add_argument("--dna", help="directory holding the 5-DNA run directories")
    ap.add_argument("--np", type=int, default=4, help="MPI ranks per job (demes + 1)")
    ap.add_argument("--jobs", type=int, default=1, help="concurrent jobs")
    ap.add_argument("--mpirun", default="mpirun")
    ap.add_argument("--seeds", default="1-10")
    ap.add_argument("--datasets", default=",".join(DATASETS))
    ap.add_argument("--small", action="store_true", help="smoke test (population 40, 20 generations)")
    args = ap.parse_args()
    sys.path.insert(0, str(Path(args.repo) / "test" / "mpi_analysis"))
    import us_mpi_regression as _reg
    reg = _reg
    a, _, b = args.seeds.partition("-")
    args.seeds = list(range(int(a), int(b) + 1)) if b else [int(x) for x in a.split(",")]
    args.datasets = args.datasets.split(",")
    Path(args.work).mkdir(parents=True, exist_ok=True)
    stages = ["2dsa", "buckets", "ga", "report"] if args.stage == "all" else [args.stage]
    for s in stages:
        {"2dsa": stage_2dsa, "buckets": stage_buckets, "ga": stage_ga, "report": stage_report}[s](args)


if __name__ == "__main__":
    main()
