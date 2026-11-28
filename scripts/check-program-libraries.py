#!/usr/bin/env python3
"""Check the area libraries of program sources that several programs use.

usage: check-program-libraries.py [--root <dir>] [--affected-programs <path> ...]

A source that several programs use is in the library of its area,
us_programs_<area>, and is compiled once. CMake declares the libraries in
programs/CMakeLists.txt, and each program directory compiles its own sources as
a library, <dir>_lib, that it adds to one; qmake has a project for each in
programlibs/<area>.

  * A program's CMakeLists.txt or .pro file must not list a .cpp outside its own
    directory. programs/CMakeLists.txt may list the sources directly in programs/.
  * Each library must have the same sources and need the same libraries in CMake
    and in qmake, and a program directory must link the same libraries in both.
  * A program directory must not include a header of an area whose library it
    neither belongs to nor links, directly or through another library. This
    checks relative cross-directory includes ("../us_x/header.h") only; bare
    includes supplied by CMake PUBLIC include paths are not checked.

The build files are read as text, so no configured build is needed. Comments are
ignored. <dir> defaults to the repository containing this script. Exits 0 when
the rules hold, 1 with one line per violation otherwise. --affected-programs
instead prints program directories that need rebuilding, including transitive
area dependents, using programlibs/link.pri and the qmake projects.
"""
import argparse
import os
import posixpath
import re
import sys

HERE = ("CMAKE_CURRENT_SOURCE_DIR", "CMAKE_CURRENT_LIST_DIR")
# Variables for the top of the source tree, relative to programs/<dir>.
TOPS = ("CMAKE_SOURCE_DIR", "PROJECT_SOURCE_DIR", "ultrascan3_SOURCE_DIR")
TOKEN = re.compile(r'[^\s()";]+')
SET = re.compile(r"\bset\s*\(\s*(\w+)\s+([^()]*)\)", re.I)
AREA = re.compile(r"\bus_programs_(\w+)\b")
INCLUDE = re.compile(r'^\s*#\s*include\s*"(?:\.\./)+(?:programs/)?(\w+)/[^"]+"', re.M)
CODE = (".cpp", ".h")


def read(root, path):
    full = os.path.join(root, path)
    if not os.path.isfile(full):
        return ""
    with open(full, encoding="utf-8", errors="replace") as handle:
        return handle.read()


def read_cmake(root, path):
    """The text with comments blanked out, keeping line numbers."""
    text = re.sub(r"#\[(=*)\[.*?\]\1\]", lambda m: "\n" * m.group(0).count("\n"), read(root, path), flags=re.S)
    return re.sub(r"#.*", "", text)


def read_qmake(root, path):
    return re.sub(r"#.*", "", read(root, path))


def qmake_list(text, name):
    """The words assigned or added to a qmake variable."""
    words = []
    for match in re.finditer(r"^[ \t]*%s\s*\+?=((?:.*\\[ \t]*\n)*.*)" % name, text, re.M):
        words += match.group(1).replace("\\", " ").split()
    return words


def expand(token, variables):
    for _ in range(10):
        expanded = re.sub(r"\$\{(\w+)\}", lambda m: variables.get(m.group(1), m.group(0)), token)
        if expanded == token:
            break
        token = expanded
    return token


def normal(token, variables):
    """The path, relative to the directory of the CMakeLists.txt when possible."""
    return posixpath.normpath(expand(token, variables))


def cmake_variables(text):
    variables = {name: "." for name in HERE}
    variables.update({name: "../.." for name in TOPS})
    for match in SET.finditer(text):
        variables.setdefault(match.group(1), match.group(2).strip().strip('"'))
    return variables


def is_outside(path):
    return path == ".." or path.startswith("../") or path.startswith("/")


def call_arguments(text, command):
    return [match.group(1).split() for match in re.finditer(r"\b%s\s*\(([^()]*)\)" % command, text)]


