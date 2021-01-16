#!/usr/bin/bash
#   copypkg-win   - copy files for packaging - Windows-64

SRCDIR=/cygdrive/c/Users/gegorbet/us3_git/ultrascan3
SOMOBASE=${SRCDIR}/us_somo
QTBIN=c:/msys64/mingw64/bin
QTPLG=c:/msys64/mingw64/share/qt5
DESTDIR=/cygdrive/c/dist
SYSTYPE=`uname -o`
if [ "${SYSTYPE}" != "Cygwin" ]; then
  echo "$0 requires a SYSTYPE (uname -o) of \"Cygwin\""
  echo "  SYSTYPE=${SYSTYPE}"
  exit 1
fi

QTDLLS=" \
 Qt53DCore.dll Qt53DInput.dll Qt53DLogic.dll Qt53DQuick.dll \
 Qt53DQuickInput.dll Qt53DQuickRender.dll Qt53DRender.dll \
 Qt5Bluetooth.dll Qt5Concurrent.dll \
 Qt5Core.dll Qt5DBus.dll \
 Qt5Gui.dll Qt5Help.dll Qt5Location.dll \
 Qt5Multimedia.dll Qt5MultimediaQuick.dll Qt5MultimediaWidgets.dll \
 Qt5Network.dll Qt5Nfc.dll Qt5OpenGL.dll Qt5Positioning.dll \
 Qt5PrintSupport.dll Qt5Qml.dll \
 Qt5Quick.dll Qt5QuickParticles.dll Qt5QuickTest.dll Qt5QuickWidgets.dll \
 Qt5Sensors.dll  Qt5SerialBus.dll Qt5SerialPort.dll \
 Qt5Sql.dll Qt5Svg.dll Qt5WebChannel.dll Qt5WebSockets.dll \
 Qt5Widgets.dll Qt5WinExtras.dll Qt5Xml.dll Qt5XmlPatterns.dll \
 assistant.exe \
"

echo "Copy lib DLLs to bin"
cp -p ${SRCDIR}/lib/*.dll ${SRCDIR}/bin/

echo "Copy bin,etc directories"
cp -rp ${SRCDIR}/bin ${DESTDIR}/
cp -rp ${SRCDIR}/etc ${DESTDIR}/
cp -p ${SRCDIR}/LICENSE.txt ${DESTDIR}/
chmod -R a+rw ${DESTDIR}/etc
if [ ! -d ${DESTDIR}/bin/plugins ]; then
  echo "Creating ${DESTDIR}/bin/plugins"
  mkdir ${DESTDIR}/bin/plugins
fi

# Remove any somo*prev* files in etc/
rm -f ${PKGDIR}/etc/somo*prev*

echo "Copy somo demo,doc directories"
cp -rp ${SOMOBASE}/somo/demo ${DESTDIR}/somo/
cp -rp ${SOMOBASE}/somo/doc ${DESTDIR}/somo/

echo "Copy Qt DLLs to bin"
for F in ${QTDLLS}; do
  echo "  cp -p ${QTBIN}/${F} ${DESTDIR}/bin/"
  cp -p ${QTBIN}/${F} ${DESTDIR}/bin/
done

echo "Copy Qt Plugins to bin"
for F in  platforms sqldrivers; do
  echo "  cp -rp ${QTPLG}/plugins/${F} ${DESTDIR}/bin/"
  cp -rp ${QTPLG}/plugins/${F} ${DESTDIR}/bin/
  echo "  cp -rp ${QTPLG}/plugins/${F} ${DESTDIR}/bin/plugins/"
  cp -rp ${QTPLG}/plugins/${F} ${DESTDIR}/bin/plugins/
done

