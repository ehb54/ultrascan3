#!/bin/bash
set -euo pipefail
root=/tmp/us3-residual-2ddf75d6e
out=$root/a979-verification
cmake -S "$root/utils-test-project" -B "$root/utils-test-build" -DCMAKE_BUILD_TYPE=Debug '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined' > "$out/tests-configure.log" 2>&1
cmake --build "$root/utils-test-build" --target test_us_utils test_us_resplot_fem -j1 > "$out/tests-build.log" 2>&1
chown -R us3:us3 "$out" "$root/build" "$root/utils-test-build"
runuser -u us3 -- env QT_QPA_PLATFORM=offscreen ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$root/utils-test-build/test_us_resplot_fem" > "$out/residual.log" 2>&1
runuser -u us3 -- env QT_QPA_PLATFORM=offscreen ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$root/utils-test-build/utils/test_us_utils" > "$out/utils.log" 2>&1
printf 'PASS\n' > "$out/sanitizer-suites.status"
