#!/bin/bash

for d in utils gui programs/*
do
  pushd $d
  make clean
  rm -f *~
  popd
done

rm -f *~
