# One time script to update all active US3 databases
#  by creating the DMGA method

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/database

for db in uslims3_cauma3

do
 echo "Updating $db"
 mysql $LOGIN $db < ./alter_dmga_settings.sql
done

##for db in `cat ~us3/database/dblist_upds.txt`
##for db in `cat ~us3/database/dblist.txt`
for db in `cat ~us3/database/dblist_upds.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./update_dmga.sql
done
