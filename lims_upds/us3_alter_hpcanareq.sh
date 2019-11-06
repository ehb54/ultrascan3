# One time script to update all active US3 databases
#  by creating the DMGA method

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
cd ~us3/lims/database

##for db in `cat ~us3/lims/database/dblist.txt`
##for db in uslims3_cauma3
##for db in `cat ~us3/lims/database/dblist_upds.txt`
##for db in uslims3_cauma3d
##for db in `cat ~us3/lims/database/dblist.txt |grep -v uslims3_cauma3`
for db in uslims3_UPENN
do
 echo "Updating $db"
 mysql $LOGIN $db < ./alter_hpcreq2.sql
done
## mysql $LOGIN $db < ./alter_hpcreq.sql
## mysql $LOGIN $db < ./alter_hpcreq2.sql
