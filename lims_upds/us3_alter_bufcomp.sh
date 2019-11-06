# One time script to update all active US3 databases
#  by altering contents of the bufferComponent table

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/database

##for db in `cat ~us3/database/dblist.txt`
##for db in uslims3_cauma3
##for db in `cat ~us3/database/dblist_upds.txt`
for db in uslims3_cauma3

do
 echo "Updating $db"
 mysql $LOGIN $db < sql/us3_buffer_components.sql
done

