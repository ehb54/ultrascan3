#!/bin/sh
#	make-somodoc.sh

TFILE=~/ultrascan3/somo/arc/somodoc.tgz
SRCDIR=~/ultrascan2

cd ${SRCDIR}
TARGS="\
 doc/manual/Deb* \
 doc/manual/lic* \
 doc/manual/somo* \
 doc/manual/ultra* \
 doc/manual/us_* \
 doc/manual/version* \
 "

tar -cvf - $TARGS | gzip --stdout > ${TFILE}

ls -l ${TFILE}
echo "zcat ${TFILE} | tar -tvf - |wc"
zcat ${TFILE} | tar -tvf - |wc

