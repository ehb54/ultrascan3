# One time script to update all active US3 databases
#  by updating the timestate stored procedures

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/lims/database/sql
##for db in uslims3_cauma3
##for db in `cat /home/us3/database/dblist.txt`
##for db in `cat /home/us3/database/dblist_upds.txt`

for db in `cat ~us3/database/dblist_upds.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./us3_timestate_procs.sql
done

