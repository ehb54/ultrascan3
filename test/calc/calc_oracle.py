#!/usr/bin/env python3
"""Independent expected values for the reporting calculations.

The oracle reads the fixture models and report items and computes what the
GMP reporter's range and replicate calculations produce for them
(programs/us_reporter_gmp/us_reporter_gmp.cpp: distrib_info near 10262,
velmwl_range_fraction at 4479, get_replicate_group_results at 10140, and the
tolerance checks at 10045, 10728 and 10946).

Model values are read as the C++ code reads them (to IEEE doubles).  Range
membership is decided in double arithmetic with the reporter's bound scaling,
because that decides which components count.  Every sum, fraction, mean and
standard deviation is then computed exactly with rational arithmetic and
rounded once, so the expected values carry no summation-order error.  The
tolerances in fixtures/expected/tolerances.json cover only the floating-point
error of the code under test.

This program does not share code with UltraScan.  It is test evidence, not a
second production implementation.

Commands:
    design   write fixtures/report_items.json from the fitted models (once)
    expect   write fixtures/expected/reporting.json
    check    recompute the expected values and compare with the stored file
    selftest run the oracle's own hand-checked cases
"""

import argparse
import json
import math
import re
import sys
from fractions import Fraction
from pathlib import Path
from xml.etree import ElementTree as ET

HERE = Path(__file__).resolve().parent
FIX = HERE / "fixtures"
MODELS = FIX / "models"
EDGE = FIX / "edge"
ITEMS = FIX / "report_items.json"
EXPECTED = FIX / "expected" / "reporting.json"

# Bound scaling of the reporter (report units to model units).  The reporter
# writes pow(10,-13), pow(10,-7) and pow(10,3); on the reference platforms
# these equal the literals below exactly.
SCALE = {"s": 1.0e-13, "D": 1.0e-7, "MW": 1.0e3, "f/f0": None}
ATTR = {"s": "s", "D": "D", "MW": "mw", "f/f0": "f_f0"}
NEAR_BOUND_REL = 1.0e-6


# --------------------------------------------------------------------------
# Model reading
# --------------------------------------------------------------------------

def read_model_elements(path):
    """Every <model> element of a model file.  A Monte Carlo file holds one
    <model> per iteration."""
    root = ET.fromstring(Path(path).read_text(encoding="utf-8"))
    out = []
    for model in root.iter("model"):
        comps = []
        for ana in model.iter("analyte"):
            comps.append({key: float(ana.get(key, "0"))
                          for key in ("s", "D", "mw", "f", "f_f0", "vbar20", "signal")})
        out.append({
            "description": model.get("description", ""),
            "monteCarlo": model.get("monteCarlo", "0"),
            "variance": float(model.get("variance", "0")),
            "components": comps,
        })
    return out


def value_range(comps, key):
    vals = [c[key] for c in comps]
    return max(vals) - min(vals)


def mc_composite(iterations):
    """The model US_Model::load builds from a multi-model file
    (US_Model::load_multi_model, utils/us_model.cpp):

    - solute points are keyed by the text "%10.4f %8.5f" of s in S and of
      f/f0, or of vbar when fewer iterations have a constant vbar
      (range < 1e-4) than a constant f/f0 (range < 1e-3);
    - components with the same key are merged; the merged concentration is
      their sum divided by the iteration count, and every other attribute
      comes from the last matching component;
    - components are ordered by the sorted key text;
    - the description becomes <first two fields>.<label>_mcN<count>.<last>.
    """
    n = len(iterations)
    ncnstv = sum(1 for it in iterations if value_range(it["components"], "vbar20") < 1.0e-4)
    ncnstk = sum(1 for it in iterations if value_range(it["components"], "f_f0") < 1.0e-3)
    cnst_vb = ncnstv >= ncnstk
    all_comps = [c for it in iterations for c in it["components"]]
    keys = ["%10.4f %8.5f" % (c["s"] * 1.0e+13, c["f_f0"] if cnst_vb else c["vbar20"])
            for c in all_comps]
    unique = sorted(set(keys))
    comps = []
    for key in unique:
        matches = [c for c, k in zip(all_comps, keys) if k == key]
        # The composite is an input to every later calculation, so it is
        # built with the legacy double operations in the legacy order.
        conc = 0.0
        for c in matches:
            conc += c["signal"]
        merged = dict(matches[-1])
        merged["signal"] = conc * (1.0 / n)
        comps.append(merged)
    first = iterations[0]["description"]
    mdsc1 = qsection(first, ".", 0, -3)
    mdsc2 = qsection(qsection(first, ".", -2, -2), "_", 0, -2)
    mdsc3 = qsection(first, ".", -1, -1)
    return {
        "description": "%s.%s_mcN%03d.%s" % (mdsc1, mdsc2, n, mdsc3),
        "monteCarlo": "1",
        "variance": iterations[0]["variance"],
        "components": comps,
    }


