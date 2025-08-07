#!/bin/sh
#  revision.sh  - update us_revision.h with latest svn REVISION value
REV1="$(git log --oneline | wc -l)"
REV2=`cut -d\" -f2 us_revision.h`
if [ "$REV1" != "$REV2" ]
then
  echo "Setting revision to $REV1"
  # Remove return character from string when running under cygwin
  REV1=`echo $REV1 | sed s/\r//`
  echo "#define REVISION \"$REV1\"" > us_revision.h
fi

