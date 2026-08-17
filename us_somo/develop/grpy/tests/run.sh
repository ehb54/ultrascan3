#!/usr/bin/env bash
# Standalone unit tests for the in-process grpy module. Independent of the SOMO
# qmake build: compiles against the module headers (..) and SOMO's vendored Eigen
# (../../include). Run from this directory (test_api reads data/).
#   ./run.sh
# Optional Qt backend proof: source qt5env (or set QTDIR) first.
set -euo pipefail
cd "$(dirname "$0")"
EIGEN="${EIGEN_INC:-../../include}"
CXX="c++ -std=c++17 -O2 -pthread -I.. -I$EIGEN"

for t in test_linalg test_assemble test_threaded test_ooc test_api test_shell; do
    echo "[$t]"; $CXX "$t.cpp" -o "/tmp/grpy_$t" && "/tmp/grpy_$t"
done

# Qt-dependent tests: the process solver (QProcess) and the Qt threading backend.
if [ -n "${QTDIR:-}" ] && [ -d "$QTDIR/lib/QtCore.framework" ]; then          # macOS
    echo "[test_process]"
    $CXX -F"$QTDIR/lib" -I"$QTDIR/lib/QtCore.framework/Headers" -DQT_NO_KEYWORDS \
         test_process.cpp -o /tmp/grpy_test_process -framework QtCore \
      && DYLD_FRAMEWORK_PATH="$QTDIR/lib" /tmp/grpy_test_process
    echo "[qt_proof]"
    $CXX -F"$QTDIR/lib" -I"$QTDIR/lib/QtCore.framework/Headers" \
         -I"$QTDIR/lib/QtConcurrent.framework/Headers" -DQT_NO_KEYWORDS \
         qt_proof.cpp -o /tmp/grpy_qt -framework QtConcurrent -framework QtCore \
      && DYLD_FRAMEWORK_PATH="$QTDIR/lib" /tmp/grpy_qt
elif [ -n "${QTDIR:-}" ] && [ -d "$QTDIR/include/QtConcurrent" ]; then          # Linux
    echo "[test_process]"
    $CXX -I"$QTDIR/include" -I"$QTDIR/include/QtCore" -DQT_NO_KEYWORDS \
         test_process.cpp -o /tmp/grpy_test_process -L"$QTDIR/lib" -lQt5Core \
      && LD_LIBRARY_PATH="$QTDIR/lib" /tmp/grpy_test_process
    echo "[test_process]"
    $CXX -I"$QTDIR/include" -I"$QTDIR/include/QtCore" -I"$QTDIR/include/QtConcurrent" \
         -DQT_NO_KEYWORDS qt_proof.cpp -o /tmp/grpy_qt \
         -L"$QTDIR/lib" -lQt5Concurrent -lQt5Core \
      && LD_LIBRARY_PATH="$QTDIR/lib" /tmp/grpy_qt
else
    echo "[qt_proof] (skip: set QTDIR to build the Qt backend proof)"
fi
echo "grpy module tests done."
