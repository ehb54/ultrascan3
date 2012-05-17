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

echo "===  Creating a somo doc archive  ==="
tar -cvf - $TARGS | gzip --stdout > ${TFILE}

ls -l ${TFILE}
echo "zcat ${TFILE} | tar -tvf - |wc"
zcat ${TFILE} | tar -tvf - |wc

echo "===  Copying somo doc files from US2 to US3 package  ==="
cd ${SRCDIR}
echo 'cp -p ${TARGS} ~/ultrascan3/somo/doc/manual/'
cp -p ${TARGS} ~/ultrascan3/somo/doc/manual/