def read_models(path):
    """The representations of a model file:  a single model, or for a Monte
    Carlo file each iteration and the composite the reporter uses."""
    elements = read_model_elements(path)
    if len(elements) == 1:
        return [dict(elements[0], rep="model", reporter=True)]
    out = [dict(e, rep="iter%d" % (i + 1), reporter=False) for i, e in enumerate(elements)]
    out.append(dict(mc_composite(elements), rep="composite", reporter=True))
    return out


def all_fixture_models():
    models = {}
    for case_dir in sorted(p for p in MODELS.iterdir() if p.is_dir()):
        for f in sorted(case_dir.glob("*.model.xml")):
            for m in read_models(f):
                models["%s/%s#%s" % (case_dir.name, f.name, m["rep"])] = dict(m, case=case_dir.name)
    for f in sorted(EDGE.glob("*.model.xml")):
        for m in read_models(f):
            models["edge/%s#%s" % (f.name, m["rep"])] = dict(m, case="edge")
    return models


# --------------------------------------------------------------------------
# Legacy rules
# --------------------------------------------------------------------------

def qsection(text, sep, start, end):
    """QString::section(sep, start, end) for a plain separator."""
    fields = text.split(sep)
    n = len(fields)
    if start < 0:
        start += n
    if end < 0:
        end += n
    if start >= n or end < 0 or start > end:
        return ""
    return sep.join(fields[max(start, 0):end + 1])


def analysis_label(description):
    """mdla in distrib_info (us_reporter_gmp.cpp near 10297)."""
    label = qsection(qsection(description, ".", -2, -2), "_", 1, -1)
    if not label:
        label = qsection(description, ".", 0, -2)
    return label


def method_matches(item_method, description):
    """method_identity in distrib_info (near 10950):  every '-' part of the
    item method must occur in the analysis label.  Methods containing PCSA
    reduce to PCSA."""
    method = "PCSA" if "PCSA" in item_method else item_method
    label = analysis_label(description)
    return all(part in label for part in method.split("-"))


def bounds(item):
    scale = SCALE[item["type"]]
    if scale is None:
        return item["low"], item["high"]
    return item["low"] * scale, item["high"] * scale   # IEEE double products


def in_range(value, lo, hi):
    return value >= lo and value <= hi


def near_bound(value, lo, hi):
    return any(b != 0.0 and abs(value - b) <= NEAR_BOUND_REL * abs(b) for b in (lo, hi))


def F(x):
    return Fraction(x)   # exact value of the double


def to_float(q):
    return None if q is None else float(q)


def relative_pass(value, target, tol):
    """Relative tolerance (10045, 10728):  target*(1-tol/100) <= v <= target*(1+tol/100)."""
    return target * (1 - tol / 100.0) <= value <= target * (1 + tol / 100.0)


def absolute_pp_pass(value, target, tol):
    """Percentage-point tolerance (10946, 4667):  |v - target| <= tol."""
    return abs(value - target) <= tol


