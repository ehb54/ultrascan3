#!/bin/bash
#  Do SVN update of us3_somo

cd $us3/../us3_somo
pwd
svn up
cat develop/include/us_revision.h

exit 0

