# Script to delete second entry in lab table

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD="delete from lab where name='UTHSCSA Lab #2'"

##for db in uslims3_cauma3
##for db in `cat dblist_upds.txt`
##for db in uslims3_devel
##for db in uslims3_AUCSOL
for db in `cat dblist.txt`
  do
  echo "delete from lab table of $db"
  mysql ${LOGIN} -sNe "${CMD}" $db
done