# --------------------------------------------------------------------------
# Calculations
# --------------------------------------------------------------------------

def whole_model(model):
    comps = model["components"]
    c_total = sum((F(c["signal"]) for c in comps), Fraction(0))
    result = {"ncomp": len(comps), "C": float(c_total)}
    for key in ("s", "D", "mw", "f_f0", "vbar20"):
        num = sum((F(c[key]) * F(c["signal"]) for c in comps), Fraction(0))
        result["mean_" + key] = to_float(num / c_total) if c_total != 0 else None
    return result


def range_result(model, item):
    comps = model["components"]
    attr = ATTR.get(item["type"])
    c_total = sum((F(c["signal"]) for c in comps), Fraction(0))
    if attr is None:
        # distrib_info matches no type branch, so nothing is selected and the
        # fraction is 0/C; velmwl_range_fraction reports the type unsupported.
        out = {
            "supported": False,
            "matched": method_matches(item["method"], model["description"]),
            "selected": 0, "R": 0.0, "C": float(c_total),
            "fraction": 0.0 if c_total != 0 else None,
            "within_mean": None, "near_bound_values": [],
        }
        add_pass(out, 0.0 if c_total != 0 else None, item)
        return out
    lo, hi = bounds(item)
    selected = [c for c in comps if in_range(c[attr], lo, hi)]
    near = [c[attr] for c in comps if near_bound(c[attr], lo, hi)]
    r = sum((F(c["signal"]) for c in selected), Fraction(0))
    num = sum((F(c[attr]) * F(c["signal"]) for c in selected), Fraction(0))
    out = {
        "supported": True,
        "matched": method_matches(item["method"], model["description"]),
        "selected": len(selected),
        "R": float(r),
        "C": float(c_total),
        # distrib_info divides without a guard; velmwl_range_fraction returns 0
        "fraction": to_float(100 * r / c_total) if c_total != 0 else None,
        "fraction_velmwl": to_float(100 * r / c_total) if c_total > 0 else 0.0,
        "within_mean": to_float(num / r) if r != 0 else None,
        "near_bound_values": near,
    }
    add_pass(out, legacy_fraction(comps, selected), item)
    return out


def legacy_fraction(comps, selected):
    """The fraction in the reporter's double arithmetic:  sums in component
    order, then (R / C) * 100 (distrib_info near 10941)."""
    c_total = 0.0
    for c in comps:
        c_total += c["signal"]
    r = 0.0
    for c in selected:
        r += c["signal"]
    return None if c_total == 0.0 else (r / c_total) * 100.0


def add_pass(out, fraction_double, item):
    """Pass/fail as the reporter reports it (decided on its double fraction),
    and the exact distance from the tolerance edge."""
    out["fraction_double"] = fraction_double
    if fraction_double is None or "target" not in item:
        return
    out["pass_pp"] = absolute_pp_pass(fraction_double, item["target"], item["tolerance"])
    exact = out["fraction"]
    out["pass_pp_margin"] = item["tolerance"] - abs(exact - item["target"])
    out["at_tolerance_edge"] = abs(out["pass_pp_margin"]) < 1e-9


def replicate_result(members, item):
    """get_replicate_group_results:  members with R >= 0, arithmetic means,
    population SD (divisor = member count)."""
    rows = [range_result(m, item) for m in members]
    eligible = [r for r in rows if r["R"] >= 0 and r["fraction"] is not None]
    n = len(eligible)
    if n == 0:
        return {"n": 0, "R_mean": 0.0, "fraction_mean": 0.0, "R_sd": 0.0, "fraction_sd": 0.0}
    rs = [F(r["R"]) for r in eligible]          # the legacy averages stored doubles
    fs = [F(r["fraction"]) for r in eligible]
    r_mean = sum(rs, Fraction(0)) / n
    f_mean = sum(fs, Fraction(0)) / n
    r_var = sum(((x - r_mean) ** 2 for x in rs), Fraction(0)) / n
    f_var = sum(((x - f_mean) ** 2 for x in fs), Fraction(0)) / n
    out = {
        "n": n,
        "R_mean": float(r_mean),
        "fraction_mean": float(f_mean),
        "R_sd": math.sqrt(float(r_var)),
        "fraction_sd": math.sqrt(float(f_var)),
    }
    if "target" in item:
        out["pass_relative"] = relative_pass(out["fraction_mean"], item["target"], item["tolerance"])
    return out


