#!/bin/sh

# run from the ultrascan root to create unix-based tar.gz 
# archive of UltraScan
#
###########################################################
#
# please edit the variables in the following section:
#
###########################################################

VERSION=9.9
TMP=/tmp

# change to QT version:
QT=3.3.8

# change to lib64/bin64 for opteron environment
LIB=lib
BIN=bin

echo "Using " $QTDIR " for the QT Library..."
echo "Using " $QWTDIR " for the QWT Library..."
echo "Using " $QWT3DDIR " for the QWT3D Plotting Library..."

echo "OK? (y/n):"

read r

if [ "$r" == "N" -o "$r" == "n" ]; then 
	exit -1
fi

# uncomment one of the following:

HARDWARE=ix86
#HARDWARE=opteron
#HARDWARE=sgi
#HARDWARE=sparc
#HARDWARE=mac

# uncomment one of the following:

OS=linux
#OS=solaris-10
#OS=solaris-9
#OS=solaris-8
#OS=solaris-7
#OS=irix-6.5
#OS=osx
#OS=freebsd
#OS=netbsd
#OS=openbsd

###########################################################
#
# comment the flash file that is not to be deleted
#
###########################################################

if [ ! -d $LIB ]; then
	echo "ultrascan/$LIB directory not found, exiting..."
	exit
fi

if [ ! -d doc ]; then
	echo "ultrascan/doc directory not found, exiting..."
	exit
fi

if [ ! -d etc ]; then
	echo "ultrascan/etc directory not found, exiting..."
	exit
fi

if [ ! -d $BIN ]; then
	echo "ultrascan/bin directory not found, exiting..."
	exit
fi

if [ ! -d develop ]; then
	echo "ultrascan/develop directory not found, exiting..."
	exit
fi

echo "Creating UltraScan backup archive..."
echo "Removing /tmp/ultrascan*..."
rm -rf $TMP/ultrascan*
echo executing: mkdir $TMP/ultrascan-$VERSION
mkdir $TMP/ultrascan-$VERSION
echo copying everything $TMP/ultrascan-$VERSION/
cp -Rpf * $TMP/ultrascan-$VERSION/.

echo "removing CVS stuff..."
cd $TMP/ultrascan-$VERSION
find . -name .svn | xargs rm -Rf

echo "removing unneeded document files..."
rm us.spec
rm TODO
rm out
rm make_tardistro.sh
rm Doxyfile
rm ultrascan*
rm change*
rm maketd
rm *~
rm Makefile
cd $TMP/ultrascan-$VERSION/doc
rm *.html
rm *.php
rm header
rm *.css
rm -rf develop
rm -rf software
rm -rf user
rm -rf HOWTO
rm -rf workshop
cd graphics
rm *_*.gif
rm bg*
rm penguin.gif
rm related.gif
rm ultrascan1.gif
rm mlist.gif
cd $TMP/ultrascan-$VERSION/etc
#rm \#*
rm lock*
rm .tigre*
rm check_*
rm database.dat
rm db_admin.pd
rm hosts.beowulf 
rm ultra-8bit.png
rm mpi*
rm us_*
rm *~
rm SelectResource.pm
rm datetest
rm *.pl
rm out
#rm .\#*
#rm \#*
rm tigre*
rm flash-freebsd.png
rm flash-irix.png
#rm flash-linux.png
rm flash-macosx.png
rm flash-netbsd.png
rm flash-openbsd.png
rm flash-opteron.png
rm flash-solaris.png
rm flash-windows.png
cd $TMP/ultrascan-$VERSION/bin64
rm mpi*
rm *~
rm tigre*
#rm \#*
cd $TMP/ultrascan-$VERSION

echo "removing sources..."
rm -rf develop
rm Changelog*

echo "adjusting permissions..."
cd $TMP/ultrascan-$VERSION/$BIN
chmod 700 us_admin
strip *
rm *.bak
rm *test
cd $TMP/ultrascan-$VERSION/$LIB

echo "Copying data from $QWTDIR/$LIB/* ..."
cp -Rpf $QWTDIR/$LIB/* .
echo "Copying data from $QWT3DDIR/$LIB/* ..."
cp -Rpf $QWT3DDIR/$LIB/* .
strip *
cd $TMP
chown -R root ultrascan-$VERSION
chgrp -R root ultrascan-$VERSION

echo "creating archive..."
cd $TMP
tar -cf ultrascan-$VERSION-qt-$QT-$HARDWARE-$OS.tar ultrascan-$VERSION

echo "compressing ultrascan-$VERSION-qt-$QT-$HARDWARE-$OS.tar..."
gzip -9 ultrascan-$VERSION-qt-$QT-$HARDWARE-$OS.tar

echo "Making QT distribution..."
rm -rf $TMP/qt-$QT*
cd $QTDIR/lib
tar -cf $TMP/qt-$QT-$HARDWARE-$OS.tar libqt-mt*
gzip -9 $TMP/qt-$QT-$HARDWARE-$OS.tar

echo "cleaning up..."
rm -rf $TMP/ultrascan-$VERSION
echo ""
cd $TMP
echo "The archives are located in $TMP ..."
ls -l ultrascan-$VERSION-qt-$QT-$HARDWARE-$OS.tar.gz qt-$QT-$HARDWARE-$OS.tar.gz

echo "Copying the archives to the biochem server..."
#scp ultrascan-$VERSION-qt-$QT-$HARDWARE-$OS.tar.gz root@biochem.uthscsa.edu:/home/www/ultrascan/software/.
#scp qt-$QT-$HARDWARE-$OS.tar.gz root@biochem.uthscsa.edu:/home/www/ultrascan/software/.
#rm $TMP/ultrascan-$VERSION-qt-$QT-$HARDWARE-$OS.tar.gz qt-$QT-$HARDWARE-$OS.tar.gz

echo "all done..."
echo ""
