#!/bin/bash

MKARGS="$@"
if [ $# -eq 0 ]; then
  MKARGS="-j 7"
  if [ `uname -s|grep -ci "mingw"` -ne 0 ]; then
    MKARGS="-j 2"
  fi
fi
export MAKE="make ${MKARGS}"

ISMAC=0
FIXMAC=""
if [ "`uname -s`" = "Darwin" ]; then
  ISMAC=1
  FIXMAC=./fix-mac-make.sh
fi

ISWIN=0
if [ `uname -s|grep -ci "msys"` -ne 0 ]; then
  ISWIN=1
fi
if [ `uname -s|grep -ci "mingw"` -ne 0 ]; then
  ISWIN=1
fi
if [ `uname -s|grep -ci "cygwin"` -ne 0 ]; then
  ISWIN=2
fi

DIR=$(pwd)
NBERR=0
##SOMO3=`(cd $us3/../us3_somo;pwd)`
SOMO3=`(cd $us3/us_somo;pwd)`

if [ $ISWIN -eq 2 ]; then
  # Run revision and qmake in Cygwin window
  cd $SOMO3/develop
  pwd
  ./version.sh
  qmake us_somo.pro
  cp Makefile Makefile-all
  cp Makefile.Release Makefile.R-all
  cp Makefile.Debug Makefile.D-all
  qmake libus_somo.pro
  cp Makefile Makefile-lib
  cp Makefile.Release Makefile.R-lib
  cp Makefile.Debug Makefile.D-lib
  cd $SOMO3
  ls -l Make*
  echo "QMAKE complete. Rerun $0 in MSYS (MINGW32) window"
  exit 0
fi

if [ $ISWIN -eq 1 ]; then
  # Run makes for lib,all in MSYS window
  cd $SOMO3/develop
  pwd
  cp Makefile-lib Makefile
  cp Makefile.R-lib Makefile.Release
  cp Makefile.D-lib Makefile.Debug
  make
  cp Makefile-all Makefile
  cp Makefile.R-all Makefile.Release
  cp Makefile.D-all Makefile.Debug
  make
  echo "MAKE of somo complete"
  cd ../
  ls -l ./bin
  cp -p bin/* ../ultrascan3/bin/
  cp -rp etc ../ultrascan3/
  exit 0
fi

# Do makes for Linux,Mac
echo "rsync -av --exclude .svn $SOMO3/etc $us3"
rsync -av --exclude .svn $SOMO3/etc $us3

cd $SOMO3/develop
sh version.sh
qmake us_somo.pro
cp -p Makefile  Makefile-all
qmake libus_somo.pro
cp -p Makefile  Makefile-lib
${MAKE} -f Makefile-lib
${MAKE} -f Makefile-all
cd $SOMO3

if [ $ISMAC -ne 0 ]; then
  echo "RUN libnames, appnames"
  ./somo_libnames.sh
  ./somo_appnames.sh
fi

ls -lrt ./lib ./bin64
echo ""
echo "rsync -av --exclude .svn $SOMO3/lib/ $us3/lib"
rsync -av --exclude .svn $SOMO3/lib/ $us3/lib
echo "rsync -av --exclude .svn $SOMO3/bin64/ $us3/bin"
rsync -av --exclude .svn $SOMO3/bin64/ $us3/bin
echo ""
echo "MAKE of somo complete"

exit 0

