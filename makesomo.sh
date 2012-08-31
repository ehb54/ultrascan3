#!/bin/bash

ISMAC=0
FIXMAC=""
if [ "`uname -s`" = "Darwin" ]; then
  ISMAC=1
  FIXMAC=./fix-mac-make.sh
fi

ISWIN=0
if [ `uname -s|grep -ci "mingw"` -ne 0 ]; then
  ISWIN=1
fi

if [ `uname -s|grep -ci "cygwin"` -ne 0 ]; then
  ISWIN=2
fi

DIR=$(pwd)
NBERR=0

if [ $ISWIN -eq 2 ]; then
  # Run revision and qmake in Cygwin window
  cd $us3/somo/develop
  pwd
  ./revision.sh
  qmake us_somo.pro
  cp Makefile Makefile-all
  cp Makefile.Release Makefile.R-all
  cp Makefile.Debug Makefile.D-all
  qmake libus_somo.pro
  cp Makefile Makefile-lib
  cp Makefile.Release Makefile.R-lib
  cp Makefile.Debug Makefile.D-lib
  ls -l Make*
  echo "QMAKE complete. Rerun $0 in MingW window"
  exit 0
fi

if [ $ISWIN -eq 1 ]; then
  # Run makes for lib,all in MingW window
  cd /c/Users/Admin/Documents/ultrascan3/somo/develop
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
  cd ../bin
  pwd
  ls -l
  exit 0
fi

cd $us3/somo/develop
./revision.sh
qmake us_somo.pro
cp -p Makefile  Makefile-all
qmake libus_somo.pro
cp -p Makefile  Makefile-lib
make -j2 -f Makefile-lib
make -j2 -f Makefile-all
ls -lrt ../lib ../bin
echo "MAKE of somo complete"

if [ $ISMAC -ne 0 ]; then
  echo "RUN libnames, appnames"
  cd ../
  ./bin/libnames.sh
  ./bin/appnames.sh
fi

exit 0

