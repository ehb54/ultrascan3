# One time script to update all active US3 databases
#  by updating the people w userlevel=4,5

. ~us3/lims/database/getdbpw
echo ""
LOGIN="-u root -p$PASSW"
cd ~us3/lims/database
##for db in `cat dblist.txt`
##for db in `cat dblist_upds.txt`

for db in `cat dblist.txt`
do
 echo "Updating $db"
 mysql $LOGIN $db < ./update_lev4_people.sql
done