# --------------------------------------------------------------------------
# Design of report items (run once; the result is a fixture)
# --------------------------------------------------------------------------

def method_of(description):
    label = analysis_label(description)
    for token in ("2DSA-IT", "2DSA-MC", "PCSA-SL", "DMGA", "GA", "2DSA"):
        if token in label:
            return token
    return label


def clear_bound(value, values, step):
    """Move a bound until no component lies within NEAR_BOUND_REL of it."""
    b = value
    for _ in range(1000):
        if not any(abs(v - b) <= NEAR_BOUND_REL * abs(b) for v in values):
            return b
        b = round(b + step, 6)
    raise RuntimeError("no clear bound near %g" % value)


def gap_bounds(values, count):
    """count+1 bounds splitting sorted values at their largest gaps."""
    v = sorted(values)
    gaps = sorted(range(len(v) - 1), key=lambda i: v[i + 1] - v[i], reverse=True)[:count - 1]
    cuts = sorted((v[i] + v[i + 1]) / 2 for i in gaps)
    return [v[0] * 0.95] + cuts + [v[-1] * 1.05]


def design(models, manifest):
    samples = {}
    for name, info in manifest["cases"].items():
        samples.setdefault(info["sample"], {"cases": [], "species": info["species_s_ff0_c"]})
        samples[info["sample"]]["cases"].append(name)

    s_windows = {
        "A": [(2.0, 3.0), (4.0, 5.0), (6.5, 7.5), (1.0, 10.0)],
        "B": [(3.8, 4.8), (5.7, 6.7), (8.5, 9.5), (1.0, 10.0)],
        "C": [(4.7, 5.15), (5.15, 5.6), (7.5, 8.5), (1.0, 10.0)],
    }
    items = {"samples": {}, "edge": edge_items(), "replicate_groups": []}
    for sample, info in sorted(samples.items()):
        smodels = [m for m in models.values() if m["case"] in info["cases"]]
        total = sum(c for _, _, c in info["species"])
        methods = sorted({method_of(m["description"]) for m in smodels})
        sitems = []
        for lo, hi in s_windows[sample]:
            lo = clear_bound(lo, [c["s"] / 1e-13 for m in smodels for c in m["components"]], 0.001)
            hi = clear_bound(hi, [c["s"] / 1e-13 for m in smodels for c in m["components"]], 0.001)
            truth = 100.0 * sum(c for s, _, c in info["species"] if lo <= s <= hi) / total
            for method in methods:
                sitems.append({"type": "s", "method": method, "low": lo, "high": hi,
                               "target": round(truth), "tolerance": 10.0,
                               "integration": round(truth / 100.0 * total, 4)})
        main = next(m for m in smodels
                    if m["reporter"] and method_of(m["description"]) == "2DSA-IT")
        for rtype, attr, unit in (("D", "D", 1e-7), ("MW", "mw", 1e3), ("f/f0", "f_f0", 1.0)):
            vals = [c[attr] / unit for m in smodels for c in m["components"]]
            cuts = gap_bounds([c[attr] / unit for c in main["components"]], 3)
            cuts = [clear_bound(float("%.4g" % b), vals, 10 ** (math.floor(math.log10(abs(b))) - 3))
                    for b in cuts]
            for lo, hi in zip(cuts, cuts[1:]):
                frac = range_result(main, {"type": rtype, "method": "2DSA-IT", "low": lo, "high": hi})["fraction"]
                sitems.append({"type": rtype, "method": "2DSA-IT", "low": lo, "high": hi,
                               "target": round(frac), "tolerance": 5.0, "integration": 0.0})
        # A failing check and a method no model of the sample matches
        first = sitems[0]
        sitems.append(dict(first, target=min(100, first["target"] + 30), note="expected to fail"))
        sitems.append(dict(first, method="PCSA-SL" if "PCSA-SL" not in methods else "DMGA-MC",
                           note="no matching model for 2DSA models"))
        if sample == "A":
            # The report item seeded in the LIMS validation data (OQ protocol)
            sitems.append({"type": "s", "method": "2DSA-IT", "low": 6.0, "high": 7.5,
                           "target": 76.0, "tolerance": 10.0, "integration": 0.535,
                           "note": "LIMS seed 27-validation-oq-protocol.sql item"})
        items["samples"][sample] = {"cases": sorted(info["cases"]), "items": sitems,
                                    "total_concentration": {"target": total, "tolerance": 10.0}}
    a_reps = sorted(c for c in samples["A"]["cases"] if c.endswith("2dsa_it"))
    for item in items["samples"]["A"]["items"]:
        if item["type"] == "s" and item["method"] == "2DSA-IT" and "note" not in item:
            items["replicate_groups"].append({"cases": a_reps, "item": item})
    return items


