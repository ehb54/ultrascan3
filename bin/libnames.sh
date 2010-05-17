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

##LIBS="utils gui db"
LIBS="qt-mt.3.3.8 qui.1.0.0 qwt.4.2.0 qwtplot3d.0.2.6 us.9.9.0"
 
#		fix each of the us3 libraries
for ULIB in ${LIBS}; do

  FILE="lib${ULIB}.dylib"

#		get list of names that need fixing
  LIBL=`otool -L ${FILE} \
    | egrep 'libus|qt-mt|qui|qwt|qca' \
    | grep -v executable \
    | grep -v ${FILE} \
    | awk '{print $1}'`

#		fix each of the shared library names
  for NAMI in ${LIBL}; do
    
    if [ `echo ${NAMI} | grep -ci qca` -eq 0 ]; then
      #		give relative path to library
      NAMO=@executable_path/../../../../lib/${NAMI}
    else
      #		give relative path to qca library
      NAMO=@executable_path/../../../../lib/qca
    fi

#		report and do the fix
    echo "install_name_tool -change ${NAMI} ${NAMO} ${FILE}"
    install_name_tool -change ${NAMI} ${NAMO} ${FILE}

  done

done

cd ${CDIR}
