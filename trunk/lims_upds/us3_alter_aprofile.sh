# One time script to update all active US3 databases
#  by modifying/adding analysisprofile-related tables

LOGIN="-u root -p`cat ~/.sec/.pwsq`"
##for db in `cat dblist.txt`
##for db in `cat dblist_upds.txt`
##for db in uslims3_uthealth
for db in uslims3_CCH

do
 echo "Updating $db"
 mysql $LOGIN $db < ./alter_aprofile.sql
done