def edge_items():
    """Items for the hand-made edge models (fixtures/edge)."""
    return [
        {"type": "s", "method": "2DSA", "low": 4.0, "high": 5.0, "target": 50.0, "tolerance": 10.0},
        {"type": "s", "method": "2DSA", "low": 4.5, "high": 4.5, "target": 0.0, "tolerance": 1.0},
        {"type": "D", "method": "2DSA", "low": 4.0, "high": 6.0, "target": 50.0, "tolerance": 10.0},
        {"type": "MW", "method": "2DSA", "low": 50.0, "high": 100.0, "target": 50.0, "tolerance": 10.0},
        {"type": "f/f0", "method": "2DSA", "low": 1.5, "high": 2.0, "target": 50.0, "tolerance": 10.0},
        {"type": "vbar", "method": "2DSA", "low": 0.7, "high": 0.75, "target": 50.0, "tolerance": 10.0},
    ]


# --------------------------------------------------------------------------
# Expected values
# --------------------------------------------------------------------------

def compute_expected(models, items):
    out = {"schema": 1, "models": {}, "ranges": [], "replicates": [], "total_concentration": []}
    for key, model in sorted(models.items()):
        out["models"][key] = dict(whole_model(model),
                                  description=model["description"],
                                  label=analysis_label(model["description"]))
    for sample, block in sorted(items["samples"].items()):
        for key, model in sorted(models.items()):
            if model["case"] not in block["cases"]:
                continue
            for index, item in enumerate(block["items"]):
                res = range_result(model, item)
                out["ranges"].append({"model": key, "sample": sample, "item": index, **res})
            c_total = whole_model(model)["C"]
            tc = block["total_concentration"]
            out["total_concentration"].append({
                "model": key, "C": c_total, "target": tc["target"], "tolerance": tc["tolerance"],
                "pass_relative": relative_pass(c_total, tc["target"], tc["tolerance"])})
    for key, model in sorted(models.items()):
        if model["case"] != "edge":
            continue
        for index, item in enumerate(items["edge"]):
            out["ranges"].append({"model": key, "sample": "edge", "item": index,
                                  **range_result(model, item)})
    for gi, group in enumerate(items["replicate_groups"]):
        members = [m for k, m in sorted(models.items())
                   if m["case"] in group["cases"] and m["reporter"]]
        out["replicates"].append({"group": gi, "cases": group["cases"],
                                  **replicate_result(members, group["item"])})
    return out


