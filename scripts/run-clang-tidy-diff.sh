#!/usr/bin/env bash
# Runs clang-tidy (config: .clang-tidy) on the CHANGED LINES since <base-ref> only.
#
# usage: scripts/run-clang-tidy-diff.sh [--fix] <base-ref> [compile-db-dir]
#   --fix   apply the auto-fixable style fixes to the worktree (modernize-*)
#
# Output: tidy.log (raw) and GitHub annotations on stdout (security findings first as errors,
# style findings as warnings). Exit code 1 if there are security findings and
# TIDY_SECURITY_BLOCKING=true, otherwise 0.
set -uo pipefail

fix=()
[ "${1:-}" = "--fix" ] && { fix=(--fix); shift; }
base=${1:?usage: $0 [--fix] <base-ref> [compile-db-dir]}
db=${2:-.}

cd "$(git rev-parse --show-toplevel)"
[ -f "$db/compile_commands.json" ] || { echo "error: $db/compile_commands.json not found" >&2; exit 2; }

python3 scripts/clang-tidy-changed.py ${fix[@]+"${fix[@]}"} "$base" "$db" > tidy.log 2>&1 || true

python3 - "$PWD" <<'PY'
import re, sys
root = sys.argv[1].rstrip("/") + "/"
pat = re.compile(r"^(?P<file>/?[^\s:]+):(?P<line>\d+):(?P<col>\d+): (?P<sev>warning|error): (?P<msg>.*) \[(?P<check>[^\]]+)\]$")
seen, sec, style = set(), [], []
for l in open("tidy.log", errors="replace"):
    m = pat.match(l.rstrip("\n"))
    if not m:
        continue
    f = m["file"].replace(root, "")
    key = (f, m["line"], m["check"])
    if key in seen or not f.startswith(("gui/", "utils/", "programs/", "test/")):
        continue
    seen.add(key)
    is_sec = m["check"].startswith(("clang-analyzer-", "bugprone-"))
    (sec if is_sec else style).append((f, m["line"], m["col"], m["check"], m["msg"]))
# security first
for f, ln, c, chk, msg in sec:
    print("::error file=%s,line=%s,col=%s,title=clang-tidy %s::%s" % (f, ln, c, chk.split(",")[0], msg))
for f, ln, c, chk, msg in style:
    print("::warning file=%s,line=%s,col=%s,title=clang-tidy %s::%s" % (f, ln, c, chk.split(",")[0], msg))
print("clang-tidy: %d security finding(s), %d style finding(s) on changed lines" % (len(sec), len(style)))
open("tidy.security.count", "w").write(str(len(sec)))
PY
[ "${TIDY_SECURITY_BLOCKING:-false}" = true ] && [ "$(cat tidy.security.count)" != 0 ] && exit 1
exit 0
