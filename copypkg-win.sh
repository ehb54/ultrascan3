#!/usr/bin/bash
#   copypkg-win   - copy files for packaging - Windows-64

SRCDIR=/cygdrive/c/Users/demelerlab/ultrascan3
##SOMOBASE=/cygdrive/c/Users/demelerlab/us3_somo
QTBIN=/cygdrive/c/Qt/5.6.0/bin
DESTDIR=/cygdrive/c/dist
SYSTYPE=`uname -o`
if [ "${SYSTYPE}" != "Cygwin" ]; then
  echo "$0 requires a SYSTYPE (uname -o) of \"Cygwin\""
  echo "  SYSTYPE=${SYSTYPE}"
  exit 1
fi

##SOMOBINS="us3_somo.exe us3_config.exe rasmol.exe \
## rasmol.hlp us_admin.exe us_somo10.dll"
##QT4DLLS="Qt3Support4.dll QtCLucene4.dll QtCore4.dll \
## QtGui4.dll QtHelp4.dll QtNetwork4.dll QtOpenGL4.dll \
## QtSql4.dll QtSvg4.dll QtWebKit4.dll QtXml4.dll assistant.exe"
QTDLLS=" \
 Qt53DCore.dll Qt53DInput.dll Qt53DLogic.dll Qt53DQuick.dll \
 Qt53DQuickInput.dll Qt53DQuickRender.dll Qt53DRender.dll \
 Qt5Bluetooth.dll Qt5CLucene.dll Qt5Concurrent.dll \
 Qt5Core.dll Qt5DBus.dll \
 Qt5Gui.dll Qt5Help.dll Qt5LabsTemplates.dll Qt5Location.dll \
 Qt5Network.dll Qt5Nfc.dll Qt5OpenGL.dll Qt5Positioning.dll \
 Qt5PrintSupport.dll Qt5Qml.dll \
 Qt5Quick.dll Qt5QuickParticles.dll Qt5QuickTest.dll Qt5QuickWidgets.dll \
 Qt5Sensors.dll  Qt5SerialBus.dll Qt5SerialPort.dll \
 Qt5Sql.dll Qt5Svg.dll Qt5WebChannel.dll Qt5WebSockets.dll \
 Qt5Widgets.dll Qt5WinExtras.dll Qt5Xml.dll Qt5XmlPatterns.dll \
"

echo "Copy lib DLLs to bin"
cp -p ${SRCDIR}/lib/*.dll ${SRCDIR}/bin/

##echo "Copy somo bins to bin"
##for F in ${SOMOBINS}; do
##  echo "  cp -p ${SOMOBASE}/bin/${F} ${SRCDIR}/bin/"
##  cp -p ${SOMOBASE}/bin/${F} ${SRCDIR}/bin/
##done

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

##if [ ! -d ${DESTDIR}/somo ]; then
##  echo "Creating ${DESTDIR}/somo"
##  mkdir ${DESTDIR}/somo
##else
##  # Clear out somo/demo so only present contents are copied
##  if [ -d ${DESTDIR}/somo/demo ]; then
##    rm -rf ${DESTDIR}/somo/demo/*
##  fi
##fi
##
##echo "Copy somo doc,demo directories"
##cp -rp ${SOMOBASE}/doc ${DESTDIR}/somo/
cp -rp ${SOMOBASE}/somo/demo ${DESTDIR}/somo/
####cp -p ${SOMOBASE}/LICENSE.txt ${DESTDIR}/
### Remove somo/doc/manual/attic directory
##rm -rf ${DESTDIR}/somo/doc/manual/attic

echo "Copy Qt DLLs to bin"
for F in ${QTDLLS}; do
  echo "  cp -p ${QTBIN}/${F} ${DESTDIR}/bin/"
  cp -p ${QTBIN}/${F} ${DESTDIR}/bin/
done

echo "Copy Qt Plugins to bin"
for F in  platforms sqldrivers; do
  echo "  cp -rp ${QTDIR}/plugins/${F} ${DESTDIR}/bin/"
  cp -rp ${QTDIR}/plugins/${F} ${DESTDIR}/bin/
  echo "  cp -rp ${QTDIR}/plugins/${F} ${DESTDIR}/bin/plugins/"
  cp -rp ${QTDIR}/plugins/${F} ${DESTDIR}/bin/plugins/
done

