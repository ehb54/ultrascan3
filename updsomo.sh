#!/bin/bash
#  Do GIT update of us_somo

cd $us3/us_somo
pwd
git pull
cat develop/include/us_revision.h

exit 0

