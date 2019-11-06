# One time script to update all active US3 databases
#  by adding lastUpdated field to project table
. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"

##for db in uslims3_cauma3;
for db in `cat dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./alter_project.sql
done
