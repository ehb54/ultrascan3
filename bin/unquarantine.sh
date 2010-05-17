#!/bin/sh
#	unquarantine.sh
 
CDIR=`pwd`
BDIR=`dirname $0`
if [ -z "${BDIR}" -o "{$BDIR}" = "." ]; then
  BDIR=${CDIR}
fi
cd ${BDIR}
FILES1=`ls -d us_[a-z]*.app`
FILES2=us.app

for F in ${FILES1} ${FILES2} ; do
  STAT=`xattr -d com.apple.quarantine ${F} 2>&1 | grep -c "^No such xattr"`
##	echo "STAT=${STAT}"
  if [ ${STAT} -ne 0 ]; then
    echo "File ${F} was already un-quarantined"
  else
    echo "File ${F} is now un-quarantined"
  fi
done
##	echo "BDIR=${BDIR}"
