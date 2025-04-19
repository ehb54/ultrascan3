#!/bin/bash

QTVERS=`qmake --version|sed -n 2,2p|awk '{print $4}'`
echo "Make All for Qt Version ${QTVERS} ..."
QTMAJV=`echo ${QTVERS}|cut -d. -f1`
if [ `echo "45"|grep -ci "${QTMAJV}"` -eq 0 ]; then
  echo "Wrong qmake, check environment"
  echo " QT Major version must be 4 or 5; is ${QTMAJV}"
  qmake --version
  exit
fi

MKARGS="$@"
if [ $# -eq 0 ]; then
  MKARGS="-j 7"
  if [ `uname -s|grep -ci "msys"` -ne 0 ]; then
    MKARGS="-j 2"
  fi
  if [ `uname -s|grep -ci "mingw"` -ne 0 ]; then
    MKARGS="-j 2"
  fi
  echo "MKARGS=$MKARGS"
fi
export MAKE="make ${MKARGS}"

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
  (cd programs/us;./revision.sh)
  echo "*** A Windows build must be done in the MSYS window!!! ***"
  exit 1
fi

ISL64=0
if [ `uname -m|grep -ci "x86_64"` -ne 0 ]; then
  ISL64=1
fi

DOMAN=1
DODOX=1

if [ -z "`which tpage`" ]; then
  DOMAN=0
fi
if [ -z "`which doxygen`" ]; then
  DODOX=0
fi
if [ -z "`which latex`" ]; then
  DODOX=0
fi
if [ -z "`which dvips`" ]; then
  DODOX=0
fi

DIR=$(pwd)
rm -f build.log
NBERR=0

for d in qwtplot3d utils gui programs/*
do
  if [ ! -d $d                          ]; then continue; fi
  if [ $d == "programs/config2"         ]; then continue; fi
  if [ $d == "programs/us_1dsa"         ]; then continue; fi
  if [ $d == "programs/us_mpi_analysis" ]; then continue; fi
  if [ $d == "programs/us_mwla_viewer"  ]; then continue; fi
  pushd $d
  sdir=`pwd`
  if [ $ISMAC -eq 0 ]; then
    qmake *.pro
  else
    qmake *.pro
    if [ "$d" = "gui" ]; then
      ${FIXMAC}
    fi
  fi
  echo "Making in $d"   >> $DIR/build.log
  (cd $sdir;${MAKE} 2>&1)  >> $DIR/build.log
  stat=$?
  if [ $stat -gt 0 ]; then
     echo "  ***ERROR*** building $d"
     NBERR=`expr ${NBERR} + 1`
  fi
  popd
done

if [ $DOMAN -ne 0 ]; then
  d=doc/manual
  pushd $d
  sdir=`pwd`
  echo "Making in $d"   >> $DIR/build.log
  ## n.b. the make failes with parallel builds, require -j1
  make -j1 2>&1  >> $DIR/build.log
  stat=$?
  if [ $stat -gt 0 ]; then
     echo "  ***ERROR*** building $d"
     NBERR=`expr ${NBERR} + 1`
  fi
  popd
fi

if [ $DODOX -ne 0 ]; then
  echo "Running doxygen ..."
  ##doxygen >> $DIR/build.log 2>$DIR/doxy.err
  doxygen >> $DIR/build.log &
else
  echo "NO Doxygen used"
fi

if [ $ISMAC -ne 0 ]; then
  echo "Running libnames and appnames ..." |tee -a $DIR/build.log
  $DIR/libnames.sh >> $DIR/build.log
  $DIR/appnames.sh >> $DIR/build.log
fi

if [ $NBERR -gt 0 ]; then
  echo "*** $NBERR Build Error(s) ***"
  echo "*** $NBERR Build Error(s) ***" >> $DIR/build.log
fi
