#!/bin/bash
# Script to count projects with bad GUIDs

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD="SELECT count(*) FROM project WHERE projectGUID LIKE ' %';"

##for db in `cat ~us3/lims/database/dblist.txt`
##for db in uslims3_cauma3
for db in `cat ~us3/lims/database/dblist.txt`
do
  echo "Examining $db"
  mysql ${LOGIN} -sNe "${CMD}" $db
done
