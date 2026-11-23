#!/usr/bin/env python3
"""Check that the libraries and programs do not depend on each other in a cycle.

usage: check-dependency-layers.py [--root <dir>]

The libraries are layered: utils (us_utils) is the base, gui (us_gui) builds on
utils, and the programs build on both. A library must never use code from a
layer above it, or the programs and libraries become circular again.

  * A gui/ or utils/ source must not include a path into programs/, or a
    "../" path that leads out of its library. A "../" path that stays inside
    the library, such as gui/moc/moc_us_widget.cpp including "../us_widget.h",
    is allowed.
  * A gui/ or utils/ source must not include a header that exists only under
    programs/ (it would compile only where a program's directory is on the
    include path).
  * A utils/ source must not include a header that exists only in gui/.
  * The libraries' build files (CMakeLists.txt, libus_*.pro and library.pri)
    must not name programs/, and utils' must not link the GUI library.
  * The program directories must not depend on each other in a cycle. One
    directory depends on another when it includes a header from it (by a
    "../dir/" path, or by a name that only that directory has) or lists one of
    its sources in its CMakeLists.txt or .pro file. Files directly in
    programs/ count as one directory.

Commented-out lines are ignored. <dir> defaults to the repository containing
this script; the tests point it at small trees that break each rule.
Exits 0 when the boundaries hold, 1 with one line per violation otherwise.
"""
import argparse
import os
import re
import sys

SOURCES = (".h", ".hh", ".hpp", ".c", ".cc", ".cpp", ".cxx")
HEADERS = (".h", ".hh", ".hpp")
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')

LIBRARIES = ("utils", "gui")
BUILD_FILES = {
    "utils": ("utils/CMakeLists.txt", "utils/libus_utils.pro", "library.pri"),
    "gui": ("gui/CMakeLists.txt", "gui/libus_gui.pro", "library.pri"),
}
# Ways the utils build files could name the GUI library or one of its variants
# (us_gui, us_gui_static, UltraScan3::Gui, -lus_gui, ...).
GUI_LINK = re.compile(r"\bus_gui\w*|-lus_gui\w*|\bUltraScan3::Gui\w*|\bGui_Static\b")


def files_under(root, top, suffixes):
    """Relative paths of files under root/top with one of the suffixes."""
    found = []
    for dirpath, dirnames, filenames in os.walk(os.path.join(root, top)):
        dirnames.sort()
        for name in sorted(filenames):
            if name.endswith(suffixes):
                found.append(os.path.relpath(os.path.join(dirpath, name), root))
    return found


def header_names(root, top):
    return {os.path.basename(path) for path in files_under(root, top, HEADERS)}


def strip_cpp_comments(text):
    """Blank out comments, keeping line numbers."""
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return [re.sub(r"//.*", "", line) for line in text.split("\n")]


def strip_build_comments(text):
    return [re.sub(r"#.*", "", line) for line in text.split("\n")]


def read(root, path):
    with open(os.path.join(root, path), encoding="utf-8", errors="replace") as handle:
        return handle.read()


def program_dirs(root):
    top = os.path.join(root, "programs")
    if not os.path.isdir(top):
        return []
    return sorted(d for d in os.listdir(top) if os.path.isdir(os.path.join(top, d)))


def program_files(root, directory):
    """The files of one program directory; "." is the files directly in programs/."""
    top = os.path.join(root, "programs")
    if directory == ".":
        return [os.path.join("programs", f) for f in sorted(os.listdir(top)) if os.path.isfile(os.path.join(top, f))]
    return files_under(root, os.path.join("programs", directory), SOURCES + (".txt", ".pro", ".pri"))


