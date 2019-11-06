# One time script to update all active US3 databases
#  by updating the expdata stored procedures

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##for db in `cat dblist.txt`
for db in uslims3_cauma3

do
 echo "Updating $db"
 mysql $LOGIN $db < us3_mrecs_procs.sql
done
