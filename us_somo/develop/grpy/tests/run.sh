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
# -std=gnu++11 deliberately, matching what SOMO itself compiles this module with. SOMO
# supports Qt5 as well as Qt6 and so must build under the older compilers Qt5 is used with;
# nothing in its build raises the standard. Compiling the tests at a HIGHER standard than the
# product hides breakage rather than finding it -- this was set to -std=c++17 and let a
# non-aggregate braced initialisation into grpy_shell.hpp that every test passed on and the
# Qt5 macOS build then refused to compile. Raise this only when SOMO's own standard moves.
CXX="c++ -std=gnu++11 -O2 -pthread -I.."

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
