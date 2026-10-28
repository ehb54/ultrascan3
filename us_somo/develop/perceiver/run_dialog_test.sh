#!/bin/sh
# Offscreen test of the non-coded residue review dialog.
#
# Unlike the rest of perceiver/, this one needs Qt and the built libus_somo, because the thing
# under test IS a widget. It runs headless (QT_QPA_PLATFORM=offscreen), so it needs no display
# and is safe in CI.
#
#   cd us_somo/develop/perceiver && ./run_dialog_test.sh
#
# Build libus_somo first:  . ../../../qt5env && cd .. && qmake libus_somo.pro && make
set -e
cd "$(dirname "$0")"
DEV=..
. ../../../qt5env

# Reuse the exact flags qmake used for the dialog itself, so this cannot drift from the real
# build; take them from the most recent compile of that translation unit.
FLAGS="-pipe -stdlib=libc++ -O2 -std=gnu++1y -fPIC \
 -DUNIX -DQT_NO_STL -DMACX -DOSX -DOPTERON -DMAC -DTHREAD -DNO_DB -DNO_EDITOR_PRINT -DQT_NO_DEBUG \
 -I$DEV -I$DEV/include -I$QTDIR/include -I$QWTDIR/include -I$DEV/src/moc"

# Qt5 on macOS ships as frameworks, so each module's headers need their own -I.
for m in QtCore QtGui QtWidgets QtNetwork QtSvg QtPrintSupport QtOpenGL QtMultimedia; do
  FLAGS="$FLAGS -I$QTDIR/lib/$m.framework/Headers"
done

clang++ $FLAGS tests/dialog_qt.cpp \
  -L$DEV/../lib -lus_somo \
  -F$QTDIR/lib -F$QWTDIR/lib \
  -framework qwt -framework QtWidgets -framework QtGui -framework QtCore \
  -framework QtSvg -framework QtPrintSupport -framework QtOpenGL \
  -framework QtNetwork -framework QtMultimedia \
  -o bin/dialog_qt

DYLD_FRAMEWORK_PATH=$QTDIR/lib:$QWTDIR/lib \
DYLD_LIBRARY_PATH=$DEV/../lib \
QT_QPA_PLATFORM=offscreen \
ULTRASCAN=${ULTRASCAN:-$HOME/ultrascan} \
  ./bin/dialog_qt
