#  revision.sh  - fill us_revision.h with lastest revision string
QTVER=`qmake --version|grep Qt|cut -d' ' -f4|cut -d'.' -f1`
if [ ${QTVER} = "4" ];then
  echo "Qt Version 4";
  LNCMD="sed -n 2p"
else
  echo "Qt Version 5";
  LNCMD="sed -n 1p"
fi
MVERS=`grep US_Version ../../utils/us_defines.h|${LNCMD}|cut -d'"' -f2`
REV1=`svn info|grep 'Changed Rev'|cut -f4 -d' '`
REV2=`(cd ../../utils;svn info|grep 'Changed Rev'|cut -f4 -d' ')`
DATE1=`svn info|grep 'Changed Date'|cut -f4 -d' '`
DATE2=`(cd ../../utils;svn info|grep 'Changed Date'|cut -f4 -d' ')`
# Compare revisions of programs/us_mpi_analysis and utils
NREV1=`expr $REV1 + 0`
NREV2=`expr $REV2 + 0`
REV1="$MVERS.$REV1 $DATE1"
REV2="$MVERS.$REV2 $DATE2"
if [ $NREV2 -gt $NREV1 ]; then
  # If utils is latest, use its revision to store in the header
  REV1=$REV2
fi

# Get the current header revision string and compare to svn-based string
REV2=`cut -f2 -d\" us_revision.h`
if [ "$REV1" != "$REV2" ]; then
  # If svn shows revision later than header, report and store that one
  echo "REVISION WAS -- \"$REV2\""
  echo "REVISION NOW -- \"$REV1\""
  echo "#define REVISION \"$REV1\"" >us_revision.h
fi

