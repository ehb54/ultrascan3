#!/usr/bin/env python3
"""Check that each program source is compiled by one program directory only.

usage: check-duplicate-sources.py [--root <dir>]

A program's CMakeLists.txt must not list a .cpp outside its own directory. A
source that several programs need belongs in an object library, "<dir>_objects",
that the directory owning the source defines and the other programs link. See
programs/us_edit/CMakeLists.txt. programs/CMakeLists.txt, which owns the sources
directly in programs/, may list those and nothing below or above it.

The CMakeLists.txt files are read as text, so no configured build is needed.
Comments are ignored. A path is outside the directory when, with the current
source and list directory variables standing for the directory itself, it climbs
out with "../", or when it starts at the top of the source tree or is absolute.
A variable set in the same file to one path counts as that path.
<dir> defaults to the repository containing this script.
Exits 0 when every source is inside, 1 with one line per violation otherwise.
"""
import argparse
import os
import posixpath
import re
import sys

HERE = ("CMAKE_CURRENT_SOURCE_DIR", "CMAKE_CURRENT_LIST_DIR")
# Variables for the top of the source tree, relative to programs/<dir>.
TOPS = {
    "CMAKE_SOURCE_DIR": "../..",
    "PROJECT_SOURCE_DIR": "../..",
    "ultrascan3_SOURCE_DIR": "../..",
}
TOKEN = re.compile(r'[^\s()";]+')
SET = re.compile(r'\bset\s*\(\s*([A-Za-z0-9_]+)\s+"?([^\s()";]+)"?\s*\)', re.I)


def strip_comments(text):
    """Blank out comments, keeping line numbers."""
    text = re.sub(r"#\[(=*)\[.*?\]\1\]", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return re.sub(r"#.*", "", text)


def outside(token, variables):
    """Whether the path climbs out of the directory of the CMakeLists.txt."""
    for _ in range(10):
        expanded = re.sub(r"\$\{(\w+)\}", lambda m: variables.get(m.group(1), m.group(0)), token)
        if expanded == token:
            break
        token = expanded
    path = posixpath.normpath(token)
    return path == ".." or path.startswith("../") or path.startswith("/")


def check_file(root, path, allowed):
    """The violations of one CMakeLists.txt; allowed says which paths may stay."""
    with open(os.path.join(root, path), encoding="utf-8", errors="replace") as handle:
        text = strip_comments(handle.read())
    variables = {name: "." for name in HERE}
    variables.update(TOPS)
    for match in SET.finditer(text):
        variables[match.group(1)] = match.group(2)
    problems = []
    for number, line in enumerate(text.split("\n"), 1):
        for token in TOKEN.findall(line):
            if token.endswith(".cpp") and not allowed(token, variables):
                problems.append("%s:%d: lists %s, a source outside its directory" % (path, number, token))
    return problems


def check(root):
    problems = []
    top = os.path.join(root, "programs")
    for directory in sorted(os.listdir(top)):
        path = "programs/%s/CMakeLists.txt" % directory
        if os.path.isfile(os.path.join(root, path)):
            problems += check_file(root, path, lambda token, variables: not outside(token, variables))
    if os.path.isfile(os.path.join(top, "CMakeLists.txt")):
        problems += check_file(
            root,
            "programs/CMakeLists.txt",
            lambda token, variables: not outside(token, variables) and "/" not in re.sub(r"^(\$\{\w+\}|\.)/", "", token),
        )
    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    args = parser.parse_args()

    if not os.path.isdir(os.path.join(args.root, "programs")):
        print("error: %s has no programs/ directory" % args.root, file=sys.stderr)
        return 2

    problems = check(args.root)
    for problem in problems:
        print(problem)
    if problems:
        print("%d source(s) listed outside their directory" % len(problems), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
