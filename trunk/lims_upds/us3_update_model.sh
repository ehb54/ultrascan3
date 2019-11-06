# One time script to update all active US3 databases
#  by updating the exp stored procedures

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##for db in uslims3_cauma3
##for db in `cat dblist.txt`
##for db in `cat dblist_upds.txt`
for db in `cat dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < sql/us3_model_procs.sql
done

