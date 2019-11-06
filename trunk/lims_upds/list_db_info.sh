# One time script to list country,org information
#  for all active US3 databases

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD='SELECT fname,lname,country,organization FROM people WHERE userlevel=3;'
##for db in uslims3_Uni_KN
for db in `cat dblist.txt`

do
 echo "=== DB:  $db ==="
 echo $CMD | mysql $LOGIN $db
done

