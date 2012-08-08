#  revision.sh  - fill us_revision.h with lastest revision string
REV1=`svn info|grep 'Changed Rev'`
REV2=`(cd ../../utils;svn info|grep 'Changed Rev')`
# Compare revisions of programs/us_mpi_analysis and utils
NREV1=`echo $REV1|cut -f4 -d' '`
NREV2=`echo $REV2|cut -f4 -d' '`
NREV1=`expr $NREV1 + 0`
NREV2=`expr $NREV2 + 0`
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
  echo "#define REVISION \"$REV1\"" > us_revision.h
fi

