#!/usr/bin/env bash
# Fail explicit selections that would otherwise succeed without running a test.
# Listing only: this never runs tests or selects a different (possibly live) suite.
set -euo pipefail
binary=${1:?usage: check-gtest-filter.sh BINARY FILTER}
filter=${2:?usage: check-gtest-filter.sh BINARY FILTER}
if ! listing=$("$binary" --gtest_filter="$filter" --gtest_list_tests); then
    echo "Could not list tests in $binary" >&2
    exit 2
fi
if ! awk '
    /^[^[:space:]]/ { suite = $1; next }
    /^[[:space:]]+[^[:space:]]/ {
        if (suite ~ /\.$/ && suite !~ /(^|\/)DISABLED_/ && $1 !~ /(^|\/)DISABLED_/)
            count++
    }
    END { exit(count > 0 ? 0 : 1) }
' <<< "$listing"; then
    echo "No enabled tests match '$filter' in $binary. Check the filter and selected suite; host/source/live-DB tests are separate." >&2
    exit 2
fi
