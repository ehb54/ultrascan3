#!/bin/sh
#	appnames.sh  -  fix install names for all apps (Mac only)

CDIR=`pwd`
BDIR=`dirname $0`
if [ -z "${BDIR}" -o "${BDIR}" = "." ]; then
  BDIR=${CDIR}
fi
BDIR=`echo ${BDIR} | sed -e 's@/bin$@@'`
#		insure we work from the ~/ultrascan3/bin directory
cd ${BDIR}/bin
 
#		fix the application in each bundle
for BUND in `ls -d *.app`;do

  FILE=`echo ${BUND} | sed -e 's/.app//'`

  APPP=${BUND}/Contents/MacOS/${FILE}

#		get list of library names to change
  LIBL=`otool -L ${APPP} \
    | egrep 'libus|qwt|qt-mt' \
    | grep -v executable \
    | awk '{print $1}'`

#		change each of the library names
  for NAMI in ${LIBL}; do
    
    #		use relative path to library
    NAMO=@executable_path/../../../../lib/${NAMI}

#		report and do the install name change
    echo "install_name_tool -change ${NAMI} ${NAMO} ${APPP}"
    install_name_tool -change ${NAMI} ${NAMO} ${APPP}

  done

done

cd ${CDIR}
