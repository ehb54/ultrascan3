#!/bin/sh
#		copypkg-lnx.sh  - copy ultrascan3 directories to us3pkg/PKGNAME

SRCDIR=$us3
HOMEQ=`cd $SRCDIR/../;pwd`
DSTDIR=$HOMEQ/us3pkg
SOMOBASE=$HOMEQ/us3_somo
SOMORHDR=${SOMOBASE}/develop/include/us_revision.h
ME=`whoami`
SYSTYPE=`uname -s`
if [ "${SYSTYPE}" != "Linux" ]; then
  echo "$0 requires a SYSTYPE (uname -s) of \"Linux\""
  echo "  SYSTYPE=${SYSTYPE}"
  exit 1
fi
QTVER=`qmake --version|grep Qt|cut -d' ' -f4|cut -d'.' -f1`
if [ ${QTVER} = "4" ];then
  echo "Qt Version 4";
  LNCMD="sed -n 2p"
else
  echo "Qt Version 5";
  LNCMD="sed -n 1p"
fi
SURL="//bcf2.uthscsa.edu/ultrascan3/trunk"
REV=`svn info svn:${SURL}|grep Revision|awk '{print $2}'`
RSYNC="rsync -av --exclude=.svn"
REVL=`svn info ${SRCDIR}|grep Revision|awk '{print $2}'`
MVERS=`grep US_Version ${SRCDIR}/utils/us_defines.h|${LNCMD}|cut -d'"' -f2`
SREV=`grep Revision ${SOMORHDR}|cut -d \" -f2|awk '{print $2}'`

if [ "${REV}" != "${REVL}" ]; then
  # Abort if source is not latest revision
  echo "Rev from ${SURL}:   ${REV}"
  echo "Rev from ${SRCDIR}:   ${REVL}"
  echo "  Do 'svn update' and build, then re-run this script"
  exit 1
fi

QTLIBS4=" \
 libQtCore.so
 libQtDBus.so
 libQtGui.so
 libQtHelp.so
 libQtNetwork.so
 libQtOpenGL.so
 libQtSql.so
 libQtSvg.so
 libQtWebKit.so
 libQtXml.so
 libQtCLucene.so
 libQt3Support.so
 libphonon.so
 "
QTLIBS5=" \
libQt53DCore.so
libQt53DInput.so
libQt53DLogic.so
libQt53DQuickInput.so
libQt53DQuickRender.so
libQt53DQuick.so
libQt53DRender.so
libQt5Bluetooth.so
libQt5CLucene.so
libQt5Concurrent.so
libQt5Core.so
libQt5DBus.so
libQt5DesignerComponents.so
libQt5Designer.so
libQt5Gui.so
libQt5Help.so
libQt5LabsTemplates.so
libQt5Location.so
libQt5MultimediaQuick_p.so
libQt5Multimedia.so
libQt5MultimediaWidgets.so
libQt5Network.so
libQt5Nfc.so
libQt5OpenGL.so
libQt5Positioning.so
libQt5PrintSupport.so
libQt5Qml.so
libQt5QuickParticles.so
libQt5Quick.so
libQt5QuickTest.so
libQt5QuickWidgets.so
libQt5Script.so
libQt5ScriptTools.so
libQt5Sensors.so
libQt5SerialBus.so
libQt5SerialPort.so
libQt5Sql.so
libQt5Svg.so
libQt5Test.so
libQt5WebChannel.so
libQt5WebSockets.so
libQt5Widgets.so
libQt5X11Extras.so
libQt5XcbQpa.so
libQt5XmlPatterns.so
libQt5Xml.so
"
if [ ${QTVER} = "4" ];then
  QTLIBS=${QTLIBS4}
  SREV="-s${SREV}"
else
  QTLIBS=${QTLIBS5}
  ##SREV=""
  SREV="-s${SREV}"
fi
##         echo "QTLIBS=${QTLIBS}"
##         exit 1
QTBINS="assistant"

# Determine if this is 32-bit or 64-bit
IS64=`which gcc`
IS64=`file ${IS64} | egrep -ci '64-bit|x86-64'`
if [ ${IS64} -ne 0 ]; then
  PKGNAME=us3-Linux64-${MVERS}.${REV}${SREV}
else
  PKGNAME=us3-Linux32-${MVERS}.${REV}${SREV}
fi
PKGDIR=${DSTDIR}/${PKGNAME}

# Insure we have a package base directory
cd $HOMEQ
if [ ! -d us3pkg ]; then
  mkdir us3pkg
fi
cd ${DSTDIR}
if [ ! -d ${PKGNAME} ]; then
  mkdir ${PKGNAME};
fi
echo "PKGDIR=${PKGDIR}"

# Copy the US3 directories
(cd ${PKGDIR};mkdir etc bin lib)
for D in etc bin lib us3-update.sh; do
  SDIR=${SRCDIR}/${D}
  DDIR=${PKGDIR}
  echo "${RSYNC} ${SDIR} ${DDIR}"
  ${RSYNC} ${SDIR} ${DDIR}
done

# Remove any somo*prev* files in etc/
rm -f ${PKGDIR}/etc/somo*prev*

