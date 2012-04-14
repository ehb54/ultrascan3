#!/bin/bash
#	somoupd.sh  - execute commands to accomplish somo update

##CNVTEXT=us3conv.hydrodyn.txt
CNVTEXT=us3conv.txt
US3BASE=$HOME
US2BASE=${US3BASE}
USDBASE=${US3BASE}
QMAKE="qmake"
SUNAME=`uname -s`
ISMAC=`echo $SUNAME | grep -ci "darwin"`
ISWIN=`echo $SUNAME | grep -ci "cygwin"`
ISUNX=`echo $SUNAME | grep -ci "linux"`

if [ $ISMAC -eq 0 -a $ISWIN -eq 0 -a $ISUNX -eq 0 ]; then
  echo "UNAME -s == $SUNAME  :   not a valid system for build"
  if [ `echo $SUNAME | grep -ci "mingw" ` -ne 0 ]; then
    echo "Build for Windows (initially) in a Cygwin window"
  fi
  exit 1
fi

if [ $ISMAC -ne 0 ]; then
  QMAKE="qmake -spec macx-g++"
  US3BASE=~/us3
  US2BASE=~/us2
fi
if [ $ISWIN -ne 0 ]; then
  US3BASE=/cygdrive/c/Users/Admin/Documents/
  US2BASE=${US3BASE}
  USDBASE=${US3BASE}
fi
export QMAKE

if [ -z "$us2" ]; then
  echo " *** '$us2' is being set: ***"
  export us2=$US2BASE/ultrascan2
  echo "    us2=$us2"
fi
if [ -z "$us3" ]; then
  echo " *** '$us3' is being set: ***"
  export us3=$US3BASE/ultrascan3
  echo "    us3=$us3"
fi
if [ -z "$us2sdev" ]; then
  echo " *** '$us2sdev' is being set: ***"
  export us2sdev=$USDBASE/us2s/develop
  echo "    us2sdev=$us2sdev"
fi
if [ -d $us2sdev ]; then
  echo " +++ removing $us2sdev +++"
  /bin/rm -rf $us2sdev
fi
if [ -f tocopy.txt ]; then
  echo " +++ removing tocopy.txt +++"
  /bin/rm -f tocopy.txt
fi
cd $us3/somo
echo "   +++ us2tosomoonly.pl +++"
perl us2tosomoonly.pl 2>&1
echo "   --- us2tosomoonly COMPLETE ---"
echo "   +++ us2somoto3.pl +++"
perl us2somoto3.pl 2>&1
echo "   --- us2somoto3 COMPLETE ---"

if [ $ISWIN -ne 0 ]; then
  cd $us3/somo/develop/src
  rm -f qwt
  mv qwt5 qwt
  cd ../include
  rm -f qwt
  mv qwt5 qwt
  cd ../
  pwd
  ls -ld src/qwt* include/qwt*
fi

cd $us3/somo/develop
${QMAKE} libus_somo.pro 2>&1

if [ $ISMAC -ne 0 ]; then
  echo "   +++ copying ultrascan.icns +++"
  cp -p us_license/ultrascan.icns us3_hydrodyn/
  cp -p us_license/ultrascan.icns us_hydrodyn/
  cp -p us_license/ultrascan.icns us_saxs_cmds_t/
fi

if [ $ISWIN -ne 0 ]; then
  echo "Do the next Windows build steps in an MSYS window:"
  echo "  > cd $us3/somo/develop"
  echo "  > make"
  echo ""
  echo "Come back to this Cygwin window and do the next qmake:"
  echo "  > ${QMAKE} us_somo.pro"
  echo ""
  echo "Then, in the MSYS window, another make"
  echo "  > make"
  echo ""
  exit 0
fi

cp Makefile Makefile-lib
if [ -f qmake_image_collection.cpp ]; then
  echo "    +++ removing qmake_image_collection.cpp +++"
  /bin/rm -f qmake_image_collection.cpp
fi
echo "   +++ making qmake_image_collection +++"
make -j1 src/obj/qmake_image_collection.o 2>&1
echo "   --- make qmake_image_collection COMPLETE ---"

echo "   +++ making somo library +++"
make -j2 2>&1
echo "   --- make somo library COMPLETE ---"
${QMAKE} us_somo.pro 2>&1
cp Makefile Makefile-all
echo "   +++ making somo applications +++"
make -j2 2>&1
echo "   --- make somo applications COMPLETE ---"
 
if [ $ISMAC  -ne 0 ]; then
  echo "   --- somo libnames, appnames ---"
  $us3/somo/bin/libnames.sh
  $us3/somo/bin/appnames.sh
fi
