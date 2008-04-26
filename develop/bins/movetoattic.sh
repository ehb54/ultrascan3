#!/bin/sh
for x in us_displp  us_simdlg; do
		 cd $ULTRASCAN/develop/src
		 mv $x.cpp $ULTRASCAN/develop/attic/.
		 svn del $x.cpp
		 cd $ULTRASCAN/develop/include
		 mv $x.h $ULTRASCAN/develop/attic/.
		 svn del $x.h
		 cd $ULTRASCAN/develop/attic
		 svn add $x.*
done
cd $ULTRASCAN/develop
ls
