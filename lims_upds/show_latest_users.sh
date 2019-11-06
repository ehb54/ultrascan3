# Script to update the list of DBs to be altered

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD="select DISTINCT Investigator_Name, Email from investigators where LastLogin like '2015%'"
CMD="${CMD} Order by Investigator_Name"

mysql ${LOGIN} -sNe "${CMD}" uslims3_global

