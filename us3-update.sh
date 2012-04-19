#!/bin/sh
#	us3-update.sh   - interactive script to install or update UltraScan3

#     Get the directory from which this script was launched
SRCDIR=`dirname $0`
if [ "${SRCDIR}" = "." ]; then
  SRCDIR=`pwd`
else
  TEMP=${SRCDIR}
  SRCDIR=`echo ${SRCDIR}|sed -e 's@^\/@@'`
  if [ "${SRCDIR}" != "${TEMP}" ]; then
    SRCDIR=${TEMP}
  else
    SRCDIR=`pwd`/${SRCDIR}
  fi
fi
         echo "   SRCDIR=${SRCDIR}"
CONFILE=$HOME/.config/UltraScan3/UltraScan.conf

if [ -f ${CONFILE} ]; then
  #      Existing config file implies UPDATE
  UBFILES="ArchiveDir|tmpDir"
  UWFILES="dataDir|resultDir|reportDir"
  echo "This appears to be an UPDATE"
  #      Get bin,work base directories from config
  UBINDIR=`egrep "${UBFILES}" ${CONFILE}|sed -n 1p|cut -d "=" -f2`
  UWRKDIR=`egrep "${UWFILES}" ${CONFILE}|sed -n 1p|cut -d "=" -f2`
         echo "   (1)UBINDIR=${UBINDIR}"
         echo "   (1)UWRKDIR=${UWRKDIR}"
  if [ -z "${UBINDIR}" ]; then
    UBINDIR=${HOME}/ultrascan3
  else
    UBINDIR=`dirname ${UBINDIR}`
  fi
  if [ -z "${UWRKDIR}" ]; then
    UWRKDIR=${HOME}/ultrascan
  else
    UWRKDIR=`dirname ${UWRKDIR}`
  fi
         echo "   (2)UBINDIR=${UBINDIR}"
         echo "   (2)UWRKDIR=${UWRKDIR}"
else
  #      Non-existing config file implies new INSTALL
  echo "This appears to be an new INSTALL";
  UBINDIR=${HOME}/ultrascan3
  UWRKDIR=${HOME}/ultrascan
         echo "   (3)UBINDIR=${UBINDIR}"
         echo "   (3)UWRKDIR=${UWRKDIR}"
fi

echo "Inferred install directories are:"
echo "  bin,... : ${UBINDIR}"
echo "  data,...: ${UWRKDIR}"
 
echo "Are these the correct destination directories? (y|n, def=y)"
read ANS
if [ "X${ANS}" != "X" -a "X${ANS}" != "Xy" ]; then
  echo "Enter base 'bin,...' directory (def=${UBINDIR}) :"
  read ANS
  if [ "X${ANS}" != "X" ]; then
    UBINDIR=${ANS}
  fi
  echo "Enter base 'data,...' directory (def=${UWRKDIR}) : "
  read ANS
  if [ "X${ANS}" != "X" ]; then
    UWRKDIR=${ANS}
  fi
         echo "   (4)UBINDIR=${UBINDIR}"
         echo "   (4)UWRKDIR=${UWRKDIR}"
fi

if [ -d ${UBINDIR} ]; then
  # Bin base exists:  check its ownership
  OWNER=`ls -ld ${UBINDIR}|awk '{print $3}'`
else
  # No bin base exists:  check ownership of its base
  TEMP=`dirname ${UBINDIR}`
  if [ ! -d ${TEMP} ]; then
    echo "*** Error:  ${TEMP} does not exist ***"
    exit 1
  else
    OWNER=`ls -ld ${TEMP}|awk '{print $3}'`
  fi
  if [ "${OWNER}" = "root" ]; then
    sudo mkdir ${UBINDIR}
  else
    mkdir ${UBINDIR}
  fi
fi

if [ ! -d ${UWRKDIR} ]; then
  # Data base does not exist:  make directories
  TEMP=`dirname ${UWRKDIR}`
  if [ ! -d ${TEMP} ]; then
    echo "*** Error:  ${TEMP} does not exist ***"
    exit 1
  else
    mkdir ${UWRKDIR}
  fi
fi

# Create BIN directory sub-directories
for D in bin etc lib somo; do
  DDIR=${UBINDIR}/${D}
  if [ ! -d ${DDIR} ]; then
    if [ "${OWNER}" = "root" ]; then
      sudo mkdir ${DDIR}
    else
      mkdir ${DDIR}
    fi
  fi
done

if [ "${SRCDIR}" = "${UBINDIR}" ]; then
  # Report and exit if "to" same as "from"
  echo "Install-to is same as install-from: ${UBINDIR}"
  echo "  No copy of directories and files is needed."
  exit 0
fi

# Install by copying directories and files
RSYNC="rsync -av"
if [ "${OWNER}" = "root" ]; then
  RSYNC="sudo ${RSYNC}"
fi
for D in bin etc lib somo; do
  SDIR=${SRCDIR}/${D}
  DDIR=${UBINDIR}
  echo "${RSYNC} ${SDIR} ${DDIR}"
  ${RSYNC} ${SDIR} ${DDIR}
done

