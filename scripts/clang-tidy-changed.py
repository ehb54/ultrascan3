#!/usr/bin/env python3
"""Runs clang-tidy only on the lines changed since <base-ref> (no dependency on clang-tidy-diff.py).

usage: clang-tidy-changed.py [--fix] <base-ref> <compile-db-dir>
Raw clang-tidy output goes to stdout.
"""
import json, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

SCOPE = ("gui/", "utils/", "programs/", "test/")
EXT = (".cpp", ".cc", ".cxx", ".h", ".hpp")


def main():
    args = sys.argv[1:]
    fix = "--fix" in args
    args = [a for a in args if a != "--fix"]
    base, db = args
    tidy = os.environ.get("CLANG_TIDY", "clang-tidy")

    diff = subprocess.run(["git", "diff", "-U0", "--no-color", "--diff-filter=AM", base], check=True,
                          capture_output=True, text=True, errors="replace").stdout
    ranges, cur = {}, None
    for l in diff.splitlines():
        if l.startswith("+++ b/"):
            cur = l[6:]
        elif l.startswith("+++ "):
            cur = None
        elif l.startswith("@@") and cur and cur.startswith(SCOPE) and cur.endswith(EXT):
            m = re.search(r"\+(\d+)(?:,(\d+))?", l)
            start, n = int(m.group(1)), int(m.group(2) or 1)
            if n:
                ranges.setdefault(cur, []).append([start, start + n - 1])

    # translation units from the compile database; headers are analysed through the TUs that include them
    db_files = {os.path.relpath(e["file"], os.getcwd()) if os.path.isabs(e["file"]) else e["file"]
                for e in json.load(open(os.path.join(db, "compile_commands.json")))}
    units = sorted(f for f in ranges if f in db_files and f.endswith((".cpp", ".cc", ".cxx")))
    if any(f.endswith((".h", ".hpp")) for f in ranges):
        # one including TU per changed header is enough; pick TUs in the same directory, else skip
        for h in (f for f in ranges if f.endswith((".h", ".hpp"))):
            tus = sorted(t for t in db_files if os.path.dirname(t) == os.path.dirname(h))
            if tus and not any(t in units for t in tus):
                units.append(tus[0])
    if not units:
        return 0

    line_filter = json.dumps([{"name": f, "lines": r} for f, r in ranges.items()])

    def run(unit):
        cmd = [tidy, "-p", db, "--quiet", "--line-filter=" + line_filter] + (["--fix"] if fix else []) + [unit]
        r = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
        return r.stdout + r.stderr

    # --fix: run serially so that two TUs never rewrite the same header concurrently
    with ThreadPoolExecutor(max_workers=1 if fix else (os.cpu_count() or 2)) as ex:
        for out in ex.map(run, units):
            sys.stdout.write(out)
    return 0


sys.exit(main())
