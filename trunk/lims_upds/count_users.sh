# One time script to update all active US3 databases
#  by creating the PCSA method

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~/lims/database

##for db in `cat ~us3/database/dblist_upds.txt`
##for db in `cat ~us3/database/dblist_upds.txt`
##for db in uslims3_cauma3
for db in `cat ~us3/lims/database/dblist.txt`

do
 echo "counting users from $db"
 mysql $LOGIN $db < ./us3_count_allusers.sql
done
