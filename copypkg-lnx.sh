#!/bin/sh
#		copypkg-lnx.sh  - copy ultrascan3 directories to us3pkg/PKGNAME

SRCDIR=$HOME/ultrascan3
DSTDIR=$HOME/us3pkg
ME=`whoami`
SYSTYPE=`uname -s`
if [ "${SYSTYPE}" != "Linux" ]; then
  echo "$0 requires a SYSTYPE (uname -s) of \"Linux\""
  echo "  SYSTYPE=${SYSTYPE}"
  exit 1
fi
SURL="//bcf.uthscsa.edu/ultrascan3/trunk"
REV=`svn info svn:${SURL}|grep Revision|awk '{print $2}'`
RSYNC="rsync -av --exclude=.svn"
REVL=`svn info ${SRCDIR}|grep Revision|awk '{print $2}'`
VERS="1.0"

if [ "${REV}" != "${REVL}" ]; then
  # Abort if source is not latest revision
  echo "Rev from ${SURL}:   ${REV}"
  echo "Rev from ${SRCDIR}:   ${REVL}"
  echo "  Do 'svn update' and build, then re-run this script"
  exit 1
fi

QTLIBS=" \
 libQtCore.so.4
 libQtDBus.so.4
 libQtGui.so.4
 libQtHelp.so.4
 libQtNetwork.so.4
 libQtOpenGL.so.4
 libQtSql.so.4
 libQtSvg.so.4
 libQtWebKit.so.4
 libQtXml.so.4
 libQtCLucene.so.4
 libQt3Support.so.4
 libphonon.so.4
 "
QTBINS="assistant"

# Determine if this is 32-bit or 64-bit
TEMP=`which gcc`
TEMP=`file ${TEMP} | egrep -ci '64-bit|x86-64'`
if [ ${TEMP} -ne 0 ]; then
  PKGNAME=ultrascan-Linux64-${VERS}-rev${REV}
else
  PKGNAME=ultrascan-Linux32-${VERS}-rev${REV}
fi
PKGDIR=${DSTDIR}/${PKGNAME}

# Insure we have a package base directory
cd $HOME
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

# Remove the SOMO bins
US3BINS=`(cd ${SRCDIR}/somo/bin;ls)`
for F in ${US3BINS}; do
  echo "/bin/rm -f ${PKGDIR}/bin/${F}"
  /bin/rm -f ${PKGDIR}/bin/${F}
done

# Leave only *.new files in etc/somo
for F in ${PKGDIR}/etc/somo*; do
  if [ `echo ${F} | grep -ic '.new$'` -eq 0 ]; then
    # Remove any etc/somo* that does not end in '.new'
    /bin/rm -f ${F}
  fi
done

# Copy the SOMO directories
SDIR=${SRCDIR}/somo/lib
DDIR=${PKGDIR}
echo "${RSYNC} ${SDIR} ${DDIR}"
${RSYNC} ${SDIR} ${DDIR}
SDIR=${SRCDIR}/somo/bin
DDIR=${PKGDIR}
echo "${RSYNC} ${SDIR} ${DDIR}"
${RSYNC} ${SDIR} ${DDIR}
(cd ${PKGDIR};mkdir somo somo/doc somo/demo somo/test);
SDIR=${SRCDIR}/somo/doc
DDIR=${PKGDIR}/somo
echo "${RSYNC} ${SDIR} ${DDIR}"
${RSYNC} ${SDIR} ${DDIR}

SOMODIR=$HOME/ultrascan/somo
if [ -d ${SOMODIR} -a -d ${SOMODIR}/demo ]; then
  # Clear out somo/demo so only present contents are copied
  /bin/rm -rf ${PKGDIR}/somo/demo/*
  # Copy somo demo,test contents
  for D in demo test; do
    SDIR=${SOMODIR}/${D}
    DDIR=${PKGDIR}/somo
    echo "${RSYNC} ${SDIR} ${DDIR}"
    ${RSYNC} ${SDIR} ${DDIR}
  done
fi

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

# Create the package archive and display it
cd ${DSTDIR}
tar -cvzf ${PKGNAME}.tar.gz ${PKGNAME}
pwd
ls -lF
cd ${PKGDIR}
pwd
ls -lF