def closure(needs, areas):
    """The areas and all they need."""
    found, pending = set(), list(areas)
    while pending:
        area = pending.pop()
        if area not in found:
            found.add(area)
            pending += needs.get(area, ())
    return found


class Build:
    """The area libraries as one of the build systems declares them."""

    def __init__(self):
        self.needs = {}  # area -> areas it needs directly
        self.sources = {}  # area -> program sources, relative to programs/
        self.headers = {}  # area -> qmake headers, relative to programs/
        self.links = {}  # program directory -> areas it links


def cmake_build(root, directories):
    build = Build()
    for arguments in call_arguments(read_cmake(root, "programs/CMakeLists.txt"), "us3_add_program_area"):
        areas = [m.group(1) for m in map(AREA.fullmatch, arguments) if m]
        if areas:
            build.needs[areas[0]] = set(areas[1:])
            build.sources[areas[0]] = set()
    for directory in [""] + directories:
        text = read_cmake(root, posixpath.join("programs", directory, "CMakeLists.txt"))
        variables = cmake_variables(text)
        libraries = {}
        for arguments in call_arguments(text, "add_library"):
            if arguments[1:2] == ["STATIC"]:
                words = [word for token in arguments[2:] for word in expand(token, variables).split()]
                libraries[arguments[0]] = {
                    posixpath.normpath(posixpath.join(directory, normal(word, variables))) for word in words if word.endswith(".cpp")
                }
        for arguments in call_arguments(text, "target_link_libraries"):
            area = AREA.fullmatch(arguments[0]) if arguments else None
            for argument in arguments[1:]:
                linked = AREA.fullmatch(argument)
                if area:
                    build.sources.setdefault(area.group(1), set()).update(libraries.get(argument, ()))
                elif linked and directory:
                    build.links.setdefault(directory, set()).add(linked.group(1))
    return build


def pro_files(root, top):
    folder = os.path.join(root, top)
    return [posixpath.join(top, f) for f in sorted(os.listdir(folder)) if f.endswith(".pro")] if os.path.isdir(folder) else []


def qmake_build(root, directories):
    build = Build()
    top = os.path.join(root, "programlibs")
    for area in sorted(os.listdir(top)) if os.path.isdir(top) else []:
        for path in pro_files(root, "programlibs/" + area):
            text = read_qmake(root, path)
            folder = posixpath.join("programlibs", area)
            build.sources[area] = {posixpath.relpath(posixpath.join(folder, s), "programs") for s in qmake_list(text, "SOURCES")}
            build.headers[area] = {posixpath.relpath(posixpath.join(folder, h), "programs") for h in qmake_list(text, "HEADERS")}
            build.needs.setdefault(area, set())
    for match in re.finditer(
        r"contains\(\s*PROGRAM_LIBS\s*,\s*(\w+)\s*\)\s*:\s*PROGRAM_LIBS\s*\+=(.*)", read_qmake(root, "programlibs/link.pri")
    ):
        build.needs.setdefault(match.group(1), set()).update(match.group(2).split())
    for directory in directories:
        for path in pro_files(root, "programs/" + directory):
            linked = qmake_list(read_qmake(root, path), "PROGRAM_LIBS")
            if linked:
                build.links.setdefault(directory, set()).update(linked)
    return build


def check_outside_sources(root, directories):
    problems = []
    for directory in [""] + directories:
        path = posixpath.join("programs", directory, "CMakeLists.txt")
        text = read_cmake(root, path)
        variables = cmake_variables(text)
        for number, line in enumerate(text.split("\n"), 1):
            for token in TOKEN.findall(line):
                target = normal(token, variables) if token.endswith(".cpp") else ""
                if is_outside(target) or (target and not directory and "/" in target):
                    problems.append("%s:%d: lists %s, a source outside its directory" % (path, number, token))
        for path in pro_files(root, posixpath.join("programs", directory)) if directory else []:
            for token in qmake_list(read_qmake(root, path), "SOURCES"):
                if is_outside(posixpath.normpath(token.replace("$$PWD", "."))):
                    problems.append("%s: lists %s, a source outside its directory" % (path, token))
    return problems


