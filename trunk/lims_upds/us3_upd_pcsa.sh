# One time script to update all active US3 databases
#  by creating the PCSA method

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/database

##for db in `cat ~us3/database/dblist_upds.txt`
##for db in `cat ~us3/database/dblist_upds.txt`
##for db in uslims3_cauma3
for db in `cat ~us3/database/dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./update_pcsa.sql
done
