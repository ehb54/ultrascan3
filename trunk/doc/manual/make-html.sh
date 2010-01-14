#!/bin/sh
#	make-html.sh	make HTML from HEADER,BODY,FOOTER files
 
#		header and footer files
HFILE=header.us3
FFILE=footer.us3

#		give usage two required parameters not given
if [ $# -lt 2 ]; then
  echo "	make HTML from HEADER,BODY,FOOTER files"
  echo "USAGE:"
  echo "	make-html.sh <body_name> <title> \\"
  echo "	             [-q|--silent] [-f]"
  echo " <body_name>  = base name of .body file used and .html file output"
  echo " <title>      = title string (enclose in double quotes if blank(s) embedded)"
  echo " -q (OR)"
  echo " --silent     = flags quiet/silent mode (no stdout) for external calls"
  echo " -f           = flags force overwrite of any existing output"
  exit 1
fi

#		get body name and title, plus any flags
NA=$#
BNAME=$1
TITLE=$2
QUIET=0
OVERW=0
HAVEN=0
while [ $NA -gt 0 ]; do
  ARG=$1
  shift
  NA=`expr ${NA} - 1`
  if [ `echo ${ARG}|grep -c '^-'` -ne 0 ]; then
    # get flag type argument (starts with dash)
    if [ `echo ${ARG}|grep -ci 's'` -ne 0 -o `echo ${ARG}|grep -ci 'q'` -ne 0 ]; then
      QUIET=1
    fi
    if [ `echo ${ARG}|grep -ci 'f'` -ne 0 ]; then
      OVERW=1
    fi
  else
    # get name or title argument
    if [ ${HAVEN} -eq 0 ]; then
      BNAME=${ARG}
      HAVEN=1
    else
      TITLE="${ARG}"
    fi
  fi
done

#		verify body name and title
BFILE=${BNAME}.body
OFILE=${BNAME}.html
if [ ! -f ${BFILE} ]; then
  # no such body file: report (non-silent) and exit
  if [ ${QUIET} -eq 0 ]; then
    echo "** File ${BFILE} does not exist! **"
  fi
  exit 2
fi
if [ -f ${OFILE} ]; then
  # output file already exists
  RECR="re-"
  if [ ${QUIET} -eq 0  -a  ${OVERW} -eq 0 ]; then
    # in non-silent mode ask whether user wants to overwrite
    echo "File ${OFILE} already exists. Overwrite? (y|n; def=y) "
    ANS=y
    read ANS
    if [ "${ANS}" = "n" -o "${ANS}" = "N" ]; then
      echo "	++ File ${OFILE} left as before ++"
      exit 3
    fi
  else
    # in silent mode, overwrite only if so flagged
    if [ ${OVERW} -eq 0 ]; then
      exit 3
    fi
  fi
else
  RECR=""
fi

#		create date string in two forms from body file last-mod date
DATE1="`date -r ${BFILE} --rfc-3339=second`"
DATE2="`date -r ${BFILE} --rfc-2822|awk '{print " ("$1" "$2" "$3" "$4")"}'`"
FDATE="${DATE1}${DATE2}"

#		create HTML: edited-HEADER + BODY + edited-FOOTER
sed -e "s/__TITLE__/${TITLE}/" ${HFILE}  >${OFILE}
cat ${BFILE}                            >>${OFILE}
sed -e "s/__DATE__/${FDATE}/" ${FFILE}  >>${OFILE}

#		report on HTML file creation
if [ ${QUIET} -eq 0 ]; then
  echo "	File ${OFILE} has been ${RECR}created, with"
  echo "		Title of \"${TITLE}\", and"
  echo "		Date of \"${FDATE}\"."
fi
exit 0

