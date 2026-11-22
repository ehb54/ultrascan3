#!/usr/bin/env python3
"""UltraScan III conventions that neither clang-format nor clang-tidy enforce.

usage: check-conventions.py [--fix] (--staged | <base-ref>)

  --staged    check the index (pre-commit hook)
  <base-ref>  check the worktree against <base-ref> (CI: merge-base with origin/main)
  --fix       append a missing trailing newline (worktree; re-stage afterwards)

On added/changed lines of C++ sources (token-based via pygments, so comments and
string/char/raw-string literals are never matched; #define bodies are included):
  * SIGNAL() / SLOT()    -> connect( a, &A::sig, b, &B::slot )
    suppress a justified use with '// NOLINT(us3-old-connect)' on the same line
On changed files:
  * file ends with a newline (fixable with --fix)
  * headers have an include guard
On NEW files (added, copied, or renamed to a different basename):
  * first line is '//! \\file <basename>'
  * headers use the guard <BASENAME_IN_UPPERCASE>_H  (us_foo.h -> US_FOO_H)

NULL -> nullptr and f( void ) -> f() are enforced by clang-tidy
(modernize-use-nullptr, modernize-redundant-void-arg; see .clang-tidy).
Requires pygments (pip install pygments / apt install python3-pygments).
"""
import codecs
import os
import re
import subprocess
import sys

try:
    from pygments.lexers import CppLexer
    from pygments.token import Comment, Name, Punctuation
except ImportError:
    sys.exit("error: pygments not found (pip install pygments or apt install python3-pygments)")

SCOPE = ("gui/", "utils/", "programs/", "test/", "admin/")
CPP = (".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp")
HDR = (".h", ".hh", ".hpp")
OLD_CONNECT = {"SIGNAL", "SLOT"}
NOLINT = re.compile(r"\bNOLINT\b(?:\(([^)]*)\))?")
GUARD = r"^#ifndef\s+%s\b[^\n]*\n#define\s+%s\b"   # trailing comments allowed

# Fixed diff format, independent of the user's git config
# (diff.noprefix, diff.mnemonicPrefix, color.ui, diff.external, core.quotePath, ...).
DIFF = ("-c", "core.quotePath=false", "diff", "--no-color", "--no-ext-diff",
        "--src-prefix=a/", "--dst-prefix=b/")


def git(*a, binary=False, stdin=None):
    r = subprocess.run(("git",) + a, check=True, capture_output=True, input=stdin)
    return r.stdout if binary else r.stdout.decode("utf-8", "replace")


def changed_files(ref):
    """({path: 'A'|'M'}, [rename/copy sources]) for C++ files in scope (NUL-safe).

    Renames count as new ('A') only if the basename changed; a pure move keeps
    '\\file <basename>' and the guard valid."""
    out = git(*DIFF, *ref, "--name-status", "-z", "--diff-filter=ACMR").split("\0")
    files, sources, i = {}, [], 0
    while i < len(out) and out[i]:
        st = out[i][0]
        if st in "RC":                       # R100 <old> <new>
            old, path, i = out[i + 1], out[i + 2], i + 3
            new = st == "C" or os.path.basename(old) != os.path.basename(path)
        else:
            path, i = out[i + 1], i + 2
            new = st == "A"
        if path.startswith(SCOPE) and path.endswith(CPP):
            files[path] = "A" if new else "M"
            if st in "RC":
                sources.append(old)
    return files, sources


def unquote(p):
    """Path from a '+++ b/<path>' header (handles git's C-quoting and trailing tab)."""
    p = p.rstrip("\t")
    if p.startswith('"') and p.endswith('"'):
        p = codecs.escape_decode(p[1:-1].encode("utf-8"))[0].decode("utf-8", "replace")
    return p[2:] if p.startswith("b/") else None


def added_lines(ref, paths, sources):
    """{path: {line numbers of added lines}} from ONE 'git diff -U0' call.

    Rename/copy sources must be in the pathspec, otherwise git cannot pair them and
    reports every line of a moved file as added."""
    res = {p: set() for p in paths}
    cur, header = None, False
    for l in git(*DIFF, *ref, "-U0", "--", *paths, *sources).split("\n"):
        if l.startswith("diff --git "):
            cur, header = None, True
        elif header and l.startswith("+++ "):
            cur = unquote(l[4:])
        elif l.startswith("@@"):
            header = False
            m = re.match(r"@@ -\S+ \+(\d+)(?:,(\d+))? @@", l)
            if m and cur in res:
                start, n = int(m.group(1)), int(m.group(2) or 1)
                res[cur].update(range(start, start + n))
    return res


