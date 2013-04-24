#!/bin/bash

for d in utils gui programs/*
do
  pushd $d
  make clean
  rm -f *~
  popd
done

rm -f *~
rm -f utils/obj/us_dataIO2.o

