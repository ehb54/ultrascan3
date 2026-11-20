#!/bin/bash
# Read the successful package target's receipt; never search build directories.
set -euo pipefail
receipt=${1:?Usage: report-package.sh receipt-file}
if [ ! -f "$receipt" ]; then
  echo "ERROR: Package receipt is missing: $receipt" >&2
  exit 1
fi
package=$(cat "$receipt")
package=${package%$'\r'}
case "$package" in
  *$'\n'*|*$'\r'*) echo "ERROR: Package receipt must contain one path" >&2; exit 1 ;;
esac
if [[ "$package" != /* ]] || [ ! -f "$package" ] || [ ! -s "$package" ]; then
  echo "ERROR: Declared package is missing, empty, or not an absolute file path: $package" >&2
  exit 1
fi
printf 'Package: %s\n' "$package"
if [ -n "${GITHUB_OUTPUT:-}" ]; then
  printf 'package_path=%s\n' "$package" >> "$GITHUB_OUTPUT"
fi