def program_graph(root, library_headers):
    """Map each program directory to the program directories it depends on."""
    directories = ["."] + program_dirs(root)
    owners = {}
    for directory in directories:
        for path in program_files(root, directory):
            if path.endswith(HEADERS):
                owners.setdefault(os.path.basename(path), set()).add(directory)

    listed = re.compile(r"\.\./([A-Za-z0-9_]+)/[A-Za-z0-9_]+\.(?:cpp|h)\b")
    graph = {directory: set() for directory in directories}
    for directory in directories:
        for path in program_files(root, directory):
            name = os.path.basename(path)
            if path.endswith(SOURCES):
                for line in strip_cpp_comments(read(root, path)):
                    match = INCLUDE.match(line)
                    if not match:
                        continue
                    target = match.group(1)
                    climb = re.match(r"(?:\.\./)+([A-Za-z0-9_]+)/", target)
                    if climb and climb.group(1) in graph:
                        graph[directory].add(climb.group(1))
                    elif "/" not in target and target not in library_headers:
                        graph[directory] |= owners.get(target, set()) - {directory}
            elif name == "CMakeLists.txt" or name.endswith((".pro", ".pri")):
                for line in strip_build_comments(read(root, path)):
                    for match in listed.finditer(line):
                        if match.group(1) in graph:
                            graph[directory].add(match.group(1))
        graph[directory].discard(directory)
    return graph


def cycles(graph):
    """Strongly connected groups of more than one node (Tarjan)."""
    index, low, stack, on_stack, groups = {}, {}, [], set(), []

    def visit(node):
        index[node] = low[node] = len(index)
        stack.append(node)
        on_stack.add(node)
        for other in sorted(graph.get(node, ())):
            if other not in index:
                visit(other)
                low[node] = min(low[node], low[other])
            elif other in on_stack:
                low[node] = min(low[node], index[other])
        if low[node] == index[node]:
            group = []
            while True:
                other = stack.pop()
                on_stack.discard(other)
                group.append(other)
                if other == node:
                    break
            if len(group) > 1:
                groups.append(sorted(group))

    sys.setrecursionlimit(max(1000, 4 * len(graph)))
    for node in sorted(graph):
        if node not in index:
            visit(node)
    return groups


def check(root):
    """Return the list of boundary violations under root."""
    program_only = header_names(root, "programs") - header_names(root, "utils") - header_names(root, "gui")
    gui_only = header_names(root, "gui") - header_names(root, "utils")
    problems = []

    for library in LIBRARIES:
        for path in files_under(root, library, SOURCES):
            for number, line in enumerate(strip_cpp_comments(read(root, path)), 1):
                match = INCLUDE.match(line)
                if not match:
                    continue
                target = match.group(1)
                name = os.path.basename(target)
                where = "%s:%d: %s includes %s" % (path, number, library, target)
                climbs = target.startswith("../") or "/../" in target
                resolved = os.path.normpath(os.path.join(os.path.dirname(path), target))
                if "programs/" in target or target.startswith("programs") or resolved.startswith("programs" + os.sep):
                    problems.append(where + ": a path into programs/")
                elif climbs and not resolved.startswith(library + os.sep):
                    problems.append(where + ': a path that climbs out of the library with "../"')
                elif name in program_only:
                    problems.append(where + ": a header that exists only under programs/")
                elif library == "utils" and name in gui_only:
                    problems.append(where + ": a header that exists only in gui/")

        for path in BUILD_FILES[library]:
            if not os.path.isfile(os.path.join(root, path)):
                continue
            for number, line in enumerate(strip_build_comments(read(root, path)), 1):
                if re.search(r"\bprograms\b", line):
                    problems.append("%s:%d: the %s build names programs/" % (path, number, library))
                if library == "utils" and path != "library.pri" and GUI_LINK.search(line):
                    problems.append("%s:%d: the utils build links the GUI library" % (path, number))

    library_headers = header_names(root, "utils") | header_names(root, "gui")
    for group in cycles(program_graph(root, library_headers)):
        problems.append("programs/: these directories depend on each other in a cycle: " + ", ".join(group))

    # library.pri is shared by both libraries; report each of its lines once.
    return sorted(set(problems))


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    args = parser.parse_args()

    for library in LIBRARIES:
        if not os.path.isdir(os.path.join(args.root, library)):
            print("error: %s has no %s/ directory" % (args.root, library), file=sys.stderr)
            return 2

    problems = check(args.root)
    for problem in problems:
        print(problem)
    if problems:
        print("%d dependency layer violation(s)" % len(problems), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
