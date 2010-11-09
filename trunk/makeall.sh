#!/bin/bash

DIR=$(pwd)
rm -f build.log
NBERR=0

for d in utils gui db qwtplot3d-qt4 programs/*
do
  if [ ! -d $d                          ]; then continue; fi
  if [ $d == "programs/config2"         ]; then continue; fi
  if [ $d == "programs/us_mpi_analysis" ]; then continue; fi
  pushd $d
  sdir=`pwd`
  qmake *.pro
  echo "Making in $d"   >> $DIR/build.log
  (cd $sdir;make 2>&1)  >> $DIR/build.log
  stat=$?
  if [ $stat -gt 0 ]; then
     echo "  ***ERROR*** building $d"
     NBERR=`expr ${NBERR} + 1`
  fi
  popd
done

doxygen >> $DIR/build.log

if [ $NBERR -gt 0 ]; then
  echo "*** $NBERR Build Error(s) ***"
  echo "*** $NBERR Build Error(s) ***" >> $DIR/build.log
fi
