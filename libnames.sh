#!/bin/sh
#	libnames.sh  -  fix install names for all libraries (Mac only)

CDIR=`pwd`
BDIR=`dirname $0`
if [ -z "${BDIR}" -o "${BDIR}" = "." ]; then
  BDIR=${CDIR}
fi
BDIR=`echo ${BDIR} | sed -e 's@/bin$@@'`

#		work from the ~/us3_somo/lib directory
cd ${BDIR}/lib
LIBQWT=`ls libqwt.*.*.*.dylib`

LIBS="utils gui"
FILES="\
 libus_somo.10.0.0.dylib \
 libus_gui.10.0.0.dylib \
 libus_utils.10.0.0.dylib \
 ${LIBQWT} \
 libqwtplot3d.0.2.7.dylib \
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
    | egrep '_utils|_gui|qwt|qwtplot3d|mysql' \
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
