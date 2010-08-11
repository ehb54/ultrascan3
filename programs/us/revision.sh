#!/bin/sh
#  revision.sh  - update us_revision.h with latest svn REVISION value
SURL="//bcf.uthscsa.edu/ultrascan3/trunk"
REV1=`svn info svn:${SURL}|grep Revision`
REV2=`cut -d\" -f2 us_revision.h`
if [ "$REV1" != "$REV2" ]
then
  
  echo "Setting revision to $REV1"
  # Remove return character from string when running under cygwin
  REV1=`echo $REV1 | sed s/\r//`
  echo "#define REVISION \"$REV1\"" > us_revision.h
fi

