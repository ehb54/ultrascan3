#!/bin/bash
# One time script to update all active US3 LIMS code

cd /home/us3/database

for d in \
        lims/html          \
        sql        

do
  echo "Updating $d"
  pushd $d
  svn up
  popd
  sleep 2

done
