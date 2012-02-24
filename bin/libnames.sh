#!/bin/sh
#	libnames.sh  -  fix install names for all libraries (Mac only)

CDIR=`pwd`
BDIR=`dirname $0`
if [ -z "${BDIR}" -o "${BDIR}" = "." ]; then
  BDIR=${CDIR}
fi
BDIR=`echo ${BDIR} | sed -e 's@/bin$@@'`
#		work from the ~/ultrascan3/lib directory
cd ${BDIR}/lib

	echo "PWD=`pwd`"
LIBS="libus.9.9.0.dylib"
 
#		fix each of the us3 libraries
for FILE in ${LIBS}; do
	echo "	FILE=${FILE}"

#		get list of names that need fixing
  LIBL=`otool -L ${FILE} \
    | egrep 'qwt|qt-mt' \
    | grep -v executable \
    | grep -v ${FILE} \
    | awk '{print $1}'`

#		fix each of the shared library names
  for NAMI in ${LIBL}; do
	echo "		NAMI=${NAMI}"
    
    #		give relative path to library
    NAMO=@executable_path/../../../../lib/${NAMI}

#		report and do the fix
    echo "install_name_tool -change ${NAMI} ${NAMO} ${FILE}"
    install_name_tool -change ${NAMI} ${NAMO} ${FILE}

  done

done

cd ${CDIR}
