#!/usr/bin/env sh
# UltraScan3: generate build metadata header for the splash/About dialogs.
# - Uses git commit count for BUILDNUM
# - Adds GIT_REVISION (commit hash)
# - Adds REVISION_DATE (commit date, not build date)
# - Adds LOCAL_CHANGES (Δ symbol if uncommitted changes)
# - Writes us_revision.h only if values changed (prevents needless rebuilds)

set -eu

# Always run relative to this script's directory
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR"

HEADER="us_revision.h"
TMP_HEADER="${HEADER}.tmp"

# --- determine build number ---
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  # Count all commits reachable from HEAD
  NEW_BUILDNUM=$(git rev-list --count HEAD | tr -d '[:space:]')

  # Get short commit hash
  NEW_GIT_REVISION=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")

  # Get commit date (last commit date)
  NEW_REVISION_DATE=$(git log -1 --format=%cd --date=format:'%Y-%m-%d %H:%M:%S UTC' 2>/dev/null || echo "unknown")

  # Check for local changes (any modified, added, deleted, or untracked files)
  if [ -n "$(git status --porcelain 2>/dev/null)" ]; then
    NEW_LOCAL_CHANGES=" Δ"
  else
    NEW_LOCAL_CHANGES=""
  fi
else
  NEW_BUILDNUM="unknown"
  NEW_GIT_REVISION="unknown"
  NEW_REVISION_DATE="unknown"
  NEW_LOCAL_CHANGES=""
fi

# --- read previous values if header exists ---
OLD_BUILDNUM=""
OLD_GIT_REVISION=""
OLD_REVISION_DATE=""
OLD_LOCAL_CHANGES=""

if [ -f "$HEADER" ]; then
  OLD_BUILDNUM=$(sed -n 's/^#define[[:space:]]\+BUILDNUM[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)
  OLD_GIT_REVISION=$(sed -n 's/^#define[[:space:]]\+GIT_REVISION[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)
  OLD_REVISION_DATE=$(sed -n 's/^#define[[:space:]]\+REVISION_DATE[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)
  OLD_LOCAL_CHANGES=$(sed -n 's/^#define[[:space:]]\+LOCAL_CHANGES[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)
fi

# --- comparison: only rewrite when something changes ---
if [ -f "$HEADER" ] \
   && [ "$NEW_BUILDNUM" = "${OLD_BUILDNUM:-}" ] \
   && [ "$NEW_GIT_REVISION" = "${OLD_GIT_REVISION:-}" ] \
   && [ "$NEW_REVISION_DATE" = "${OLD_REVISION_DATE:-}" ] \
   && [ "$NEW_LOCAL_CHANGES" = "${OLD_LOCAL_CHANGES:-}" ]; then
  # No change — keep existing header to avoid needless rebuilds
  echo "us_revision.h unchanged (build ${NEW_BUILDNUM}, rev ${NEW_GIT_REVISION})"
  exit 0
fi

# --- write new header to a temp file, then atomically replace ---
{
  echo "#ifndef US_REVISION_H"
  echo "#define US_REVISION_H"
  echo ""
  echo "/* Auto-generated: DO NOT EDIT. */"
  echo "#define BUILDNUM        \"${NEW_BUILDNUM}\""
  echo "#define GIT_REVISION    \"${NEW_GIT_REVISION}\""
  echo "#define REVISION_DATE   \"${NEW_REVISION_DATE}\""
  echo "#define LOCAL_CHANGES   \"${NEW_LOCAL_CHANGES}\""
  echo ""
  echo "#endif /* US_REVISION_H */"
} > "$TMP_HEADER"

mv -f "$TMP_HEADER" "$HEADER"
echo "Created/updated $HEADER (build ${NEW_BUILDNUM}, rev ${NEW_GIT_REVISION}${NEW_LOCAL_CHANGES}, ${NEW_REVISION_DATE})"