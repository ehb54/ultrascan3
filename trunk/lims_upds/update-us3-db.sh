# One time script to update all active US3 databases

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##for db in uslims3_cauma3
for db in `cat dblist.txt`
do
 echo "Updating $db"
 mysql $LOGIN $db < ./alter_ripro_type.sql
done

