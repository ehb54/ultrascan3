# One time script to update all active US3 databases
#  by altering/updating ClusterAuthorizations
. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"

for db in `cat dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./add_manual.sql
done