def check_same(cmake, qmake, directories_with_both):
    problems = []
    for area in sorted(set(cmake.sources) | set(qmake.sources)):
        for path in sorted(cmake.sources.get(area, set()) - qmake.sources.get(area, set())):
            problems.append("us_programs_%s: programs/%s is in the CMake library but not in the qmake one" % (area, path))
        for path in sorted(qmake.sources.get(area, set()) - cmake.sources.get(area, set())):
            problems.append("us_programs_%s: programs/%s is in the qmake library but not in the CMake one" % (area, path))
    for area in sorted(set(cmake.needs) | set(qmake.needs)):
        ours, theirs = closure(cmake.needs, [area]), closure(qmake.needs, [area])
        if ours != theirs:
            problems.append(
                "us_programs_%s: needs %s in CMake but %s in qmake" % (area, sorted(ours - {area}), sorted(theirs - {area}))
            )
    for directory in directories_with_both:
        ours, theirs = cmake.links.get(directory, set()), qmake.links.get(directory, set())
        if ours != theirs:
            problems.append("programs/%s: links %s in CMake but %s in qmake" % (directory, sorted(ours), sorted(theirs)))
    return problems


def check_includes(root, directories, build):
    owners = {}
    for area, paths in build.sources.items():
        for path in paths:
            owners.setdefault(posixpath.dirname(path), set()).add(area)
    problems = []
    for directory in directories:
        allowed = closure(build.needs, owners.get(directory, set()) | build.links.get(directory, set()))
        folder = os.path.join(root, "programs", directory)
        for name in sorted(os.listdir(folder)):
            if not name.endswith(CODE):
                continue
            text = read(root, posixpath.join("programs", directory, name))
            text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
            text = re.sub(r"//.*", "", text)
            for other in sorted(set(INCLUDE.findall(text))):
                missing = owners.get(other, set()) - allowed
                if other != directory and missing:
                    problems.append(
                        "programs/%s/%s: includes a header of %s, but does not link us_programs_%s"
                        % (directory, name, other, ", ".join(sorted(missing)))
                    )
    return problems


def affected_programs(root, changed):
    """Programs affected by changed paths under programs/."""
    top = os.path.join(root, "programs")
    directories = [d for d in sorted(os.listdir(top)) if os.path.isdir(os.path.join(top, d))]
    build = qmake_build(root, directories)
    changed = {posixpath.normpath(path).removeprefix("programs/") for path in changed}
    changed_dirs = {path.split("/", 1)[0] for path in changed} & set(directories)
    areas = set()
    for area, sources in build.sources.items():
        files = sources | build.headers[area]
        if files & changed or any(path.split("/", 1)[0] in changed_dirs for path in files):
            areas.add(area)
    return ["programs/" + directory for directory in directories
            if directory in changed_dirs or closure(build.needs, build.links.get(directory, ())) & areas]


def check(root):
    top = os.path.join(root, "programs")
    directories = [d for d in sorted(os.listdir(top)) if os.path.isdir(os.path.join(top, d))]
    both = [
        d for d in directories if pro_files(root, "programs/" + d) and os.path.isfile(os.path.join(top, d, "CMakeLists.txt"))
    ]
    cmake, qmake = cmake_build(root, directories), qmake_build(root, directories)
    return check_outside_sources(root, directories) + check_same(cmake, qmake, both) + check_includes(root, directories, cmake)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    parser.add_argument("--affected-programs", nargs="+", metavar="PATH")
    args = parser.parse_args()

    if not os.path.isdir(os.path.join(args.root, "programs")):
        print("error: %s has no programs/ directory" % args.root, file=sys.stderr)
        return 2

    if args.affected_programs:
        for directory in affected_programs(args.root, args.affected_programs):
            print(directory)
        return 0

    problems = check(args.root)
    for problem in problems:
        print(problem)
    if problems:
        print("%d program library violation(s)" % len(problems), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
