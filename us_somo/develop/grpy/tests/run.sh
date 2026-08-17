#!/usr/bin/env bash
# Unit tests for the grpy module. Independent of the SOMO qmake build: compiles against the
# module headers (..) only.
#
#   ./run.sh
#
# The solver itself is NOT tested here -- it is a separate GPLv3 program (ehb54/grpy-cpp)
# with its own golden tests against the original Fortran. What lives here is what SOMO
# keeps: the shell reduction, the exposure ranking, and the process boundary between them.
#
# test_shell needs no Qt. test_process drives QProcess, so it needs QtCore: source qt5env
# (or set QTDIR) to include it.
set -euo pipefail
cd "$(dirname "$0")"
CXX="c++ -std=c++17 -O2 -pthread -I.."

echo "[test_shell]"
$CXX test_shell.cpp -o /tmp/grpy_test_shell && /tmp/grpy_test_shell

if [ -n "${QTDIR:-}" ] && [ -d "$QTDIR/lib/QtCore.framework" ]; then          # macOS
    echo "[test_process]"
    $CXX -F"$QTDIR/lib" -I"$QTDIR/lib/QtCore.framework/Headers" -DQT_NO_KEYWORDS \
         test_process.cpp -o /tmp/grpy_test_process -framework QtCore \
      && DYLD_FRAMEWORK_PATH="$QTDIR/lib" /tmp/grpy_test_process
elif [ -n "${QTDIR:-}" ] && [ -d "$QTDIR/include/QtCore" ]; then              # Linux
    echo "[test_process]"
    $CXX -I"$QTDIR/include" -I"$QTDIR/include/QtCore" -DQT_NO_KEYWORDS \
         test_process.cpp -o /tmp/grpy_test_process -L"$QTDIR/lib" -lQt5Core \
      && LD_LIBRARY_PATH="$QTDIR/lib" /tmp/grpy_test_process
else
    echo "[test_process] (skip: set QTDIR to build the process-solver test)"
fi
echo "grpy module tests done."