##if [ ${QTVER} = "4" ];then
  SOMODIR=$SOMOBASE/somo
  # Copy the SOMO directories
  cd ${PKGDIR};mkdir somo somo/demo somo/doc somo/doc/manual;
  SDIR=${SOMODIR}/doc
  DDIR=${PKGDIR}/somo
  echo "${RSYNC} ${SDIR} ${DDIR}"
  ${RSYNC} ${SDIR} ${DDIR}

  # Remove the somo/doc/manual/attic directory
  rm -rf ${PKGDIR}/somo/doc/manual/attic

  if [ -d ${SOMODIR} -a -d ${SOMODIR}/demo ]; then
    # Clear out somo/demo so only present contents are copied
    /bin/rm -rf ${PKGDIR}/somo/demo/*
    # Copy somo demo contents
    SDIR=${SOMODIR}/demo
    DDIR=${PKGDIR}/somo
    echo "${RSYNC} ${SDIR} ${DDIR}"
    ${RSYNC} ${SDIR} ${DDIR}
  fi
##fi

# Copy special library files
CPYCMD='cp -d --preserve=timestamps'
cd ${PKGDIR}
MYSQL=`ldd bin/u* lib/*|grep -i mysql|sed -n 1p`
      echo "MYSQL=${MYSQL}"
if [ ! -z "${MYSQL}" ]; then
  MYSQL=`echo ${MYSQL} | awk '{print $3}' | sed -e 's/sqlclient.so.*$/sqlclient.so/'`
      echo "MYSQL=${MYSQL}"
  ${CPYCMD} ${MYSQL}*  ${PKGDIR}/lib/.
fi
JPEG=`ldd bin/u* lib/*|grep -i jpeg|sed -n 1p`
      echo "JPEG=${JPEG}"
if [ ! -z "${JPEG}" ]; then
  JPEG=`echo ${JPEG} | awk '{print $3}' | sed -e 's/jpeg.so.*$/jpeg.so/'`
      echo "JPEG=${JPEG}"
  ${CPYCMD} ${JPEG}*  ${PKGDIR}/lib/.
fi
PNG=`ldd bin/u* lib/*|grep -i png|sed -n 1p`
      echo "PNG=${PNG}"
if [ ! -z "${PNG}" ]; then
  PNG=`echo ${PNG} | awk '{print $3}' | sed -e 's/png.so.*$/png.so/'`
      echo "PNG=${PNG}"
  ${CPYCMD} ${PNG}*  ${PKGDIR}/lib/.
fi
CRYPT=`ldd bin/u* lib/*|grep -i libcrypt|sed -n 1p`
      echo "CRYPT=${CRYPT}"
if [ ! -z "${CRYPT}" ]; then
  CRYPT=`echo ${CRYPT} | awk '{print $3}' | sed -e 's/libcrypt.*$/libcrypt/'`
      echo "CRYPT=${CRYPT}"
  ${CPYCMD} ${CRYPT}*  ${PKGDIR}/lib/.
fi
SSL=`ldd bin/u* lib/*|grep -i ssl|sed -n 1p`
      echo "SSL=${SSL}"
if [ ! -z "${SSL}" ]; then
  SSL=`echo ${SSL} | awk '{print $3}' | sed -e 's/ssl.so.*$/ssl.so/'`
      echo "SSL=${SSL}"
  ${CPYCMD} ${SSL}*  ${PKGDIR}/lib/.
fi

      echo "QTDIR=${QTDIR}"
if [ ! -z "${QTDIR}" ]; then
  for x in ${QTLIBS}; do
    ${CPYCMD} ${QTDIR}/lib/${x}* ${PKGDIR}/lib
  done
  for x in ${QTBINS}; do
    ${CPYCMD} ${QTDIR}/bin/${x}* ${PKGDIR}/bin
  done
fi

${CPYCMD} -r ${QTDIR}/plugins ${PKGDIR}/lib/

      echo "QWTDIR=${QWTDIR}"
if [ -z "${QWTDIR}" ]; then
  QWTDIR=`ldd bin/u* lib/*|grep -i qwt.so|sed -n 1p`
  QWTDIR=`echo ${QWTDIR} | awk '{print $3}' | sed -e 's@lib/libqwt.*$@@'`
fi
      echo "QWTDIR=${QWTDIR}"
if [ ! -z "${QWTDIR}" ]; then
  ${CPYCMD} ${QWTDIR}/lib/libqwt.so* ${PKGDIR}/lib/.
fi

# Strip libs and bins
echo "Stripping debugging symbols..."
strip ${PKGDIR}/bin/u*
strip ${PKGDIR}/lib/lib*

# Remove .svn
cd ${PKGDIR}
find ./ -name '.svn' | xargs rm -Rf
ls -lF

# If 64-bit, create bin64, lib64 links
if [ ${IS64} -ne 0 ]; then
  if [ ! -d bin64 ]; then
    ln -s bin bin64
  fi
  if [ ! -d lib64 ]; then
    ln -s lib lib64
  fi
fi

# Create the package archive and display it
cd ${DSTDIR}
tar -cvzf ${PKGNAME}.tar.gz ${PKGNAME}
pwd
ls -lF
cd ${PKGDIR}
pwd
ls -lF

