#!/usr/bin/env python3
"""Run each load in its own process and compare it with the recorded profile."""
import argparse
import difflib
import json
import os
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("executable")
parser.add_argument("--record", action="store_true")
parser.add_argument("--case", action="append")
args = parser.parse_args()
fixtures = Path(__file__).parent / "fixtures"
listing = subprocess.run([args.executable, "-datatags"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=True)
cases = args.case or [line.rsplit(" ", 1)[1] for line in listing.stdout.decode().splitlines() if " record " in line]
if not cases:
    raise SystemExit("no cases listed")
failed = 0
for case in cases:
    with tempfile.TemporaryDirectory(prefix="profile-1329-") as tmp:
        output = Path(tmp) / "profile.json"
        env = dict(os.environ, US3_PROFILE_SNAPSHOT=str(output), QT_LOGGING_RULES="*.debug=false")
        proc = subprocess.run([args.executable, "record:" + case, "-nocrashhandler"],
                              env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=45)
        log = proc.stdout.decode(errors="replace")
        if "SKIP" in log and proc.returncode == 0:
            print(f"SKIP {case}: {log}")
            continue
        if proc.returncode != 0:
            print(f"FAIL {case}: {log}")
            failed += 1
            continue
        else:
            actual = json.loads(output.read_text())
        path = fixtures / (case + ".json")
        if args.record:
            path.write_text(json.dumps(actual, separators=(",", ":"), sort_keys=True) + "\n")
            print(f"RECORDED {case}")
        elif not path.exists() or json.loads(path.read_text()) != actual:
            print(f"FAIL {case}")
            # The records are one line each; compare them indented
            recorded = json.loads(path.read_text()) if path.exists() else {}
            lines = [json.dumps(v, indent=2, sort_keys=True).splitlines() for v in (recorded, actual)]
            print("\n".join(difflib.unified_diff(*lines, fromfile=str(path), tofile="actual", lineterm="")))
            failed += 1
        else:
            print(f"PASS {case}")
raise SystemExit(bool(failed))
