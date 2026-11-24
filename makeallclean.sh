#!/bin/bash

for d in utils gui programlibs/* programs/*
do
  if [ ! -d $d ]; then continue; fi
  pushd $d
  make clean
  rm -f *~
  popd
done

rm -f *~
rm -f utils/obj/us_dataIO2.o