def check_design(expected):
    """Fitted-model ranges must not have components within NEAR_BOUND_REL of
    a bound (the edge fixtures test exact bounds on purpose)."""
    bad = [r for r in expected["ranges"]
           if r["sample"] != "edge" and r.get("near_bound_values")]
    if bad:
        raise SystemExit("components near a range bound: %s" % bad[:3])
    near_pass = [r for r in expected["ranges"]
                 if r["sample"] != "edge" and "pass_pp_margin" in r
                 and abs(r["pass_pp_margin"]) < 1e-6]
    if near_pass:
        raise SystemExit("pass/fail within 1e-6 of the tolerance: %s" % near_pass[:3])


# --------------------------------------------------------------------------
# Self test with hand-checked values
# --------------------------------------------------------------------------

def selftest():
    def comp(s, c, D=4.0e-7, mw=60000.0, ff0=1.5, v=0.72):
        return {"s": s, "D": D, "mw": mw, "f": 0.0, "f_f0": ff0, "vbar20": v, "signal": c}
    m = {"description": "run.1A260.e1_a2611090000_2DSA-IT_01.model",
         "components": [comp(4.0e-13, 0.25), comp(4.5e-13, 0.25), comp(6.0e-13, 0.5)]}
    r = range_result(m, {"type": "s", "method": "2DSA-IT", "low": 4.0, "high": 5.0,
                         "target": 50.0, "tolerance": 10.0})
    assert r["selected"] == 2 and r["R"] == 0.5 and r["C"] == 1.0, r
    assert r["fraction"] == 50.0 and r["pass_pp"], r
    assert abs(r["within_mean"] - 4.25e-13) < 1e-27, r
    assert method_matches("2DSA-IT", m["description"])
    assert not method_matches("2DSA-MC", m["description"])
    assert method_matches("PCSA-SL", "x.1A260.e_a2611090000_PCSA-SL_01.model")
    assert analysis_label("a.b.c_d_e.model") == "d_e"
    assert analysis_label("plain.model") == "plain"
    assert not absolute_pp_pass(79.9, 90.0, 10.0) and absolute_pp_pass(80.0, 90.0, 10.0)
    assert not relative_pass(80.9, 90.0, 10.0) and relative_pass(81.0, 90.0, 10.0)
    zero = {"description": "", "components": [comp(4.5e-13, 0.0)]}
    z = range_result(zero, {"type": "s", "method": "2DSA", "low": 4.0, "high": 5.0})
    assert z["fraction"] is None and z["fraction_velmwl"] == 0.0, z
    reps = [{"description": "x", "components": [comp(4.5e-13, c), comp(9e-13, 1.0 - c)]}
            for c in (0.2, 0.4, 0.6)]
    g = replicate_result(reps, {"type": "s", "method": "x", "low": 4.0, "high": 5.0,
                                "target": 40.0, "tolerance": 10.0})
    assert g["n"] == 3 and abs(g["fraction_mean"] - 40.0) < 1e-12, g
    assert abs(g["fraction_sd"] - math.sqrt(800.0 / 3.0)) < 1e-12, g
    print("selftest: ok")


# --------------------------------------------------------------------------

def dump(obj, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(obj, indent=1, sort_keys=True) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["design", "expect", "check", "selftest"])
    args = parser.parse_args()
    if args.command == "selftest":
        selftest()
        return
    models = all_fixture_models()
    if args.command == "design":
        manifest = json.loads((MODELS / "MANIFEST.json").read_text())
        dump(design(models, manifest), ITEMS)
        print("wrote %s" % ITEMS)
        return
    items = json.loads(ITEMS.read_text())
    expected = compute_expected(models, items)
    check_design(expected)
    if args.command == "expect":
        dump(expected, EXPECTED)
        print("wrote %s (%d range results, %d replicate groups)"
              % (EXPECTED, len(expected["ranges"]), len(expected["replicates"])))
        return
    stored = json.loads(EXPECTED.read_text())
    if stored != json.loads(json.dumps(expected, sort_keys=True)):
        sys.exit("fixtures/expected/reporting.json is out of date; run calc_oracle.py expect")
    print("check: ok (%d range results)" % len(expected["ranges"]))


if __name__ == "__main__":
    main()
