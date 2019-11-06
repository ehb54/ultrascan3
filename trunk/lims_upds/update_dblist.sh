# Script to update the list of DBs to be altered

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"

mysql ${LOGIN} -sNe "SHOW DATABASES LIKE 'uslims3%'" | grep -v uslims3_global >dblist.txt
echo "us3" >> dblist.txt

