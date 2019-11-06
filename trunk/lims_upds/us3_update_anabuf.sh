# One time script to update all active US3 databases
#  by updating the exp stored procedures

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/lims/database
sqldir=sql
##sqldir=~gegorbet/work
##for db in uslims3_cauma3
##for db in `cat dblist.txt`
##for db in `cat dblist_upds.txt`
for db in uslims3_Uni_DuE

do
 echo "Updating $db"
 mysql $LOGIN $db < $sqldir/us3_analyte_procs.sql
 mysql $LOGIN $db < $sqldir/us3_buffer_procs.sql
done

