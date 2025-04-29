#!/bin/sh
# make-html.sh - make HTML from template files using tpage

if [ $# -lt 2 ]; then
  echo "USAGE: make-html.sh <body_name> <title> [-q|--silent] [-f] [-o output_dir]"
  exit 1
fi

# Get parameters
BNAME=$1
TITLE=$2
QUIET=0
OVERW=0
OUTPUT_DIR=""

# Process flags
shift 2
while [ $# -gt 0 ]; do
  case "$1" in
    -q|--silent) QUIET=1 ;;
    -f) OVERW=1 ;;
    -o)
      shift
      OUTPUT_DIR="$1"
      ;;
  esac
  shift
done

# Setup file paths
BFILE=${BNAME}.body
OFILE=${OUTPUT_DIR:+${OUTPUT_DIR}/}${BNAME}.html

# Create output directory if specified
if [ -n "$OUTPUT_DIR" ]; then
  mkdir -p "$OUTPUT_DIR"
fi

# Check if body file exists
if [ ! -f ${BFILE} ]; then
  [ ${QUIET} -eq 0 ] && echo "** File ${BFILE} does not exist! **"
  exit 2
fi

# Check if output file should be overwritten
if [ -f ${OFILE} ] && [ ${OVERW} -eq 0 ]; then
  if [ ${QUIET} -eq 0 ]; then
    echo "File ${OFILE} already exists. Overwrite? (y|n; def=y) "
    read ANS
    [ "${ANS}" = "n" -o "${ANS}" = "N" ] && exit 0
  else
    exit 0
  fi
fi

# Create date string
if date -v 1d > /dev/null 2>&1; then
  # BSD date (macOS)
  FDATE="$(date -r $(stat -f %m ${BFILE}))"
else
  # GNU date (Linux)
  FDATE="$(date -r ${BFILE} "+%Y-%m-%d %H:%M:%S (%a %b %d %Y)")"
fi

# Try template processing first
[ ${QUIET} -eq 0 ] && echo "Processing ${BFILE} -> ${OFILE}..."

# Use tpage for template processing
tpage --define title="${TITLE}" --define date="${FDATE}" ${BFILE} > ${OFILE} 2>/dev/null

# Check if template processing worked
if [ ! -s ${OFILE} ] || [ $(wc -c <${OFILE}) -lt 10 ]; then
  [ ${QUIET} -eq 0 ] && echo "Falling back to manual inclusion..."

  # Fall back to manual inclusion
  sed -e "s/__TITLE__/${TITLE}/" header.us3 > ${OFILE}
  grep -v 'INCLUDE' ${BFILE} >> ${OFILE}
  sed -e "s/__DATE__/${FDATE}/" footer.us3 >> ${OFILE}
fi

[ ${QUIET} -eq 0 ] && echo "Created: ${OFILE}"
exit 0