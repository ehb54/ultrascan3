#!/bin/bash

if [ $# -lt 1 ]; then
  echo "`basename $0` <base_name>"
  echo "  <base_name> = base name for .qxx? files"
  exit 1
fi
BASENM="$1"

qhelpgenerator ${BASENM}.qhp -o ${BASENM}.qch
qcollectiongenerator ${BASENM}.qhcp -o ${BASENM}.qhc

mv ${BASENM}.qch ${BASENM}.qhc ../../bin

