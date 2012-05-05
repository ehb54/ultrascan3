#!/bin/sh
#	libnames.sh  -  fix install names for all libraries (Mac only)

CDIR=`pwd`
BDIR=`dirname $0`
if [ -z "${BDIR}" -o "${BDIR}" = "." ]; then
  BDIR=${CDIR}
fi
BDIR=`echo ${BDIR} | sed -e 's@/bin$@@'`
BDIR=~/us3/ultrascan3
#		work from the ~/ultrascan3/lib directory
cd ${BDIR}/lib

LIBS="utils gui"
FILES="\
 libqwt.5.2.2.dylib \
 libqwtplot3d-qt4.0.2.7.dylib \
 libus_gui.10.0.0.dylib \
 libus_utils.10.0.0.dylib \
"

 
#		fix each of the us3 libraries
for FILE in ${FILES} ; do

  SHORTF=`echo "${FILE}" | sed -e "s/[0-9]\.[0-9]\.dylib/dylib/"`
#		get list of names that need fixing
  ##LIBL=`otool -L ${FILE} \
  ##  | egrep '_utils|_gui|qwt|qwtplot3d-qt4|framework|mysql' \
  ##  | grep -v executable \
  ##  | grep -v ${FILE} \
  ##  | awk '{print $1}'`
  LIBL=`otool -L ${FILE} \
    | egrep '_utils|_gui|qwt|qwtplot3d-qt4|mysql' \
    | grep -v executable \
    | grep -v ${FILE} \
    | grep -v ${SHORTF} \
    | grep -v Library \
    | awk '{print $1}'`

#		fix each of the shared library names
  for NAMI in ${LIBL}; do
    
    if [ `echo ${NAMI} | grep -ci qca` -eq 0 ]; then
      #		give relative path to library
      if [ `echo ${NAMI} | grep -ci mysql` -ne 0 ]; then
        NAMO=@executable_path/../../../../lib/libmysqlclient.18.dylib
      else
        if [ `echo ${NAMI} | grep -ci framework` -eq 0 ]; then
          NAMO=@executable_path/../../../../lib/${NAMI}
        else
          NAMO=@executable_path/../../../../lib/Frameworks/${NAMI}
        fi
      fi
    fi

#		report and do the fix
    echo "install_name_tool -change ${NAMI} ${NAMO} ${FILE}"
    install_name_tool -change ${NAMI} ${NAMO} ${FILE}

  done

done

cd ${CDIR}
