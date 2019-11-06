# One time script to update all active US3 databases
#  by adding the protocol and protocolPerson tables

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##for db in uslims3_cauma3
##for db in uslims3_cauma3 uslims3_CAUMA
##for db in `cat dblist.txt`
for db in uslims3_cauma3 uslims3_CAUMA

do
 echo "Updating $db"
 mysql $LOGIN $db < ./add_protocol.sql
done
