# Script to update the list of DBs to be altered

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD="select * from metadata"
DB="newus3"

mysql ${LOGIN} -sNe "$CMD" $DB >metadata_list.txt

