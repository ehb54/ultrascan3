# One time script to update stored procedures for all active US3 databases
. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/lims/database/sql

for db in `cat ~us3/lims/database/dblist.txt`
do
 echo "Updating $db"
 mysql $LOGIN $db < ./us3_procedures.sql
done
