# One time script to update US3 database 'us3_notice'
#  by adding the notice table

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
for db in us3_notice

do
 echo "Updating $db"
 mysql $LOGIN $db < ./add_notice.sql
done
