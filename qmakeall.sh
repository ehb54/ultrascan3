#!/bin/bash

# WIN32 program that does a make all clean and regenerates makefiles from .pro files

for d in utils gui programs/*
do
   if [ ! -d $d                          ]; then continue; fi
   if [ $d == "programs/config2"         ]; then continue; fi
   if [ $d == "programs/us_mpi_analysis" ]; then continue; fi

   pushd $d
   rm -rf obj moc Make* vc* *.ncb *.sln *vcproj* *.rc *.suo generatedfiles debug release
   qmake *.pro
   popd
done
