#!/bin/bash

DIR=$(pwd)
rm -f build.log

for d in utils gui db programs/*
do
  if [ ! -d $d ]; then continue; fi
  if [ $d == "programs/config2" ]; then continue; fi
  pushd $d
  qmake *.pro
  echo "Making in $d" >> $DIR/build.log
  make -j44           >> $DIR/build.log
  popd
done

doxygen >> $DIR/build.log
