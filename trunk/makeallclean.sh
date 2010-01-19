#!/bin/bash

for d in utils gui db programs/*
do
  pushd $d
  make clean
  rm -f *~
  popd
done

rm -f *~
