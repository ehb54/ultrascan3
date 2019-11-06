#!/bin/bash
# One time script to update all active US3 LIMS code

cd /data/srv/www/htdocs/uslims3

USER=$1
. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD="SELECT fname,lname,country,organization FROM people WHERE lname like '"
CMD="${CMD}${USER}"
CMD="${CMD}%'"
  echo "CMD=$CMD"
##CMD='SELECT fname,lname,country,organization FROM people'
##for db in `cat ~us3/lims/database/dblist.txt`
##for db in uslims3_cauma3
##for db in `cat ~us3/lims/database/dblist_upds.txt`
##for db in `cat ~us3/lims/database/dblist.txt`
for db in `cat ~us3/lims/database/dblist.txt`
do
  echo "== Searching $db for $USER =="
  pushd $db
  echo $CMD | mysql $LOGIN $db
  popd
  sleep 1

done