def read_index(paths):
    """{path: bytes} of the staged versions from ONE 'git cat-file --batch' call."""
    out = git("cat-file", "--batch", binary=True,
              stdin="".join(":%s\n" % p for p in paths).encode("utf-8"))
    res, pos = {}, 0
    for p in paths:
        nl = out.index(b"\n", pos)
        hdr = out[pos:nl].split()
        if len(hdr) < 3 or hdr[1] != b"blob":    # '<name> missing'
            res[p], pos = None, nl + 1
            continue
        size = int(hdr[2])
        res[p], pos = out[nl + 1:nl + 1 + size], nl + 1 + size + 1
    return res


def tokens(text, line=1):
    """Yield (line, ttype, value) of the non-whitespace tokens.

    pygments lexes a preprocessor directive as opaque Comment.Preproc chunks, split
    at '/', comments and line continuations; a #define is therefore reassembled up
    to its terminating newline and its body lexed again as C++."""
    toks = list(CppLexer(stripnl=False, ensurenl=False).get_tokens(text))
    i = 0
    while i < len(toks):
        tt, v = toks[i]
        if tt in Comment.Preproc and v == "#":
            j, body = i + 1, ""
            while j < len(toks) and toks[j][0] in Comment:
                body += toks[j][1]
                j += 1
                if body.endswith("\n") and not body[:-1].rstrip(" \t").endswith("\\"):
                    break
            if re.match(r"\s*define\b", body):
                yield from tokens(body, line)
                line += (v + body).count("\n")
                i = j
                continue
        if v.strip():
            yield line, tt, v
        line += v.count("\n")
        i += 1


def old_connects(text):
    """Lines using SIGNAL( / SLOT( that are not suppressed via NOLINT."""
    text = re.sub(r"\r(?!\n)", " ", text)        # pygments maps a lone CR to LF; git does not
    toks = list(tokens(text))
    hits, suppressed = set(), set()
    for k, (ln, tt, v) in enumerate(toks):
        if tt in Comment:
            for m in NOLINT.finditer(v):
                if m.group(1) is None or "us3-old-connect" in m.group(1):
                    suppressed.add(ln)
        elif (tt in Name and v in OLD_CONNECT and k + 1 < len(toks)
              and toks[k + 1][1] in Punctuation and toks[k + 1][2] == "("):
            hits.add(ln)
    return hits - suppressed


def main():
    args = sys.argv[1:]
    fix = "--fix" in args
    args = [a for a in args if a != "--fix"]
    if len(args) != 1:
        sys.exit(__doc__)
    staged = args[0] == "--staged"
    ref = ["--cached"] if staged else [args[0]]

    os.chdir(git("rev-parse", "--show-toplevel").strip())
    files, sources = changed_files(ref)
    files = {p: st for p, st in files.items() if staged or os.path.exists(p)}
    if not files:
        return 0
    paths = sorted(files)
    changed = added_lines(ref, paths, sources)
    if staged:
        contents = read_index(paths)
    else:
        contents = {}
        for p in paths:
            with open(p, "rb") as f:
                contents[p] = f.read()
    problems = []

    for path in paths:
        st, data = files[path], contents[path]
        if data is None:
            continue
        text = data.decode("utf-8", "replace")
        base = os.path.basename(path)

        # Lexing is the dominant cost; a hit needs SIGNAL/SLOT on a changed line.
        lines = re.sub(r"\r(?!\n)", " ", text).split("\n")
        if any("SIGNAL" in lines[n - 1] or "SLOT" in lines[n - 1]
               for n in changed[path] if n <= len(lines)):
            hits = old_connects(text) & changed[path]
        else:
            hits = set()
        for ln in sorted(hits):
            problems.append((path, ln, "use connect( a, &A::signal, b, &B::slot ) instead of "
                                       "SIGNAL()/SLOT() (or '// NOLINT(us3-old-connect)')"))

        if data and not data.endswith(b"\n"):
            fixed = False
            if fix and os.path.exists(path):
                with open(path, "rb") as f:
                    wt = f.read()
                if wt and not wt.endswith(b"\n"):
                    with open(path, "wb") as f:
                        f.write(wt.rstrip(b" \t") + b"\n")
                fixed = not staged          # staged: the index still needs 'git add'
            if not fixed:
                problems.append((path, text.count("\n") + 1, "missing newline at end of file"
                                 + (" (fixed in worktree, re-stage)" if fix and staged else "")))

        if path.endswith(HDR) and not re.search(GUARD % (r"\w+", r"\w+"), text, re.M):
            problems.append((path, 1, "header has no #ifndef/#define include guard"))
        if st == "A":
            if not text.startswith("//! \\file " + base):
                problems.append((path, 1, "first line must be '//! \\file %s'" % base))
            if path.endswith(".h"):
                guard = re.sub(r"\W", "_", base).upper()
                if not re.search(GUARD % (guard, guard), text, re.M):
                    problems.append((path, 1, "include guard must be '%s'" % guard))

    for f, n, msg in problems:
        print("%s:%s: %s" % (f, n, msg))
    if problems:
        print("\nConvention violations found (see scripts/check-conventions.py).")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
