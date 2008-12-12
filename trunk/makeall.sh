#!/bin/bash

DIR=$(pwd)
rm build.log

for d in utils gui db programs/*
do
  pushd $d
  qmake *.pro
  echo "Making in $d" >> $DIR/build.log
  make                >> $DIR/build.log
  popd
done
