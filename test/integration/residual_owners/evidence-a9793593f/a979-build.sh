#!/bin/bash
set -euo pipefail
root=/tmp/us3-residual-2ddf75d6e
out=$root/a979-verification
mkdir -p "$out"
curl -fsS http://10.211.55.2:18768/a979-delta.tar -o "$out/delta.tar"
tar -xf "$out/delta.tar" -C "$root/source"
curl -fsS http://10.211.55.2:18768/a979-manifest.json -o "$out/source-manifest.json"
curl -fsS http://10.211.55.2:18768/a979-utils-tests.cmake -o "$root/utils-test-project/CMakeLists.txt"
cmake --build "$root/build" --target us_fematch us_autoflow_analysis us_query_rmsd us_reporter_gmp us_com_project us_comproject_academic us_protocoldev -j2 > "$out/consumers-build.log" 2>&1
printf 'PASS\n' > "$out/consumers-build.status"
cmake -S "$root/utils-test-project" -B "$root/utils-test-build" -DCMAKE_BUILD_TYPE=Debug '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined' > "$out/tests-configure.log" 2>&1
cmake --build "$root/utils-test-build" --target test_us_utils test_us_resplot_fem -j2 > "$out/tests-build.log" 2>&1
chown -R us3:us3 "$out" "$root/build" "$root/utils-test-build"
ctest --test-dir "$root/utils-test-build" -L dependency_layers --output-on-failure > "$out/m12-ctest.log" 2>&1
runuser -u us3 -- env QT_QPA_PLATFORM=offscreen ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$root/utils-test-build/test_us_resplot_fem" > "$out/residual.log" 2>&1
runuser -u us3 -- env QT_QPA_PLATFORM=offscreen ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$root/utils-test-build/utils/test_us_utils" > "$out/utils.log" 2>&1
printf 'PASS\n' > "$out/tests.status"
