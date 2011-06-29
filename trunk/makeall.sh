#!/bin/bash

QMAKEVER=`qmake --version|grep version|grep 4`
if [ "x$QMAKEVER" == "x" ]; then
  echo "Wrong qmake, check environment"
  exit
fi

ISMAC=0
FIXMAC=""
if [ "`uname -s`" = "Darwin" ]; then
  ISMAC=1
  FIXMAC=./fix-mac-make.sh
fi

DIR=$(pwd)
rm -f build.log
NBERR=0

for d in utils gui qwtplot3d-qt4 programs/*
do
  if [ ! -d $d                          ]; then continue; fi
  if [ $d == "programs/config2"         ]; then continue; fi
  if [ $d == "programs/us_mpi_analysis" ]; then continue; fi
  pushd $d
  sdir=`pwd`
  if [ $ISMAC -eq 0 ]; then
    qmake *.pro
  else
    qmake -spec /usr/local/Qt4.7/mkspecs/macx-g++ *.pro
    if [ "$d" = "gui" ]; then
      ${FIXMAC}
    fi
  fi
  echo "Making in $d"   >> $DIR/build.log
  (cd $sdir;make 2>&1)  >> $DIR/build.log
  stat=$?
  if [ $stat -gt 0 ]; then
     echo "  ***ERROR*** building $d"
     NBERR=`expr ${NBERR} + 1`
  fi
  popd
done

if [ $ISMAC -eq 0 ]; then
  doxygen >> $DIR/build.log
else
  $DIR/libnames.sh >> $DIR/build.log
  $DIR/appnames.sh >> $DIR/build.log
fi

if [ $NBERR -gt 0 ]; then
  echo "*** $NBERR Build Error(s) ***"
  echo "*** $NBERR Build Error(s) ***" >> $DIR/build.log
fi
