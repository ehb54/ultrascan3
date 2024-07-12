#!/bin/sh
#  version.sh  - update us_version with latest US_Version value
DESF=./include/us_version.h
VER1=`grep US_Version $DESF |cut -d\" -f2`
REV1=`grep SOMO_Revision $DESF |cut -d\" -f2`
##VER2=`grep US_Version $us3/utils/us_defines.h|cut -d\" -f2|sed -n 1p`
VER2=`sed -n '/0x0500/,/define/p' $us3/utils/us_defines.h|grep US_Vers|cut -d'"' -f2`
REV2="SOMOgit-`(cd $us3/us_somo;git log --oneline . | wc -l)`"

if [ "$VER1" != "$VER2"  -o  "$REV1" != "$REV2" ]
then
  
  VER1=`echo $VER1 | sed s/\r//`    # Change LF/CR on Windows to LF
  VER2=`echo $VER2 | sed s/\r//`
  REV1=`echo $REV1 | sed s/\r//`
  REV2=`echo $REV2 | sed s/\r//`
  if [ "$VER1" != "$VER2" ]; then
    echo "Setting US3 version to $VER2 from $VER1"
  fi
  if [ "$REV1" != "$REV2" ]; then
    echo "Setting US3 SOMO revision to $REV2 from $REV1"
  fi
  
  echo "#ifndef US_VERSION_H"                         >$DESF
  echo "#define US_VERSION_H"                        >>$DESF
  echo ""                                            >>$DESF
  echo "#define US_Version_string \"$VER2\""         >>$DESF
  echo "#define WIN32Version      \"-WIN32-$VER2\""  >>$DESF
  echo "#define SOMO_Revision     \"$REV2\""         >>$DESF
  echo ""                                            >>$DESF
  echo "#endif"                                      >>$DESF
else
  echo "  No Changes:  still $VER1 and $REV1"
  echo "===  $DESF :  ==="
  cat ./include/us_version.h
fi

