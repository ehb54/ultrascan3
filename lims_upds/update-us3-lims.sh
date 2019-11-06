#!/bin/bash
# One time script to update all active US3 LIMS code

cd /srv/www/htdocs/uslims3

##for db in `cat ~us3/lims/database/dblist.txt`
##for db in uslims3_cauma3
##for db in `cat ~us3/lims/database/dblist_upds.txt`
for db in `cat ~us3/lims/database/dblist.txt`
do
  echo "Updating $db"
  pushd $db
  svn up
  popd
  sleep 2

done
