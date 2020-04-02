#!/bin/bash
#  Do SVN update of us3_somo

##cd $us3/../us3_somo
cd $us3/us_somo
pwd
##svn up
git pull
cat develop/include/us_revision.h

exit 0

