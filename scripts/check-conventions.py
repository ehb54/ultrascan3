#!/usr/bin/env python3
"""UltraScan III conventions that clang-format cannot enforce (changed code only).

usage: check-conventions.py [--fix] (--staged | <base-ref>)

On added/changed lines of C++ sources:
  * NULL                 -> nullptr
  * f( void )            -> f()
  * SIGNAL() / SLOT()    -> connect( a, &A::sig, b, &B::slot )
On changed files:
  * file ends with a newline (fixable with --fix)
  * headers have an include guard
On NEW files:
  * first line is '//! \\file <basename>'
  * headers use the guard <BASENAME_IN_UPPERCASE>_H  (us_foo.h -> US_FOO_H)
"""
import os, re, subprocess, sys

SCOPE = ("gui/", "utils/", "programs/", "test/", "admin/")
CPP = (".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp")


def git(*a):
    return subprocess.run(("git",) + a, check=True, capture_output=True, text=True,
                          errors="replace").stdout


def strip(line):
    """Remove string literals and // comments so they are not matched."""
    line = re.sub(r'"(\\.|[^"\\])*"', '""', line)
    return re.sub(r"//.*", "", line)


def main():
    args = sys.argv[1:]
    fix = "--fix" in args
    args = [a for a in args if a != "--fix"]
    if len(args) != 1:
        sys.exit(__doc__)
    ref = ["--cached"] if args[0] == "--staged" else [args[0]]

    status = git("diff", *ref, "--name-status", "--diff-filter=AM").splitlines()
    files = {}
    for s in status:
        st, path = s.split("\t")[0], s.split("\t")[-1]
        if path.startswith(SCOPE) and path.endswith(CPP):
            files[path] = st[0]

    if not files:
        return 0
    problems = []
    cur, lineno = None, 0
    for l in git("diff", *ref, "-U0", "--no-color", "--", *files).splitlines():
        if l.startswith("+++ b/"):
            cur = l[6:]
        elif l.startswith("@@"):
            lineno = int(re.search(r"\+(\d+)", l).group(1))
        elif l.startswith("+") and cur:
            code = strip(l[1:])
            if re.search(r"\bNULL\b", code):
                problems.append((cur, lineno, "use nullptr instead of NULL"))
            if re.search(r"\w\s*\(\s*void\s*\)", code) and not re.search(r"\(\s*\*", code):
                problems.append((cur, lineno, "use f() instead of f( void )"))
            if re.search(r"\b(SIGNAL|SLOT)\s*\(", code):
                problems.append((cur, lineno, "use connect( a, &A::signal, b, &B::slot ) instead of SIGNAL()/SLOT()"))
            lineno += 1

    for path, st in files.items():
        if not os.path.exists(path):
            continue
        data = open(path, "rb").read()
        if data and not data.endswith(b"\n"):
            if fix:
                open(path, "wb").write(data.rstrip(b" \t") + b"\n")
            else:
                problems.append((path, 0, "missing newline at end of file"))
        text = data.decode("utf-8", "replace")
        base = os.path.basename(path)
        if path.endswith((".h", ".hh", ".hpp")) and not re.search(r"^#ifndef\s+\w+\s*\n#define\s+\w+", text, re.M):
            problems.append((path, 1, "header has no #ifndef/#define include guard"))
        if st == "A":
            if not text.startswith("//! \\file " + base):
                problems.append((path, 1, "first line must be '//! \\file %s'" % base))
            if path.endswith(".h"):
                guard = re.sub(r"\W", "_", base).upper()
                if not re.search(r"^#ifndef\s+%s\s*\n#define\s+%s\b" % (guard, guard), text, re.M):
                    problems.append((path, 1, "include guard must be '%s'" % guard))

    for f, n, msg in problems:
        print("%s:%s: %s" % (f, n or 1, msg))
    if problems:
        print("\nConvention violations found (see scripts/check-conventions.py).")
    return 1 if problems else 0


sys.exit(main())
