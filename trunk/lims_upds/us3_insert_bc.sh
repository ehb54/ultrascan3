# One time script to update all active US3 databases
#  by newly inserting all bufferComponents entries

LOGIN="-u root -p`cat ~/.sec/.pwsq`"
cd ~us3/lims/database
##for db in `cat dblist.txt`
##for db in uslims3_devel
##for db in uslims3_cauma3
for db in `cat dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db <sql/us3_buffer_components.sql
done

