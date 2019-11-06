# One time script to add mgroupcount field to all active US3 databases

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/lims/database

##for db in uslims3_cauma3
for db in `cat dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./us3_update_groupcount.sql
done
