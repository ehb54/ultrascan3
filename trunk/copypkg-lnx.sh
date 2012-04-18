#!/bin/sh
#		copypkg-lnx.sh  - copy ultrascan3 directories to us3pkg/PKGNAME

SRCDIR=$HOME/ultrascan3
DSTDIR=$HOME/us3pkg
ME=`whoami`
SURL="//bcf.uthscsa.edu/ultrascan3/trunk"
REV=`svn info svn:${SURL}|grep Revision|awk '{print $2}'`
RSYNC="rsync -av --exclude=.svn"

TEMP=`which gcc`
TEMP=`file ${TEMP} | egrep -ci '64-bit|x86-64'`
if [ ${TEMP} -ne 0 ]; then
  PKGNAME=ultrascan-Linux64-1.0-rev${REV}
else
  PKGNAME=ultrascan-Linux32-1.0-rev${REV}
fi
PKGDIR=${DSTDIR}/${PKGNAME}
cd $HOME
if [ ! -d us3pkg ]; then
  mkdir us3pkg
fi
cd ${DSTDIR}
if [ ! -d ${PKGNAME} ]; then
  mkdir ${PKGNAME};
fi
echo "PKGDIR=${PKGDIR}"

(cd ${PKGDIR};mkdir etc bin lib)
for D in etc bin lib; do
  SDIR=${SRCDIR}/${D}
  DDIR=${PKGDIR}
  echo "${RSYNC} ${SDIR} ${DDIR}"
  ${RSYNC} ${SDIR} ${DDIR}
done

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
  for D in demo test; do
    SDIR=${SOMODIR}/${D}
    DDIR=${PKGDIR}/somo
    echo "${RSYNC} ${SDIR} ${DDIR}"
    ${RSYNC} ${SDIR} ${DDIR}
  done
fi

cd ${PKGDIR}
find ./ -name '.svn' | xargs rm -Rf
ls -lF

cd ${DSTDIR}
tar -cvf - ${PKGNAME} | gzip --stdout > ${PKGNAME}.tgz
pwd
ls -lF
cd ${PKGDIR}
pwd
ls -lF
