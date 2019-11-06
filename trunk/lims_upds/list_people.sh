# One time script to list people in most-used databases

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##for db in `cat dblist.txt`
for db in uslims3_CU_Boulder uslims3_NVAX

do
 echo "==========================================================="
 echo "Email list from $db"
 mysql $LOGIN $db < ./list_people.sql
done
