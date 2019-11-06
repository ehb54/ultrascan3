# One time script to update all active US3 databases
#  by altering/updating clusterAuthorizations

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##for db in uslims3_cauma3
##for db in `cat dblist.txt`
##for db in `cat dblist_clustauth.txt`
##for db in `cat dblist.txt`

for db in `cat dblist_clustauth.txt`
do
 echo "Updating $db"
 mysql $LOGIN $db < ./alter_cluster_auth.sql
done
