#!/bin/bash
#	somoupd.sh  - execute commands to accomplish somo update

##CNVTEXT=us3conv.hydrodyn.txt
CNVTEXT=us3conv.txt
QMAKE="qmake"
if [ `uname -s | grep -ci "darwin"` -ne 0 ]; then
  QMAKE="qmake -spec macx-g++"
fi
export QMAKE

if [ -z "$us2" ]; then
  echo " *** '$us2' is being set: ***"
  export us2=~/us2/ultrascan2
  echo "    us2=$us2"
fi
if [ -z "$us3" ]; then
  echo " *** '$us3' is being set: ***"
  export us3=~/us3/ultrascan3
  echo "    us3=$us3"
fi
if [ -z "$us2sdev" ]; then
  echo " *** '$us2sdev' is being set: ***"
  export us2sdev=~/us2s/develop
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
##echo "   +++ us3conv.pl ${CNVTEXT} +++"
##perl us3conv.pl -c ${CNVTEXT} 2>&1
##echo "   --- us3conv COMPLETE ---"
cd $us3/somo/develop
${QMAKE} libus_somo.pro 2>&1
cp Makefile Makefile-lib
if [ -f qmake_image_collection.cpp ]; then
  echo "    +++ removing qmake_image_collection.cpp +++"
  /bin/rm -f qmake_image_collection.cpp
fi
if [ `uname -s | grep -ci "darwin"` -ne 0 ]; then
  echo "   +++ copying ultrascan.icns +++"
  cp -p us_license/ultrascan.icns us3_hydrodyn/
  cp -p us_license/ultrascan.icns us_hydrodyn/
  cp -p us_license/ultrascan.icns us_saxs_cmds_t/
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
 
if [ `uname -s | grep -ci "darwin"` -ne 0 ]; then
  echo "   --- somo libnames, appnames ---"
  $us3/somo/bin/libnames.sh
  $us3/somo/bin/appnames.sh
fi
