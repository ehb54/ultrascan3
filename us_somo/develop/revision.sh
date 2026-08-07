#!/bin/bash
#	revision.sh
 
cd $ULTRASCAN/us_somo
RDATE=`git log -1 --shortstat .|grep Date:|sed -e 's/Date:   //'`
SHASH=`git log -1 --oneline .| cut -d' ' -f1`
RNUMB=`git log --oneline | sed -n "/$SHASH/,99999p" | wc -l`
cd develop/include
if [ -f us_revision.h ]; then
  HDATE=`grep REVISION_DATE us_revision.h | cut -d\" -f2`
else
  HDATE=""
fi

if [ "$HDATE" != "$RDATE" ]; then
  echo "Setting revision to ${RNUMB}"
  echo "#define REVISION \"Revision: ${RNUMB}\"" |tee us_revision.h
  echo "#define REVISION_DATE \"${RDATE}\""      |tee -a us_revision.h
cat us_revision.h
pwd
ls -l us_revision.h
else
  echo "us_revision.h:   revision remains ${RNUMB} ${HDATE}"
fi

