# One time script to update all active US3 databases
#  by creating the new speedstep table
. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"

##for db in uslims3_cauma3
##for db in `cat /home/us3/database/dblist_upds.txt`
for db in `cat /home/us3/database/dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./add_speedSteps.sql
done
