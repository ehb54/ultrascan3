#!/usr/bin/env sh
# UltraScan3: generate build metadata header for the splash/About dialogs.
# - Counts git commits for BUILDNUM
# - Writes us_revision.h only if values changed (rev1/rev2 logic)
# - Keeps legacy REVISION macro for compatibility

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
else
  NEW_BUILDNUM="unknown"
fi

# --- date/time ---
NEW_BUILD_DATE=$(date -u '+%Y-%m-%d')
NEW_BUILD_TIME=$(date -u '+%H:%M:%S')

# --- read previous values (rev2) if header exists ---
OLD_BUILDNUM=""
OLD_BUILD_DATE=""
OLD_BUILD_TIME=""

if [ -f "$HEADER" ]; then
  OLD_BUILDNUM=$(sed -n 's/^#define[[:space:]]\+BUILDNUM[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)
  OLD_BUILD_DATE=$(sed -n 's/^#define[[:space:]]\+BUILD_DATE[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)
  OLD_BUILD_TIME=$(sed -n 's/^#define[[:space:]]\+BUILD_TIME[[:space:]]\+"\(.*\)".*$/\1/p' "$HEADER" | head -n1 || true)

  # Backward-compat: if old file only had REVISION "Revision: 7999", extract the number
  if [ -z "$OLD_BUILDNUM" ]; then
    OLD_BUILDNUM=$(sed -n 's/^#define[[:space:]]\+REVISION[[:space:]]\+"Revision:[[:space:]]\+\([0-9][0-9]*\)".*$/\1/p' "$HEADER" | head -n1 || true)
  fi
fi

# --- rev1 / rev2 comparison: only rewrite when something changes ---
if [ -f "$HEADER" ] \
   && [ "$NEW_BUILDNUM" = "${OLD_BUILDNUM:-}" ] \
   && [ "$NEW_BUILD_DATE" = "${OLD_BUILD_DATE:-}" ] \
   && [ "$NEW_BUILD_TIME" = "${OLD_BUILD_TIME:-}" ]; then
  # No change — keep existing header to avoid needless rebuilds
  echo "us_revision.h unchanged (build ${NEW_BUILDNUM})"
  exit 0
fi

# --- write new header to a temp file, then atomically replace ---
{
  echo "#ifndef US_REVISION_H"
  echo "#define US_REVISION_H"
  echo ""
  echo "/* Auto-generated: DO NOT EDIT. */"
  echo "#define BUILDNUM   \"${NEW_BUILDNUM}\""
  echo "#define BUILD_DATE \"${NEW_BUILD_DATE}\""
  echo "#define BUILD_TIME \"${NEW_BUILD_TIME}\""
  echo ""
  echo "#endif /* US_REVISION_H */"
} > "$TMP_HEADER"

mv -f "$TMP_HEADER" "$HEADER"
echo "Created/updated $HEADER (build ${NEW_BUILDNUM}, ${NEW_BUILD_DATE} ${NEW_BUILD_TIME})"
