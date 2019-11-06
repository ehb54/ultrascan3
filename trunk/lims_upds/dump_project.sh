# Script to update the list of DBs to be altered

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
##CMD="select * from project where projectID=63"
##CMD="select * from project where projectID=66"
CMD="select * from project where projectID=64"

mysql ${LOGIN} -sNe "${CMD}" uslims3_HI

