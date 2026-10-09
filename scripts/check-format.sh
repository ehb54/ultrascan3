#!/usr/bin/env bash
# Checks (or fixes) the UltraScan III code style (.clang-format) on CHANGED LINES only.
#
# Also runs scripts/check-conventions.py (nullptr, f(), connect syntax, guards, \file header).
#
# usage: scripts/check-format.sh [--fix] [--staged | <base-ref>]
#   --staged    check the staged changes (pre-commit hook)          [default]
#   <base-ref>  check the diff between <base-ref> and the worktree  (CI: origin/main)
#   --fix       apply the formatting instead of only reporting it
#
# Only C/C++ sources below gui/, utils/, programs/, test/ and admin/ are considered;
# third-party code (us_somo, qwtplot3d, vcpkg) is excluded.
set -euo pipefail

REQUIRED_MAJOR=18
fix=0
mode=staged
base=""
for a in "$@"; do
   case "$a" in
      --fix)    fix=1 ;;
      --staged) mode=staged ;;
      -h|--help) sed -n '2,13p' "$0"; exit 0 ;;
      *)        mode=ref; base="$a" ;;
   esac
done

cd "$(git rev-parse --show-toplevel)"

cf=${CLANG_FORMAT:-clang-format}
gcf=${GIT_CLANG_FORMAT:-git-clang-format}
for t in "$cf" "$gcf"; do
   command -v "$t" >/dev/null 2>&1 || {
      echo "error: '$t' not found. Install clang-format $REQUIRED_MAJOR (e.g. 'apt install clang-format-$REQUIRED_MAJOR'; set CLANG_FORMAT/GIT_CLANG_FORMAT if suffixed). Note: git-clang-format is not part of the pip package." >&2
      exit 2; }
done
major=$("$cf" --version | sed -E 's/.*version ([0-9]+)\..*/\1/')
[ "$major" = "$REQUIRED_MAJOR" ] || echo "warning: clang-format $major found, style is pinned to $REQUIRED_MAJOR" >&2

exts="c,cc,cpp,cxx,h,hh,hpp"
paths=(gui utils programs test admin)
args=(--extensions "$exts" --binary "$(command -v "$cf")")

if [ "$mode" = staged ]; then
   args+=(--staged)
   ref=()
else
   ref=("$base")
fi

conv=(scripts/check-conventions.py)
if [ "$mode" = staged ]; then conv+=(--staged); else conv+=("$base"); fi

if [ "$fix" = 1 ]; then
   "$gcf" "${args[@]}" "${ref[@]}" -- "${paths[@]}" || true
   python3 "${conv[0]}" --fix "${conv[@]:1}" || true   # fixes what it can, reports the rest
   exit 0
fi

rc=0
python3 "${conv[@]}" || rc=1

out=$("$gcf" "${args[@]}" --diff "${ref[@]}" -- "${paths[@]}" || true)
case "$out" in
   *"no modified files to format"*|*"did not modify any files"*|"") exit $rc ;;
esac
echo "$out"
echo
echo "Code style violations found (see wiki: UltraScan-III-Coding-Standards)."
echo "Fix with: scripts/check-format.sh --fix $([ "$mode" = ref ] && echo "$base" || echo --staged)   (then re-stage with git add)"
exit 1
