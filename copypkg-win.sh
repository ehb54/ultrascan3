#!/usr/bin/bash
#   copypkg-win   - copy files for packaging - Windows

SRCDIR=/cygdrive/c/Users/Admin/Documents/ultrascan3
SOMOBASE=/cygdrive/c/Users/Admin/Documents/us3_somo
QTBIN=/cygdrive/c/Qt/4.8.4/bin
DESTDIR=/cygdrive/c/dist
SYSTYPE=`uname -o`
if [ "${SYSTYPE}" != "Cygwin" ]; then
  echo "$0 requires a SYSTYPE (uname -o) of \"Cygwin\""
  echo "  SYSTYPE=${SYSTYPE}"
  exit 1
fi

SOMOBINS="us3_somo.exe us3_config.exe rasmol.exe \
 rasmol.hlp us_admin.exe us_somo10.dll"
QTDLLS="Qt3Support4.dll QtCLucene4.dll QtCore4.dll \
 QtGui4.dll QtHelp4.dll QtNetwork4.dll QtOpenGL4.dll \
 QtSql4.dll QtSvg4.dll QtWebKit4.dll QtXml4.dll assistant.exe"


echo "Copy lib DLLs to bin"
cp -p ${SRCDIR}/lib/*.dll ${SRCDIR}/bin/

echo "Copy somo bins to bin"
for F in ${SOMOBINS}; do
  echo "  cp -p ${SOMOBASE}/bin/${F} ${SRCDIR}/bin/"
  cp -p ${SOMOBASE}/bin/${F} ${SRCDIR}/bin/
done

echo "Copy bin,etc directories"
cp -rp ${SRCDIR}/bin ${DESTDIR}/
cp -rp ${SRCDIR}/etc ${DESTDIR}/
cp -p ${SRCDIR}/LICENSE.txt ${DESTDIR}/
chmod -R a+rw ${DESTDIR}/etc

# Remove any somo*prev* files in etc/
rm -f ${PKGDIR}/etc/somo*prev*

if [ ! -d ${DESTDIR}/somo ]; then
  echo "Creating ${DESTDIR}/somo"
  mkdir ${DESTDIR}/somo
else
  # Clear out somo/demo so only present contents are copied
  if [ -d ${DESTDIR}/somo/demo ]; then
    rm -rf ${DESTDIR}/somo/demo/*
  fi
fi

echo "Copy somo doc,demo directories"
cp -rp ${SOMOBASE}/doc ${DESTDIR}/somo/
cp -rp ${SOMOBASE}/somo/demo ${DESTDIR}/somo/
##cp -p ${SOMOBASE}/LICENSE.txt ${DESTDIR}/
# Remove somo/doc/manual/attic directory
rm -rf ${DESTDIR}/somo/doc/manual/attic

echo "Copy Qt DLLs to bin"
for F in ${QTDLLS}; do
  echo "  cp -p ${QTBIN}/${F} ${DESTDIR}/bin/"
  cp -p ${QTBIN}/${F} ${DESTDIR}/bin/
done

