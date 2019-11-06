# One time script to update all active US3 databases
#  by adding the protocol and protocolPerson tables

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
#cd ~us3/database

##for db in uslims3_cauma3
##for db in uslims3_cauma3 uslims3_CAUMA
##for db in `cat dblist.txt`
for db in `cat ~us3/lims/database/dblist_cch.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db < ./add_autoflow.sql
done
